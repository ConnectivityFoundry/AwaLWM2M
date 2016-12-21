/************************************************************************************************************************
 Copyright (c) 2016, Imagination Technologies Limited and/or its affiliated group companies.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
 following conditions are met:
     1. Redistributions of source code must retain the above copyright notice, this list of conditions and the
        following disclaimer.
     2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
        following disclaimer in the documentation and/or other materials provided with the distribution.
     3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote
        products derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
************************************************************************************************************************/

#include <stdbool.h>
#include <string.h>

#include "lwm2m_debug.h"
#include "dtls_abstraction.h"

#include <errno.h>
#include <stdio.h>
#define mbedtls_printf     printf
#define mbedtls_fprintf    fprintf

#define mbedtls_time_t     time_t

#include "mbedtls/net.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "mbedtls/timing.h"
#include "mbedtls/pk.h"
#include "mbedtls/ssl_cookie.h"

typedef struct
{
    NetworkAddress * NetworkAddress;
    mbedtls_ssl_context Context;
    mbedtls_ssl_config Config;
    bool InUse;
    bool SessionEstablished;
    void * UserContext;
    uint8_t * Buffer;
    int BufferLength;
} DTLS_Session;

#ifndef MAX_DTLS_SESSIONS
    #define MAX_DTLS_SESSIONS 3
#endif

const char * DTLS_LibraryName = "mbedTLS";

static DTLS_Session sessions[MAX_DTLS_SESSIONS];

static uint8_t * certificate = NULL;
static int certificateLength = 0;
static AwaCertificateFormat certificateFormat;

static const char * pskIdentity = NULL;
static const uint8_t * pskKey = NULL;
static int pskKeyLength = 0;

static DTLS_NetworkSendCallback NetworkSend = NULL;

static int supportedCipherSuites[6];

static DTLS_Session * GetSession(NetworkAddress * address);
static void SetupNewSession(int index, NetworkAddress * networkAddress, bool client);
static void FreeSession(DTLS_Session * session);
static int DecryptCallBack(void * context, unsigned char * recieveBuffer, size_t receiveBufferLegth);
static int EncryptCallBack(void * context, const unsigned char * sendBuffer,size_t sendBufferLength);
static int PSKCallBack(void * parameter, mbedtls_ssl_context * context, const unsigned char * identity, size_t identityLength);
static int SSLSendCallBack(void * context, const unsigned char * sendBuffer, size_t sendBufferLength);


static mbedtls_entropy_context entropy;
static mbedtls_ctr_drbg_context secureRandom;
static mbedtls_timing_delay_context timer;
static mbedtls_x509_crt cacert;
static mbedtls_pk_context privateKey;
static mbedtls_ssl_cookie_ctx cookie_context;

#define SUCCESS (0)


void DTLS_Init(void)
{
    memset(sessions, 0, sizeof(sessions));
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&secureRandom);
    mbedtls_ctr_drbg_seed(&secureRandom, mbedtls_entropy_func, &entropy, NULL, 0);
    mbedtls_x509_crt_init(&cacert);
    mbedtls_pk_init(&privateKey);
    mbedtls_ssl_cookie_init(&cookie_context);
    mbedtls_ssl_cookie_setup(&cookie_context, mbedtls_ctr_drbg_random, &secureRandom);
}

void DTLS_Shutdown(void)
{
    int index;
    for (index = 0;index < MAX_DTLS_SESSIONS; index++)
    {
        if (sessions[index].InUse)
        {
            FreeSession(&sessions[index]);
        }
    }
    mbedtls_ctr_drbg_free(&secureRandom);
    mbedtls_entropy_free(&entropy);
}

void DTLS_Reset(NetworkAddress * address)
{
    DTLS_Session * session = GetSession(address);
    if (session)
    {
        FreeSession(session);
    }
}

void DTLS_SetCertificate(const uint8_t * cert, int certLength, AwaCertificateFormat format)
{
    if (certLength > 0)
    {
        if (mbedtls_pk_parse_key(&privateKey, (const unsigned char*)cert, certLength, NULL, 0) == 0
        &&  mbedtls_x509_crt_parse(&cacert, (const unsigned char*)cert, certLength) == 0)
        {
            certificate = (uint8_t *)cert;
            certificateLength = certLength;
            certificateFormat = format;
        }
    }
}

void DTLS_SetNetworkSendCallback(DTLS_NetworkSendCallback sendCallback)
{
    NetworkSend = sendCallback;
}

void DTLS_SetPSK(const char * identity, const uint8_t * key, int keyLength)
{
    if (keyLength > 0)
    {
        pskIdentity = identity;
        pskKey = key;
        pskKeyLength = keyLength;
    }
}

bool DTLS_Decrypt(NetworkAddress * sourceAddress, uint8_t * encrypted, int encryptedLength, uint8_t * decryptBuffer, int decryptBufferLength, int * decryptedLength, void *context)
{
    bool result = false;
    DTLS_Session * session = GetSession(sourceAddress);
    if (session)
    {
        session->Buffer = encrypted;
        session->BufferLength = encryptedLength;
        if (session->SessionEstablished)
        {
            *decryptedLength = mbedtls_ssl_read(&session->Context, decryptBuffer, decryptBufferLength);
            result = (*decryptedLength > 0);
            if (!result)
            {
                FreeSession(session);
                session = NULL;
            }
        }
        else
        {
            *decryptedLength = 0;
            session->SessionEstablished = (mbedtls_ssl_handshake(&session->Context) == SUCCESS);
            if (session->SessionEstablished)
                Lwm2m_Info("Session established");
        }
    }

    if (!session)
    {
        int index;
        for (index = 0;index < MAX_DTLS_SESSIONS; index++)
        {
            if (!sessions[index].InUse)
            {
                SetupNewSession(index, sourceAddress, false);
                sessions[index].UserContext = context;
                sessions[index].Context.f_send = SSLSendCallBack;
                sessions[index].Buffer = encrypted;
                sessions[index].BufferLength = encryptedLength;
                sessions[index].SessionEstablished = (mbedtls_ssl_handshake(&sessions[index].Context) == SUCCESS);
                break;
            }
        }
    }
    return result;
}

bool DTLS_Encrypt(NetworkAddress * destAddress, uint8_t * plainText, int plainTextLength, uint8_t * encryptedBuffer, int encryptedBufferLength, int * encryptedLength, void *context)
{
    bool result = false;
    DTLS_Session * session = GetSession(destAddress);
    if (session)
    {
        if (session->SessionEstablished)
        {
            session->Context.f_send = EncryptCallBack;
            session->Buffer = encryptedBuffer;
            session->BufferLength = encryptedBufferLength;
            int written = mbedtls_ssl_write(&session->Context, plainText, plainTextLength);
            if (written >= 0)
            {
                *encryptedLength = encryptedBufferLength - session->BufferLength;
                result = (*encryptedLength > 0);
            }
        }
        else
        {
            session->UserContext = context;
            session->Context.f_send = SSLSendCallBack;
            session->SessionEstablished = (mbedtls_ssl_handshake(&session->Context) == SUCCESS);
            if (session->SessionEstablished)
                Lwm2m_Info("DTLS Session established\n");
        }
    }
    else
    {
        int index;
        for (index = 0;index < MAX_DTLS_SESSIONS; index++)
        {
            if (!sessions[index].InUse)
            {
                SetupNewSession(index, destAddress, true);
                sessions[index].UserContext = context;
                sessions[index].Context.f_send = SSLSendCallBack;
                sessions[index].SessionEstablished = (mbedtls_ssl_handshake(&sessions[index].Context) == SUCCESS);
                break;
            }
        }
    }
    return result;
}

static DTLS_Session * GetSession(NetworkAddress * address)
{
    DTLS_Session * result = NULL;
    int index;
    for (index = 0;index < MAX_DTLS_SESSIONS; index++)
    {
        if (NetworkAddress_Compare(sessions[index].NetworkAddress,address) == 0)
        {
            result = &sessions[index];
            break;
        }
    }
    return result;
}

static void SetupNewSession(int index, NetworkAddress * networkAddress, bool client)
{
    int flags;
    DTLS_Session * session = &sessions[index];
    session->NetworkAddress = networkAddress;
    mbedtls_ssl_context * context = &session->Context;
    mbedtls_ssl_config * config = &session->Config;

    mbedtls_ssl_config_init(config);
    if (client)
        flags = MBEDTLS_SSL_IS_CLIENT;
    else
    {
        flags = MBEDTLS_SSL_IS_SERVER;
    }

    mbedtls_ssl_config_defaults(config, flags, MBEDTLS_SSL_TRANSPORT_DATAGRAM, MBEDTLS_SSL_PRESET_DEFAULT);
    mbedtls_ssl_conf_rng(config, mbedtls_ctr_drbg_random, &secureRandom);

    if (!client)
    {
        mbedtls_ssl_conf_dtls_cookies(config, NULL, NULL, &cookie_context);
    }

    int cipherIndex = 0;
    if (certificate || !pskIdentity)
    {
        supportedCipherSuites[cipherIndex] = MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8;
        cipherIndex++;
        supportedCipherSuites[cipherIndex] = MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256;
        cipherIndex++;
        if (certificate)
        {
            mbedtls_ssl_conf_own_cert(config, &cacert, &privateKey);
        }
        mbedtls_ssl_conf_authmode(config, MBEDTLS_SSL_VERIFY_OPTIONAL);
    }
    if (pskIdentity)
    {
        supportedCipherSuites[cipherIndex] = MBEDTLS_TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256;
        cipherIndex++;
        supportedCipherSuites[cipherIndex] = MBEDTLS_TLS_PSK_WITH_AES_128_CCM_8;
        cipherIndex++;
        supportedCipherSuites[cipherIndex] = MBEDTLS_TLS_PSK_WITH_AES_128_CBC_SHA256;
        cipherIndex++;
        if (client)
        {
            if (!certificate)
            {
                mbedtls_ssl_conf_psk(config,pskKey, pskKeyLength, pskIdentity, strlen(pskIdentity));
            }
        }
        else
        {
            mbedtls_ssl_conf_psk_cb(config, PSKCallBack, (void *)pskIdentity);
        }
    }
    supportedCipherSuites[cipherIndex] = 0;
    mbedtls_ssl_conf_ciphersuites(config, supportedCipherSuites);
    mbedtls_ssl_init(context);
    if (mbedtls_ssl_setup(context, config) == SUCCESS)
    {
        mbedtls_ssl_set_bio(context, session, SSLSendCallBack, DecryptCallBack, NULL);
        mbedtls_ssl_set_timer_cb(context, &timer, mbedtls_timing_set_delay, mbedtls_timing_get_delay);
        session->InUse = true;
    }
}

static void FreeSession(DTLS_Session * session)
{
    mbedtls_ssl_close_notify(&session->Context);
    mbedtls_ssl_session_reset(&session->Context);
    mbedtls_ssl_free(&session->Context);
    mbedtls_ssl_config_free(&session->Config);
    memset(session,0, sizeof(DTLS_Session));
}

static int DecryptCallBack(void * context, unsigned char * recieveBuffer, size_t receiveBufferLegth)
{
    ssize_t result;
    DTLS_Session * session = (DTLS_Session *)context;
    if (session->BufferLength > 0)
    {
        if (receiveBufferLegth < session->BufferLength)
        {
            result =  receiveBufferLegth;
        }
        else
        {
            result =  session->BufferLength;
        }
        memcpy(recieveBuffer, session->Buffer, result);
        session->BufferLength = session->BufferLength - result;
        session->Buffer += result;
    }
    else
    {
        result = MBEDTLS_ERR_SSL_WANT_READ;
    }
    return result;
}

static int EncryptCallBack(void * context, const unsigned char * sendBuffer, size_t sendBufferLength)
{
    int result;
    DTLS_Session * session = (DTLS_Session *)context;
    if (session->BufferLength > 0)
    {
        if (sendBufferLength < session->BufferLength)
        {
            result =  sendBufferLength;
        }
        else
        {
            result =  session->BufferLength;
        }
        memcpy(session->Buffer, sendBuffer, result);
        session->BufferLength = session->BufferLength - result;
        session->Buffer += result;
    }
    else
    {
        result = MBEDTLS_ERR_SSL_WANT_WRITE;
    }
    return result;
}

static int PSKCallBack(void * parameter, mbedtls_ssl_context * context, const unsigned char * identity, size_t identityLength)
{
    mbedtls_ssl_set_hs_psk(context, pskKey, pskKeyLength);
    return 0;
}

static int SSLSendCallBack(void * context, const unsigned char * sendBuffer, size_t sendBufferLength)
{
    ssize_t result;
    DTLS_Session * session = (DTLS_Session *)context;
    if (NetworkSend)
    {
        NetworkTransmissionError error = NetworkSend(session->NetworkAddress, sendBuffer, sendBufferLength, session->UserContext);
        if (error == NetworkTransmissionError_None)
            result = sendBufferLength;
        else
            result = 0;
    }
    else
    {
        result = 0;
    }
    return result;
}

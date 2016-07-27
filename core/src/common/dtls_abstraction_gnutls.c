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

#include <gnutls/gnutls.h>
#include <gnutls/x509.h>


//GnuTLS added DTLS in 2.99.0
#if ((GNUTLS_VERSION_MAJOR < 2) || ((GNUTLS_VERSION_MAJOR == 2) && (GNUTLS_VERSION_MINOR < 99)))
#error "This version of GnuTLS does not support DTLS"
#endif

typedef enum
{
    CredentialType_NotSet,
    CredentialType_ClientPSK,
    CredentialType_ServerPSK
}CredentialType;


typedef struct
{
    NetworkAddress * NetworkAddress;
    gnutls_session_t Session;
    void * Credentials;
    uint8_t CredentialType;
    bool SessionEstablished;
    void * UserContext;
    uint8_t * Buffer;
    int BufferLength;
}DTLS_Session;

#ifndef MAX_DTLS_SESSIONS
    #define MAX_DTLS_SESSIONS 3
#endif

const char * DTLS_LibraryName = "GnuTLS";

static DTLS_Session sessions[MAX_DTLS_SESSIONS];

static uint8_t * certificate = NULL;
static int certificateLength = 0;
static AwaCertificateFormat certificateFormat;

static const char * pskIdentity = NULL;

static  gnutls_datum_t pskKey;

static  DTLS_NetworkSendCallback NetworkSend = NULL;

//Comment out as init of DH params takes a while
//static gnutls_dh_params_t _DHParameters;
static gnutls_priority_t _PriorityCache;
static gnutls_certificate_credentials_t _CertCredentials = NULL;


static DTLS_Session * GetSession(NetworkAddress * address);
static void SetupNewSession(int index, NetworkAddress * networkAddress, bool client);
static void FreeSession(DTLS_Session * session);
static ssize_t DecryptCallBack(gnutls_transport_ptr_t context, void *recieveBuffer, size_t receiveBufferLegth);
static ssize_t EncryptCallBack(gnutls_transport_ptr_t context, const void * sendBuffer,size_t sendBufferLength);
static int PSKClientCallBack(gnutls_session_t session, char **username, gnutls_datum_t * key);
static int PSKCallBack(gnutls_session_t session, const char *username, gnutls_datum_t * key);
static ssize_t SSLSendCallBack(gnutls_transport_ptr_t context, const void * sendBuffer,size_t sendBufferLength);
#if GNUTLS_VERSION_MAJOR >= 3
static int ReceiveTimeout(gnutls_transport_ptr_t context, unsigned int ms);
static int CertificateVerify(gnutls_session_t session);
#endif



void DTLS_Init(void)
{
    memset(sessions,0,sizeof(DTLS_Session) * MAX_DTLS_SESSIONS);
    gnutls_global_init();
    //    unsigned int bits = gnutls_sec_param_to_pk_bits(GNUTLS_PK_DH, GNUTLS_SEC_PARAM_LEGACY);
    //    gnutls_dh_params_init(&_DHParameters);
    //    gnutls_dh_params_generate2(_DHParameters, bits);
    gnutls_priority_init(&_PriorityCache, "NONE:+VERS-ALL:+ECDHE-ECDSA:+ECDHE-PSK:+PSK:+CURVE-ALL:+AES-128-CCM-8:+AES-128-CBC:+MAC-ALL:-SHA1:+COMP-ALL:+SIGN-ALL:+CTYPE-X.509", NULL);
}

void DTLS_Shutdown(void)
{
    int index;
    for (index = 0;index < MAX_DTLS_SESSIONS; index++)
    {
        if (sessions[index].Session)
        {
            FreeSession(&sessions[index]);
        }
    }
    if (_CertCredentials)
    {
        gnutls_certificate_free_credentials(_CertCredentials);
        _CertCredentials = NULL;
    }
//  gnutls_dh_params_deinit(_DHParameters);
    gnutls_priority_deinit(_PriorityCache);
    gnutls_global_deinit();
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
        certificate = (uint8_t *)cert;
        certificateLength = certLength;
        certificateFormat = format;
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
        pskKey.data = (unsigned char *)key;
        pskKey.size = keyLength;
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
            *decryptedLength = gnutls_read(session->Session, decryptBuffer, decryptBufferLength);
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
            session->SessionEstablished = (gnutls_handshake(session->Session) == GNUTLS_E_SUCCESS);
            if (session->SessionEstablished)
                Lwm2m_Info("Session established");
        }
    }

    if (!session)
    {
        int index;
        for (index = 0;index < MAX_DTLS_SESSIONS; index++)
        {
            if (!sessions[index].Session)
            {
                SetupNewSession(index, sourceAddress, false);
                sessions[index].UserContext = context;
                gnutls_transport_set_push_function(sessions[index].Session, SSLSendCallBack);
                sessions[index].Buffer = encrypted;
                sessions[index].BufferLength = encryptedLength;
                sessions[index].SessionEstablished = (gnutls_handshake(sessions[index].Session) == GNUTLS_E_SUCCESS);
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
            gnutls_transport_set_push_function(session->Session, EncryptCallBack);
            session->Buffer = encryptedBuffer;
            session->BufferLength = encryptedBufferLength;
            int written = gnutls_write(session->Session, plainText, plainTextLength);
            if (written >= 0)
            {
                *encryptedLength = encryptedBufferLength - session->BufferLength;
                result = (*encryptedLength > 0);
            }
        }
        else
        {
            session->UserContext = context;
            gnutls_transport_set_push_function(session->Session, SSLSendCallBack);
            session->SessionEstablished = (gnutls_handshake(session->Session) == GNUTLS_E_SUCCESS);
            if (session->SessionEstablished)
                Lwm2m_Info("DTLS Session established\n");
        }
    }
    else
    {
        int index;
        for (index = 0;index < MAX_DTLS_SESSIONS; index++)
        {
            if (!sessions[index].Session)
            {
                SetupNewSession(index, destAddress, true);
                sessions[index].UserContext = context;
                gnutls_transport_set_push_function(sessions[index].Session, SSLSendCallBack);
                sessions[index].SessionEstablished = (gnutls_handshake(sessions[index].Session) == GNUTLS_E_SUCCESS);
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
    DTLS_Session * session = &sessions[index];
    session->NetworkAddress = networkAddress;
    unsigned int flags;
#if GNUTLS_VERSION_MAJOR >= 3
    if (client)
        flags = GNUTLS_CLIENT | GNUTLS_DATAGRAM | GNUTLS_NONBLOCK;
    else
        flags = GNUTLS_SERVER | GNUTLS_DATAGRAM | GNUTLS_NONBLOCK;
#else
    if (client)
        flags = GNUTLS_CLIENT;
    else
        flags = GNUTLS_SERVER;
#endif
    if (gnutls_init(&session->Session, flags) == GNUTLS_E_SUCCESS)
    {
        gnutls_transport_set_pull_function(session->Session, DecryptCallBack);
        gnutls_transport_set_push_function(session->Session, SSLSendCallBack);
#if GNUTLS_VERSION_MAJOR >= 3
        gnutls_transport_set_pull_timeout_function(session->Session, ReceiveTimeout);
#endif
        gnutls_transport_set_ptr(session->Session, session);

        if (certificate || !pskIdentity)
        {
            if (_CertCredentials)
            {
                gnutls_credentials_set(session->Session, GNUTLS_CRD_CERTIFICATE, _CertCredentials);
            }
            else if (gnutls_certificate_allocate_credentials(&_CertCredentials) == GNUTLS_E_SUCCESS)
            {
                if (certificate)
                {
                    gnutls_datum_t certificateData;
                    certificateData.data = certificate;
                    certificateData.size = certificateLength;
                    int format = GNUTLS_X509_FMT_PEM;
                    if (certificateFormat == AwaCertificateFormat_ASN1)
                        format = GNUTLS_X509_FMT_DER;
    //                if (client)
    //                    gnutls_certificate_set_x509_trust_mem(session->Credentials, &certificateData, format);
    //                else
                    gnutls_certificate_set_x509_key_mem(_CertCredentials, &certificateData, &certificateData, format);
                }
#if GNUTLS_VERSION_MAJOR >= 3
                    gnutls_certificate_set_verify_function(_CertCredentials, CertificateVerify);
                    //gnutls_certificate_set_retrieve_function(xcred, cert_callback);
                    //gnutls_session_set_verify_cert(session->Session, NULL, GNUTLS_VERIFY_DISABLE_CA_SIGN);
#else
                    gnutls_certificate_set_verify_flags(_CertCredentials, GNUTLS_VERIFY_DISABLE_CA_SIGN);
#endif
                gnutls_credentials_set(session->Session, GNUTLS_CRD_CERTIFICATE, _CertCredentials);
            }
        }
        else if (pskIdentity)
        {
            if (client)
            {
                gnutls_psk_client_credentials_t credentials;
                if (gnutls_psk_allocate_client_credentials(&credentials) == GNUTLS_E_SUCCESS)
                {
                    if (gnutls_psk_set_client_credentials(credentials, pskIdentity, &pskKey, GNUTLS_PSK_KEY_RAW) == GNUTLS_E_SUCCESS)
                    {
                        gnutls_credentials_set(session->Session, GNUTLS_CRD_PSK, credentials);
                        session->Credentials = credentials;
                        session->CredentialType = CredentialType_ClientPSK;
                    }
                    else
                    {
                        gnutls_psk_set_client_credentials_function(credentials, PSKClientCallBack);
                        session->Credentials = credentials;
                        session->CredentialType = CredentialType_ClientPSK;
                    }
                }
            }
            else
            {
                gnutls_psk_server_credentials_t credentials;
                if (gnutls_psk_allocate_server_credentials(&credentials) == GNUTLS_E_SUCCESS)
                {
                    gnutls_psk_set_server_credentials_function(credentials, PSKCallBack);
                    gnutls_credentials_set(session->Session, GNUTLS_CRD_PSK, credentials);
                    session->Credentials = credentials;
                    session->CredentialType = CredentialType_ServerPSK;
                }
            }
        }


        gnutls_priority_set(session->Session, _PriorityCache);
        if (!client)
        {
            gnutls_certificate_server_set_request(session->Session, GNUTLS_CERT_REQUEST); // GNUTLS_CERT_IGNORE  Don't require Client Cert
        }

#if GNUTLS_VERSION_MAJOR >= 3
        gnutls_handshake_set_timeout(session->Session, GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);
#endif
    }
}

static void FreeSession(DTLS_Session * session)
{
    if (session->Credentials)
    {
        if (session->CredentialType == CredentialType_ClientPSK)
            gnutls_psk_free_client_credentials(session->Credentials);
        else if (session->CredentialType == CredentialType_ServerPSK)
            gnutls_psk_free_server_credentials(session->Credentials);

    }
    gnutls_deinit(session->Session);
    memset(session,0, sizeof(DTLS_Session));

}

#if GNUTLS_VERSION_MAJOR >= 3
static int CertificateVerify(gnutls_session_t session)
{
    return 0;
}
#endif

static ssize_t DecryptCallBack(gnutls_transport_ptr_t context, void *recieveBuffer, size_t receiveBufferLegth)
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
        errno = EAGAIN;
        result = -1;
    }
    return result;
}

static ssize_t EncryptCallBack(gnutls_transport_ptr_t context, const void * sendBuffer,size_t sendBufferLength)
{
    ssize_t result;
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
        result = 0;
    }
    return result;
}

static int PSKClientCallBack(gnutls_session_t session, char **username, gnutls_datum_t * key)
{
    (void)session;
    int length = strlen(pskIdentity);
    *username = gnutls_malloc(length+1);
    strncpy(*username, pskIdentity, length);
    (*username)[length] = '\0';
    key->data = gnutls_malloc(pskKey.size);
    key->size = pskKey.size;
    memcpy(key->data, pskKey.data, pskKey.size);
    return 0;
}

static int PSKCallBack(gnutls_session_t session, const char *username, gnutls_datum_t * key)
{
    (void)session;
    (void)username;
    key->data = gnutls_malloc(pskKey.size);
    key->size = pskKey.size;
    memcpy(key->data, pskKey.data, pskKey.size);
    return 0;
}

#if GNUTLS_VERSION_MAJOR >= 3
static int ReceiveTimeout(gnutls_transport_ptr_t context, unsigned int ms)
{
//  fd_set rfds;
//  struct timeval tv;
//  int ret;
//  GnuTLS_Session *session = (GnuTLS_Session*)context;
//  int fd = session->ConnectionHandle;
//
//  FD_ZERO(&rfds);
//  FD_SET(fd, &rfds);
//
//  tv.tv_sec = ms/1000;
//  tv.tv_usec = (ms % 1000) * 1000;
//
//  ret = select(fd + 1, &rfds, NULL, NULL, &tv);
//  if (ret <= 0)
//      return ret;
//  return ret;
    return 1;
}
#endif

static ssize_t SSLSendCallBack(gnutls_transport_ptr_t context, const void * sendBuffer,size_t sendBufferLength)
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

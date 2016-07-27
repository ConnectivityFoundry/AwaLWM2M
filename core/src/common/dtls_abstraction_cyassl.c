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

#ifndef CYASSL_DTLS
#define CYASSL_DTLS
#endif

#ifndef WOLFSSL_DTLS
#define WOLFSSL_DTLS
#endif

#ifdef MICROCHIP_PIC32
#include <tcpip/berkeley_api.h>
#endif // MICROCHIP_PIC32

#include "cyassl/ssl.h"
#include "cyassl/version.h"
#include "cyassl/ctaocrypt/memory.h"

typedef struct
{
    NetworkAddress * NetworkAddress;
    CYASSL * Session;
    CYASSL_CTX * Context;
    bool SessionEstablished;
    bool Client;
    void * UserContext;
    uint8_t * Buffer;
    int BufferLength;
}DTLS_Session;

#ifndef MAX_DTLS_SESSIONS
    #define MAX_DTLS_SESSIONS 3
#endif

const char * DTLS_LibraryName = "CyaSSL";

static DTLS_Session sessions[MAX_DTLS_SESSIONS];

static uint8_t * certificate = NULL;
static int certificateLength = 0;
static AwaCertificateFormat certificateFormat;

static const char * pskIdentity = NULL;
static const uint8_t * pskKey = NULL;
static int pskKeyLength = 0;

static DTLS_NetworkSendCallback NetworkSend = NULL;

static DTLS_Session * AllocateSession(NetworkAddress * address, bool client, void * context);
static DTLS_Session * GetSession(NetworkAddress * address);
static void FreeSession(DTLS_Session * session);
static void SetupNewSession(int index, NetworkAddress * networkAddress, bool client);
static int DecryptCallBack(CYASSL *sslSessioon, char *recieveBuffer, int receiveBufferLegth, void *vp);
static int EncryptCallBack(CYASSL *sslSessioon, char *sendBuffer, int sendBufferLength, void *vp);
static unsigned int PSKCallBack(CYASSL *sslSession, const char* hint, char* identity, unsigned int id_max_len, unsigned char* key, unsigned int key_max_len);
static unsigned int ServerPSKCallBack(WOLFSSL *sslSessioon, const char* identity, unsigned char* key, unsigned int key_max_len);
static int SSLSendCallBack(CYASSL *sslSessioon, char *sendBuffer, int sendBufferLength, void *vp);


#define CERTCIPHERSUITES "ECDHE-ECDSA-AES128-CCM-8:ECDHE-ECDSA-AES128-SHA256"
#define PSKCIPHERSUITES "PSK-AES128-CCM-8:PSK-AES128-CBC-SHA256"

#ifdef MICROCHIP_PIC32
#ifndef wolfDTLSv1_2_client_method
WOLFSSL_METHOD* wolfDTLSv1_2_client_method(void);
#endif
#endif

#ifdef XMALLOC_USER
void *XMALLOC(size_t n, void* heap, int type)
{
    (void)heap;
    (void)type;
    return malloc(n);
}

void *XREALLOC(void *p, size_t n, void* heap, int type)
{
    (void)heap;
    (void)type;
    return realloc(p,n);
}

void XFREE(void *p, void* heap, int type)
{
    (void)heap;
    (void)type;
    free(p);
}
#endif

#ifdef DEBUG_WOLFSSL
void WOLFSSL_ENTER(const char* msg)
{
    Lwm2m_Info("%s\n", msg);
}

void WOLFSSL_LEAVE(const char* msg, int ret)
{
    Lwm2m_Info("%s %d\n", msg, ret);
}

void WOLFSSL_ERROR(int error)
{
    Lwm2m_Info("%d\n", error);
}

void WOLFSSL_MSG(const char* msg)
{
    Lwm2m_Info("%s\n", msg);
}

void WOLFSSL_BUFFER(byte* buffer, word32 length)
{

}
#endif


void DTLS_Init(void)
{
    memset(sessions,0,sizeof(DTLS_Session) * MAX_DTLS_SESSIONS);
    CyaSSL_Init();
#ifdef DEBUG_WOLFSSL
    CyaSSL_Debugging_ON();
#endif
    //CYASSL_API int  CyaSSL_dtls(CYASSL* ssl);
    //CyaSSL_SetAllocators(Flow_MemAlloc, Flow_MemSafeFree, Flow_MemRealloc);
}

void DTLS_Shutdown(void)
{
    int index;
    for (index = 0;index < MAX_DTLS_SESSIONS; index++)
    {
        if (sessions[index].Context)
        {
            FreeSession(&sessions[index]);
        }
    }
    CyaSSL_Cleanup();
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
        pskKey = key;
        pskKeyLength = keyLength;
    }
}


bool DTLS_Decrypt(NetworkAddress * sourceAddress, uint8_t * encrypted, int encryptedLength, uint8_t * decryptBuffer, int decryptBufferLength, int * decryptedLength, void *context)
{
    (void)context;
    bool result = false;
    DTLS_Session * session = GetSession(sourceAddress);
    if (!session)
    {
        session = AllocateSession(sourceAddress, false, context);
    }
    if (session)
    {
        session->Buffer = encrypted;
        session->BufferLength = encryptedLength;
        if (session->SessionEstablished)
        {
            *decryptedLength = CyaSSL_read(session->Session, decryptBuffer, decryptBufferLength);
            result = (*decryptedLength > 0);
        }
        else
        {
            *decryptedLength = 0;
            if (session->Client)
                session->SessionEstablished = (CyaSSL_connect(session->Session) == SSL_SUCCESS);
            else
            {
                int acceptResult = CyaSSL_accept(session->Session);
                session->SessionEstablished = (acceptResult == SSL_SUCCESS);
            }
            if (session->SessionEstablished)
                Lwm2m_Info("DTLS session established\n");
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
            CyaSSL_SetIOSend(session->Context, EncryptCallBack);
            session->Buffer = encryptedBuffer;
            session->BufferLength = encryptedBufferLength;
            int written = CyaSSL_write(session->Session, plainText, plainTextLength);
            if (written >= 0)
            {
                *encryptedLength = encryptedBufferLength - session->BufferLength;
                result = (*encryptedLength > 0);
            }
        }
        else
        {
            session->UserContext = context;
            CyaSSL_SetIOSend(session->Context, SSLSendCallBack);
            if (session->Client)
            {
                session->SessionEstablished = (CyaSSL_connect(session->Session) == SSL_SUCCESS);
            }
            else
                session->SessionEstablished = (CyaSSL_accept(session->Session) == SSL_SUCCESS);
            if (session->SessionEstablished)
                Lwm2m_Info("DTLS session established\n");
        }
    }
    else
    {
        session = AllocateSession(destAddress, true, context);
        if (session)
        {
            session->SessionEstablished = (CyaSSL_connect(session->Session) == SSL_SUCCESS);
        }
    }
    return result;
}

static DTLS_Session * AllocateSession(NetworkAddress * address, bool client, void * context)
{
    DTLS_Session * result = NULL;
    int index;
    for (index = 0;index < MAX_DTLS_SESSIONS; index++)
    {
        if (!sessions[index].Context)
        {
            SetupNewSession(index, address, client);
            sessions[index].UserContext = context;
            CyaSSL_SetIOSend(sessions[index].Context, SSLSendCallBack);
            result = &sessions[index];
            break;
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

static void FreeSession(DTLS_Session * session)
{
    if (session)
    {
        if (session->Session)
        {
            CyaSSL_shutdown(session->Session);
            CyaSSL_free(session->Session);
        }
        if (session->Context)
        {
            CyaSSL_CTX_free(session->Context);
        }
        memset(session,0, sizeof(DTLS_Session));
    }
}


static void SetupNewSession(int index, NetworkAddress * networkAddress, bool client)
{
    DTLS_Session * session = &sessions[index];
    session->NetworkAddress = networkAddress;
    session->Client = client;
    if (client)
        session->Context =  CyaSSL_CTX_new(CyaDTLSv1_2_client_method());
    else
        session->Context =  CyaSSL_CTX_new(CyaDTLSv1_2_server_method());
    if (session->Context)
    {

        if (certificate)
        {
            int format = SSL_FILETYPE_PEM;
            if (certificateFormat == AwaCertificateFormat_ASN1)
                format = SSL_FILETYPE_ASN1 ;

            uint8_t * privateKey = certificate;
            int privateKeyLength = certificateLength;

            uint8_t * certificatePart = certificate;
            int certificatePartLength = certificateLength;
            if (format == SSL_FILETYPE_PEM)
            {
                char * foundPosition = strstr(certificate ,"-----BEGIN CERTIFICATE-----");
                if (foundPosition)
                {
                    char * endPosition = strstr(foundPosition, "-----END CERTIFICATE-----");
                    if (endPosition)
                    {
                        certificatePart = foundPosition;
                        certificatePartLength = (endPosition-foundPosition) + 25;
                    }
                }
            }

            CyaSSL_CTX_use_certificate_buffer(session->Context, certificatePart, certificatePartLength, format);
            CyaSSL_CTX_use_PrivateKey_buffer(session->Context, privateKey, privateKeyLength, format);
        }
        if (pskIdentity)
        {
            if (client)
                CyaSSL_CTX_set_psk_client_callback(session->Context, PSKCallBack);
            else
            {
                CyaSSL_CTX_set_psk_server_callback(session->Context, ServerPSKCallBack);
                CyaSSL_CTX_use_psk_identity_hint(session->Context, pskIdentity);
            }
        }
        if (certificate &&  pskIdentity)
            CyaSSL_CTX_set_cipher_list(session->Context, CERTCIPHERSUITES ":" PSKCIPHERSUITES );
        else if (certificate)
            CyaSSL_CTX_set_cipher_list(session->Context, CERTCIPHERSUITES);
        else if (pskIdentity)
            CyaSSL_CTX_set_cipher_list(session->Context, PSKCIPHERSUITES);
//        if (caCertificate)
//        {
//            CyaSSL_CTX_load_verify_buffer(session->Context, caCertificate, caCertificateSize, SSL_FILETYPE_PEM);
//            CyaSSL_CTX_set_verify(session->Context, SSL_VERIFY_PEER, NULL);
//        }
//        else
            CyaSSL_CTX_set_verify(session->Context, SSL_VERIFY_NONE, NULL);
        session->Session = CyaSSL_new(session->Context);
        if (session->Session)
        {
            CyaSSL_dtls_set_peer(session->Session, networkAddress, sizeof(struct sockaddr_storage));
            CyaSSL_set_fd(session->Session, index);
            CyaSSL_set_using_nonblock(session->Session, 1);
            CyaSSL_SetIORecv(session->Context, DecryptCallBack);
        }
    }
}


static int DecryptCallBack(CYASSL *sslSessioon, char *recieveBuffer, int receiveBufferLegth, void *vp)
{
    int result;
    int index = CyaSSL_get_fd(sslSessioon);
    DTLS_Session * session = &sessions[index];
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
        result = CYASSL_CBIO_ERR_WANT_READ;
    }
    return result;
}

static int EncryptCallBack(CYASSL *sslSessioon, char *sendBuffer, int sendBufferLength, void *vp)
{
    int result;
    int index = CyaSSL_get_fd(sslSessioon);
    DTLS_Session * session = &sessions[index];
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
        result = CYASSL_CBIO_ERR_WANT_WRITE;
    }
    return result;
}

static unsigned int ServerPSKCallBack(WOLFSSL *sslSessioon, const char* identity, unsigned char* key, unsigned int key_max_len)
{
    memcpy(key, pskKey, pskKeyLength);
    return pskKeyLength;
}

static unsigned int PSKCallBack(CYASSL *sslSession, const char* hint, char* identity, unsigned int id_max_len, unsigned char* key, unsigned int key_max_len)
{
    (void)sslSession;
    (void)hint;
    (void)key_max_len;
    strncpy(identity, pskIdentity, id_max_len);
    memcpy(key, pskKey, pskKeyLength);
    return pskKeyLength;
}

static int SSLSendCallBack(CYASSL *sslSessioon, char *sendBuffer, int sendBufferLength, void *vp)
{
    int result;
    int index = CyaSSL_get_fd(sslSessioon);
    DTLS_Session * session = &sessions[index];
    if (NetworkSend)
    {
        NetworkTransmissionError error = NetworkSend(session->NetworkAddress, sendBuffer, sendBufferLength, session->UserContext);
        switch(error)
        {
            case NetworkTransmissionError_None:
                result = sendBufferLength;
                break;
            case NetworkTransmissionError_RecieveBufferEmpty:
                result = CYASSL_CBIO_ERR_WANT_READ;
                break;
            case NetworkTransmissionError_TransmitBufferFull:
                result = CYASSL_CBIO_ERR_WANT_WRITE;
                break;
            case NetworkTransmissionError_ConnectionClosed:
                result = CYASSL_CBIO_ERR_CONN_CLOSE;
                break;
            case NetworkTransmissionError_ConnectionReset:
                result = CYASSL_CBIO_ERR_CONN_RST;
                break;
            default:
                result = 0;
                break;
        }
    }
    else
    {
        result = CYASSL_CBIO_ERR_WANT_WRITE;
    }
    return result;
}

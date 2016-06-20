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

#include "cyassl/ssl.h"
#include "cyassl/version.h"
#include "cyassl/ctaocrypt/memory.h"

typedef struct
{
    NetworkAddress * NetworkAddress;
    CYASSL *Session;
    CYASSL_CTX *Context;
    bool SessionEstablished;
    void *UserContext;
    uint8_t * Buffer;
    int BufferLength;
}DTLS_Session;

#ifndef MAX_DTLS_SESSIONS
    #define MAX_DTLS_SESSIONS 3
#endif

DTLS_Session sessions[MAX_DTLS_SESSIONS];

uint8_t * certificate = NULL;
int certificateLength = 0;
CertificateFormat certificateFormat;

const char * pskIdentity = NULL;
uint8_t * pskKey;
int pskKeyLength;

DTLS_NetworkSendCallback NetworkSend = NULL;

static DTLS_Session * GetSession(NetworkAddress * address);
static void SetupNewSession(int index, NetworkAddress * networkAddress);
static int DecryptCallBack(CYASSL *sslSessioon, char *recieveBuffer, int receiveBufferLegth, void *vp);
static int EncryptCallBack(CYASSL *sslSessioon, char *sendBuffer, int sendBufferLength, void *vp);
static unsigned int PSKCallBack(CYASSL* sslSession, const char* hint, char* identity, unsigned int id_max_len, unsigned char* key, unsigned int key_max_len);
static int SSLSendCallBack(CYASSL *sslSessioon, char *sendBuffer, int sendBufferLength, void *vp);


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

void DTLS_Init(void)
{
    memset(sessions,0,sizeof(DTLS_Session) * MAX_DTLS_SESSIONS);
    CyaSSL_Init();
    //CYASSL_API int  CyaSSL_dtls(CYASSL* ssl);
    //CyaSSL_SetAllocators(Flow_MemAlloc, Flow_MemSafeFree, Flow_MemRealloc);
}

void DTLS_Shutdown(void)
{
    CyaSSL_Cleanup();
}

void DTLS_SetCertificate(const uint8_t * cert, int certLength, CertificateFormat format)
{
    certificate = (uint8_t *)cert;
    certificateLength = certLength;
    certificateFormat = format;
}

void DTLS_SetNetworkSendCallback(DTLS_NetworkSendCallback sendCallback)
{
    NetworkSend = sendCallback;
}

void DTLS_SetPSK(const char * identity, uint8_t * key, int keyLength)
{
    pskIdentity = identity;
    pskKey = key;
    pskKeyLength = keyLength;
}


bool DTLS_Decrypt(NetworkAddress * sourceAddress, uint8_t * encrypted, int encryptedLength, uint8_t * decryptBuffer, int decryptBufferLength, int * decryptedLength)
{
    bool result = false;
    DTLS_Session * session = GetSession(sourceAddress);
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
            session->SessionEstablished = (CyaSSL_connect(session->Session) == SSL_SUCCESS);
            if (session->SessionEstablished)
                Lwm2m_Info("Session established");
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
            session->SessionEstablished = (CyaSSL_connect(session->Session) == SSL_SUCCESS);
            if (session->SessionEstablished)
                Lwm2m_Info("Session established");
        }
    }
    else
    {
        int index;
        for (index = 0;index < MAX_DTLS_SESSIONS; index++)
        {
            if (!sessions[index].Session)
            {
                SetupNewSession(index, destAddress);
                sessions[index].UserContext = context;
                CyaSSL_SetIOSend(sessions[index].Context, SSLSendCallBack);
                sessions[index].SessionEstablished = (CyaSSL_connect(sessions[index].Session) == SSL_SUCCESS);
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

static void SetupNewSession(int index, NetworkAddress * networkAddress)
{
    DTLS_Session * session = &sessions[index];
    session->NetworkAddress = networkAddress;
    session->Context =  CyaSSL_CTX_new(CyaDTLSv1_2_client_method());
    if (session->Context)
    {
        if (certificate)
        {
            int format = SSL_FILETYPE_PEM;
            if (certificateFormat == CertificateFormat_ASN1)
                format = SSL_FILETYPE_ASN1 ;
            CyaSSL_CTX_use_certificate_buffer(session->Context, certificate, certificateLength, format);
            CyaSSL_CTX_use_PrivateKey_buffer(session->Context, certificate, certificateLength, format);
        }
        else if (pskIdentity)
        {
            CyaSSL_CTX_set_psk_client_callback(session->Context, PSKCallBack);
        }
//        if (controlBlock->TLSCertificateData)
//        {
//            CyaSSL_CTX_load_verify_buffer(session->Context, controlBlock->TLSCertificateData, controlBlock->TLSCertificateSize, SSL_FILETYPE_PEM);
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

static unsigned int PSKCallBack(CYASSL* sslSession, const char* hint, char* identity, unsigned int id_max_len, unsigned char* key, unsigned int key_max_len)
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

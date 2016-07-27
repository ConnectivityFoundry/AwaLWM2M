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

#ifndef DTLSv12
#define DTLSv12
#endif

// TinyDTLS does not support full Certificate 8-(
//#ifndef DTLS_ECC
//#define DTLS_ECC
//#endif

#ifndef DTLS_PSK
#define DTLS_PSK
#endif

#include <dtls.h>

#define TINY_DTLS_SUCCESS (0)

typedef enum
{
    CredentialType_NotSet,
    CredentialType_ClientPSK,
    CredentialType_ServerPSK
}CredentialType;


typedef struct
{
    NetworkAddress * NetworkAddress;
    session_t Session;
    dtls_context_t * Context;
    dtls_handler_t Callbacks;

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

const char * DTLS_LibraryName = "TinyDTLS";

static DTLS_Session sessions[MAX_DTLS_SESSIONS];

static uint8_t * certificate = NULL;
static int certificateLength = 0;
static AwaCertificateFormat certificateFormat;

static const char * pskIdentity = NULL;
static const uint8_t * pskKey = NULL;
static int pskKeyLength = 0;

static DTLS_NetworkSendCallback NetworkSend = NULL;


static DTLS_Session * AllocateSession(NetworkAddress * address, bool client, void * context);
static int DummySendCallBack(struct dtls_context_t *context, session_t *session, uint8 * sendBuffer, size_t sendBufferLength);
static DTLS_Session * GetSession(NetworkAddress * address);
static void SetupNewSession(int index, NetworkAddress * networkAddress, bool client);
static void FreeSession(DTLS_Session * session);
#ifdef DTLS_ECC
static int CertificateVerify(struct dtls_context_t *ctx, const session_t *session, const unsigned char *other_pub_x, const unsigned char *other_pub_y, size_t key_size);
#endif
static int DecryptCallBack(struct dtls_context_t *context, session_t *session, uint8 *recieveBuffer, size_t receiveBufferLegth);
static int EncryptCallBack(struct dtls_context_t *context, session_t *session, uint8 * sendBuffer, size_t sendBufferLength);
static int EventCallBack(struct dtls_context_t *context, session_t *session, dtls_alert_level_t level, unsigned short code);
static int PSKCallBack(struct dtls_context_t *ctx, const session_t *session, dtls_credentials_type_t type, const unsigned char *id,
        size_t id_len, unsigned char *result, size_t result_length);
static int SSLSendCallBack(struct dtls_context_t *context, session_t *session, uint8 * sendBuffer, size_t sendBufferLength);


#ifdef WITH_CONTIKI
static dtls_context_t * dtlsContext;
#endif

void DTLS_Init(void)
{
    memset(sessions,0,sizeof(DTLS_Session) * MAX_DTLS_SESSIONS);
    dtls_init();
#ifdef WITH_CONTIKI
    dtlsContext  = dtls_new_context(NULL);
#endif
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
#ifdef WITH_CONTIKI
    dtls_free_context(dtlsContext);
#endif
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
    bool result = false;
    bool tryAgain = true;
    DTLS_Session * session = GetSession(sourceAddress);
    if (!session)
    {
        session = AllocateSession(sourceAddress, false, context);
    }
    while (session && tryAgain)
    {
        tryAgain = false;
        session->Buffer = decryptBuffer;
        session->BufferLength = decryptBufferLength;
        bool hadSessionEstablished = session->SessionEstablished;
        dtls_set_app_data(session->Context, session);
        if (dtls_handle_message(session->Context, &session->Session, encrypted, encryptedLength) == TINY_DTLS_SUCCESS)
        {
            *decryptedLength = decryptBufferLength - session->BufferLength;
            if (session->SessionEstablished)
            {
                result = (*decryptedLength > 0);
            }
            else
            {
                if (hadSessionEstablished)
                    tryAgain = true;
                dtls_peer_t * peer = dtls_get_peer(session->Context, &session->Session);
                if (peer)
                {
                    session->SessionEstablished = (dtls_peer_state(peer) == DTLS_STATE_CONNECTED);
                }
            }
        }
        else
        {
            *decryptedLength = 0;
            if (session->SessionEstablished)
            {
                FreeSession(session);
                session = AllocateSession(sourceAddress, false, context);
                tryAgain = true;
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
            session->Callbacks.write = EncryptCallBack;
            session->Buffer = encryptedBuffer;
            session->BufferLength = encryptedBufferLength;
            dtls_set_app_data(session->Context, session);
            int written = dtls_write(session->Context, &session->Session, plainText, plainTextLength);
            if (written >= 0)
            {
                *encryptedLength = encryptedBufferLength - session->BufferLength;
                result = (*encryptedLength > 0);
            }
        }
        else
        {
            session->UserContext = context;
            session->Callbacks.write = SSLSendCallBack;
            dtls_peer_t * peer = dtls_get_peer(session->Context, &session->Session);
            if (!peer)
            {
                dtls_set_app_data(session->Context, session);
                dtls_connect(session->Context, &session->Session);
            }
        }
    }
    else
    {
        session = AllocateSession(destAddress, true, context);
        if (session)
        {
            dtls_peer_t * peer = dtls_get_peer(session->Context, &session->Session);
            if (!peer)
            {
                dtls_set_app_data(session->Context, session);
                dtls_connect(session->Context, &session->Session);
            }
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
            sessions[index].Callbacks.write = SSLSendCallBack;
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

static void SetupNewSession(int index, NetworkAddress * networkAddress, bool client)
{
    DTLS_Session * session = &sessions[index];
    if (!client)
        session->Callbacks.event = EventCallBack;
    session->Callbacks.read = DecryptCallBack;
    session->Callbacks.write = SSLSendCallBack;
#ifdef DTLS_PSK
    session->Callbacks.get_psk_info = PSKCallBack;
#endif
#ifdef DTLS_ECC
    session->Callbacks.get_ecdsa_key = GetCertificate;
    session->Callbacks.verify_ecdsa_key = CertificateVerify;
#endif
    session->NetworkAddress = networkAddress;
#ifdef WITH_CONTIKI
    session->Context = dtlsContext;
#else
    session->Context = dtls_new_context(session);
#endif
    if (session->Context)
    {
        dtls_set_handler(session->Context, &session->Callbacks);
//        if (!client)
//        {
//            //dtls_peer_t *peer =
//            dtls_new_peer(&session->Session);
//        }
    }
}

static void FreeSession(DTLS_Session * session)
{
    if (session->Context)
    {
        session->Callbacks.write = DummySendCallBack;
        dtls_peer_t * peer = dtls_get_peer(session->Context, &session->Session);
        if (peer)
        {
            dtls_reset_peer(session->Context, peer);
        }
#ifndef WITH_CONTIKI
        dtls_free_context(session->Context);
#endif
    }
    memset(session,0, sizeof(DTLS_Session));
}

#if GNUTLS_VERSION_MAJOR >= 3
static int CertificateVerify(gnutls_session_t session)
{
    return 0;
}
#endif

static int DecryptCallBack(struct dtls_context_t *context, session_t *session, uint8 *recieveBuffer, size_t receiveBufferLegth)
{
    int result;
    DTLS_Session * dtlsSession = (DTLS_Session *)dtls_get_app_data(context);
    if (dtlsSession && dtlsSession->BufferLength > 0)
    {
        if (receiveBufferLegth < dtlsSession->BufferLength)
        {
            result =  receiveBufferLegth;
        }
        else
        {
            result =  dtlsSession->BufferLength;
        }
        memcpy(dtlsSession->Buffer, recieveBuffer, result);
        dtlsSession->BufferLength = dtlsSession->BufferLength - result;
        dtlsSession->Buffer += result;
    }
    else
    {
        result = -1;
    }
    return result;
}
static int EncryptCallBack(struct dtls_context_t *context, session_t *session, uint8 * sendBuffer, size_t sendBufferLength)
{
    int result;
    DTLS_Session * dtlsSession = (DTLS_Session *)dtls_get_app_data(context);
    if (dtlsSession && dtlsSession->BufferLength > 0)
    {
        if (sendBufferLength < dtlsSession->BufferLength)
        {
            result =  sendBufferLength;
        }
        else
        {
            result =  dtlsSession->BufferLength;
        }
        memcpy(dtlsSession->Buffer, sendBuffer, result);
        dtlsSession->BufferLength = dtlsSession->BufferLength - result;
        dtlsSession->Buffer += result;
    }
    else
    {
        result = 0;
    }
    return result;
}

static int EventCallBack(struct dtls_context_t *context, session_t *session, dtls_alert_level_t level, unsigned short code)
{
    if (code == DTLS_EVENT_CONNECTED)
    {
        DTLS_Session * dtlsSession = (DTLS_Session *)dtls_get_app_data(context);
        if (dtlsSession)
        {
            if (dtlsSession->SessionEstablished)
            {
                dtlsSession->Callbacks.write = NULL;
                dtls_peer_t * peer = dtls_get_peer(context, session);
                if (peer)
                {
                    dtls_reset_peer(context, peer);
                }
                dtlsSession->Callbacks.write = SSLSendCallBack;
            }
            dtlsSession->SessionEstablished = false;
        }
    }
    return 0;
}

static int PSKCallBack(struct dtls_context_t *ctx, const session_t *session, dtls_credentials_type_t type, const unsigned char *id,
        size_t id_len, unsigned char *result, size_t result_length)
{
    switch (type)
    {
    case DTLS_PSK_HINT:
    case DTLS_PSK_IDENTITY:
        if (id_len)
        {
            Lwm2m_Debug("got psk_identity_hint: '%.*s'\n", (int)id_len, id);
        }
        int pskIdentityLength = strlen(pskIdentity);
        if (result_length < pskIdentityLength)
        {
            Lwm2m_Warning("cannot set psk_identity -- buffer too small\n");
            return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
        }

        memcpy(result, pskIdentity, pskIdentityLength);
        return pskIdentityLength;
    case DTLS_PSK_KEY:
        pskIdentityLength = strlen(pskIdentity);
        if (id_len != pskIdentityLength || memcmp(pskIdentity, id, id_len) != 0)
        {
            Lwm2m_Warning("PSK for unknown id requested, exiting\n");
            return dtls_alert_fatal_create(DTLS_ALERT_ILLEGAL_PARAMETER);
        }
        else if (result_length < pskKeyLength)
        {
            Lwm2m_Warning("cannot set psk -- buffer too small\n");
            return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
        }

        memcpy(result, pskKey, pskKeyLength);
        return pskKeyLength;
    default:
        Lwm2m_Warning("unsupported request type: %d\n", type);
    }

    return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
}


static int SSLSendCallBack(struct dtls_context_t *context, session_t *session, uint8 * sendBuffer, size_t sendBufferLength)
{
    int result;
    DTLS_Session * dtlsSession = (DTLS_Session *)dtls_get_app_data(context);
    if (dtlsSession && NetworkSend)
    {
        NetworkTransmissionError error = NetworkSend(dtlsSession->NetworkAddress, sendBuffer, sendBufferLength, dtlsSession->UserContext);
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

#ifdef DTLS_ECC
static int CertificateVerify(struct dtls_context_t *ctx, const session_t *session, const unsigned char *other_pub_x, const unsigned char *other_pub_y, size_t key_size)
{
    return TINY_DTLS_SUCCESS;
}
#endif


static int DummySendCallBack(struct dtls_context_t *context, session_t *session, uint8 * sendBuffer, size_t sendBufferLength)
{
    return 0;
}

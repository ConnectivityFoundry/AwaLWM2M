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

#include <string.h>
#include <stdlib.h>
#include "coap_abstraction.h"
#include "lwm2m_debug.h"
#include "network_abstraction.h"
#include "dtls_abstraction.h"

#include "er-resource.h"
#include "er-coap-engine.h"
#include "er-coap.h"

#ifndef MAX_COAP_PATH
#define MAX_COAP_PATH 64
#endif

typedef struct
{
    AddressType Address;
    char Path[MAX_COAP_PATH];
    TransactionCallback Callback;
    void * Context;
    bool TransactionUsed;
    coap_transaction_t * TransactionPtr;
} TransactionType;

#define COAP_OPTION_TO_RESPONSE_CODE(N) (((N >> 5) * 100) | (N & 0x1f))
#define COAP_RESPONSE_CODE(N) (((N)/100 << 5) | (N)%100)
#define PRINT6ADDR(addr) "[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]\n", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15]

static CoapInfo coapInfo;
static void * context = NULL;
static RequestHandler requestHandler = NULL;

const char * coap_LibraryName = "Erbium";

#ifndef MAX_COAP_TRANSACTIONS
#define MAX_COAP_TRANSACTIONS (2)
#endif

int CurrentTransactionIndex = 0;
TransactionType CurrentTransaction[MAX_COAP_TRANSACTIONS];

static NetworkSocket * networkSocket = NULL;
extern NetworkAddress * sourceAddress;

typedef enum
{
    ObserveState_None, ObserveState_Establish, ObserveState_Cancel
} ObserveState;

typedef struct
{
    NetworkAddress * Address;
    char Path[MAX_COAP_PATH];
    int Token;
    TransactionCallback Callback;
    void * Context;
} Observation;

#ifndef MAX_COAP_OBSERVATIONS
#define MAX_COAP_OBSERVATIONS (10)
#endif

Observation Observations[MAX_COAP_OBSERVATIONS];

static int coap_HandleRequest(void *packet, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static int addObserve(NetworkAddress * remoteAddress, char * path, TransactionCallback callback, void * context);
static int removeObserve(NetworkAddress * remoteAddress, char * path);

CoapInfo * coap_Init(const char * ipAddress, int port, bool secure, int logLevel)
{
    CoapInfo * result = NULL;
    memset(CurrentTransaction, 0, sizeof(CurrentTransaction));
    memset(Observations, 0, sizeof(Observations));
    coap_init_connection(port);
    coap_init_transactions();
    coap_set_service_callback(coap_HandleRequest);
    DTLS_Init();
    if (secure)
    	networkSocket = NetworkSocket_New(ipAddress, NetworkSocketType_UDP | NetworkSocketType_Secure, port);
    else
    	networkSocket = NetworkSocket_New(ipAddress, NetworkSocketType_UDP, port);
    if (networkSocket)
    {
        if (NetworkSocket_StartListening(networkSocket))
        {
            Lwm2m_Info("Bind port: %d\n", port);
            coapInfo.fd = NetworkSocket_GetFileDescriptor(networkSocket);
            result = &coapInfo;
        }
    }
    else
    {
        Lwm2m_Error("Failed to bind port: %d\n", port);
    }
    return result;
}


void coap_Reset(const char * uri)
{
    NetworkAddress * remoteAddress = NetworkAddress_New(uri, strlen(uri));
    if (remoteAddress)
    {
        DTLS_Reset(remoteAddress);
        NetworkAddress_Free(&remoteAddress);
    }
}

void coap_SetCertificate(const uint8_t * cert, int certLength, AwaCertificateFormat format)
{
	NetworkSocket_SetCertificate(networkSocket, cert, certLength, format);
}

void coap_SetPSK(const char * identity, const uint8_t * key, int keyLength)
{
	NetworkSocket_SetPSK(networkSocket, identity, key, keyLength);
}

void coap_SetLogLevel(int logLevel)
{
    // TODO - set log level for Erbium (replace PRINTFs)
}

int coap_WaitMessage(int timeout, int fd)
{
    coap_receive(networkSocket);
    coap_check_transactions();
    return timeout;
}

static int coap_HandleRequest(void *packet, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    int result = 1;
    const char *url = NULL;
    int urlLen = 0;
    const uint8_t * payload = NULL;
    int payloadLen = 0;
    int content = -1;

    coap_packet_t * const request = (coap_packet_t *) packet;

    CoapResponse coapResponse =
    { .responseContent = buffer, .responseContentLen = preferred_size, .responseCode = 400, };

    payloadLen = coap_get_payload(request, &payload);

    if ((urlLen = coap_get_header_uri_path(request, &url)))
    {
        char uriBuf[MAX_COAP_PATH] = { 0 };
        rest_resource_flags_t method = (rest_resource_flags_t) (1 << (((coap_packet_t *) packet)->code - 1)); //coap_get_rest_method(request);

        uriBuf[0] = '/';
        memcpy(&uriBuf[1], url, urlLen);

        char queryBuf[128] = "?";
        const char * query = NULL;

        int queryLength = coap_get_header_uri_query(request, &query);
        if (queryLength > 0)
            memcpy(&queryBuf[1], query, queryLength);

        queryBuf[queryLength+1] = '\0';

        CoapRequest coapRequest =
        { .ctxt = context, .addr =
        { 0 }, .path = uriBuf, .query = queryBuf, .token = request->token, .tokenLength = request->token_len, .requestContent = payload,
                .requestContentLen = payloadLen, };

        NetworkAddress_SetAddressType(sourceAddress, &coapRequest.addr);

        switch (method)
        {
        case METHOD_GET:

            coap_get_header_accept(request, &content);
            coapRequest.contentType = content;

            int32_t observe;

            if (!coap_get_header_observe(request, &observe))
                observe = -1;

            switch (observe)
            {
            case -1:
                Lwm2m_Debug("Coap GET for %s\n", uriBuf);
                coapRequest.type = COAP_GET_REQUEST;
                requestHandler(&coapRequest, &coapResponse);
                break;
            case 0:
                Lwm2m_Debug("Coap OBSERVE for %s\n", uriBuf);

                coapRequest.type = COAP_OBSERVE_REQUEST;
                requestHandler(&coapRequest, &coapResponse);
                coap_set_header_observe(response, 1);
                break;
            case 1:
                Lwm2m_Debug("Coap CANCEL OBSERVE for %s\n", uriBuf);

                coapRequest.type = COAP_CANCEL_OBSERVE_REQUEST;
                requestHandler(&coapRequest, &coapResponse);
                break;
            default:
                break;
            }
            coap_set_header_content_format(response, coapResponse.responseContentType); /* text/plain is the default, hence this option could be omitted. */
            break;

        case METHOD_POST:
            coap_get_header_content_format(request, &content);
            coapRequest.contentType = content;
            coapRequest.type = COAP_POST_REQUEST;
            Lwm2m_Debug("Coap POST for %s\n", uriBuf);
            requestHandler(&coapRequest, &coapResponse);
            if (coapResponse.responseContentLen > 0 && coapResponse.responseCode == 201)
            {
                coap_set_header_location_path(response, coapResponse.responseContent);
            }
            break;

        case METHOD_PUT:
            coap_get_header_content_format(request, &content);
            coapRequest.contentType = content;
            coapRequest.type = COAP_PUT_REQUEST;

            Lwm2m_Debug("Coap PUT for %s\n", uriBuf);
            requestHandler(&coapRequest, &coapResponse);
            break;

        case METHOD_DELETE:
            coapRequest.contentType = AwaContentType_None;
            coapRequest.type = COAP_DELETE_REQUEST;

            Lwm2m_Debug("Coap DELETE for %s\n", uriBuf);
            requestHandler(&coapRequest, &coapResponse);
            break;

        default:
            break;
        }

        Lwm2m_Debug("Coap Response code %d\n", coapResponse.responseCode);

        if (coapResponse.responseContentLen > 0 && coapResponse.responseCode == 205)
        {
            coap_set_payload(response, coapResponse.responseContent, coapResponse.responseContentLen);
        }
    }

    coap_set_status_code(response, COAP_RESPONSE_CODE(coapResponse.responseCode));
    return result;
}

bool coap_getPathQueryFromURI(const char * uri, char * path, char * query)
{
    bool result = false;
    char * pathStart = strchr(uri, '/');
    if (pathStart && pathStart[1] == '/')
    {
        pathStart += 2;
        pathStart = strchr(pathStart, '/');
        if (pathStart)
        {
            pathStart += 1;
            char * pathEnd = strchr(pathStart, '?');
            if (pathEnd == NULL )
            {
                strcpy(path, pathStart);
            }
            else
            {
                char * queryStart = pathEnd + 1;
                int length = pathEnd - pathStart;
                memcpy(path, pathStart, length);
                path[length] = '\0';
                strcpy(query, queryStart);
            }
            result = true;
        }
    }
    return result;
}

bool coap_ResolveAddressByURI(unsigned char * address, AddressType * addr)
{
    bool result = false;

    Lwm2m_Debug("resolve address from Uri: %s\n", address);
    NetworkAddress * networkAddress = NetworkAddress_New(address, strlen(address));
    if (networkAddress)
    {
        NetworkAddress_SetAddressType(networkAddress, addr);
        NetworkAddress_Free(&networkAddress);
        result = true;
    }
    return result;
}

void coap_CoapRequestCallback(void *callback_data, void *response)
{
    TransactionType * transaction = (TransactionType *) callback_data;
    coap_packet_t * coap_response = (coap_packet_t *) response;
    int ContentType = 0;
    const char *url = NULL;
    char * payload = NULL;
    char uriBuf[64] =
    { 0 };

    if (transaction != NULL)
    {
        if (transaction->Callback)
        {
            if (response != NULL )
            {
                int urlLen = 0;
                if ((urlLen = coap_get_header_location_path(response, &url)))
                {
                    uriBuf[0] = '/';
                    memcpy(&uriBuf[1], url, urlLen);
                }
                else
                {
                    uriBuf[0] = '/';
                    urlLen = strlen(transaction->Path);
                    memcpy(&uriBuf[1], transaction->Path, urlLen);
                }
                coap_get_header_content_format(response, &ContentType);
                int payloadLen = coap_get_payload(response, (const uint8_t **) &payload);

                transaction->Callback(transaction->Context, &transaction->Address, uriBuf, COAP_OPTION_TO_RESPONSE_CODE(coap_response->code),
                        ContentType, payload, payloadLen);
            }
            else
            {
                transaction->Callback(transaction->Context, NULL, NULL, 0, 0, NULL, 0);
            }
        }
        transaction->TransactionUsed = false;
    }
}

void coap_createCoapRequest(coap_method_t method, const char * uri, AwaContentType contentType, ObserveState observeState,
        const char * payload, int payloadLen, TransactionCallback callback, void * context)
{
    coap_packet_t request;
    char path[MAX_COAP_PATH] =
    { 0 };
    char query[128] =
    { 0 };
    coap_transaction_t *transaction;
    NetworkAddress * remoteAddress = NetworkAddress_New(uri, strlen(uri));

    if ((strcmp(DTLS_LibraryName, "None") == 0) && NetworkAddress_IsSecure(remoteAddress))
    {
        NetworkAddress_Free(&remoteAddress);
        Lwm2m_Error("Cannot send request to %s - not built with DTLS support\n\n", uri);
        return;
    }

    coap_getPathQueryFromURI(uri, path, query);

    Lwm2m_Info("Coap request: %s\n", uri);
    //Lwm2m_Debug("Coap request path: %s\n", path);
    //Lwm2m_Debug("Coap request query: %s\n", query);

    coap_init_message(&request, COAP_TYPE_CON, method, coap_get_mid());

    coap_set_header_uri_path(&request, path);
    if (strlen(query) > 0)
        coap_set_header_uri_query(&request, query);
    // TODO - REVIEW: Erbium must copy path/query from request - else mem out of scope

    if (contentType != AwaContentType_None)
    {
        if ((method == COAP_POST) || (method == COAP_PUT))
        {
            coap_set_header_content_format(&request, contentType);
            coap_set_payload(&request, payload, payloadLen);
        }
        else
        {
            coap_set_header_accept(&request, contentType);
        }
    }


    if (method == COAP_GET)
    {
        if (observeState == ObserveState_Establish)
        {
            coap_set_header_observe(&request, 0);
            int token = addObserve(remoteAddress, path, callback, context);
            if (token != 0)
                coap_set_token(&request, (const uint8_t *) &token, sizeof(token));

        }
        else if (observeState == ObserveState_Cancel)
        {
            coap_set_header_observe(&request, 1);
            int token = removeObserve(remoteAddress, path);
            if (token != 0)
                coap_set_token(&request, (const uint8_t *) &token, sizeof(token));
        }
    }

//    if (CurrentTransaction[CurrentTransactionIndex].TransactionUsed && CurrentTransaction[CurrentTransactionIndex].TransactionPtr)
//    {
//        Lwm2m_Warning("Canceled previous transaction [%d]: %p\n", CurrentTransactionIndex,
//                CurrentTransaction[CurrentTransactionIndex].TransactionPtr);
//        coap_clear_transaction(&CurrentTransaction[CurrentTransactionIndex].TransactionPtr);
//    }

    //if ((transaction = coap_new_transaction(request.mid, remote_ipaddr, uip_htons(remote_port))))
    if ((transaction = coap_new_transaction(networkSocket, request.mid, remoteAddress)))
    {
        transaction->callback = coap_CoapRequestCallback;
        memcpy(CurrentTransaction[CurrentTransactionIndex].Path, path, MAX_COAP_PATH);
        CurrentTransaction[CurrentTransactionIndex].Callback = callback;
        CurrentTransaction[CurrentTransactionIndex].Context = context;
        CurrentTransaction[CurrentTransactionIndex].TransactionUsed = true;
        CurrentTransaction[CurrentTransactionIndex].TransactionPtr = transaction;
        NetworkAddress_SetAddressType(remoteAddress, &CurrentTransaction[CurrentTransactionIndex].Address);

        transaction->callback_data = &CurrentTransaction[CurrentTransactionIndex];

        transaction->packet_len = coap_serialize_message(&request, transaction->packet);

        Lwm2m_Debug("Sending transaction [%d]: %p\n", CurrentTransactionIndex, CurrentTransaction[CurrentTransactionIndex].TransactionPtr);
        coap_send_transaction(transaction);

        CurrentTransactionIndex++;

        if (CurrentTransactionIndex >= MAX_COAP_TRANSACTIONS)
        {
            CurrentTransactionIndex = 0;
        }
    }
}

int coap_Destroy(void)
{
    Lwm2m_Info("Close port: \n");     //  TODO - remove
    if (networkSocket)
        NetworkSocket_Free(&networkSocket);
    // TODO - close any open sessions
//    coap_free_context(coapContext);
//    DestroyLists();
    return 0;
}

void coap_Process(void)
{
    coap_check_transactions();
}

void coap_HandleMessage(void)
{
    coap_receive(networkSocket);
}

void coap_GetRequest(void * context, const char * path, AwaContentType contentType, TransactionCallback callback)
{
    coap_createCoapRequest(COAP_GET, path, contentType, ObserveState_None, NULL, 0, callback, context);
}

void coap_PostRequest(void * context, const char * path, AwaContentType contentType, const char * payload, int payloadLen,
        TransactionCallback callback)
{
    coap_createCoapRequest(COAP_POST, path, contentType, ObserveState_None, payload, payloadLen, callback, context);
}

void coap_PutRequest(void * context, const char * path, AwaContentType contentType, const char * payload, int payloadLen,
        TransactionCallback callback)
{
    coap_createCoapRequest(COAP_PUT, path, contentType, ObserveState_None, payload, payloadLen, callback, context);
}

// This is a dummy function - Delete requests are not required on the constrained device and are only used by the LWM2M Server.
void coap_DeleteRequest(void * context, const char * path, TransactionCallback callback)
{
    coap_createCoapRequest(COAP_DELETE, path, AwaContentType_None, ObserveState_None, NULL, 0, callback, context);
}

// This is a dummy function - Observe requests are not required on the constrained device and are only used by the LWM2M Server.
void coap_Observe(void * context, const char * path, AwaContentType contentType, TransactionCallback callback,
        NotificationFreeCallback notificationFreeCallback)
{
    coap_createCoapRequest(COAP_GET, path, contentType, ObserveState_Establish, NULL, 0, callback, context);
}

// This is a dummy function - Cancel Observe Requests are not required on the constrained device and are only used by the LWM2M Server.
void coap_CancelObserve(void * context, const char * path, AwaContentType contentType, TransactionCallback callback)
{
    coap_createCoapRequest(COAP_GET, path, contentType, ObserveState_Cancel, NULL, 0, callback, context);
}

void coap_SendNotify(AddressType * addr, const char * path, const char * token, int tokenSize, AwaContentType contentType,
        const char * payload, int payloadLen, int sequence)
{
    // TODO - FIXME: if path is not full uri then map addr to Network address + append path(?)
    coap_packet_t notify;
    coap_transaction_t *transaction;
    NetworkAddress * remoteAddress = NetworkAddress_New(path, strlen(path));

    Lwm2m_Debug("Coap notify: %s\n", path);
    //Lwm2m_Debug("Coap IPv6 request address: " PRINT6ADDR(&addr->Addr));
    //Lwm2m_Debug("Coap request port: %d\n", addr->Port);

    coap_init_message(&notify, COAP_TYPE_NON, CONTENT_2_05, coap_get_mid());

    if (contentType != AwaContentType_None)
    {
        coap_set_header_content_format(&notify, contentType);
        coap_set_payload(&notify, payload, payloadLen);
    }

    coap_set_token(&notify, token, tokenSize);
    coap_set_header_observe(&notify, sequence);

    if ((transaction = coap_new_transaction(networkSocket, notify.mid, remoteAddress)))
    {
        transaction->packet_len = coap_serialize_message(&notify, transaction->packet);

        coap_send_transaction(transaction); // for NON confirmable messages this will call coap_clear_transaction();
    }
}

void coap_SetContext(void * ctxt)
{
    context = ctxt;
}

void coap_SetRequestHandler(RequestHandler handler)
{
    requestHandler = handler;
}

int coap_RegisterUri(const char * uri)
{
    // Do nothing
    return 0;
}

int coap_DeregisterUri(const char * path)
{
    // Do nothing
    return 0;
}

static int addObserve(NetworkAddress * remoteAddress, char * path, TransactionCallback callback, void * context)
{
    int result = 0;
    int index;
    Observation * observation = NULL;
    for (index = 0; index < MAX_COAP_OBSERVATIONS; index++)
    {
        if (Observations[index].Token == 0)
        {
            observation = &Observations[index];
            break;
        }
    }
    if (observation)
    {
        observation->Address = remoteAddress;
        int length = strlen(path);
        memcpy(observation->Path, path, length);
        observation->Path[length] = '\0';
        bool found;
        do
        {
            result = rand();
            if (result == 0)
                found = true;
            else
            {
                found = false;
                for (index = 0; index < MAX_COAP_OBSERVATIONS; index++)
                {
                    if (Observations[index].Token == result)
                    {
                        found = true;
                        break;
                    }
                }
            }
        } while (found);
        observation->Token = result;
        observation->Callback =  callback;
        observation->Context = context;
    }
    return result;
}

static int removeObserve(NetworkAddress * remoteAddress, char * path)
{
    int result = 0;
    int index;
    for (index = 0; index < MAX_COAP_OBSERVATIONS; index++)
    {
        if ((NetworkAddress_Compare(Observations[index].Address, sourceAddress) == 0) && (strcmp(Observations[index].Path,path) == 0))
        {
            result = Observations[index].Token;
            memset(&Observations[index],0, sizeof(Observation));
            break;
        }
    }
    return result;
}


void coap_handle_notification(NetworkAddress * sourceAddress, coap_packet_t * message)
{
    if (message->token_len == sizeof(int))
    {
        int token;
        memcpy(&token, message->token, sizeof(int));
        Observation * observation = NULL;
        int index;
        for (index = 0; index < MAX_COAP_OBSERVATIONS; index++)
        {
            if ((Observations[index].Token == token) && (NetworkAddress_Compare(Observations[index].Address, sourceAddress) == 0))
            {
                observation = &Observations[index];
                break;
            }
        }
        if (observation && observation->Callback)
        {
            AddressType address;
            int ContentType = 0;
            char * payload = NULL;

            NetworkAddress_SetAddressType(sourceAddress, &address);
            coap_get_header_content_format(message, &ContentType);
            int payloadLen = coap_get_payload(message, (const uint8_t **) &payload);

            observation->Callback(observation->Context, &address, observation->Path, COAP_OPTION_TO_RESPONSE_CODE(message->code),
                    ContentType, payload, payloadLen);
        }
    }

}


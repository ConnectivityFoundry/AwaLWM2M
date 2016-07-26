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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <inttypes.h>
#include <poll.h>


#include "coap_abstraction.h"
#include "lwm2m_util.h"
#include "lwm2m_list.h"
#include "lwm2m_debug.h"

#define WITH_POSIX 1
//#define  COAP_4_1_1  // build against libcoap 4.1.1 (last released version)

#include <coap.h>
#include <address.h>

#ifdef COAP_4_1_1
  #define COAP_OBSERVE_ESTABLISH 0
  #define COAP_OBSERVE_CANCEL 1
#endif

#define COAP_RESOURCE_CHECK_TIME           2
#define COAP_MESSAGE_OBSERVE          0x80
#define COAP_MESSAGE_CANCEL_OBSERVE   0x40
#define COAP_MESSAGE_NOTIFY           0x20
#define COAP_OPTION_TO_RESPONSE_CODE(N) (((N >> 5) * 100) | (N & 0x1f))

typedef struct
{
    struct ListHead List;
    TransactionCallback Callback;
    NotificationFreeCallback NotificationFreeCallback;
    coap_tid_t TransactionID;
    AddressType Address;
    void * Context;
    char * Path;
} TransactionType;

typedef struct
{
    struct ListHead List;
    TransactionCallback Callback;
    char Token[8];
    int TokenLength;
    char * Path;
    void * Context;
    NotificationFreeCallback FreeCallback;
    AddressType * Address;
} NotificationHandler;

const char * coap_LibraryName = "libcoap";

static struct ListHead transactionCallbackList;
static struct ListHead notifyCallbackList;
static coap_context_t * coapContext = NULL;
static void * context = NULL;
static CoapInfo coapInfo;
static RequestHandler requestHandler = NULL;


void coap_Reset(const char * uri)
{
}

void coap_SetContext(void * ctxt)
{
    context = ctxt;
}

void coap_SetRequestHandler(RequestHandler handler)
{
    requestHandler = handler;
}

int coap_WaitMessage(int timeout, int fd)
{
	// TODO - review/needs fixing?: moved from lwm2m_static/AwaStaticClient_Process()
    int result;
    struct pollfd fds[1];
    int nfds = 1;

    fds[0].fd = fd;
    fds[0].events = POLLIN;

    result = poll(fds, nfds, timeout);
    if (result < 0)
    {
        if (errno == EINTR)
        {
            result = timeout;
        }
        else
        {
            perror("poll:");
        }
    }
    else if (result > 0)
    {
        if (fds[0].revents == POLLIN)
        {
            coap_HandleMessage();
        }
        result = timeout;
    }

    if (result == timeout)
    {
        coap_Process();
    }
    return result;
}

bool coap_ResolveAddressByURI(unsigned char * address, AddressType * addr)
{
    bool result = false;
    coap_uri_t uri;
    coap_split_uri(address, strlen(address), &uri);

    if (Lwm2mCore_ResolveAddressByName(uri.host.s, uri.host.length, addr))
    {
#ifndef CONTIKI
        addr->Addr.Sin.sin_port = uri.port;
#else
        addr->Port = uri.port;
#endif
        result = true;
    }
    return result;
}

static void coap_CoapAddressTypeToAddressType(coap_address_t * coapAddress, AddressType * addressType)
{
    addressType->Size = coapAddress->size;
    memcpy(&addressType->Addr.St, &coapAddress->addr.st, sizeof(addressType->Addr.St));
}

/* Notification handler, used by the server to map a callback to a token in a notification packet
 * this is not required on the client, as the client can simply use the object store
 */
static NotificationHandler * lookup_NotificationHandler(char * token, int tokenLength)
{
    struct ListHead * i;
    ListForEach(i, &notifyCallbackList)
    {
        NotificationHandler * notify = ListEntry(i, NotificationHandler, List);
        if ((notify->TokenLength == tokenLength) && (memcmp(notify->Token, token, tokenLength) == 0))
        {
            return notify;
        }
    }
    return NULL;
}

static int create_NotificationHandler(char * token, int tokenLength, void * context, char * path, TransactionCallback callback,
                                      NotificationFreeCallback freeCallback, AddressType * address)
{
    int result = -1;
    NotificationHandler * notify = malloc(sizeof(NotificationHandler));
    if (notify != NULL)
    {
        notify->TokenLength = tokenLength;
        memcpy(notify->Token, token, tokenLength);
        notify->Context = context;
        notify->Callback = callback;
        notify->FreeCallback = freeCallback;
        notify->Path = strdup(path);
        notify->Address = address;

        ListAdd(&notify->List, &notifyCallbackList);
        result = 0;
    }
    return result;
}

static int remove_NotificationHandler(NotificationHandler * notify)
{
    ListRemove(&notify->List);

    free(notify->Path);
    free(notify);
    return 0;
}

static TransactionType * lookup_Transaction(coap_tid_t transactionID)
{
    struct ListHead * i;
    ListForEach(i, &transactionCallbackList)
    {
        TransactionType * transaction = ListEntry(i, TransactionType, List);
        if (transaction->TransactionID == transactionID)
        {
            return transaction;
        }
    }
    return NULL;
}

static int create_Transaction(coap_tid_t transactionID, coap_address_t * address, const char * path, void * context,
                              TransactionCallback transactionCallback, NotificationFreeCallback notificationFreeCallback)
{
    if (path == NULL)
    {
        Lwm2m_Error("path is NULL\n");
        return -1;
    }

    TransactionType * transaction = lookup_Transaction(transactionID);
    if (transaction)
    {
        // already exists
        return -1;
    }

    transaction = malloc(sizeof(TransactionType));
    if (!transaction)
    {
        return -1;
    }

    memset(transaction, 0, sizeof(*transaction));
    transaction->TransactionID = transactionID;
    transaction->Context = context;
    transaction->Callback = transactionCallback;
    transaction->NotificationFreeCallback = notificationFreeCallback;
    coap_CoapAddressTypeToAddressType(address, &transaction->Address);
    transaction->Path = strdup(path);

    ListAdd(&transaction->List, &transactionCallbackList);

    return 0;
}

static int remove_Transaction(TransactionType * transaction)
{
    ListRemove(&transaction->List);
    free(transaction->Path);
    free(transaction);
    return 0;
}

static void HandleGetRequest(void * context, coap_address_t * addr, const char * path, const char * query, AwaContentType contentType, coap_pdu_t * request, coap_pdu_t * response)
{
    char responseContent[4096];

    CoapRequest coapRequest = {
        .type = COAP_GET_REQUEST, 
        .ctxt = context, 
        .addr = { 0 },
        .path = path,
        .query = query,
        .token = NULL,
        .tokenLength = 0,
        .contentType = contentType,
        .requestContent = NULL,
        .requestContentLen = 0,
    };

    CoapResponse coapResponse = { 
        .responseContent = responseContent, 
        .responseContentLen = sizeof(responseContent),
        .responseCode = 500,
    };

    coap_CoapAddressTypeToAddressType(addr, &coapRequest.addr);

    Lwm2m_Debug("COAP_REQUEST_GET: path %s, query %s contentType %d, request->length %d\n", path, query, contentType, request->length);

    if (requestHandler(&coapRequest, &coapResponse) == 0)
    {
        unsigned char optbuf[2];

        coap_add_option(response, COAP_OPTION_CONTENT_TYPE,
                coap_encode_var_bytes(optbuf, coapResponse.responseContentType), optbuf);

        coap_add_option(response, COAP_OPTION_MAXAGE,
                coap_encode_var_bytes(optbuf, 86400), optbuf);

        coap_add_data(response, coapResponse.responseContentLen, coapResponse.responseContent);
    }

    response->hdr->code = COAP_RESPONSE_CODE(coapResponse.responseCode);
    Lwm2m_Debug("COAP_REQUEST_GET: responseCode %d, responseContentLen %zu\n", coapResponse.responseCode, coapResponse.responseContentLen);
}

static void HandleObserveRequest(void * context, coap_address_t * addr, const char * path, const char * query, AwaContentType contentType, coap_pdu_t * request, coap_pdu_t * response)
{
    char responseContent[512];

    CoapRequest coapRequest = {
        .type = COAP_OBSERVE_REQUEST,
        .ctxt = context,
        .addr = { 0 },
        .path = path,
        .query = query,
        .token = request->hdr->token,
        .tokenLength = request->hdr->token_length,
        .contentType = contentType,
        .requestContent = NULL,
        .requestContentLen = 0,
    };

    CoapResponse coapResponse = {
        .responseContent = responseContent,
        .responseContentLen = sizeof(responseContent),
        .responseCode = 500,
    };

    coap_CoapAddressTypeToAddressType(addr, &coapRequest.addr);

    Lwm2m_Debug("COAP_REQUEST_OBSERVE: path %s, query %s, contentType %d, request->length %d\n", path, query, contentType, request->length);

    if (requestHandler(&coapRequest, &coapResponse) == 0)
    {
        unsigned char optbuf[16];
        int sequence = 1; // start the sequence at 1

        coap_add_option(response, COAP_OPTION_OBSERVE, coap_encode_var_bytes(optbuf, sequence), optbuf);

        coap_add_option(response, COAP_OPTION_CONTENT_TYPE,
                        coap_encode_var_bytes(optbuf, coapResponse.responseContentType), optbuf);

        coap_add_option(response, COAP_OPTION_MAXAGE,
                        coap_encode_var_bytes(optbuf, 86400), optbuf);

        coap_add_data(response, coapResponse.responseContentLen, coapResponse.responseContent);
    }

    response->hdr->code = COAP_RESPONSE_CODE(coapResponse.responseCode);
    Lwm2m_Debug("COAP_REQUEST_OBSERVE: responseCode %d, responseContentLen %zu\n", coapResponse.responseCode, coapResponse.responseContentLen);
}

static void HandleCancelObserveRequest(void * context, coap_address_t * addr, const char * path, const char * query,
                                       AwaContentType contentType, coap_pdu_t * request, coap_pdu_t * response)
{
    char responseContent[512];

    CoapRequest coapRequest = {
        .type = COAP_CANCEL_OBSERVE_REQUEST,
        .ctxt = context,
        .addr = { 0 },
        .path = path,
        .query = query,
        .token = request->hdr->token,
        .tokenLength = request->hdr->token_length,
        .contentType = contentType,
        .requestContent = NULL,
        .requestContentLen = 0,
    };

    CoapResponse coapResponse = {
        .responseContent = responseContent,
        .responseContentLen = sizeof(responseContent),
        .responseCode = 500,
    };

    coap_CoapAddressTypeToAddressType(addr, &coapRequest.addr);

    Lwm2m_Debug("COAP_REQUEST_CANCEL_OBSERVE: path %s, query %s contentType %d, request->length %d\n", path, query, contentType, request->length);

    if (requestHandler(&coapRequest, &coapResponse) == 0)
    {
        unsigned char optbuf[16];

        coap_add_option(response, COAP_OPTION_CONTENT_TYPE,
                        coap_encode_var_bytes(optbuf, coapResponse.responseContentType), optbuf);

        coap_add_option(response, COAP_OPTION_MAXAGE,
                        coap_encode_var_bytes(optbuf, 86400), optbuf);

        coap_add_data(response, coapResponse.responseContentLen, coapResponse.responseContent);
    }

    response->hdr->code = COAP_RESPONSE_CODE(coapResponse.responseCode);
    Lwm2m_Debug("COAP_REQUEST_CANCEL_OBSERVE: responseCode %d, responseContentLen %zu\n", coapResponse.responseCode, coapResponse.responseContentLen);
}

/* 8.2.4: A CoAP PUT is used for the Replace
 * 5.4.3: Replace: replaces the Object Instance or the Resource(s) with the new value provided in the “Write” operation.
 */
static void HandlePutRequest(void * context, coap_address_t * addr, const char * path, const char * query, AwaContentType contentType, coap_pdu_t * request, coap_pdu_t * response)
{
    size_t requestLength;
    unsigned char * requestData;

    CoapRequest coapRequest = {
        .type = COAP_PUT_REQUEST,
        .ctxt = context,
        .addr = { 0 },
        .path = path,
        .query = query,
        .token = request->hdr->token,
        .tokenLength = request->hdr->token_length,
        .contentType = contentType,
        .requestContent = NULL,
        .requestContentLen = 0,
    };

    CoapResponse coapResponse = {
        .responseContent = NULL,
        .responseContentLen = 0,
        .responseCode = 500,
    };

    coap_CoapAddressTypeToAddressType(addr, &coapRequest.addr);

    if (coap_get_data(request, &requestLength, &requestData))
    {
        coapRequest.requestContent = (const char *)requestData;
        coapRequest.requestContentLen = requestLength;
    }

    Lwm2m_Debug("COAP_REQUEST_PUT: path %s, query %s contentType %d, request->length %zu\n", path, query, contentType, requestLength);

    if (requestHandler(&coapRequest, &coapResponse) == 0)
    {
        // Nothing to do here.
    }

    response->hdr->code = COAP_RESPONSE_CODE(coapResponse.responseCode);
    Lwm2m_Debug("COAP_REQUEST_PUT: responseCode %d\n", coapResponse.responseCode);
}

/* 8.2.4: CoAP POST is used for Partial Update
 * 8.2.4: An Object Instance is Created by sending a CoAP POST to the corresponding path
 * 5.4.3: Partial Update: adds or updates Resources or Resource Instances provided in the new value and leaves other existing Resources or Resource Instances unchanged.
 */
static void HandlePostRequest(void * context, coap_address_t * addr, const char * path, const char * query, AwaContentType contentType, coap_pdu_t * request, coap_pdu_t * response)
{
    char responseLocation[512] = {0};
    size_t requestLength = 0;
    unsigned char * requestData = NULL;

    CoapRequest coapRequest = {
        .type = COAP_POST_REQUEST,
        .ctxt = context,
        .addr = { 0 },
        .path = path,
        .query = query,
        .token = request->hdr->token,
        .tokenLength = request->hdr->token_length,
        .contentType = contentType,
        .requestContent = NULL,
        .requestContentLen = 0,
    };

    CoapResponse coapResponse = {
        .responseContent = responseLocation,//NULL,
        .responseContentLen = sizeof(responseLocation), //0
        .responseLocation = responseLocation,
        .responseLocationLen = sizeof(responseLocation),
        .responseCode = 500,
    };

    coap_CoapAddressTypeToAddressType(addr, &coapRequest.addr);

    if (coap_get_data(request, &requestLength, &requestData))
    {
        coapRequest.requestContent = (const char *)requestData;
        coapRequest.requestContentLen = requestLength;
    }

    Lwm2m_Debug("COAP_REQUEST_POST: path %s, query %s, contentType %d, request->length %zu\n", path, query, contentType, requestLength);

    if (requestHandler(&coapRequest, &coapResponse) == 0)
    {
        if (strlen(coapResponse.responseLocation) > 0)
        {
            // add Location-Path
            unsigned char _buf[256];
            unsigned char *buf = _buf;
            size_t buflen = 256;

            int res = coap_split_path(coapResponse.responseLocation, strlen(coapResponse.responseLocation), buf, &buflen);

            while (res--)
            {
                coap_add_option(response, COAP_OPTION_LOCATION_PATH,
                              COAP_OPT_LENGTH(buf), COAP_OPT_VALUE(buf));

                buf += COAP_OPT_SIZE(buf);
            }
        }
    }

    response->hdr->code = COAP_RESPONSE_CODE(coapResponse.responseCode);
    Lwm2m_Debug("COAP_REQUEST_POST: responseCode %d\n", coapResponse.responseCode);
}

static void HandleDeleteRequest(void * context, coap_address_t * addr, const char * path, const char * query, AwaContentType contentType, coap_pdu_t * request, coap_pdu_t * response)
{
    char responseContent[512] = { 0 };

    CoapRequest coapRequest = {
        .type = COAP_DELETE_REQUEST,
        .ctxt = context,
        .addr = { 0 },
        .path = path,
        .query = query,
        .token = request->hdr->token,
        .tokenLength = request->hdr->token_length,
        .contentType = contentType,
    };

    CoapResponse coapResponse = {
        .responseContent = responseContent, // why do we need this for delete?
        .responseContentLen = 0,
        .responseCode = 500,
    };

    coap_CoapAddressTypeToAddressType(addr, &coapRequest.addr);

    Lwm2m_Debug("COAP_REQUEST_DELETE: path %s, query %s request->length %d\n", path, query, request->length);

    // Call directly for now.
    requestHandler(&coapRequest, &coapResponse);

    response->hdr->code = COAP_RESPONSE_CODE(coapResponse.responseCode);
    Lwm2m_Debug("COAP_REQUEST_DELETE: responseCode %d, responseContentLen %zu\n", coapResponse.responseCode, coapResponse.responseContentLen);
}

static void coap_HandleRequest(coap_context_t * ctx,
        struct coap_resource_t * resource,
#ifndef COAP_4_1_1
        const coap_endpoint_t * local_interface,
#endif
        coap_address_t * peer,
        coap_pdu_t * request,
        str * token,
        coap_pdu_t * response)
{
    char requestPath[256] = {0};
    char requestQuery[256] = {0};
    int observe;
    AwaContentType contentType = AwaContentType_None;
    coap_opt_iterator_t opt_iter;
    coap_opt_filter_t filter;
    coap_opt_t *option;

    coap_option_filter_clear(filter);
    coap_option_setb(filter, COAP_OPTION_URI_PATH);

    coap_option_iterator_init(request, &opt_iter, filter);

    while ((option = coap_option_next(&opt_iter)) != NULL)
    {
        char op[128] = {0};
        memcpy(op, COAP_OPT_VALUE(option), COAP_OPT_LENGTH(option));
        sprintf(requestPath + strlen(requestPath), "/%s", op);
    }

    coap_option_filter_clear(filter);
    coap_option_setb(filter, COAP_OPTION_URI_QUERY);

    coap_option_iterator_init(request, &opt_iter, filter);

    bool first = true;

    while ((option = coap_option_next(&opt_iter)))
    {
        char query[128] = {0};
        memcpy(query, COAP_OPT_VALUE(option), COAP_OPT_LENGTH(option));
        if (!first)
        {
            sprintf(requestQuery + strlen(requestQuery), "&%s", query);
        }
        else
        {
            sprintf(requestQuery, "?%s", query);
            first = false;
        }
    }

    if (request->hdr->code == COAP_REQUEST_GET)
    {
        // Get ACCEPTED content type for GET Requests
        option = coap_check_option(request, COAP_OPTION_ACCEPT, &opt_iter);
        if (option != NULL)
        {
            contentType = coap_decode_var_bytes(COAP_OPT_VALUE(option), COAP_OPT_LENGTH(option));
        }

        option = coap_check_option(request, COAP_OPTION_OBSERVE, &opt_iter);
        if (option != NULL)
        {
            observe = coap_decode_var_bytes(COAP_OPT_VALUE(option), COAP_OPT_LENGTH(option));
        }
        else
        {
            observe = -1;
        }
    }
    else
    {
        // For PUT/POST get the payload content type
        option = coap_check_option(request, COAP_OPTION_CONTENT_TYPE, &opt_iter);
        if (option != NULL)
        {
            contentType = coap_decode_var_bytes(COAP_OPT_VALUE(option), COAP_OPT_LENGTH(option));
        }
    }

    switch(request->hdr->code)
    {
        case COAP_REQUEST_GET:
            switch (observe)
            {
                case -1:
                    HandleGetRequest(context, peer, requestPath, requestQuery, contentType, request, response);
                    break;
                case 0:
                    HandleObserveRequest(context, peer, requestPath, requestQuery, contentType, request, response);
                    break;
                case 1:
                    HandleCancelObserveRequest(context, peer, requestPath, requestQuery, contentType, request, response);
                    break;
            }
            break;

        case COAP_REQUEST_PUT:
            HandlePutRequest(context, peer, requestPath, requestQuery, contentType, request, response);
            break;

        case COAP_REQUEST_POST:
            HandlePostRequest(context, peer, requestPath, requestQuery, contentType, request, response);
            break;

        case COAP_REQUEST_DELETE:
            HandleDeleteRequest(context, peer, requestPath, requestQuery, contentType, request, response);
            break;
    }
}

void coap_ResponseHandler(struct coap_context_t * ctx,
#ifndef COAP_4_1_1
        const coap_endpoint_t * local_interface,
#endif
        const coap_address_t * remote,
        coap_pdu_t * sent,
        coap_pdu_t * received,
        const coap_tid_t id)
{
    size_t len = 0;
    unsigned char * databuf = NULL;
    AwaContentType contentType = AwaContentType_None;

    coap_opt_iterator_t opt_iter;
    coap_opt_filter_t filter;
    coap_opt_t *option;

    if (received->hdr->type == COAP_MESSAGE_RST)
    {
        Lwm2m_Debug("got RST for %d\n", id);
        return;
    }

    // Retrieve content type
    option = coap_check_option(received, COAP_OPTION_CONTENT_TYPE, &opt_iter);
    if (option != NULL)
    {
        contentType = coap_decode_var_bytes(COAP_OPT_VALUE(option), COAP_OPT_LENGTH(option));
    }

    if (sent)
    {
        // call callback in LwM2M layer
        TransactionType * transaction = lookup_Transaction(id);

        if (transaction != NULL)
        {
            char responsePath[1024] = {0};
            //bool locationPathOption = false;

            coap_option_filter_clear(filter);
            coap_option_setb(filter, COAP_OPTION_LOCATION_PATH);
            coap_option_iterator_init(received, &opt_iter, filter);
            while ((option = coap_option_next(&opt_iter)))
            {
                char op[128] = {0};
                memcpy(op, COAP_OPT_VALUE(option), COAP_OPT_LENGTH(option));

                if (COAP_OPT_LENGTH(option))
                {
                    sprintf(responsePath + strlen(responsePath), "/%s", op);
                }
            }

            if (strlen(responsePath) == 0)
            {
                // If we do not receive a response path in the response packet,
                // use the path from the request associated with this response.
                sprintf(responsePath, "/%s", transaction->Path);
            }

            if (sent->hdr->code == COAP_REQUEST_GET)
            {
                // There is no block option set, just read the data and we are done.
                if (!coap_get_data(received, &len, &databuf))
                {
                    Lwm2m_Error("Failed to read data from GET response for %d\n", id);
                }

                // If this is a response to an Observe request, then we need to setup a notification handler.
                option = coap_check_option(received, COAP_OPTION_OBSERVE, &opt_iter);
                if (option != NULL)
                {
                    // if we have established an observation, then create a new observer
                    if (!lookup_NotificationHandler(received->hdr->token, received->hdr->token_length))
                    {
                        create_NotificationHandler(received->hdr->token, received->hdr->token_length, transaction->Context, responsePath, transaction->Callback, transaction->NotificationFreeCallback, &transaction->Address);
                    }
                }
                else
                {
                    // Client responded without the observe options set, so clean up and notification handlers
                    NotificationHandler * notify = lookup_NotificationHandler(received->hdr->token, received->hdr->token_length);
                    if (notify != NULL)
                    {
                        remove_NotificationHandler(notify);
                    }
                }
            }

            if (transaction != NULL)
            {
                Lwm2m_Debug("Calling transaction callback\n");
                transaction->Callback(transaction->Context, &transaction->Address, responsePath, COAP_OPTION_TO_RESPONSE_CODE(received->hdr->code), contentType, databuf, len);
                remove_Transaction(transaction);
            }
        }
    }
    else  // unsolicited "response" message, check to see if it's a notification.
    {
        if ((received->hdr->type == COAP_MESSAGE_CON) || (received->hdr->type == COAP_MESSAGE_NON))
        {
            // Notification message, lookup handler
            NotificationHandler * notify = lookup_NotificationHandler(received->hdr->token, received->hdr->token_length);
            if (notify != NULL)
            {
                if (!coap_get_data(received, &len, &databuf))
                {
                    Lwm2m_Error("Failed to read data from unsolicited response message for %d\n", id);
                }

                notify->Callback(notify->Context, notify->Address, notify->Path, COAP_OPTION_TO_RESPONSE_CODE(received->hdr->code), contentType, databuf, len);
            }
        }
    }
}

int coap_RegisterUri(const char * uri)
{
    coap_context_t * context = coapContext;

    if (context == NULL)
    {
        return -1;
    }

    coap_resource_t * r;

    Lwm2m_Debug("register %s\n", uri);
    r = coap_resource_init(uri, strlen(uri), 0);

    coap_register_handler(r, COAP_REQUEST_GET,    coap_HandleRequest);
    coap_register_handler(r, COAP_REQUEST_POST,   coap_HandleRequest);
    coap_register_handler(r, COAP_REQUEST_PUT,    coap_HandleRequest);
    coap_register_handler(r, COAP_REQUEST_DELETE, coap_HandleRequest);

    coap_add_resource(context, r);

    return 0;
}

int coap_DeregisterUri(const char * uri)
{
    coap_context_t * context = coapContext;

    if (context == NULL)
    {
        return -1;
    }

    coap_key_t key;
    memset(key, 0, sizeof(key));
    coap_hash_path(uri, strlen(uri), key);
    coap_delete_resource(context, key);

    Lwm2m_Debug("deregister %s\n", uri);

    return 0;
}

static coap_context_t * getContext(const char * ipAddress, int port)
{
    coap_context_t *ctx = NULL;

    coap_address_t addr;
    coap_address_init(&addr);
    if (inet_pton(AF_INET, ipAddress, &addr.addr.sin.sin_addr) == 1)
    {
        ((struct sockaddr_in *)&addr.addr)->sin_family = AF_INET;
        ((struct sockaddr_in *)&addr.addr)->sin_port = htons(port);
        addr.size = sizeof(struct sockaddr_in);
        ctx = coap_new_context(&addr);
    }
    else if (inet_pton(AF_INET6, ipAddress, &addr.addr.sin6.sin6_addr) == 1)
    {
        ((struct sockaddr_in6 *)&addr.addr)->sin6_family = AF_INET6;
        ((struct sockaddr_in6 *)&addr.addr)->sin6_port = htons(port);
        addr.size = sizeof(struct sockaddr_in6);
        ctx = coap_new_context(&addr);
    }
    else
    {
        Lwm2m_Error("no context available for interface '%s'\n", ipAddress);
    }

    return ctx;
}

void coap_HandleMessage(void)
{
    coap_context_t * ctx = coapContext;
    coap_read(ctx);
#ifdef COAP_4_1_1
    coap_dispatch(ctx);
#endif
}

void coap_Process(void)
{
    coap_context_t * ctx = coapContext;

    coap_queue_t *nextpdu;
    coap_tick_t now;

    nextpdu = coap_peek_next( ctx );

    coap_ticks(&now);
    while (nextpdu && (nextpdu->t <= now - ctx->sendqueue_basetime))
    {
        coap_queue_t * node = coap_pop_next( ctx );

        // lookup transaction
        TransactionType * transaction = lookup_Transaction(node->id);

        coap_tid_t tid = coap_retransmit(ctx, node);

        if (transaction != NULL)
        {
            if (tid == COAP_INVALID_TID)
            {
                Lwm2m_Error("Transaction Timed out\n");
                transaction->Callback(transaction->Context, &transaction->Address, transaction->Path, 504, AwaContentType_None, NULL, 0);

                remove_Transaction(transaction);
            }
            else
            {
                // update transaction ID
                transaction->TransactionID = tid;
            }
        }

        nextpdu = coap_peek_next( ctx );
    }
}

static void coap_SendRequest(int messageType, void * context, char * token, int tokenSize, const char * path, AwaContentType contentType,
                             const char * payload, int payloadLen, TransactionCallback transactionCallback, NotificationFreeCallback notificationFreeCallback)
{
    AddressType addr;
    coap_address_t dst;
    coap_pdu_t *request;
    coap_tid_t tid;
    coap_uri_t uri;

    enum { BUFSIZE = 1024 };
    unsigned char _buf[BUFSIZE];
    unsigned char *buf = _buf;
    size_t buflen;
    int res;
    unsigned short msgID;
    unsigned char * tokenValue;

    coap_split_uri((char *)path, strlen(path), &uri);

    // resolve destination address where server should be sent
    if (!Lwm2mCore_ResolveAddressByName(uri.host.s, uri.host.length, &addr))
    {
        Lwm2m_Error("failed to resolve address\n");
        return;
    }

    memcpy(&dst.addr, &addr.Addr, sizeof(addr.Addr));

    dst.size = addr.Size;
    dst.addr.sin.sin_port = htons(uri.port);

    msgID = coap_new_message_id(coapContext);
    request = coap_pdu_init(COAP_MESSAGE_CON, messageType & 0x0F,
            msgID, COAP_MAX_PDU_SIZE);

    if (token != NULL && tokenSize > 0)
    {
        tokenValue = token;
        request->hdr->token_length = tokenSize;
    }
    else
    {
        // add msg id as token so that each request is treated as it's own
        // independent message, for block transfers we will have to use the same id
        request->hdr->token_length = sizeof(msgID);
        tokenValue = (unsigned char *)&msgID;
    }

    if (!coap_add_token(request, request->hdr->token_length, tokenValue))
    {
        Lwm2m_Error("cannot add token to request\n");
    }

    if (messageType & COAP_MESSAGE_OBSERVE)
    {
        coap_add_option(request, COAP_OPTION_OBSERVE, coap_encode_var_bytes(buf, COAP_OBSERVE_ESTABLISH), buf);
    }
    else if (messageType & COAP_MESSAGE_CANCEL_OBSERVE)
    {
        coap_add_option(request, COAP_OPTION_OBSERVE, coap_encode_var_bytes(buf, COAP_OBSERVE_CANCEL), buf);
    }

    if (uri.port != COAP_DEFAULT_PORT)
    {
        unsigned char portbuf[2];
        coap_add_option(request, COAP_OPTION_URI_PORT,
                coap_encode_var_bytes(portbuf, uri.port), portbuf);
    }

    if (uri.path.length && ((messageType & COAP_MESSAGE_NOTIFY) == 0))
    {
        buflen = BUFSIZE;
        res = coap_split_path(uri.path.s, uri.path.length, buf, &buflen);

        while (res--)
        {
            coap_add_option(request, COAP_OPTION_URI_PATH,
                    COAP_OPT_LENGTH(buf), COAP_OPT_VALUE(buf));

            buf += COAP_OPT_SIZE(buf);
        }
    }

    if ((messageType & 0xF) != COAP_REQUEST_GET)
    {
        // The content type needs to be added to the options before the query,
        // however the payload needs to be added afterwards
        if (payload && payloadLen && contentType != AwaContentType_None)
        {
            coap_add_option(request, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, contentType), buf);
        }
    }

    if (uri.query.length > 0)
    {
        buflen = BUFSIZE;
        res = coap_split_query(uri.query.s, uri.query.length, buf, &buflen);

        while (res--)
        {
            coap_add_option(request, COAP_OPTION_URI_QUERY,
                             COAP_OPT_LENGTH(buf), COAP_OPT_VALUE(buf));

            buf += COAP_OPT_SIZE(buf);
        }
    }

    if ((messageType & 0xF) == COAP_REQUEST_GET)
    {
        if (contentType != AwaContentType_None)
        {
            coap_add_option(request, COAP_OPTION_ACCEPT, coap_encode_var_bytes(buf, contentType), buf);
        }
    }

    if (payload && payloadLen)
    {
        coap_add_data(request, payloadLen, payload);
    }

#ifdef COAP_4_1_1
    tid = coap_send_confirmed(coapContext, &dst, request);
#else
    tid = coap_send_confirmed(coapContext, coapContext->endpoint, &dst, request);
#endif
    if (tid == COAP_INVALID_TID)
    {
        coap_delete_pdu(request);
        return;
    }

    if (transactionCallback != NULL)
    {
        create_Transaction(tid, &dst, uri.path.s, context, transactionCallback, notificationFreeCallback);
    }
}

void coap_PostRequest(void * context, const char * path, AwaContentType contentType, const char * payload, int payloadLen, TransactionCallback callback)
{
    coap_SendRequest(COAP_REQUEST_POST, context, NULL, 0, path, contentType, payload, payloadLen, callback, NULL);
}

void coap_PutRequest(void * context, const char * path, AwaContentType contentType, const char * payload, int payloadLen, TransactionCallback callback)
{
    coap_SendRequest(COAP_REQUEST_PUT, context,  NULL, 0, path, contentType, payload, payloadLen, callback, NULL);
}

void coap_GetRequest(void * context, const char * path, AwaContentType contentType, TransactionCallback callback)
{
    coap_SendRequest(COAP_REQUEST_GET, context,  NULL, 0, path, contentType, NULL, 0, callback, NULL);
}

void coap_Observe(void * context, const char * path, AwaContentType contentType, TransactionCallback transactionCallback, NotificationFreeCallback notificationFreeCallback)
{
    coap_SendRequest(COAP_REQUEST_GET | COAP_MESSAGE_OBSERVE, context,  NULL, 0, path, contentType, NULL, 0, transactionCallback, notificationFreeCallback);
}

void coap_CancelObserve(void * context, const char * path, AwaContentType contentType, TransactionCallback callback)
{
    coap_SendRequest(COAP_REQUEST_GET | COAP_MESSAGE_CANCEL_OBSERVE, context,  NULL, 0, path, contentType, NULL, 0, callback, NULL);
}

void coap_DeleteRequest(void * context, const char * path, TransactionCallback callback)
{
    coap_SendRequest(COAP_REQUEST_DELETE, context,  NULL, 0, path, AwaContentType_None, NULL, 0, callback, NULL);
}

void coap_SendNotify(AddressType * addr, const char * path, const char * token, int tokenSize, AwaContentType contentType, const char * payload, int payloadLen, int sequence)
{
    AddressType destAddr;
    coap_address_t dst;
    coap_pdu_t *request;
    coap_tid_t tid;
    coap_uri_t uri;

    enum { BUFSIZE = 1024 };
    unsigned char _buf[BUFSIZE];
    unsigned char *buf = _buf;
    unsigned short msgID;

    coap_split_uri((char *)path, strlen(path), &uri);

    // resolve destination address where server should be sent
    if (!Lwm2mCore_ResolveAddressByName(uri.host.s, uri.host.length, &destAddr))
    {
        Lwm2m_Error("failed to resolve address\n");
        return;
    }

    memcpy(&dst.addr, &destAddr.Addr, sizeof(destAddr.Addr));

    dst.size = destAddr.Size;
    dst.addr.sin.sin_port = htons(uri.port);

    msgID = coap_new_message_id(coapContext);

    // Response type
    request = coap_pdu_init(COAP_MESSAGE_CON, 0, msgID, COAP_MAX_PDU_SIZE);
    request->hdr->code = COAP_RESPONSE_CODE(205);

    if (!coap_add_token(request, tokenSize, token))
    {
        Lwm2m_Error("cannot add token to request\n");
    }

    coap_add_option(request, COAP_OPTION_OBSERVE, coap_encode_var_bytes(buf, sequence), buf);

    // Add Port to packet
    if (uri.port != COAP_DEFAULT_PORT)
    {
        unsigned char portbuf[2];
        coap_add_option(request, COAP_OPTION_URI_PORT,
                coap_encode_var_bytes(portbuf, uri.port), portbuf);
    }

    coap_add_option(request, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, contentType), buf);

    coap_add_option(request, COAP_OPTION_MAXAGE,
            coap_encode_var_bytes(buf, 86400), buf);

    coap_add_data(request, payloadLen, payload);


#ifdef COAP_4_1_1
    tid = coap_send_confirmed(coapContext, &dst, request);
#else
    tid = coap_send_confirmed(coapContext, coapContext->endpoint, &dst, request);
#endif

    if (tid == COAP_INVALID_TID)
    {
        coap_delete_pdu(request);
        return;
    }
}

CoapInfo * coap_Init(const char * ipAddress, int port, bool secure, int logLevel)
{

    coap_SetLogLevel(logLevel);


    coapContext = getContext(ipAddress != NULL ? ipAddress : "::", port);
    if (coapContext == NULL)
    {
        return NULL;
    }

    coap_register_option(coapContext, COAP_OPTION_BLOCK2);
    coap_register_response_handler(coapContext, coap_ResponseHandler);

    coapInfo.fd = coapContext->sockfd;

    ListInit(&transactionCallbackList);
    ListInit(&notifyCallbackList);

    return &coapInfo;
}

void coap_SetCertificate(const uint8_t * cert, int certLength, AwaCertificateFormat format)
{
	(void)cert;
	(void)certLength;
	(void)format;
}

void coap_SetPSK(const char * identity, const uint8_t * key, int keyLength)
{
    (void)identity;
    (void)key;
    (void)keyLength;
}

void coap_SetLogLevel(int logLevel)
{
    coap_set_log_level(logLevel);
}

static void DestroyLists(void)
{
    struct ListHead * i, * n;

    // free transactionCallbackList
    ListForEachSafe(i, n, &transactionCallbackList)
    {
        TransactionType * transaction = ListEntry(i, TransactionType, List);
        if (transaction != NULL)
        {
            //free(transaction->Context);
            free(transaction->Path);
            free(transaction);
        }
    }

    // free notifyCallbackList
    ListForEachSafe(i, n, &notifyCallbackList)
    {
        NotificationHandler * notify = ListEntry(i, NotificationHandler, List);
        if (notify != NULL)
        {
            if (notify->FreeCallback != NULL)
            {
                notify->FreeCallback(notify->Context);
            }
            free(notify->Path);
            free(notify);
        }
    }
}

int coap_Destroy(void)
{
    coap_free_context(coapContext);
    DestroyLists();

    return 0;
}

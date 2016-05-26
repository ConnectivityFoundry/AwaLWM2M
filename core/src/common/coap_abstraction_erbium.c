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

#include "er-resource.h"
#include "er-session.h"
#include "er-coap-engine.h"

typedef struct
{
    AddressType Address;
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

#define MAX_COAP_TRANSACTIONS (2)
int CurrentTransactionIndex = 0;
//TransactionType CurrentTransaction[MAX_COAP_TRANSACTIONS] = {{0}, {0}};
TransactionType CurrentTransaction[MAX_COAP_TRANSACTIONS];
static coap_session coapSession;		// TODO - support N sessions

int coap_WaitMessage(int timeout, int fd)
{
    // No wait in Erbium (TODO - non-blocking poll for Rx message)
    return timeout;
}

#ifdef CONTIKI
static void coap_HandleResource(/*CoapRequestHandlerCallbacks * RequestCB,*/ void *packet, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    const char *url = NULL;
    int urlLen = 0;
    const uint8_t * payload = NULL;
    int payloadLen = 0;
    int content = -1;

    coap_packet_t *const request = (coap_packet_t *)packet;

    CoapResponse coapResponse = {
       .responseContent = buffer,
       .responseContentLen = preferred_size,
       .responseCode = 400,
    };

    payloadLen = REST.get_request_payload(request, &payload);

    if ((urlLen = REST.get_url(request, &url)))
    {
        char uriBuf[64] = {0};
        rest_resource_flags_t method = REST.get_method_type(request);

        uriBuf[0] = '/';
        memcpy(&uriBuf[1], url, urlLen);

        const char * query = NULL;

        REST.get_query(request, &query);

        CoapRequest coapRequest = {
            .ctxt = context,
            .addr = { 0 },
            .path = uriBuf,
            .query = query,
            .token = request->token,
            .tokenLength = request->token_len,
            .requestContent = payload,
            .requestContentLen = payloadLen,
        };

        memcpy(&coapRequest.addr.Addr, &UIP_IP_BUF->srcipaddr, sizeof(uip_ipaddr_t));
        coapRequest.addr.Port = uip_ntohs(UIP_UDP_BUF->srcport);

        switch(method)
        {
        case METHOD_GET:

            REST.get_header_accept(request, &content);
            coapRequest.contentType = content;

            int32_t observe;

            if (!coap_get_header_observe(request, &observe))
              observe = -1;

            switch(observe)
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
            REST.set_header_content_type(response, coapResponse.responseContentType); /* text/plain is the default, hence this option could be omitted. */
            break;

        case METHOD_POST:
            REST.get_header_content_type(request, &content);
            coapRequest.contentType = content;
            coapRequest.type = COAP_POST_REQUEST;
            Lwm2m_Debug("Coap POST for %s\n", uriBuf);
            requestHandler(&coapRequest, &coapResponse);
            break;

        case METHOD_PUT:
            REST.get_header_content_type(request, &content);
            coapRequest.contentType = content;
            coapRequest.type = COAP_PUT_REQUEST;

            Lwm2m_Debug("Coap PUT for %s\n", uriBuf);
            requestHandler(&coapRequest, &coapResponse);
            break;

        case METHOD_DELETE:
            coapRequest.contentType = ContentType_None;
            coapRequest.type = COAP_DELETE_REQUEST;

            Lwm2m_Debug("Coap DELETE for %s\n", uriBuf);
            requestHandler(&coapRequest, &coapResponse);
            break;

        default:
            break;
        }

        if (coapResponse.responseContentLen > 0 && coapResponse.responseCode == 205 )
        {
            REST.set_response_payload(response, coapResponse.responseContent, coapResponse.responseContentLen);
        }
    }

    REST.set_response_status(response, COAP_RESPONSE_CODE(coapResponse.responseCode));
}

static inline void request_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    coap_HandleResource(request, response, buffer, preferred_size, offset);
}
#endif

int convert_nibble(uint8_t char_val, uint8_t * nibble)
{
    if ((char_val >= 0x30) && (char_val <= 0x39))
    {
        return (*nibble = (char_val & 0x0F));
    }

    if ( ((char_val >= 0x61) && (char_val <= 0x66)) ||
         ((char_val >= 0x41) && (char_val <= 0x46)))
    {
        return (*nibble = ((char_val & 0x0F) + 9));
    }

    return -1;
}

#ifdef CONTIKI
uip_ipaddr_t * coap_getIpFromURI(const char * uri)
{
    char * ipStart = NULL;
    char * ipEnd = NULL;
    static uip_ipaddr_t ipaddr;

    memset(&ipaddr, 0, sizeof(uip_ipaddr_t));

    // TODO - handle IPv6 or IPv4
    // TODO - support DNS? (host IP lookup?)
    ipStart = strchr(uri, '[') + 1;

    if(ipStart != NULL)
    {
        ipEnd = strchr(ipStart, ']');
        if(ipEnd != NULL)
        {
            int ipLen = ipEnd - ipStart;
            char * str = ipStart;
            int colonCount;

            //Count colons
            for (colonCount = 0; ipEnd != &str[colonCount]; str[colonCount] == ':' ? colonCount++ : *str++);

            if(colonCount < 8)
            {
                int currentWord = 0;
                int currentNibble = strchr(ipStart, ':') - (ipStart+1);
                bool jump = false;
                int i;

                if((currentNibble > 3) || (currentNibble < 0))
                {
                    i = ipLen;
                }
                else
                {
                    i = 0;
                }

                for(; i < ipLen; i++)
                {
                    if(ipStart[i] == ':')
                    {
                        if(ipStart[i+1] == ':')
                        {
                            if(!jump)
                            {
                                currentWord += (8 - colonCount);
                                i++;
                                jump = true;
                            }
                            else
                            {
                                break;
                            }
                        }

                        currentWord++;
                        if(currentWord < 7)
                        {
                            currentNibble =  strchr(&ipStart[i]+1, ':') - (&ipStart[i]+2);
                        }
                        else
                        {
                            currentNibble = ipEnd - (&ipStart[i]+2);
                        }

                        if((currentNibble > 3) || (currentNibble < 0))
                        {
                            break;
                        }
                    }
                    else
                    {
                        if(currentNibble >= 0)
                        {
                            uint8_t nibble;
                            if(convert_nibble((uint8_t)ipStart[i], &nibble) != -1)
                            {
                                ipaddr.u16[currentWord] += nibble << (currentNibble*4);

                                if(currentNibble == 0)
                                {
                                    ipaddr.u16[currentWord] = uip_htons(ipaddr.u16[currentWord]);
                                }
                            }
                            else
                            {
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                        currentNibble--;
                    }

                }
            }
        }
    }

    return &ipaddr;
}
#endif

int coap_getPortFromURI(const char * uri)
{
    int port = -1;

    // TODO - support IPv4 (c.f. libCoap or libFlowCore Uri support)
    //char * portStart = strchr(uri, ']') + 2;
    char * portStart = strchr(uri, ']');

    if(portStart != NULL)
    {
    	portStart += 2;
        char * portEnd = strchr(portStart, '/');

        if(portEnd != NULL)
        {
            char portStr[10] = {0};

            if((portEnd - portStart) < (sizeof(portStr) - 1))
            {
                memcpy(&portStr, portStart, portEnd - portStart);

                port = atoi(portStr);
            }
        }
    }
    else
    {
    	port = 15685;	// TODO - temp
    }

    return port;
}

int  coap_getPathQueryFromURI(const char * uri, char * path, char * query)
{
    // TODO - support IPv4 (c.f. libCoap or libFlowCore Uri support)
    int result = -1;
    //char * portStart = strchr(uri, ']') + 2;
    char * portStart = strchr(uri, ']');
    if(portStart != NULL)
    {
    	portStart += 2;
        char * pathStart = strchr(portStart, '/') + 1;

        if(pathStart != NULL)
        {
            char * pathEnd = strchr(pathStart, '?');

            if(pathEnd != NULL)
            {
                char * queryStart = pathEnd + 1;

                memcpy(path, pathStart, pathEnd - pathStart);

                strcpy(query, queryStart);
            }
        }
    }
    else
    {
    	// TODO - remove dummy IPv4 stub
    	// coap://127.0.0.1:15685/bs?ep=ErbiumStaticClient1
    	strcpy(path, "bs");
    	strcpy(query, "ep=ErbiumStaticClient1");
    }

    return result;
}

int coap_ResolveAddressByURI(unsigned char * address, AddressType * addr)
{
    int result = -1;

    Lwm2m_Debug("resolve address from Uri: %s\n", address);		//  TODO - remove
#ifdef CONTIKI
    uip_ipaddr_t * ipv6addr = coap_getIpFromURI(address);
    int port = coap_getPortFromURI(address);

    if(ipv6addr != NULL)
    {
        memcpy(&addr->Addr, ipv6addr, sizeof(*ipv6addr));
        addr->Port = port;
        result = 0;
    }
#endif
    return result;
}

void coap_CoapRequestCallback(void *callback_data, void *response)
{
    TransactionType * transaction = (TransactionType *)callback_data;
    coap_packet_t * coap_response = (coap_packet_t *)response;
    int ContentType = 0;
    const char *url = NULL;
    char * payload = NULL;
    char uriBuf[64] = {0};

    if(callback_data != NULL)
    {
        if(response != NULL)
        {
            int urlLen = 0;
            if ((urlLen = coap_get_header_location_path(response, &url)))
            {
                uriBuf[0] = '/';
                memcpy(&uriBuf[1], url, urlLen);
            }

            coap_get_header_content_format(response, &ContentType);
            int payloadLen = coap_get_payload(response, (const uint8_t **)&payload);

            transaction->Callback(transaction->Context, &transaction->Address, uriBuf, COAP_OPTION_TO_RESPONSE_CODE(coap_response->code), ContentType, payload, payloadLen);
        }
        else
        {
            transaction->Callback(transaction->Context, NULL, NULL, 0, 0, NULL, 0);
        }

        transaction->TransactionUsed = false;
    }
}

static coap_session * coap_GetSessionFromUri(const char * uri)
{
	// TODO - parse uri and map to coap session
    Lwm2m_Debug("get session from Uri: %s\n", uri);		//  TODO - remove

	coap_session * result = NULL;
	result = &coapSession;
	return result;
}

void coap_createCoapRequest(void * context, coap_method_t method, const char * uri, ContentType contentType, const char * payload, int payloadLen, TransactionCallback callback)
{
    coap_packet_t request;
//    uip_ipaddr_t * remote_ipaddr = coap_getIpFromURI(uri);
    int remote_port = coap_getPortFromURI(uri);
    char path[128] = {0};
    char query[128] = {0};
    coap_transaction_t *transaction;
    coap_session * session = coap_GetSessionFromUri(uri);	// TODO - create new session if needed, with max limit

    coap_getPathQueryFromURI(uri, path, query);

    Lwm2m_Info("Coap request: %s\n", uri);
    //Lwm2m_Debug("Coap IPv6 request address: " PRINT6ADDR(remote_ipaddr));
    Lwm2m_Debug("Coap request port: %d\n", remote_port);
    Lwm2m_Debug("Coap request path: %s\n", path);
    Lwm2m_Debug("Coap request query: %s\n", query);

    coap_init_message(&request, COAP_TYPE_CON, method, coap_get_mid());

    coap_set_header_uri_path(&request, path);
    coap_set_header_uri_query(&request, query);
    // TODO - REVIEW: Erbium must copy path/query from request - else mem out of scope

    if (contentType != ContentType_None)
    {
        coap_set_header_content_format(&request, contentType);
        coap_set_payload(&request, payload, payloadLen);
    }

    if (CurrentTransaction[CurrentTransactionIndex].TransactionUsed && CurrentTransaction[CurrentTransactionIndex].TransactionPtr)
    {
        Lwm2m_Warning("Canceled previous transaction [%d]: %p\n", CurrentTransactionIndex, CurrentTransaction[CurrentTransactionIndex].TransactionPtr);
        coap_clear_transaction(CurrentTransaction[CurrentTransactionIndex].TransactionPtr);
    }

    //if ((transaction = coap_new_transaction(request.mid, remote_ipaddr, uip_htons(remote_port))))
    if ((transaction = coap_new_transaction(request.mid, session)))
    {
        transaction->callback = coap_CoapRequestCallback;
        CurrentTransaction[CurrentTransactionIndex].Callback = callback;
        CurrentTransaction[CurrentTransactionIndex].Context = context;
        CurrentTransaction[CurrentTransactionIndex].TransactionUsed = true;
        CurrentTransaction[CurrentTransactionIndex].TransactionPtr = transaction;
//        memcpy(&CurrentTransaction[CurrentTransactionIndex].Address.Addr, remote_ipaddr, sizeof(uip_ipaddr_t));
//        CurrentTransaction[CurrentTransactionIndex].Address.Port = uip_htons(remote_port);

        transaction->callback_data = &CurrentTransaction[CurrentTransactionIndex];

        transaction->packet_len = coap_serialize_message(&request, transaction->packet);

        Lwm2m_Debug("Sending transaction [%d]: %p\n", CurrentTransactionIndex, CurrentTransaction[CurrentTransactionIndex].TransactionPtr);
        coap_send_transaction(transaction);

        CurrentTransactionIndex++;

        if(CurrentTransactionIndex >= MAX_COAP_TRANSACTIONS)
        {
            CurrentTransactionIndex = 0;
        }
    }
}

CoapInfo * coap_Init(const char * ipAddress, int port, int logLevel)
{
	// TODO - create UDP sessions & bind to info (c.f. abstraction_libcoap)
	memset(CurrentTransaction, sizeof(CurrentTransaction), 0);
	memset(&coapSession, sizeof(coapSession), 0);
	coap_init_transactions();
    //rest_init_engine();
    return &coapInfo;
}

void coap_SetLogLevel(int logLevel)
{
	// TODO - set log level for Erbium (replace PRINTFs)
}

int coap_Destroy(void)
{
	// TODO - close any open sessions
//    coap_free_context(coapContext);
//    DestroyLists();
    return 0;
}

void coap_Process(void)
{
	// TODO - needed for Erbium?
	// Do nothing - libCoap only
}

void coap_HandleMessage(void)
{
	// Do nothing - libCoap only
}

void coap_GetRequest(void * context, const char * path, ContentType contentType, TransactionCallback callback)
{
	// Do nothing - libCoap only
}

void coap_PostRequest(void * context, const char * path, ContentType contentType, const char * payload, int payloadLen, TransactionCallback callback)
{
    coap_createCoapRequest(context, COAP_POST, path, contentType, payload, payloadLen, callback);
}

void coap_PutRequest(void * context, const char * path, ContentType contentType, const char * payload, int payloadLen, TransactionCallback callback)
{
    coap_createCoapRequest(context, COAP_PUT, path, contentType, payload, payloadLen, callback);
}

// This is a dummy function - Delete requests are not required on the constrained device and are only used by the LWM2M Server.
void coap_DeleteRequest(void * context, const char * path, TransactionCallback callback)
{
}

// This is a dummy function - Observe requests are not required on the constrained device and are only used by the LWM2M Server.
void coap_Observe(void * context, const char * path, ContentType contentType, TransactionCallback callback, NotificationFreeCallback notificationFreeCallback)
{
}

// This is a dummy function - Cancel Observe Requests are not required on the constrained device and are only used by the LWM2M Server.
void coap_CancelObserve(void * context, const char * path, ContentType contentType, TransactionCallback callback)
{
}

void coap_SendNotify(AddressType * addr, const char * path, const char * token, int tokenSize, ContentType contentType, const char * payload, int payloadLen, int sequence)
{
    coap_packet_t notify;
    coap_transaction_t *transaction;
    coap_session * session = coap_GetSessionFromUri(path);	// TODO - fixme

    Lwm2m_Debug("Coap notify: %s\n", path);
    //Lwm2m_Debug("Coap IPv6 request address: " PRINT6ADDR(&addr->Addr));
    //Lwm2m_Debug("Coap request port: %d\n", addr->Port);

    coap_init_message(&notify, COAP_TYPE_NON, CONTENT_2_05, coap_get_mid());

    if (contentType != ContentType_None)
    {
        coap_set_header_content_format(&notify, contentType);
        coap_set_payload(&notify, payload, payloadLen);
    }

    coap_set_token(&notify, token, tokenSize);
    coap_set_header_observe(&notify, sequence);

    if ((transaction = coap_new_transaction(notify.mid, session)))
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

// TODO - register c/b handler per coap session
//RESOURCE(rest_resource_template,
//         "", //"title=\"Hello world: ?len=0..\";rt=\"Text\"",
//         request_handler,
//         request_handler,
//         request_handler,
//         request_handler);

int coap_RegisterUri(const char * uri)
{
    // TODO - register resource Uri

//    resource_t * temp = malloc(sizeof(resource_t));
//    char * uriCopy = strdup(&uri[1]);
//    Lwm2m_Debug("register %s\n", uriCopy);
//    &rest_resource_template, sizeof(resource_t));
//    rest_activate_resource(temp, uriCopy);

    // TODO - create udp session
    return 0;
}

int coap_DeregisterUri(const char * path)
{
    // Do nothing - not supported in static Awa
    return 0;
}

// Temp session stubs
void session_send_data(coap_session * session, size_t data_length)
{
    Lwm2m_Info("TODO - session_send_data\n");
}
void session_send_data_ptr(coap_session * session, uint8_t * data, size_t data_length)
{
    Lwm2m_Info("TODO - session_send_data_ptr\n");
}


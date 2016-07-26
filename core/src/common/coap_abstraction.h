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


#ifndef COAP_ABSTRACTION_H
#define COAP_ABSTRACTION_H

#include "lwm2m_types.h"
#include "network_abstraction.h"

#ifdef __cplusplus
extern "C" {
#endif

#define COAP_GET_REQUEST 0
#define COAP_PUT_REQUEST 1
#define COAP_POST_REQUEST 2
#define COAP_DELETE_REQUEST 3
#define COAP_OBSERVE_REQUEST 4
#define COAP_CANCEL_OBSERVE_REQUEST 5

typedef struct
{
    int type;
    void * ctxt;
    AddressType addr;
    const char * path;
    const char * query;
    const char * token;
    int tokenLength;
    AwaContentType contentType;
    const char * requestContent;
    size_t requestContentLen;

} CoapRequest;

typedef struct
{
    AwaContentType responseContentType;
    char * responseContent;
    size_t responseContentLen;
    char * responseLocation;
    size_t responseLocationLen;
    int responseCode;

} CoapResponse;

typedef int (*RequestHandler)(CoapRequest * request, CoapResponse * response);
typedef void (*TransactionCallback)(void * context, AddressType * addr, const char * responsePath, int responseCode, AwaContentType contentType, char * payload, size_t payloadLen);
typedef void (*NotificationFreeCallback)(void * context);

typedef struct
{
    int fd;
} CoapInfo;

extern const char * coap_LibraryName;

CoapInfo * coap_Init(const char * ipAddress, int port, bool secure, int logLevel);

void coap_Reset(const char * uri);
void coap_SetCertificate(const uint8_t * cert, int certLength, AwaCertificateFormat format);
void coap_SetPSK(const char * identity, const uint8_t * key, int keyLength);

int coap_Destroy(void);
void coap_Process(void);
void coap_HandleMessage(void);

void coap_SetLogLevel(int logLevel);

void coap_GetRequest(void * context, const char * path, AwaContentType contentType, TransactionCallback callback);
void coap_PostRequest(void * context, const char * uri, AwaContentType contentType, const char * payload, int payloadLen, TransactionCallback callback);
void coap_PutRequest(void * context, const char * path, AwaContentType contentType, const char * payload, int payloadLen, TransactionCallback callback);
void coap_DeleteRequest(void * context, const char * path, TransactionCallback callback);
void coap_Observe(void * context, const char * path, AwaContentType contentType, TransactionCallback transactionCallback, NotificationFreeCallback notificationFreeCallback);
void coap_CancelObserve(void * context, const char * path, AwaContentType contentType, TransactionCallback callback);
void coap_SendNotify(AddressType * addr, const char * path, const char * token, int tokenSize, AwaContentType contentType, const char * payload, int payloadLen, int sequence);

void coap_SetContext(void * ctxt);
void coap_SetRequestHandler(RequestHandler handler);

int coap_WaitMessage(int timeout, int fd);

bool coap_ResolveAddressByURI(unsigned char * address, AddressType * addr);

int coap_RegisterUri(const char * uri);
int coap_DeregisterUri(const char * uri);

#ifdef __cplusplus
}
#endif

#endif // COAP_ABSTRACTION_H

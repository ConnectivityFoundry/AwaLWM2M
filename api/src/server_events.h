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

#ifndef SERVER_EVENTS_H
#define SERVER_EVENTS_H

#include "awa/server.h"
#include "client_iterator.h"
#include "ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

// forward-declare to avoid circular dependency of including registered_entity_iterator.h
typedef struct _RegisteredEntityIterator RegisteredEntityIterator;


typedef struct _ServerEventsCallbackInfo ServerEventsCallbackInfo;

typedef struct _ClientRegisterEvent ClientRegisterEvent;
typedef struct _ClientDeregisterEvent ClientDeregisterEvent;
typedef struct _ClientUpdateEvent ClientUpdateEvent;


// ClientRegisterEvent functions:
ClientRegisterEvent * ClientRegisterEvent_New(void);

void ClientRegisterEvent_Free(ClientRegisterEvent ** event);

int ClientRegisterEvent_AddNotification(ClientRegisterEvent * event, IPCMessage * notification, const AwaServerSession * session);

ClientIterator * ClientRegisterEvent_NewClientIterator(ClientRegisterEvent * event);

RegisteredEntityIterator * ClientRegisterEvent_NewRegisteredEntityIterator(ClientRegisterEvent * event, const char * clientID);


// ClientDeregisterEvent functions:
ClientDeregisterEvent * ClientDeregisterEvent_New(void);

void ClientDeregisterEvent_Free(ClientDeregisterEvent ** event);

int ClientDeregisterEvent_AddNotification(ClientDeregisterEvent * event, IPCMessage * notification, const AwaServerSession * session);

ClientIterator * ClientDeregisterEvent_NewClientIterator(ClientDeregisterEvent * event);


// ClientUpdateEvent functions:
ClientUpdateEvent * ClientUpdateEvent_New(void);

void ClientUpdateEvent_Free(ClientUpdateEvent ** event);

int ClientUpdateEvent_AddNotification(ClientUpdateEvent * event, IPCMessage * notification, const AwaServerSession * session);

ClientIterator * ClientUpdateEvent_NewClientIterator(ClientUpdateEvent * event);

RegisteredEntityIterator * ClientUpdateEvent_NewRegisteredEntityIterator(ClientUpdateEvent * event, const char * clientID);


// ServerEventsCallbackInfo functions:
ServerEventsCallbackInfo * ServerEventsCallbackInfo_New(void);

void ServerEventsCallbackInfo_Free(ServerEventsCallbackInfo ** serverEvents);

int ServerEventsCallbackInfo_SetClientRegisterCallback(ServerEventsCallbackInfo * info, AwaServerClientRegisterEventCallback callback, void * context);

int ServerEventsCallbackInfo_SetClientDeregisterCallback(ServerEventsCallbackInfo * info, AwaServerClientDeregisterEventCallback callback, void * context);

int ServerEventsCallbackInfo_SetClientUpdateCallback(ServerEventsCallbackInfo * info, AwaServerClientUpdateEventCallback callback, void * context);

int ServerEventsCallbackInfo_InvokeClientRegisterCallback(ServerEventsCallbackInfo * info, const ClientRegisterEvent * event);

int ServerEventsCallbackInfo_InvokeClientDeregisterCallback(ServerEventsCallbackInfo * info, const ClientDeregisterEvent * event);

int ServerEventsCallbackInfo_InvokeClientUpdateCallback(ServerEventsCallbackInfo * info, const ClientUpdateEvent * event);


#ifdef __cplusplus
}
#endif

#endif // SERVER_EVENTS_H

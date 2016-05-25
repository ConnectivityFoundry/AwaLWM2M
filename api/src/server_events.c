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

#include "server_events.h"
#include "awa/server.h"
#include "log.h"
#include "server_session.h"
#include "memalloc.h"
#include "server_response.h"
#include "registered_entity_iterator.h"

struct _ServerEventsCallbackInfo
{
    AwaServerClientRegisterEventCallback ClientRegisterEventCallback;
    void * ClientRegisterEventContext;

    AwaServerClientDeregisterEventCallback ClientDeregisterEventCallback;
    void * ClientDeregisterEventContext;

    AwaServerClientUpdateEventCallback ClientUpdateEventCallback;
    void * ClientUpdateEventContext;
};



// These structs are used for API type safety and are never instantiated.
// DO NOT USE THESE STRUCTURES!
struct _AwaServerClientRegisterEvent {};
struct _AwaServerClientDeregisterEvent {};
struct _AwaServerClientUpdateEvent {};

// translate between API types and internal types with casts:
AwaClientIterator * AwaServerClientRegisterEvent_NewClientIterator(const AwaServerClientRegisterEvent * event)
{
    return (AwaClientIterator *)ClientRegisterEvent_NewClientIterator((ClientRegisterEvent *)event);
}

AwaRegisteredEntityIterator * AwaServerClientRegisterEvent_NewRegisteredEntityIterator(const AwaServerClientRegisterEvent * event, const char * clientID)
{
    return (AwaRegisteredEntityIterator *)ClientRegisterEvent_NewRegisteredEntityIterator((ClientRegisterEvent *)event, clientID);
}

AwaClientIterator * AwaServerClientDeregisterEvent_NewClientIterator(const AwaServerClientDeregisterEvent * event)
{
    return (AwaClientIterator *)ClientDeregisterEvent_NewClientIterator((ClientDeregisterEvent *)event);
}

AwaClientIterator * AwaServerClientUpdateEvent_NewClientIterator(const AwaServerClientUpdateEvent * event)
{
    return (AwaClientIterator *)ClientUpdateEvent_NewClientIterator((ClientUpdateEvent *)event);
}

AwaRegisteredEntityIterator * AwaServerClientUpdateEvent_NewRegisteredEntityIterator(const AwaServerClientUpdateEvent * event, const char * clientID)
{
    return (AwaRegisteredEntityIterator *)ClientUpdateEvent_NewRegisteredEntityIterator((ClientUpdateEvent *)event, clientID);
}



// Generic ClientEvent implementation:

typedef struct
{
    IPCMessage * Notification;

    // These are needed to construct a ServerResponse so that ListClients code can be reused
    const AwaServerSession * ServerSession;
    ServerOperation * ServerOperation;
    ServerResponse * ServerResponse;

} ClientEvent;

static ClientEvent * ClientEvent_New(void)
{
    ClientEvent * event = Awa_MemAlloc(sizeof(*event));
    if (event != NULL)
    {
        memset(event, 0, sizeof(*event));
        LogNew("ClientEvent", event);
    }
    else
    {
        LogErrorWithEnum(AwaError_OutOfMemory);
    }
    return event;
}

static void ClientEvent_Free(ClientEvent ** event)
{
    if ((event != NULL) && (*event != NULL))
    {
        LogFree("ClientEvent", *event);
        ServerOperation_Free(&(*event)->ServerOperation);
        ServerResponse_Free(&(*event)->ServerResponse);
        Awa_MemSafeFree(*event);
        *event = NULL;
    }
}

static int ClientEvent_AddNotification(ClientEvent * event, IPCMessage * notification, const char * expectedSubType, const AwaServerSession * session)
{
    int result = -1;
    if (event != NULL)
    {
        if (notification != NULL)
        {
            if (session != NULL)
            {
                // check it's the right Notification
                const char * type = NULL;
                const char * subType = NULL;
                if (IPCMessage_GetType(notification, &type, &subType) == InternalError_Success)
                {
                    if (strcmp(IPC_MESSAGE_TYPE_NOTIFICATION, type) == 0)
                    {
                        if (expectedSubType != NULL)
                        {
                            if (strcmp(expectedSubType, subType) == 0)
                            {
                                event->Notification = notification;
                                event->ServerSession = session;
                                result = 0;
                            }
                            else
                            {
                                LogError("message sub-type '%s' is unexpected", subType);
                            }
                        }
                        else
                        {
                            LogError("expectedSubType is NULL");
                        }
                    }
                    else
                    {
                        LogError("message type '%s' is unexpected", type);
                    }
                }
                else
                {
                    LogError("message is malformed");
                }
            }
            else
            {
                LogError("session is NULL");
            }
        }
        else
        {
            LogError("notification is NULL");
        }
    }
    else
    {
        LogError("event is NULL");
    }
    return result;
}

static const ServerResponse * GetServerResponse(ClientEvent * event)
{
    if (event != NULL)
    {
        if (event->ServerResponse == NULL)
        {
            if (event->Notification != NULL)
            {
                if (event->ServerSession != NULL)
                {
                    TreeNode contentNode = IPCMessage_GetContentNode(event->Notification);
                    TreeNode clientsNode = Xml_Find(contentNode, "Clients");
                    if (event->ServerOperation == NULL)
                    {
                        event->ServerOperation = ServerOperation_New(event->ServerSession);
                    }

                    if (event->ServerOperation != NULL)
                    {
                        event->ServerResponse = ServerResponse_NewFromServerOperation(event->ServerOperation, clientsNode);
                    }
                    else
                    {
                        LogError("Cannot create ServerOperation");
                    }
                }
                else
                {
                    LogError("Event session is NULL");
                }
            }
            else
            {
                LogError("Event notification is NULL");
            }
        }
    }
    else
    {
        LogError("event is NULL");
    }
    return (event != NULL) ? event->ServerResponse : NULL;
}

static ClientIterator * ClientEvent_NewClientIterator(ClientEvent * event)
{
    ClientIterator * iterator = NULL;
    if (event != NULL)
    {
        iterator = ServerResponse_NewClientIterator(GetServerResponse(event));
    }
    else
    {
        LogError("event is NULL");
    }
    return iterator;
}

static RegisteredEntityIterator * ClientEvent_NewRegisteredEntityIterator(ClientEvent * event, const char * clientID)
{
    RegisteredEntityIterator * iterator = NULL;
    if (event != NULL)
    {
        if (clientID != NULL)
        {
            const ResponseCommon * clientResponse = ServerResponse_GetClientResponse(GetServerResponse(event), clientID);
            if (clientResponse != NULL)
            {
                iterator = RegisteredEntityIterator_New(clientResponse);
            }
            else
            {
                LogError("No client response for client ID '%s'", clientID);
            }
        }
        else
        {
            LogError("clientID is NULL");
        }
    }
    else
    {
        LogError("event is NULL");
    }
    return iterator;
}




// ClientRegisterEvent implementation:

struct _ClientRegisterEvent
{
    ClientEvent * ClientEvent;
};

ClientRegisterEvent * ClientRegisterEvent_New(void)
{
    ClientRegisterEvent * event = Awa_MemAlloc(sizeof(*event));
    if (event != NULL)
    {
        memset(event, 0, sizeof(*event));
        event->ClientEvent = ClientEvent_New();
        if (event->ClientEvent != NULL)
        {
            LogNew("ClientRegisterEvent", event);
        }
    }
    else
    {
        LogErrorWithEnum(AwaError_OutOfMemory);
    }
    return event;
}

void ClientRegisterEvent_Free(ClientRegisterEvent ** event)
{
    if ((event != NULL) && (*event != NULL))
    {
        LogFree("ClientRegisterEvent", *event);
        ClientEvent_Free(&(*event)->ClientEvent);
        Awa_MemSafeFree(*event);
        *event = NULL;
    }
}

int ClientRegisterEvent_AddNotification(ClientRegisterEvent * event, IPCMessage * notification, const AwaServerSession * session)
{
    int result = -1;
    if (event != NULL)
    {
        result = ClientEvent_AddNotification(event->ClientEvent, notification, IPC_MESSAGE_SUB_TYPE_CLIENT_REGISTER, session);
    }
    else
    {
        LogError("event is NULL");
    }
    return result;
}

ClientIterator * ClientRegisterEvent_NewClientIterator(ClientRegisterEvent * event)
{
    ClientIterator * iterator = NULL;
    if (event != NULL)
    {
        iterator = ClientEvent_NewClientIterator(event->ClientEvent);
    }
    else
    {
        LogError("event is NULL");
    }
    return iterator;
}

RegisteredEntityIterator * ClientRegisterEvent_NewRegisteredEntityIterator(ClientRegisterEvent * event, const char * clientID)
{
    RegisteredEntityIterator * iterator = NULL;
    if (event != NULL)
    {
        iterator = ClientEvent_NewRegisteredEntityIterator(event->ClientEvent, clientID);
    }
    else
    {
        LogError("event is NULL");
    }
    return iterator;
}


// ClientDeregisterEvent implementation:

struct _ClientDeregisterEvent
{
    ClientEvent * ClientEvent;
};

ClientDeregisterEvent * ClientDeregisterEvent_New(void)
{
    ClientDeregisterEvent * event = Awa_MemAlloc(sizeof(*event));
    if (event != NULL)
    {
        memset(event, 0, sizeof(*event));
        event->ClientEvent = ClientEvent_New();
        if (event->ClientEvent != NULL)
        {
            LogNew("ClientDeregisterEvent", event);
        }
    }
    else
    {
        LogErrorWithEnum(AwaError_OutOfMemory);
    }
    return event;
}

void ClientDeregisterEvent_Free(ClientDeregisterEvent ** event)
{
    if ((event != NULL) && (*event != NULL))
    {
        LogFree("ClientDeregisterEvent", *event);
        ClientEvent_Free(&(*event)->ClientEvent);
        Awa_MemSafeFree(*event);
        *event = NULL;
    }
}

int ClientDeregisterEvent_AddNotification(ClientDeregisterEvent * event, IPCMessage * notification, const AwaServerSession * session)
{
    int result = -1;
    if (event != NULL)
    {
        result = ClientEvent_AddNotification(event->ClientEvent, notification, IPC_MESSAGE_SUB_TYPE_CLIENT_DEREGISTER, session);
    }
    else
    {
        LogError("event is NULL");
    }
    return result;
}

ClientIterator * ClientDeregisterEvent_NewClientIterator(ClientDeregisterEvent * event)
{
    ClientIterator * iterator = NULL;
    if (event != NULL)
    {
        iterator = ClientEvent_NewClientIterator(event->ClientEvent);
    }
    else
    {
        LogError("event is NULL");
    }
    return iterator;
}


// ClientDeregisterEvent implementation:

struct _ClientUpdateEvent
{
    ClientEvent * ClientEvent;
};

ClientUpdateEvent * ClientUpdateEvent_New(void)
{
    ClientUpdateEvent * event = Awa_MemAlloc(sizeof(*event));
    if (event != NULL)
    {
        memset(event, 0, sizeof(*event));
        event->ClientEvent = ClientEvent_New();
        if (event->ClientEvent != NULL)
        {
            LogNew("ClientUpdateEvent", event);
        }
    }
    else
    {
        LogErrorWithEnum(AwaError_OutOfMemory);
    }
    return event;
}

void ClientUpdateEvent_Free(ClientUpdateEvent ** event)
{
    if ((event != NULL) && (*event != NULL))
    {
        LogFree("ClientUpdateEvent", *event);
        ClientEvent_Free(&(*event)->ClientEvent);
        Awa_MemSafeFree(*event);
        *event = NULL;
    }
}

int ClientUpdateEvent_AddNotification(ClientUpdateEvent * event, IPCMessage * notification, const AwaServerSession * session)
{
    int result = -1;
    if (event != NULL)
    {
        result = ClientEvent_AddNotification(event->ClientEvent, notification, IPC_MESSAGE_SUB_TYPE_CLIENT_UPDATE, session);
    }
    else
    {
        LogError("event is NULL");
    }
    return result;
}

ClientIterator * ClientUpdateEvent_NewClientIterator(ClientUpdateEvent * event)
{
    ClientIterator * iterator = NULL;
    if (event != NULL)
    {
        iterator = ClientEvent_NewClientIterator(event->ClientEvent);
    }
    else
    {
        LogError("event is NULL");
    }
    return iterator;
}

RegisteredEntityIterator * ClientUpdateEvent_NewRegisteredEntityIterator(ClientUpdateEvent * event, const char * clientID)
{
    RegisteredEntityIterator * iterator = NULL;
    if (event != NULL)
    {
        iterator = ClientEvent_NewRegisteredEntityIterator(event->ClientEvent, clientID);
    }
    else
    {
        LogError("event is NULL");
    }
    return iterator;
}


// ServerEventsCallbackInfo implementation:

ServerEventsCallbackInfo * ServerEventsCallbackInfo_New(void)
{
    ServerEventsCallbackInfo * serverEvents = Awa_MemAlloc(sizeof(*serverEvents));
    if (serverEvents != NULL)
    {
        memset(serverEvents, 0, sizeof(*serverEvents));
        LogNew("ServerEvents", serverEvents);
    }
    return serverEvents;
}

void ServerEventsCallbackInfo_Free(ServerEventsCallbackInfo ** serverEvents)
{
    if ((serverEvents != NULL) && (*serverEvents != NULL))
    {
        LogFree("ServerEvents", *serverEvents);
        Awa_MemSafeFree(*serverEvents);
        *serverEvents = NULL;
    }
}

int ServerEventsCallbackInfo_SetClientRegisterCallback(ServerEventsCallbackInfo * info, AwaServerClientRegisterEventCallback callback, void * context)
{
    int result = -1;
    if (info != NULL)
    {
        // callback may be NULL
        info->ClientRegisterEventCallback = callback;
        info->ClientRegisterEventContext = context;
        result = 0;
    }
    else
    {
        LogError("info is NULL");
        result = -1;
    }
    return result;
}

int ServerEventsCallbackInfo_SetClientDeregisterCallback(ServerEventsCallbackInfo * info, AwaServerClientDeregisterEventCallback callback, void * context)
{
    int result = -1;
    if (info != NULL)
    {
        // callback may be NULL
        info->ClientDeregisterEventCallback = callback;
        info->ClientDeregisterEventContext = context;
        result = 0;
    }
    else
    {
        LogError("info is NULL");
        result = -1;
    }
    return result;
}

int ServerEventsCallbackInfo_SetClientUpdateCallback(ServerEventsCallbackInfo * info, AwaServerClientUpdateEventCallback callback, void * context)
{
    int result = -1;
    if (info != NULL)
    {
        // callback may be NULL
        info->ClientUpdateEventCallback = callback;
        info->ClientUpdateEventContext = context;
        result = 0;
    }
    else
    {
        LogError("info is NULL");
        result = -1;
    }
    return result;
}

int ServerEventsCallbackInfo_InvokeClientRegisterCallback(ServerEventsCallbackInfo * info, const ClientRegisterEvent * event)
{
    int result = -1;
    if (info != NULL)
    {
        if (event != NULL)
        {
            if (info->ClientRegisterEventCallback != NULL)
            {
                info->ClientRegisterEventCallback((AwaServerClientRegisterEvent *)event, info->ClientRegisterEventContext);
            }
            else
            {
                LogDebug("No ClientRegisterCallback set");
            }
            result = 0;
        }
        else
        {
            LogError("event is NULL");
            result = -1;
        }
    }
    else
    {
        LogError("info is NULL");
        result = -1;
    }
    return result;
}

int ServerEventsCallbackInfo_InvokeClientDeregisterCallback(ServerEventsCallbackInfo * info, const ClientDeregisterEvent * event)
{
    int result = -1;
    if (info != NULL)
    {
        if (event != NULL)
        {
            if (info->ClientDeregisterEventCallback != NULL)
            {
                info->ClientDeregisterEventCallback((AwaServerClientDeregisterEvent *)event, info->ClientDeregisterEventContext);
            }
            else
            {
                LogDebug("No ClientDeregisterCallback set");
            }
            result = 0;
        }
        else
        {
            LogError("event is NULL");
            result = -1;
        }
    }
    else
    {
        LogError("info is NULL");
        result = -1;
    }
    return result;
}

int ServerEventsCallbackInfo_InvokeClientUpdateCallback(ServerEventsCallbackInfo * info, const ClientUpdateEvent * event)
{
    int result = -1;
    if (info != NULL)
    {
        if (event != NULL)
        {
            if (info->ClientUpdateEventCallback != NULL)
            {
                info->ClientUpdateEventCallback((AwaServerClientUpdateEvent *)event, info->ClientUpdateEventContext);
            }
            else
            {
                LogDebug("No ClientUpdateCallback set");
            }
            result = 0;
        }
        else
        {
            LogError("event is NULL");
            result = -1;
        }
    }
    else
    {
        LogError("info is NULL");
        result = -1;
    }
    return result;
}

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



// This struct is used for API type safety and is never instantiated.
// DO NOT USE THIS STRUCTURE!
struct _AwaServerClientRegisterEvent {};

// translate between API types and internal types with casts:
AwaClientIterator * AwaServerClientRegisterEvent_NewClientIterator(const AwaServerClientRegisterEvent * event)
{
    return (AwaClientIterator *)ClientRegisterEvent_NewClientIterator((ClientRegisterEvent *)event);
}

AwaRegisteredEntityIterator * AwaServerClientRegisterEvent_NewRegisteredEntityIterator(const AwaServerClientRegisterEvent * event, const char * clientID)
{
    return (AwaRegisteredEntityIterator *)ClientRegisterEvent_NewRegisteredEntityIterator((ClientRegisterEvent *)event, clientID);
}



// ClientRegisterEvent implementation:

struct _ClientRegisterEvent
{
    IPCMessage * Notification;

    // These are needed to construct a ServerResponse so that ListClients code can be reused
    const AwaServerSession * ServerSession;
    ServerOperation * ServerOperation;
    ServerResponse * ServerResponse;
};

ClientRegisterEvent * ClientRegisterEvent_New(void)
{
    ClientRegisterEvent * event = Awa_MemAlloc(sizeof(*event));
    if (event != NULL)
    {
        memset(event, 0, sizeof(*event));
        LogNew("ClientRegisterEvent", event);
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
        ServerOperation_Free(&(*event)->ServerOperation);
        ServerResponse_Free(&(*event)->ServerResponse);
        Awa_MemSafeFree(*event);
        *event = NULL;
    }
}

int ClientRegisterEvent_AddNotification(ClientRegisterEvent * event, IPCMessage * notification, const AwaServerSession * session)
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
                        if (strcmp(IPC_MESSAGE_SUB_TYPE_CLIENT_REGISTER, subType) == 0)
                        {
                            event->Notification = notification;
                            event->ServerSession = session;
                            result = 0;
                        }
                        else
                        {
                            LogError("message sub0type '%s' is unexpected", subType);
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

static const ServerResponse * GetServerResponse(ClientRegisterEvent * event)
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
    return event->ServerResponse;
}

ClientIterator * ClientRegisterEvent_NewClientIterator(ClientRegisterEvent * event)
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

RegisteredEntityIterator * ClientRegisterEvent_NewRegisteredEntityIterator(ClientRegisterEvent * event, const char * clientID)
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


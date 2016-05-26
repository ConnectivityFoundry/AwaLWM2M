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
#include <stdlib.h>

#include "awa/server.h"
#include "server_session.h"
#include "session_common.h"
#include "memalloc.h"
#include "log.h"
#include "queue.h"
#include "server_notification.h"
#include "observe_operation.h"
#include "server_events.h"

struct _AwaServerSession
{
    SessionCommon * SessionCommon;
    MapType * Observers;
    QueueType * NotificationQueue;
    ServerEventsCallbackInfo * ServerEventsCallbackInfo;
};

AwaServerSession * AwaServerSession_New(void)
{
    AwaServerSession * session = Awa_MemAlloc(sizeof(*session));
    if (session != NULL)
    {
        memset(session, 0, sizeof(*session));

        session->SessionCommon = SessionCommon_New(SessionType_Server);
        if (session->SessionCommon != NULL)
        {
            session->Observers = Map_New();
            if (session->Observers != NULL)
            {
                session->NotificationQueue = Queue_New();
                if (session->NotificationQueue != NULL)
                {
                    session->ServerEventsCallbackInfo = ServerEventsCallbackInfo_New();
                    if (session->ServerEventsCallbackInfo != NULL)
                    {
                        LogNew("AwaServerSession", session);
                    }
                    else
                    {
                        LogErrorWithEnum(AwaError_OutOfMemory, "Could not create server events");
                        Queue_Free(&session->NotificationQueue);
                        Map_Free(&session->Observers);
                        SessionCommon_Free(&session->SessionCommon);
                        Awa_MemSafeFree(session);
                        session = NULL;
                    }
                }
                else
                {
                    LogErrorWithEnum(AwaError_OutOfMemory, "Could not create notification queue");
                    Map_Free(&session->Observers);
                    SessionCommon_Free(&session->SessionCommon);
                    Awa_MemSafeFree(session);
                    session = NULL;
                }
            }
            else
            {
                LogErrorWithEnum(AwaError_OutOfMemory, "Could not create observer list");
                SessionCommon_Free(&session->SessionCommon);
                Awa_MemSafeFree(session);
                session = NULL;
            }
        }
        else
        {
            LogErrorWithEnum(AwaError_OutOfMemory, "Could not create common session");
            SessionCommon_Free(&session->SessionCommon);
            Awa_MemSafeFree(session);
            session = NULL;
        }
    }
    else
    {
        LogErrorWithEnum(AwaError_OutOfMemory, "Could not create ServerSession");
    }
    return session;
}

static void RemoveObservationLinkToSession(const char * key, void * value, void * context)
{
    AwaServerObservation * observation = (AwaServerObservation *)value;
    ServerObservation_RemoveSession(observation);
}

AwaError AwaServerSession_Free(AwaServerSession ** session)
{
    AwaError result = AwaError_Success;
    if ((session != NULL) && (*session != NULL))
    {
        SessionCommon_Free(&((*session)->SessionCommon));
        (*session)->SessionCommon = NULL;

        Map_ForEach((*session)->Observers, RemoveObservationLinkToSession, NULL);
        Map_Free(&(*session)->Observers);
        Queue_Free(&((*session)->NotificationQueue));
        ServerEventsCallbackInfo_Free(&((*session)->ServerEventsCallbackInfo));

        // Free the session itself
        LogFree("AwaServerSession", *session);
        Awa_MemSafeFree(*session);
        *session = NULL;
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return result;
}

AwaError AwaServerSession_SetIPCAsUDP(AwaServerSession * session, const char * address, unsigned short port)
{
    AwaError result = AwaError_Unspecified;
    if (session != NULL)
    {
        result = SessionCommon_SetIPCAsUDP(session->SessionCommon, address, port);
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid);
    }
    return result;
}

AwaError AwaServerSession_Connect(AwaServerSession * session)
{
    AwaError result = AwaError_Unspecified;
    if (session != NULL)
    {
        // use a default IPC configuration if one isn't already set
        if (!SessionCommon_HasIPCInfo(session->SessionCommon))
        {
            if (SessionCommon_SetIPCAsUDP(session->SessionCommon, IPC_DEFAULT_ADDRESS, IPC_DEFAULT_SERVER_PORT) != AwaError_Success)
            {
                result = LogErrorWithEnum(AwaError_IPCError, "Default IPC not suitable");
                goto out;
            }
        }
        result = SessionCommon_ConnectSession(session->SessionCommon);
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
out:
    return result;
}

AwaError AwaServerSession_Disconnect(AwaServerSession * session)
{
    AwaError result = AwaError_Unspecified;
    if (session != NULL)
    {
        result = SessionCommon_DisconnectSession(session->SessionCommon);
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return result;
}

const AwaObjectDefinition * AwaServerSession_GetObjectDefinition(const AwaServerSession * session, AwaObjectID objectID)
{
    const AwaObjectDefinition * objectDefinition = NULL;
    if (session != NULL)
    {
        objectDefinition = SessionCommon_GetObjectDefinition(session->SessionCommon, objectID);
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return objectDefinition;
}

AwaObjectDefinitionIterator * AwaServerSession_NewObjectDefinitionIterator(const AwaServerSession * session)
{
    AwaObjectDefinitionIterator * iterator = NULL;
    if (session != NULL)
    {
        iterator = SessionCommon_NewObjectDefinitionIterator(session->SessionCommon);
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return iterator;
}

bool ServerSession_IsConnected(const AwaServerSession * session)
{
    bool result = false;
    if (session != NULL)
    {
        result = SessionCommon_IsConnected(session->SessionCommon);
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return result;
}

IPCChannel * ServerSession_GetChannel(const AwaServerSession * session)
{
    IPCChannel * channel = NULL;
    if (session != NULL)
    {
        channel = SessionCommon_GetChannel(session->SessionCommon);
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return channel;
}

DefinitionRegistry * ServerSession_GetDefinitionRegistry(const AwaServerSession * session)
{
    DefinitionRegistry * definitions = NULL;
    if (session != NULL)
    {
        definitions = SessionCommon_GetDefinitionRegistry(session->SessionCommon);
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return definitions;
}

AwaError AwaServerSession_PathToIDs(const AwaServerSession * session, const char * path, AwaObjectID * objectID, AwaObjectInstanceID * objectInstanceID, AwaResourceID * resourceID)
{
    AwaError result = AwaError_Unspecified;
    if (session != NULL)
    {
        result = SessionCommon_PathToIDs(session->SessionCommon, path, objectID, objectInstanceID, resourceID);
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return result;
}

AwaError ServerSession_CheckResourceTypeFromPath(const AwaServerSession * session, const char * path, AwaResourceType expected)
{
    AwaError result = AwaError_Unspecified;
    if (session != NULL)
    {
        result = SessionCommon_CheckResourceTypeFromPath(session->SessionCommon, path, expected);
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return result;
}

const AwaResourceDefinition * ServerSession_GetResourceDefinitionFromPath(const AwaServerSession * session, const char * path)
{
    const AwaResourceDefinition * result = NULL;
    if (session != NULL)
    {
        result = SessionCommon_GetResourceDefinitionFromPath(session->SessionCommon, path);
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return result;
}

bool AwaServerSession_IsObjectDefined(const AwaServerSession * session, AwaObjectID objectID)
{
    bool result = false;
    if (session != NULL)
    {
        result = SessionCommon_IsObjectDefined(session->SessionCommon, objectID);
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return result;
}

AwaError AwaServerSession_Process(AwaServerSession * session, AwaTimeout timeout)
{
    AwaError result = AwaError_Unspecified;

    if (session != NULL)
    {
        while (IPC_WaitForNotification(ServerSession_GetChannel(session), timeout) == AwaError_Success)
        {
            IPCMessage * notification;
            if (IPC_ReceiveNotification(ServerSession_GetChannel(session), &notification) == AwaError_Success)
            {
                if (!Queue_Push(session->NotificationQueue, notification))
                {
                    // Queue full?
                    IPCMessage_Free(&notification);
                }
            }
            // we have received at least 1 packet, so we no longer have any reason to wait
            // if there are no more in the pipeline.
            timeout = 0;
        }

        result = AwaError_Success;
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return result;
}

AwaError AwaServerSession_DispatchCallbacks(AwaServerSession * session)
{
    AwaError result = AwaError_Unspecified;
    if (session != NULL)
    {
        IPCMessage * notification;
        while (Queue_Pop(session->NotificationQueue, (void **)&notification))
        {
            ServerNotification_Process(session, notification);
            IPCMessage_Free(&notification);
        }
        result = AwaError_Success;
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return result;
}

// For testing purposes only:
IPCSessionID AwaServerSession_GetSessionID(const AwaServerSession * session)
{
    IPCSessionID sessionID = -1;
    if (session != NULL)
    {
        sessionID = SessionCommon_GetSessionID(session->SessionCommon);
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return sessionID;
}

SessionCommon * ServerSession_GetSessionCommon(const AwaServerSession * session)
{
    SessionCommon * common = NULL;
    if (session != NULL)
    {
        common = session->SessionCommon;
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return common;
}

MapType * ServerSession_GetObservers(const AwaServerSession * session)
{
    MapType * list = NULL;
    if (session != NULL)
    {
        list = session->Observers;
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return list;
}

AwaError AwaServerSession_SetClientRegisterEventCallback(AwaServerSession * session, AwaServerClientRegisterEventCallback callback, void * context)
{
    AwaError result = AwaError_Unspecified;
    if (session != NULL)
    {
        if (ServerEventsCallbackInfo_SetClientRegisterCallback(session->ServerEventsCallbackInfo, callback, context) == 0)
        {
            result = AwaError_Success;
        }
        else
        {
            result = LogErrorWithEnum(AwaError_Internal, "session->ServerEventsCallbackInfo is NULL");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return result;
}

AwaError AwaServerSession_SetClientDeregisterEventCallback(AwaServerSession * session, AwaServerClientDeregisterEventCallback callback, void * context)
{
    AwaError result = AwaError_Unspecified;
    if (session != NULL)
    {
        if (ServerEventsCallbackInfo_SetClientDeregisterCallback(session->ServerEventsCallbackInfo, callback, context) == 0)
        {
            result = AwaError_Success;
        }
        else
        {
            result = LogErrorWithEnum(AwaError_Internal, "session->ServerEventsCallbackInfo is NULL");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return result;
}

AwaError AwaServerSession_SetClientUpdateEventCallback(AwaServerSession * session, AwaServerClientUpdateEventCallback callback, void * context)
{
    AwaError result = AwaError_Unspecified;
    if (session != NULL)
    {
        if (ServerEventsCallbackInfo_SetClientUpdateCallback(session->ServerEventsCallbackInfo, callback, context) == 0)
        {
            result = AwaError_Success;
        }
        else
        {
            result = LogErrorWithEnum(AwaError_Internal, "session->ServerEventsCallbackInfo is NULL");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return result;
}

ServerEventsCallbackInfo * ServerSession_GetServerEventsCallbackInfo(const AwaServerSession * session)
{
    ServerEventsCallbackInfo * info = NULL;
    if (session != NULL)
    {
        info = session->ServerEventsCallbackInfo;
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return info;
}

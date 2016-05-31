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

#include "awa/client.h"
#include "client_session.h"
#include "session_common.h"
#include "memalloc.h"
#include "log.h"
#include "queue.h"
#include "get_response.h"
#include "list.h"
#include "client_subscribe.h"
#include "ipc.h"

struct _AwaClientSession
{
    SessionCommon * SessionCommon;
    MapType * Subscribers;
    QueueType * NotificationQueue;
};

AwaClientSession * AwaClientSession_New(void)
{
    AwaClientSession * session = Awa_MemAlloc(sizeof(*session));
    if (session != NULL)
    {
        memset(session, 0, sizeof(*session));

        session->SessionCommon = SessionCommon_New(SessionType_Client);
        if (session->SessionCommon == NULL)
        {
            LogErrorWithEnum(AwaError_OutOfMemory, "Could not create common session.");
        }

        session->Subscribers = Map_New();
        if (session->Subscribers)
        {
            session->NotificationQueue = Queue_New();
        }
        else
        {
            LogErrorWithEnum(AwaError_OutOfMemory, "Could not create subscriber list.");
            SessionCommon_Free(&session->SessionCommon);
            Awa_MemSafeFree(session);
            session = NULL;
        }
    }
    else
    {
        LogErrorWithEnum(AwaError_OutOfMemory);
    }
    return session;
}

void RemoveSubscriptionLinkToSession(const char * key, void * value, void * context)
{
    AwaClientSubscription * subscription = (AwaClientSubscription *)value;
    ClientSubscription_RemoveSession(subscription);
}

AwaError AwaClientSession_Free(AwaClientSession ** session)
{
    AwaError result = AwaError_Success;
    if ((session != NULL) && (*session != NULL))
    {
        SessionCommon_Free(&((*session)->SessionCommon));
        (*session)->SessionCommon = NULL;

        //free subscriptions
        Map_ForEach((*session)->Subscribers, RemoveSubscriptionLinkToSession, NULL);
        Map_Free(&(*session)->Subscribers);
        Queue_Free(&((*session)->NotificationQueue));

        // Free the session itself
        LogFree("AwaClientSession", *session);
        Awa_MemSafeFree(*session);
        *session = NULL;
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid, "Session is NULL");
    }
    return result;
}

AwaError AwaClientSession_SetIPCAsUDP(AwaClientSession * session, const char * address, unsigned short port)
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

AwaError AwaClientSession_SetDefaultTimeout(AwaClientSession * session, AwaTimeout timeout)
{
    AwaError result = AwaError_Unspecified;
    if (session != NULL)
    {
        SessionCommon * sessionCommon = ClientSession_GetSessionCommon(session);
        if (sessionCommon != NULL)
        {
            result = SessionCommon_SetDefaultTimeout(sessionCommon, timeout);
        }
        else
        {
            result = LogErrorWithEnum(AwaError_SessionInvalid, "sessionCommon is NULL");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return result;
}

AwaError AwaClientSession_Connect(AwaClientSession * session)
{
    AwaError result = AwaError_Unspecified;
    if (session != NULL)
    {
        // use a default IPC configuration if one isn't already set
        if (!SessionCommon_HasIPCInfo(session->SessionCommon))
        {
            if (SessionCommon_SetIPCAsUDP(session->SessionCommon, IPC_DEFAULT_ADDRESS, IPC_DEFAULT_CLIENT_PORT) != AwaError_Success)
            {
                result = LogErrorWithEnum(AwaError_IPCError, "Default IPC not suitable");
                goto out;
            }
        }
        result = SessionCommon_ConnectSession(session->SessionCommon);
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid);
    }
out:
    return result;
}

AwaError AwaClientSession_Disconnect(AwaClientSession * session)
{
    AwaError result = AwaError_Unspecified;
    if (session != NULL)
    {
        result = SessionCommon_DisconnectSession(session->SessionCommon);
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid);
    }
    return result;
}

const AwaObjectDefinition * AwaClientSession_GetObjectDefinition(const AwaClientSession * session, AwaObjectID objectID)
{
    const AwaObjectDefinition * objectDefinition = NULL;
    if (session != NULL)
    {
        objectDefinition = SessionCommon_GetObjectDefinition(session->SessionCommon, objectID);
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid);
    }
    return objectDefinition;
}

AwaObjectDefinitionIterator * AwaClientSession_NewObjectDefinitionIterator(const AwaClientSession * session)
{
    AwaObjectDefinitionIterator * iterator = NULL;
    if (session != NULL)
    {
        iterator = SessionCommon_NewObjectDefinitionIterator(session->SessionCommon);
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid);
    }
    return iterator;
}

bool ClientSession_IsConnected(const AwaClientSession * session)
{
    bool result = false;
    if (session != NULL)
    {
        result = SessionCommon_IsConnected(session->SessionCommon);
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid);
    }
    return result;
}

IPCChannel * ClientSession_GetChannel(const AwaClientSession * session)
{
    IPCChannel * channel = NULL;
    if (session != NULL)
    {
        channel = SessionCommon_GetChannel(session->SessionCommon);
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid);
    }
    return channel;
}

DefinitionRegistry * ClientSession_GetDefinitionRegistry(const AwaClientSession * session)
{
    DefinitionRegistry * definitions = NULL;
    if (session != NULL)
    {
        definitions = SessionCommon_GetDefinitionRegistry(session->SessionCommon);
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid);
    }
    return definitions;
}

AwaError AwaClientSession_PathToIDs(const AwaClientSession * session, const char * path, AwaObjectID * objectID, AwaObjectInstanceID * objectInstanceID, AwaResourceID * resourceID)
{
    AwaError result = AwaError_Unspecified;
    if (session != NULL)
    {
        result = SessionCommon_PathToIDs(session->SessionCommon, path, objectID, objectInstanceID, resourceID);
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid);
    }
    return result;
}

AwaError ClientSession_CheckResourceTypeFromPath(const AwaClientSession * session, const char * path, AwaResourceType expected)
{
    AwaError result = AwaError_Unspecified;
    if (session != NULL)
    {
        result = SessionCommon_CheckResourceTypeFromPath(session->SessionCommon, path, expected);
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid);
    }
    return result;
}

const AwaResourceDefinition * ClientSession_GetResourceDefinitionFromPath(const AwaClientSession * session, const char * path)
{
    const AwaResourceDefinition * result = NULL;
    if (session != NULL)
    {
        result = SessionCommon_GetResourceDefinitionFromPath(session->SessionCommon, path);
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid);
    }
    return result;
}

bool AwaClientSession_IsObjectDefined(const AwaClientSession * session, AwaObjectID objectID)
{
    bool result = false;
    if (session != NULL)
    {
        result = SessionCommon_IsObjectDefined(session->SessionCommon, objectID);
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid);
    }
    return result;
}

// TODO: make this work in combination with Server_ProcessSession
AwaError AwaClientSession_Process(AwaClientSession * session, AwaTimeout timeout)
{
    AwaError result = AwaError_Unspecified;

    if (session != NULL)
    {
        while (IPC_WaitForNotification(ClientSession_GetChannel(session), timeout) == AwaError_Success)
        {
            IPCMessage * notification;
            if (IPC_ReceiveNotification(ClientSession_GetChannel(session), &notification) == AwaError_Success)
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

// TODO: make this work in combination with Server_ProcessSession
AwaError AwaClientSession_DispatchCallbacks(AwaClientSession * session)
{
    AwaError result = AwaError_Unspecified;
    if (session != NULL)
    {
        IPCMessage * notification;

        while (Queue_Pop(session->NotificationQueue, (void **)&notification))
        {
            ClientNotification_Process(session, notification);
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
IPCSessionID AwaClientSession_GetSessionID(const AwaClientSession * session)
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

SessionCommon * ClientSession_GetSessionCommon(const AwaClientSession * session)
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

MapType * ClientSession_GetSubscribers(const AwaClientSession * session)
{
    MapType * list = NULL;
    if (session != NULL)
    {
        list = session->Subscribers;
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return list;
}

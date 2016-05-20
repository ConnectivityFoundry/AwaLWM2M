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

#include "server_notification.h"
#include "awa/server.h"
#include "awa/common.h"
#include "log.h"
#include "memalloc.h"
#include "path_iterator.h"
#include "path.h"
#include "utils.h"
#include "arrays.h"
#include "ipc.h"
#include "xml.h"
#include "observe_operation.h"
#include "server_events.h"

static AwaError HandleObserveNotification(AwaServerSession * session, IPCMessage * notification)
{
    LogDebug("Handle Observe Notification");
    AwaError result = AwaError_Success;
    if (notification != NULL)
    {
        TreeNode contentNode = IPCMessage_GetContentNode(notification);
        if (contentNode != NULL)
        {
            TreeNode clientsNode = Xml_Find(contentNode, "Clients");
            if (clientsNode != NULL)
            {
                result = ServerObserveOperation_CallObservers(session, clientsNode);
            }
            else
            {
                result = LogErrorWithEnum(AwaError_IPCError, "Clients node not found");
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_IPCError, "Content node not found");
        }
    }
    return result;
}

static AwaError HandleClientRegisterNotification(AwaServerSession * session, IPCMessage * notification)
{
    AwaError result = AwaError_Unspecified;
    if (session != NULL)
    {
        ServerEventsCallbackInfo * info = ServerSession_GetServerEventsCallbackInfo(session);
        if (info != NULL)
        {
            ClientRegisterEvent * event = ClientRegisterEvent_New();
            if (event != NULL)
            {
                if (ClientRegisterEvent_AddNotification(event, notification, session) == 0)
                {
                    ServerEventsCallbackInfo_InvokeClientRegisterCallback(info, event);
                    result = AwaError_Success;
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_IPCError, "Cannot add notification to event");
                }
                ClientRegisterEvent_Free(&event);
            }
            else
            {
                result = LogErrorWithEnum(AwaError_OutOfMemory, "Cannot create event");
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_IPCError, "info is NULL");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return result;
}

static AwaError HandleClientDeregisterNotification(AwaServerSession * session, IPCMessage * notification)
{
    AwaError result = AwaError_Unspecified;
    if (session != NULL)
    {
        ServerEventsCallbackInfo * info = ServerSession_GetServerEventsCallbackInfo(session);
        if (info != NULL)
        {
            ClientDeregisterEvent * event = ClientDeregisterEvent_New();
            if (event != NULL)
            {
                if (ClientDeregisterEvent_AddNotification(event, notification, session) == 0)
                {
                    ServerEventsCallbackInfo_InvokeClientDeregisterCallback(info, event);
                    result = AwaError_Success;
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_IPCError, "Cannot add notification to event");
                }
                ClientDeregisterEvent_Free(&event);
            }
            else
            {
                result = LogErrorWithEnum(AwaError_OutOfMemory, "Cannot create event");
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_IPCError, "info is NULL");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return result;
}

static AwaError HandleClientUpdateNotification(AwaServerSession * session, IPCMessage * notification)
{
    AwaError result = AwaError_Unspecified;
    if (session != NULL)
    {
        ServerEventsCallbackInfo * info = ServerSession_GetServerEventsCallbackInfo(session);
        if (info != NULL)
        {
            ClientUpdateEvent * event = ClientUpdateEvent_New();
            if (event != NULL)
            {
                if (ClientUpdateEvent_AddNotification(event, notification, session) == 0)
                {
                    ServerEventsCallbackInfo_InvokeClientUpdateCallback(info, event);
                    result = AwaError_Success;
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_IPCError, "Cannot add notification to event");
                }
                ClientUpdateEvent_Free(&event);
            }
            else
            {
                result = LogErrorWithEnum(AwaError_OutOfMemory, "Cannot create event");
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_IPCError, "info is NULL");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }
    return result;
}

AwaError ServerNotification_Process(AwaServerSession * session, IPCMessage * notification)
{
    AwaError result = AwaError_Unspecified;
    if (session != NULL)
    {
        if (notification != NULL)
        {
            const char * type = NULL;
            const char * subType = NULL;
            if (IPCMessage_GetType(notification, &type, &subType) == InternalError_Success)
            {
                if ((type != NULL) && (subType != NULL))
                {
                    if (strcmp(type, IPC_MESSAGE_TYPE_NOTIFICATION) == 0)
                    {
                        if (strcmp(subType, IPC_MESSAGE_SUB_TYPE_OBSERVE) == 0)
                        {
                            result = HandleObserveNotification(session, notification);
                        }
                        else if (strcmp(subType, IPC_MESSAGE_SUB_TYPE_CLIENT_REGISTER) == 0)
                        {
                            result = HandleClientRegisterNotification(session, notification);
                        }
                        else if (strcmp(subType, IPC_MESSAGE_SUB_TYPE_CLIENT_DEREGISTER) == 0)
                        {
                            result = HandleClientDeregisterNotification(session, notification);
                        }
                        else if (strcmp(subType, IPC_MESSAGE_SUB_TYPE_CLIENT_UPDATE) == 0)
                        {
                            result = HandleClientUpdateNotification(session, notification);
                        }
                        else
                        {
                            result = LogErrorWithEnum(AwaError_IPCError, "Unexpected notification sub-type '%s'", subType);
                        }
                    }
                    else
                    {
                        result = LogErrorWithEnum(AwaError_IPCError, "Unexpected message type '%s'", type);
                    }
                }
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_IPCError, "notification is NULL");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
    }

    return result;
}



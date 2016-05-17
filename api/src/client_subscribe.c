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
#include "client_subscribe.h"
#include "client_subscribe_response.h"
#include "memalloc.h"
#include "log.h"
#include "operation_common.h"
#include "client_session.h"
#include "path.h"
#include "utils.h"
#include "xml.h"
#include "map.h"
#include "list.h"
#include "changeset.h"
#include "subscribe_observe_common.h"

struct _AwaClientSubscription
{
    const char * Path;
    AwaSubscribeType Type;
    void * Callback;
    void * Context;
    bool Cancel;
    const AwaClientSession * Session;
    ListType * Operations;
};

struct _AwaClientSubscribeOperation
{
    OperationCommon * Common;
    MapType * Subscribers;
    AwaClientSubscribeResponse * Response;
};

typedef struct
{
    const AwaClientSession * Session;
    AwaClientSubscribeResponse * Response;
    AwaError Result;
} PerformSuccessfulCallbackContext;

typedef struct
{
    AwaError Result;
    OperationCommon * OperationCommon;
} PerformAddPathCallbackContext;

static void ClientSubscriberList_InvokeSubscribeCallback(MapType * subscribers, const char * path, const AwaChangeSet * changeSet)
{
    AwaClientSubscription * subscription = NULL;
    Map_Get(subscribers, path, (void **)&subscription);
    if (subscription != NULL)
    {
        switch (subscription->Type)
        {
            case AwaSubscribeType_Change:
            {
                AwaClientSubscribeToChangeCallback callback = (AwaClientSubscribeToChangeCallback)subscription->Callback;
                callback(changeSet, subscription->Context);
                break;
            }

            case AwaSubscribeType_Execute:
            {
                AwaExecuteArguments arguments = {NULL, 0};
                AwaError result = AwaError_Unspecified;
                if ((result = ChangeSet_GetExecuteArguments(changeSet, path, &arguments)) == AwaError_Success)
                {
                    AwaClientSubscribeToExecuteCallback callback = (AwaClientSubscribeToExecuteCallback)subscription->Callback;
                    callback(&arguments, subscription->Context);
                }
                else
                {
                    LogErrorWithEnum(result, "Failed to get arguments from execute notification");
                }
                break;
            }

            default:
                LogErrorWithEnum(AwaError_Unsupported, "Unsupported subscribe type: %d", subscription->Type);
                break;
        }
    }
    else
    {
        LogDebug("No subscription for path %s\n", path);
    }
}

/**************************************************************************************************
 * Subscribe Operation
 **************************************************************************************************/

AwaClientSubscribeOperation * AwaClientSubscribeOperation_New(const AwaClientSession * session)
{
    AwaClientSubscribeOperation * operation = NULL;

    if (session != NULL)
    {
        if (ClientSession_IsConnected(session) != false)
        {
            operation = Awa_MemAlloc(sizeof(*operation));
            if (operation != NULL)
            {
                memset(operation, 0, sizeof(*operation));
                operation->Common = OperationCommon_NewWithClientSession(session);
                if (operation->Common != NULL)
                {
                    operation->Subscribers = Map_New();

                    if (operation->Subscribers != NULL)
                    {
                        LogNew("AwaClientSubscribeOperation", operation);
                    }
                    else
                    {
                        LogErrorWithEnum(AwaError_OutOfMemory, "Could not create subscriber list.");
                        Awa_MemSafeFree(operation);
                        operation = NULL;
                    }
                }
                else
                {
                    LogErrorWithEnum(AwaError_Internal, "Unable to initialise operation.");
                    Awa_MemSafeFree(operation);
                    operation = NULL;
                }
            }
            else
            {
                LogErrorWithEnum(AwaError_OutOfMemory);
            }
        }
        else
        {
            LogError("Session is not connected");
        }
    }
    else
    {
        LogError("Session is NULL");
    }
    return operation;
}

void RemoveSubscriptionLinkToOperation(const char * key, void * value, void * context)
{
    AwaClientSubscription * subscription = (AwaClientSubscription *)value;
    AwaClientSubscribeOperation * operation = (AwaClientSubscribeOperation *)context;
    List_Remove(subscription->Operations, operation);
}

AwaError AwaClientSubscribeOperation_Free(AwaClientSubscribeOperation ** operation)
{
    AwaError result = AwaError_OperationInvalid;
    if (operation != NULL && *operation != NULL)
    {
        OperationCommon_Free(&(*operation)->Common);
        Map_ForEach((*operation)->Subscribers, RemoveSubscriptionLinkToOperation, *operation);
        Map_Free(&(*operation)->Subscribers);

        if ((*operation)->Response != NULL)
        {
            SubscribeResponse_Free(&(*operation)->Response);
        }

        LogFree("AwaClientSubscribeOperation", *operation);
        Awa_MemSafeFree(*operation);
        *operation = NULL;
        result = AwaError_Success;
    }
    return result;
}

OperationCommon * ClientSubscribeOperation_GetOperationCommon(const AwaClientSubscribeOperation * operation)
{
    return (operation != NULL) ? operation->Common : NULL;
}

MapType * ClientSubscribeOperation_GetSubscribers(const AwaClientSubscribeOperation * operation)
{
    return (operation != NULL) ? operation->Subscribers : NULL;
}

static AwaError ClientSubscribeOperation_Add(AwaClientSubscribeOperation * operation, AwaClientSubscription * subscription, bool cancel)
{
    AwaError result = AwaError_Unspecified;

    if (operation != NULL)
    {
        if (subscription != NULL)
        {
            AwaClientSubscription * existingSubscription = NULL;
            Map_Get(operation->Subscribers, subscription->Path, (void **)&existingSubscription);
            if (existingSubscription == NULL)
            {
                if (Map_Put(operation->Subscribers, subscription->Path, subscription))
                {
                    List_Add(subscription->Operations, operation);
                    subscription->Cancel = cancel;
                    result = AwaError_Success;
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_Internal, "Failed to add subscription to operation");
                }
            }
            else
            {
                result = LogErrorWithEnum(AwaError_OperationInvalid, "A subscription already exists for path %s in the given operation.", subscription->Path);
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_SubscriptionInvalid, "Subscription is NULL");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "Operation is NULL");
    }

    return result;
}

AwaError AwaClientSubscribeOperation_AddChangeSubscription(AwaClientSubscribeOperation * operation, AwaClientChangeSubscription * subscription)
{
    return ClientSubscribeOperation_Add(operation, (AwaClientSubscription *)subscription, false);
}

AwaError AwaClientSubscribeOperation_AddExecuteSubscription(AwaClientSubscribeOperation * operation, AwaClientExecuteSubscription * subscription)
{
    return ClientSubscribeOperation_Add(operation, (AwaClientSubscription *)subscription, false);
}

AwaError AwaClientSubscribeOperation_AddCancelChangeSubscription(AwaClientSubscribeOperation * operation, AwaClientChangeSubscription * subscription)
{
    return ClientSubscribeOperation_Add(operation, (AwaClientSubscription *)subscription, true);
}

AwaError AwaClientSubscribeOperation_AddCancelExecuteSubscription(AwaClientSubscribeOperation * operation, AwaClientExecuteSubscription * subscription)
{
    return ClientSubscribeOperation_Add(operation, (AwaClientSubscription *)subscription, true);
}

InternalError ClientSubscribe_AddAwaSubscribeType(TreeNode leafNode, AwaClientSubscription * subscription)
{
    InternalError result = InternalError_Unspecified;
    const char * messageType = NULL;
    switch(subscription->Type)
    {
        case AwaSubscribeType_Change:
            messageType = subscription->Cancel? IPC_MESSAGE_TAG_CANCEL_SUBSCRIBE_TO_CHANGE : IPC_MESSAGE_TAG_SUBSCRIBE_TO_CHANGE;
            break;
        case AwaSubscribeType_Execute:
            messageType = subscription->Cancel? IPC_MESSAGE_TAG_CANCEL_SUBSCRIBE_TO_EXECUTE : IPC_MESSAGE_TAG_SUBSCRIBE_TO_EXECUTE;
            break;
        default:
            result = LogErrorWithEnum(InternalError_Unspecified, "node is NULL");
            goto error;
    }

    if (leafNode != NULL)
    {
        TreeNode subscribeTypeNode = Xml_CreateNode(messageType);
        if (TreeNode_AddChild(leafNode, subscribeTypeNode))
        {
            result = InternalError_Success;
        }
        else
        {
            result = LogErrorWithEnum(InternalError_Tree, "Failed to add AwaSubscribeType tag to leaf node");
        }
    }
    else
    {
        result = LogErrorWithEnum(InternalError_ParameterInvalid, "node is NULL");
    }
error:
    return result;
}

static void ClientSubscribe_PerformAddPathCallback(const char * path, void * value, void * context)
{
    PerformAddPathCallbackContext * addPathContext = (PerformAddPathCallbackContext *)context;
    if (addPathContext->Result == AwaError_Success)
    {
        OperationCommon * operationCommon = addPathContext->OperationCommon;
        AwaClientSubscription * subscription = (AwaClientSubscription *)value;

        TreeNode resultNode;
        if ((addPathContext->Result = OperationCommon_AddPathV2(operationCommon, subscription->Path, &resultNode)) == AwaError_Success)
        {
            if (ClientSubscribe_AddAwaSubscribeType(resultNode, subscription) == InternalError_Success)
            {
                addPathContext->Result = AwaError_Success;
            }
            else
            {
                addPathContext->Result = LogErrorWithEnum(AwaError_Internal, "Failed to add value to path");
            }
        }
    }
}

static void ClientSubscribe_PerformSuccessfulCallback(const char * path, void * value, void * context)
{
    AwaClientSubscription * subscription = (AwaClientSubscription *)value;
    PerformSuccessfulCallbackContext * successContext = (PerformSuccessfulCallbackContext *)context;

    const AwaPathResult * result = AwaClientSubscribeResponse_GetPathResult(successContext->Response, subscription->Path);
    if (!subscription->Cancel)
    {
        if (AwaPathResult_GetError(result) == AwaError_Success)
        {
            Map_Put(ClientSession_GetSubscribers(successContext->Session), subscription->Path, subscription);

            // map the subscription to this session.
            subscription->Session = successContext->Session;
        }
        else
        {
            LogErrorWithEnum(AwaPathResult_GetError(result), "Failed to subscribe to path %s\n", subscription->Path);
            successContext->Result = AwaError_Response;
        }
    }
}

AwaError AwaClientSubscribeOperation_Perform(AwaClientSubscribeOperation * operation, AwaTimeout timeout)
{
    AwaError result = AwaError_Unspecified;

    if (timeout >= 0)
    {
        if (operation != NULL)
        {
            PerformAddPathCallbackContext addPathContext;
            addPathContext.OperationCommon = operation->Common;
            addPathContext.Result = AwaError_Success;
            Map_ForEach(operation->Subscribers, ClientSubscribe_PerformAddPathCallback, (void *)&addPathContext);
            result = addPathContext.Result;

            if (result != AwaError_Success)
            {
                goto error;
            }

            const AwaClientSession * session = OperationCommon_GetSession(operation->Common, NULL);
            if (ClientSession_IsConnected(session))
            {
                TreeNode objectsTree = OperationCommon_GetObjectsTree(operation->Common);
                if (objectsTree != NULL)
                {
                    if (TreeNode_GetChildCount(objectsTree) > 0)
                    {
                        // build an IPC message and inject our content (object paths) into it
                        IPCMessage * subscribeRequest = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_REQUEST, IPC_MESSAGE_SUB_TYPE_SUBSCRIBE, OperationCommon_GetSessionID(operation->Common));
                        IPCMessage_AddContent(subscribeRequest, objectsTree);
                        IPCMessage * subscribeResponse = NULL;
                        result = IPC_SendAndReceive(ClientSession_GetChannel(session), subscribeRequest, &subscribeResponse, timeout);

                        if (result == AwaError_Success)
                        {
                            IPCResponseCode responseCode = IPCMessage_GetResponseCode(subscribeResponse);
                            if (responseCode == IPCResponseCode_Success)
                            {
                                // Free an old response if it exists
                                if (operation->Response != NULL)
                                {
                                    result = SubscribeResponse_Free(&operation->Response);
                                }
                                
                                if (result == AwaError_Success)
                                {
                                    // Detach the response content and store it in the operation's ClientGetResponse
                                    TreeNode contentNode = IPCMessage_GetContentNode(subscribeResponse);
                                    TreeNode objectsNode = Xml_Find(contentNode, "Objects");
                                    operation->Response = SubscribeResponse_New(operation, objectsNode);

                                    if (operation->Response)
                                    {
                                        PerformSuccessfulCallbackContext successContext;
                                        successContext.Session = OperationCommon_GetSession(operation->Common, SessionType_Client);
                                        successContext.Response = operation->Response;
                                        successContext.Result = AwaError_Success;
                                        Map_ForEach(operation->Subscribers, ClientSubscribe_PerformSuccessfulCallback, (void *)&successContext);

                                        result = successContext.Result;
                                        LogDebug("Perform Subscribe Operation finished %s\n", (result == AwaError_Success? "successfully" : "with errors"));
                                    }
                                    else
                                    {
                                        result = LogErrorWithEnum(AwaError_Internal, "An internal error occurred when parsing Get Response");
                                    }
                                }
                            }
                            else
                            {
                                result = LogErrorWithEnum(AwaError_IPCError, "Unexpected IPC response code: %d", responseCode);
                            }
                        }

                        IPCMessage_Free(&subscribeRequest);
                        IPCMessage_Free(&subscribeResponse);
                    }
                    else
                    {
                        result = LogErrorWithEnum(AwaError_OperationInvalid, "No paths specified");
                    }
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_Internal, "ObjectsTree is NULL");
                }
            }
            else
            {
                LogError("Session is not connected");
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_OperationInvalid, "Operation is NULL");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "Invalid timeout");
    }
error:
    return result;
}

const AwaClientSubscribeResponse * AwaClientSubscribeOperation_GetResponse(const AwaClientSubscribeOperation * operation)
{
    return (operation != NULL) ? operation->Response : NULL;
}

AwaError ClientSubscribe_CallSubscribers(AwaClientSession * session, TreeNode contentNode)
{
    MapType * subscribers = ClientSession_GetSubscribers(session);

    TreeNode objectsTree = Xml_Find(contentNode, "Objects");
    AwaChangeSet * changeSet = ChangeSet_New(session, SessionType_Client, objectsTree);

    AwaError result = SubscribeObserveCommon_CallObservers(subscribers, changeSet, ClientSubscriberList_InvokeSubscribeCallback);
    ChangeSet_Free(&changeSet);
    return result;
}

static AwaClientSubscription * ClientSubscription_New(const char * path, void * callback, void * context, AwaSubscribeType subscribeType)
{
    AwaClientSubscription * subscription = NULL;

    if ((path != NULL) && (callback != NULL))
    {
        subscription = Awa_MemAlloc(sizeof(*subscription));

        if (subscription != NULL)
        {
            subscription->Path = strdup(path);
            if (subscription->Path != NULL)
            {
                subscription->Operations = List_New();
                if (subscription->Operations != NULL)
                {
                    subscription->Callback = (void *)callback;
                    subscription->Context = context;
                    subscription->Session = NULL;
                    subscription->Type = subscribeType;
                }
                else
                {
                    free((void *)subscription->Path);
                    Awa_MemSafeFree(subscription);
                    LogErrorWithEnum(AwaError_OutOfMemory);
                    subscription = NULL;
                }
            }
            else
            {
                Awa_MemSafeFree(subscription);
                LogErrorWithEnum(AwaError_OutOfMemory);
                subscription = NULL;
            }
        }
        else
        {
            LogErrorWithEnum(AwaError_OutOfMemory);
        }
    }
    return subscription;
}

void RemoveSubscriptionFromOperation(size_t index, void * value, void * context)
{
    AwaClientSubscribeOperation * operation = (AwaClientSubscribeOperation *)value;
    AwaClientSubscription * subscription = (AwaClientSubscription *)context;
    Map_Remove(operation->Subscribers, subscription->Path);
}

static AwaError ClientSubscription_Free(AwaClientSubscription ** subscription)
{
    AwaError result = AwaError_SubscriptionInvalid;

    if ((subscription != NULL) && (*subscription != NULL))
    {
        List_ForEach((*subscription)->Operations, RemoveSubscriptionFromOperation, *subscription);
        List_Free(&(*subscription)->Operations);

        if ((*subscription)->Session != NULL)
        {
            Map_Remove(ClientSession_GetSubscribers((*subscription)->Session), (*subscription)->Path);
        }

        free((void *)(*subscription)->Path);
        Awa_MemSafeFree(*subscription);
        *subscription = NULL;

        result = AwaError_Success;
    }
    return result;
}

const char * ClientSubscribe_GetPath(AwaClientChangeSubscription * subscription)
{
    const char * path = NULL;
    if (subscription != NULL)
    {
        path = subscription->Path;
    }
    return path;
}

ListType * ClientSubscription_GetOperations(AwaClientSubscription * subscription)
{
    ListType * operations = NULL;
    if (subscription != NULL)
    {
        operations = subscription->Operations;
    }
    return operations;
}

void ClientSubscription_RemoveSession(AwaClientSubscription * subscription)
{
    if (subscription != NULL)
    {
        subscription->Session = NULL;
    }
}


/**************************************************************************************************
 * Change Subscriptions
 **************************************************************************************************/

AwaClientChangeSubscription * AwaClientChangeSubscription_New(const char * path, AwaClientSubscribeToChangeCallback callback, void * context)
{
    return (AwaClientChangeSubscription *)ClientSubscription_New(path, callback, context, AwaSubscribeType_Change);
}

AwaError AwaClientChangeSubscription_Free(AwaClientChangeSubscription ** subscription)
{
    return ClientSubscription_Free((AwaClientSubscription **)subscription);
}

const char * AwaClientChangeSubscription_GetPath(AwaClientChangeSubscription * subscription)
{
    return ClientSubscribe_GetPath((AwaClientSubscription *)subscription);
}


/**************************************************************************************************
 * Execute Operation
 **************************************************************************************************/

AwaClientExecuteSubscription * AwaClientExecuteSubscription_New(const char * path, AwaClientSubscribeToExecuteCallback callback, void * context)
{
    return (AwaClientExecuteSubscription *)ClientSubscription_New(path, callback, context, AwaSubscribeType_Execute);
}

AwaError AwaClientExecuteSubscription_Free(AwaClientExecuteSubscription ** subscription)
{
    return ClientSubscription_Free((AwaClientSubscription **)subscription);
}

const char * AwaClientExecuteSubscription_GetPath(AwaClientExecuteSubscription * subscription)
{
    return ClientSubscribe_GetPath((AwaClientSubscription *)subscription);
}

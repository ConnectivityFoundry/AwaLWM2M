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
#include "observe_operation.h"
#include "memalloc.h"
#include "log.h"
#include "server_operation.h"
#include "server_session.h"
#include "path.h"
#include "utils.h"
#include "xml.h"
#include "map.h"
#include "server_response.h"
#include "subscribe_observe_common.h"
#include "list.h"

struct _AwaServerObservation
{
    const char * Path;
    void * Callback;
    void * Context;
    bool Cancel;
    const AwaServerSession * Session;
    ListType * Operations;
    const char * ClientID;
};

struct _AwaServerObserveOperation
{
    ServerOperation * ServerOperation;
    MapType * Observers;
    ServerResponse * Response;
};

typedef struct
{
    AwaError Result;
    ServerOperation * ServerOperation;
} PerformAddPathCallbackContext;

typedef struct
{
    const AwaServerSession * Session;
    AwaError Result;
    AwaServerObserveOperation * Operation;
} PerformSuccessfulCallbackContext;

// This struct is used for API type safety and is never instantiated.
// DO NOT USE THIS STRUCTURE!
struct _AwaServerObserveResponse {};

static void ServerObserverList_InvokeObserveCallback(MapType * observers, const char * path, const AwaChangeSet * changeSet)
{
    AwaServerObservation * observation = NULL;
    Map_Get(observers, path, (void **)&observation);
    if (observation != NULL)
    {
        AwaServerObservationCallback callback = (AwaServerObservationCallback)observation->Callback;
        callback(changeSet, observation->Context);
    }
    else
    {
        LogDebug("No observation for path %s", path);
    }
}

AwaServerObserveOperation * AwaServerObserveOperation_New(const AwaServerSession * session)
{
    AwaServerObserveOperation * operation = NULL;

    if (session != NULL)
    {
        if (ServerSession_IsConnected(session) != false)
        {
            operation = Awa_MemAlloc(sizeof(*operation));
            if (operation != NULL)
            {
                memset(operation, 0, sizeof(*operation));
                operation->ServerOperation = ServerOperation_New(session);
                if (operation->ServerOperation != NULL)
                {
                    operation->Observers = Map_New();

                    if (operation->Observers != NULL)
                    {
                        LogNew("AwaServerObserveOperation", operation);
                    }
                    else
                    {
                        LogErrorWithEnum(AwaError_OutOfMemory, "Could not create observer list.");
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

void RemoveObservationLinkToOperation(const char * key, void * value, void * context)
{
    AwaServerObservation * Observation = (AwaServerObservation *)value;
    AwaServerObserveOperation * operation = (AwaServerObserveOperation *)context;
    List_Remove(Observation->Operations, operation);
}

AwaError AwaServerObserveOperation_Free(AwaServerObserveOperation ** operation)
{
    AwaError result = AwaError_OperationInvalid;
    if ((operation != NULL) && (*operation != NULL))
    {
        ServerOperation_Free(&(*operation)->ServerOperation);

        Map_ForEach((*operation)->Observers, RemoveObservationLinkToOperation, *operation);
        Map_Free(&(*operation)->Observers);

        ServerResponse_Free(&(*operation)->Response);

        LogFree("AwaServerObserveOperation", *operation);
        Awa_MemSafeFree(*operation);
        *operation = NULL;
        result = AwaError_Success;
    }
    return result;
}

AwaServerObservation * AwaServerObservation_New(const char * clientID, const char * path, AwaServerObservationCallback callback, void * context)
{
    AwaServerObservation * observation = NULL;

    if ((clientID != NULL) && (path != NULL) && (callback != NULL))
    {
        observation = Awa_MemAlloc(sizeof(*observation));

        if (observation != NULL)
        {
            observation->Path = strdup(path);
            if (observation->Path != NULL)
            {
                observation->ClientID = strdup(clientID);
                if (observation->ClientID != NULL)
                {
                    observation->Operations = List_New();
                    if (observation->Operations != NULL)
                    {
                        observation->Callback = (void*)callback;
                        observation->Context = context;
                        observation->Session = NULL;
                    }
                    else
                    {
                        free((void *)observation->Path);
                        free((void *)observation->ClientID);
                        Awa_MemSafeFree(observation);
                        LogErrorWithEnum(AwaError_OutOfMemory);
                        observation = NULL;
                    }
                }
                else
                {
                    free((void *)observation->Path);
                    Awa_MemSafeFree(observation);
                    LogErrorWithEnum(AwaError_OutOfMemory);
                    observation = NULL;
                }
            }
            else
            {
                Awa_MemSafeFree(observation);
                LogErrorWithEnum(AwaError_OutOfMemory);
                observation = NULL;
            }
        }
        else
        {
            LogErrorWithEnum(AwaError_OutOfMemory);
        }
    }
    return observation;
}

// Retrieve the path from a Change subscription.
const char * AwaServerObservation_GetPath(AwaServerObservation * observation)
{
    const char * path = NULL;
    if (observation != NULL)
    {
        path = observation->Path;
    }
    return path;
}

void RemoveObservationFromOperation(size_t index, void * value, void * context)
{
    AwaServerObserveOperation * operation = (AwaServerObserveOperation *)value;
    AwaServerObservation * Observation = (AwaServerObservation *)context;
    Map_Remove(operation->Observers, Observation->Path);
}

AwaError AwaServerObservation_Free(AwaServerObservation ** observation)
{
    AwaError result = AwaError_ObservationInvalid;

    if ((observation != NULL) && (*observation != NULL))
    {
        List_ForEach((*observation)->Operations, RemoveObservationFromOperation, *observation);
        List_Free(&(*observation)->Operations);

        if ((*observation)->Session != NULL)
        {
            Map_Remove(ServerSession_GetObservers((*observation)->Session), (*observation)->Path);
        }

        free((void *)(*observation)->Path);
        free((void *)(*observation)->ClientID);
        Awa_MemSafeFree(*observation);
        *observation = NULL;

        result = AwaError_Success;
    }
    return result;
}

InternalError ServerObserve_AddObserveType(TreeNode leafNode, AwaServerObservation * subscription)
{
    InternalError result = InternalError_Unspecified;
    const char * messageType = subscription->Cancel? IPC_MESSAGE_TAG_CANCEL_OBSERVATION : IPC_MESSAGE_TAG_OBSERVE;

    if (leafNode != NULL)
    {
        TreeNode subscribeTypeNode = Xml_CreateNode(messageType);
        if (TreeNode_AddChild(leafNode, subscribeTypeNode))
        {
            result = InternalError_Success;
        }
        else
        {
            result = LogErrorWithEnum(InternalError_Tree, "Failed to add ObserveType tag to leaf node");
        }
    }
    else
    {
        result = LogErrorWithEnum(InternalError_ParameterInvalid, "node is NULL");
    }
    return result;
}

static void ServerObserve_PerformAddPathCallback(const char * path, void * value, void * context)
{
    PerformAddPathCallbackContext * addPathContext = (PerformAddPathCallbackContext *)context;
    if (addPathContext->Result == AwaError_Success)
    {
        ServerOperation * serverOperation = addPathContext->ServerOperation;
        AwaServerObservation * observation = (AwaServerObservation *)value;

        TreeNode resultNode;
        if ((addPathContext->Result = ServerOperation_AddPath(serverOperation, observation->ClientID, observation->Path, &resultNode)) == AwaError_Success)
        {
            if (ServerObserve_AddObserveType(resultNode, observation) == InternalError_Success)
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

static void ServerObserve_PerformSuccessfulCallback(const char * path, void * value, void * context)
{
    AwaServerObservation * observation = (AwaServerObservation *)value;
    PerformSuccessfulCallbackContext * successContext = (PerformSuccessfulCallbackContext *)context;

    const AwaServerObserveResponse * response = AwaServerObserveOperation_GetResponse(successContext->Operation, observation->ClientID);
    const AwaPathResult * result = AwaServerObserveResponse_GetPathResult(response, observation->Path);
    if (!observation->Cancel)
    {
        if (AwaPathResult_GetError(result) == AwaError_Success)
        {
            Map_Put(ServerSession_GetObservers(successContext->Session), observation->Path, observation);

            // map the observation to this session.
            observation->Session = successContext->Session;
        }
        else
        {
            LogErrorWithEnum(AwaPathResult_GetError(result), "Failed to observe to path %s", observation->Path);
            successContext->Result = AwaError_Response;
        }

    }
}

AwaError AwaServerObserveOperation_Perform(AwaServerObserveOperation * operation, AwaTimeout timeout)
{
    AwaError result = AwaError_Unspecified;

    if (timeout >= 0)
    {
        if (operation != NULL)
        {
            PerformAddPathCallbackContext addPathContext;
            addPathContext.ServerOperation = operation->ServerOperation;
            addPathContext.Result = AwaError_Success;
            Map_ForEach(operation->Observers, ServerObserve_PerformAddPathCallback, (void *)&addPathContext);
            result = addPathContext.Result;

            if (result != AwaError_Success)
            {
                goto error;
            }

            const AwaServerSession * session = ServerOperation_GetSession(operation->ServerOperation);
            if (ServerSession_IsConnected(session))
            {
                TreeNode clientsTree = ServerOperation_GetClientsTree(operation->ServerOperation);
                if (clientsTree != NULL)
                {
                    if (TreeNode_GetChildCount(clientsTree) > 0)
                    {
                        // build an IPC message and inject our content (object paths) into it
                        IPCMessage * observeRequest = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_REQUEST, IPC_MESSAGE_SUB_TYPE_OBSERVE, ServerOperation_GetSessionID(operation->ServerOperation));
                        IPCMessage_AddContent(observeRequest, clientsTree);

                        IPCMessage * observeResponse = NULL;
                        result = IPC_SendAndReceive(ServerSession_GetChannel(session), observeRequest, &observeResponse, timeout);

                        if (result == AwaError_Success)
                        {
                            IPCResponseCode responseCode = IPCMessage_GetResponseCode(observeResponse);
                            if (responseCode == IPCResponseCode_Success)
                            {
                                // Free an old response if it exists
                                if (operation->Response != NULL)
                                {
                                    ServerResponse_Free(&operation->Response);
                                }
                                // Detach the response content and store it in the operation's ServerGetResponse
                                TreeNode contentNode = IPCMessage_GetContentNode(observeResponse);
                                TreeNode clientsNode = Xml_Find(contentNode, "Clients");
                                operation->Response = ServerResponse_NewFromServerOperation(operation->ServerOperation, clientsNode);

                                if (operation->Response)
                                {
                                    PerformSuccessfulCallbackContext successContext;
                                    successContext.Session = (AwaServerSession*)ServerOperation_GetSession(operation->ServerOperation);
                                    successContext.Result = AwaError_Success;
                                    successContext.Operation = operation;
                                    Map_ForEach(operation->Observers, ServerObserve_PerformSuccessfulCallback, (void *)&successContext);

                                    result = successContext.Result;
                                    LogDebug("Perform Observe Operation finished %s", result == AwaError_Success? "successfully" : "with errors");
                                }
                                else
                                {
                                    result = LogErrorWithEnum(AwaError_Internal, "An internal error occurred when parsing Response");
                                }
                            }
                            else
                            {
                                result = LogErrorWithEnum(AwaError_IPCError, "Unexpected IPC response code: %d", responseCode);
                            }
                        }

                        IPCMessage_Free(&observeRequest);
                        IPCMessage_Free(&observeResponse);
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

static AwaError ServerObserveOperation_Add(AwaServerObserveOperation * operation, AwaServerObservation * observation, bool cancel)
{
    AwaError result = AwaError_Unspecified;

    if (operation != NULL)
    {
        if (observation != NULL)
        {
            AwaServerObservation * existingObservation = NULL;
            Map_Get(operation->Observers, observation->Path, (void **)&existingObservation);
            if (existingObservation == NULL)
            {
                if (Map_Put(operation->Observers, observation->Path, observation))
                {
                    List_Add(observation->Operations, operation);
                    observation->Cancel = cancel;
                    result = AwaError_Success;
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_Internal, "Failed to add observation to operation");
                }
            }
            else
            {
                result = LogErrorWithEnum(AwaError_OperationInvalid, "A observation already exists for path %s in the given operation.", observation->Path);
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_ObservationInvalid, "Observation is NULL");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "Operation is NULL");
    }
    return result;
}

AwaError AwaServerObserveOperation_AddObservation(AwaServerObserveOperation * operation, AwaServerObservation * observation)
{
    return ServerObserveOperation_Add(operation, observation, false);
}

AwaError AwaServerObserveOperation_AddCancelObservation(AwaServerObserveOperation * operation, AwaServerObservation * observation)
{
    return ServerObserveOperation_Add(operation, observation, true);
}

const AwaServerObserveResponse * AwaServerObserveOperation_GetResponse(const AwaServerObserveOperation * operation, const char * clientID)
{
    const ResponseCommon * response = NULL;
    if (operation != NULL)
    {
        response = ServerResponse_GetClientResponse(operation->Response, clientID);
    }
    // AwaServerObserveResponse is an alias for ResponseCommon
    return (const AwaServerObserveResponse *)response;
}

AwaPathIterator * AwaServerObserveResponse_NewPathIterator(const AwaServerObserveResponse * response)
{
    // AwaServerObserveResponse is an alias for ResponseCommon
    // AwaPathIterator is an alias for PathIterator
    return (AwaPathIterator *)ResponseCommon_NewPathIterator((const ResponseCommon *)response);
}

const AwaPathResult * AwaServerObserveResponse_GetPathResult(const AwaServerObserveResponse * response, const char * path)
{
    // AwaServerObserveResponse is an alias for ResponseCommon
    const PathResult * pathResult = NULL;
    ResponseCommon_GetPathResult((const ResponseCommon *)response, path, &pathResult);
    // AwaPathResult is an alias for PathResult
    return (AwaPathResult *)pathResult;
}

AwaError ServerObserveOperation_CallObservers(AwaServerSession * session, TreeNode clientsNode)
{
    AwaError result = AwaError_Unspecified;

    uint32_t clientIndex = 0;
    TreeNode client = NULL;
    while ((client = Xml_FindFrom(clientsNode, "Client", &clientIndex)) != NULL)
    {
        const char * clientID = ServerOperation_GetClientIDFromClientNode(client);
        TreeNode objectsTree = Xml_Find(client, "Objects");

        AwaChangeSet * changeSet = ChangeSet_NewWithClientID(session, SessionType_Server, objectsTree, clientID);

        result = SubscribeObserveCommon_CallObservers(ServerSession_GetObservers(session), changeSet, ServerObserverList_InvokeObserveCallback);
        ChangeSet_Free(&changeSet);
        if (result != AwaError_Success)
        {
            break;
        }
    }
    return result;
}

ServerOperation * ServerObserveOperation_GetServerOperation(const AwaServerObserveOperation * operation)
{
    return (operation != NULL) ? operation->ServerOperation : NULL;
}

MapType * ServerObserveOperation_GetObservers(const AwaServerObserveOperation * operation)
{
    return (operation != NULL) ? operation->Observers : NULL;
}

ListType * ServerObservation_GetOperations(AwaServerObservation * observation)
{
    ListType * operations = NULL;
    if (observation != NULL)
    {
        operations = observation->Operations;
    }
    return operations;
}

void ServerObservation_RemoveSession(AwaServerObservation * observation)
{
    if (observation != NULL)
    {
        observation->Session = NULL;
    }
}


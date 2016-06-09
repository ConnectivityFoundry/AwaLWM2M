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
#include <inttypes.h>

#include "awa/server.h"
#include "server_session.h"
#include "memalloc.h"
#include "log.h"
#include "xml.h"
#include "path.h"
#include "operation_common.h"
#include "lwm2m_xml_interface.h"
#include "utils.h"
#include "arrays.h"
#include "set_write_common.h"
#include "server_response.h"
#include "client_iterator.h"
#include "response_common.h"
#include "write_mode.h"

// The server write operation does NOT use a ServerOperation due to being able to only write to one client at a time.
// ClientID is specified at the time the operation is performed, rather than per-path. This is due to Write being a
// more heavyweight operation, and it is more efficient to build a single operation and send it to client one at a time,
// rather than a single operation that contains paths to multiple clients potentially containing a much larger payload. 
struct _AwaServerWriteOperation
{
    ServerOperation * ServerOperation;
    AwaWriteMode DefaultWriteMode;
    AwaWriteMode ObjectInstanceWriteMode;
    AwaWriteMode ResourceInstancesWriteMode;

    ServerResponse * Response;
};

// This struct is used for API type safety and is never instantiated.
// DO NOT USE THIS STRUCTURE!
// AwaServerWriteResponse is an alias for ResponseCommon
struct _AwaServerWriteResponse {};


AwaServerWriteOperation * AwaServerWriteOperation_New(const AwaServerSession * session, AwaWriteMode defaultMode)
{
    AwaServerWriteOperation * operation = NULL;

    if (session != NULL)
    {
        if (ServerSession_IsConnected(session) != false)
        {
            if (IsWriteModeValid(defaultMode))
            {
                operation = Awa_MemAlloc(sizeof(*operation));
                if (operation != NULL)
                {
                    memset(operation, 0, sizeof(*operation));
                    operation->ServerOperation = ServerOperation_New(session);
                    if (operation->ServerOperation != NULL)
                    {
                        operation->DefaultWriteMode = defaultMode;
                        LogNew("AwaServerWriteOperation", operation);
                    }
                    else
                    {
                        LogErrorWithEnum(AwaError_Internal, "Unable to initialise operation");
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
                LogErrorWithEnum(AwaError_OperationInvalid, "Invalid default AwaWriteMode: %d", defaultMode);
            }
        }
        else
        {
            LogErrorWithEnum(AwaError_SessionInvalid);

        }
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid, "Session is NULL");
    }

    return operation;
}

AwaError AwaServerWriteOperation_Free(AwaServerWriteOperation ** operation)
{
    AwaError result = AwaError_OperationInvalid;
    if ((operation != NULL) && (*operation != NULL))
    {
        ServerOperation_Free(&(*operation)->ServerOperation);
        ServerResponse_Free(&(*operation)->Response);
        LogFree("AwaServerWriteOperation", *operation);
        Awa_MemSafeFree(*operation);
        *operation = NULL;
        result = AwaError_Success;
    }
    return result;
}

AwaError AwaServerWriteOperation_Perform(AwaServerWriteOperation * operation, const char * clientID, AwaTimeout timeout)
{
    AwaError result = AwaError_Unspecified;

    if (timeout >= 0)
    {
        if (operation != NULL)
        {
            const Session * session = ServerOperation_GetSession(operation->ServerOperation);
            if (session != NULL)
            {
                const AwaServerSession * serverSession = (AwaServerSession * )session;
                if (ServerSession_IsConnected(serverSession))
                {
                    if (clientID != NULL)
                    {
                        OperationCommon * defaultClientOperation = ServerOperation_GetDefaultClientOperation(operation->ServerOperation);
                        if (defaultClientOperation != NULL)
                        {
                            TreeNode objectsTree = OperationCommon_GetObjectsTree(defaultClientOperation);

                            if (objectsTree != NULL)
                            {
                                if (TreeNode_GetChildCount(objectsTree) > 0)
                                {
                                    // build an IPC message and inject our content into it
                                    IPCMessage * request = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_REQUEST, IPC_MESSAGE_SUB_TYPE_WRITE, ServerOperation_GetSessionID(operation->ServerOperation));

                                    // Add client node
                                    TreeNode clientsNode = Xml_CreateNode("Clients");
                                    TreeNode clientNode = Xml_CreateNode("Client");
                                    TreeNode_AddChild(clientsNode, clientNode);

                                    // Add client ID
                                    TreeNode clientIDnode = Xml_CreateNodeWithValue("ID", "%s",clientID);
                                    TreeNode_AddChild(clientNode, clientIDnode);

                                    // Add default write mode
                                    TreeNode defaultWriteModeNode = Xml_CreateNodeWithValue("DefaultWriteMode", "%s", WriteMode_ToString(operation->DefaultWriteMode));
                                    TreeNode_AddChild(clientNode, defaultWriteModeNode);

                                    //Add objects tree
                                    TreeNode_AddChild(clientNode, objectsTree);

                                    // Add Content to message
                                    IPCMessage_AddContent(request, clientsNode);

                                    // Send via IPC
                                    IPCMessage * response = NULL;
                                    result = IPC_SendAndReceive(ServerSession_GetChannel(serverSession), request, &response, timeout);

                                    // Process the response
                                    if (result == AwaError_Success)
                                    {
                                        IPCResponseCode responseCode = IPCMessage_GetResponseCode(response);
                                        if (responseCode == IPCResponseCode_Success)
                                        {
                                            // Free an old Write response record if it exists
                                            if (operation->Response != NULL)
                                            {
                                                ServerResponse_Free(&operation->Response);
                                            }

                                            // Detach the response's content and add it to the Server Response
                                            TreeNode contentNode = IPCMessage_GetContentNode(response);
                                            TreeNode clientsNode = Xml_Find(contentNode, "Clients");
                                            operation->Response = ServerResponse_NewFromServerOperation(operation->ServerOperation, clientsNode);

                                            LogDebug("Perform Write Operation successful");

                                            result = ServerResponse_CheckForErrors(operation->Response);
                                        }
                                        else if (responseCode == IPCResponseCode_FailureBadRequest)
                                        {
                                            result = LogErrorWithEnum(AwaError_IPCError, "Unable to perform Write operation: Bad Request");
                                        }
                                        else
                                        {
                                            result = LogErrorWithEnum(AwaError_IPCError, "Unexpected IPC response code: %d", responseCode);
                                        }
                                    }
                                    // Free allocated memory
                                    Tree_DetachNode(objectsTree);
                                    Tree_Delete(clientsNode);

                                    IPCMessage_Free(&request);
                                    IPCMessage_Free(&response);
                                }
                                else
                                {
                                    result = LogErrorWithEnum(AwaError_OperationInvalid, "No paths specified");
                                }
                            }
                            else
                            {
                                result = LogErrorWithEnum(AwaError_Internal, "objectsTree is NULL");
                            }
                        }
                        else
                        {
                            result = LogErrorWithEnum(AwaError_Internal, "default client operation is NULL");
                        }
                    }
                    else
                    {
                        result = LogErrorWithEnum(AwaError_SessionNotConnected, "session is not connected");
                    }
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
                }
            }
            else
            {
                result = LogErrorWithEnum(AwaError_OperationInvalid, "ClientID is NULL");
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_OperationInvalid, "Operation is NULL");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "Invalid timeout specified");
    }
    return result;
}

// Given an object or object instance node, add a create tag. In the case only an object node is given,
// an object instance node without an ID will be created, marking that the new instance should have a generated ID.
InternalError ServerWriteOperation_AddCreate(TreeNode node)
{
    InternalError result = InternalError_Unspecified;

    if (node != NULL)
    {
        if (strcmp(TreeNode_GetName(node),"Object") == 0 || strcmp(TreeNode_GetName(node),"ObjectInstance") == 0)
        {
            if (TreeNode_AddChild(node, Xml_CreateNode(IPC_MESSAGE_TAG_CREATE)))
            {
                result = InternalError_Success;
            }
            else
            {
                LogError("Failed to add create tag to object instance node");
                result = InternalError_Tree;
            }
        }
        else
        {
            LogError("Unexpected input node: %s", TreeNode_GetName(node));
            result = InternalError_ParameterInvalid;
            goto error;
        }
    }
    else
    {
        LogError("node is NULL");
        result = InternalError_ParameterInvalid;
    }

error:
    return result;
}

AwaError AwaServerWriteOperation_CreateObjectInstance(AwaServerWriteOperation * operation, const char * path)
{
    AwaError result = AwaError_Unspecified;

    if (path != NULL)
    {
        AwaObjectID objectID;
        AwaResourceID resourceID;

        if (Path_IsValid(path))
        {
            objectID = Path_GetObjectID(path);
            resourceID = Path_GetResourceID(path);

            if ((objectID != AWA_INVALID_ID) && (resourceID == AWA_INVALID_ID))
            {
                if (operation != NULL)
                {
                    TreeNode resultNode = NULL;
                    TreeNode objectsTree = OperationCommon_GetObjectsTree(ServerOperation_GetDefaultClientOperation(operation->ServerOperation));
                    if ((result = OperationCommon_AddPathToObjectsTree(objectsTree, path, &resultNode)) == AwaError_Success && resultNode != NULL)
                    {
                        //NB: if object instance ID isn't specified in the path we have to check the response for the generated ID
                        if (ServerWriteOperation_AddCreate(resultNode) == InternalError_Success)
                        {
                            result = AwaError_Success;
                        }
                        else
                        {
                            result = LogErrorWithEnum(AwaError_Internal, "Failed to add value to path");
                        }
                    }
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_OperationInvalid, "Operation is NULL");
                }
            }
            else
            {
                result = LogErrorWithEnum(AwaError_PathInvalid,  "%s is not a valid object or object instance path", path);
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_PathInvalid,  "%s is not a valid path", path);
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_PathInvalid, "Path is NULL");
    }
    return result;

}

static AwaError ServerWriteOperation_AddValue(AwaServerWriteOperation * operation, const char * path, int resourceInstanceID, void * value, size_t size, AwaResourceType type)
{
    AwaError result = AwaError_Unspecified;
    if (operation != NULL)
    {
        result = SetWriteCommon_AddValue(ServerOperation_GetDefaultClientOperation(operation->ServerOperation), SessionType_Server, path, resourceInstanceID, value, size, type, SetArrayMode_Unspecified);
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "operation is NULL");
    }
    return result;
}

static AwaError ServerWriteOperation_AddValues(AwaServerWriteOperation * operation, const char * path, const AwaArray * array, AwaResourceType type)
{
    AwaError result = AwaError_Unspecified;

    AwaArrayIterator * iterator = ArrayIterator_New(array);

    if (iterator)
    {
        while (ArrayIterator_Next(iterator))
        {
            AwaArrayIndex index = ArrayIterator_GetIndex(iterator);
            void * value = ArrayIterator_GetValue(iterator);
            size_t length = ArrayIterator_GetValueLength(iterator);

            if (type == AwaResourceType_OpaqueArray)
            {
                AwaOpaque * opaque = (AwaOpaque *)value;
                length = opaque->Size;
                value = opaque->Data;
            }

            LogDebug("Adding value %p", value);

            if (value)
            {
                result = ServerWriteOperation_AddValue(operation, path, index, value, length, type);
                if (result != AwaError_Success)
                {
                    LogError("Unable to add array resource Instance %zu", index);
                    break;
                }
            }
            else
            {
                result = LogErrorWithEnum(AwaError_Internal , "Problem with array");
                break;
            }
        }

        ArrayIterator_Free(&iterator);
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "Operation is NULL");
    }

    return result;
}


AwaError AwaServerWriteOperation_AddValueAsCString(AwaServerWriteOperation * operation, const char * path, const char * value)
{
    AwaError result = AwaError_Unspecified;
    if (value != NULL)
    {
        result = ServerWriteOperation_AddValue(operation, path, 0, (void *)value, strlen(value), AwaResourceType_String);
    }
    else
    {
        result = LogErrorWithEnum(AwaError_TypeMismatch, "value is NULL");
    }
    return result;
}

AwaError AwaServerWriteOperation_AddValueAsStringArray(AwaServerWriteOperation * operation, const char * path, const AwaStringArray * array)
{
    return ServerWriteOperation_AddValues(operation, path, (const AwaArray *)array, AwaResourceType_StringArray);
}

AwaError AwaServerWriteOperation_AddValueAsInteger(AwaServerWriteOperation * operation, const char * path, AwaInteger value)
{
    return ServerWriteOperation_AddValue(operation, path, 0, (void *)&value, sizeof(AwaInteger), AwaResourceType_Integer);
}

AwaError AwaServerWriteOperation_AddValueAsIntegerArray(AwaServerWriteOperation * operation, const char * path, const AwaIntegerArray * array)
{
    return ServerWriteOperation_AddValues(operation, path, (const AwaArray *)array, AwaResourceType_IntegerArray);
}

AwaError AwaServerWriteOperation_AddValueAsFloat(AwaServerWriteOperation * operation, const char * path, AwaFloat value)
{
    return ServerWriteOperation_AddValue(operation, path, 0, (void *)&value, sizeof(AwaFloat), AwaResourceType_Float);
}

AwaError AwaServerWriteOperation_AddValueAsFloatArray(AwaServerWriteOperation * operation, const char * path, const AwaFloatArray * array)
{
    return ServerWriteOperation_AddValues(operation, path, (const AwaArray *)array, AwaResourceType_FloatArray);
}

AwaError AwaServerWriteOperation_AddValueAsBoolean(AwaServerWriteOperation * operation, const char * path, AwaBoolean value)
{
    return ServerWriteOperation_AddValue(operation, path, 0, (void *)&value, sizeof(AwaBoolean), AwaResourceType_Boolean);
}

AwaError AwaServerWriteOperation_AddValueAsBooleanArray(AwaServerWriteOperation * operation, const char * path, const AwaBooleanArray * array)
{
    return ServerWriteOperation_AddValues(operation, path, (const AwaArray *)array, AwaResourceType_BooleanArray);
}

AwaError AwaServerWriteOperation_AddValueAsTime(AwaServerWriteOperation * operation, const char * path, AwaTime value)
{
    return ServerWriteOperation_AddValue(operation, path, 0, (void *)&value, sizeof(AwaTime), AwaResourceType_Time);
}

AwaError AwaServerWriteOperation_AddValueAsTimeArray(AwaServerWriteOperation * operation, const char * path, const AwaTimeArray * array)
{
    return ServerWriteOperation_AddValues(operation, path, (const AwaArray *)array, AwaResourceType_TimeArray);
}

AwaError AwaServerWriteOperation_AddValueAsOpaque(AwaServerWriteOperation * operation, const char * path, AwaOpaque value)
{
    return ServerWriteOperation_AddValue(operation, path, 0, value.Data, value.Size, AwaResourceType_Opaque);
}

AwaError AwaServerWriteOperation_AddValueAsOpaqueArray(AwaServerWriteOperation * operation, const char * path, const AwaOpaqueArray * array)
{
    return ServerWriteOperation_AddValues(operation, path, (const AwaArray *)array, AwaResourceType_OpaqueArray);
}

AwaError AwaServerWriteOperation_AddValueAsObjectLink(AwaServerWriteOperation * operation, const char * path, AwaObjectLink value)
{
    return ServerWriteOperation_AddValue(operation, path, 0, (void *)&value, sizeof(AwaObjectLink), AwaResourceType_ObjectLink);
}

AwaError AwaServerWriteOperation_AddValueAsObjectLinkArray(AwaServerWriteOperation * operation, const char * path, const AwaObjectLinkArray * array)
{
    return ServerWriteOperation_AddValues(operation, path, (const AwaArray *)array, AwaResourceType_ObjectLinkArray);
}

AwaClientIterator * AwaServerWriteOperation_NewClientIterator(const AwaServerWriteOperation * operation)
{
    AwaClientIterator * iterator = NULL;
    if (operation != NULL)
    {
        iterator = (AwaClientIterator *)ServerResponse_NewClientIterator(operation->Response);
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "operation is NULL");
    }
    return iterator;
}

const AwaServerWriteResponse * AwaServerWriteOperation_GetResponse(const AwaServerWriteOperation * operation, const char * clientID)
{
    const ResponseCommon * response = NULL;
    if (operation != NULL)
    {
        response = ServerResponse_GetClientResponse(operation->Response, clientID);
    }
    // AwaServerWriteResponse is an alias for ResponseCommon
    return (const AwaServerWriteResponse *)response;
}

AwaPathIterator * AwaServerWriteResponse_NewPathIterator(const AwaServerWriteResponse * response)
{
    // AwaServerWriteResponse is an alias for ResponseCommon
    // AwaPathIterator is an alias for PathIterator
    return (AwaPathIterator *)ResponseCommon_NewPathIterator((const ResponseCommon *)response);
}

const AwaPathResult * AwaServerWriteResponse_GetPathResult(const AwaServerWriteResponse * response, const char * path)
{
    // AwaServerWriteResponse is an alias for ResponseCommon
    const PathResult * pathResult = NULL;
    ResponseCommon_GetPathResult((const ResponseCommon *)response, path, &pathResult);
    // AwaPathResult is an alias for PathResult
    return (AwaPathResult *)pathResult;
}


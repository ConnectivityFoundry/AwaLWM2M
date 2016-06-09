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
#include "lwm2m_xml_interface.h"

#include "utils.h"
#include "arrays.h"
#include "server_response.h"
#include "server_operation.h"
#include "client_iterator.h"
#include "objects_tree.h"
#include "set_write_common.h"

struct _AwaServerWriteAttributesOperation
{
    ServerOperation * ServerOperation;
    ServerResponse * Response;
};

// This struct is used for API type safety and is never instantiated.
// DO NOT USE THIS STRUCTURE!
struct _AwaServerWriteAttributesResponse {};

AwaServerWriteAttributesOperation * AwaServerWriteAttributesOperation_New(const AwaServerSession * session)
{
    AwaServerWriteAttributesOperation * operation = NULL;

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
                    LogNew("AwaServerWriteAttributesOperation", operation);
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
            LogErrorWithEnum(AwaError_SessionInvalid);

        }
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid, "Session is NULL");
    }

    return operation;
}

AwaError AwaServerWriteAttributesOperation_Free(AwaServerWriteAttributesOperation ** operation)
{
    AwaError result = AwaError_OperationInvalid;
    if ((operation != NULL) && (*operation != NULL))
    {
        ServerOperation_Free(&(*operation)->ServerOperation);
        ServerResponse_Free(&(*operation)->Response);
        LogFree("AwaServerWriteAttributesOperation", *operation);
        Awa_MemSafeFree(*operation);
        *operation = NULL;
        result = AwaError_Success;
    }
    return result;
}

static AwaError ServerWriteAttributesOperation_AddAttribute(AwaServerWriteAttributesOperation * operation, const char * clientID, const char * path, const char * link, void * value, int valueSize, AwaResourceType resourceType)
{
    AwaError result = AwaError_Unspecified;

    if (operation != NULL)
    {
        if (clientID != NULL)
        {
            if (path != NULL)
            {
                if (Path_IsValid(path))
                {
                    const AwaServerSession * session = ServerOperation_GetSession(operation->ServerOperation);

                    if (link != NULL)
                    {
                        if (session != NULL)
                        {
                            if (ServerSession_IsConnected(session))
                            {
                                AwaObjectID objectID = Path_GetObjectID(path);
                                AwaResourceID resourceID = Path_GetResourceID(path);
                                const AwaObjectDefinition * objectDefinition = AwaServerSession_GetObjectDefinition(session, objectID);
                                const AwaResourceDefinition * resourceDefinition = ServerSession_GetResourceDefinitionFromPath((AwaServerSession*)session, path);

                                if (objectDefinition != NULL && (resourceDefinition != NULL || resourceID == -1))
                                {
                                    TreeNode resultNode = NULL;
                                    if ((result = ServerOperation_AddPath(operation->ServerOperation, clientID, path, &resultNode)) == AwaError_Success)
                                    {
                                        if (resultNode != NULL)
                                        {
                                            TreeNode attributeNode = Xml_CreateNode("Attribute");

                                            TreeNode linkNode = Xml_CreateNodeWithValue("Link", "%s", link);
                                            TreeNode valueTypeNode = Xml_CreateNodeWithValue("ValueType", "%s", Lwm2mCore_ResourceTypeToString(Utils_GetPrimativeResourceType(resourceType)));

                                            char * encodedValue = xmlif_EncodeValue(Utils_GetPrimativeResourceType(resourceType), value, valueSize);
                                            TreeNode valueNode = Xml_CreateNodeWithValue("Value", "%s", encodedValue);
                                            Awa_MemSafeFree(encodedValue);

                                            TreeNode_AddChild(attributeNode, linkNode);
                                            TreeNode_AddChild(attributeNode, valueNode);
                                            TreeNode_AddChild(attributeNode, valueTypeNode);

                                            if (TreeNode_AddChild(resultNode, attributeNode))
                                            {
                                                result = AwaError_Success;
                                            }
                                            else
                                            {
                                                LogError("Failed to add value to resource node");
                                                result = AwaError_Internal;
                                            }
                                        }
                                        else
                                        {
                                            result = LogErrorWithEnum(AwaError_Internal, "AddPath failed to return result node");
                                        }
                                    }
                                }
                                else
                                {
                                    result = AwaError_NotDefined; // already logged no need to call LogErrorWithEnum
                                }
                            }
                            else
                            {
                                result = LogErrorWithEnum(AwaError_SessionNotConnected);
                            }
                        }
                        else
                        {
                            result = LogErrorWithEnum(AwaError_SessionInvalid, "Session is NULL");
                        }
                    }
                    else
                    {
                        result = LogErrorWithEnum(AwaError_TypeMismatch, "Link is NULL");
                    }
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_PathInvalid, "%s is not a resource path", path);
                }
            }
            else
            {
                result = LogErrorWithEnum(AwaError_PathInvalid, "Path is NULL");
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_TypeMismatch, "ClientID is NULL");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "Operation is NULL");
    }
    return result;
}

AwaError AwaServerWriteAttributesOperation_AddAttributeAsInteger(AwaServerWriteAttributesOperation * operation, const char * clientID, const char * path, const char * link, AwaInteger value)
{
    return ServerWriteAttributesOperation_AddAttribute(operation, clientID, path, link, (void *)&value, sizeof(AwaInteger), AwaResourceType_Integer);
}

AwaError AwaServerWriteAttributesOperation_AddAttributeAsFloat(AwaServerWriteAttributesOperation * operation, const char * clientID, const char * path, const char * link, AwaFloat value)
{
    return ServerWriteAttributesOperation_AddAttribute(operation, clientID, path, link, (void *)&value, sizeof(AwaFloat), AwaResourceType_Float);
}

AwaError AwaServerWriteAttributesOperation_Perform(AwaServerWriteAttributesOperation * operation, AwaTimeout timeout)
{
    AwaError result = AwaError_Unspecified;

    if (timeout >= 0)
    {
        if (operation != NULL)
        {
            const AwaServerSession * session = ServerOperation_GetSession(operation->ServerOperation);
            if (session != NULL)
            {
                if (ServerSession_IsConnected(session))
                {
                    TreeNode clientsTree = ServerOperation_GetClientsTree(operation->ServerOperation);
                    if (clientsTree != NULL)
                    {
                        if (TreeNode_GetChildCount(clientsTree) > 0)
                        {
                            // build an IPC message and inject our content into it
                            IPCMessage * request = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_REQUEST, IPC_MESSAGE_SUB_TYPE_WRITE_ATTRIBUTES, ServerOperation_GetSessionID(operation->ServerOperation));
                            IPCMessage_AddContent(request, clientsTree);

                            // Send via IPC
                            IPCMessage * response = NULL;
                            result = IPC_SendAndReceive(ServerSession_GetChannel(session), request, &response, timeout);

                            // Process the response
                            if (result == AwaError_Success)
                            {
                                IPCResponseCode responseCode = IPCMessage_GetResponseCode(response);
                                if (responseCode == IPCResponseCode_Success)
                                {
                                    // Free an old WriteAttributes response record if it exists
                                    if (operation->Response != NULL)
                                    {
                                        ServerResponse_Free(&operation->Response);
                                    }

                                    // Detach the response's content and add it to the Server Response
                                    TreeNode contentNode = IPCMessage_GetContentNode(response);
                                    TreeNode clientsNode = Xml_Find(contentNode, "Clients");
                                    operation->Response = ServerResponse_NewFromServerOperation(operation->ServerOperation, clientsNode);

                                    LogDebug("Perform WriteAttributes Operation successful");

                                    result = ServerResponse_CheckForErrors(operation->Response);
                                }
                                else if (responseCode == IPCResponseCode_FailureBadRequest)
                                {
                                    result = LogErrorWithEnum(AwaError_IPCError, "Unable to perform WriteAttributes operation: Bad Request");
                                }
                                else
                                {
                                    result = LogErrorWithEnum(AwaError_IPCError, "Unexpected IPC response code: %d", responseCode);
                                }
                            }
                            // Free allocated memory
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
            result = LogErrorWithEnum(AwaError_OperationInvalid, "Operation is NULL");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "Invalid timeout specified");
    }
    return result;
}

AwaClientIterator * AwaServerWriteAttributesOperation_NewClientIterator(const AwaServerWriteAttributesOperation * operation)
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

const AwaServerWriteAttributesResponse * AwaServerWriteAttributesOperation_GetResponse(const AwaServerWriteAttributesOperation * operation, const char * clientID)
{
    const ResponseCommon * response = NULL;
    if (operation != NULL)
    {
        response = ServerResponse_GetClientResponse(operation->Response, clientID);
    }
    // AwaServerWriteAttributesResponse is an alias for ResponseCommon
    return (const AwaServerWriteAttributesResponse *)response;
}

bool AwaServerWriteAttributesResponse_ContainsPath(const AwaServerWriteAttributesResponse * response, const char * path)
{
    // AwaServerWriteAttributesResponse is an alias for ResponseCommon
    return ResponseCommon_ContainsPath((const ResponseCommon *)response, path);
}

AwaPathIterator * AwaServerWriteAttributesResponse_NewPathIterator(const AwaServerWriteAttributesResponse * response)
{
    // AwaServerWriteAttributesResponse is an alias for ResponseCommon
    // AwaPathIterator is an alias for PathIterator
    return (AwaPathIterator *)ResponseCommon_NewPathIterator((const ResponseCommon *)response);
}

const AwaPathResult * AwaServerWriteAttributesResponse_GetPathResult(const AwaServerWriteAttributesResponse * response, const char * path)
{
    // AwaServerWriteAttributesResponse is an alias for ResponseCommon
    const PathResult * pathResult = NULL;
    ResponseCommon_GetPathResult((const ResponseCommon *)response, path, &pathResult);
    // AwaPathResult is an alias for PathResult
    return (AwaPathResult *)pathResult;
}


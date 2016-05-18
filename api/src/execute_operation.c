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
#include "set_write_common.h"

struct _AwaServerExecuteOperation
{
    ServerOperation * ServerOperation;
    ServerResponse * Response;
};

// This struct is used for API type safety and is never instantiated.
// DO NOT USE THIS STRUCTURE!
struct _AwaServerExecuteResponse {};

AwaServerExecuteOperation * AwaServerExecuteOperation_New(const AwaServerSession * session)
{
    // AwaServerExecuteResponse is an alias for ResponseCommon
    AwaServerExecuteOperation * operation = NULL;

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
                    LogNew("AwaServerExecuteOperation", operation);
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

AwaError AwaServerExecuteOperation_Free(AwaServerExecuteOperation ** operation)
{
    AwaError result = AwaError_OperationInvalid;
    if ((operation != NULL) && (*operation != NULL))
    {
        ServerOperation_Free(&(*operation)->ServerOperation);
        ServerResponse_Free(&(*operation)->Response);
        LogFree("AwaServerExecuteOperation", *operation);
        Awa_MemSafeFree(*operation);
        *operation = NULL;
        result = AwaError_Success;
    }
    return result;
}

AwaError AwaServerExecuteOperation_AddPath(AwaServerExecuteOperation * operation, const char * clientID, const char * path, const AwaExecuteArguments * arguments)
{
    AwaError result = AwaError_Unspecified;

    if (operation != NULL)
    {
        if (Path_IsValidForResource(path))
        {
            TreeNode resultNode = NULL;
            if ((result = ServerOperation_AddPath(operation->ServerOperation, clientID, path, &resultNode)) == AwaError_Success)
            {
                if (resultNode != NULL)
                {
                    char * encodedValue = NULL;
                    if (arguments != NULL && arguments->Size > 0)
                    {
                        encodedValue = xmlif_EncodeValue(AwaResourceType_Opaque, arguments->Data, arguments->Size);
                    }

                    if (encodedValue == NULL || SetWriteCommon_SetResourceNodeValue(resultNode, encodedValue) == InternalError_Success)
                    {
                        result = AwaError_Success;
                    }
                    else
                    {
                        result = LogErrorWithEnum(AwaError_Internal, "Failed to set resource value");
                    }

                    if (encodedValue)
                    {
                        Awa_MemSafeFree(encodedValue);
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
            result = LogErrorWithEnum(AwaError_PathInvalid, "%s is not a valid path to an executable resource", path);
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "Operation is NULL");
    }
    return result;
}

AwaError AwaServerExecuteOperation_Perform(AwaServerExecuteOperation * operation, AwaTimeout timeout)
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
                            IPCMessage * request = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_REQUEST, IPC_MESSAGE_SUB_TYPE_EXECUTE, ServerOperation_GetSessionID(operation->ServerOperation));

                            // Add Content to message
                            IPCMessage_AddContent(request, clientsTree);

                            // Send via IPC
                            IPCMessage * response = NULL;
                            result = IPC_SendAndReceive(ServerSession_GetChannel(session), request, &response, timeout > 0 ? timeout : -1);

                            // Process the response
                            if (result == AwaError_Success)
                            {
                                IPCResponseCode responseCode = IPCMessage_GetResponseCode(response);
                                if (responseCode == IPCResponseCode_Success)
                                {
                                    // Free an old Execute response record if it exists
                                    if (operation->Response != NULL)
                                    {
                                        ServerResponse_Free(&operation->Response);
                                    }

                                    // Detach the response's content and add it to the Server Response
                                    TreeNode contentNode = IPCMessage_GetContentNode(response);
                                    TreeNode clientsNode = Xml_Find(contentNode, "Clients");
                                    operation->Response = ServerResponse_NewFromServerOperation(operation->ServerOperation, clientsNode);

                                    LogDebug("Perform Execute Operation successful");

                                    result = ServerResponse_CheckForErrors(operation->Response);
                                }
                                else if (responseCode == IPCResponseCode_FailureBadRequest)
                                {
                                    result = LogErrorWithEnum(AwaError_IPCError, "Unable to perform Execute operation: Bad Request");
                                }
                                else
                                {
                                    result = LogErrorWithEnum(AwaError_IPCError, "Unexpected IPC response code: %d", responseCode);
                                }
                            }

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

const AwaServerExecuteResponse * AwaServerExecuteOperation_GetResponse(const AwaServerExecuteOperation * operation, const char * clientID)
{
    const ResponseCommon * response = NULL;
    if (operation != NULL)
    {
        response = ServerResponse_GetClientResponse(operation->Response, clientID);
    }
    // AwaServerExecuteResponse is an alias for ResponseCommon
    return (const AwaServerExecuteResponse *)response;
}

bool AwaServerExecuteResponse_ContainsPath(const AwaServerExecuteResponse * response, const char * path)
{
    // AwaServerExecuteResponse is an alias for ResponseCommon
    return ResponseCommon_ContainsPath((const ResponseCommon *)response, path);
}

AwaClientIterator * AwaServerExecuteOperation_NewClientIterator(const AwaServerExecuteOperation * operation)
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

AwaPathIterator * AwaServerExecuteResponse_NewPathIterator(const AwaServerExecuteResponse * response)
{
    // AwaServerExecuteResponse is an alias for ResponseCommon
    // AwaPathIterator is an alias for PathIterator
    return (AwaPathIterator *)ResponseCommon_NewPathIterator((const ResponseCommon *)response);
}

const AwaPathResult * AwaServerExecuteResponse_GetPathResult(const AwaServerExecuteResponse * response, const char * path)
{
    // AwaServerExecuteResponse is an alias for ResponseCommon
    const PathResult * pathResult = NULL;
    ResponseCommon_GetPathResult((const ResponseCommon *)response, path, &pathResult);
    // AwaPathResult is an alias for PathResult
    return (AwaPathResult *)pathResult;
}





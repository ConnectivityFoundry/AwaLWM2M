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


#include "awa/server.h"
#include "memalloc.h"
#include "log.h"
#include "path.h"
#include "operation_common.h"
#include "server_response.h"
#include "server_operation.h"

struct _AwaServerDeleteOperation
{
    ServerOperation * ServerOperation;
    ServerResponse * Response;
};

AwaServerDeleteOperation * AwaServerDeleteOperation_New(const AwaServerSession * session)
{
    AwaServerDeleteOperation * operation = NULL;

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
                    LogNew("AwaServerDeleteOperation", operation);
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
            LogError("Session is not connected");
        }
    }
    else
    {
        LogError("Session is NULL");
    }
    return operation;
}

AwaError AwaServerDeleteOperation_AddPath(AwaServerDeleteOperation * operation, const char * clientID, const char * path)
{
    AwaError result = AwaError_Unspecified;

    if (operation != NULL)
    {
        result = ServerOperation_AddPath(operation->ServerOperation, clientID, path, NULL);
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "Operation is NULL");
    }
    return result;
}

AwaError AwaServerDeleteOperation_Perform(AwaServerDeleteOperation * operation, AwaTimeout timeout)
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
                            IPCMessage * request = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_REQUEST, IPC_MESSAGE_SUB_TYPE_DELETE, ServerOperation_GetSessionID(operation->ServerOperation));
                            IPCMessage_AddContent(request, clientsTree);

                            // Send via IPC
                            IPCMessage * response = NULL;
                            result = IPC_SendAndReceive(ServerSession_GetChannel(session), request, &response, (timeout > 0) ? timeout : -1);

                            // Process the response
                            if (result == AwaError_Success)
                            {
                                IPCResponseCode responseCode = IPCMessage_GetResponseCode(response);
                                if (responseCode == IPCResponseCode_Success)
                                {
                                    // Free an old Read response record if it exists
                                    if (operation->Response != NULL)
                                    {
                                        ServerResponse_Free(&operation->Response);
                                    }

                                    // Detach the response's content and add it to the Server Response
                                    TreeNode contentNode = IPCMessage_GetContentNode(response);
                                    TreeNode clientsNode = Xml_Find(contentNode, "Clients");
                                    operation->Response = ServerResponse_NewFromServerOperation(operation->ServerOperation, clientsNode);

                                    LogDebug("Perform Delete Operation successful");

                                    result = ServerResponse_CheckForErrors(operation->Response);
                                }
                                else if (responseCode == IPCResponseCode_FailureBadRequest)
                                {
                                    result = LogErrorWithEnum(AwaError_IPCError, "Unable to perform Delete operation: Bad Request");
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

AwaError AwaServerDeleteOperation_Free(AwaServerDeleteOperation ** operation)
{
    AwaError result = AwaError_OperationInvalid;
    if ((operation != NULL) && (*operation != NULL))
    {
        ServerOperation_Free(&(*operation)->ServerOperation);
        ServerResponse_Free(&(*operation)->Response);
        LogFree("AwaServerDeleteOperation", *operation);
        Awa_MemSafeFree(*operation);
        *operation = NULL;
        result = AwaError_Success;
    }
    return result;
}

AwaClientIterator * AwaServerDeleteOperation_NewClientIterator(const AwaServerDeleteOperation * operation)
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

const AwaServerDeleteResponse * AwaServerDeleteOperation_GetResponse(const AwaServerDeleteOperation * operation, const char * clientID)
{
    const ResponseCommon * response = NULL;
    if (operation != NULL)
    {
        response = ServerResponse_GetClientResponse(operation->Response, clientID);
    }
    // AwaServerDeleteResponse is an alias for ResponseCommon
    return (const AwaServerDeleteResponse *)response;
}

AwaPathIterator * AwaServerDeleteResponse_NewPathIterator(const AwaServerDeleteResponse * response)
{
    // AwaServerDeleteResponse is an alias for ResponseCommon
    // AwaPathIterator is an alias for PathIterator
    return (AwaPathIterator *)ResponseCommon_NewPathIterator((const ResponseCommon *)response);
}

const AwaPathResult * AwaServerDeleteResponse_GetPathResult(const AwaServerDeleteResponse * response, const char * path)
{
    // AwaServerDeleteResponse is an alias for ResponseCommon
    const PathResult * pathResult = NULL;
    ResponseCommon_GetPathResult((const ResponseCommon *)response, path, &pathResult);
    // AwaPathResult is an alias for PathResult
    return (AwaPathResult *)pathResult;
}


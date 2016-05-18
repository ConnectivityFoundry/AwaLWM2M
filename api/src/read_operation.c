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

struct _AwaServerReadOperation
{
    ServerOperation * ServerOperation;
    ServerResponse * Response;
};

// This struct is used for API type safety and is never instantiated.
// DO NOT USE THIS STRUCTURE!
struct _AwaServerReadResponse {};

// AwaServerReadResponse is an alias for ResponseCommon

AwaServerReadOperation * AwaServerReadOperation_New(const AwaServerSession * session)
{
    AwaServerReadOperation * operation = NULL;

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
                    LogNew("AwaServerReadOperation", operation);
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

AwaError AwaServerReadOperation_Free(AwaServerReadOperation ** operation)
{
    AwaError result = AwaError_OperationInvalid;
    if ((operation != NULL) && (*operation != NULL))
    {
        ServerOperation_Free(&(*operation)->ServerOperation);
        ServerResponse_Free(&(*operation)->Response);
        LogFree("AwaServerReadOperation", *operation);
        Awa_MemSafeFree(*operation);
        *operation = NULL;
        result = AwaError_Success;
    }
    return result;
}

AwaError AwaServerReadOperation_AddPath(AwaServerReadOperation * operation, const char * clientID, const char * path)
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

AwaError AwaServerReadOperation_Perform(AwaServerReadOperation * operation, AwaTimeout timeout)
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
                            IPCMessage * request = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_REQUEST, IPC_MESSAGE_SUB_TYPE_READ, ServerOperation_GetSessionID(operation->ServerOperation));
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

                                    LogDebug("Perform Read Operation successful");

                                    result = ServerResponse_CheckForErrors(operation->Response);
                                }
                                else if (responseCode == IPCResponseCode_FailureBadRequest)
                                {
                                    result = LogErrorWithEnum(AwaError_IPCError, "Unable to perform Read operation: Bad Request");
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

AwaClientIterator * AwaServerReadOperation_NewClientIterator(const AwaServerReadOperation * operation)
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

const AwaServerReadResponse * AwaServerReadOperation_GetResponse(const AwaServerReadOperation * operation, const char * clientID)
{
    const ResponseCommon * response = NULL;
    if (operation != NULL)
    {
        response = ServerResponse_GetClientResponse(operation->Response, clientID);
    }
    // AwaServerReadResponse is an alias for ResponseCommon
    return (const AwaServerReadResponse *)response;
}

AwaPathIterator * AwaServerReadResponse_NewPathIterator(const AwaServerReadResponse * response)
{
    // AwaServerReadResponse is an alias for ResponseCommon
    // AwaPathIterator is an alias for PathIterator
    return (AwaPathIterator *)ResponseCommon_NewPathIterator((const ResponseCommon *)response);
}

const AwaPathResult * AwaServerReadResponse_GetPathResult(const AwaServerReadResponse * response, const char * path)
{
    // AwaServerReadResponse is an alias for ResponseCommon
    const PathResult * pathResult = NULL;
    ResponseCommon_GetPathResult((const ResponseCommon *)response, path, &pathResult);
    // AwaPathResult is an alias for PathResult
    return (AwaPathResult *)pathResult;
}

bool AwaServerReadResponse_ContainsPath(const AwaServerReadResponse * response, const char * path)
{
    // AwaServerReadResponse is an alias for ResponseCommon
    return ResponseCommon_ContainsPath((const ResponseCommon *)response, path);
}

bool AwaServerReadResponse_HasValue(const AwaServerReadResponse * response, const char * path)
{
    // AwaServerReadResponse is an alias for ResponseCommon
    return ResponseCommon_HasValue((const ResponseCommon *)response, path);
}

AwaError AwaServerReadResponse_GetValueAsCStringPointer(const AwaServerReadResponse * response, const char * path, const char ** value)
{
    // AwaServerReadResponse is an alias for ResponseCommon
    return ResponseCommon_GetValuePointerWithNull((const ResponseCommon *)response, path, (const void **)value, NULL, AwaResourceType_String, -1);
}

AwaError AwaServerReadResponse_GetValueAsIntegerPointer(const AwaServerReadResponse * response, const char * path, const AwaInteger ** value)
{
    // AwaServerReadResponse is an alias for ResponseCommon
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)value, NULL, AwaResourceType_Integer, sizeof(AwaInteger));
}

AwaError AwaServerReadResponse_GetValueAsFloatPointer(const AwaServerReadResponse * response, const char * path, const AwaFloat ** value)
{
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)value, NULL, AwaResourceType_Float, sizeof(AwaFloat));
}

AwaError AwaServerReadResponse_GetValueAsBooleanPointer(const AwaServerReadResponse * response, const char * path, const AwaBoolean ** value)
{
    // AwaServerReadResponse is an alias for ResponseCommon
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)value, NULL, AwaResourceType_Boolean, sizeof(AwaBoolean));
}

AwaError AwaServerReadResponse_GetValueAsTimePointer(const AwaServerReadResponse * response, const char * path, const AwaTime ** value)
{
    // AwaServerReadResponse is an alias for ResponseCommon
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)value, NULL, AwaResourceType_Time, sizeof(AwaTime));
}

AwaError AwaServerReadResponse_GetValueAsOpaque(const AwaServerReadResponse * response, const char * path, AwaOpaque * value)
{
    // AwaServerReadResponse is an alias for ResponseCommon
    return ResponseCommon_GetValueAsOpaque((const ResponseCommon *)response, path, value);
}

AwaError AwaServerReadResponse_GetValueAsObjectLink(const AwaServerReadResponse * response, const char * path, AwaObjectLink * value)
{
    // AwaServerReadResponse is an alias for ResponseCommon
    return ResponseCommon_GetValueAsObjectLink((const ResponseCommon *)response, path, value);
}

AwaError AwaServerReadResponse_GetValueAsObjectLinkPointer(const AwaServerReadResponse * response, const char * path, const AwaObjectLink ** value)
{
    // AwaServerReadResponse is an alias for ResponseCommon
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)value, NULL, AwaResourceType_ObjectLink, sizeof(AwaObjectLink));
}

AwaError AwaServerReadResponse_GetValueAsOpaquePointer(const AwaServerReadResponse * response, const char * path, const AwaOpaque ** value)
{
    // AwaServerReadResponse is an alias for ResponseCommon
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)value, NULL, AwaResourceType_Opaque, sizeof(AwaOpaque));
}

AwaError AwaServerReadResponse_GetValuesAsStringArrayPointer(const AwaServerReadResponse * response, const char * path, const AwaStringArray ** valueArray)
{
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)valueArray, NULL, AwaResourceType_StringArray, sizeof(AwaStringArray *));
}

AwaError AwaServerReadResponse_GetValuesAsIntegerArrayPointer(const AwaServerReadResponse * response, const char * path, const AwaIntegerArray ** valueArray)
{
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)valueArray, NULL, AwaResourceType_IntegerArray, sizeof(AwaIntegerArray *));
}

AwaError AwaServerReadResponse_GetValuesAsFloatArrayPointer(const AwaServerReadResponse * response, const char * path, const AwaFloatArray ** valueArray)
{
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)valueArray, NULL, AwaResourceType_FloatArray, sizeof(AwaFloatArray *));
}

AwaError AwaServerReadResponse_GetValuesAsBooleanArrayPointer(const AwaServerReadResponse * response, const char * path, const AwaBooleanArray ** valueArray)
{
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)valueArray, NULL, AwaResourceType_BooleanArray, sizeof(AwaBooleanArray *));
}

AwaError AwaServerReadResponse_GetValuesAsTimeArrayPointer(const AwaServerReadResponse * response, const char * path, const AwaTimeArray ** valueArray)
{
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)valueArray, NULL, AwaResourceType_TimeArray, sizeof(AwaTimeArray *));
}

AwaError AwaServerReadResponse_GetValuesAsOpaqueArrayPointer(const AwaServerReadResponse * response, const char * path, const AwaOpaqueArray ** valueArray)
{
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)valueArray, NULL, AwaResourceType_OpaqueArray, sizeof(AwaOpaqueArray *));
}

AwaError AwaServerReadResponse_GetValuesAsObjectLinkArrayPointer(const AwaServerReadResponse * response, const char * path, const AwaObjectLinkArray ** valueArray)
{
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)valueArray, NULL, AwaResourceType_ObjectLinkArray, sizeof(AwaObjectLinkArray *));
}


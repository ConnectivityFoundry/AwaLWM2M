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

#include "lwm2m_result.h"
#include "lwm2m_tree_node.h"
#include "lwm2m_definition.h"

#include "get_operation.h"
#include "awa/client.h"
#include "client_session.h"
#include "memalloc.h"
#include "log.h"
#include "xml.h"
#include "path.h"
#include "ipc.h"
#include "operation_common.h"
#include "response_common.h"
#include "get_response.h"

struct _AwaClientGetOperation
{
    OperationCommon * Common;
    ResponseCommon * Response;
};

AwaClientGetOperation * AwaClientGetOperation_New(const AwaClientSession * session)
{
    AwaClientGetOperation * operation = NULL;

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
                    LogNew("AwaClientGetOperation", operation);
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

AwaError AwaClientGetOperation_Free(AwaClientGetOperation ** operation)
{
    AwaError result = AwaError_OperationInvalid;
    if ((operation != NULL) && (*operation != NULL))
    {
        OperationCommon_Free(&(*operation)->Common);

        if ((*operation)->Response != NULL)
        {
            ResponseCommon_Free(&(*operation)->Response);
        }

        Awa_MemSafeFree(*operation);
        *operation = NULL;
        LogFree("AwaClientGetOperation", *operation);
        result = AwaError_Success;
    }
    return result;
}

AwaError AwaClientGetOperation_AddPath(AwaClientGetOperation * operation, const char * path)
{
    AwaError result = AwaError_Unspecified;

    if (operation != NULL)
    {
        result = OperationCommon_AddPathV2(operation->Common, path, NULL);
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "Operation is NULL");
    }
    return result;
}

AwaError AwaClientGetOperation_AddPathWithArrayRange(AwaClientGetOperation * operation, const char * path, AwaArrayIndex startIndex, AwaArrayLength indexCount)
{
    AwaError result = AwaError_Unspecified;

    if (operation != NULL)
    {
        result = OperationCommon_AddPathWithArrayRange(operation->Common, path, startIndex, indexCount);
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "Operation is NULL");
    }
    return result;
}

AwaError AwaClientGetOperation_Perform(AwaClientGetOperation * operation, AwaTimeout timeout)
{
    AwaError result = AwaError_Unspecified;

    if (timeout >= 0)
    {
        if (operation != NULL)
        {
            const AwaClientSession * session = ClientGetOperation_GetSession(operation);
            if (session != NULL)
            {
                if (ClientSession_IsConnected(session))
                {
                    TreeNode objectsTree = OperationCommon_GetObjectsTree(operation->Common);
                    if (objectsTree != NULL)
                    {
                        if (TreeNode_GetChildCount(objectsTree) > 0)
                        {
                            // build an IPC message and inject our content (object paths) into it
                            IPCMessage * getRequest = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_REQUEST, IPC_MESSAGE_SUB_TYPE_GET, OperationCommon_GetSessionID(operation->Common));
                            IPCMessage_AddContent(getRequest, objectsTree);

                            IPCMessage * getResponse = NULL;
                            result = IPC_SendAndReceive(ClientSession_GetChannel(session), getRequest, &getResponse, timeout);

                            // Process the response
                            if (result == AwaError_Success)
                            {
                                // Free an old response if it exists
                                if (operation->Response != NULL)
                                {
                                    ResponseCommon_Free(&operation->Response);
                                }

                                // Detach the response content and store it in the operation's ClientGetResponse
                                TreeNode contentNode = IPCMessage_GetContentNode(getResponse);

                                TreeNode objectsNode = Xml_Find(contentNode, "Objects");
                                operation->Response = ResponseCommon_New(operation->Common, objectsNode);

                                result = ResponseCommon_CheckForErrors(operation->Response);
                                LogDebug("Perform Get Operation successful");
                            }

                            IPCMessage_Free(&getRequest);
                            IPCMessage_Free(&getResponse);
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
            result = LogErrorWithEnum(AwaError_OperationInvalid, "operation is NULL");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "Invalid timeout specified");
    }
    return result;
}

const AwaClientGetResponse * AwaClientGetOperation_GetResponse(const AwaClientGetOperation * operation)
{
    const ResponseCommon * response = operation != NULL? operation->Response : NULL;
    // AwaClientGetResponse is an alias for ResponseCommon
    return (const AwaClientGetResponse *)response;
}

const AwaClientSession * ClientGetOperation_GetSession(const AwaClientGetOperation * operation)
{
    return (operation != NULL) ? OperationCommon_GetSession(operation->Common, NULL) : NULL;
}

OperationCommon * ClientGetOperation_GetOperationCommon(const AwaClientGetOperation * operation)
{
    return (operation != NULL) ? operation->Common : NULL;
}


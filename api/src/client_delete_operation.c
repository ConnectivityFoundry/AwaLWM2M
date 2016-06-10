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


#include "awa/client.h"
#include "client_session.h"
#include "memalloc.h"
#include "log.h"
#include "path.h"
#include "operation_common.h"
#include "operation_common.h"
#include "response_common.h"

struct _AwaClientDeleteOperation
{
    OperationCommon * Common;
    ResponseCommon * Response;
};

AwaClientDeleteOperation * AwaClientDeleteOperation_New(const AwaClientSession * session)
{
    AwaClientDeleteOperation * operation = NULL;

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
                    LogNew("AwaClientDeleteOperation", operation);
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

AwaError AwaClientDeleteOperation_Free(AwaClientDeleteOperation ** operation)
{
    AwaError result = AwaError_OperationInvalid;
    if ((operation != NULL) && (*operation != NULL))
    {
        OperationCommon_Free(&(*operation)->Common);

        if ((*operation)->Response != NULL)
            ResponseCommon_Free(&(*operation)->Response);

        LogFree("AwaClientDeleteOperation", *operation);
        Awa_MemSafeFree(*operation);
        *operation = NULL;
        result = AwaError_Success;
    }
    return result;
}

AwaError AwaClientDeleteOperation_AddPath(AwaClientDeleteOperation * operation, const char * path)
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

AwaError AwaClientDeleteOperation_AddPathWithArrayRange(AwaClientDeleteOperation * operation, const char * path, AwaArrayIndex startIndex, AwaArrayLength indexCount)
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

AwaError AwaClientDeleteOperation_Perform(AwaClientDeleteOperation * operation, AwaTimeout timeout)
{
    AwaError result = AwaError_Unspecified;

    if (timeout >= 0)
    {
        if (operation != NULL)
        {
            TreeNode objectsTree = OperationCommon_GetObjectsTree(operation->Common);
            if (objectsTree != NULL)
            {
                if (TreeNode_GetChildCount(objectsTree) > 0)
                {
                    //build an IPC message and inject our content (object paths) into it
                    IPCMessage * request = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_REQUEST, IPC_MESSAGE_SUB_TYPE_DELETE, OperationCommon_GetSessionID(operation->Common));

                    IPCMessage_AddContent(request, objectsTree);

                    IPCMessage * response = NULL;
                    result = IPC_SendAndReceive(ClientSession_GetChannel(OperationCommon_GetSession(operation->Common, NULL)), request, &response, timeout);

                    if (result == AwaError_Success)
                    {
                        IPCResponseCode responseCode = IPCMessage_GetResponseCode(response);
                        if (responseCode == IPCResponseCode_Success)
                        {
                            // Free an old Read response record if it exists
                            if (operation->Response != NULL)
                            {
                                ResponseCommon_Free(&operation->Response);
                            }

                            TreeNode contentNode = IPCMessage_GetContentNode(response);
                            TreeNode objectsNode = Xml_Find(contentNode, "Objects");
                            operation->Response = ResponseCommon_New(operation->Common, objectsNode);

                            result = ResponseCommon_CheckForErrors(operation->Response);

                            LogDebug("Perform Delete Operation successful");
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
                result = LogErrorWithEnum(AwaError_Internal, "ObjectsTree is NULL");
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

const AwaClientDeleteResponse * AwaClientDeleteOperation_GetResponse(const AwaClientDeleteOperation * operation)
{
    const ResponseCommon * response = operation != NULL? operation->Response : NULL;
    // AwaClientDeleteResponse is an alias for ResponseCommon
    return (const AwaClientDeleteResponse *)response;
}

AwaPathIterator * AwaClientDeleteResponse_NewPathIterator(const AwaClientDeleteResponse * response)
{
    // AwaClientDeleteResponse is an alias for ResponseCommon
    // AwaPathIterator is an alias for PathIterator
    return (AwaPathIterator *)ResponseCommon_NewPathIterator((const ResponseCommon *)response);
}

const AwaPathResult * AwaClientDeleteResponse_GetPathResult(const AwaClientDeleteResponse * response, const char * path)
{
    // AwaServerDeleteResponse is an alias for ResponseCommon
    const PathResult * pathResult = NULL;
    ResponseCommon_GetPathResult((const ResponseCommon *)response, path, &pathResult);
    // AwaPathResult is an alias for PathResult
    return (AwaPathResult *)pathResult;
}

bool AwaClientDeleteResponse_ContainsPath(const AwaClientDeleteResponse * response, const char * path)
{
    // AwaServerDeleteResponse is an alias for ResponseCommon
    return ResponseCommon_ContainsPath((const ResponseCommon *)response, path);
}


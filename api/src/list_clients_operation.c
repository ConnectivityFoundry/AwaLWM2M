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
#include "operation_common.h"
#include "server_response.h"
#include "client_iterator.h"
#include "log.h"
#include "memalloc.h"
#include "path_iterator.h"
#include "registered_entity_iterator.h"
#include "map.h"
#include "objects_tree.h"

struct _AwaServerListClientsResponse
{
    const ResponseCommon * Response;
};

struct _AwaServerListClientsOperation
{
    ServerOperation * ServerOperation;
    ServerResponse * ServerResponse;
    MapType * ClientResponseMap;         // stores built-on-demand Response instances
};

AwaServerListClientsOperation * AwaServerListClientsOperation_New(const AwaServerSession * session)
{
    AwaServerListClientsOperation * operation = NULL;

    if (session != NULL)
    {
        if (ServerSession_IsConnected(session) != false)
        {
            operation = Awa_MemAlloc(sizeof(*operation));
            if (operation != NULL)
            {
                memset(operation, 0, sizeof(*operation));
                operation->ServerResponse = NULL;
                operation->ServerOperation = ServerOperation_New(session);
                if (operation->ServerOperation != NULL)
                {
                    operation->ClientResponseMap = Map_New();
                    LogNew("AwaServerListClientsOperation", operation);
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
            LogErrorWithEnum(AwaError_SessionNotConnected);
        }
    }
    else
    {
        LogError("Session is NULL");
    }
    return operation;
}

AwaError AwaServerListClientsOperation_Free(AwaServerListClientsOperation ** operation)
{
    AwaError result = AwaError_OperationInvalid;
    if ((operation != NULL) && (*operation != NULL))
    {
        ServerResponse_Free(&(*operation)->ServerResponse);
        ServerOperation_Free(&(*operation)->ServerOperation);

        Map_FreeValues((*operation)->ClientResponseMap);
        Map_Free(&((*operation)->ClientResponseMap));

        LogFree("AwaServerListClientsOperation", *operation);
        Awa_MemSafeFree(*operation);
        *operation = NULL;
        result = AwaError_Success;
    }
    return result;
}

AwaError AwaServerListClientsOperation_Perform(AwaServerListClientsOperation * operation, AwaTimeout timeout)
{
    AwaError result = AwaError_Unspecified;

    if (timeout >= 0)
    {
        if (operation != NULL)
        {
            // build an IPC message and inject our content (object paths) into it
            IPCMessage * request = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_REQUEST, IPC_MESSAGE_SUB_TYPE_LIST_CLIENTS, ServerOperation_GetSessionID(operation->ServerOperation));
            IPCMessage * response = NULL;

            // a timeout of 0 means an infinite timeout
            result = IPC_SendAndReceive(ServerSession_GetChannel(ServerOperation_GetSession(operation->ServerOperation)), request, &response, timeout);

            if (result == AwaError_Success)
            {
                IPCResponseCode responseCode = IPCMessage_GetResponseCode(response);
                if (responseCode == IPCResponseCode_Success)
                {
                    // Free an old Clients record if it exists
                    if (operation->ServerResponse != NULL)
                    {
                        ServerResponse_Free(&operation->ServerResponse);
                    }

                    // Detach the response's content and add it to the Server Response
                    TreeNode contentNode = IPCMessage_GetContentNode(response);
                    TreeNode clientsNode = Xml_Find(contentNode, "Clients");
                    operation->ServerResponse = ServerResponse_NewFromServerOperation(operation->ServerOperation, clientsNode);

                    // if there are any cached Responses, free them
                    Map_FreeValues(operation->ClientResponseMap);

                    LogDebug("Perform ListClients Operation successful");
                    result = AwaError_Success;
                }
                else if (responseCode == IPCResponseCode_FailureBadRequest)
                {
                    result = LogErrorWithEnum(AwaError_IPCError, "Unable to perform List Clients operation");
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
            result = LogErrorWithEnum(AwaError_OperationInvalid, "Operation is NULL");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "Invalid timeout specified");
    }
    return result;
}

AwaClientIterator * AwaServerListClientsOperation_NewClientIterator(const AwaServerListClientsOperation * operation)
{
    AwaClientIterator * iterator = NULL;
    if (operation != NULL)
    {
        iterator = (AwaClientIterator *)ServerResponse_NewClientIterator(operation->ServerResponse);
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "operation is NULL");
    }
    return iterator;
}

// TODO: is maintaining a separate ClientResponseMap unnecessary? Can the ServerResponse's own Map be used instead?
const AwaServerListClientsResponse * AwaServerListClientsOperation_GetResponse(const AwaServerListClientsOperation * operation, const char * clientID)
{
    AwaServerListClientsResponse * listClientsResponse = NULL;
    if (operation != NULL)
    {
        if (clientID != NULL)
        {
            if (operation->ServerResponse != NULL)
            {
                // check that client response exists:
                const ResponseCommon * clientResponse = ServerResponse_GetClientResponse(operation->ServerResponse, clientID);
                if (clientResponse != NULL)
                {
                    // look up existing Response in map, return it.
                    // if it doesn't exist, create new Response and add to map, return it.
                    Map_Get(operation->ClientResponseMap, clientID, (void *)&listClientsResponse);
                    if (listClientsResponse == NULL)
                    {
                        LogDebug("Create new AwaServerListClientsResponse");
                        listClientsResponse = Awa_MemAlloc(sizeof(*listClientsResponse));
                        if (listClientsResponse != NULL)
                        {
                            memset(listClientsResponse, 0, sizeof(*listClientsResponse));
                            listClientsResponse->Response = clientResponse;

                            // cache the listClientsResponse
                            if (Map_Put(operation->ClientResponseMap, clientID, (void *)listClientsResponse) == false)
                            {
                                // do not return the response if we can't retain it, as it will eventually leak
                                LogErrorWithEnum(AwaError_Internal, "Map put failed");
                                Awa_MemSafeFree(listClientsResponse);
                                listClientsResponse = NULL;
                            }
                        }
                        else
                        {
                            LogErrorWithEnum(AwaError_OutOfMemory);
                        }
                    }
                    else
                    {
                        LogDebug("Retrieved cached AwaServerListClientsResponse");
                    }
                }
                else
                {
                    LogErrorWithEnum(AwaError_ClientNotFound, "Client ID %s not found", clientID);
                }
            }
            else
            {
                LogErrorWithEnum(AwaError_ResponseInvalid, "operation response is NULL");
            }
        }
        else
        {
            LogErrorWithEnum(AwaError_ClientIDInvalid, "clientID is NULL");
        }
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "operation is NULL");
    }
    return listClientsResponse;
}

AwaRegisteredEntityIterator * AwaServerListClientsResponse_NewRegisteredEntityIterator(const AwaServerListClientsResponse * response)
{
    RegisteredEntityIterator * iterator = NULL;
    if (response != NULL)
    {
        iterator = RegisteredEntityIterator_New(response->Response);
    }
    else
    {
        LogErrorWithEnum(AwaError_ResponseInvalid, "response is NULL");
    }
    return (AwaRegisteredEntityIterator *)iterator;
}


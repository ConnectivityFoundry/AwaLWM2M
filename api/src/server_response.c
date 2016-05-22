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


#include "server_response.h"
#include "xmltree.h"
#include "xml.h"
#include "log.h"
#include "memalloc.h"
#include "map.h"
#include "lwm2m_xml_serdes.h"

// A ServerResponse handles the <Clients>, <Client> tags, creating a new
// ResponseCommon for each Client in the response.
struct _ServerResponse
{
    TreeNode Clients;
    MapType * ClientResponses;   // map of ClientIDs to ResponseCommon pointers
};

ServerResponse * ServerResponse_NewFromServerOperation(const ServerOperation * serverOperation, const TreeNode clientsNode)
{
    ServerResponse * response = NULL;
    if (serverOperation != NULL)
    {
        if (clientsNode != NULL)
        {
            if (strcmp("Clients", TreeNode_GetName(clientsNode)) == 0)
            {
                response = Awa_MemAlloc(sizeof(*response));
                if (response != NULL)
                {
                    memset(response, 0, sizeof(*response));

                    if (clientsNode != NULL)
                    {
                        // keep a copy of the Clients tree, and prune it
                        response->Clients = Tree_Copy(clientsNode);
                        response->ClientResponses = Map_New();

                        uint32_t index = 0;
                        TreeNode clientNode = NULL;
                        while ((clientNode = Xml_FindFrom(response->Clients, "Client", &index)) != NULL)
                        {
                            TreeNode objectsNode = Xml_Find(clientNode, "Objects");
                            if (objectsNode != NULL)
                            {
                                const char * clientID = (const char *)xmlif_GetOpaque(clientNode, "Client/ID");
                                OperationCommon * operation = ServerOperation_GetOperationCommon(serverOperation, clientID);
                                if (operation == NULL)
                                {
                                    // Write only uses default operation on send, but receives a clients response back
                                    // It will not have an OperationCommon for this ClientID, so we need to create it.
                                    operation = ServerOperation_CreateOperationCommon(serverOperation, clientID);
                                }

                                ResponseCommon * clientResponse = ResponseCommon_New(operation, objectsNode);
                                Map_Put(response->ClientResponses, clientID, clientResponse);
                                LogDebug("Map put %s:%p", clientID, clientResponse);

                                // detach and delete the Objects tree, we don't need it any more
                                Tree_DetachNode(objectsNode);
                                Tree_Delete(objectsNode);
                            }
                        }

                        LogNew("ServerResponse", response);
                    }
                    else
                    {
                        LogErrorWithEnum(AwaError_Internal, "No Clients child in contentNode");
                        Awa_MemSafeFree(response);
                        response = NULL;
                    }
                }
                else
                {
                    LogErrorWithEnum(AwaError_OutOfMemory);
                }
            }
            else
            {
                LogErrorWithEnum(AwaError_ResponseInvalid, "Expected <Clients> node");
            }
        }
        else
        {
            LogErrorWithEnum(AwaError_Internal, "clientsNode is NULL");
        }
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "serverOperation is NULL");
    }
    return response;
}

static void FreeResponseCommon(const char * clientID, void * response, void * context)
{
    if (response != NULL)
    {
        ResponseCommon * responseCommon = (ResponseCommon *)response;
        ResponseCommon_Free(&responseCommon);
    }
}

AwaError ServerResponse_Free(ServerResponse ** response)
{
    AwaError result = AwaError_Unspecified;
    if ((response != NULL) && (*response != NULL))
    {
        LogFree("ServerResponse", *response);
        Tree_Delete((*response)->Clients);
        Map_ForEach((*response)->ClientResponses, FreeResponseCommon, NULL);
        Map_Free(&((*response)->ClientResponses));
        Awa_MemSafeFree(*response);
        *response = NULL;
        result = AwaError_Success;
    }
    return result;
}

const char * ServerResponse_GetNextClientID(const ServerResponse * response, const char * previousClientID)
{
    const char * clientID = NULL;
    TreeNode previousClientNode = NULL;
    if (previousClientID == NULL)
    {
        // find the first "Client" child
        previousClientNode = Xml_Find(response->Clients, "Client");
        clientID = ServerOperation_GetClientIDFromClientNode(previousClientNode);
    }
    else
    {
        // find client node index corresponding to previousClientID
        uint32_t index = 0;
        bool found = false;
        while ((previousClientNode = TreeNode_GetChild(response->Clients, index)))
        {
            if (strcmp(previousClientID, xmlif_GetOpaque(previousClientNode, "Client/ID")) == 0)
            {
                found = true;
                break;
            }
            ++index;
        }

        if (found)
        {
            ++index;

            // use it to find the next sibling, if any
            TreeNode clientNode = Xml_FindFrom(response->Clients, "Client", &index);
            if (clientNode != NULL)
            {
                clientID = ServerOperation_GetClientIDFromClientNode(clientNode);
            }
        }
    }
    return clientID;
}

const ResponseCommon * ServerResponse_GetClientResponse(const ServerResponse * response, const char * clientID)
{
    const ResponseCommon * clientResponse = NULL;
    if (response != NULL)
    {
        Map_Get(response->ClientResponses, clientID, (void **)&clientResponse);
    }
    else
    {
        LogErrorWithEnum(AwaError_ResponseInvalid, "response is NULL");
    }
    return clientResponse;
}

AwaError ServerResponse_CheckForErrors(const ServerResponse * response)
{
    AwaError error = AwaError_Success;
    if (response != NULL)
    {
        // check each client for errors in its response
        const char * clientID = NULL;
        while ((clientID = ServerResponse_GetNextClientID(response, clientID)) != NULL)
        {
            const ResponseCommon * clientResponse = ServerResponse_GetClientResponse(response, clientID);
            if (ResponseCommon_CheckForErrors(clientResponse) != AwaError_Success)
            {
                error = AwaError_Response;
            }
        }
    }
    else
    {
        error = LogErrorWithEnum(AwaError_ResponseInvalid, "response is NULL");
    }
    return error;
}

ClientIterator * ServerResponse_NewClientIterator(const ServerResponse * response)
{
    ClientIterator * iterator = NULL;
    if (response != NULL)
    {
        iterator = ClientIterator_New();
        if (iterator != NULL)
        {
            const char * clientID = NULL;
            while ((clientID = ServerResponse_GetNextClientID(response, clientID)) != NULL)
            {
                LogDebug("Iterator add ClientID: %s", clientID);
                ClientIterator_Add(iterator, clientID);
            }
        }
        else
        {
            LogErrorWithEnum(AwaError_OutOfMemory);
        }
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "operation response is NULL");
    }

    return iterator;
}


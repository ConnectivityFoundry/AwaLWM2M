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


#include "lwm2m_result.h"
#include "lwm2m_xml_serdes.h"

#include "operation_common.h"
#include "error.h"
#include "path.h"
#include "xml.h"
#include "log.h"
#include "memalloc.h"
#include "server_session.h"
#include "server_operation.h"
#include "lwm2m_ipc.h"
#include "objects_tree.h"

struct _ServerOperation
{
    const AwaServerSession * Session;
    TreeNode Clients;
    MapType * OperationCommons;               // map of ClientIDs to OperationCommon pointers
    OperationCommon * DefaultClientOperation; // for operations that don't take ClientID until perform (WRITE)
};

ServerOperation * ServerOperation_New(const AwaServerSession * session)
{
    ServerOperation * operation = Awa_MemAlloc(sizeof(*operation));
    if (operation != NULL)
    {
        memset(operation, 0, sizeof(*operation));
        operation->Session = session;
        operation->Clients = Xml_CreateNode("Clients");

        if (operation->Clients != NULL)
        {
            operation->DefaultClientOperation = OperationCommon_New(session, SessionType_Server);
            if (operation->DefaultClientOperation != NULL)
            {
                operation->OperationCommons = Map_New();
                if (operation->OperationCommons != NULL)
                {
                    LogNew("ServerOperation", operation);
                }
                else
                {
                    LogErrorWithEnum(AwaError_OutOfMemory, "Unable to initialise operation");
                    Tree_Delete(operation->Clients);
                    OperationCommon_Free(&operation->DefaultClientOperation);
                    Awa_MemSafeFree(operation);
                    operation = NULL;
                }
            }
            else
            {
                LogErrorWithEnum(AwaError_OutOfMemory, "Unable to initialise operation");
                Tree_Delete(operation->Clients);
                Awa_MemSafeFree(operation);
                operation = NULL;
            }
        }
        else
        {
            LogErrorWithEnum(AwaError_OutOfMemory, "Unable to initialise operation");
            Awa_MemSafeFree(operation);
            operation = NULL;
        }
    }
    else
    {
        LogErrorWithEnum(AwaError_OutOfMemory);
    }
    return operation;
}

static void FreeOperationCommons(const char * clientID, void * operation, void * context)
{
    if (operation != NULL)
    {
        OperationCommon * operationCommon = (OperationCommon *)operation;
        OperationCommon_Free(&operationCommon);
    }
}

void ServerOperation_Free(ServerOperation ** operation)
{
    if ((operation != NULL) && (*operation != NULL))
    {
        // do not free the session, it is not owned by the operation

        OperationCommon_Free(&(*operation)->DefaultClientOperation);

        Map_ForEach((*operation)->OperationCommons, FreeOperationCommons, NULL);
        Map_Free(&(*operation)->OperationCommons);

        ObjectsTree_Free((*operation)->Clients);

        LogFree("ServerOperation", operation);
        Awa_MemSafeFree(*operation);
        *operation = NULL;
    }
}

const AwaServerSession * ServerOperation_GetSession(const ServerOperation * operation)
{
    const AwaServerSession * session = NULL;
    if (operation != NULL)
    {
        session = operation->Session;
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "operation is NULL");
    }
    return session;
}

TreeNode ServerOperation_GetClientsTree(const ServerOperation * operation)
{
    TreeNode objectsTree = NULL;
    if (operation != NULL)
    {
        objectsTree = operation->Clients;
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "operation is NULL");
    }
    return objectsTree;
}

TreeNode ServerOperation_GetObjectsTree(const ServerOperation * operation, const char * clientID)
{
    TreeNode objectsTree = NULL;
    if (operation != NULL)
    {
        if (operation->Clients != NULL)
        {
            if (clientID != NULL)
            {
                TreeNode clientNode = Xml_FindChildWithGrandchildValue(operation->Clients, "Client", "ID", clientID);
                if (clientNode != NULL)
                {
                    objectsTree = Xml_Find(clientNode, "Objects");
                }
            }
            else
            {
                LogErrorWithEnum(AwaError_TypeMismatch, "ClientID is NULL");
            }
        }
        else
        {
            LogErrorWithEnum(AwaError_OperationInvalid, "Clients tree is NULL");
        }
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "operation is NULL");
    }
    return objectsTree;
}

OperationCommon * ServerOperation_CreateOperationCommon(const ServerOperation * operation, const char * clientID)
{
    OperationCommon * operationCommon = NULL;
    if (operation != NULL)
    {
        if (operation->Clients != NULL)
        {
            if (ServerOperation_GetObjectsTree(operation, clientID) == NULL)
            {
                if (clientID != NULL)
                {
                    operationCommon = OperationCommon_New(ServerOperation_GetSession(operation), SessionType_Server);
                    Map_Put(operation->OperationCommons, clientID, operationCommon);

                    TreeNode clientNode = Xml_CreateNode("Client");
                    TreeNode clientIDNode = Xml_CreateNodeWithValue("ID", "%s", clientID);

                    TreeNode objectsTree = OperationCommon_GetObjectsTree(operationCommon);
                    TreeNode_AddChild(clientNode, clientIDNode);
                    TreeNode_AddChild(clientNode, objectsTree);
                    TreeNode_AddChild(operation->Clients, clientNode);
                }
                else
                {
                    LogErrorWithEnum(AwaError_TypeMismatch, "ClientID is NULL");
                }
            }
            else
            {
                LogErrorWithEnum(AwaError_CannotCreate, "Node with client ID %s already exists in clients tree", clientID);
            }
        }
        else
        {
            LogErrorWithEnum(AwaError_OperationInvalid, "Clients tree is NULL");
        }
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "operation is NULL");
    }
    return operationCommon;
}


AwaError ServerOperation_AddPath(ServerOperation * operation, const char * clientID, const char * path, TreeNode * resultNode)
{
    AwaError result = AwaError_Unspecified;
    OperationCommon * operationCommon = ServerOperation_GetOperationCommon(operation, clientID);
    if (operationCommon == NULL)
    {
        operationCommon = ServerOperation_CreateOperationCommon(operation, clientID);
    }
    result = OperationCommon_AddPathToObjectsTree(OperationCommon_GetObjectsTree(operationCommon), path, resultNode);
    return result;
}

OperationCommon * ServerOperation_GetOperationCommon(const ServerOperation * operation, const char * clientID)
{
    OperationCommon * clientOperation = NULL;
    if (operation != NULL)
    {
        if (operation->OperationCommons != NULL)
        {
            Map_Get(operation->OperationCommons, clientID, (void **)&clientOperation);
        }
        else
        {
            LogErrorWithEnum(AwaError_Internal, "OperationsCommon is NULL");
        }
    }
    return clientOperation;
}

OperationCommon * ServerOperation_GetDefaultClientOperation(const ServerOperation * operation)
{
    OperationCommon * defaultClientOperation = NULL;
    if (operation != NULL)
    {
        defaultClientOperation = operation->DefaultClientOperation;
    }
    return defaultClientOperation;
}

const char * ServerOperation_GetClientIDFromClientNode(const TreeNode clientNode)
{
    return (const char *)xmlif_GetOpaque(clientNode, "Client/ID");
}

IPCSessionID ServerOperation_GetSessionID(const ServerOperation * operation)
{
    return SessionCommon_GetSessionID(ServerSession_GetSessionCommon(operation->Session));
}


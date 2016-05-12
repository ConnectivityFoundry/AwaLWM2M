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

#include "lwm2m_server_xml_events.h"
#include "lwm2m_server_xml_handlers.h"
#include "lwm2m_server_xml_registered_entity_tree.h"
#include "lwm2m_xml_interface.h"
#include "lwm2m_ipc.h"

#define MSGTYPE_EVENT_REGISTER "Register"

void xmlif_HandleRegistrationEvent(RegistrationEventType eventType, void * context)
{
    // request should refer to the Notify channel:
    RequestInfoType * request = (RequestInfoType *)context;

    // TODO
    Lwm2m_Error("xmlif_HandleRegistrationEvent: %d %p\n", eventType, context);

    Lwm2mContextType * lwm2mContext = (Lwm2mContextType*)request->Context;

    TreeNode clientsNode = IPC_NewClientsNode();

    struct ListHead * i;
    ListForEach(i, Lwm2mCore_GetClientList(lwm2mContext))
    {
        const Lwm2mClientType * client = ListEntry(i, Lwm2mClientType, list);

        TreeNode clientNode = IPC_AddClientNode(clientsNode, client->EndPointName);

        // add tree of registered entities (objects and object instances)
        TreeNode objectsTree = BuildRegisteredEntityTree(client);
        TreeNode_AddChild(clientNode, objectsTree);
    }

    // Build response
    TreeNode contentNode = IPC_NewContentNode();
    TreeNode_AddChild(contentNode, clientsNode);

    TreeNode responseNode = IPC_NewEventNode(MSGTYPE_EVENT_REGISTER);
    TreeNode_AddChild(responseNode, contentNode);

    IPC_SendResponse(responseNode, request->Sockfd, &request->FromAddr, request->AddrLen);

    Tree_Delete(responseNode);
    free(request);
}

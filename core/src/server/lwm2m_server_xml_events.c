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

#define MSGTYPE_EVENT_REGISTER   "Register"
#define MSGTYPE_EVENT_UPDATE     "Update"
#define MSGTYPE_EVENT_DEREGISTER "Deregister"

void xmlif_HandleRegistrationEvent(RegistrationEventType eventType, void * context, void * parameter)
{
    // should refer to the Notify channel:
    EventContext * eventContext = (EventContext *)context;
    const Lwm2mClientType * client = (const Lwm2mClientType *)parameter;

    if ((eventContext != NULL) && (client != NULL))
    {
        Lwm2m_Debug("Event Callback: eventType %d, context %p, parameter %p\n", eventType, context, parameter);

        TreeNode clientsNode = IPC_NewClientsNode();
        TreeNode clientNode = IPC_AddClientNode(clientsNode, client->EndPointName);

        if ((eventType == RegistrationEventType_Register) || (eventType == RegistrationEventType_Update))
        {
            // add tree of registered entities (objects and object instances)
            TreeNode objectsTree = BuildRegisteredEntityTree(client);
            TreeNode_AddChild(clientNode, objectsTree);
        }

        // Build response
        TreeNode contentNode = IPC_NewContentNode();
        TreeNode_AddChild(contentNode, clientsNode);

        const char * msgType = NULL;
        switch (eventType)
        {
        case RegistrationEventType_Register:
            msgType = MSGTYPE_EVENT_REGISTER;
            break;
        case RegistrationEventType_Update:
            msgType = MSGTYPE_EVENT_UPDATE;
            break;
        case RegistrationEventType_Deregister:
            msgType = MSGTYPE_EVENT_DEREGISTER;
            break;
        default:
            Lwm2m_Error("Unhandled eventType %d\n", eventType)
        }

        if (msgType != NULL)
        {
            TreeNode notificationNode = IPC_NewNotificationNode(msgType);
            TreeNode_AddChild(notificationNode, contentNode);

            IPC_SendResponse(notificationNode, eventContext->Sockfd, &eventContext->FromAddr, eventContext->AddrLen);
            Tree_Delete(notificationNode);
        }
    }
    else
    {
        Lwm2m_Error("Bad callback: eventType %d, context %p, parameter %p\n", eventType, context, parameter);
    }
}

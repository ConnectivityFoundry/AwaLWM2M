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


#ifndef LWM2M_IPC_H
#define LWM2M_IPC_H

#include "lwm2m_result.h"
#include "xmltree.h"
#include "../../api/src/ipc_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IPC_MESSAGE_TYPE_REQUEST      "Request"
#define IPC_MESSAGE_TYPE_RESPONSE     "Response"
#define IPC_MESSAGE_TYPE_NOTIFICATION "Notification"

TreeNode IPC_NewResponseNode(const char * subType, AwaResult code, IPCSessionID sessionID);
TreeNode IPC_NewNotificationNode(const char * subType, IPCSessionID sessionID);

void IPC_SetSessionID(TreeNode message, IPCSessionID sessionID);
IPCSessionID IPC_GetSessionID(const TreeNode content);

TreeNode IPC_NewClientsNode();
TreeNode IPC_NewContentNode();
TreeNode IPC_AddClientNode(TreeNode clientsNode, const char * clientID);

// Serialise and send the IPC response back to the originator
int IPC_SendResponse(TreeNode responseNode, int sockfd, const struct sockaddr * fromAddr, int addrLen);

TreeNode IPC_AddResultTag(TreeNode leafNode, int error);
TreeNode IPC_AddServerResultTag(TreeNode leafNode, int error, int serverError);
void IPC_AddResultTagToAllLeafNodes(TreeNode objectInstanceNode, int error);
void IPC_AddServerResultTagToAllLeafNodes(TreeNode objectInstanceNode, int error, int serverError);

#ifdef __cplusplus
}
#endif

#endif // LWM2M_IPC_H

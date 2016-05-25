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


#include <string.h>

#include "lwm2m_ipc.h"

#include "../../../api/src/objects_tree.h"
#include "../../api/src/ipc_defs.h"
#include "xml.h"
#include "lwm2m_xml_interface.h"
#include "lwm2m_debug.h"

#include <awa/static.h>

static TreeNode IPC_NewNode(const char * type, const char * subType, IPCSessionID sessionID)
{
    TreeNode responseNode = Xml_CreateNode(type);
    TreeNode subTypeNode = Xml_CreateNodeWithValue("Type", "%s", subType);
    TreeNode_AddChild(responseNode, subTypeNode);
    if (sessionID > 0)
    {
        IPC_SetSessionID(responseNode, sessionID);
    }
    return responseNode;
}

TreeNode IPC_NewResponseNode(const char * subType, AwaResult code, IPCSessionID sessionID)
{
    TreeNode responseNode = IPC_NewNode("Response", subType, sessionID);
    TreeNode codeNode = Xml_CreateNodeWithValue("Code", "%d", code);
    TreeNode_AddChild(responseNode, codeNode);
    return responseNode;
}

TreeNode IPC_NewNotificationNode(const char * subType, IPCSessionID sessionID)
{
    return IPC_NewNode("Notification", subType, sessionID);
}

void IPC_SetSessionID(TreeNode message, IPCSessionID sessionID)
{
    // assume there is no session ID already
    // TODO: check this assumption!
    TreeNode sessionIDNode = Xml_CreateNodeWithValue("SessionID", "%d", sessionID);
    TreeNode_AddChild(message, sessionIDNode);
}

IPCSessionID IPC_GetSessionID(const TreeNode content)
{
    IPCSessionID sessionID = -1;
    if (content != NULL)
    {
        const char * type = NULL;
        if ((type = TreeNode_GetName(content)) != NULL)
        {
            enum { PATH_LEN = 128 };
            char path[PATH_LEN] = { 0 };
            if (snprintf(path, PATH_LEN, "%s/SessionID", type) > 0)
            {
                TreeNode sessionIDNode = TreeNode_Navigate(content, path);
                const char * sessionIDStr = NULL;

                if ((sessionIDStr = (const char *)TreeNode_GetValue(sessionIDNode)) != NULL)
                {
                    sessionID = atoi(sessionIDStr);
                }
            }
        }
    }
    else
    {
        Lwm2m_Error("content is NULL");
    }
    return sessionID;
}

TreeNode IPC_NewClientsNode()
{
    return Xml_CreateNode("Clients");
}

TreeNode IPC_NewContentNode()
{
    return Xml_CreateNode("Content");
}

TreeNode IPC_AddClientNode(TreeNode clientsNode, const char * clientID)
{
    TreeNode clientNode = Xml_CreateNode("Client");
    TreeNode clientIDNode = Xml_CreateNodeWithValue("ID", "%s", clientID);
    TreeNode_AddChild(clientNode, clientIDNode);
    TreeNode_AddChild(clientsNode, clientNode);
    return clientNode;
}

int IPC_SendResponse(TreeNode responseNode, int sockfd, const struct sockaddr * fromAddr, int addrLen)
{
    int rc = 0;
    // Serialise response
    char buffer[IPC_MAX_BUFFER_LEN] = { 0 };
    if (Xml_TreeToString(responseNode, buffer, sizeof(buffer)) > 0)
    {
        xmlif_SendTo(sockfd, buffer, strlen(buffer), 0, fromAddr, addrLen);
    }
    else
    {
        Lwm2m_Error("Xml_TreeToString failed\n");
        rc = -1;
    }
    return rc;
}

TreeNode IPC_AddResultTag(TreeNode leafNode, int error)
{
    TreeNode resultTag = Xml_Find(leafNode, "Result");
    if (resultTag == NULL)
    {
        resultTag = Xml_CreateNode("Result");
        TreeNode errorTag = Xml_CreateNodeWithValue("Error", "%s", AwaError_ToString(error));
        TreeNode_AddChild(resultTag, errorTag);
        TreeNode_AddChild(leafNode, resultTag);
    }
    else
    {
        Lwm2m_Warning("A result tag already exists in the specified node\n");
    }
    return resultTag;
}

TreeNode IPC_AddServerResultTag(TreeNode leafNode, int error, int serverError)
{
    TreeNode resultTag = IPC_AddResultTag(leafNode, error);
    if (resultTag != NULL)
    {
        if (error == AwaError_LWM2MError && Xml_Find(resultTag, "LWM2MError") == NULL)
        {
            TreeNode lwm2mErrorTag = Xml_CreateNodeWithValue("LWM2MError", "%s", AwaLWM2MError_ToString(serverError));
            TreeNode_AddChild(resultTag, lwm2mErrorTag);
        }
        else
        {
            Lwm2m_Warning("A LWM2MError tag already exists in the specified node\n");
        }
    }
    else
    {
        Lwm2m_Error("resultTag is NULL\n");
    }
    return resultTag;
}

void IPC_AddResultTagToAllLeafNodes(TreeNode objectInstanceNode, int error)
{
    TreeNode leaf = objectInstanceNode;
    if (ObjectsTree_IsLeafNode(leaf))
    {
        IPC_AddResultTag(leaf, error);
    }

    while ((leaf = ObjectsTree_GetNextLeafNode(leaf)) != NULL)
    {
        IPC_AddResultTag(leaf, error);
    }
}

void xmlif_AddResultTagToAllNodes(TreeNode node, int error)
{
    if (ObjectsTree_IsObjectNode(node) || ObjectsTree_IsObjectInstanceNode(node) || ObjectsTree_IsResourceNode(node))
    {
        IPC_AddResultTag(node, error);
    }
    TreeNode child = NULL;
    uint32_t index = 0;
    while ((child = TreeNode_GetChild(node, index++)) != NULL)
    {
        xmlif_AddResultTagToAllNodes(child, error);
    }
}

void IPC_AddServerResultTagToAllLeafNodes(TreeNode objectInstanceNode, int error, int serverError)
{
    TreeNode leaf = objectInstanceNode;
    if (ObjectsTree_IsLeafNode(leaf))
    {
        IPC_AddServerResultTag(leaf, error, serverError);
    }

    while ((leaf = ObjectsTree_GetNextLeafNode(leaf)) != NULL)
    {
        IPC_AddServerResultTag(leaf, error, serverError);
    }
}


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


#ifndef CONTIKI

#include <xmltree.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <float.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <inttypes.h>

#include "lwm2m_xml_interface.h"
#include "lwm2m_types.h"
#include "lwm2m_debug.h"
#include "lwm2m_list.h"
#include "xml.h"
#include "lwm2m_result.h"
#include "lwm2m_xml_serdes.h"
#include "lwm2m_ipc.h"
#include "ipc_session.h"
#include "../../api/src/ipc_defs.h"

typedef struct
{
    struct ListHead list;
    XmlRequestHandler Function;
    char * Name;
} IpcHandlerType;

static struct ListHead handlerList;
static void * g_context = NULL;


int xmlif_AddRequestHandler(const char * msgType, XmlRequestHandler handler)
{
    IpcHandlerType * new = malloc(sizeof(IpcHandlerType));
    new->Name = strdup(msgType);
    new->Function = handler;
    ListAdd(&new->list, &handlerList);
    return 0;
}

ssize_t xmlif_SendTo(int sockfd, const void *buf, size_t len, int flags,
                     const struct sockaddr *dest_addr, socklen_t addrlen)
{
    Lwm2m_Debug("Send %zu bytes on IPC\n%s\n", len , (const char *)buf);
    ssize_t result = sendto(sockfd, buf, len, flags, dest_addr, addrlen);
    if (result == -1)
    {
        perror("sendto");
    }
    return result;
}

int xmlif_init(void * context, int port)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char service[32];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    sprintf(service, "%d", port);

    if ((rv = getaddrinfo(NULL, service, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    // Loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            sockfd = -1;
            perror("listener: bind");  // may be a benign warning - disable?

            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            char ipAddress[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(ipv4->sin_addr), ipAddress, INET_ADDRSTRLEN);
            fprintf(stderr, "ipAddress %s, port %d\n", ipAddress, ntohs(ipv4->sin_port));

            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "listener: failed to bind socket\n");
        freeaddrinfo(servinfo);
        return -1;
    }

    freeaddrinfo(servinfo);

    // Keep track of context to use.
    g_context = context;
    ListInit(&handlerList);

    IPCSession_Init();

    return sockfd;
}

static void HandleInvalidRequest(const RequestInfoType * request)
{
    TreeNode responseNode = IPC_NewResponseNode(IPC_MESSAGE_SUB_TYPE_INVALID, AwaResult_BadRequest, request->SessionID);
    IPC_SendResponse(responseNode, request->Sockfd, &request->FromAddr, request->AddrLen);
    Tree_Delete(responseNode);
}

int xmlif_process(int sockfd)
{
    struct sockaddr_storage their_addr;
    char buf[IPC_MAX_BUFFER_LEN] = {0};
    socklen_t addr_len;
    int numbytes;
    TreeNode root;

    // Read data from socket. Assume the XML is all in one UDP packet.
    addr_len = sizeof(their_addr);
    if ((numbytes = recvfrom(sockfd, buf, IPC_MAX_BUFFER_LEN-1 , 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1)
    {
        perror("recvfrom");
        return -1;
    }

    Lwm2m_Debug("Received %d bytes on IPC\n%s\n", numbytes, buf);

    // assuming we received a full message, process it.
    root = TreeNode_ParseXML(buf, numbytes, true);
    if (root != NULL)
    {
        TreeNode node = TreeNode_Navigate(root, "Request/Type");
        if (node)
        {
            const char * value = TreeNode_GetValue(node);
            if (value == NULL)
            {
                Lwm2m_Error("Missing Request Type\n");
                goto error;
            }

            TreeNode content = TreeNode_Navigate(root, "Request/Content");

            bool handled = false;
            struct ListHead * i;
            ListForEach(i, &handlerList)
            {
                IpcHandlerType * handler = ListEntry(i, IpcHandlerType, list);
                if (!strcmp(handler->Name, value))
                {
                    RequestInfoType * request = malloc(sizeof(RequestInfoType));
                    if (request != NULL)
                    {
                        memset(request, 0, sizeof(*request));
                        request->Sockfd = sockfd;
                        memcpy(&request->FromAddr, &their_addr, addr_len);
                        request->AddrLen = addr_len;
                        request->Context = g_context;

                        // Ensure requests have a valid SessionID
                        if (strcmp(IPC_MESSAGE_SUB_TYPE_CONNECT, value) == 0)
                        {
                            // CONNECT requests should have no session ID - allocate one
                            request->SessionID = IPCSession_AssignSessionID();
                        }
                        else
                        {
                            IPCSessionID sessionID = IPC_GetSessionID(root);
                            if (!IPCSession_IsValid(sessionID))
                            {
                                Lwm2m_Error("Invalid Session ID %d\n", sessionID);
                                free(request);
                                goto error;
                            }
                            else
                            {
                                request->SessionID = sessionID;
                            }
                        }

                        handler->Function(request, content);
                        handled = true;
                        break;
                    }
                    else
                    {
                        Lwm2m_Error("Failed to allocate memory\n");
                        goto error;
                    }
                }
            }

            if (!handled)
            {
                printf("Unrecognised Request Type %s\n", value);
                goto error;
            }
        }
        else
        {
            // no Request/Type
            Lwm2m_Error("Missing Request Type\n");
            goto error;
        }
    }
    else
    {
        // no root node
        Lwm2m_Error("Invalid Request\n");
        goto error;
    }

    Tree_Delete(root);
    return 0;

error:
    Tree_Delete(root);

    RequestInfoType * request = malloc(sizeof(RequestInfoType));
    if (request != NULL)
    {
        memset(request, 0, sizeof(*request));
        request->Sockfd = sockfd;
        memcpy(&request->FromAddr, &their_addr, addr_len);
        request->AddrLen = addr_len;
        request->Context = g_context;
        HandleInvalidRequest(request);
        free(request);
    }
    else
    {
        Lwm2m_Error("Failed to allocate memory\n");
    }

    return 0;
}

void xmlif_destroy(int sockfd)
{
    if (sockfd >= 0)
    {
        close(sockfd);
    }

    // clean up handlerList
    {
        struct ListHead * i, * n;
        ListForEachSafe(i, n, &handlerList)
        {
            IpcHandlerType * handler = ListEntry(i, IpcHandlerType, list);
            if (handler != NULL)
            {
                free(handler->Name);
                free(handler);
            }
        }
    }

    IPCSession_Shutdown();
}

TreeNode xmlif_GenerateConnectResponse(DefinitionRegistry * definitionRegistry, IPCSessionID sessionID)
{
    ObjectDefinition * objFormat = 0;
    int result = AwaResult_Success;

    TreeNode content = Xml_CreateNode("Content");

    // Create a container for object definitions.
    TreeNode objectDefinitionsNode = Xml_CreateNode("ObjectDefinitions");
    TreeNode_AddChild(content, objectDefinitionsNode);

    // Iterate through all defined object types, adding metadata descriptions to tree.
    ObjectIDType objectID = -1;
    while ((objectID = Definition_GetNextObjectType(definitionRegistry, objectID)) != -1)
    {
        objFormat = Definition_LookupObjectDefinition(definitionRegistry, objectID);
        if (objFormat == NULL)
        {
            result = AwaResult_NotFound;
            goto end;
        }

        TreeNode objectMetaData = xmlif_ConstructObjectDefinitionNode(definitionRegistry, objFormat, objectID);
        if (objectMetaData != NULL)
        {
            TreeNode_AddChild(objectDefinitionsNode, objectMetaData);
        }
        else
        {
            Tree_Delete(content);
            result = AwaResult_NotFound;
        }
    }

end: ;
    TreeNode response = IPC_NewResponseNode(IPC_MESSAGE_SUB_TYPE_CONNECT, result, sessionID);
    TreeNode_AddChild(response, content);
    return response;
}

TreeNode xmlif_ConstructObjectDefinitionNode(const DefinitionRegistry * definitions, const ObjectDefinition * objFormat, int objectID)
{
    TreeNode objectMetaData = Xml_CreateNode("ObjectMetadata");

    TreeNode_AddChild(objectMetaData, Xml_CreateNodeWithValue("ObjectID", "%d", objFormat->ObjectID));
    TreeNode_AddChild(objectMetaData, Xml_CreateNodeWithValue("SerialisationName", "%s", objFormat->ObjectName));
    TreeNode_AddChild(objectMetaData, Xml_CreateNodeWithValue("MaximumInstances", "%d", objFormat->MaximumInstances));
    TreeNode_AddChild(objectMetaData, Xml_CreateNodeWithValue("MinimumInstances", "%d", objFormat->MinimumInstances));

    TreeNode properties = Xml_CreateNode("Properties");
    TreeNode_AddChild(objectMetaData, properties);

    int resourceID = -1;
    while ((resourceID = Definition_GetNextResourceType(definitions, objectID, resourceID)) != -1)
    {
        TreeNode property = Xml_CreateNode("Property");

        ResourceDefinition * resFormat = Definition_LookupResourceDefinition(definitions, objectID, resourceID);
        if (resFormat == NULL)
        {
            Tree_Delete(objectMetaData);
            Tree_Delete(property);
            goto error;
        }

        TreeNode_AddChild(property, Xml_CreateNodeWithValue("PropertyID", "%d", resFormat->ResourceID));
        TreeNode_AddChild(property, Xml_CreateNodeWithValue("DataType", "%s", xmlif_DataTypeToString(resFormat->Type)));
        TreeNode_AddChild(property, Xml_CreateNodeWithValue("Access", "%s", xmlif_OperationToString(resFormat->Operation)));
        TreeNode_AddChild(property, Xml_CreateNodeWithValue("SerialisationName", "%s", resFormat->ResourceName));
        TreeNode_AddChild(property, Xml_CreateNodeWithValue("MaximumInstances", "%d", resFormat->MaximumInstances));
        TreeNode_AddChild(property, Xml_CreateNodeWithValue("MinimumInstances", "%d", resFormat->MinimumInstances));

        if (resFormat->DefaultValueNode != NULL)
        {
            if (!IS_MULTIPLE_INSTANCE(resFormat))
            {
                uint16_t defaultValueLength;
                const uint8_t * defaultValue = Lwm2mTreeNode_GetValue(resFormat->DefaultValueNode, &defaultValueLength);

                if (defaultValue != NULL)
                {
                    char * value = xmlif_EncodeValue(resFormat->Type, defaultValue, defaultValueLength);
                    TreeNode_AddChild(property, Xml_CreateNodeWithValue("DefaultValue", value));
                    free(value);
                }
                else
                {
                    TreeNode_AddChild(property, Xml_CreateNodeWithValue("DefaultValue", ""));
                }
            }
            else
            {
                TreeNode defaultValueNode = Xml_CreateNode("DefaultValue");

                Lwm2mTreeNode * child = Lwm2mTreeNode_GetFirstChild(resFormat->DefaultValueNode);

                while(child != NULL)
                {
                    int resourceInstanceID;
                    Lwm2mTreeNode_GetID(child, &resourceInstanceID);
                    uint16_t defaultValueLength;
                    const uint8_t * defaultValue = Lwm2mTreeNode_GetValue(child, &defaultValueLength);

                    TreeNode resourceInstanceNode = Xml_CreateNode("ResourceInstance");

                    TreeNode_AddChild(resourceInstanceNode, Xml_CreateNodeWithValue("ID", "%d", resourceInstanceID));

                    if (defaultValue != NULL)
                    {
                        char * value = xmlif_EncodeValue(resFormat->Type, defaultValue, defaultValueLength);
                        TreeNode_AddChild(resourceInstanceNode, Xml_CreateNodeWithValue("Value", value));
                        free(value);
                    }
                    else
                    {
                        TreeNode_AddChild(resourceInstanceNode, Xml_CreateNodeWithValue("Value", ""));
                    }
                    TreeNode_AddChild(defaultValueNode, resourceInstanceNode);

                    child = Lwm2mTreeNode_GetNextChild(resFormat->DefaultValueNode, child);
                }
            }
        }

        TreeNode_AddChild(properties, property);
    }
    return objectMetaData;

error:
    return NULL;
}

#endif // CONTIKI

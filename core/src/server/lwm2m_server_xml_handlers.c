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

#include <xmltree.h>

#include "lwm2m_server_xml_handlers.h"

#include "../../../api/src/lwm2m_error.h"
#include "../../../api/src/objects_tree.h"
#include "lwm2m_xml_interface.h"
#include "lwm2m_xml_serdes.h"
#include "lwm2m_xml_interface.h"
#include "lwm2m_tree_builder.h"
#include "lwm2m_types.h"
#include "b64.h"
#include "lwm2m_debug.h"
#include "coap_abstraction.h"
#include "lwm2m_tlv.h"
#include "lwm2m_serdes.h"
#include "lwm2m_list.h"
#include "lwm2m_core.h"
#include "server/lwm2m_registration.h"
#include "xml.h"
#include "lwm2m_ipc.h"
#include "lwm2m_tree_node.h"
#include "lwm2m_server_xml_events.h"
#include "lwm2m_server_xml_registered_entity_tree.h"
#include "lwm2m_events.h"
#include "../../api/src/path.h"
#include "../../api/src/utils.h"
#include "../../api/include/awa/common.h"
#include "../../api/include/awa/server.h"
#include "../../api/src/ipc_defs.h"
#include "../common/ipc_session.h"

#define MAX_PAYLOAD_SIZE (10240)
#define MAX_URI_LENGTH   (256)

typedef struct
{
    RequestInfoType * Request;
    TreeNode ResponseContentNode;
    TreeNode ResponseObjectsTree;
    AwaResult Result;
    bool Reusable;
    size_t ResponseCount;
    bool AddResultTags;
} IpcCoapRequestContext;

static int xmlif_HandlerConnectRequest(RequestInfoType * request, TreeNode content);
static int xmlif_HandlerEstablishNotifyRequest(RequestInfoType * request, TreeNode content);
static int xmlif_HandlerDisconnectRequest(RequestInfoType * request, TreeNode content);
static int xmlif_HandlerListClients(RequestInfoType * request, TreeNode content);
static int xmlif_HandlerDefineRequest(RequestInfoType * request, TreeNode content);
static int xmlif_HandlerObserveRequest(RequestInfoType * request, TreeNode content);
static int xmlif_HandlerReadRequest(RequestInfoType * request, TreeNode content);
static int xmlif_HandlerDeleteRequest(RequestInfoType * request, TreeNode content);
static int xmlif_HandlerWriteRequest(RequestInfoType * request, TreeNode content);
static int xmlif_HandlerWriteAttributesRequest(RequestInfoType * request, TreeNode content);
static int xmlif_HandlerExecuteRequest(RequestInfoType * request, TreeNode content);
static int xmlif_HandlerDiscoverRequest(RequestInfoType * request, TreeNode content);

static void xmlif_HandlerReadResponse(void * ctxt, AddressType* address, const char * responsePath,
                                      int coapResponseCode, ContentType contentType, char * payload, size_t payloadLen);
static void xmlif_HandlerWriteResponse(void * ctxt, AddressType* address, const char * responsePath,
                                       int coapResponseCode, ContentType contentType, char * payload, size_t payloadLen);
static void xmlif_HandlerCreateResponse(void * ctxt, AddressType* address, const char * responsePath,
                                        int coapResponseCode, ContentType contentType, char * payload, size_t payloadLen);
static void xmlif_HandlerExecuteResponse(void * ctxt, AddressType* address, const char * responsePath,
                                         int coapResponseCode, ContentType contentType, char * payload, size_t payloadLen);
static void xmlif_HandlerDeleteResponse(void * ctxt, AddressType* address, const char * responsePath,
                                        int responseCode, ContentType contentType, char * payload, size_t payloadLen);
static void xmlif_HandlerObserveResponse(void * ctxt, AddressType* address, const char * responsePath,
                                         int coapResponseCode, ContentType contentType, char * payload, size_t payloadLen);
static void xmlif_HandlerCancelObserveResponse(void * ctxt, AddressType* address, const char * responsePath,
                                               int coapResponseCode, ContentType contentType, char * payload, size_t payloadLen);
static void xmlif_HandlerDiscoverResponse(void * ctxt, AddressType* address, const char * responsePath,
                                          int responseCode, ContentType contentType, char * payload, size_t payloadLen);
static void xmlif_HandlerWriteAttributesResponse(void * ctxt, AddressType* address, const char * responsePath,
                                                 int coapResponseCode, ContentType contentType, char * payload, size_t payloadLen);

// send CoAP request handlers
typedef bool (*SendCoapRequestHandler)(IpcCoapRequestContext * requestContext, Lwm2mClientType * client,
                                       ObjectInstanceResourceKey * key, TreeNode currentLeafNode, TreeNode currentResponsePathNode);
static bool xmlif_HandlerSendCoapReadRequest(IpcCoapRequestContext * requestContext, Lwm2mClientType * client,
                                             ObjectInstanceResourceKey * key, TreeNode currentLeafNode, TreeNode currentResponsePathNode);
static bool xmlif_HandlerSendCoapObserveRequest(IpcCoapRequestContext * requestContext, Lwm2mClientType * client,
                                                ObjectInstanceResourceKey * key, TreeNode currentLeafNode, TreeNode currentResponsePathNode);
static bool xmlif_HandlerSendCoapExecuteRequest(IpcCoapRequestContext * requestContext, Lwm2mClientType * client,
                                                ObjectInstanceResourceKey * key, TreeNode currentLeafNode, TreeNode currentResponsePathNode);
static bool xmlif_HandlerSendCoapWriteAttributesRequest(IpcCoapRequestContext * requestContext, Lwm2mClientType * client,
                                                        ObjectInstanceResourceKey * key, TreeNode currentLeafNode, TreeNode currentResponsePathNode);

// successful response callbacks
typedef void (*IpcCoapSuccessCallback)(IpcCoapRequestContext * requestContext, const char * responsePath,
                                       int coapResponseCode, TreeNode pathNode, const char * responseType, ContentType contentType, char * payload, size_t payloadLen);
static void xmlif_HandlerDefaultSuccessfulResponse(IpcCoapRequestContext * requestContext, const char * responsePath,
                                                   int coapResponseCode, TreeNode pathNode, const char * responseType, ContentType contentType, char * payload, size_t payloadLen);
static void xmlif_HandlerSuccessfulReadResponse(IpcCoapRequestContext * requestContext, const char * responsePath,
                                                int coapResponseCode, TreeNode pathNode, const char * responseType, ContentType contentType, char * payload, size_t payloadLen);
static void xmlif_HandlerSuccessfulNotifyResponse(IpcCoapRequestContext * requestContext, const char * responsePath,
                                                  int coapResponseCode, TreeNode pathNode, const char * responseType, ContentType contentType, char * payload, size_t payloadLen);
static void xmlif_HandlerSuccessfulExecuteResponse(IpcCoapRequestContext * requestContext, const char * responsePath,
                                                   int coapResponseCode, TreeNode pathNode, const char * responseType, ContentType contentType, char * payload, size_t payloadLen);
static void xmlif_HandlerSuccessfulWriteAttributesResponse(IpcCoapRequestContext * requestContext, const char * responsePath,
                                                           int coapResponseCode, TreeNode pathNode, const char * responseType, ContentType contentType, char * payload, size_t payloadLen);

static void xmlif_HandlerFreeIpcCoapRequestContext(void * ctxt);

static int xmlif_SerialiseResourceIntoExistingObjectsTree(Lwm2mTreeNode * resourceNode, TreeNode destResourceNode, const DefinitionRegistry * definitionRegistry,
                                                          ObjectIDType objectID, ObjectIDType objectInstanceID, ResourceIDType ResourceID)
{
    int result = -1;
    if (Lwm2mTreeNode_GetType(resourceNode) != Lwm2mTreeNodeType_Resource)
    {
        Lwm2m_Error("resourceNode is not of type Lwm2mTreeNodeType_Resource");
        goto error;
    }
    if (strcmp(TreeNode_GetName(destResourceNode), "Resource") != 0)
    {
        Lwm2m_Error("destResourceNode is not a Resource node");
        goto error;
    }
    Lwm2mTreeNode * resourceInstanceNode = Lwm2mTreeNode_GetFirstChild(resourceNode);

    if (Definition_IsTypeMultiInstance(definitionRegistry, objectID, ResourceID))
    {
        while(resourceInstanceNode != NULL)
        {
            int resourceInstanceID;
            Lwm2mTreeNode_GetID(resourceInstanceNode, &resourceInstanceID);
            TreeNode responseResourceInstanceNode = ObjectsTree_FindOrCreateChildNode(destResourceNode, "ResourceInstance", resourceInstanceID);

            TreeNode resourceInstanceValueNode = Xml_CreateNode("Value");
            TreeNode_AddChild(responseResourceInstanceNode, resourceInstanceValueNode);

            uint16_t dataLength;
            const uint8_t * dataValue;

            dataValue = Lwm2mTreeNode_GetValue(resourceInstanceNode, &dataLength);

            ResourceDefinition * definition = (ResourceDefinition *)Lwm2mTreeNode_GetDefinition(resourceNode);

            char * encodedValue = xmlif_EncodeValue(definition->Type, dataValue, dataLength);

            if (dataValue != NULL)
            {
                TreeNode_SetValue(resourceInstanceValueNode, encodedValue, strlen(encodedValue));
            }
            else
            {
                TreeNode_SetValue(resourceInstanceValueNode, "", 0);
            }

            free(encodedValue);

            resourceInstanceNode = Lwm2mTreeNode_GetNextChild(resourceNode, resourceInstanceNode);
        }
    }
    else
    {
        TreeNode resourceValueNode = Xml_CreateNode("Value");
        TreeNode_AddChild(destResourceNode, resourceValueNode);

        uint16_t dataLength;
        const uint8_t * dataValue;

        dataValue = Lwm2mTreeNode_GetValue(resourceInstanceNode, &dataLength);

        ResourceDefinition * definition = (ResourceDefinition *)Lwm2mTreeNode_GetDefinition(resourceNode);

        char * encodedValue = xmlif_EncodeValue(definition->Type, dataValue, dataLength);

        if (encodedValue != NULL)
        {
            TreeNode_SetValue(resourceValueNode, encodedValue, strlen(encodedValue));
        }
        else
        {
            TreeNode_SetValue(resourceValueNode, "", 0);
        }

        free(encodedValue);
    }
    result = 0;
error:
    return result;
}

static int xmlif_SerialiseObjectInstanceIntoExistingObjectsTree(Lwm2mTreeNode * objectInstanceNode, TreeNode destObjectInstanceNode,
                                                                const DefinitionRegistry * definitionRegistry, ObjectIDType objectID, ObjectIDType objectInstanceID)
{
    int result = -1;
    if (Lwm2mTreeNode_GetType(objectInstanceNode) != Lwm2mTreeNodeType_ObjectInstance)
    {
        Lwm2m_Error("objectNode is not of type Lwm2mTreeNodeType_Object");
        goto error;
    }
    if (strcmp(TreeNode_GetName(destObjectInstanceNode), "ObjectInstance") != 0)
    {
        Lwm2m_Error("destObjectInstanceNode is not an Object instance node");
        goto error;
    }

    Lwm2mTreeNode * resourceNode = Lwm2mTreeNode_GetFirstChild(objectInstanceNode);
    while(resourceNode != NULL)
    {
        int resourceID;
        Lwm2mTreeNode_GetID(resourceNode, &resourceID);
        TreeNode destResourceNode = ObjectsTree_FindOrCreateChildNode(destObjectInstanceNode, "Resource", resourceID);
        xmlif_SerialiseResourceIntoExistingObjectsTree(resourceNode, destResourceNode, definitionRegistry, objectID, objectInstanceID, resourceID);
        resourceNode = Lwm2mTreeNode_GetNextChild(objectInstanceNode, resourceNode);
    }
    result = 0;
error:
    return result;
}

int xmlif_SerialiseObjectIntoExistingObjectsTree(Lwm2mTreeNode * objectNode, TreeNode destObjectNode,
                                                 const DefinitionRegistry * definitionRegistry, ObjectIDType objectID)
{
    int result = -1;
    if (Lwm2mTreeNode_GetType(objectNode) != Lwm2mTreeNodeType_Object)
    {
        Lwm2m_Error("objectNode is not of type Lwm2mTreeNodeType_Object");
        goto error;
    }
    if (strcmp(TreeNode_GetName(destObjectNode), "Object") != 0)
    {
        Lwm2m_Error("destObjectNode is not an Object node");
        goto error;
    }

    Lwm2mTreeNode * objectInstanceNode = Lwm2mTreeNode_GetFirstChild(objectNode);
    while(objectInstanceNode != NULL)
    {
        int objectInstanceID;
        Lwm2mTreeNode_GetID(objectInstanceNode, &objectInstanceID);
        TreeNode destObjectInstanceNode = ObjectsTree_FindOrCreateChildNode(destObjectNode, "ObjectInstance", objectInstanceID);
        xmlif_SerialiseObjectInstanceIntoExistingObjectsTree(objectInstanceNode, destObjectInstanceNode, definitionRegistry, objectID, objectInstanceID);
        objectInstanceNode = Lwm2mTreeNode_GetNextChild(objectNode, objectInstanceNode);
    }
    result = 0;
error:
    return result;
}

void xmlif_RegisterHandlers(void)
{
    xmlif_AddRequestHandler(IPC_MESSAGE_SUB_TYPE_CONNECT,           xmlif_HandlerConnectRequest);
    xmlif_AddRequestHandler(IPC_MESSAGE_SUB_TYPE_ESTABLISH_NOTIFY,  xmlif_HandlerEstablishNotifyRequest);
    xmlif_AddRequestHandler(IPC_MESSAGE_SUB_TYPE_DISCONNECT,        xmlif_HandlerDisconnectRequest);
    xmlif_AddRequestHandler(IPC_MESSAGE_SUB_TYPE_LIST_CLIENTS,      xmlif_HandlerListClients);
    xmlif_AddRequestHandler(IPC_MESSAGE_SUB_TYPE_DEFINE,            xmlif_HandlerDefineRequest);
    xmlif_AddRequestHandler(IPC_MESSAGE_SUB_TYPE_DELETE,            xmlif_HandlerDeleteRequest);
    xmlif_AddRequestHandler(IPC_MESSAGE_SUB_TYPE_READ,              xmlif_HandlerReadRequest);
    xmlif_AddRequestHandler(IPC_MESSAGE_SUB_TYPE_WRITE,             xmlif_HandlerWriteRequest);
    xmlif_AddRequestHandler(IPC_MESSAGE_SUB_TYPE_EXECUTE,           xmlif_HandlerExecuteRequest);
    xmlif_AddRequestHandler(IPC_MESSAGE_SUB_TYPE_OBSERVE,           xmlif_HandlerObserveRequest);
    xmlif_AddRequestHandler(IPC_MESSAGE_SUB_TYPE_DISCOVER,          xmlif_HandlerDiscoverRequest);
    xmlif_AddRequestHandler(IPC_MESSAGE_SUB_TYPE_WRITE_ATTRIBUTES,  xmlif_HandlerWriteAttributesRequest);
}

const char * xmlif_GetURIForClient(Lwm2mClientType * client, ObjectInstanceResourceKey * key)
{
    static char uri[MAX_URI_LENGTH];
    char addr[MAX_URI_LENGTH];
    char buffer[MAX_URI_LENGTH];
    const char* ip;
    int port;

    memset(uri, 0, sizeof(uri));

    switch (client->Address.Addr.Sa.sa_family)
    {
        case AF_INET:
            ip = inet_ntop(AF_INET,&client->Address.Addr.Sin.sin_addr, buffer, sizeof(buffer));
            port = ntohs(client->Address.Addr.Sin.sin_port);
            sprintf(addr, "%s:%d", ip, port);
            break;
        case AF_INET6:
            ip = inet_ntop(AF_INET6,&client->Address.Addr.Sin6.sin6_addr, buffer, sizeof(buffer));
            port =  ntohs(client->Address.Addr.Sin6.sin6_port);
            sprintf(addr, "[%s]:%d", ip, port);
            break;
    }

    if (key->ResourceID != -1)
    {
        sprintf(uri, "coap://%s/%d/%d/%d", addr, key->ObjectID, key->InstanceID, key->ResourceID);
    }
    else if (key->InstanceID != -1)
    {
        sprintf(uri, "coap://%s/%d/%d", addr, key->ObjectID, key->InstanceID);
    }
    else
    {
        sprintf(uri, "coap://%s/%d", addr, key->ObjectID);
    }

    return (const char *)&uri[0];
}

static int xmlif_ParseRequest(RequestInfoType * request, TreeNode content, Lwm2mClientType ** client,
                              ObjectInstanceResourceKey * key, TreeNode * objectNode)
{
    int result = AwaResult_Success;
    TreeNode instance;
    TreeNode property;

    TreeNode clientNode = TreeNode_Navigate(content, "Content/Client");
    if (clientNode == NULL)
    {
        result = AwaResult_BadRequest;
        goto error;
    }

    TreeNode clientIDNode = TreeNode_Navigate(clientNode, "Client/ClientID");
    if (clientIDNode == NULL)
    {
        result = AwaResult_BadRequest;
        goto error;
    }

    const char * clientID = TreeNode_GetValue(clientIDNode);
    if (clientID == NULL)
    {
        result = AwaResult_BadRequest;
        goto error;
    }

    *client = Lwm2m_LookupClientByName(request->Context, clientID);
    if (*client == NULL)
    {
        result = AwaResult_BadRequest;
        goto error;
    }
    request->Client = *client;

    TreeNode object = TreeNode_Navigate(clientNode, "Client/Object");
    if (object == NULL)
    {
        result = AwaResult_BadRequest;
        goto error;
    }

    if ((key->ObjectID = xmlif_GetInteger(object, "Object/ObjectID")) == -1)
    {
        result = AwaResult_BadRequest;
        goto error;
    }

    if (objectNode != NULL)
    {
        *objectNode = object;
    }

    instance = TreeNode_Navigate(object, "Object/Instance");
    if (instance == NULL)
    {
        key->InstanceID = AWA_INVALID_ID;
        goto done;
    }

    if ((key->InstanceID = xmlif_GetInteger(instance, "Instance/InstanceID")) == -1)
    {
        result = AwaResult_BadRequest;
        goto error;
    }

    property = TreeNode_Navigate(instance, "Instance/Property");
    if (property == NULL)
    {
        key->ResourceID = AWA_INVALID_ID;
        goto done;
    }

    if ((key->ResourceID = xmlif_GetInteger(property, "Property/PropertyID")) == -1)
    {
        result = AwaResult_BadRequest;
        goto error;
    }
done:
error:
    return result;
}

static int xmlif_RegisterObjectFromXML(Lwm2mContextType * context, TreeNode meta)
{
    int result = AwaResult_Success;
    int res;
    ObjectIDType objectID = -1;
    const char * objectName = NULL;
    const char * value;
    TreeNode node;

    uint16_t maximumInstances = 1;
    uint16_t minimumInstances = 0;

    node = TreeNode_Navigate(meta, "ObjectMetadata/SerialisationName");
    if (node != NULL)
    {
        value = TreeNode_GetValue(node);
        if (value != NULL)
        {
            objectName = value;
        }
    }

    node = TreeNode_Navigate(meta, "ObjectMetadata/ObjectID");
    if (node != NULL)
    {
        value = TreeNode_GetValue(node);
        if (value)
        {
            objectID = atoi(value);
        }
    }

    node = TreeNode_Navigate(meta, "ObjectMetadata/MaximumInstances");
    if (node != NULL)
    {
        value = TreeNode_GetValue(node);
        if (value != NULL)
        {
            maximumInstances = atoi(value);
        }
    }

    node = TreeNode_Navigate(meta, "ObjectMetadata/MinimumInstances");
    if (node != NULL)
    {
        value = TreeNode_GetValue(node);
        if (value != NULL)
        {
            minimumInstances = atoi(value);
        }
    }

    res = Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), objectName ? objectName : "", objectID, maximumInstances, minimumInstances, NULL);
    if (res < 0)
    {
        result = AwaResult_Forbidden;
        goto error;
    }

    node = TreeNode_Navigate(meta, "ObjectMetadata/Properties");
    if (node)
    {
        TreeNode property;
        int childIndex = 0;
        while ((property = TreeNode_GetChild(node, childIndex)) != NULL)
        {
            TreeNode resNode;
            ResourceIDType resourceID = AWA_INVALID_ID;
            int dataType = -1;
            const char * resourceName = NULL;
            uint16_t resourceMaximumInstances = 1;
            uint16_t resourceMinimumInstances = 0;
            AwaResourceOperations operation = AwaResourceOperations_None;
            Lwm2mTreeNode * defaultValueNode = NULL;

            resNode = TreeNode_Navigate(property, "Property/PropertyID");
            if (resNode != NULL)
            {
                value = TreeNode_GetValue(resNode);
                if (value != NULL)
                {
                    resourceID = atoi(value);
                }
            }

            resNode = TreeNode_Navigate(property, "Property/SerialisationName");
            if (resNode != NULL)
            {
                value = TreeNode_GetValue(resNode);
                if (value != NULL)
                {
                    resourceName = value;
                }
            }

            resNode = TreeNode_Navigate(property, "Property/DataType");
            if (resNode != NULL)
            {
                value = TreeNode_GetValue(resNode);
                if (value != NULL)
                {
                    dataType = xmlif_StringToDataType(value);
                }
            }

            resNode = TreeNode_Navigate(property, "Property/MaximumInstances");
            if (resNode != NULL)
            {
                value = TreeNode_GetValue(resNode);
                if (value != NULL)
                {
                    resourceMaximumInstances = atoi(value);
                }
            }

            resNode = TreeNode_Navigate(property, "Property/MinimumInstances");
            if (resNode != NULL)
            {
                value = TreeNode_GetValue(resNode);
                if (value != NULL)
                {
                    resourceMinimumInstances = atoi(value);
                }
            }

            resNode = TreeNode_Navigate(property, "Property/Access");
            if (resNode != NULL)
            {
                value = TreeNode_GetValue(resNode);
                if (value != NULL)
                {
                    operation = xmlif_StringToOperation(value);
                }
            }

            resNode = TreeNode_Navigate(property, "Property/DefaultValue");
            if (resNode != NULL)
            {
                defaultValueNode = Lwm2mTreeNode_Create();
                Lwm2mTreeNode_SetType(defaultValueNode, Lwm2mTreeNodeType_Resource);

                value = TreeNode_GetValue(resNode);

                const uint8_t * defaultValue = NULL;
                uint16_t defaultValueLength = 0;

                if (value != NULL)
                {
                    defaultValueLength = xmlif_DecodeValue((char**)&defaultValue, dataType, value, strlen(value));
                }

                Lwm2mTreeNode * resourceInstanceNode = Lwm2mTreeNode_Create();
                Lwm2mTreeNode_AddChild(defaultValueNode, resourceInstanceNode);
                Lwm2mTreeNode_SetType(resourceInstanceNode, Lwm2mTreeNodeType_ResourceInstance);
                Lwm2mTreeNode_SetValue(resourceInstanceNode, defaultValue, defaultValueLength);
                Lwm2mTreeNode_SetID(resourceInstanceNode, 0);
                if (defaultValue != NULL)
                {
                    free((void*)defaultValue);
                }
            }
            else
            {
                resNode = TreeNode_Navigate(property, "Property/DefaultValueArray");
                if (resNode != NULL)
                {
                    defaultValueNode = Lwm2mTreeNode_Create();
                    Lwm2mTreeNode_SetType(defaultValueNode, Lwm2mTreeNodeType_Resource);

                    TreeNode resourceInstance;
                    int childIndex = 0;
                    while ((resourceInstance = Xml_FindFrom(resNode, "ResourceInstance", &childIndex)) != NULL)
                    {
                        int resourceInstanceID = xmlif_GetInteger(resourceInstance, "ResourceInstance/ID");
                        value = xmlif_GetOpaque(resourceInstance, "ResourceInstance/Value");

                        const uint8_t * defaultValue = NULL;
                        uint16_t defaultValueLength = 0;

                        defaultValueLength = xmlif_DecodeValue((char**)&defaultValue, dataType, value, strlen(value));

                        Lwm2mTreeNode * resourceInstanceNode = Lwm2mTreeNode_Create();
                        Lwm2mTreeNode_AddChild(defaultValueNode, resourceInstanceNode);
                        Lwm2mTreeNode_SetType(resourceInstanceNode, Lwm2mTreeNodeType_ResourceInstance);
                        Lwm2mTreeNode_SetValue(resourceInstanceNode, defaultValue, defaultValueLength);
                        Lwm2mTreeNode_SetID(resourceInstanceNode, resourceInstanceID);

                        free((void*)defaultValue);
                    }
                }
            }

            res = Lwm2mCore_RegisterResourceTypeWithDefaultValue(context, resourceName ? resourceName : "", objectID, resourceID,
                    dataType, resourceMaximumInstances, resourceMinimumInstances, operation, NULL, defaultValueNode);
            Lwm2mTreeNode_DeleteRecursive(defaultValueNode);
            if (res < 0)
            {
                result = AwaResult_Forbidden;
                goto error;
            }

            childIndex++;
        }
    }

error:
    return result;
}

// Called to handle a request with the type "Connect". Returns 0 on success.
static int xmlif_HandlerConnectRequest(RequestInfoType * request, TreeNode content)
{
    Lwm2mContextType * context = (Lwm2mContextType*)request->Context;
    TreeNode response = xmlif_GenerateConnectResponse(Lwm2mCore_GetDefinitions(context), request->SessionID);
    if ((response != NULL) &&
        (IPCSession_New(request->SessionID) == 0) &&
        (IPCSession_AddRequestChannel(request->SessionID, request->Sockfd, &request->FromAddr, request->AddrLen) == 0))
    {
#ifndef CONTIKI
        Lwm2m_Info("IPC connected from %s - allocated session ID %d\n", Lwm2mCore_DebugPrintSockAddr(&request->FromAddr), request->SessionID);
#endif
        IPC_SendResponse(response, request->Sockfd, &request->FromAddr, request->AddrLen);
        Tree_Delete(response);
    }
    else
    {
        TreeNode response = IPC_NewResponseNode(IPC_MESSAGE_SUB_TYPE_CONNECT, AwaResult_BadRequest, request->SessionID);
        IPC_SendResponse(response, request->Sockfd, &request->FromAddr, request->AddrLen);
        Tree_Delete(response);
    }

    free(request);
    return 0;
}

static int xmlif_HandlerEstablishNotifyRequest(RequestInfoType * request, TreeNode content)
{
    if (IPCSession_AddNotifyChannel(request->SessionID, request->Sockfd, &request->FromAddr, request->AddrLen) == 0)
    {
#ifndef CONTIKI
        Lwm2m_Info("IPC Notify session %d connected from %s\n", request->SessionID, Lwm2mCore_DebugPrintSockAddr(&request->FromAddr));
#endif
        // Set up registration callbacks for Events
        EventContext * eventContext = EventContext_New(request);
        Lwm2m_AddRegistrationEventCallback(request->Context, request->SessionID, xmlif_HandleRegistrationEvent, eventContext);

        TreeNode response = IPC_NewResponseNode(IPC_MESSAGE_SUB_TYPE_ESTABLISH_NOTIFY, AwaResult_Success, request->SessionID);
        IPC_SendResponse(response, request->Sockfd, &request->FromAddr, request->AddrLen);
        Tree_Delete(response);
    }
    else
    {
        TreeNode response = IPC_NewResponseNode(IPC_MESSAGE_SUB_TYPE_ESTABLISH_NOTIFY, AwaResult_BadRequest, request->SessionID);
        IPC_SetSessionID(response, request->SessionID);
        IPC_SendResponse(response, request->Sockfd, &request->FromAddr, request->AddrLen);
        Tree_Delete(response);
    }

    free(request);
    return 0;
}

static int xmlif_HandlerDisconnectRequest(RequestInfoType * request, TreeNode content)
{
#ifndef CONTIKI
    Lwm2m_Info("IPC disconnected from %s\n", Lwm2mCore_DebugPrintSockAddr(&request->FromAddr));
#endif
    TreeNode response = IPC_NewResponseNode(IPC_MESSAGE_SUB_TYPE_DISCONNECT, AwaResult_Success, request->SessionID);
    IPC_SendResponse(response, request->Sockfd, &request->FromAddr, request->AddrLen);
    Lwm2m_DeleteRegistrationEventCallback(request->Context, request->SessionID);
    Tree_Delete(response);

    free(request);
    return 0;
}

/* Handle incoming ListClients requests.
 * For each registered client, add a Client node to the response.
 * This will contain the Client ID, and the set of registered objects
 * and object instances known to the LWM2M Server at this point in time.
 */
static int xmlif_HandlerListClients(RequestInfoType * request, TreeNode content)
{
    int rc = 0;
    Lwm2mContextType * context = (Lwm2mContextType*)request->Context;

    TreeNode clientsNode = IPC_NewClientsNode();

    struct ListHead * i;
    ListForEach(i, Lwm2mCore_GetClientList(context))
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

    TreeNode responseNode = IPC_NewResponseNode(IPC_MESSAGE_SUB_TYPE_LIST_CLIENTS, AwaResult_Success, request->SessionID);
    TreeNode_AddChild(responseNode, contentNode);
    rc = IPC_SendResponse(responseNode, request->Sockfd, &request->FromAddr, request->AddrLen);

    Tree_Delete(responseNode);
    free(request);
    return rc;
}

static int xmlif_HandlerDefineRequest(RequestInfoType * request, TreeNode content)
{
    Lwm2mContextType * context = (Lwm2mContextType*)request->Context;

    TreeNode objectDefinitions = TreeNode_Navigate(content, "Content/ObjectDefinitions");
    TreeNode objectDefinition = (objectDefinitions) ? TreeNode_GetChild(objectDefinitions, 0) : TreeNode_Navigate(content, "Content/ObjectDefinition");
    int objectDefinitionIndex = 1;
    int successCount = 0;
    while (objectDefinition != NULL)
    {
        if (xmlif_RegisterObjectFromXML(context, objectDefinition) == AwaResult_Success)
        {
            ++successCount;
        }
        objectDefinition = (objectDefinitions) ? TreeNode_GetChild(objectDefinitions, objectDefinitionIndex++) : NULL;
    }

    TreeNode response = IPC_NewResponseNode(IPC_MESSAGE_SUB_TYPE_DEFINE, AwaResult_Success, request->SessionID);
    IPC_SendResponse(response, request->Sockfd, &request->FromAddr, request->AddrLen);
    Tree_Delete(response);

    free(request);
    return 0;
}

static void xmlif_GenerateResponse(void * ctxt, AddressType* address, const char * responsePath, int responseCode, const char * responseType)
{
    RequestInfoType * request = ctxt;
    Lwm2mContextType * lwm2mContext = (Lwm2mContextType*)request->Context;
    TreeNode response = IPC_NewResponseNode(responseType, responseCode, request->SessionID);

    Lwm2mClientType * client = address? Lwm2m_LookupClientByAddress(lwm2mContext, address) : NULL;

    if (client != NULL)
    {
        TreeNode content = Xml_CreateNode("Content");

        TreeNode clientNode = Xml_CreateNode("Client");
        TreeNode clientIDnode = Xml_CreateNodeWithValue("ClientID", "%s", client->EndPointName);
        TreeNode_AddChild(clientNode, clientIDnode);
        TreeNode_AddChild(content, clientNode);
        TreeNode_AddChild(response, content);
    }

    IPC_SendResponse(response, request->Sockfd, &request->FromAddr, request->AddrLen);
    Tree_Delete(response);
}

static int xmlif_HandleRequestHeader(RequestInfoType * request, TreeNode content,
                                     IpcCoapRequestContext ** requestContext, TreeNode * requestObjectsNode, Lwm2mClientType ** client)
{
    int rc = -1;

    *requestContext = (IpcCoapRequestContext * )malloc(sizeof(IpcCoapRequestContext));
    if (*requestContext == NULL)
    {
        Lwm2m_Error("Out of memory");
        goto error;
    }

    memset(*requestContext, 0, sizeof(IpcCoapRequestContext));
    (*requestContext)->Request = request;
    (*requestContext)->Result = AwaResult_Unspecified;
    (*requestContext)->ResponseContentNode = IPC_NewContentNode();
    (*requestContext)->Reusable = false;
    (*requestContext)->ResponseCount = 0;
    (*requestContext)->AddResultTags = true;

    TreeNode requestClientsNode = TreeNode_Navigate(content, "Content/Clients");
    if (requestClientsNode == NULL)
    {
        Lwm2m_Error("No <Clients> node in request content");
        (*requestContext)->Result = AwaResult_BadRequest;
        goto error;
    }

    TreeNode requestClientNode = TreeNode_Navigate(requestClientsNode, "Clients/Client");
    if (requestClientNode == NULL)
    {
        Lwm2m_Error("No <Client> node in request content");
        (*requestContext)->Result = AwaResult_BadRequest;
        goto error;
    }

    const char * clientID = xmlif_GetOpaque(requestClientNode, "Client/ID");
    if (clientID == NULL)
    {
        Lwm2m_Error("No Client ID in request content");
        (*requestContext)->Result = AwaResult_BadRequest;
        goto error;
    }

    // start building up the response data
    TreeNode responseClientsNode = IPC_NewClientsNode();
    TreeNode responseClientNode = IPC_AddClientNode(responseClientsNode, clientID);
    TreeNode_AddChild((*requestContext)->ResponseContentNode, responseClientsNode);

    *client = Lwm2m_LookupClientByName(request->Context, clientID);
    if (*client == NULL)
    {
        Lwm2m_Warning("No client exists for ID: %s\n", clientID);
    }

    // Add objects tree to response
    (*requestContext)->ResponseObjectsTree = ObjectsTree_New();
    TreeNode_AddChild(responseClientNode, (*requestContext)->ResponseObjectsTree);

    *requestObjectsNode = TreeNode_Navigate(requestClientNode, "Client/Objects");
    if (*requestObjectsNode == NULL)
    {
        Lwm2m_Error("No <Objects> node in request content");
        (*requestContext)->Result = AwaResult_BadRequest;
        goto error;
    }

    rc = 0;
error:
    return rc;
}

static int xmlif_BackupPartiallyBuiltResponseTree(TreeNode responseContentNode, TreeNode responseObjectsTree,
                                                  TreeNode * backupResponseContentNode, TreeNode * backupResponseObjectsTree)
{
    *backupResponseContentNode = Tree_Copy(responseContentNode);

    int clientNodeIndex = TreeNode_GetID(TreeNode_GetParent(responseObjectsTree)) - 1; // TreeNode IDs start at 1, but their index in the array starts at 0

    if (clientNodeIndex >= 0)
    {
        TreeNode clientsTree = TreeNode_Navigate(*backupResponseContentNode, "Content/Clients");
        TreeNode clientTree = TreeNode_GetChild(clientsTree, clientNodeIndex);
        *backupResponseObjectsTree = TreeNode_Navigate(clientTree, "Client/Objects");
    }
    else
    {
        Lwm2m_Error("Couldn't find client node index");
    }

    return 0;
}

static void xmlif_HandleResponse(IpcCoapRequestContext * requestContext, const char * responsePath, int coapResponseCode,
                                 const char * type, const char * subType, ContentType contentType, char * payload, size_t payloadLen, IpcCoapSuccessCallback successCallback)
{
    RequestInfoType * request = requestContext->Request;

    TreeNode backupResponseContentNode = NULL;
    TreeNode backupResponseObjectsTree = NULL;
    if (requestContext->Reusable)
    {
        // Backup the partially-built response node and restore after the response is sent, for cases where we need to be able to re-use it.
        xmlif_BackupPartiallyBuiltResponseTree(requestContext->ResponseContentNode, requestContext->ResponseObjectsTree, &backupResponseContentNode, &backupResponseObjectsTree);
    }

    int responseCode = AwaResult_Success;
    if ((requestContext->Result == AwaResult_Success) || (requestContext->Result == AwaResult_SuccessDeleted))
    {
        // IPC was parsed correctly and we sent a CoAP request. Note that this doesn't guarantee the CoAP request succeeded.
        // Modify response content based on the CoAP response received.

        if (responsePath != NULL)
        {
            Lwm2m_Debug("Response path: %s\n", responsePath);
            char * responsePathWithoutQueryString = malloc(strlen(responsePath) + 2);
            if (responsePathWithoutQueryString != NULL)
            {
                if (responsePath[0] == '/')
                {
                    strcpy(responsePathWithoutQueryString, responsePath);
                }
                else 
                {
                    responsePathWithoutQueryString[0] = '/';
                    strcpy(&responsePathWithoutQueryString[1], responsePath);
                }
            
                char * queryString = strchr(responsePathWithoutQueryString, '?');
                if (queryString != NULL)
                {
                    queryString[0] = '\0';
                }

                TreeNode pathNode = NULL;
                InternalError result = ObjectsTree_FindPathNode(requestContext->ResponseObjectsTree, responsePathWithoutQueryString, &pathNode);
                if (result == InternalError_Success)
                {
                    if (AwaResult_IsSuccess(coapResponseCode))
                    {
                        successCallback(requestContext, responsePathWithoutQueryString, coapResponseCode, pathNode, subType, contentType, payload, payloadLen);
                    }
                    else
                    {
                        IPC_AddServerResultTagToAllLeafNodes(pathNode, AwaError_LWM2MError, LWM2MError_FromCoapResponseCode(coapResponseCode));
                    }
                }
                else
                {
                    Lwm2m_Error("Could not find CoAP response path in IPC response: %s\n", responsePathWithoutQueryString);
                }
                free(responsePathWithoutQueryString);
            }
            else
            {
                Lwm2m_Error("Failed to allocate memory for response\n");
                responseCode = AwaResult_OutOfMemory;
            }
        }
        else
        {
            Lwm2m_Warning("Request path is NULL - No CoAP request was sent\n");
        }
    }
    else
    {
        Lwm2m_Error("Bad response code: %d\n", requestContext->Result);
        responseCode = AwaResult_BadRequest;
    }

    int IPCSockFd = 0;
    const struct sockaddr * IPCAddr = NULL;
    int IPCAddrLen = 0;

    // Build response/notification
    TreeNode responseNode = NULL;
    if (strcmp(type, IPC_MESSAGE_TYPE_NOTIFICATION) == 0)
    {
        if (IPCSession_GetNotifyChannel(request->SessionID, &IPCSockFd, &IPCAddr, &IPCAddrLen) == 0)
        {
            responseNode = IPC_NewNotificationNode(subType, request->SessionID);
        }
        else
        {
            Lwm2m_Error("Unable to get IPC Notify channel for session %d", request->SessionID);
            responseNode = NULL;
        }
    }
    else if (strcmp(type, IPC_MESSAGE_TYPE_RESPONSE) == 0)
    {
        if (IPCSession_GetRequestChannel(request->SessionID, &IPCSockFd, &IPCAddr, &IPCAddrLen) == 0)
        {

            responseNode = IPC_NewResponseNode(subType, responseCode, request->SessionID);
        }
        else
        {
            Lwm2m_Error("Unable to get IPC Response channel for session %d", request->SessionID);
            responseNode = NULL;
        }
    }
    else
    {
        Lwm2m_Error("Unhandled message type '%s'\n", type);
        responseNode = NULL;
    }

    if (responseNode != NULL)
    {
        TreeNode_AddChild(responseNode, requestContext->ResponseContentNode);
    }
    else
    {
        responseNode = IPC_NewResponseNode(subType, AwaResult_InternalError, request->SessionID);
    }
    IPC_SendResponse(responseNode, IPCSockFd, IPCAddr, IPCAddrLen);

    Tree_Delete(requestContext->ResponseContentNode);

    if (!requestContext->Reusable)
    {
        free(requestContext->Request);
        free(requestContext);
    }
    else
    {
        // Restore backed up partial response tree.
        // For observe, each time we receive a notification of a change, a response has to be sent.
        // The request context should only be freed when the observation is cancelled.
        requestContext->ResponseContentNode = backupResponseContentNode;
        requestContext->ResponseObjectsTree = backupResponseObjectsTree;
    }
}

static void xmlif_HandlerDefaultSuccessfulResponse(IpcCoapRequestContext * requestContext, const char * responsePath,
                                                   int coapResponseCode, TreeNode pathNode, const char * responseType, ContentType contentType, char * payload, size_t payloadLen)
{
    IPC_AddResultTagToAllLeafNodes(pathNode, AwaError_Success);
}

static int xmlif_HandleError(IpcCoapRequestContext * requestContext, Lwm2mClientType * client, TransactionCallback callback, int numCoapRequests)
{
    int rc = 0;
    if ((!AwaResult_IsSuccess(requestContext->Result)) || (client == NULL) || (numCoapRequests == 0))
    {
        if (!AwaResult_IsSuccess(requestContext->Result))
        {
            // complete bad request, so delete our response and send a 400
            Tree_Delete(requestContext->ResponseContentNode);
            requestContext->ResponseContentNode = NULL;
        }
        rc = -1;
        callback(requestContext, NULL, NULL, rc, ContentType_None, NULL, 0);
    }

    return rc;
}

static int xmlif_HandleContentRequest(RequestInfoType * request, TreeNode content,
                                      SendCoapRequestHandler requestCallback, TransactionCallback responseCallback, AwaResourceOperations validOperations)
{
    int numCoapRequests = 0;
    Lwm2mContextType * context = (Lwm2mContextType * )request->Context;
    TreeNode requestObjectsNode = NULL;
    IpcCoapRequestContext * requestContext;
    Lwm2mClientType * client;

    if (xmlif_HandleRequestHeader(request, content, &requestContext, &requestObjectsNode, &client) != 0)
    {
        goto error;
    }

    if (ObjectsTree_GetLeafCount(requestObjectsNode) > 1)
    {
        Lwm2m_Error("Unsupported: Requested multiple resources in single request");
        requestContext->Result = AwaResult_Unsupported;
        goto error;
    }

    // Check each leaf node, if it is valid send a CoAP request to get the value
    TreeNode currentLeafNode = requestObjectsNode;
    while ((currentLeafNode = ObjectsTree_GetNextLeafNode(currentLeafNode)) != NULL)
    {
        ObjectInstanceResourceKey key = { .ObjectID = AWA_INVALID_ID, .InstanceID = AWA_INVALID_ID, .ResourceID = AWA_INVALID_ID, };
        ObjectsTree_GetIDsFromLeafNode(currentLeafNode, &key.ObjectID, &key.InstanceID, &key.ResourceID);

        if (key.ObjectID == AWA_INVALID_ID)
        {
            Lwm2m_Error("No object specified: /%d/%d/%d\n", key.ObjectID, key.InstanceID, key.ResourceID);
            requestContext->Result = AwaResult_BadRequest;
            goto error;
        }

        TreeNode responseObjectNode = ObjectsTree_FindOrCreateChildNode(requestContext->ResponseObjectsTree, "Object", key.ObjectID);

        ObjectDefinition * objectDefinition = Definition_LookupObjectDefinition(Lwm2mCore_GetDefinitions(context), key.ObjectID);
        if (objectDefinition == NULL)
        {
            Lwm2m_Debug("No definition for object %d\n", key.ObjectID);
            IPC_AddServerResultTag(responseObjectNode, AwaError_LWM2MError, AwaLWM2MError_NotFound);
            continue;
        }

        if (key.InstanceID != AWA_INVALID_ID)
        {
            TreeNode responseObjectInstanceNode = ObjectsTree_FindOrCreateChildNode(responseObjectNode, "ObjectInstance", key.InstanceID);

            if (key.ResourceID != AWA_INVALID_ID)
            {
                TreeNode responseResourceNode = ObjectsTree_FindOrCreateChildNode(responseObjectInstanceNode, "Resource", key.ResourceID);

                ResourceDefinition * resourceDefinition = Definition_LookupResourceDefinition(Lwm2mCore_GetDefinitions(context), key.ObjectID, key.ResourceID);
                if (resourceDefinition != NULL)
                {
                    if (Operations_Contains(validOperations, resourceDefinition->Operation))
                    {
                        if (client != NULL)
                        {
                            if (!requestCallback(requestContext, client, &key, currentLeafNode, responseResourceNode))
                            {
                                goto error;
                            }
                            numCoapRequests++;
                        }
                        else
                        {
                            // no client exists for the given ClientID, add the error to all paths.
                            IPC_AddResultTag(responseResourceNode, AwaError_ClientNotFound);
                        }
                    }
                    else
                    {
                        Lwm2m_Debug("Object %d resource %d operation not allowed\n", key.ObjectID, key.ResourceID);
                        IPC_AddServerResultTag(responseResourceNode, AwaError_LWM2MError, AwaLWM2MError_BadRequest);
                        continue;
                    }
                }
                else
                {
                    Lwm2m_Debug("No definition for object %d resource %d\n", key.ObjectID, key.ResourceID);
                    IPC_AddServerResultTag(responseResourceNode, AwaError_LWM2MError, AwaLWM2MError_NotFound);
                    continue;
                }
            }
            else
            {
                if (client != NULL)
                {
                    if (!requestCallback(requestContext, client, &key, currentLeafNode, responseObjectInstanceNode))
                    {
                        goto error;
                    }
                    numCoapRequests++;
                }
                else
                {
                    // no client exists for the given ClientID, add the error to all paths.
                    IPC_AddResultTag(responseObjectInstanceNode, AwaError_ClientNotFound);
                }
            }
        }
        else
        {
            if (client != NULL)
            {
                if (!requestCallback(requestContext, client, &key, currentLeafNode, responseObjectNode))
                {
                    goto error;
                }
                numCoapRequests++;
            }
            else
            {
                // no client exists for the given ClientID, add the error to all paths.
                IPC_AddResultTag(responseObjectNode, AwaError_ClientNotFound);
            }
        }
    }

    requestContext->Result = AwaResult_Success;

error:
    return xmlif_HandleError(requestContext, client, responseCallback, numCoapRequests);
}

static int xmlif_HandlerReadRequest(RequestInfoType * request, TreeNode content)
{
    return xmlif_HandleContentRequest(request, content, xmlif_HandlerSendCoapReadRequest, xmlif_HandlerReadResponse, AwaResourceOperations_ReadWrite);
}

static bool xmlif_HandlerSendCoapReadRequest(IpcCoapRequestContext * requestContext, Lwm2mClientType * client,
                                             ObjectInstanceResourceKey * key, TreeNode currentLeafNode, TreeNode currentResponsePathNode)
{
    ContentType contentType = requestContext != NULL && 
                              requestContext->Request != NULL && 
                              requestContext->Request->Context != NULL ? Lwm2mCore_GetContentType((Lwm2mContextType *)requestContext->Request->Context) : ContentType_ApplicationOmaLwm2mTLV;
    coap_GetRequest(requestContext, xmlif_GetURIForClient(client, key), contentType, xmlif_HandlerReadResponse);
    return true;
}

static void xmlif_HandlerReadResponse(void * ctxt, AddressType* address, const char * responsePath, int coapResponseCode, ContentType contentType, char * payload, size_t payloadLen)
{
    IpcCoapRequestContext * requestContext = (IpcCoapRequestContext *) ctxt;
    xmlif_HandleResponse(requestContext, responsePath, coapResponseCode, IPC_MESSAGE_TYPE_RESPONSE, IPC_MESSAGE_SUB_TYPE_READ, contentType, payload, payloadLen, xmlif_HandlerSuccessfulReadResponse);
}

static void xmlif_HandlerSuccessfulReadResponse(IpcCoapRequestContext * requestContext, const char * responsePath, int coapResponseCode, TreeNode pathNode,
                                                const char * responseType, ContentType contentType, char * payload, size_t payloadLen)
{
    RequestInfoType * request = requestContext->Request;
    Lwm2mContextType * context = (Lwm2mContextType * )request->Context;
    ObjectInstanceResourceKey key = UriToOir(responsePath);
    Lwm2mTreeNode * root = NULL;

    int len;

    if (key.ResourceID != -1)
    {
        len = DeserialiseResource(contentType, &root, Lwm2mCore_GetDefinitions(context), key.ObjectID, key.InstanceID, key.ResourceID, payload, payloadLen);
    }
    else if (key.InstanceID != -1)
    {
        len = DeserialiseObjectInstance(contentType, &root, Lwm2mCore_GetDefinitions(context), key.ObjectID, key.InstanceID, payload, payloadLen);
    }
    else
    {
        len = DeserialiseObject(contentType, &root, Lwm2mCore_GetDefinitions(context), key.ObjectID, payload, payloadLen);
    }

    if (len >= 0)
    {
        int serialiseResult = 0;
        if (key.ResourceID != -1)
        {
            serialiseResult = xmlif_SerialiseResourceIntoExistingObjectsTree(root, pathNode, Lwm2mCore_GetDefinitions(context), key.ObjectID, key.InstanceID, key.ResourceID);
        }
        else if (key.InstanceID != -1)
        {
            serialiseResult = xmlif_SerialiseObjectInstanceIntoExistingObjectsTree(root, pathNode, Lwm2mCore_GetDefinitions(context), key.ObjectID, key.InstanceID);
        }
        else
        {
            serialiseResult = xmlif_SerialiseObjectIntoExistingObjectsTree(root, pathNode, Lwm2mCore_GetDefinitions(context), key.ObjectID);
        }

        if (serialiseResult == 0)
        {
            if (requestContext->AddResultTags)
            {
                IPC_AddServerResultTagToAllLeafNodes(pathNode, AwaError_Success, AwaLWM2MError_Success);
            }
        }
        else
        {
            Lwm2m_Error("Serialise to XML error\n");
            if (requestContext->AddResultTags)
            {
                IPC_AddResultTagToAllLeafNodes(pathNode, AwaError_Internal);
            }
        }
    }
    else
    {
        Lwm2m_Error("Deserialise from internal tree error\n");
        if (requestContext->AddResultTags)
        {
            IPC_AddResultTagToAllLeafNodes(pathNode, AwaError_Internal);
        }
    }

    Lwm2mTreeNode_DeleteRecursive(root);
}

static int xmlif_HandlerObserveRequest(RequestInfoType * request, TreeNode content)
{
    return xmlif_HandleContentRequest(request, content, xmlif_HandlerSendCoapObserveRequest, xmlif_HandlerObserveResponse, AwaResourceOperations_ReadWrite);
}

static bool xmlif_HandlerSendCoapObserveRequest(IpcCoapRequestContext * requestContext, Lwm2mClientType * client,
                                                ObjectInstanceResourceKey * key, TreeNode currentLeafNode, TreeNode currentResponsePathNode)
{
    bool result = true;
    ContentType contentType = ((requestContext != NULL) &&
                              (requestContext->Request != NULL) &&
                              (requestContext->Request->Context != NULL)) ? Lwm2mCore_GetContentType((Lwm2mContextType *)requestContext->Request->Context) : ContentType_ApplicationOmaLwm2mTLV;
    TreeNode observeTypeNode = NULL;
    if ((observeTypeNode = Xml_Find(currentLeafNode, IPC_MESSAGE_TAG_OBSERVE)) != NULL)
    {
        TreeNode_AddChild(currentResponsePathNode, Tree_Copy(observeTypeNode));
        coap_Observe(requestContext, xmlif_GetURIForClient(client, key), contentType, xmlif_HandlerObserveResponse, xmlif_HandlerFreeIpcCoapRequestContext);
    }
    else if ((observeTypeNode = Xml_Find(currentLeafNode, IPC_MESSAGE_TAG_CANCEL_OBSERVATION)) != NULL)
    {
        TreeNode_AddChild(currentResponsePathNode, Tree_Copy(observeTypeNode));
        coap_CancelObserve(requestContext, xmlif_GetURIForClient(client, key), contentType, xmlif_HandlerCancelObserveResponse);
    }
    else
    {
        Lwm2m_Error("No <Observe> or <CancelObserve> tag within leaf node\n");
        if (requestContext != NULL)
        {
            requestContext->Result = AwaResult_BadRequest;
        }
        result = false;
    }
    return result;
}

static void xmlif_HandlerFreeIpcCoapRequestContext(void * ctxt)
{
    IpcCoapRequestContext * requestContext = (IpcCoapRequestContext * )ctxt;
    free(requestContext->Request);
    Tree_Delete(requestContext->ResponseContentNode);
    free(requestContext);
}

static void xmlif_HandlerObserveResponse(void * ctxt, AddressType* address, const char * responsePath, int coapResponseCode, ContentType contentType, char * payload, size_t payloadLen)
{
    IpcCoapRequestContext * requestContext = (IpcCoapRequestContext *) ctxt;
    bool successfulResponse = (coapResponseCode >= 200) && (coapResponseCode < 300);
    requestContext->Reusable = successfulResponse; // Context must only be freed when the notification is removed, or we received a bad request.

    if (++requestContext->ResponseCount == 1)
    {
        // first response means we're confirming an observation. Should not contain values, only path results.
        requestContext->AddResultTags = true;
        xmlif_HandleResponse(requestContext, responsePath, coapResponseCode, IPC_MESSAGE_TYPE_RESPONSE, IPC_MESSAGE_SUB_TYPE_OBSERVE, contentType, payload, payloadLen, xmlif_HandlerDefaultSuccessfulResponse);
    }
    if (successfulResponse)
    {
        // it's a notification containing value changes, or if this is the first response then it's the current value of the resources.
        requestContext->AddResultTags = false;
        xmlif_HandleResponse(requestContext, responsePath, coapResponseCode, IPC_MESSAGE_TYPE_NOTIFICATION, IPC_MESSAGE_SUB_TYPE_OBSERVE, contentType, payload, payloadLen, xmlif_HandlerSuccessfulNotifyResponse);
    }
}

static void xmlif_HandlerSuccessfulNotifyResponse(IpcCoapRequestContext * requestContext, const char * responsePath, int coapResponseCode,
                                                  TreeNode pathNode, const char * responseType, ContentType contentType, char * payload, size_t payloadLen)
{
    xmlif_HandlerSuccessfulReadResponse(requestContext, responsePath, coapResponseCode, pathNode, responseType, contentType, payload, payloadLen);
}

static void xmlif_HandlerCancelObserveResponse(void * ctxt, AddressType* address, const char * responsePath,
                                               int coapResponseCode, ContentType contentType, char * payload, size_t payloadLen)
{
    IpcCoapRequestContext * requestContext = (IpcCoapRequestContext *) ctxt;
    xmlif_HandleResponse(requestContext, responsePath, coapResponseCode, IPC_MESSAGE_TYPE_RESPONSE, IPC_MESSAGE_SUB_TYPE_OBSERVE, contentType, payload, payloadLen, xmlif_HandlerDefaultSuccessfulResponse);
}

static int xmlif_HandlerDeleteRequest(RequestInfoType * request, TreeNode content)
{
    Lwm2mClientType * client;
    IpcCoapRequestContext * requestContext;
    TreeNode requestObjectsNode = NULL;
    int numCoapRequests = 0;

    if (xmlif_HandleRequestHeader(request, content, &requestContext, &requestObjectsNode, &client) != 0)
    {
        goto error;
    }

    if (ObjectsTree_GetLeafCount(requestObjectsNode) > 1)
    {
        Lwm2m_Error("Unsupported: Requested multiple resources in single request");
        requestContext->Result = AwaResult_Unsupported;
        goto error;
    }

    TreeNode currentLeafNode = requestObjectsNode;
    while ((currentLeafNode = ObjectsTree_GetNextLeafNode(currentLeafNode)) != NULL)
    {
        ObjectInstanceResourceKey key = { .ObjectID = AWA_INVALID_ID, .InstanceID = AWA_INVALID_ID, .ResourceID = AWA_INVALID_ID, };
        ObjectsTree_GetIDsFromLeafNode(currentLeafNode, &key.ObjectID, &key.InstanceID, &key.ResourceID);

        TreeNode responseObjectNode = ObjectsTree_FindOrCreateChildNode(requestContext->ResponseObjectsTree, "Object", key.ObjectID);

        if(key.InstanceID != -1)
        {
            TreeNode responseObjectInstanceNode = ObjectsTree_FindOrCreateChildNode(responseObjectNode, "ObjectInstance", key.InstanceID);

            if(key.ResourceID != -1)
            {
                TreeNode responseResourceNode = ObjectsTree_FindOrCreateChildNode(responseObjectInstanceNode, "Resource", key.ResourceID);
                // method not allowed - only instances can be deleted
                Lwm2m_Debug("Delete on /%d/%d/%d not allowed - only object instances can be deleted\n", key.ObjectID, key.InstanceID, key.ResourceID);
                IPC_AddServerResultTag(responseResourceNode, AwaError_LWM2MError, AwaLWM2MError_MethodNotAllowed);
            }
            else if (client != NULL)
            {
                coap_DeleteRequest(requestContext, xmlif_GetURIForClient(client, &key), xmlif_HandlerDeleteResponse);
                numCoapRequests++;
            }
            else
            {
                // no client exists for the given ClientID, add the error to all paths.
                IPC_AddResultTag(responseObjectInstanceNode, AwaError_ClientNotFound);
            }
        }
        else
        {
            // method not allowed - only instances can be deleted
            Lwm2m_Debug("Delete on /%d not allowed - only object instances can be deleted\n", key.ObjectID);
            IPC_AddServerResultTag(responseObjectNode, AwaError_LWM2MError, AwaLWM2MError_MethodNotAllowed);
        }
    }
    requestContext->Result = AwaResult_SuccessDeleted;

error:
    return xmlif_HandleError(requestContext, client, xmlif_HandlerDeleteResponse, numCoapRequests);
}

static void xmlif_HandlerDeleteResponse(void * ctxt, AddressType* address, const char * responsePath, int coapResponseCode, ContentType contentType, char * payload, size_t payloadLen)
{
    IpcCoapRequestContext * requestContext = (IpcCoapRequestContext *) ctxt;
    xmlif_HandleResponse(requestContext, responsePath, coapResponseCode, IPC_MESSAGE_TYPE_RESPONSE, IPC_MESSAGE_SUB_TYPE_DELETE, contentType, payload, payloadLen, xmlif_HandlerDefaultSuccessfulResponse);
}

static int xmlif_HandlerDiscoverRequest(RequestInfoType * request, TreeNode content)
{
    int result;
    ObjectInstanceResourceKey key = {-1,-1,-1};
    Lwm2mClientType * client;

    result = xmlif_ParseRequest(request, content, &client, &key, NULL);
    if (result == AwaResult_Success)
    {
        coap_GetRequest(request, xmlif_GetURIForClient(client, &key), ContentType_ApplicationLinkFormat, xmlif_HandlerDiscoverResponse);
    }
    else
    {
        xmlif_GenerateResponse(request, NULL, NULL, result, IPC_MESSAGE_SUB_TYPE_DISCOVER);
        free(request);
    }
    return result;
}

static void xmlif_HandlerDiscoverResponse(void * ctxt, AddressType* address, const char * responsePath, int responseCode, ContentType contentType, char * payload, size_t payloadLen)
{
    xmlif_GenerateResponse(ctxt, address, responsePath, responseCode, IPC_MESSAGE_SUB_TYPE_DISCOVER);
    free(ctxt);
}

static int xmlif_DecodeValueNode(TreeNode source, Lwm2mTreeNode * destination, ResourceDefinition * resourceDefinition)
{
    int result = AwaResult_Success;
    const char * data;
    int dataLength;
    char * dataValue = NULL;
    if ((data = (char*)TreeNode_GetValue(source)) == NULL)
    {
        Lwm2m_Error("Missing value data for resource\n");
        result = AwaResult_BadRequest;
        goto error;
    }

    dataLength = xmlif_DecodeValue(&dataValue, resourceDefinition->Type, data, strlen(data));
    if (dataLength < 0)
    {
        char path[MAX_PATH_LENGTH];
        Lwm2m_Error("Failed to decode value data for resource %s\n", ObjectsTree_GetPath(source, path, sizeof(path)));
        free(dataValue);
        result = AwaResult_GetLastResult();
        goto error;
    }
    Lwm2mTreeNode_SetValue(destination, dataValue, dataLength);
    free(dataValue);

error:
    return result;
}

static int xmlif_AddDefaultsForMissingMandatoryValues(Lwm2mContextType * context, Lwm2mTreeNode * node)
{
    int result = -1;
    int objectID = 0;

    switch(Lwm2mTreeNode_GetType(node))
    {
        case Lwm2mTreeNodeType_Object:
            Lwm2mTreeNode_GetID(node, &objectID);
            break;
        case Lwm2mTreeNodeType_ObjectInstance:
            Lwm2mTreeNode_GetID(TreeNode_GetParent(node), &objectID);
            break;
        default:
            Lwm2m_Error("Unexpected node type: %d\n", Lwm2mTreeNode_GetType(node));
            result = 1;
            goto error;
    }

    int resourceID = -1;
    while ((resourceID = Definition_GetNextResourceType(Lwm2mCore_GetDefinitions(context), objectID, resourceID)) != -1)
    {
        ResourceDefinition * resourceDefinition = Definition_LookupResourceDefinition(Lwm2mCore_GetDefinitions(context), objectID, resourceID);
        if (resourceDefinition != NULL)
        {
            if (resourceDefinition->Type == AwaResourceType_None)
            {
                continue;
            }
            if (resourceDefinition->DefaultValueNode == NULL && resourceDefinition->MaximumInstances > 1 && resourceDefinition->MinimumInstances == 0)
            {
                continue;  // don't add default values for optional multiple instance resources
            }
            if (resourceDefinition->MinimumInstances == 0)
            {
                continue;  // don't add default values for optional resources
            }

            Lwm2mTreeNode * child = Lwm2mTreeNode_GetFirstChild(node);
            while(child != NULL)
            {
                int childID;
                Lwm2mTreeNode_GetID(child, &childID);
                if (childID == resourceID)
                {
                    break;  // found
                }
                child = Lwm2mTreeNode_GetNextChild(node, child);
            }
            if (child == NULL)
            {
                if (resourceDefinition->DefaultValueNode != NULL)
                {
                    child = Lwm2mTreeNode_CopyRecursive(resourceDefinition->DefaultValueNode);

                    if (child != NULL)
                    {
                        Lwm2mTreeNode_SetID(child, resourceID);
                        Lwm2mTreeNode_SetType(child, Lwm2mTreeNodeType_Resource);
                        Lwm2mTreeNode_SetDefinition(child, resourceDefinition);
                        Lwm2mTreeNode_AddChild(node, child);
                    }
                    else
                    {
                        Lwm2m_Error("Failed to copy default value node\n");
                        Lwm2mTreeNode_DeleteRecursive(child);
                        goto error;
                    }
                }
                else
                {
                    child = Lwm2mTreeNode_Create();
                    if (child != NULL)
                    {
                        Lwm2mTreeNode_SetID(child, resourceID);
                        Lwm2mTreeNode_SetType(child, Lwm2mTreeNodeType_Resource);
                        Lwm2mTreeNode_SetDefinition(child, resourceDefinition);
                        Lwm2mTreeNode_AddChild(node, child);

                        ResourceInstanceIDType resourceInstanceID = 0;
                        int minimumInstances = resourceDefinition->MinimumInstances > 0 ? resourceDefinition->MinimumInstances : 1;
                        for (; resourceInstanceID < minimumInstances; resourceInstanceID++)
                        {
                            const void * defaultData = NULL;
                            int defaultLen = 0;
                            if (Definition_AllocSensibleDefault(resourceDefinition, &defaultData, &defaultLen) == 0)
                            {
                                TreeNode resourceInstance = Lwm2mTreeNode_Create();
                                Lwm2mTreeNode_SetID(resourceInstance, resourceInstanceID);
                                Lwm2mTreeNode_SetType(resourceInstance, Lwm2mTreeNodeType_ResourceInstance);
                                Lwm2mTreeNode_SetValue(resourceInstance, defaultData, defaultLen);
                                Lwm2mTreeNode_AddChild(child, resourceInstance);
                            }
                            else
                            {
                                Lwm2m_Error("Failed to allocate sensible default for object %d resource %d (resource instance %d)\n", objectID, resourceID, resourceInstanceID);
                                Lwm2mTreeNode_DeleteRecursive(child);
                                goto error;
                            }
                        }
                    }
                    else
                    {
                        Lwm2m_Error("Failed to create child node\n");
                        goto error;
                    }
                }
                Lwm2m_Debug("Added default value to create request for resource: %d\n", resourceID);
            }
        }
        else
        {
            Lwm2m_Error("Resource definition not found for object ID %d, resource ID %d\n", objectID, resourceID);
        }
    }
    result = 0;
error:
    return result;
}

static int xmlif_SendCoapCreateRequest(RequestInfoType * request, Lwm2mClientType * client, Lwm2mTreeNode * node,
                                       TransactionCallback callback, void * context, AwaWriteMode writeMode)
{
    int len = -1;
    int objectID = 0;

    char payload[MAX_PAYLOAD_SIZE];
    ContentType contentType = request != NULL && request->Context != NULL ? Lwm2mCore_GetContentType((Lwm2mContextType *)request->Context) : ContentType_ApplicationOmaLwm2mTLV;

    // node can be either an object (no id specified - will be generated by client) or an object instance (id specified)
    if (Lwm2mTreeNode_GetType(Lwm2mTreeNode_GetParent(node)) == Lwm2mTreeNodeType_Root)
    {
        // no object instance ID specified - client needs to generate one.
        // this is actually an object node with resource values but no object instance.
        // temporarily set it as an object instance node so it can be serialised correctly.
        Lwm2mTreeNode_SetType(node, Lwm2mTreeNodeType_ObjectInstance);
        Lwm2mTreeNode_GetID(node, &objectID);
        len = SerialiseObjectInstance(contentType, node, objectID, -1, payload, sizeof(payload));
    }
    else
    {
        Lwm2mTreeNode_GetID(TreeNode_GetParent(node), &objectID);
        len = SerialiseObject(contentType, TreeNode_GetParent(node), objectID, payload, sizeof(payload));
    }

    if (len >= 0)
    {
        ObjectInstanceResourceKey key = { .ObjectID = objectID, .InstanceID = -1, .ResourceID = -1, };
        coap_PostRequest(context, xmlif_GetURIForClient(client, &key), contentType, payload, len, callback);
    }
    return len;
}

static int xmlif_SendCoapWriteRequest(RequestInfoType * request, Lwm2mClientType * client, Lwm2mTreeNode * objectInstanceNode,
                                      TransactionCallback callback, void * context, AwaWriteMode writeMode)
{
    int len = -1;
    ObjectInstanceResourceKey key = { .ObjectID = -1, .InstanceID = -1, .ResourceID = -1, };

    if (Lwm2mTreeNode_GetID(objectInstanceNode, &key.InstanceID) == -1)
    {
        Lwm2m_Error("Object Instance node has no ID");
        goto error;
    }

    if (Lwm2mTreeNode_GetID(TreeNode_GetParent(objectInstanceNode), &key.ObjectID) == -1)
    {
        Lwm2m_Error("Object node has no ID");
        goto error;
    }

    ContentType contentType = request != NULL && request->Context != NULL ? Lwm2mCore_GetContentType((Lwm2mContextType *)request->Context) : ContentType_ApplicationOmaLwm2mTLV;

    char payload[MAX_PAYLOAD_SIZE];
    if (Lwm2mTreeNode_GetChildCount(objectInstanceNode) == 1 && writeMode == AwaWriteMode_Replace)
    {
        // can only ensure the resource endpoint exists if we are doing a PUT
        // single resource, write to /O/I/R
        Lwm2mTreeNode * resource = Lwm2mTreeNode_GetFirstChild(objectInstanceNode);
        if (Lwm2mTreeNode_GetID(resource, &key.ResourceID) != 0)
        {
            Lwm2m_Error("Failed to retrieve ID from resource node");
            goto error;
        }

        len = SerialiseResource(contentType, resource, key.ObjectID, key.InstanceID, key.ResourceID, payload, sizeof(payload));

        if (len <= 0)
        {
            Lwm2m_Error("Failed to serialise resource: /%d/%d/%d\n", key.ObjectID, key.InstanceID, key.ResourceID);
            goto error;
        }
    }
    else
    {
        // We are sending an object instance, so ignore the resource id.
        len = SerialiseObjectInstance(contentType, objectInstanceNode, key.ObjectID, key.InstanceID, payload, sizeof(payload));

        if (len <= 0)
        {
            Lwm2m_Error("Failed to serialise object instance: /%d/%d\n", key.ObjectID, key.InstanceID);
            goto error;
        }
    }

    switch(writeMode)
    {
        case AwaWriteMode_Replace:
            coap_PutRequest(context, xmlif_GetURIForClient(client, &key), contentType, payload, len, callback);
            break;
        case AwaWriteMode_Update:
            coap_PostRequest(context, xmlif_GetURIForClient(client, &key), contentType, payload, len, callback);
            break;
        default:
            Lwm2m_Error("Invalid write mode: %s\n", AwaWriteMode_ToString(writeMode));
            len = -1;
            break;
    }

error:
    return len;
}

static int xmlif_HandlerWriteRequest(RequestInfoType * request, TreeNode content)
{
    Lwm2mContextType * context = (Lwm2mContextType * )request->Context;
    TreeNode requestObjectsNode = NULL;
    IpcCoapRequestContext * requestContext;
    Lwm2mClientType * client;
    int numCoapRequests = 0;

    if (xmlif_HandleRequestHeader(request, content, &requestContext, &requestObjectsNode, &client) != 0)
    {
        goto error;
    }

    Lwm2mTreeNode * root = NULL;

    TreeNode requestClientNode = TreeNode_GetParent(requestObjectsNode);

    AwaWriteMode defaultWriteMode = AwaWriteMode_LAST;
    const char * defaultWriteModeString = xmlif_GetOpaque(requestClientNode, "Client/DefaultWriteMode");
    if (defaultWriteModeString != NULL)
    {
        defaultWriteMode = AwaWriteMode_FromString(defaultWriteModeString);
    }
    if (defaultWriteMode == AwaWriteMode_LAST)
    {
        Lwm2m_Error("Invalid default write mode: %s\n", defaultWriteModeString);
        goto error;
    }

    root = Lwm2mTreeNode_Create();
    Lwm2mTreeNode_SetType(root, Lwm2mTreeNodeType_Root);

    // Read each leaf node and if it is valid place its value in the internal tree to serialize
    TreeNode currentLeafNode = requestObjectsNode;
    while ((currentLeafNode = ObjectsTree_GetNextLeafNode(currentLeafNode)) != NULL)
    {
        ObjectInstanceResourceKey key = { .ObjectID = AWA_INVALID_ID, .InstanceID = AWA_INVALID_ID, .ResourceID = AWA_INVALID_ID, };
        ObjectsTree_GetIDsFromLeafNode(currentLeafNode, &key.ObjectID, &key.InstanceID, &key.ResourceID);

        TreeNode responseObjectNode = ObjectsTree_FindOrCreateChildNode(requestContext->ResponseObjectsTree, "Object", key.ObjectID);
        TreeNode responseObjectInstanceNode = key.InstanceID != AWA_INVALID_ID? ObjectsTree_FindOrCreateChildNode(responseObjectNode, "ObjectInstance", key.InstanceID) : NULL;

        ObjectDefinition * objectDefinition = Definition_LookupObjectDefinition(Lwm2mCore_GetDefinitions(context), key.ObjectID);
        if (objectDefinition == NULL)
        {
            Lwm2m_Debug("No definition for object %d\n", key.ObjectID);
            IPC_AddServerResultTag(responseObjectNode, AwaError_LWM2MError, AwaLWM2MError_NotFound);
            continue;
        }

        bool createObjectInstance = false;

        // For create, leaves can be resources, with the create tag in their parent object instance.
        // Otherwise, create tag will be in the object, or object instance - with no children,
        // meaning that we will use sensible or resource defaults for every resource in the created object instance.
        if (ObjectsTree_IsObjectNode(currentLeafNode))
        {
            createObjectInstance = Xml_Find(currentLeafNode, IPC_MESSAGE_TAG_CREATE);
            if (createObjectInstance)
            {
                Lwm2mTreeNode_FindOrCreateChildNode(root, key.ObjectID, Lwm2mTreeNodeType_Object, objectDefinition, true);
            }
            else
            {
                // Both object ID and instance ID are required for a standard write.
                Lwm2m_Error("No object instance specified: /%d/%d/%d\n", key.ObjectID, key.InstanceID, key.ResourceID);
                requestContext->Result = AwaResult_BadRequest;
                goto error;
            }
        }
        else if (ObjectsTree_IsObjectInstanceNode(currentLeafNode))
        {
            createObjectInstance = Xml_Find(currentLeafNode, IPC_MESSAGE_TAG_CREATE);
            if (createObjectInstance)
            {
                Lwm2mTreeNode * object = Lwm2mTreeNode_FindOrCreateChildNode(root, key.ObjectID, Lwm2mTreeNodeType_Object, objectDefinition, false);
                Lwm2mTreeNode_FindOrCreateChildNode(object, key.InstanceID, Lwm2mTreeNodeType_ObjectInstance, objectDefinition, true);
            }
            else
            {
                // No resources in this object, skip
                continue;
            }
        }
        else // Resource
        {
            TreeNode parent = TreeNode_GetParent(currentLeafNode);
            createObjectInstance = Xml_Find(parent, IPC_MESSAGE_TAG_CREATE);
        }

        if (createObjectInstance && client != NULL)
        {
            /* The Object Instance that is created in the LWM2M Client by the LWM2M Server MUST be an Object type supported
             * by the LWM2M Client and announced to the LWM2M Server using the “Register” and “Update” operations
             * of the LWM2M Client Registration Interface.
             */
            if (!Lwm2m_ClientSupportsObject(client, key.ObjectID, AWA_INVALID_ID))
            {
                Lwm2m_Error("Attempting to write to an instance of object type %d which is not supported by the client\n", key.ObjectID);

                IPC_AddServerResultTag(responseObjectInstanceNode? responseObjectInstanceNode : responseObjectNode, AwaError_LWM2MError, AwaLWM2MError_MethodNotAllowed);
                continue;
            }
        }

        if (key.ResourceID != AWA_INVALID_ID)
        {
            TreeNode responseResourceNode = ObjectsTree_FindOrCreateChildNode(responseObjectInstanceNode, "Resource", key.ResourceID);

            ResourceDefinition * resourceDefinition = Definition_LookupResourceDefinition(Lwm2mCore_GetDefinitions(context), key.ObjectID, key.ResourceID);
            if (resourceDefinition != NULL)
            {
                Lwm2mTreeNode * objectNode = Lwm2mTreeNode_FindOrCreateChildNode(root, key.ObjectID, Lwm2mTreeNodeType_Object, objectDefinition, false);
                Lwm2mTreeNode * objectInstanceNode = Lwm2mTreeNode_FindOrCreateChildNode(objectNode, key.InstanceID, Lwm2mTreeNodeType_ObjectInstance, NULL, createObjectInstance);
                Lwm2mTreeNode * resourceNode = Lwm2mTreeNode_FindOrCreateChildNode(objectInstanceNode, key.ResourceID, Lwm2mTreeNodeType_Resource, resourceDefinition, false);

                if (client != NULL)
                {
                    if (resourceDefinition->MaximumInstances == 1)
                    {
                        // add a single instance resource value
                        TreeNode valueNode = NULL;
                        if ((valueNode = Xml_Find(currentLeafNode, "Value")) != NULL)
                        {
                            // add the value within a resource instance because the core still treats
                            // single instance resources as a resource with a single resource instance.
                            Lwm2mTreeNode * resourceInstanceNode = Lwm2mTreeNode_Create();
                            int resourceInstanceID = 0;
                            Lwm2mTreeNode_SetID(resourceInstanceNode, resourceInstanceID);
                            Lwm2mTreeNode_SetType(resourceInstanceNode, Lwm2mTreeNodeType_ResourceInstance);
                            Lwm2mTreeNode_AddChild(resourceNode, resourceInstanceNode);
                            if ((requestContext->Result = xmlif_DecodeValueNode(valueNode, resourceInstanceNode, resourceDefinition)) != AwaResult_Success)
                            {
                                Lwm2m_Error("Failed to decode resource value");
                                goto error;
                            }
                        }
                        else
                        {
                            Lwm2m_Error("No Value node in resource");
                            requestContext->Result = AwaResult_BadRequest;
                            goto error;
                        }
                    }
                    else
                    {
                        // Loop through and add all resource instances
                        uint32_t valueIndex = 0;
                        TreeNode requestResourceInstance = NULL;
                        while ((requestResourceInstance = Xml_FindFrom(currentLeafNode, "ResourceInstance", &valueIndex)) != NULL)
                        {
                            int resourceInstanceID = xmlif_GetInteger(requestResourceInstance, "ResourceInstance/ID");

                            if (resourceInstanceID < 0)
                            {
                                Lwm2m_Error("No ID node in resource instance");
                                requestContext->Result = AwaResult_BadRequest;
                                goto error;
                            }
                            TreeNode valueNode = NULL;
                            if ((valueNode = Xml_Find(requestResourceInstance, "Value")) != NULL) // resourceNode.hasChild("Value")
                            {
                                Lwm2mTreeNode * resourceInstanceNode = Lwm2mTreeNode_Create();
                                Lwm2mTreeNode_SetID(resourceInstanceNode, resourceInstanceID);
                                Lwm2mTreeNode_SetType(resourceInstanceNode, Lwm2mTreeNodeType_ResourceInstance);
                                Lwm2mTreeNode_AddChild(resourceNode, resourceInstanceNode);

                                if ((requestContext->Result = xmlif_DecodeValueNode(valueNode, resourceInstanceNode, resourceDefinition)) != AwaResult_Success)
                                {
                                    Lwm2m_Error("Failed to decode resource instance value");
                                    goto error;
                                }
                            }
                            else
                            {
                                Lwm2m_Error("No Value node in resource instance");
                                requestContext->Result = AwaResult_BadRequest;
                                goto error;
                            }
                        }
                    }
                }
                else
                {
                    // no client exists for the given ClientID, add the error to all paths.
                    IPC_AddResultTag(responseResourceNode, AwaError_ClientNotFound);
                }
            }
            else
            {
                Lwm2m_Debug("No definition for object %d resource %d\n", key.ObjectID, key.ResourceID);
                IPC_AddServerResultTag(responseResourceNode, AwaError_LWM2MError, AwaLWM2MError_NotFound);
                continue;
            }

        }
    }

    {
        // Currently we do not support writing to multiple object instances as they
        // each require a separate CoAP request, meaning we will have to wait for
        // each CoAP response before we can send a response back through the IPC layer.
        int numObjectInstances = 0;
        Lwm2mTreeNode * objectNode = Lwm2mTreeNode_GetFirstChild(root);
        while(objectNode != NULL)
        {
            Lwm2mTreeNode * objectInstanceNode = Lwm2mTreeNode_GetFirstChild(objectNode);
            while(objectInstanceNode != NULL)
            {
                numObjectInstances++;
                objectInstanceNode = Lwm2mTreeNode_GetNextChild(objectNode, objectInstanceNode);
            }
            objectNode = Lwm2mTreeNode_GetNextChild(root, objectNode);
        }

        if (numObjectInstances > 1)
        {
            Lwm2m_Error("Unsupported: Multiple object instances in single request");
            requestContext->Result = AwaResult_Unsupported;
            goto error;
        }
    }

    requestContext->Result = AwaResult_Success;

    if (client != NULL)
    {
        // Iterate through the root node and send a coap request for each object instance
        Lwm2mTreeNode * objectNode = Lwm2mTreeNode_GetFirstChild(root);
        while(objectNode != NULL)
        {
            int objectID;
            Lwm2mTreeNode_GetID(objectNode, &objectID);
            TreeNode responseObjectNode = ObjectsTree_FindOrCreateChildNode(requestContext->ResponseObjectsTree, "Object", objectID);

            bool createObjectWithoutSpecifyingInstanceID = Lwm2mTreeNode_IsCreateFlagSet(objectNode);
            if (createObjectWithoutSpecifyingInstanceID)
            {
                if (xmlif_AddDefaultsForMissingMandatoryValues(context, objectNode) == 0)
                {
                    if (xmlif_SendCoapCreateRequest(request, client, objectNode, xmlif_HandlerCreateResponse, requestContext, defaultWriteMode) != -1)
                    {
                        numCoapRequests++;
                    }
                    else
                    {
                        IPC_AddResultTag(responseObjectNode, AwaError_Internal);
                        continue;
                    }
                }
                else
                {
                    IPC_AddResultTag(responseObjectNode, AwaError_Internal);
                    continue;
                }
            }
            else
            {
                Lwm2mTreeNode * objectInstanceNode = Lwm2mTreeNode_GetFirstChild(objectNode);
                int objectInstanceID;
                Lwm2mTreeNode_GetID(objectInstanceNode, &objectInstanceID);
                TreeNode responseObjectInstanceNode = ObjectsTree_FindOrCreateChildNode(responseObjectNode, "ObjectInstance", objectInstanceID);

                while(objectInstanceNode != NULL)
                {
                    if (Lwm2mTreeNode_IsCreateFlagSet(objectInstanceNode))
                    {
                        if (xmlif_AddDefaultsForMissingMandatoryValues(context, objectInstanceNode) == 0)
                        {
                            if (xmlif_SendCoapCreateRequest(request, client, objectInstanceNode, xmlif_HandlerCreateResponse, requestContext, defaultWriteMode) != -1)
                            {
                                numCoapRequests++;
                            }
                            else
                            {
                                IPC_AddResultTag(responseObjectInstanceNode, AwaError_Internal);
                            }
                        }
                        else
                        {
                            IPC_AddResultTag(responseObjectInstanceNode, AwaError_Internal);
                        }
                    }
                    else
                    {
                        if (xmlif_SendCoapWriteRequest(request, client, objectInstanceNode, xmlif_HandlerWriteResponse, requestContext, defaultWriteMode) != -1)
                        {
                            numCoapRequests++;
                        }
                        else
                        {
                            IPC_AddResultTag(responseObjectInstanceNode, AwaError_Internal);
                        }
                    }

                    objectInstanceNode = Lwm2mTreeNode_GetNextChild(objectNode, objectInstanceNode);
                }
            }
            objectNode = Lwm2mTreeNode_GetNextChild(root, objectNode);
        }
    }

error:
    Lwm2mTreeNode_DeleteRecursive(root);
    return xmlif_HandleError(requestContext, client, xmlif_HandlerWriteResponse, numCoapRequests);
}

static void xmlif_HandlerCreateResponse(void * ctxt, AddressType* address, const char * responsePath, int coapResponseCode, ContentType contentType, char * payload, size_t payloadLen)
{
    IpcCoapRequestContext * requestContext = (IpcCoapRequestContext *) ctxt;

    if ((coapResponseCode == AwaResult_SuccessCreated) && (!ObjectsTree_ContainsPath(requestContext->ResponseObjectsTree, responsePath)))
    {
        // add path to response
        TreeNode resultNode;
        ObjectsTree_AddPath(requestContext->ResponseObjectsTree, responsePath, &resultNode);
    }

    xmlif_HandleResponse(requestContext, responsePath, coapResponseCode, IPC_MESSAGE_TYPE_RESPONSE, IPC_MESSAGE_SUB_TYPE_WRITE, contentType, payload, payloadLen, xmlif_HandlerDefaultSuccessfulResponse);
}

static void xmlif_HandlerWriteResponse(void * ctxt, AddressType* address, const char * responsePath, int coapResponseCode, ContentType contentType, char * payload, size_t payloadLen)
{
    IpcCoapRequestContext * requestContext = (IpcCoapRequestContext *) ctxt;
    xmlif_HandleResponse(requestContext, responsePath, coapResponseCode, IPC_MESSAGE_TYPE_RESPONSE, IPC_MESSAGE_SUB_TYPE_WRITE, contentType, payload, payloadLen, xmlif_HandlerDefaultSuccessfulResponse);
}

static int xmlif_HandlerWriteAttributesRequest(RequestInfoType * request, TreeNode content)
{
    return xmlif_HandleContentRequest(request, content, xmlif_HandlerSendCoapWriteAttributesRequest, xmlif_HandlerWriteAttributesResponse, AwaResourceOperations_ReadWrite);
}

static bool xmlif_HandlerSendCoapWriteAttributesRequest(IpcCoapRequestContext * requestContext, Lwm2mClientType * client,
                                                        ObjectInstanceResourceKey * key, TreeNode currentLeafNode, TreeNode currentResponsePathNode)
{
    bool result = true;
    int numValidAttributes = 0;
    char * query = NULL;
    msprintf(&query, "%s?", xmlif_GetURIForClient(client, key));

    // Build up our query from reading attribute link-value pairs
    TreeNode attributeNode = NULL;
    uint32_t attributeIndex = 0;
    while ((attributeNode = Xml_FindFrom(currentLeafNode, "Attribute", &attributeIndex)) != NULL)
    {
        TreeNode linkNode = Xml_Find(attributeNode, "Link");
        TreeNode valueNode = Xml_Find(attributeNode, "Value");
        TreeNode valueTypeNode = Xml_Find(attributeNode, "ValueType");

        const char * linkString = (char*)TreeNode_GetValue(linkNode);
        const char * encodedValueString = (char*)TreeNode_GetValue(valueNode);
        const char * valueTypeString = (char*)TreeNode_GetValue(valueTypeNode);
        AwaResourceType valueType = Lwm2mCore_ResourceTypeFromString(valueTypeString);

        if (linkString != NULL)
        {
            if (valueType == AwaResourceType_Integer || valueType == AwaResourceType_Float)
            {
                char * temp = NULL;
                if (numValidAttributes > 0)
                {
                    msprintf(&temp, "%s&", query);
                    free(query);
                    query = temp;
                }

                msprintf(&temp, "%s%s=", query, linkString);
                free(query);
                query = temp;

                int dataLength = 0;
                char * dataValue = NULL;

                if ((encodedValueString = (char*)TreeNode_GetValue(valueNode)) == NULL)
                {
                    Lwm2m_Error("Missing value data for value node\n");
                    requestContext->Result = AwaResult_BadRequest;
                    result = false;
                    goto error;
                }

                dataLength = xmlif_DecodeValue(&dataValue, valueType, encodedValueString, strlen(encodedValueString));
                if (dataLength < 0)
                {
                    Lwm2m_Error("Failed to decode value data for attribute link %s\n", encodedValueString);
                    free(dataValue);
                    requestContext->Result = AwaResult_BadRequest;
                    result = false;
                    goto error;
                }

                switch(valueType)
                {
                    case AwaResourceType_Integer:
                    {
                        int64_t value;
                        memcpy(&value, dataValue, sizeof(value));
                        msprintf(&temp, "%s%" PRId64, query, value);
                        break;
                    }
                    case AwaResourceType_Float:
                    {
                        double value;
                        memcpy(&value, dataValue, sizeof(value));
                        msprintf(&temp, "%s%.17g", query, value);
                        break;
                    }
                    default:
                    {
                        Lwm2m_Error("Unsupported write attribute resource type: %s\n", valueTypeString);
                        free(dataValue);
                        requestContext->Result = AwaResult_BadRequest;
                        result = false;
                        goto error;
                    }
                }
                free(dataValue);
                free(query);
                query = temp;

                ++numValidAttributes;
            }
            else
            {
                Lwm2m_Error("Unsupported write attribute resource type: %s\n", valueTypeString);
                requestContext->Result = AwaResult_BadRequest;
                result = false;
                goto error;
            }
        }
        else
        {
            Lwm2m_Error("No link in attribute node\n");
            requestContext->Result = AwaResult_BadRequest;
            result = false;
            goto error;
        }
    }

    if (numValidAttributes > 0)
    {
        Lwm2m_Error("PUT attributes WITH QUERY %s\n", query);
        coap_PutRequest(requestContext, query, ContentType_None, NULL, 0, xmlif_HandlerWriteAttributesResponse);
        result = true;
    }
    else
    {
        Lwm2m_Error("No attributes in query on path %s\n", xmlif_GetURIForClient(client, key));
        requestContext->Result = AwaResult_BadRequest;
        result = false;
    }
error:
    free(query);
    return result;
}

static void xmlif_HandlerWriteAttributesResponse(void * ctxt, AddressType* address, const char * responsePath, int coapResponseCode,
                                                 ContentType contentType, char * payload, size_t payloadLen)
{
    IpcCoapRequestContext * requestContext = (IpcCoapRequestContext *) ctxt;
    xmlif_HandleResponse(requestContext, responsePath, coapResponseCode, IPC_MESSAGE_TYPE_RESPONSE, IPC_MESSAGE_SUB_TYPE_WRITE_ATTRIBUTES, contentType, payload, payloadLen, xmlif_HandlerSuccessfulWriteAttributesResponse);
}

static void xmlif_HandlerSuccessfulWriteAttributesResponse(IpcCoapRequestContext * requestContext, const char * responsePath, int coapResponseCode, TreeNode pathNode,
                                                           const char * responseType, ContentType contentType, char * payload, size_t payloadLen)
{
    xmlif_HandlerDefaultSuccessfulResponse(requestContext, responsePath, coapResponseCode, pathNode, responseType, contentType, payload, payloadLen);
}

static int xmlif_HandlerExecuteRequest(RequestInfoType * request, TreeNode content)
{
    return xmlif_HandleContentRequest(request, content, xmlif_HandlerSendCoapExecuteRequest, xmlif_HandlerExecuteResponse, AwaResourceOperations_Execute);
}

static bool xmlif_HandlerSendCoapExecuteRequest(IpcCoapRequestContext * requestContext, Lwm2mClientType * client,
                                                ObjectInstanceResourceKey * key, TreeNode currentLeafNode, TreeNode currentResponsePathNode)
{
    bool result = true;
    if (ObjectsTree_IsResourceNode(currentLeafNode))
    {
        int dataLength = 0;
        char * dataValue = NULL;

        TreeNode valueNode = NULL;
        if ((valueNode = Xml_Find(currentLeafNode, "Value")) != NULL)
        {
            const char * data;

            if ((data = (char*)TreeNode_GetValue(valueNode)) == NULL)
            {
                Lwm2m_Error("Missing value data for value node\n");
                requestContext->Result = AwaResult_BadRequest;
                result = false;
                goto error;
            }

            dataLength = xmlif_DecodeValue(&dataValue, AwaResourceType_None, data, strlen(data));
            if (dataLength < 0)
            {
                Lwm2m_Error("Failed to decode execute value data for resource %s\n", xmlif_GetURIForClient(client, key));
                free(dataValue);
                requestContext->Result = AwaResult_BadRequest;
                result = false;
                goto error;
            }
        }

        coap_PostRequest(requestContext, xmlif_GetURIForClient(client, key), (dataLength > 0) ? ContentType_ApplicationOctetStream : ContentType_None, dataValue, dataLength, xmlif_HandlerExecuteResponse);
        free(dataValue);
        result = true;
    }
    else
    {
        Lwm2m_Error("Execute attempt on non-resource path: %s\n", xmlif_GetURIForClient(client, key));
        requestContext->Result = AwaResult_BadRequest;
        result = false;
    }
error:
    return result;
}

static void xmlif_HandlerExecuteResponse(void * ctxt, AddressType* address, const char * responsePath, int coapResponseCode, ContentType contentType, char * payload, size_t payloadLen)
{
    IpcCoapRequestContext * requestContext = (IpcCoapRequestContext *) ctxt;
    xmlif_HandleResponse(requestContext, responsePath, coapResponseCode, IPC_MESSAGE_TYPE_RESPONSE, IPC_MESSAGE_SUB_TYPE_EXECUTE, contentType, payload, payloadLen, xmlif_HandlerSuccessfulExecuteResponse);
}

static void xmlif_HandlerSuccessfulExecuteResponse(IpcCoapRequestContext * requestContext, const char * responsePath, int coapResponseCode,
                                                   TreeNode pathNode, const char * responseType, ContentType contentType, char * payload, size_t payloadLen)
{
    xmlif_HandlerDefaultSuccessfulResponse(requestContext, responsePath, coapResponseCode, pathNode, responseType, contentType, payload, payloadLen);
}


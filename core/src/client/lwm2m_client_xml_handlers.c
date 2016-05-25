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
#include <xmltree.h>

#include "lwm2m_xml_interface.h"
#include "lwm2m_xml_serdes.h"
#include "lwm2m_client_xml_handlers.h"
#include "lwm2m_types.h"
#include <inttypes.h>
#include "b64.h"
#include "lwm2m_debug.h"
#include "lwm2m_list.h"
#include "lwm2m_core.h"
#include "xml.h"
#include "lwm2m_result.h"
#include "lwm2m_ipc.h"
#include "lwm2m_registration.h"
#include "../common/ipc_session.h"
// #include "lwm2m_events.h"

// Components from API
#include "../../../api/src/objects_tree.h"
#include "../../api/include/awa/common.h"
#include "../../../api/src/lwm2m_error.h"
#include "../../api/src/ipc_defs.h"

#define STR(s) # s
#define STRINGIFY(s) STR(s)

static int xmlif_HandlerConnectRequest(RequestInfoType * request, TreeNode content);
static int xmlif_HandlerEstablishNotifyRequest(RequestInfoType * request, TreeNode content);
static int xmlif_HandlerDisconnectRequest(RequestInfoType * request, TreeNode content);
static int xmlif_HandlerDefineRequest(RequestInfoType * request, TreeNode content);
static int xmlif_HandlerGetRequest(RequestInfoType * request, TreeNode content);
static int xmlif_HandlerSetRequest(RequestInfoType * request, TreeNode content);
static int xmlif_HandlerDeleteRequest(RequestInfoType * request, TreeNode content);
static int xmlif_HandlerSubscribeRequest(RequestInfoType * request, TreeNode content);
static void xmlif_GenerateChangeNotification(void * ctxt, AddressType* address, const char * responsePath, int responseCode, const char * responseType);


static ResourceOperationHandlers xmlifResourceOperationHandlers = {
    .Execute = xmlif_ExecuteResourceHandler,
    .CreateOptionalResource = xmlif_CreateOptionalResourceHandler,
};

static struct ListHead executeHandlers;

void xmlif_RegisterHandlers(void)
{
    ListInit(&executeHandlers);

    xmlif_AddRequestHandler(IPC_MESSAGE_SUB_TYPE_CONNECT,           xmlif_HandlerConnectRequest);
    xmlif_AddRequestHandler(IPC_MESSAGE_SUB_TYPE_ESTABLISH_NOTIFY,  xmlif_HandlerEstablishNotifyRequest);
    xmlif_AddRequestHandler(IPC_MESSAGE_SUB_TYPE_DISCONNECT,        xmlif_HandlerDisconnectRequest);
    xmlif_AddRequestHandler(IPC_MESSAGE_SUB_TYPE_DEFINE,            xmlif_HandlerDefineRequest);
    xmlif_AddRequestHandler(IPC_MESSAGE_SUB_TYPE_GET,               xmlif_HandlerGetRequest);
    xmlif_AddRequestHandler(IPC_MESSAGE_SUB_TYPE_SET,               xmlif_HandlerSetRequest);
    xmlif_AddRequestHandler(IPC_MESSAGE_SUB_TYPE_DELETE,            xmlif_HandlerDeleteRequest);
    xmlif_AddRequestHandler(IPC_MESSAGE_SUB_TYPE_SUBSCRIBE,         xmlif_HandlerSubscribeRequest);
}

int xmlif_AddExecuteHandler(RequestInfoType * request, ObjectInstanceResourceKey * key)
{
    ExecuteHandlerType * exeHandler = NULL;

    exeHandler = (ExecuteHandlerType *)malloc(sizeof(ExecuteHandlerType));
    if (exeHandler == NULL)
    {
        return -1;
    }

    exeHandler->ObjectID = key->ObjectID;
    exeHandler->ObjectInstanceID = key->InstanceID;
    exeHandler->ResourceID = key->ResourceID;
    exeHandler->SessionID = request->SessionID;
    memcpy(&exeHandler->ExecuteTarget, request, sizeof(RequestInfoType));

    ListAdd(&exeHandler->List, &executeHandlers);
    return 0;
}

ExecuteHandlerType * xmlif_GetExecuteHandler(ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    struct ListHead * i;
    ListForEach(i, &executeHandlers)
    {
        ExecuteHandlerType * executeHandler = ListEntry(i, ExecuteHandlerType, List);
        if (executeHandler != NULL)
        {
            if ((executeHandler->ObjectID == objectID) && (executeHandler->ObjectInstanceID == objectInstanceID) && (executeHandler->ResourceID == resourceID))
            {
                return executeHandler;
            }
        }
    }
    return NULL;
}

ExecuteHandlerType * xmlif_GetNextExecuteHandler(ExecuteHandlerType * currentHandler, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    struct ListHead * i;
    bool found = false;

    ListForEach(i, &executeHandlers)
    {
        ExecuteHandlerType * executeHandler = ListEntry(i, ExecuteHandlerType, List);
        if (executeHandler != NULL)
        {
            if ((executeHandler->ObjectID == objectID) && (executeHandler->ObjectInstanceID == objectInstanceID) && (executeHandler->ResourceID == resourceID))
            {
                if (found)
                {
                    return executeHandler;
                }

                if (!found && (currentHandler == executeHandler))
                {
                    found = true;
                }
            }
        }
    }
    return NULL;
}

int xmlif_RemoveExecuteHandler(ObjectInstanceResourceKey * key, RequestInfoType * request)
{
    int result = -1;
    struct ListHead * i, * n;
    ListForEachSafe(i, n, &executeHandlers)
    {
        ExecuteHandlerType * executeHandler = ListEntry(i, ExecuteHandlerType, List);
        if (executeHandler)
        {
            if ((executeHandler->ObjectID == key->ObjectID) && (executeHandler->ObjectInstanceID == key->InstanceID) && (executeHandler->ResourceID == key->ResourceID))
            {
                if (!memcmp(&executeHandler->ExecuteTarget, request, sizeof(RequestInfoType)))
                {
                    ListRemove(&executeHandler->List);
                    free(executeHandler);
                    result = 0;
                }
            }
        }
    }
    return result;
}

void xmlif_DestroyExecuteHandlers(void)
{
    struct ListHead * i, * n;
    ListForEachSafe(i, n, &executeHandlers)
    {
        ExecuteHandlerType * executeHandler = ListEntry(i, ExecuteHandlerType, List);
        ListRemove(&executeHandler->List);
        free(executeHandler);
    }
}

int xmlif_ExecuteResourceHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID,
                                 uint8_t * inValueBuffer, size_t inValueBufferLen)
{
    ExecuteHandlerType * executeHandler = NULL;
    executeHandler = xmlif_GetExecuteHandler(objectID, objectInstanceID, resourceID);

    if (executeHandler != NULL)
    {
        char * base64content = NULL;
        // Encode any argument data as base64
        Lwm2m_Info("xmlif_ResourceOperation: %s\n", (char *)inValueBuffer);
        if (inValueBuffer != NULL)
        {
            base64content = xmlif_EncodeValue(AwaResourceType_Opaque, inValueBuffer, inValueBufferLen);
        }

        do
        {
            int IPCSockFd = 0;
            const struct sockaddr * IPCAddr = NULL;
            int IPCAddrLen = 0;

            if (IPCSession_GetNotifyChannel(executeHandler->SessionID, &IPCSockFd, &IPCAddr, &IPCAddrLen) == 0)
            {

                TreeNode response = IPC_NewNotificationNode(IPC_MESSAGE_SUB_TYPE_SERVER_EXECUTE, executeHandler->SessionID);

                TreeNode content = Xml_CreateNode("Content");
                TreeNode_AddChild(response, content);

                TreeNode objects = Xml_CreateNode("Objects");
                TreeNode_AddChild(content, objects);

                TreeNode object = Xml_CreateNode("Object");
                TreeNode_AddChild(object, Xml_CreateNodeWithValue("ID", "%d", objectID));
                TreeNode_AddChild(objects, object);

                TreeNode instance = Xml_CreateNode("ObjectInstance");
                TreeNode_AddChild(instance, Xml_CreateNodeWithValue("ID", "%d", objectInstanceID));
                TreeNode_AddChild(object, instance);

                TreeNode resource = Xml_CreateNode("Resource");
                TreeNode_AddChild(resource, Xml_CreateNodeWithValue("ID", "%d", resourceID));
                TreeNode_AddChild(instance, resource);

                TreeNode_AddChild(resource, Xml_CreateNodeWithValue("Value", "%s", base64content ? base64content : ""));

                IPC_SendResponse(response, IPCSockFd, IPCAddr, IPCAddrLen);
                Tree_Delete(response);
            }
            else
            {
                Lwm2m_Error("Unable to get IPC Notify channel for session %d", executeHandler->SessionID);
            }

            executeHandler = xmlif_GetNextExecuteHandler(executeHandler, objectID, objectInstanceID, resourceID);

        } while(executeHandler != NULL);

        free(base64content);
        base64content = NULL;
    }
    return 0;
}

int xmlif_CreateOptionalResourceHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    // No action
    return 0;
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
        if (value != NULL)
        {
            objectID = atoi(value);
        }
    }

    // Defining objects/resources uses a min/max approach. From this it can be determined if a an object/resource is multi/single and mandatory/optional.
    //
    // If an object/resource is a Singleton/Collection (respectively) then the maximumInstances determines this:
    //
    // If maximumInstances = 1 then the object is single-instance or the resource is single-instance
    // If maximumInstances > 1 then the object is multiple-instance or the resource is multiple-instance
    //
    // Note: see IS_MULTIPLE_INSTANCE in lwm2m_dewfinition.h
    //
    // If an object/resource is mandatory/optional then the minimumInstances determines this:
    //
    // If minimumInstances = 0 then the object/resource is optional
    // If minimumInstances > 1 then the object/resource is mandatory
    //
    // Note: see IS_MANDATORY in lwm2m_dewfinition.h

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

    res = Lwm2mCore_RegisterObjectType(context, objectName ? objectName : "", objectID, maximumInstances, minimumInstances, &defaultObjectOperationHandlers);
    if (res < 0)
    {
        result = AwaResult_Forbidden;
        goto error;
    }

    node = TreeNode_Navigate(meta, "ObjectMetadata/Properties");
    if (node != NULL)
    {
        TreeNode property;
        int childIndex = 0;
        while ((property = TreeNode_GetChild(node, childIndex)))
        {
            TreeNode resNode;
            ResourceIDType resourceID = -1;
            int dataType = -1;
            const char * resourceName = NULL;
            uint16_t resourceMaximumInstances = 1;
            uint16_t resourceMinimumInstances = 0;
            Lwm2mTreeNode * defaultValueNode = NULL;

            AwaResourceOperations operation = AwaResourceOperations_None;

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
                int defaultValueLength = 0;

                if (value != NULL)
                {
                    defaultValueLength = xmlif_DecodeValue((char**)&defaultValue, dataType, value, strlen(value));
                }

                if (defaultValueLength >= 0)
                {
                    Lwm2mTreeNode * resourceInstanceNode = Lwm2mTreeNode_Create();
                    Lwm2mTreeNode_AddChild(defaultValueNode, resourceInstanceNode);
                    Lwm2mTreeNode_SetType(resourceInstanceNode, Lwm2mTreeNodeType_ResourceInstance);
                    Lwm2mTreeNode_SetValue(resourceInstanceNode, defaultValue, (uint16_t)defaultValueLength);
                    Lwm2mTreeNode_SetID(resourceInstanceNode, 0);
                }
                else
                {
                    Lwm2m_Error("xmlif_DecodeValue failed\n");
                }

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
                        int defaultValueLength = 0;

                        defaultValueLength = xmlif_DecodeValue((char**)&defaultValue, dataType, value, strlen(value));
                        if (defaultValueLength >= 0)
                        {
                            Lwm2mTreeNode * resourceInstanceNode = Lwm2mTreeNode_Create();
                            Lwm2mTreeNode_AddChild(defaultValueNode, resourceInstanceNode);
                            Lwm2mTreeNode_SetType(resourceInstanceNode, Lwm2mTreeNodeType_ResourceInstance);
                            Lwm2mTreeNode_SetValue(resourceInstanceNode, defaultValue, (uint16_t)defaultValueLength);
                            Lwm2mTreeNode_SetID(resourceInstanceNode, resourceInstanceID);
                        }
                        else
                        {
                            Lwm2m_Error("xmlif_DecodeValue failed\n");
                        }
                        free((void*)defaultValue);
                    }
                }
            }

            if (operation & AwaResourceOperations_Execute)
            {
                // Register xmlif operation for any executable resources so that we can produce XML when a resource is executed.
                res = Lwm2mCore_RegisterResourceTypeWithDefaultValue(context, resourceName ? resourceName : "", objectID, resourceID, dataType, resourceMaximumInstances, resourceMinimumInstances, operation, &xmlifResourceOperationHandlers, defaultValueNode);
            }
            else
            {
                res = Lwm2mCore_RegisterResourceTypeWithDefaultValue(context, resourceName ? resourceName : "", objectID, resourceID, dataType, resourceMaximumInstances, resourceMinimumInstances, operation, &defaultResourceOperationHandlers, defaultValueNode);
            }

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
        Lwm2m_Error("Bad IPC Connect request\n");
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
        TreeNode response = IPC_NewResponseNode(IPC_MESSAGE_SUB_TYPE_ESTABLISH_NOTIFY, AwaResult_Success, request->SessionID);
        IPC_SendResponse(response, request->Sockfd, &request->FromAddr, request->AddrLen);
        Tree_Delete(response);
    }
    else
    {
        Lwm2m_Error("Bad IPC ConnectNotify request\n");
        TreeNode response = IPC_NewResponseNode(IPC_MESSAGE_SUB_TYPE_ESTABLISH_NOTIFY, AwaResult_BadRequest, request->SessionID);
        IPC_SendResponse(response, request->Sockfd, &request->FromAddr, request->AddrLen);
        Tree_Delete(response);
    }
    free(request);
    return 0;
}

// Called to handle a request with the type "Disconnect". Returns 0 on success.
static int xmlif_HandlerDisconnectRequest(RequestInfoType * request, TreeNode content)
{
    // No check for known client - proceed regardless.
#ifndef CONTIKI
    Lwm2m_Info("IPC disconnected from %s\n", Lwm2mCore_DebugPrintSockAddr(&request->FromAddr));
#endif
    //TODO: cleanup for notify channel

    TreeNode response = IPC_NewResponseNode(IPC_MESSAGE_SUB_TYPE_DISCONNECT, AwaResult_Success, request->SessionID);
    IPC_SendResponse(response, request->Sockfd, &request->FromAddr, request->AddrLen);
    Tree_Delete(response);

    free(request);
    return 0;
}


// Called to handle a request with the type "Define". Returns 0 on success.
static int xmlif_HandlerDefineRequest(RequestInfoType * request, TreeNode content)
{
    Lwm2mContextType * context = (Lwm2mContextType*)request->Context;

    TreeNode objectDefinitions = TreeNode_Navigate(content, "Content/ObjectDefinitions");
    TreeNode objectDefinition = (objectDefinitions != NULL) ? TreeNode_GetChild(objectDefinitions, 0) : TreeNode_Navigate(content, "Content/ObjectDefinition");
    int objectDefinitionIndex = 1;
    int successCount = 0;
    while (objectDefinition != NULL)
    {
        if (xmlif_RegisterObjectFromXML(context, objectDefinition) == AwaResult_Success)
        {
            ++successCount;
        }
        objectDefinition = (objectDefinitions != NULL) ? TreeNode_GetChild(objectDefinitions, objectDefinitionIndex++) : NULL;
    }

    TreeNode response = IPC_NewResponseNode(IPC_MESSAGE_SUB_TYPE_DEFINE, AwaResult_Success, request->SessionID);
    IPC_SendResponse(response, request->Sockfd, &request->FromAddr, request->AddrLen);
    Tree_Delete(response);

    // Send an update so that all servers this client is connected to know that the client has this object defined.
    Lwm2m_SetUpdateRegistration(context);

    free(request);
    return 0;
}

// Create a response message.
static void xmlif_GenerateResponse(void * ctxt, AddressType* address, const char * responsePath, int responseCode, char * responseType, TreeNode * responseObjectsTree)
{
    RequestInfoType * request = ctxt;

    TreeNode response = IPC_NewResponseNode(responseType, responseCode, request->SessionID);
    if (AwaResult_IsSuccess(responseCode))
    {
        TreeNode content = Xml_CreateNode("Content");
        TreeNode_AddChild(content, responseObjectsTree);
        TreeNode_AddChild(response, content);
    }

    IPC_SendResponse(response, request->Sockfd, &request->FromAddr, request->AddrLen);
    Tree_Delete(response);

    free(request);
}

static AwaError AddResourceInstanceToGetResponse(Lwm2mContextType * context, int objectID, int instanceID, int resourceID, int resourceInstanceID, TreeNode responseLeafNode)
{
    AwaError result = AwaError_Unspecified;
    int outLength;
    char * dataValue = NULL;
    const char * buffer = NULL;

    AwaResourceType dataType = Definition_GetResourceType(Lwm2mCore_GetDefinitions(context), objectID, resourceID);
    if (dataType != AwaResourceType_None)
    {
        size_t dataLength = 0;

        outLength = Lwm2mCore_GetResourceInstanceValue(context, objectID, instanceID, resourceID, resourceInstanceID, (const void **)&buffer, &dataLength);
        if (outLength < 0)
        {
            result = AwaError_PathNotFound;
            goto error;
        }

        if (dataLength != 0)
        {
            dataValue = xmlif_EncodeValue(dataType, buffer, dataLength);
            if (dataValue == NULL)
            {
                result = AwaError_Internal;
                goto error;
            }
        }
    }

    TreeNode valueNode = Xml_CreateNode("Value");
    if (dataValue != NULL)
    {
        TreeNode_SetValue(valueNode, dataValue, strlen(dataValue));
    }
    else
    {
        TreeNode_SetValue(valueNode, "", 0);
    }

    TreeNode_AddChild(responseLeafNode, valueNode);

    free(dataValue);
    result = AwaError_Success;

error:
    return result;
}

static AwaError AddResourceToGetResponse(Lwm2mContextType * context, int objectID, int instanceID, int resourceID, TreeNode responseResourceNode, int idRangeStart, int idRangeEndExclusive)
{
    AwaError result = AwaError_Success;
    if (Definition_IsTypeMultiInstance(Lwm2mCore_GetDefinitions(context), objectID, resourceID))
    {
        int resourceInstanceID = -1;
        while ((resourceInstanceID = Lwm2mCore_GetNextResourceInstanceID(context, objectID, instanceID, resourceID, resourceInstanceID)) != -1)
        {
            if (((idRangeStart != -1) && (idRangeStart > resourceInstanceID)) ||
                ((idRangeEndExclusive != -1) && (idRangeEndExclusive <= resourceInstanceID)))
            {
                continue;
            }
            TreeNode responseResourceInstanceNode = ObjectsTree_FindOrCreateChildNode(responseResourceNode, "ResourceInstance", resourceInstanceID);
            if ((result = AddResourceInstanceToGetResponse(context, objectID, instanceID, resourceID, resourceInstanceID, responseResourceInstanceNode)) != AwaError_Success)
            {
                break;
            }
        }
    }
    else
    {
        result = AddResourceInstanceToGetResponse(context, objectID, instanceID, resourceID, 0, responseResourceNode);
    }
    return result;
}

static AwaError AddResourcesToGetResponse(Lwm2mContextType * context, int objectID, int instanceID, TreeNode responseObjectInstanceNode)
{
    AwaError result = AwaError_Success;
    int resourceID = -1;

    while ((resourceID = Lwm2mCore_GetNextResourceID(context, objectID, instanceID, resourceID)) != -1)
    {
        TreeNode responseResourceNode = ObjectsTree_FindOrCreateChildNode(responseObjectInstanceNode, "Resource", resourceID);
        if ((result = AddResourceToGetResponse(context, objectID, instanceID, resourceID, responseResourceNode, -1, -1)) != AwaError_Success)
        {
            break;
        }
    }
    return result;
}

static AwaError AddObjectInstancesToGetResponse(Lwm2mContextType * context, int objectID, TreeNode responseObjectNode)
{
    AwaError result = AwaError_Success;
    int instanceID = -1;

    while ((instanceID = Lwm2mCore_GetNextObjectInstanceID(context, objectID, instanceID)) != -1)
    {
        TreeNode responseObjectInstanceNode = ObjectsTree_FindOrCreateChildNode(responseObjectNode, "ObjectInstance", instanceID);
        if ((result = AddResourcesToGetResponse(context, objectID, instanceID, responseObjectInstanceNode)) != AwaError_Success)
        {
            break;
        }
    }
    return result;
}

// Called to handle a request with the type "Get".
static int xmlif_HandlerGetRequest(RequestInfoType * request, TreeNode xmlRequestContent)
{
    AwaResult result = AwaResult_Success;
    Lwm2mContextType * context = (Lwm2mContextType*)request->Context;
    TreeNode requestObjectsNode = TreeNode_Navigate(xmlRequestContent, "Content/Objects");
    TreeNode responseObjectsTree = ObjectsTree_New();

    TreeNode currentLeafNode = requestObjectsNode;
    while ((currentLeafNode = ObjectsTree_GetNextLeafNode(currentLeafNode)) != NULL)
    {
        ObjectInstanceResourceKey key = { .ObjectID = AWA_INVALID_ID, .InstanceID = AWA_INVALID_ID, .ResourceID = AWA_INVALID_ID, };
        ObjectsTree_GetIDsFromLeafNode(currentLeafNode, &key.ObjectID, &key.InstanceID, &key.ResourceID);

        if (key.ObjectID == AWA_INVALID_ID)
        {
            Lwm2m_Error("No object specified: /%d/%d/%d\n", key.ObjectID, key.InstanceID, key.ResourceID);
            result = AwaResult_BadRequest;
            goto error;
        }

        Lwm2m_Debug("Get request: %s (ID = /%d/%d/%d)\n", TreeNode_GetName(currentLeafNode), key.ObjectID, key.InstanceID, key.ResourceID);

        TreeNode responseObjectNode = ObjectsTree_FindOrCreateChildNode(responseObjectsTree, "Object", key.ObjectID);
        TreeNode responseObjectInstanceNode = key.InstanceID != AWA_INVALID_ID? ObjectsTree_FindOrCreateChildNode(responseObjectNode, "ObjectInstance", key.InstanceID) : NULL;
        TreeNode responseResourceNode = key.ResourceID != AWA_INVALID_ID? ObjectsTree_FindOrCreateChildNode(responseObjectInstanceNode, "Resource", key.ResourceID) : NULL;
        TreeNode responseLeafNode = responseResourceNode? responseResourceNode : responseObjectInstanceNode? responseObjectInstanceNode : responseObjectNode;

        if (Lwm2mCore_Exists(context, key.ObjectID, key.InstanceID, key.ResourceID))
        {
            if (key.ResourceID != -1)
            {
                int idRangeStart = -1;
                int idRangeEndExclusive = -1;
                TreeNode range = NULL;
                AwaError result = AwaError_Success;
                if ((range = Xml_Find(currentLeafNode, "IDRange")) != NULL)
                {
                    idRangeStart = xmlif_GetInteger(range, "IDRange/Start");
                    idRangeEndExclusive = xmlif_GetInteger(range, "IDRange/EndExclusive");
                    if ((idRangeEndExclusive <= idRangeStart) || (idRangeStart < 0) || idRangeStart > LWM2M_MAX_ID || idRangeEndExclusive > LWM2M_MAX_ID + 1)
                    {
                        Lwm2m_Debug("IDRange out of bounds - start: %d endExclusive: %d", idRangeStart, idRangeEndExclusive);
                        result = AwaError_RangeInvalid;
                    }
                }
                if (result == AwaError_Success)
                {
                    result = AddResourceToGetResponse(context, key.ObjectID, key.InstanceID, key.ResourceID, responseLeafNode, idRangeStart, idRangeEndExclusive);
                }
                IPC_AddResultTag(responseLeafNode, result);
            }
            else if (key.InstanceID != -1)
            {
                AwaError result = AddResourcesToGetResponse(context, key.ObjectID, key.InstanceID, responseLeafNode);
                IPC_AddResultTag(responseLeafNode, result);
                IPC_AddResultTagToAllLeafNodes(responseLeafNode, result);
            }
            else
            {
                AwaError result = AddObjectInstancesToGetResponse(context, key.ObjectID, responseLeafNode);
                IPC_AddResultTag(responseLeafNode, result);
                IPC_AddResultTagToAllLeafNodes(responseLeafNode, result);
            }
        }
        else
        {
            Lwm2m_Debug("Resource does not exist with ID: /%d/%d/%d\n", key.ObjectID, key.InstanceID, key.ResourceID);
            IPC_AddResultTag(responseLeafNode, AwaError_PathNotFound);
        }
    }

error:
    xmlif_GenerateResponse(request, NULL, NULL, result, IPC_MESSAGE_SUB_TYPE_GET, responseObjectsTree);
    return result;
}

static Lwm2mTreeNode * xmlif_xmlObjectToLwm2mObject(Lwm2mContextType * context, const TreeNode xmlObjectNode, bool readValues)
{
    Lwm2mTreeNode * objectNode = Lwm2mTreeNode_Create();
    Lwm2mTreeNode_SetType(objectNode, Lwm2mTreeNodeType_Object);
    int objectID = xmlif_GetInteger(xmlObjectNode, "Object/ID");
    Lwm2mTreeNode_SetCreateFlag(objectNode, Xml_Find(xmlObjectNode, "Create"));
    Lwm2mTreeNode_SetID(objectNode, objectID);

    ObjectDefinition * definition = Definition_LookupObjectDefinition(Lwm2mCore_GetDefinitions(context), objectID);
    if (definition != NULL)
    {
        Lwm2mTreeNode_SetDefinition(objectNode, definition);

        uint32_t instanceIndex = 0;
        TreeNode xmlObjectInstanceNode = NULL;
        while ((xmlObjectInstanceNode = Xml_FindFrom(xmlObjectNode, "ObjectInstance", &instanceIndex)) != NULL)
        {
            int instanceID;

            bool createInstance = Xml_Find(xmlObjectInstanceNode, "Create");
            instanceID = xmlif_GetInteger(xmlObjectInstanceNode, "ObjectInstance/ID");

            if (instanceID == -1 && !createInstance)
            {
                Lwm2m_Error("Missing instance ID node in instance for object ID %d\n", objectID);
                Lwm2mTreeNode_DeleteRecursive(objectNode);
                objectNode = NULL;
                goto error;
            }

            Lwm2mTreeNode * objectInstanceNode = Lwm2mTreeNode_Create();
            if (instanceID != -1)
            {
                Lwm2mTreeNode_SetID(objectInstanceNode, (uint16_t)instanceID);
            }
            Lwm2mTreeNode_SetType(objectInstanceNode, Lwm2mTreeNodeType_ObjectInstance);
            Lwm2mTreeNode_SetCreateFlag(objectInstanceNode, createInstance);
            Lwm2mTreeNode_AddChild(objectNode, objectInstanceNode);

            if (instanceID != -1)
            {
                uint32_t propertyIndex = 0;
                TreeNode xmlResourceNode = NULL;
                while ((xmlResourceNode = Xml_FindFrom(xmlObjectInstanceNode, "Resource", &propertyIndex)) != NULL)
                {
                    int resourceID;
                    bool createOptionalResource = Xml_Find(xmlResourceNode, "Create") != NULL;

                    if ((resourceID = xmlif_GetInteger(xmlResourceNode, "Resource/ID")) == -1)
                    {
                        Lwm2m_Error("Missing resource ID node in for object instance %d/%d\n", objectID, instanceID);
                        Lwm2mTreeNode_DeleteRecursive(objectNode);
                        objectNode = NULL;
                        goto error;
                    }

                    Lwm2mTreeNode * resourceNode = Lwm2mTreeNode_Create();
                    Lwm2mTreeNode_SetID(resourceNode, resourceID);
                    Lwm2mTreeNode_SetType(resourceNode, Lwm2mTreeNodeType_Resource);
                    Lwm2mTreeNode_SetCreateFlag(resourceNode, createOptionalResource);
                    Lwm2mTreeNode_AddChild(objectInstanceNode, resourceNode);

                    ResourceDefinition * resourceDefinition = Definition_LookupResourceDefinition(Lwm2mCore_GetDefinitions(context), objectID, resourceID);
                    if (resourceDefinition != NULL)
                    {
                        Lwm2mTreeNode_SetDefinition(resourceNode, resourceDefinition);

                        if (!IS_MULTIPLE_INSTANCE(resourceDefinition))
                        {
                            uint16_t resourceInstanceID = 0;
                            Lwm2mTreeNode * resourceInstanceNode = Lwm2mTreeNode_Create();
                            Lwm2mTreeNode_SetID(resourceInstanceNode, resourceInstanceID);
                            Lwm2mTreeNode_SetType(resourceInstanceNode, Lwm2mTreeNodeType_ResourceInstance);

                            if (readValues)
                            {
                                if (!createOptionalResource || Xml_Find(xmlResourceNode, "Value"))
                                {
                                    const char * data;
                                    int dataLength;
                                    char * dataValue = NULL;
                                    if ((data = (char*)xmlif_GetOpaque(xmlResourceNode, "Resource/Value")) == NULL)
                                    {
                                        Lwm2m_Error("Missing value data for resource %d/%d/%d\n", objectID, instanceID, resourceID);
                                        Lwm2mTreeNode_DeleteRecursive(objectNode);
                                        objectNode = NULL;
                                        Lwm2mTreeNode_DeleteRecursive(resourceInstanceNode);
                                        resourceInstanceNode = NULL;
                                        goto error;
                                    }

                                    dataLength = xmlif_DecodeValue(&dataValue, resourceDefinition->Type, data, strlen(data));
                                    if (dataLength < 0)
                                    {
                                        Lwm2m_Error("Failed to decode value data for resource %d/%d/%d\n", objectID, instanceID, resourceID);
                                        free(dataValue);
                                        Lwm2mTreeNode_DeleteRecursive(objectNode);
                                        objectNode = NULL;
                                        Lwm2mTreeNode_DeleteRecursive(resourceInstanceNode);
                                        resourceInstanceNode = NULL;
                                        goto error;
                                    }

                                    Lwm2mTreeNode_SetValue(resourceInstanceNode, (const uint8_t*)dataValue, dataLength);
                                    free(dataValue);

                                    Lwm2mTreeNode_AddChild(resourceNode, resourceInstanceNode);
                                    resourceInstanceNode = NULL;
                                }
                            }

                            if (resourceInstanceNode != NULL)
                            {
                                Lwm2mTreeNode_DeleteRecursive(resourceInstanceNode);
                                resourceInstanceNode = NULL;
                            }
                        }
                        else
                        {
                            bool replace = false;
                            const char * setWriteModeString = xmlif_GetOpaque(xmlResourceNode, "Resource/SetArrayMode");
                            if (setWriteModeString != NULL)
                            {
                                replace = (strcmp(setWriteModeString, "Replace") == 0);
                            }
                            Lwm2mTreeNode_SetReplaceFlag(resourceNode, replace);

                            uint32_t valueIndex = 0;
                            TreeNode xmlResourceInstanceNode = NULL;
                            while ((xmlResourceInstanceNode = Xml_FindFrom(xmlResourceNode, "ResourceInstance", &valueIndex)) != NULL)
                            {
                                if (readValues)
                                {
                                    int valueID = xmlif_GetInteger(xmlResourceInstanceNode, "ResourceInstance/ID");
                                    Lwm2mTreeNode * resourceInstanceNode = Lwm2mTreeNode_Create();
                                    Lwm2mTreeNode_SetID(resourceInstanceNode, valueID);
                                    Lwm2mTreeNode_SetType(resourceInstanceNode, Lwm2mTreeNodeType_ResourceInstance);

                                    const char * data;
                                    int dataLength;
                                    char * dataValue = NULL;
                                    if ((data = (char*)xmlif_GetOpaque(xmlResourceInstanceNode, "ResourceInstance/Value")) == NULL)
                                    {
                                        Lwm2m_Error("Missing value data for resource %d/%d/%d\n", objectID, instanceID, resourceID);
                                        Lwm2mTreeNode_DeleteRecursive(objectNode);
                                        objectNode = NULL;
                                        Lwm2mTreeNode_DeleteRecursive(resourceInstanceNode);
                                        resourceInstanceNode = NULL;
                                        goto error;
                                    }

                                    dataLength = xmlif_DecodeValue(&dataValue, resourceDefinition->Type, data, strlen(data));
                                    if (dataLength < 0)
                                    {
                                        Lwm2m_Error("Failed to decode value data for resource %d/%d/%d\n", objectID, instanceID, resourceID);
                                        free(dataValue);
                                        Lwm2mTreeNode_DeleteRecursive(objectNode);
                                        objectNode = NULL;
                                        Lwm2mTreeNode_DeleteRecursive(resourceInstanceNode);
                                        resourceInstanceNode = NULL;
                                        goto error;
                                    }

                                    Lwm2mTreeNode_SetValue(resourceInstanceNode, (const uint8_t*)dataValue, dataLength);
                                    free(dataValue);

                                    Lwm2mTreeNode_AddChild(resourceNode, resourceInstanceNode);
                                }
                            }
                        }
                    }
                    else
                    {
                        Lwm2m_Debug("No definition for object %d resource %d\n", objectID, resourceID);
                        Lwm2mTreeNode_DeleteRecursive(objectNode);
                        objectNode = NULL;
                        goto error;
                    }
                }
            }
        }
    }
    else
    {
        Lwm2m_Debug("No definition for object /%d\n", objectID);
        Lwm2mTreeNode_DeleteRecursive(objectNode);
        objectNode = NULL;
        goto error;
    }

error:
    return objectNode;
}

// Called to handle a request with the type "Set".
static int xmlif_HandlerSetRequest(RequestInfoType * request, TreeNode content)
{
    AwaResult result = AwaResult_Success;
    Lwm2mContextType * context = (Lwm2mContextType*)request->Context;
    TreeNode requestObjectsNode = TreeNode_Navigate(content, "Content/Objects");
    TreeNode responseObjectsTree = ObjectsTree_New();

    TreeNode requestObjectNode = NULL;
    int objectIndex = 0;
    while ((requestObjectNode = TreeNode_GetChild(requestObjectsNode, objectIndex++)) != NULL)
    {
        // convert to Object Lwm2mTreeNode so we can check permissions / write as a single entity.
        Lwm2mTreeNode * object = xmlif_xmlObjectToLwm2mObject(context, requestObjectNode, true);

        if (object != NULL)
        {
            int objectID;
            int createdObjectInstanceID = -1;
            Lwm2mTreeNode_GetID(object, &objectID);
            AwaResult setResult = Lwm2mCore_CheckWritePermissionsForObjectNode(context, Lwm2mRequestOrigin_Client, object, false);
            if (AwaResult_IsSuccess(setResult))
            {
                setResult = Lwm2mCore_ParseObjectNodeAndWriteToStore(context, object, false, false, false, &createdObjectInstanceID);
            }
            AwaError error = AwaResult_ToAwaError(setResult, AwaError_PathInvalid);
            TreeNode responseObjectNode = ObjectsTree_FindOrCreateChildNode(responseObjectsTree, "Object", xmlif_GetInteger(requestObjectNode, "Object/ID"));
            Lwm2mTreeNode * objectInstance = Lwm2mTreeNode_GetFirstChild(object);
            if (objectInstance != NULL)
            {
                while (objectInstance != NULL)
                {
                    int objectInstanceID = createdObjectInstanceID;
                    if (objectInstanceID == -1)
                    {
                        Lwm2mTreeNode_GetID(objectInstance, &objectInstanceID);
                    }
                    TreeNode responseObjectInstanceNode = ObjectsTree_FindOrCreateChildNode(responseObjectNode, "ObjectInstance", objectInstanceID);

                    Lwm2mTreeNode * resource = Lwm2mTreeNode_GetFirstChild(objectInstance);
                    if (resource != NULL)
                    {
                        while (resource != NULL)
                        {
                            int resourceID;
                            Lwm2mTreeNode_GetID(resource, &resourceID);
                            TreeNode responseResourceNode = ObjectsTree_FindOrCreateChildNode(responseObjectInstanceNode, "Resource", resourceID);

                            AwaError resourceError = error;
                            if (error != AwaError_Success && Lwm2mTreeNode_IsCreateFlagSet(resource))
                            {
                                resourceError = AwaError_CannotCreate;
                            }
                            IPC_AddResultTag(responseResourceNode, resourceError);

                            resource = Lwm2mTreeNode_GetNextChild(objectInstance, resource);
                        }
                    }
                    else
                    {
                        AwaError objectInstanceError = error;
                        if (error != AwaError_Success && Lwm2mTreeNode_IsCreateFlagSet(objectInstance))
                        {
                            objectInstanceError = AwaError_CannotCreate;
                        }
                        IPC_AddResultTag(responseObjectInstanceNode, objectInstanceError);
                    }
                    objectInstance = Lwm2mTreeNode_GetNextChild(object, objectInstance);
                }
            }
            else
            {
                AwaError objectError = error;
                if (error != AwaError_Success && Lwm2mTreeNode_IsCreateFlagSet(object))
                {
                    objectError = AwaError_CannotCreate;
                }
                IPC_AddResultTag(responseObjectNode, objectError);
            }
            Lwm2mTreeNode_DeleteRecursive(object);
        }
        else
        {
            Lwm2m_Error("Failed to parse XML object");
            result = AwaResult_BadRequest;
            goto error;
        }
    }

error:
    xmlif_GenerateResponse(request, NULL, NULL, result, IPC_MESSAGE_SUB_TYPE_SET, responseObjectsTree);
    return result;
}

int xmlif_Lwm2mNotificationCallback(void * context, AddressType * address, int sequence, const char * token, int tokenLength, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ContentType contentType, void * contextData)
{
    ObjectInstanceResourceKey key;
    RequestInfoType * request = (RequestInfoType *)malloc(sizeof(RequestInfoType));

    key.InstanceID = objectInstanceID;
    key.ObjectID = objectID;
    key.ResourceID = resourceID;

    memcpy(request, contextData, sizeof(RequestInfoType));

    xmlif_GenerateChangeNotification(request, NULL, (char*)OirToUri(key), AwaResult_Success, IPC_MESSAGE_SUB_TYPE_SERVER_CHANGE);

    return 0;
}

static AwaResult xmlif_HandleObserve(void * context, RequestInfoType * request, ObjectIDType objectID, ObjectInstanceIDType instanceID, ResourceIDType resourceID)
{
    AwaResult result = AwaResult_Success;
    AddressType addr;
    memset(&addr, 0, sizeof(addr));
    addr.Size = sizeof(request->FromAddr);
    addr.Addr.Sa = request->FromAddr;

    // Note: this memory is released when the observe fails, or the observer is destroyed in CancelObserve.
    RequestInfoType * temp = malloc(sizeof(RequestInfoType));
    memcpy(temp, request, sizeof(RequestInfoType));

    if (Lwm2mCore_Observe(context, &addr, NULL, 0, objectID, instanceID, resourceID, ContentType_ApplicationOmaLwm2mTLV, xmlif_Lwm2mNotificationCallback, (void*)temp) < 0)
    {
        result = AwaResult_BadRequest;
        free(temp);
        goto error;
    }

error:;
    return result;
}

// Called to handle a request with the type "Subscribe".
static int xmlif_HandlerSubscribeRequest(RequestInfoType * request, TreeNode xmlRequestContentNode)
{
    AwaResult result = AwaResult_Success;
    Lwm2mContextType * context = (Lwm2mContextType*)request->Context;
    TreeNode requestObjectsNode = TreeNode_Navigate(xmlRequestContentNode, "Content/Objects");
    TreeNode responseObjectsTree = ObjectsTree_New();

    TreeNode currentLeafNode = requestObjectsNode;
    while ((currentLeafNode = ObjectsTree_GetNextLeafNode(currentLeafNode)) != NULL)
    {
        ObjectInstanceResourceKey key = { .ObjectID = AWA_INVALID_ID, .InstanceID = AWA_INVALID_ID, .ResourceID = AWA_INVALID_ID, };
        ObjectsTree_GetIDsFromLeafNode(currentLeafNode, &key.ObjectID, &key.InstanceID, &key.ResourceID);

        if (key.ObjectID == AWA_INVALID_ID)
        {
            Lwm2m_Error("No object specified: /%d/%d/%d\n", key.ObjectID, key.InstanceID, key.ResourceID);
            result = AwaResult_BadRequest;
            goto error;
        }

        Lwm2m_Debug("Subscribe to %s (ID = /%d/%d/%d)\n", TreeNode_GetName(currentLeafNode), key.ObjectID, key.InstanceID, key.ResourceID);

        TreeNode responseObjectNode = ObjectsTree_FindOrCreateChildNode(responseObjectsTree, "Object", key.ObjectID);
        TreeNode responseObjectInstanceNode = key.InstanceID != AWA_INVALID_ID? ObjectsTree_FindOrCreateChildNode(responseObjectNode, "ObjectInstance", key.InstanceID) : NULL;
        TreeNode responseResourceNode = key.ResourceID != AWA_INVALID_ID? ObjectsTree_FindOrCreateChildNode(responseObjectInstanceNode, "Resource", key.ResourceID) : NULL;
        TreeNode responseLeafNode = responseResourceNode? responseResourceNode : responseObjectInstanceNode? responseObjectInstanceNode : responseObjectNode;

        if (Lwm2mCore_Exists(context, key.ObjectID, key.InstanceID, key.ResourceID))
        {
            AwaSubscribeType subscribeType = AwaSubscribeType_None;
            bool cancel = false;
            if (Xml_Find(currentLeafNode, IPC_MESSAGE_TAG_SUBSCRIBE_TO_CHANGE) != NULL)
            {
                subscribeType = AwaSubscribeType_Change;
                TreeNode_AddChild(responseLeafNode, Xml_CreateNode(IPC_MESSAGE_TAG_SUBSCRIBE_TO_CHANGE));
            }
            else if (Xml_Find(currentLeafNode, IPC_MESSAGE_TAG_SUBSCRIBE_TO_EXECUTE) != NULL)
            {
                subscribeType = AwaSubscribeType_Execute;
                TreeNode_AddChild(responseLeafNode, Xml_CreateNode(IPC_MESSAGE_TAG_SUBSCRIBE_TO_EXECUTE));
            }
            else if (Xml_Find(currentLeafNode, IPC_MESSAGE_TAG_CANCEL_SUBSCRIBE_TO_CHANGE) != NULL)
            {
                subscribeType = AwaSubscribeType_Change;
                cancel = true;
                TreeNode_AddChild(responseLeafNode, Xml_CreateNode(IPC_MESSAGE_TAG_CANCEL_SUBSCRIBE_TO_CHANGE));
            }
            else if (Xml_Find(currentLeafNode, IPC_MESSAGE_TAG_CANCEL_SUBSCRIBE_TO_EXECUTE) != NULL)
            {
                subscribeType = AwaSubscribeType_Execute;
                cancel = true;
                TreeNode_AddChild(responseLeafNode, Xml_CreateNode(IPC_MESSAGE_TAG_CANCEL_SUBSCRIBE_TO_EXECUTE));
            }

            if (subscribeType != AwaSubscribeType_None)
            {
                if (!cancel)
                {
                    if (subscribeType == AwaSubscribeType_Execute)
                    {
                        if (xmlif_AddExecuteHandler(request, &key) >= 0)
                        {
                            IPC_AddResultTag(responseLeafNode, AwaError_Success);
                        }
                        else
                        {
                            IPC_AddResultTag(responseLeafNode, AwaResult_InternalError);
                            goto error;
                        }
                    }
                    else
                    {
                        AwaResult observeResult = AwaResult_Unspecified;
                        if ((observeResult = xmlif_HandleObserve(context, request, key.ObjectID, key.InstanceID, key.ResourceID)) == AwaResult_Success)
                        {
                            IPC_AddResultTag(responseLeafNode, AwaError_Success);
                        }
                        else
                        {
                            Lwm2m_Debug("Failed to observe resource /%d/%d/%d\n", key.ObjectID, key.InstanceID, key.ResourceID);
                            switch(observeResult)
                            {

                            default:
                                IPC_AddResultTag(responseLeafNode, AwaError_Unspecified);
                                break;
                            }
                        }
                    }
                }
                else
                {
                    if (subscribeType == AwaSubscribeType_Execute)
                    {
                        if (key.ResourceID != -1)
                        {
                            ResourceDefinition * resFormat = NULL;
                            resFormat = Definition_LookupResourceDefinition(Lwm2mCore_GetDefinitions(context), key.ObjectID, key.ResourceID);
                            if (resFormat != NULL)
                            {
                                 // Cancel SubscribeToExecute or SubscribeToChange
                                 if (resFormat->Operation == AwaResourceOperations_Execute)
                                 {
                                     if (xmlif_RemoveExecuteHandler(&key, request) >= 0)
                                     {
                                         IPC_AddResultTag(responseLeafNode, AwaError_Success);
                                     }
                                     else
                                     {
                                         IPC_AddResultTag(responseLeafNode, AwaError_SubscriptionInvalid);
                                     }
                                 }
                                 else
                                 {
                                     IPC_AddResultTag(responseLeafNode, AwaError_PathInvalid);
                                 }
                            }
                            else
                            {
                                IPC_AddResultTag(responseLeafNode, AwaError_PathNotFound);
                            }
                        }
                        else
                        {
                            IPC_AddResultTag(responseLeafNode, AwaError_PathInvalid);
                        }
                    }

                    AddressType addr;
                    memset(&addr, 0, sizeof(addr));
                    addr.Size = sizeof(request->FromAddr);
                    addr.Addr.Sa = request->FromAddr;
                    if (Lwm2mCore_CancelObserve(context, &addr, key.ObjectID, key.InstanceID, key.ResourceID) >= 0)
                    {
                        IPC_AddResultTag(responseLeafNode, AwaError_Success);
                    }
                    else
                    {
                        IPC_AddResultTag(responseLeafNode, AwaError_SubscriptionInvalid);
                    }
                }
            }
            else
            {
                Lwm2m_Debug("Unknown observe type for resource /%d/%d/%d\n", key.ObjectID, key.InstanceID, key.ResourceID);
                IPC_AddResultTag(responseLeafNode, AwaError_IPCError);
            }
        }
        else
        {
            Lwm2m_Debug("Resource does not exist with ID: /%d/%d/%d\n", key.ObjectID, key.InstanceID, key.ResourceID);
            IPC_AddResultTag(responseLeafNode, AwaError_PathNotFound);
        }
    }

error:
    xmlif_GenerateResponse(request, NULL, NULL, result, IPC_MESSAGE_SUB_TYPE_SUBSCRIBE, responseObjectsTree);
    return result;
}


// Create a response with an object as the content.
static void xmlif_GenerateChangeNotification(void * ctxt, AddressType* address, const char * responsePath, int responseCode, const char * responseType)
{
    RequestInfoType * request = ctxt;
    Lwm2mContextType * context = (Lwm2mContextType*)request->Context;

    TreeNode content = NULL;

    // CoAP 2.XX responses indicate success, 2.05 and 2.04 may both have content in the old spec.
    if ((responseCode & AwaResult_Success) != 0)
    {
        content = IPC_NewContentNode();
        TreeNode objects = ObjectsTree_New();
        TreeNode_AddChild(content, objects);
        ObjectInstanceResourceKey key = UriToOir(responsePath);
        int instanceID;

        TreeNode object = Xml_CreateNode("Object");
        TreeNode_AddChild(objects, object);

        TreeNode objectIDnode = Xml_CreateNodeWithValue("ID", "%d", key.ObjectID);
        TreeNode_AddChild(object, objectIDnode);

        instanceID = -1;
        instanceID = (key.InstanceID == -1) ? Lwm2mCore_GetNextObjectInstanceID(context, key.ObjectID, instanceID) : key.InstanceID;

        while (instanceID != -1)
        {
            TreeNode instance = Xml_CreateNode("ObjectInstance");
            TreeNode_AddChild(object, instance);

            TreeNode instanceIDnode = Xml_CreateNodeWithValue("ID", "%d", instanceID);
            TreeNode_AddChild(instance, instanceIDnode);

            int resourceID = -1;
            resourceID = (key.ResourceID == -1) ? Lwm2mCore_GetNextResourceID(context, key.ObjectID, instanceID, resourceID) : key.ResourceID;

            while (resourceID != -1)
            {
                TreeNode resource = Xml_CreateNode("Resource");
                TreeNode_AddChild(instance, resource);

                TreeNode resourceIDnode = Xml_CreateNodeWithValue("ID", "%d", resourceID);
                TreeNode_AddChild(resource, resourceIDnode);

                ResourceDefinition * resourceDefinition = Definition_LookupResourceDefinition(Lwm2mCore_GetDefinitions(context), key.ObjectID, resourceID);
                if (resourceDefinition != NULL)
                {
                    if (IS_MULTIPLE_INSTANCE(resourceDefinition))
                    {
                        int resourceInstanceID = -1;
                        while ((resourceInstanceID = Lwm2mCore_GetNextResourceInstanceID(context, key.ObjectID, instanceID, resourceID, resourceInstanceID)) != -1)
                        {
                            TreeNode resourceInstance = Xml_CreateNode("ResourceInstance");
                            TreeNode resourceInstanceIDnode = Xml_CreateNodeWithValue("ID", "%d", resourceInstanceID);
                            TreeNode_AddChild(resourceInstance, resourceInstanceIDnode);
                            TreeNode_AddChild(resource, resourceInstance);

                            AddResourceInstanceToGetResponse(context, key.ObjectID, instanceID, resourceID, resourceInstanceID, resourceInstance);
                        }
                    }
                    else
                    {
                        AddResourceInstanceToGetResponse(context, key.ObjectID, instanceID, resourceID, 0, resource);
                    }

                    // If we have requested a specific resource, then return -1 here and break.
                    resourceID = (key.ResourceID == -1) ? Lwm2mCore_GetNextResourceID(context, key.ObjectID, instanceID, resourceID) : -1;
                }
                else
                {
                    Lwm2m_Error("Resource definition not found for object ID %d, resource ID %d\n", key.ObjectID, resourceID);
                }
            }
            instanceID = (key.InstanceID == -1) ? Lwm2mCore_GetNextObjectInstanceID(context, key.ObjectID, instanceID) : -1;
        }
        goto done;
    }

done:
    {
        int IPCSockFd = 0;
        const struct sockaddr * IPCAddr = NULL;
        int IPCAddrLen = 0;

        if (IPCSession_GetNotifyChannel(request->SessionID, &IPCSockFd, &IPCAddr, &IPCAddrLen) == 0)
        {
            TreeNode response = IPC_NewNotificationNode(responseType, request->SessionID);
            TreeNode_AddChild(response, content);

            IPC_SendResponse(response, IPCSockFd, IPCAddr, IPCAddrLen);
            Tree_Delete(response);
        }
        else
        {
            Lwm2m_Error("Unable to get IPC Notify channel for session %d", request->SessionID);
        }
    }
    free(request);
}

// Called to handle a request with the type "Delete".
static int xmlif_HandlerDeleteRequest(RequestInfoType * request, TreeNode content)
{
    AwaResult result = AwaResult_Success;
    Lwm2mContextType * context = (Lwm2mContextType*)request->Context;
    TreeNode requestObjectsNode = TreeNode_Navigate(content, "Content/Objects");
    TreeNode responseObjectsTree = ObjectsTree_New();

    TreeNode currentLeafNode = requestObjectsNode;
    while ((currentLeafNode = ObjectsTree_GetNextLeafNode(currentLeafNode)) != NULL)
    {
        ObjectInstanceResourceKey key = { .ObjectID = AWA_INVALID_ID, .InstanceID = AWA_INVALID_ID, .ResourceID = AWA_INVALID_ID, };
        ObjectsTree_GetIDsFromLeafNode(currentLeafNode, &key.ObjectID, &key.InstanceID, &key.ResourceID);

        if (key.ObjectID == AWA_INVALID_ID)
        {
            Lwm2m_Error("No object specified: /%d/%d/%d\n", key.ObjectID, key.InstanceID, key.ResourceID);
            result = AwaResult_BadRequest;
            goto error;
        }

        TreeNode responseObjectNode = ObjectsTree_FindOrCreateChildNode(responseObjectsTree, "Object", key.ObjectID);
        TreeNode responseObjectInstanceNode = key.InstanceID != AWA_INVALID_ID? ObjectsTree_FindOrCreateChildNode(responseObjectNode, "ObjectInstance", key.InstanceID) : NULL;
        TreeNode responseResourceNode = key.ResourceID != AWA_INVALID_ID? ObjectsTree_FindOrCreateChildNode(responseObjectInstanceNode, "Resource", key.ResourceID) : NULL;
        TreeNode responseLeafNode = responseResourceNode? responseResourceNode : responseObjectInstanceNode? responseObjectInstanceNode : responseObjectNode;

        AwaResult deleteResult = Lwm2mCore_Delete(context, Lwm2mRequestOrigin_Client, key.ObjectID, key.InstanceID, key.ResourceID, false);

        IPC_AddResultTag(responseLeafNode, AwaResult_ToAwaError(deleteResult, AwaError_CannotDelete));
    }

error:
    xmlif_GenerateResponse(request, NULL, NULL, result, IPC_MESSAGE_SUB_TYPE_DELETE, responseObjectsTree);
    return result;
}

#endif // CONTIKI

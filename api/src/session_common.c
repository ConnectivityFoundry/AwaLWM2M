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


#include <stdbool.h>
#include <string.h>

#include "session_common.h"
#include "ipc.h"
#include "memalloc.h"
#include "log.h"
#include "define_common.h"
#include "path.h"
#include "utils.h"

#include "lwm2m_definition.h"
#include "xml.h"
#include "xmltree.h"
#include "lwm2m_xml_serdes.h"

#define SESSION_CONNECT_TIMEOUT (10 * 1000)  // 10 second timeout

/* Notes:
 *  - a Session is considered 'connected' if it has a non-NULL IPCChannel.
 */

struct _SessionCommon
{
    IPCInfo * IPCInfo;
    IPCChannel * IPCChannel;
    DefinitionRegistry * DefinitionRegistry;
    SessionType SessionType;
    IPCSessionID SessionID;
};

static bool SessionType_IsValid(SessionType type)
{
    return type == SessionType_Client || type == SessionType_Server;
}

SessionCommon * SessionCommon_New(SessionType sessionType)
{
    SessionCommon * session = NULL;
    if (SessionType_IsValid(sessionType))
    {
        session = Awa_MemAlloc(sizeof(*session));
        if (session != NULL)
        {
            memset(session, 0, sizeof(*session));

            session->SessionType = sessionType;
            session->SessionID = 0;
            session->DefinitionRegistry = DefinitionRegistry_Create();
            if (session->DefinitionRegistry != NULL)
            {
                LogNew("SessionCommon", session);
            }
            else
            {
                LogErrorWithEnum(AwaError_OutOfMemory, "Could not create definition registry.");
                Awa_MemSafeFree(session);
                session = NULL;
            }
        }
        else
        {
            LogErrorWithEnum(AwaError_OutOfMemory, "Could not create SessionCommon");
            Awa_MemSafeFree(session);
            session = NULL;
        }
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid, "Session type is invalid: %d", sessionType);
    }

    return session;
}

AwaError SessionCommon_Free(SessionCommon ** session)
{
    AwaError result = AwaError_Success;
    if ((session != NULL) && (*session != NULL))
    {
        // Automatically disconnect if connected
        if (SessionCommon_IsConnected(*session) != false)
        {
            LogWarning("Automatic disconnect");
            SessionCommon_DisconnectSession(*session);
        }

        // Free any memory owned by a session
        IPCInfo_Free(&((*session)->IPCInfo));
        IPCChannel_Free(&((*session)->IPCChannel));
        DefinitionRegistry_Destroy((*session)->DefinitionRegistry);

        // Free the session itself
        LogFree("SessionCommon", *session);
        Awa_MemSafeFree(*session);
        *session = NULL;
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid, "Session is NULL");
    }
    return result;
}

AwaError SessionCommon_SetIPCAsUDP(SessionCommon * session, const char * address, unsigned short port)
{
    AwaError result = AwaError_Success;
    if (session != NULL)
    {
        // Free existing record, if present
        IPCInfo_Free(&session->IPCInfo);

        IPCInfo * ipcInfo = IPCInfo_NewUDP(address, port);
        if (ipcInfo != NULL)
        {
            session->IPCInfo = ipcInfo;
            LogVerbose("Session IPC configured for UDP: address %s, port %d", address, port);
        }
        else
        {
            result = LogErrorWithEnum(AwaError_IPCError, "IPC not configured");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid, "Session is NULL");
    }
    return result;
}

bool SessionCommon_HasIPCInfo(const SessionCommon * session)
{
    return (session->IPCInfo != NULL);
}

static bool SessionCommon_RegisterObjectFromXML(DefinitionRegistry * definitions, TreeNode meta)
{
    bool result = true;
    int res;
    ObjectIDType objectID = AWA_INVALID_ID;
    const char * objectName = NULL;
    const uint8_t * value;
    TreeNode node;

    uint16_t MaximumInstances = 1;
    uint16_t MinimumInstances = 0;

    node = TreeNode_Navigate(meta, "ObjectMetadata/SerialisationName");
    if (node)
    {
        value = TreeNode_GetValue(node);
        if (value)
        {
            objectName = (const char *)value;
        }
    }

    node = TreeNode_Navigate(meta, "ObjectMetadata/ObjectID");
    if (node)
    {
        value = TreeNode_GetValue(node);
        if (value)
        {
            objectID = atoi((const char *)value);
        }
    }

    node = TreeNode_Navigate(meta, "ObjectMetadata/MaximumInstances");
    if (node)
    {
        value = TreeNode_GetValue(node);
        if (value)
        {
            MaximumInstances = atoi((const char *)value);
        }
    }

    node = TreeNode_Navigate(meta, "ObjectMetadata/MinimumInstances");
    if (node)
    {
        value = TreeNode_GetValue(node);
        if (value)
        {
            MinimumInstances = atoi((const char *)value);
        }
    }

    res = Definition_RegisterObjectType(definitions, objectName ? objectName : "", objectID, MaximumInstances, MinimumInstances, NULL);
    if (res < 0)
    {
        result = false;
        goto error;
    }

    node = TreeNode_Navigate(meta, "ObjectMetadata/Properties");
    if (node)
    {
        TreeNode property;
        int childIndex = 0;
        while ((property = TreeNode_GetChild(node, childIndex)))
        {
            TreeNode resNode;
            ResourceIDType resourceID = AWA_INVALID_ID;
            int dataType = -1;
            const char * resourceName = NULL;
            uint16_t ResourceMaximumInstances = 1;
            uint16_t ResourceMinimumInstances = 0;
            AwaResourceOperations operation = AwaResourceOperations_None;
            Lwm2mTreeNode * defaultValueNode = NULL;

            resNode = TreeNode_Navigate(property, "Property/PropertyID");
            if (resNode)
            {
                value = TreeNode_GetValue(resNode);
                if (value)
                {
                    resourceID = atoi((const char *)value);
                }
            }

            resNode = TreeNode_Navigate(property, "Property/SerialisationName");
            if (resNode)
            {
                value = TreeNode_GetValue(resNode);
                if (value)
                {
                    resourceName = (const char *)value;
                }
            }

            resNode = TreeNode_Navigate(property, "Property/DataType");
            if (resNode)
            {
                value = TreeNode_GetValue(resNode);
                if (value)
                {
                    dataType = xmlif_StringToDataType((const char *)value);
                }
            }

            resNode = TreeNode_Navigate(property, "Property/MaximumInstances");
            if (resNode)
            {
                value = TreeNode_GetValue(resNode);
                if (value)
                {
                    ResourceMaximumInstances = atoi((const char *)value);
                }
            }

            resNode = TreeNode_Navigate(property, "Property/MinimumInstances");
            if (resNode)
            {
                value = TreeNode_GetValue(resNode);
                if (value)
                {
                    ResourceMinimumInstances = atoi((const char *)value);
                }
            }

            resNode = TreeNode_Navigate(property, "Property/Access");
            if (resNode)
            {
                value = TreeNode_GetValue(resNode);
                if (value)
                {
                    operation = xmlif_StringToOperation((const char *)value);
                }
            }

            resNode = TreeNode_Navigate(property, "Property/DefaultValue");
            if (resNode)
            {
                defaultValueNode = Lwm2mTreeNode_Create();
                Lwm2mTreeNode_SetType(defaultValueNode, Lwm2mTreeNodeType_Resource);

                value = TreeNode_GetValue(resNode);

                const uint8_t * defaultValue = NULL;
                uint16_t defaultValueLength = 0;

                if (value)
                {
                    defaultValueLength = xmlif_DecodeValue((char **)&defaultValue, dataType, (const char *)value, strlen((const char *)value));
                }

                Lwm2mTreeNode * resourceInstanceNode = Lwm2mTreeNode_Create();
                Lwm2mTreeNode_AddChild(defaultValueNode, resourceInstanceNode);
                Lwm2mTreeNode_SetType(resourceInstanceNode, Lwm2mTreeNodeType_ResourceInstance);
                Lwm2mTreeNode_SetValue(resourceInstanceNode, defaultValue, defaultValueLength);
                Lwm2mTreeNode_SetID(resourceInstanceNode, 0);

                if (defaultValue)
                {
                    free((void *)defaultValue);
                }
            }
            else
            {
                resNode = TreeNode_Navigate(property, "Property/DefaultValueArray");
                if (resNode)
                {
                    defaultValueNode = Lwm2mTreeNode_Create();
                    Lwm2mTreeNode_SetType(defaultValueNode, Lwm2mTreeNodeType_Resource);

                    TreeNode resourceInstance;
                    uint32_t childIndex = 0;
                    while ((resourceInstance = Xml_FindFrom(resNode, "ResourceInstance", &childIndex)) != NULL)
                    {
                        int resourceInstanceID = xmlif_GetInteger(resourceInstance, "ResourceInstance/ID");
                        value = (const uint8_t *)xmlif_GetOpaque(resourceInstance, "ResourceInstance/Value");

                        const uint8_t * defaultValue = NULL;
                        uint16_t defaultValueLength = 0;

                        defaultValueLength = xmlif_DecodeValue((char **)&defaultValue, dataType, (const char *)value, strlen((const char *)value));

                        Lwm2mTreeNode * resourceInstanceNode = Lwm2mTreeNode_Create();
                        Lwm2mTreeNode_AddChild(defaultValueNode, resourceInstanceNode);
                        Lwm2mTreeNode_SetType(resourceInstanceNode, Lwm2mTreeNodeType_ResourceInstance);
                        Lwm2mTreeNode_SetValue(resourceInstanceNode, defaultValue, defaultValueLength);
                        Lwm2mTreeNode_SetID(resourceInstanceNode, resourceInstanceID);

                        free((void *)defaultValue);
                    }
                }
            }

            res = Definition_RegisterResourceType(definitions, resourceName ? resourceName : "", objectID, resourceID, dataType, ResourceMaximumInstances, ResourceMinimumInstances, operation, NULL, defaultValueNode);
            Lwm2mTreeNode_DeleteRecursive(defaultValueNode);
            if (res < 0)
            {
                result = false;
                goto error;
            }

            childIndex++;
        }
    }

error:
    return result;
}

static AwaError ConnectChannel(SessionCommon * session)
{
    // no SessionID to be specified
    IPCMessage * connectRequest = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_REQUEST, IPC_MESSAGE_SUB_TYPE_CONNECT, -1);
    IPCMessage * connectResponse = NULL;
    AwaError result = IPC_SendAndReceive(session->IPCChannel, connectRequest, &connectResponse, SESSION_CONNECT_TIMEOUT);

    if (result == AwaError_Success)
    {
        IPCResponseCode code = IPCMessage_GetResponseCode(connectResponse);
        if (code == IPCResponseCode_Success)
        {
            session->SessionID = IPCMessage_GetSessionID(connectResponse);
            if (session->SessionID > 0)
            {
                // populate object definition registry
                TreeNode content = IPCMessage_GetContentNode(connectResponse);

                if (content)
                {
                    TreeNode objectDefinitions = TreeNode_Navigate(content, "Content/ObjectDefinitions");
                    TreeNode objectDefinition = (objectDefinitions) ? TreeNode_GetChild(objectDefinitions, 0) : TreeNode_Navigate(content, "Content/ObjectDefinition");
                    int objectDefinitionIndex = 1;
                    int successCount = 0;
                    while (objectDefinition)
                    {
                        SessionCommon_RegisterObjectFromXML(session->DefinitionRegistry, objectDefinition);

                        successCount++;

                        TreeNode objectIDNode = TreeNode_Navigate(objectDefinition, "ObjectMetadata/ObjectID");
                        if (objectIDNode != NULL)
                        {
                            LogDebug("Defined object with ID %s", TreeNode_GetValue(objectIDNode));
                        }

                        objectDefinition = (objectDefinitions) ? TreeNode_GetChild(objectDefinitions, objectDefinitionIndex++) : NULL;
                    }

                    if (successCount + 1 == objectDefinitionIndex)
                    {
                        result = AwaError_Success;
                        LogDebug("%d object definitions received", successCount);
                    }
                    else
                    {
                        result = LogErrorWithEnum(AwaError_IPCError, "Definitions in connect message incorrect");
                    }
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_IPCError, "No connect response content");
                }
            }
            else
            {
                result = LogErrorWithEnum(AwaError_IPCError, "Connect failed with invalid session ID %d", session->SessionID);
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_IPCError, "Connect failed with code %d", code);
        }
        IPCMessage_Free(&connectResponse);
    }

    IPCMessage_Free(&connectRequest);
    return result;
}

static AwaError EstablishNotifyChannel(IPCChannel * ipcChannel, IPCSessionID sessionID)
{
    AwaError result = AwaError_Unspecified;
    if (ipcChannel != NULL)
    {
        IPCMessage * connectRequest = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_REQUEST, IPC_MESSAGE_SUB_TYPE_ESTABLISH_NOTIFY, sessionID);
        if (connectRequest != NULL)
        {
            IPCMessage * connectResponse = NULL;
            result = IPC_SendAndReceiveOnNotifySocket(ipcChannel, connectRequest, &connectResponse, SESSION_CONNECT_TIMEOUT);
            if (result == AwaError_Success)
            {
                IPCResponseCode code = IPCMessage_GetResponseCode(connectResponse);
                if (code == IPCResponseCode_Success)
                {
                    result = AwaError_Success;
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_IPCError, "Connect failed with code %d", code);
                }
                IPCMessage_Free(&connectResponse);
            }
            IPCMessage_Free(&connectRequest);
        }
        else
        {
            result = LogErrorWithEnum(AwaError_IPCError, "Failed to create message");
        }
    }
    return result;
}

AwaError SessionCommon_ConnectSession(SessionCommon * session)
{
    AwaError result = AwaError_Unspecified;
    if (session != NULL)
    {
        if (session->IPCInfo != NULL)
        {
            // check if already connected
            if (SessionCommon_IsConnected(session) == false)
            {
                session->IPCChannel = IPCChannel_New(session->IPCInfo);
                if (session->IPCChannel != NULL)
                {
                    result = ConnectChannel(session);
                    if (result == AwaError_Success)
                    {
                        result = EstablishNotifyChannel(session->IPCChannel, session->SessionID);
                        if (result == AwaError_Success)
                        {
                            LogVerbose("Session connected");
                        }
                    }
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_IPCError, "Channel missing");
                }

                if (result != AwaError_Success)
                {
                    IPCChannel_Free(&session->IPCChannel);
                }
            }
            else
            {
                result = LogErrorWithEnum(AwaError_IPCError, "Session already connected");
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_IPCError, "Session not configured");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid, "Session is NULL");
    }
    return result;
}

static AwaError DisconnectChannel(IPCChannel * ipcChannel, IPCSessionID sessionID)
{
    AwaError result = AwaError_Unspecified;
    if (ipcChannel != NULL)
    {
        IPCMessage * disconnectRequest = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_REQUEST, IPC_MESSAGE_SUB_TYPE_DISCONNECT, sessionID);
        if (disconnectRequest != NULL)
        {
            IPCMessage * disconnectResponse = NULL;
            result = IPC_SendAndReceive(ipcChannel, disconnectRequest, &disconnectResponse, SESSION_CONNECT_TIMEOUT);

            if (result == AwaError_Success)
            {
                IPCResponseCode code = IPCMessage_GetResponseCode(disconnectResponse);
                if (code == IPCResponseCode_Success)
                {
                    LogDebug("Disconnect OK");
                    result = AwaError_Success;
                }
                else
                {
                    LogErrorWithEnum(AwaError_IPCError, "Disconnect failed with code %d", code);
                }
                IPCMessage_Free(&disconnectResponse);
            }
            IPCMessage_Free(&disconnectRequest);
        }
        else
        {
            result = LogErrorWithEnum(AwaError_IPCError, "Failed to create message");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_IPCError, "ipcChannel is NULL");
    }
    return result;
}

AwaError SessionCommon_DisconnectSession(SessionCommon * session)
{
    AwaError result = AwaError_Unspecified;
    if (session != NULL)
    {
        if (session->IPCInfo != NULL)
        {
            // check if connected
            if (SessionCommon_IsConnected(session) != false)
            {
                // orderly disconnect from IPC server
                result = DisconnectChannel(session->IPCChannel, session->SessionID);
                if (result == AwaError_Success)
                {
                    LogVerbose("Session disconnected");
                }

                IPCChannel_Free(&session->IPCChannel);
            }
            else
            {
                result = LogErrorWithEnum(AwaError_SessionNotConnected, "Session not connected");
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_IPCError, "Session not configured");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid, "Session is NULL");
    }
    return result;
}

const AwaObjectDefinition * SessionCommon_GetObjectDefinition(const SessionCommon * session, AwaObjectID objectID)
{
    AwaObjectDefinition * objectDefinition = NULL;
    if (session != NULL)
    {
        objectDefinition = Definition_LookupObjectDefinition(session->DefinitionRegistry, (ObjectIDType)objectID);
        if (objectDefinition == NULL)
        {
            LogVerbose("object definition for %d not found", objectID);
        }
    }
    return objectDefinition;
}

AwaObjectDefinitionIterator * SessionCommon_NewObjectDefinitionIterator(const SessionCommon * session)
{
    AwaObjectDefinitionIterator * iterator = NULL;
    if (session != NULL)
    {
        iterator = Awa_MemAlloc(sizeof(*iterator));

        if(iterator != NULL)
        {
            memset(iterator, 0, sizeof(*iterator));
            iterator->ObjectID = AWA_INVALID_ID;
            iterator->Definitions = session->DefinitionRegistry;
        }
        else
        {
            LogErrorWithEnum(AwaError_OutOfMemory);
        }
    }
    else
    {
        LogError("AwaObjectDefinition is NULL");
    }
    return iterator;
}

bool SessionCommon_IsConnected(const SessionCommon * session)
{
    bool result = false;
    if (session != NULL)
    {
        result = (session->IPCChannel == NULL) ? false : true;
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid);
    }
    return result;
}

IPCChannel * SessionCommon_GetChannel(const SessionCommon * session)
{
    IPCChannel * channel = NULL;
    if (session != NULL)
    {
        channel = session->IPCChannel;
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid);
    }
    return channel;
}

DefinitionRegistry * SessionCommon_GetDefinitionRegistry(const SessionCommon * session)
{
    DefinitionRegistry * definitions = NULL;
    if (session != NULL)
    {
        definitions = session->DefinitionRegistry;
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid);
    }
    return definitions;
}

AwaError SessionCommon_PathToIDs(const SessionCommon * session, const char * path, AwaObjectID * objectID, AwaObjectInstanceID * objectInstanceID, AwaResourceID * resourceID)
{
    AwaError result = AwaError_Unspecified;
    if (session != NULL)
    {
        if (path != NULL)
        {
            if (Path_Parse(path, NULL, objectID, objectInstanceID, resourceID) != false)
            {
                result = AwaError_Success;
            }
            else
            {
                result = LogErrorWithEnum(AwaError_PathInvalid, "Invalid path %s", path);
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_PathInvalid, "Path is NULL");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid, "Session is NULL");
    }
    return result;
}

AwaError SessionCommon_CheckResourceTypeFromPath(const SessionCommon * session, const char * path, AwaResourceType expected)
{
    AwaError result = AwaError_Unspecified;
    if (session != NULL)
    {
        if (SessionCommon_IsConnected(session))
        {
            if (Path_IsValid(path))
            {
                const AwaResourceDefinition * resourceDefinition = SessionCommon_GetResourceDefinitionFromPath(session, path);
                if (resourceDefinition != NULL)
                {
                    AwaResourceType resourceType = AwaResourceDefinition_GetType(resourceDefinition);
                    if (resourceType == expected)
                    {
                        result = AwaError_Success;
                    }
                    else
                    {
                        result = LogErrorWithEnum(AwaError_TypeMismatch, "%s is not of type %s, received %s", path, Utils_ResourceTypeToString(expected), Utils_ResourceTypeToString(resourceType));
                    }
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_NotDefined, "%s is not defined", path);
                }
            }
            else
            {
                result = LogErrorWithEnum(AwaError_PathInvalid, "%s is not a valid path", path);
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_SessionNotConnected, "Session is not connected");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_SessionInvalid, "Session is NULL");
    }
    return result;
}

const AwaResourceDefinition * SessionCommon_GetResourceDefinitionFromPath(const SessionCommon * session, const char * path)
{
    const AwaResourceDefinition * result = NULL;
    if (session != NULL)
    {
        AwaObjectID objectID = Path_GetObjectID(path);
        const AwaObjectDefinition * objectDefinition = SessionCommon_GetObjectDefinition(session, objectID);

        if (objectDefinition != NULL)
        {
            AwaResourceID resourceID = Path_GetResourceID(path);
            result = AwaObjectDefinition_GetResourceDefinition(objectDefinition, resourceID);
        }
        else
        {
            LogErrorWithEnum(AwaError_NotDefined, "%s is not defined", path);
            result = NULL;
        }
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid, "Session is NULL");
    }
    return result;
}

bool SessionCommon_IsObjectDefined(const SessionCommon * session, AwaObjectID objectID)
{
    return (SessionCommon_GetObjectDefinition(session, objectID) != NULL);
}

AwaError SessionCommon_SendDefineMessage(const SessionCommon * session, const TreeNode objectDefinitionsNode, int32_t timeout)
{
    AwaError result = AwaError_Unspecified;

    if (objectDefinitionsNode != NULL)
    {
        IPCMessage * registerRequest = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_REQUEST, IPC_MESSAGE_SUB_TYPE_DEFINE, SessionCommon_GetSessionID(session));
        if (registerRequest != NULL)
        {
            if ((result = IPCMessage_AddContent(registerRequest, objectDefinitionsNode)) == AwaError_Success)
            {
                IPCMessage * registerResponse = NULL;
                result = IPC_SendAndReceive(SessionCommon_GetChannel(session), registerRequest, &registerResponse, (timeout > 0) ? timeout : -1);

                if (registerResponse != NULL)
                {
                    IPCResponseCode responseCode = IPCMessage_GetResponseCode(registerResponse);
                    if (responseCode == IPCResponseCode_Success)
                    {
                        result = AwaError_Success;
                    }
                    else
                    {
                        result = LogErrorWithEnum(AwaError_IPCError, "Unexpected IPC response code: %d", responseCode);
                    }
                    IPCMessage_Free(&registerResponse);
                }
            }
            IPCMessage_Free(&registerRequest);
        }
        else
        {
            result = LogErrorWithEnum(AwaError_IPCError);
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_DefinitionInvalid, "objectDefinitionsNode is NULL");
    }
    return result;
}

SessionType SessionCommon_GetSessionType(const SessionCommon * session)
{
    SessionType type = SessionType_Invalid;
    if (session != NULL)
    {
        type = session->SessionType;
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid, "Session is NULL");
    }
    return type;
}

IPCSessionID SessionCommon_GetSessionID(const SessionCommon * session)
{
    IPCSessionID sessionID = -1;
    if (session != NULL)
    {
        sessionID = session->SessionID;
    }
    else
    {
        LogErrorWithEnum(AwaError_SessionInvalid, "Session is NULL");
    }
    return sessionID;
}


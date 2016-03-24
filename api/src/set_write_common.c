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


#include "operation_common.h"
#include "error.h"
#include "path.h"
#include "xml.h"
#include "log.h"
#include "memalloc.h"
#include "session_common.h"
#include "client_session.h"
#include "set_write_common.h"
#include "lwm2m_xml_interface.h"
#include "lwm2m_result.h"
#include "lwm2m_xml_serdes.h"
#include "utils.h"
#include "arrays.h"
#include "objects_tree.h"

#define MAX_RESOURCE_INSTANCE_STR_LEN (12)

static const char * SetArrayModeStrings[] =
{
    "Unspecified",
    "Replace",
    "Update",
};

static const char * SetArrayModeToString(SetArrayMode mode)
{
    const char * modeString = NULL;

    if (mode > SetArrayMode_Unspecified && mode < SetArrayMode_LAST)
    {
        modeString = SetArrayModeStrings[mode];
    }
    else
    {
        LogErrorWithEnum(AwaError_Internal, "Unsupported SetArrayMode: %d", mode);
    }

    return modeString;
}

static SetArrayMode SetArrayModeFromString(char * setArrayModeString)
{
    SetArrayMode mode = SetArrayMode_Unspecified;
    int i = 0;
    for (; i < SetArrayMode_LAST; i++)
    {
        if (strcmp(setArrayModeString, SetArrayModeStrings[i]) == 0)
        {
            mode = i;
            break;
        }
    }
    return mode;
}

AwaError SetWriteCommon_AddValue(OperationCommon * operation, SessionType sessionType, const char * path, int resourceInstanceID, void * value, size_t size, AwaResourceType type, SetArrayMode setArrayMode)
{
    AwaError result = AwaError_Unspecified;

    if (operation != NULL)
    {
        if (path != NULL)
        {
            if (Path_IsValidForResource(path))
            {
                const Session * session = OperationCommon_GetSession(operation, NULL);
                if (session != NULL)
                {
                    SessionCommon * sessionCommon = NULL;
                    switch(sessionType)
                    {
                        case SessionType_Client:
                            sessionCommon = ClientSession_GetSessionCommon((AwaClientSession*)session);
                            break;
                        case SessionType_Server:
                            sessionCommon = ServerSession_GetSessionCommon((AwaServerSession*)session);
                            break;
                        default:
                            result = LogErrorWithEnum(AwaError_Internal, "SessionType is invalid: %d\n", sessionType);
                            goto error;
                    }
                    if (SessionCommon_IsConnected(sessionCommon))
                    {
                        const AwaResourceDefinition * resourceDefinition = NULL;
                        switch(sessionType)
                        {
                            case SessionType_Client:
                                resourceDefinition = ClientSession_GetResourceDefinitionFromPath((AwaClientSession*)session, path);
                                break;
                            case SessionType_Server:
                                resourceDefinition = ServerSession_GetResourceDefinitionFromPath((AwaServerSession*)session, path);
                                break;
                            default:
                                result = LogErrorWithEnum(AwaError_Internal, "SessionType is invalid: %d\n", sessionType);
                                goto error;
                        }
                        if (resourceDefinition != NULL)
                        {
                            // ensure the type matches:
                            if (AwaResourceDefinition_GetType(resourceDefinition) == type)
                            {
                                TreeNode objectsTree = OperationCommon_GetObjectsTree(operation);
                                if (objectsTree != NULL)
                                {
                                    TreeNode resultNode;
                                    ObjectsTree_FindPathNode(objectsTree, path, &resultNode);

                                    if (ObjectsTree_GetNumChildrenWithName(resultNode, "ResourceInstance") < resourceDefinition->MaximumInstances)
                                    {
                                        char * encodedValue = xmlif_EncodeValue(Utils_GetPrimativeResourceType(type), value, size);

                                        if (ObjectsTree_AddPath(objectsTree, path, &resultNode) == InternalError_Success && resultNode != NULL)
                                        {
                                            if (resourceDefinition->MaximumInstances == 1) // single instance resource
                                            {
                                                if (resourceInstanceID == 0)
                                                {
                                                    if (SetWriteCommon_SetResourceNodeValue(resultNode, encodedValue) == InternalError_Success)
                                                    {
                                                        result = AwaError_Success;
                                                    }
                                                    else
                                                    {
                                                        result = LogErrorWithEnum(AwaError_Internal, "Failed to set resource value");
                                                    }
                                                }
                                                else
                                                {
                                                    result = LogErrorWithEnum(AwaError_Internal, "Resource instance ID is not 0 on single instance resource");
                                                }
                                            }
                                            else
                                            {
                                                if (setArrayMode != SetArrayMode_Unspecified)
                                                {
                                                    TreeNode setArrayModeNode = Xml_Find(resultNode, "SetArrayMode");

                                                    if (!setArrayModeNode)
                                                    {
                                                        setArrayModeNode = Xml_CreateNodeWithValue("SetArrayMode", "%s", SetArrayModeToString(setArrayMode));
                                                        TreeNode_AddChild(resultNode, setArrayModeNode);
                                                    }
                                                    else
                                                    {
                                                        SetArrayMode currentMode = SetArrayModeFromString((char *)TreeNode_GetValue(setArrayModeNode));
                                                        if (currentMode == SetArrayMode_Update && setArrayMode == SetArrayMode_Replace)
                                                        {
                                                            char * newModeString = NULL;
                                                            msprintf(&newModeString, "%s", SetArrayModeToString(setArrayMode));
                                                        }
                                                    }
                                                }

                                                if (SetWriteCommon_AddValueToResourceNode(resultNode, resourceInstanceID, encodedValue) == InternalError_Success)
                                                {
                                                    result = AwaError_Success;
                                                }
                                                else
                                                {
                                                    result = LogErrorWithEnum(AwaError_Internal, "Failed to add value to path");
                                                }
                                            }
                                        }
                                        else
                                        {
                                            result = LogErrorWithEnum(AwaError_Internal, "AddPath failed");
                                        }

                                        Awa_MemSafeFree(encodedValue);
                                    }
                                    else
                                    {
                                        result = LogErrorWithEnum(AwaError_AddInvalid, "Exceeded maximum number of instances in definition (%d)", resourceDefinition->MaximumInstances);
                                    }
                                }
                                else
                                {
                                    result = LogErrorWithEnum(AwaError_Internal, "ObjectsTree is NULL");
                                }
                            }
                            else
                            {
                                result = LogErrorWithEnum(AwaError_TypeMismatch, "%s is not of type %s, received %s", path, Utils_ResourceTypeToString(type), Utils_ResourceTypeToString(AwaResourceDefinition_GetType(resourceDefinition)));
                            }
                        }
                        else
                        {
                            result = LogErrorWithEnum(AwaError_NotDefined, "%s is not defined", path);
                        }
                    }
                    else
                    {
                        result = LogErrorWithEnum(AwaError_SessionNotConnected);
                    }
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_SessionInvalid, "Session is NULL");
                }
            }
            else
            {
                result = LogErrorWithEnum(AwaError_PathInvalid, "%s is not a resource path", path);
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_PathInvalid, "Path is NULL");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "Operation is NULL");
    }
error:
    return result;
}

InternalError SetWriteCommon_SetResourceNodeValue(TreeNode resourceNode, const char * value)
{
    InternalError result = InternalError_Unspecified;
    TreeNode valueNode = Xml_CreateNodeWithValue("Value", "%s", value);

    if (TreeNode_AddChild(resourceNode, valueNode))
    {
        result = InternalError_Success;
    }
    else
    {
        LogError("Failed to add value to resource node");
        result = InternalError_Tree;
    }
    return result;
}

InternalError SetWriteCommon_AddValueToResourceNode(TreeNode resourceNode, int resourceInstanceID, const char * value)
{
    InternalError result = InternalError_Unspecified;

    if (resourceNode != NULL)
    {
        if (value != NULL)
        {
            if ((resourceInstanceID > AWA_INVALID_ID) && (resourceInstanceID < AWA_MAX_ID))
            {
                if (strcmp(TreeNode_GetName(resourceNode),"Resource") == 0)
                {
                    char resourceInstanceIDString[MAX_RESOURCE_INSTANCE_STR_LEN];
                    sprintf(resourceInstanceIDString, "%d", resourceInstanceID);
                    if (Xml_FindChildWithGrandchildValue(resourceNode, "ResourceInstance", "ID", resourceInstanceIDString) == NULL)
                    {
                        TreeNode resourceInstanceNode = Xml_CreateNode("ResourceInstance");
                        TreeNode idNode = Xml_CreateNodeWithValue("ID", "%d", resourceInstanceID);
                        TreeNode valueNode = Xml_CreateNodeWithValue("Value", "%s", value);

                        if (TreeNode_AddChild(resourceInstanceNode, idNode))
                        {
                            if (TreeNode_AddChild(resourceInstanceNode, valueNode))
                            {
                                if (TreeNode_AddChild(resourceNode, resourceInstanceNode))
                                {
                                    result = InternalError_Success;
                                }
                                else
                                {
                                    LogError("Failed to add resource instance node to resource node");
                                    result = InternalError_Tree;
                                }
                            }
                            else
                            {
                                LogError("Failed to add value to resource instance node");
                                result = InternalError_Tree;
                            }
                        }
                        else
                        {
                            LogError("Failed to add ID to resource instance node");
                            result = InternalError_Tree;
                        }
                    }
                    else
                    {
                        LogError("Resource instance ID already exists in tree: %d\n", resourceInstanceID);
                        result = InternalError_Tree;
                    }
                }
                else
                {
                    LogError("Input node is not a resource node: %s", TreeNode_GetName(resourceNode));
                    result = InternalError_ParameterInvalid;
                }
            }
            else
            {
                LogError("invalid resource instance id: %d", resourceInstanceID);
                result = InternalError_ParameterInvalid;
            }
        }
        else
        {
            LogError("value is NULL");
            result = InternalError_ParameterInvalid;
        }
    }
    else
    {
        LogError("node is NULL");
        result = InternalError_ParameterInvalid;
    }
    return result;
}


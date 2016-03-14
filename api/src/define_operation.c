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


#include "define_operation.h"
#include "operation_common.h"
#include "lwm2m_definition.h"
#include "log.h"
#include "memalloc.h"

#include "lwm2m_xml_interface.h"
#include "lwm2m_xml_serdes.h"
#include "xml.h"
#include "xmltree.h"
#include "set_write_common.h"
#include "utils.h"

struct _DefineOperation
{
    OperationCommon * Common;
    DefinitionRegistry * DefinitionRegistry;
};

static DefineOperation * _DefineOperation_New(const Session * session, SessionType sessionType)
{
    DefineOperation * operation = NULL;
    if (session != NULL)
    {
        operation = (DefineOperation *)Awa_MemAlloc(sizeof(*operation));
        if (operation != NULL)
        {
            memset(operation, 0, sizeof(*operation));
            operation->Common = OperationCommon_New(session, sessionType);
            operation->DefinitionRegistry = DefinitionRegistry_Create();
            LogNew("DefineOperation", session);
        }
        else
        {
            LogErrorWithEnum(AwaError_OutOfMemory);
        }
    }
    return operation;
}

DefineOperation * DefineOperation_NewWithClientSession(const AwaClientSession * session)
{
    return _DefineOperation_New(session, SessionType_Client);
}

DefineOperation * DefineOperation_NewWithServerSession(const AwaServerSession * session)
{
    return _DefineOperation_New(session, SessionType_Server);
}

AwaError DefineOperation_Free(DefineOperation ** operation)
{
    AwaError result = AwaError_Unspecified;

    if ((operation != NULL) && (*operation != NULL))
    {
        OperationCommon_Free(&(*operation)->Common);
        DefinitionRegistry_Destroy((*operation)->DefinitionRegistry);

        LogFree("DefineOperation", *operation);
        Awa_MemSafeFree(*operation);
        *operation = NULL;

        result = AwaError_Success;
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "Operation is NULL");
    }
    return result;
}

AwaError DefineOperation_Add(DefineOperation * operation, const AwaObjectDefinition * objectDefinition)
{
    AwaError result = AwaError_Unspecified;

    if (operation != NULL)
    {
        if (objectDefinition != NULL)
        {
            AwaObjectDefinition * objectDefinitionCopy = Definition_CopyObjectDefinition(objectDefinition);

            if (Definition_AddObjectType(operation->DefinitionRegistry, objectDefinitionCopy) != 0)
            {
                AwaObjectDefinition_Free(&objectDefinitionCopy);
                result = LogErrorWithEnum(AwaError_DefinitionInvalid);
            }
            else
            {
                result = AwaError_Success;
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_DefinitionInvalid);
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_DefinitionInvalid);
    }
    return result;
}

static TreeNode DefineOperation_ConstructObjectDefinitionNode(const DefinitionRegistry * definitions, const ObjectDefinition * objFormat, int objectID)
{
    TreeNode objectMetaData = Xml_CreateNode("ObjectMetadata");

    TreeNode_AddChild(objectMetaData, Xml_CreateNodeWithValue("ObjectID", "%d", objFormat->ObjectID));
    TreeNode_AddChild(objectMetaData, Xml_CreateNodeWithValue("SerialisationName", "%s", objFormat->ObjectName));

    //FIXME: These fields are deprecated in APIv2 - should be removed when APIv1 is obsoleted
    TreeNode_AddChild(objectMetaData, Xml_CreateNodeWithValue("Singleton", "%s", IS_MULTIPLE_INSTANCE(objFormat) ? "False" : "True"));
    TreeNode_AddChild(objectMetaData, Xml_CreateNodeWithValue("IsMandatory", "%s", IS_MANDATORY(objFormat) ? "True" : "False"));
    TreeNode_AddChild(objectMetaData, Xml_CreateNodeWithValue("MaximumInstances", "%d", objFormat->MaximumInstances));
    TreeNode_AddChild(objectMetaData, Xml_CreateNodeWithValue("MinimumInstances", "%d", objFormat->MinimumInstances));

    TreeNode properties = Xml_CreateNode("Properties");
    TreeNode_AddChild(objectMetaData, properties);

    int resourceID = AWA_INVALID_ID;
    while ((resourceID = Definition_GetNextResourceType(definitions, objectID, resourceID)) != AWA_INVALID_ID)
    {
        TreeNode property = Xml_CreateNode("Property");

        ResourceDefinition * resFormat = Definition_LookupResourceDefinition(definitions, objectID, resourceID);
        if(!resFormat)
        {
            Tree_Delete(objectMetaData);
            Tree_Delete(property);
            goto error;
        }

        TreeNode_AddChild(property, Xml_CreateNodeWithValue("PropertyID", "%d", resFormat->ResourceID));
        TreeNode_AddChild(property, Xml_CreateNodeWithValue("DataType", "%s", xmlif_DataTypeToString(resFormat->Type)));
        TreeNode_AddChild(property, Xml_CreateNodeWithValue("Access", "%s", xmlif_OperationToString(resFormat->Operation)));
        TreeNode_AddChild(property, Xml_CreateNodeWithValue("SerialisationName", "%s", resFormat->ResourceName));

        //FIXME: These fields are deprecated in APIv2 - should be removed when APIv1 is obsoleted
        TreeNode_AddChild(property, Xml_CreateNodeWithValue("IsCollection", "%s", IS_MULTIPLE_INSTANCE(resFormat) ? "True" : "False"));
        TreeNode_AddChild(property, Xml_CreateNodeWithValue("IsMandatory", "%s", IS_MANDATORY(resFormat) ? "True" : "False"));
        TreeNode_AddChild(property, Xml_CreateNodeWithValue("MaximumInstances", "%d", resFormat->MaximumInstances));
        TreeNode_AddChild(property, Xml_CreateNodeWithValue("MinimumInstances", "%d", resFormat->MinimumInstances));

        if (resFormat->DefaultValueNode != NULL)
        {
            if (!IS_MULTIPLE_INSTANCE(resFormat))
            {
                Lwm2mTreeNode * child = Lwm2mTreeNode_GetFirstChild(resFormat->DefaultValueNode);
                uint16_t defaultValueLength;
                const uint8_t * defaultValue = Lwm2mTreeNode_GetValue(child, &defaultValueLength);

                if (defaultValue != NULL)
                {
                    char * value = xmlif_EncodeValue(resFormat->Type, (const char *)defaultValue, defaultValueLength);
                    TreeNode_AddChild(property, Xml_CreateNodeWithValue("DefaultValue", value));
                    Awa_MemSafeFree(value);
                }
                else
                {
                    TreeNode_AddChild(property, Xml_CreateNodeWithValue("DefaultValue", ""));
                }
            }
            else
            {
                TreeNode defaultValueNode = Xml_CreateNode("DefaultValueArray");

                Lwm2mTreeNode * child = Lwm2mTreeNode_GetFirstChild(resFormat->DefaultValueNode);

                while(child)
                {
                    int resourceInstanceID;

                    Lwm2mTreeNode_GetID(child, &resourceInstanceID);

                    uint16_t defaultValueLength;
                    const uint8_t * defaultValue = Lwm2mTreeNode_GetValue(child, &defaultValueLength);

                    TreeNode resourceInstanceNode = Xml_CreateNode("ResourceInstance");

                    TreeNode_AddChild(resourceInstanceNode, Xml_CreateNodeWithValue("ID", "%d", resourceInstanceID));
                    if (defaultValue != NULL)
                    {
                        char * value = xmlif_EncodeValue(resFormat->Type, (const char *)defaultValue, defaultValueLength);
                        TreeNode_AddChild(resourceInstanceNode, Xml_CreateNodeWithValue("Value", value));
                        Awa_MemSafeFree(value);
                    }
                    else
                    {
                        TreeNode_AddChild(resourceInstanceNode, Xml_CreateNodeWithValue("Value", ""));
                    }

                    TreeNode_AddChild(defaultValueNode, resourceInstanceNode);
                    child = Lwm2mTreeNode_GetNextChild(resFormat->DefaultValueNode, child);
                }
                TreeNode_AddChild(property, defaultValueNode);
            }
        }

        TreeNode_AddChild(properties, property);
    }
    return objectMetaData;

error:
    return NULL;
}

AwaError DefineOperation_Perform(DefineOperation * operation, AwaTimeout timeout)
{
    AwaError result = AwaError_Unspecified;

    if (timeout >= 0)
    {
        if ((operation != NULL) && (operation->DefinitionRegistry != NULL))
        {
            const SessionCommon * sessionCommon = OperationCommon_GetSessionCommon(operation->Common);
            if (sessionCommon != NULL)
            {
                if (SessionCommon_IsConnected(sessionCommon))
                {
                    // create a container for object definitions
                    TreeNode objectDefinitionsNode = Xml_CreateNode("ObjectDefinitions");

                    // iterate through all defined object types, adding metadata descriptions to tree
                    ObjectIDType objectID = AWA_INVALID_ID;
                    while ((objectID = Definition_GetNextObjectType(operation->DefinitionRegistry, objectID)) != AWA_INVALID_ID)
                    {
                        AwaObjectDefinition * objFormat = Definition_LookupObjectDefinition(operation->DefinitionRegistry, objectID);
                        if (objFormat != NULL)
                        {
                            TreeNode objectMetaData = DefineOperation_ConstructObjectDefinitionNode(operation->DefinitionRegistry, objFormat, objectID);
                            if (objectMetaData != NULL)
                            {
                                TreeNode_AddChild(objectDefinitionsNode, objectMetaData);
                            }
                            else
                            {
                                Tree_Delete(objectDefinitionsNode);
                                objectDefinitionsNode = NULL;
                                break;
                            }
                        }
                    }

                    if (objectDefinitionsNode != NULL)
                    {
                        result = SessionCommon_SendDefineMessage(sessionCommon, objectDefinitionsNode, timeout);
                        Tree_Delete(objectDefinitionsNode);
                        if (result == AwaError_Success)
                        {
                            objectID = AWA_INVALID_ID;
                            while ((objectID = Definition_GetNextObjectType(operation->DefinitionRegistry, objectID)) != AWA_INVALID_ID)
                            {
                                AwaObjectDefinition * objFormat = Definition_LookupObjectDefinition(operation->DefinitionRegistry, objectID);

                                if (objFormat != NULL)
                                {
                                    ObjectDefinition * objectDefinition = Definition_CopyObjectDefinition(objFormat);
                                    if (objectDefinition != NULL)
                                    {
                                        if (Definition_AddObjectType(SessionCommon_GetDefinitionRegistry(sessionCommon), objectDefinition) != 0)
                                        {
                                            Definition_FreeObjectType(objectDefinition);
                                            result = LogErrorWithEnum(AwaError_AlreadyDefined, "Failed to add object definition");
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        result = LogErrorWithEnum(AwaError_DefinitionInvalid);
                    }
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_SessionNotConnected);
                }
            }
            else
            {
                result = LogErrorWithEnum(AwaError_SessionInvalid);
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_OperationInvalid);
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "Invalid timeout specified");
    }
    return result;
}


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

#include "lwm2m_core.h"
#include "lwm2m_definition.h"
#include "server/lwm2m_server_xml_handlers.h"

#include "awa/client.h"
#include "define_common.h"
#include "memalloc.h"
#include "xml.h"
#include "ipc.h"
#include "log.h"
#include "client_session.h"
#include "utils.h"
#include "session_common.h"
#include "arrays.h"

#define RESOURCE_HANDLER    NULL
#define IS_VALID_ID(ID)  (ID >= 0 && ID < AWA_MAX_ID)

AwaObjectDefinition * AwaObjectDefinition_New(AwaObjectID objectID, const char * objectName, int minimumInstances, int maximumInstances)
{
    AwaObjectDefinition * definition = NULL;
    if ((IS_VALID_ID(objectID)) &&
       (minimumInstances >= 0 && minimumInstances <= AWA_MAX_ID) &&
       (maximumInstances > 0 && maximumInstances <= AWA_MAX_ID) &&
       (objectName != NULL && minimumInstances <= maximumInstances))
    {
        definition = Definition_NewObjectType(objectName, objectID, maximumInstances, minimumInstances, NULL);
        LogNew("AwaObjectDefinition", definition);
    }
    return definition;
}

void AwaObjectDefinition_Free(AwaObjectDefinition ** objectDefinition)
{
    if ((objectDefinition != NULL) && (*objectDefinition != NULL))
    {
        LogFree("AwaObjectDefinition", *objectDefinition);
        Definition_FreeObjectType(*objectDefinition);
        *objectDefinition = NULL;
    }
}

AwaError ObjectDefinition_AddResourceDefinition(AwaObjectDefinition * objectDefinition, AwaResourceType resourceType, AwaResourceID resourceID, const char * resourceName, int minimumInstances, int maximumInstances, AwaResourceOperations operations, Lwm2mTreeNode * defaultValueNode)
{
    AwaError result = AwaError_Unspecified;

    if ((objectDefinition != NULL) &&
        (resourceName != NULL) &&
        ((operations == AwaResourceOperations_Execute) == (resourceType == AwaResourceType_None)))
    {
        if (IS_VALID_ID(resourceID))
        {
            if (Definition_LookupResourceDefinitionFromObjectDefinition(objectDefinition, resourceID) == NULL)
            {
                AwaResourceType coreResourceType = Utils_GetPrimativeResourceType(resourceType);

                if (Definition_NewResourceType(objectDefinition, resourceName, resourceID, coreResourceType, maximumInstances, minimumInstances, operations, RESOURCE_HANDLER, defaultValueNode))
                {
                    result = AwaError_Success;
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_OutOfMemory);
                }
            }
            else
            {
                result = LogErrorWithEnum(AwaError_AlreadyDefined);
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_IDInvalid);
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_DefinitionInvalid);
    }

    Lwm2mTreeNode_DeleteRecursive(defaultValueNode);
    return result;
}

static Lwm2mTreeNode * SingleDefaultValueToTreeNode(uint8_t * defaultValue, uint16_t defaultValueLength)
{
    Lwm2mTreeNode * resourceNode = Lwm2mTreeNode_Create();

    Lwm2mTreeNode_SetType(resourceNode, Lwm2mTreeNodeType_Resource);

    Lwm2mTreeNode * resourceInstanceNode = Lwm2mTreeNode_Create();
    Lwm2mTreeNode_AddChild(resourceNode, resourceInstanceNode);
    Lwm2mTreeNode_SetType(resourceInstanceNode, Lwm2mTreeNodeType_ResourceInstance);
    Lwm2mTreeNode_SetValue(resourceInstanceNode, defaultValue, defaultValueLength);
    Lwm2mTreeNode_SetID(resourceInstanceNode, 0);  // single instance
    return resourceNode;
}

static Lwm2mTreeNode * ArrayDefaultValueToTreeNode(AwaArray * array, AwaResourceType resourceType)
{
    Lwm2mTreeNode * resourceNode = NULL;

    if (array != NULL)
    {
        resourceNode = Lwm2mTreeNode_Create();
        Lwm2mTreeNode_SetType(resourceNode, Lwm2mTreeNodeType_Resource);
        AwaArrayIterator * iterator = ArrayIterator_New(array);

        while (ArrayIterator_Next(iterator))
        {
            uint8_t * defaultValue = NULL;
            uint16_t defaultValueLength = 0;
            if (resourceType == AwaResourceType_OpaqueArray)
            {
                AwaOpaque * opaque = ArrayIterator_GetValue(iterator);
                defaultValue = opaque->Data;
                defaultValueLength = opaque->Size;
            }
            else
            {
                defaultValue = ArrayIterator_GetValue(iterator);
                defaultValueLength = ArrayIterator_GetValueLength(iterator);
            }

            int resourceInstanceID = ArrayIterator_GetIndex(iterator);

            Lwm2mTreeNode * resourceInstanceNode = Lwm2mTreeNode_Create();
            Lwm2mTreeNode_AddChild(resourceNode, resourceInstanceNode);
            Lwm2mTreeNode_SetType(resourceInstanceNode, Lwm2mTreeNodeType_ResourceInstance);
            Lwm2mTreeNode_SetValue(resourceInstanceNode, defaultValue, defaultValueLength);
            Lwm2mTreeNode_SetID(resourceInstanceNode, resourceInstanceID);
        }
        ArrayIterator_Free(&iterator);
    }
    return resourceNode;
}

AwaError AwaObjectDefinition_AddResourceDefinitionAsNoType(AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, bool isMandatory, AwaResourceOperations operations)
{
    return ObjectDefinition_AddResourceDefinition(objectDefinition, AwaResourceType_None, resourceID, resourceName,  isMandatory ? 1 : 0, 1, operations, NULL);
}

AwaError AwaObjectDefinition_AddResourceDefinitionAsString(AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, bool isMandatory, AwaResourceOperations operations, const char * defaultValue)
{
    return ObjectDefinition_AddResourceDefinition(objectDefinition, AwaResourceType_String, resourceID, resourceName,  isMandatory ? 1 : 0, 1, operations, defaultValue != NULL? SingleDefaultValueToTreeNode((uint8_t *)defaultValue, strlen(defaultValue)) : NULL);
}

AwaError AwaObjectDefinition_AddResourceDefinitionAsStringArray(AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, int minimumInstances, int maximumInstances, AwaResourceOperations operations, const AwaStringArray * defaultArray)
{
    return ObjectDefinition_AddResourceDefinition(objectDefinition, AwaResourceType_StringArray, resourceID, resourceName, minimumInstances, maximumInstances, operations, ArrayDefaultValueToTreeNode((AwaArray *)defaultArray, AwaResourceType_StringArray));
}

AwaError AwaObjectDefinition_AddResourceDefinitionAsInteger(AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, bool isMandatory, AwaResourceOperations operations, AwaInteger defaultValue)
{
    return ObjectDefinition_AddResourceDefinition(objectDefinition, AwaResourceType_Integer, resourceID, resourceName,  isMandatory ? 1 : 0, 1, operations, SingleDefaultValueToTreeNode((uint8_t *)&defaultValue, sizeof(AwaInteger)));
}

AwaError AwaObjectDefinition_AddResourceDefinitionAsIntegerArray(AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, int minimumInstances, int maximumInstances, AwaResourceOperations operations, const AwaIntegerArray * defaultArray)
{
    return ObjectDefinition_AddResourceDefinition(objectDefinition, AwaResourceType_IntegerArray, resourceID, resourceName, minimumInstances, maximumInstances, operations, ArrayDefaultValueToTreeNode((AwaArray *)defaultArray, AwaResourceType_IntegerArray));
}

AwaError AwaObjectDefinition_AddResourceDefinitionAsFloat(AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, bool isMandatory, AwaResourceOperations operations, AwaFloat defaultValue)
{
    return ObjectDefinition_AddResourceDefinition(objectDefinition, AwaResourceType_Float, resourceID, resourceName,  isMandatory ? 1 : 0, 1, operations, SingleDefaultValueToTreeNode((uint8_t *)&defaultValue, sizeof(AwaFloat)));
}

AwaError AwaObjectDefinition_AddResourceDefinitionAsFloatArray(AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, int minimumInstances, int maximumInstances, AwaResourceOperations operations, const AwaFloatArray * defaultArray)
{
    return ObjectDefinition_AddResourceDefinition(objectDefinition, AwaResourceType_FloatArray, resourceID, resourceName, minimumInstances, maximumInstances, operations, ArrayDefaultValueToTreeNode((AwaArray *)defaultArray, AwaResourceType_FloatArray));
}

AwaError AwaObjectDefinition_AddResourceDefinitionAsBoolean(AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, bool isMandatory, AwaResourceOperations operations, AwaBoolean defaultValue)
{
    return ObjectDefinition_AddResourceDefinition(objectDefinition, AwaResourceType_Boolean, resourceID, resourceName,  isMandatory ? 1 : 0, 1, operations, SingleDefaultValueToTreeNode((uint8_t *)&defaultValue, sizeof(AwaBoolean)));
}

AwaError AwaObjectDefinition_AddResourceDefinitionAsBooleanArray(AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, int minimumInstances, int maximumInstances, AwaResourceOperations operations, const AwaBooleanArray * defaultArray)
{
    return ObjectDefinition_AddResourceDefinition(objectDefinition, AwaResourceType_BooleanArray, resourceID, resourceName, minimumInstances, maximumInstances, operations, ArrayDefaultValueToTreeNode((AwaArray *)defaultArray, AwaResourceType_BooleanArray));
}

AwaError AwaObjectDefinition_AddResourceDefinitionAsOpaque(AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, bool isMandatory, AwaResourceOperations operations, AwaOpaque defaultValue)
{
    return ObjectDefinition_AddResourceDefinition(objectDefinition, AwaResourceType_Opaque, resourceID, resourceName,  isMandatory ? 1 : 0, 1, operations, SingleDefaultValueToTreeNode((uint8_t *)defaultValue.Data, defaultValue.Size));
}

AwaError AwaObjectDefinition_AddResourceDefinitionAsOpaqueArray(AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, int minimumInstances, int maximumInstances, AwaResourceOperations operations, const AwaOpaqueArray * defaultArray)
{
    return ObjectDefinition_AddResourceDefinition(objectDefinition, AwaResourceType_OpaqueArray, resourceID, resourceName, minimumInstances, maximumInstances, operations, ArrayDefaultValueToTreeNode((AwaArray *)defaultArray, AwaResourceType_OpaqueArray));
}

AwaError AwaObjectDefinition_AddResourceDefinitionAsTime(AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, bool isMandatory, AwaResourceOperations operations, AwaTime defaultValue)
{
    return ObjectDefinition_AddResourceDefinition(objectDefinition, AwaResourceType_Time, resourceID, resourceName,  isMandatory ? 1 : 0, 1, operations, SingleDefaultValueToTreeNode((uint8_t *)&defaultValue, sizeof(AwaTime)));
}

AwaError AwaObjectDefinition_AddResourceDefinitionAsTimeArray(AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, int minimumInstances, int maximumInstances, AwaResourceOperations operations, const AwaTimeArray * defaultArray)
{
    return ObjectDefinition_AddResourceDefinition(objectDefinition, AwaResourceType_TimeArray, resourceID, resourceName, minimumInstances, maximumInstances, operations, ArrayDefaultValueToTreeNode((AwaArray *)defaultArray, AwaResourceType_TimeArray));
}

AwaError AwaObjectDefinition_AddResourceDefinitionAsObjectLink(AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, bool isMandatory, AwaResourceOperations operations, AwaObjectLink defaultValue)
{
    return ObjectDefinition_AddResourceDefinition(objectDefinition, AwaResourceType_ObjectLink, resourceID, resourceName,  isMandatory ? 1 : 0, 1, operations, SingleDefaultValueToTreeNode((uint8_t *)&defaultValue, sizeof(AwaObjectLink)));
}

AwaError AwaObjectDefinition_AddResourceDefinitionAsObjectLinkArray(AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, int minimumInstances, int maximumInstances, AwaResourceOperations operations, const AwaObjectLinkArray * defaultArray)
{
    return ObjectDefinition_AddResourceDefinition(objectDefinition, AwaResourceType_ObjectLinkArray, resourceID, resourceName, minimumInstances, maximumInstances, operations, ArrayDefaultValueToTreeNode((AwaArray *)defaultArray, AwaResourceType_ObjectLinkArray));
}

AwaObjectID AwaObjectDefinition_GetID(const AwaObjectDefinition * objectDefinition)
{
    AwaObjectID objectID = AWA_INVALID_ID;

    if (objectDefinition != NULL)
    {
        objectID = objectDefinition->ObjectID;
    }

    return objectID;
}

const char * AwaObjectDefinition_GetName(const AwaObjectDefinition * objectDefinition)
{
    const char * name = NULL;

    if (objectDefinition != NULL)
    {
        name = objectDefinition->ObjectName;
    }

    return name;
}

int AwaObjectDefinition_GetMaximumInstances (const AwaObjectDefinition * objectDefinition)
{
    int MaximumInstances = -1;

    if (objectDefinition != NULL)
    {
        MaximumInstances = objectDefinition->MaximumInstances;
    }
    return MaximumInstances;
}

int AwaObjectDefinition_GetMinimumInstances (const AwaObjectDefinition * objectDefinition)
{
    int MinimumInstances = -1;

    if (objectDefinition != NULL)
    {
        MinimumInstances = objectDefinition->MinimumInstances;
    }

    return MinimumInstances;
}

const AwaResourceDefinition * AwaObjectDefinition_GetResourceDefinition(const AwaObjectDefinition * objectDefinition, AwaResourceID resourceID)
{
    return Definition_LookupResourceDefinitionFromObjectDefinition(objectDefinition, resourceID);
}

AwaResourceID AwaResourceDefinition_GetID(const AwaResourceDefinition * resourceDefinition)
{
    AwaResourceID resourceID = AWA_INVALID_ID;

    if (resourceDefinition != NULL)
    {
        resourceID = resourceDefinition->ResourceID;
    }

    return resourceID;
}

AwaResourceType AwaResourceDefinition_GetType(const AwaResourceDefinition * resourceDefinition)
{
    AwaResourceType resourceType = AwaResourceType_Invalid;

    if (resourceDefinition != NULL)
    {
        if (!IS_MULTIPLE_INSTANCE(resourceDefinition))
        {
            resourceType = resourceDefinition->Type;
        }
        else
        {
            switch (resourceDefinition->Type)
            {
                case AwaResourceType_Opaque:
                    resourceType = AwaResourceType_OpaqueArray;
                    break;
                case AwaResourceType_Integer:
                    resourceType = AwaResourceType_IntegerArray;
                    break;
                case AwaResourceType_Float:
                    resourceType = AwaResourceType_FloatArray;
                    break;
                case AwaResourceType_Boolean:
                    resourceType = AwaResourceType_BooleanArray;
                    break;
               case AwaResourceType_String:
                    resourceType = AwaResourceType_StringArray;
                    break;
               case AwaResourceType_Time:
                    resourceType = AwaResourceType_TimeArray;
                    break;
               case AwaResourceType_ObjectLink:
                    resourceType = AwaResourceType_ObjectLinkArray;
                    break;
               default:
                    LogError("Invalid resource type %d", resourceDefinition->Type);
                    resourceType = AwaResourceType_Invalid;
            }
        }
    }
    return resourceType;
}


const char * AwaResourceDefinition_GetName(const AwaResourceDefinition * resourceDefinition)
{
    const char * name = NULL;

    if (resourceDefinition != NULL)
    {
        name = resourceDefinition->ResourceName;
    }

    return name;
}

int AwaResourceDefinition_GetMinimumInstances(const AwaResourceDefinition * resourceDefinition)
{
    int minimumInstances = -1;

    if (resourceDefinition != NULL)
    {
        minimumInstances = resourceDefinition->MinimumInstances;
    }

    return minimumInstances;
}

int AwaResourceDefinition_GetMaximumInstances(const AwaResourceDefinition * resourceDefinition)
{
    int maximumInstances = -1;

    if (resourceDefinition)
    {
        maximumInstances = resourceDefinition->MaximumInstances;
    }

    return maximumInstances;
}

AwaResourceOperations AwaResourceDefinition_GetSupportedOperations(const AwaResourceDefinition * resourceDefinition)
{
    AwaResourceOperations operations = AwaResourceOperations_None;

    if (resourceDefinition != NULL)
    {
        operations = resourceDefinition->Operation;
    }

    return operations;
}

bool AwaResourceDefinition_IsMandatory(const AwaResourceDefinition * resourceDefinition)
{
    bool mandatory = false;

    if (resourceDefinition != NULL)
    {
        mandatory = IS_MANDATORY(resourceDefinition);
    }

    return mandatory;
}

bool AwaObjectDefinition_IsResourceDefined(const AwaObjectDefinition * objectDefinition, AwaResourceID resourceID)
{
    return (Definition_LookupResourceDefinitionFromObjectDefinition(objectDefinition, resourceID) != NULL);
}

const AwaObjectDefinition * AwaObjectDefinitionIterator_Get(const AwaObjectDefinitionIterator * iterator)
{
     ObjectDefinition * objectDefinition = NULL;

     if (iterator == NULL)
     {
         LogError("AwaObjectDefinitionIterator is NULL");
         return NULL;
     }

     if (iterator->ObjectID != AWA_INVALID_ID)
     {
          objectDefinition = Definition_LookupObjectDefinition(iterator->Definitions, iterator->ObjectID);
     }

     return objectDefinition;
}

bool AwaObjectDefinitionIterator_Next(AwaObjectDefinitionIterator * iterator)
{
    bool result = false;

    if (iterator != NULL)
    {
        iterator->ObjectID = Definition_GetNextObjectType(iterator->Definitions, iterator->ObjectID);
        if (iterator->ObjectID != AWA_INVALID_ID)
        {
            result = true;
        }
    }
    return result;
}

void AwaObjectDefinitionIterator_Free(AwaObjectDefinitionIterator ** iterator)
{
    if ((iterator != NULL) && (*iterator != NULL))
    {
        Awa_MemSafeFree(*iterator);
        *iterator = NULL;
    }
}

AwaResourceDefinitionIterator * AwaObjectDefinition_NewResourceDefinitionIterator(const AwaObjectDefinition * objectDefinition)
{
    AwaResourceDefinitionIterator * iterator = NULL;

    if (objectDefinition != NULL)
    {
        iterator = Awa_MemAlloc(sizeof(*iterator));

        if (iterator != NULL)
        {
            memset(iterator, 0, sizeof(*iterator));
            iterator->ResourceID = AWA_INVALID_ID;
            iterator->Definition = objectDefinition;
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

const AwaResourceDefinition * AwaResourceDefinitionIterator_Get(const AwaResourceDefinitionIterator * iterator)
{
     ResourceDefinition * resourceDefinition = NULL;

     if (iterator == NULL)
     {
         LogError("AwaResourceDefinitionIterator is NULL");
         return NULL;
     }

     if (iterator->ResourceID != AWA_INVALID_ID)
     {
          resourceDefinition = Definition_LookupResourceDefinitionFromObjectDefinition(iterator->Definition, iterator->ResourceID);
     }

     return resourceDefinition;
}

bool AwaResourceDefinitionIterator_Next(AwaResourceDefinitionIterator * iterator)
{
    bool result = false;

    if (iterator)
    {
        iterator->ResourceID = Definition_GetNextResourceTypeFromObjectType(iterator->Definition, iterator->ResourceID);
        if (iterator->ResourceID != AWA_INVALID_ID)
        {
            result = true;
        }
    }

    return result;
}

void AwaResourceDefinitionIterator_Free(AwaResourceDefinitionIterator ** iterator)
{
    if ((iterator != NULL) && (*iterator != NULL))
    {
        Awa_MemSafeFree(*iterator);
        *iterator = NULL;
    }
}

Lwm2mTreeNode * ResourceDefinition_GetDefaultValueNode(const AwaResourceDefinition * definition)
{
    return (definition != NULL) ? definition->DefaultValueNode : NULL;
}


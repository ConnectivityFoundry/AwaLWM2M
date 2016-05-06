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


#ifndef LWM2M_DEFINITION_H
#define LWM2M_DEFINITION_H

#include <stdbool.h>

#include "lwm2m_types.h"
#include "lwm2m_list.h"
#include "lwm2m_tree_node.h"
#include "lwm2m_result.h"

#ifdef __cplusplus
extern "C" {
#endif

// Checks if an ObjectDefinition/ResourceDefinition is multiple instance
#define IS_MULTIPLE_INSTANCE(definition)  (definition->MaximumInstances > 1)

// Checks if an ObjectDefinition/ResourceDefinition is mandatory
#define IS_MANDATORY(definition)  (definition->MinimumInstances >= 1)

// handler to call to retrieve a value from a resource instance
typedef int (*ReadHandler)(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID,
                           ResourceInstanceIDType resourceInstanceID, const void ** buffer, size_t * bufferLen);

typedef int (*GetLengthHandler)(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID,
                                ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID);

// handler to call to write a value to a resource instance
typedef int (*WriteHandler)(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID,
                            ResourceInstanceIDType resourceInstanceID, uint8_t * srcBuffer, size_t srcBufferLen, bool * changed);

// handler to call to execute a resource
typedef int (*ExecuteHandler)(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID,
                              uint8_t * srcBuffer, size_t srcBufferLen);

// handler to call to delete an object, or object instance and all associated resources
typedef int (*DeleteHandler)(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);

// handler to call to create an object instance
typedef int (*CreateInstanceHandler)(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID);

// handler to call to create an optional resource
typedef int (*CreateOptionalResourceHandler)(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);

typedef AwaResult (*LWM2MHandler)(void * context, AwaOperation operation, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID, void ** dataPointer, size_t * dataSize, bool * changed);

typedef struct
{
    WriteHandler Write;
    ReadHandler Read;
    ExecuteHandler Execute;
    //GetLengthHandler GetLength;
    CreateOptionalResourceHandler CreateOptionalResource;
} ResourceOperationHandlers;

typedef struct
{
    DeleteHandler Delete;
    CreateInstanceHandler CreateInstance;
} ObjectOperationHandlers;

struct  _ResourceDefinition
{
    struct ListHead list;
    char * ResourceName;
    ResourceIDType ResourceID;
    AwaResourceType Type;

    //Min/Max names MUST be the same as ObjectDefinition
    uint16_t MaximumInstances;
    uint16_t MinimumInstances;

    AwaResourceOperations Operation;
    ResourceOperationHandlers Handlers;
    LWM2MHandler Handler;

    Lwm2mTreeNode * DefaultValueNode;

    // When using "static" mode, these values are used to
    // determine where to locate the memory for this resource type
    void * DataPointers;
    bool IsPointerArray;  
    size_t DataElementSize;
    size_t DataStepSize;
};

struct _ObjectDefinition
{
    struct ListHead list;
    char * ObjectName;
    ObjectIDType ObjectID;

    //Min/Max names MUST be the same as ResourceDefinition
    uint16_t MaximumInstances;
    uint16_t MinimumInstances;

    struct ListHead Resource;

    ObjectOperationHandlers Handlers;
    LWM2MHandler Handler;
};

typedef struct _ResourceDefinition ResourceDefinition;
typedef struct _ObjectDefinition ObjectDefinition;

typedef struct
{
    struct ListHead ObjectDefinition;
} DefinitionRegistry;

DefinitionRegistry * DefinitionRegistry_Create(void);
int DefinitionRegistry_Destroy(DefinitionRegistry * registry);

ObjectDefinition * Definition_LookupObjectDefinition(const DefinitionRegistry * registry, ObjectIDType objectID);
ResourceDefinition * Definition_LookupResourceDefinition(const DefinitionRegistry * registry, ObjectIDType objectID, ResourceIDType resourceID);
ResourceDefinition * Definition_LookupResourceDefinitionFromObjectDefinition(const ObjectDefinition * objFormat, ResourceIDType resourceID);
ObjectDefinition * Definition_NewObjectType(const char * objName, ObjectIDType objectID, uint16_t maximumInstances,
                                            uint16_t minimumInstances, const ObjectOperationHandlers * handlers);
ObjectDefinition * Definition_NewObjectTypeWithHandler(const char * objName, ObjectIDType objectID, uint16_t minimumInstances,
                                                       uint16_t maximumInstances, LWM2MHandler handler);
int Definition_SetObjectHandler(ObjectDefinition * objFormat, LWM2MHandler handler);
void Definition_FreeObjectType(ObjectDefinition * definition);
int Definition_AddObjectType(DefinitionRegistry * registry, ObjectDefinition * objFormat);
ObjectDefinition * Definition_CopyObjectDefinition(const ObjectDefinition * definition);
int Definition_RegisterObjectType(DefinitionRegistry * registry, const char * objName, ObjectIDType objectID, uint16_t maximumInstances,
                                  uint16_t minimumInstances, ObjectOperationHandlers * handlers);
int Definition_GetNextObjectType(DefinitionRegistry * registry, ObjectIDType objectID);
ResourceDefinition * Definition_NewResourceType(ObjectDefinition * objFormat, const char * resName, ResourceIDType resourceID, AwaResourceType resourceType,
                                                uint16_t maximumInstances, uint16_t minimumInstances, AwaResourceOperations operations,
                                                ResourceOperationHandlers * handlers, Lwm2mTreeNode * defaultValueNode);
ResourceDefinition * Definition_NewResourceTypeWithHandler(ObjectDefinition * objFormat, const char * resName, ResourceIDType resourceID, AwaResourceType resourceType,
                                                           uint16_t minimumInstances, uint16_t maximumInstances,  AwaResourceOperations operations, LWM2MHandler handler);
int Definition_RegisterResourceType(DefinitionRegistry * registry, const char * resName, ObjectIDType objectID, ResourceIDType resourceID, AwaResourceType resourceType,
                                    uint16_t maximumInstances, uint16_t minimumInstances, AwaResourceOperations operations,
                                    ResourceOperationHandlers * handlers, Lwm2mTreeNode * defaultValueNode);
int Definition_GetNextResourceTypeFromObjectType(const ObjectDefinition * objFormat, ResourceIDType resourceID);
int Definition_GetNextResourceType(const DefinitionRegistry * registry, ObjectIDType objectID, ResourceIDType resourceID);
int Definition_GetFormatName(const DefinitionRegistry * registry, ObjectIDType objectID, ResourceIDType resourceID, const char ** name);
int Definition_IsTypeMultiInstance(const DefinitionRegistry * registry, ObjectIDType objectID, ResourceIDType resourceID);
int Definition_IsResourceTypeExecutable(const DefinitionRegistry * registry, ObjectIDType objectID, ResourceIDType resourceID);
int Definition_IsResourceTypeWritable(const DefinitionRegistry * registry, ObjectIDType objectID, ResourceIDType resourceID);
AwaResourceType Definition_GetResourceType(const DefinitionRegistry * registry, ObjectIDType objectID, ResourceIDType resourceID);

int Definition_AllocSensibleDefault(const ResourceDefinition * resourceDefinition, const void ** data, int * dataLen);

#ifdef __cplusplus
}
#endif

#endif // LWM2M_DEFINITION_H

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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lwm2m_definition.h"
#include "lwm2m_list.h"
#include "lwm2m_limits.h"
#include "lwm2m_debug.h"
#include "lwm2m_result.h"
#include "lwm2m_types.h"

ObjectDefinition * Definition_LookupObjectDefinition(const DefinitionRegistry * registry, ObjectIDType objectID)
{
    ObjectDefinition * object = NULL;

    struct ListHead * i;
    ListForEach(i, &registry->ObjectDefinition)
    {
        ObjectDefinition * o = ListEntry(i, ObjectDefinition, list);
        if (o->ObjectID == objectID)
        {
            object = o;
            break;
        }
    }

    return object;
}

ResourceDefinition * Definition_LookupResourceDefinitionFromObjectDefinition(const ObjectDefinition * objFormat, ResourceIDType resourceID)
{
    ResourceDefinition * resource = NULL;
    if (objFormat)
    {
        struct ListHead * i;
        ListForEach(i, &objFormat->Resource)
        {
            ResourceDefinition * r = ListEntry(i, ResourceDefinition, list);
            if (r->ResourceID == resourceID)
            {
                resource = r;
                break;
            }
        }
    }
    return resource;
}

ResourceDefinition * Definition_LookupResourceDefinition(const DefinitionRegistry * registry, ObjectIDType objectID, ResourceIDType resourceID)
{
    ObjectDefinition * objFormat = Definition_LookupObjectDefinition(registry, objectID);
    return Definition_LookupResourceDefinitionFromObjectDefinition(objFormat, resourceID);
}

static ObjectDefinition * NewObjectType(const char * objName, ObjectIDType objectID, uint16_t maximumInstances,
                                        uint16_t minimumInstances, const ObjectOperationHandlers * handlers, LWM2MHandler handler)
{
    ObjectDefinition * objFormat = NULL;
    objFormat = (ObjectDefinition *)malloc(sizeof(ObjectDefinition));

     if (objFormat != NULL)
     {
         memset(objFormat, 0, sizeof(*objFormat));

         objFormat->ObjectName = strdup(objName);
         objFormat->ObjectID = objectID;
         objFormat->MinimumInstances = minimumInstances;
         objFormat->MaximumInstances = maximumInstances;
         objFormat->Handler = handler;

         if (handlers != NULL)
         {
             memcpy(&objFormat->Handlers, handlers, sizeof(*handlers));
         }
         else
         {
             memset(&objFormat->Handlers, 0, sizeof(*handlers));
         }

         ListInit(&objFormat->Resource);
     }
     else
     {
         AwaResult_SetResult(AwaResult_OutOfMemory);
     }

     return objFormat;
}

ObjectDefinition * Definition_NewObjectType(const char * objName, ObjectIDType objectID, uint16_t maximumInstances,
                                            uint16_t minimumInstances, const ObjectOperationHandlers * handlers)
{
     return NewObjectType(objName, objectID, maximumInstances, minimumInstances, handlers, NULL);
}

ObjectDefinition * Definition_NewObjectTypeWithHandler(const char * objName, ObjectIDType objectID, uint16_t minimumInstances,
                                                       uint16_t maximumInstances, LWM2MHandler handler)
{
    return NewObjectType(objName, objectID, maximumInstances, minimumInstances, NULL, handler);
}

int Definition_SetObjectHandler(ObjectDefinition * objectDefinition, LWM2MHandler handler)
{
    int result = -1;

    if (objectDefinition != NULL)
    {
        objectDefinition->Handler = handler;
        result = 0;
    }
    else
    {
        Lwm2m_Error("objectDefinition is NULL\n");
        result = -1;
    }

    return result;
}

int Definition_AddObjectType(DefinitionRegistry * registry, ObjectDefinition * objFormat)
{
    int result = -1;
    ObjectDefinition * existingObjFormat = NULL;

    if ((existingObjFormat = Definition_LookupObjectDefinition(registry, objFormat->ObjectID)))
    {
        if (objFormat->MaximumInstances != existingObjFormat->MaximumInstances)
        {
            AwaResult_SetResult(AwaResult_MismatchedDefinition);
        }
        else if (strlen(existingObjFormat->ObjectName) != strlen(objFormat->ObjectName) ||
                 memcmp(existingObjFormat->ObjectName, objFormat->ObjectName, strlen(existingObjFormat->ObjectName)))
        {
            AwaResult_SetResult(AwaResult_MismatchedDefinition);
        }
        else if (objFormat->MinimumInstances != existingObjFormat->MinimumInstances)
        {
            AwaResult_SetResult(AwaResult_MismatchedDefinition);
        }
        else
        {
            AwaResult_SetResult(AwaResult_AlreadyDefined);
        }
    }
    else
    {
        ListAdd(&objFormat->list, &registry->ObjectDefinition);
        AwaResult_SetResult(AwaResult_Success);
        result = 0;
    }

    return result;
}

ResourceDefinition * Definition_CopyResourceTypeToDefinition(ObjectDefinition * objectDefinition, ResourceDefinition * definition)
{
    ResourceDefinition * newDefinition = NULL;

    if ((newDefinition = Definition_LookupResourceDefinitionFromObjectDefinition(objectDefinition, definition->ResourceID)) != NULL)
    {
        if ((newDefinition->MinimumInstances != definition->MinimumInstances) ||
           (newDefinition->MaximumInstances != definition->MaximumInstances) ||
           (newDefinition->Operation != definition->Operation) ||
           (newDefinition->ResourceID != definition->ResourceID) ||
           (newDefinition->Type != definition->Type) ||
           (strlen(newDefinition->ResourceName) != strlen(definition->ResourceName)) ||
           (memcmp(newDefinition->ResourceName, definition->ResourceName, strlen(newDefinition->ResourceName))) ||
           (Lwm2mTreeNode_CompareRecursive(newDefinition->DefaultValueNode, definition->DefaultValueNode)))
        {
            newDefinition = NULL;
        }
    }
    else
    {
        newDefinition = Definition_NewResourceType(objectDefinition, definition->ResourceName, definition->ResourceID, definition->Type,
                definition->MaximumInstances, definition->MinimumInstances, definition->Operation, &definition->Handlers, definition->DefaultValueNode);
    }
    return newDefinition;
}

ObjectDefinition * Definition_CopyObjectDefinition(const ObjectDefinition * definition)
{
    ObjectDefinition * newDefinition = NULL;
    if (definition != NULL)
    {
        newDefinition = Definition_NewObjectType(definition->ObjectName, definition->ObjectID, definition->MaximumInstances, definition->MinimumInstances, &definition->Handlers);
        if (newDefinition != NULL)
        {
            ResourceIDType resourceID = -1;
            while ((resourceID = Definition_GetNextResourceTypeFromObjectType(definition, resourceID)) != -1)
            {
                ResourceDefinition * resourceDefinition = Definition_LookupResourceDefinitionFromObjectDefinition(definition, resourceID);
                if (resourceDefinition != NULL)
                {
                    if (Definition_CopyResourceTypeToDefinition(newDefinition, resourceDefinition) == NULL)
                    {
                        Definition_FreeObjectType(newDefinition);
                        newDefinition = NULL;
                        break;
                    }
                }
                else
                {
                    Definition_FreeObjectType(newDefinition);
                    newDefinition = NULL;
                    break;
                }
            }
        }
    }
    return newDefinition;
}

int Definition_RegisterObjectType(DefinitionRegistry * registry, const char * objName, ObjectIDType objectID,
                                  uint16_t MaximumInstances, uint16_t MinimumInstances, ObjectOperationHandlers * Handlers)
{
    int result = -1;
    ObjectDefinition * objFormat = NULL;
    if (registry == NULL)
    {
        goto error;
    }

    if ((objFormat = Definition_NewObjectType(objName, objectID, MaximumInstances, MinimumInstances, Handlers)) == NULL)
    {
        goto error;
    }

    if (Definition_AddObjectType(registry, objFormat) != 0)
    {
        Definition_FreeObjectType(objFormat);
        goto error;
    }
    result = 0;
error:
    return result;
}

int Definition_GetNextObjectType(DefinitionRegistry * registry, ObjectIDType objectID)
{
    int nextObjectID = -1;
    if (registry != NULL)
    {
        bool found = (objectID == -1) ? true : false;
        struct ListHead * i;
        ListForEach(i, &registry->ObjectDefinition)
        {
            ObjectDefinition * objFormat = ListEntry(i, ObjectDefinition, list);
            if (found)
            {
                nextObjectID = objFormat->ObjectID;
                goto done;
            }

            if (objFormat->ObjectID == objectID)
            {
                found = true;
            }
        }
    }
done:
    return nextObjectID;
}

AwaResourceType Definition_GetResourceType(const DefinitionRegistry * registry, ObjectIDType objectID, ResourceIDType resourceID)
{
    AwaResourceType resourceType = AwaResourceType_Invalid;
    AwaResult_SetResult(AwaResult_NotFound);
    ResourceDefinition * resFormat = Definition_LookupResourceDefinition(registry, objectID, resourceID);
    if (resFormat != NULL)
    {
        AwaResult_SetResult(AwaResult_Success);
        resourceType = resFormat->Type;
    }
    return resourceType;
}

int Definition_IsResourceTypeExecutable(const DefinitionRegistry * registry, ObjectIDType objectID, ResourceIDType resourceID)
{
    ResourceDefinition * resFormat = Definition_LookupResourceDefinition(registry, objectID, resourceID);
    return (resFormat != NULL) ? Operations_IsResourceTypeExecutable(resFormat->Operation) : -1;
}

int Definition_IsResourceTypeWritable(const DefinitionRegistry * registry, ObjectIDType objectID, ResourceIDType resourceID)
{
    ResourceDefinition * resFormat = Definition_LookupResourceDefinition(registry, objectID, resourceID);
    return (resFormat != NULL) ? Operations_IsResourceTypeWritable(resFormat->Operation) : -1;
}

int Definition_IsTypeMultiInstance(const DefinitionRegistry * registry, ObjectIDType objectID, ResourceIDType resourceID)
{
    int isMultipleInstance = -1;
    AwaResult_SetResult(AwaResult_NotFound);

    if (resourceID == -1)
    {
        ObjectDefinition * objFormat = Definition_LookupObjectDefinition(registry, objectID);
        if (objFormat != NULL)
        {
            AwaResult_SetResult(AwaResult_Success);
            isMultipleInstance = IS_MULTIPLE_INSTANCE(objFormat);
        }
    }
    else
    {
        ResourceDefinition * resFormat = Definition_LookupResourceDefinition(registry, objectID, resourceID);
        if (resFormat != NULL)
        {
            AwaResult_SetResult(AwaResult_Success);
            isMultipleInstance = IS_MULTIPLE_INSTANCE(resFormat);
        }
    }

    return isMultipleInstance;
}

ResourceDefinition * NewResourceType(ObjectDefinition * objFormat, const char * resName, ResourceIDType resourceID,
                                                AwaResourceType resourceType, uint16_t maximumInstances, uint16_t minimumInstances,
                                                AwaResourceOperations operations, ResourceOperationHandlers * handlers, LWM2MHandler handler, Lwm2mTreeNode * defaultValueNode)
{
    ResourceDefinition * resFormat = (ResourceDefinition *)malloc(sizeof(*resFormat));
    if (resFormat != NULL)
    {
        memset(resFormat, 0, sizeof(*resFormat));
        resFormat->ResourceName = strdup(resName);
        resFormat->ResourceID = resourceID;
        resFormat->Operation = operations;
        resFormat->Type = (AwaResourceType)resourceType;
        resFormat->MaximumInstances = maximumInstances;
        resFormat->MinimumInstances = minimumInstances;
        resFormat->DefaultValueNode = (defaultValueNode != NULL) ? Lwm2mTreeNode_CopyRecursive(defaultValueNode) : NULL;
        resFormat->Handler = handler;

        if (handlers != NULL)
        {
            memcpy(&resFormat->Handlers, handlers, sizeof(resFormat->Handlers));
        }
        else
        {
            memset(&resFormat->Handlers, 0, sizeof(resFormat->Handlers));
        }

        ListAdd(&resFormat->list, &objFormat->Resource);
    }
    return resFormat;
}

ResourceDefinition * Definition_NewResourceType(ObjectDefinition * objFormat, const char * resName, ResourceIDType resourceID,
                                                AwaResourceType resourceType, uint16_t maximumInstances, uint16_t minimumInstances,
                                                AwaResourceOperations operations, ResourceOperationHandlers * handlers, Lwm2mTreeNode * defaultValueNode)
{
    return NewResourceType(objFormat, resName, resourceID, resourceType, maximumInstances, minimumInstances, operations, handlers, NULL, defaultValueNode);
}

ResourceDefinition * Definition_NewResourceTypeWithHandler(ObjectDefinition * objFormat, const char * resName, ResourceIDType resourceID, AwaResourceType resourceType,
                                                           uint16_t MinimumInstances, uint16_t MaximumInstances, AwaResourceOperations operations, LWM2MHandler Handler)
{
    return NewResourceType(objFormat, resName, resourceID, resourceType, MaximumInstances, MinimumInstances, operations, NULL, Handler, NULL);
}

int Definition_RegisterResourceType(DefinitionRegistry * registry, const char * resName, ObjectIDType objectID, ResourceIDType resourceID, AwaResourceType resourceType,
                                    uint16_t maximumInstances, uint16_t minimumInstances, AwaResourceOperations operations, ResourceOperationHandlers * handlers, Lwm2mTreeNode * defaultValueNode)
{
    int result = -1;

    if (registry == NULL)
    {
        goto error;
    }

    if ((resourceID < 0) || (resourceID > LWM2M_LIMITS_MAX_RESOURCE_ID))
    {
        Lwm2m_Error("resource ID out of range\n");
        AwaResult_SetResult(AwaResult_BadRequest);
        goto error;
    }

    if (Operations_IsResourceTypeExecutable(operations) && resourceType != AwaResourceType_None)
    {
        AwaResult_SetResult(AwaResult_BadRequest);
        goto error;
    }

    if (Operations_IsResourceTypeExecutable(operations) && maximumInstances > 1)
    {
        AwaResult_SetResult(AwaResult_BadRequest);
        goto error;
    }

    if (Definition_LookupResourceDefinition(registry, objectID, resourceID))
    {
        AwaResult_SetResult(AwaResult_AlreadyDefined);
        goto error;
    }

    ObjectDefinition * objFormat;
    if (!(objFormat = Definition_LookupObjectDefinition(registry, objectID)))
    {
        Lwm2m_Error("Failed to look up object format\n");
        AwaResult_SetResult(AwaResult_NotFound);
        goto error;
    }

    if (Definition_NewResourceType(objFormat, resName, resourceID, resourceType, maximumInstances, minimumInstances, operations, handlers, defaultValueNode) == NULL)
    {
        Lwm2m_Error("Unable to allocate memory\n");
        AwaResult_SetResult(AwaResult_OutOfMemory);
        goto error;
    }

    result = 0;
    AwaResult_SetResult(AwaResult_Success);

error:
    return result;
}

int Definition_GetNextResourceTypeFromObjectType(const ObjectDefinition * objFormat, ResourceIDType resourceID)
{
    int nextResourceID = -1;
    if (objFormat != NULL)
    {
        bool found = (resourceID == -1) ? true : false;
        struct ListHead * i;
        ListForEach(i, &objFormat->Resource)
        {
            ResourceDefinition * resFormat = ListEntry(i, ResourceDefinition, list);
            if (found)
            {
                nextResourceID = resFormat->ResourceID;
                goto done;
            }

            if (resFormat->ResourceID == resourceID)
            {
                found = true;
            }
        }
    }
done:
    return nextResourceID;
}

int Definition_GetNextResourceType(const DefinitionRegistry * registry, ObjectIDType objectID, ResourceIDType resourceID)
{
    ObjectDefinition * objFormat = Definition_LookupObjectDefinition(registry, objectID);
    return Definition_GetNextResourceTypeFromObjectType(objFormat, resourceID);
}

int Definition_GetFormatName(const DefinitionRegistry * registry, ObjectIDType objectID, ResourceIDType resourceID, const char ** name)
{
    int result = -1;
    if (resourceID == -1)
    {
        ObjectDefinition * objFormat = Definition_LookupObjectDefinition(registry, objectID);
        if (objFormat != NULL)
        {
            *name = objFormat->ObjectName;
            if (*name != NULL)
            {
                result = strlen(*name);
            }
        }
    }
    else
    {
        ResourceDefinition * resFormat = Definition_LookupResourceDefinition(registry, objectID, resourceID);
        if (resFormat != NULL)
        {
            *name = resFormat->ResourceName;
            if (*name != NULL)
            {
                result = strlen(*name);
            }
        }
    }
    return result;
}

static void DestroyResourceFormatList(struct ListHead * resourceDefinition)
{
    if (resourceDefinition != NULL)
    {
        struct ListHead * i, * n;
        ListForEachSafe(i, n, resourceDefinition)
        {
            ResourceDefinition * definition = ListEntry(i, ResourceDefinition, list);
            if (definition != NULL)
            {
                if (definition->DefaultValueNode != NULL)
                {
                    Lwm2mTreeNode_DeleteRecursive(definition->DefaultValueNode);
                }
                free(definition->ResourceName);
                free(definition);
            }
        }
    }
}

void Definition_FreeObjectType(ObjectDefinition * definition)
{
    DestroyResourceFormatList(&definition->Resource);
    free(definition->ObjectName);
    free(definition);
}

static void DestroyObjectFormatList(struct ListHead * objectDefinition)
{
    if (objectDefinition != NULL)
    {
        struct ListHead * i, * n;
        ListForEachSafe(i, n, objectDefinition)
        {
            ObjectDefinition * definition = ListEntry(i, ObjectDefinition, list);
            if (definition != NULL)
            {
                Definition_FreeObjectType(definition);
            }
        }
    }
}

DefinitionRegistry * DefinitionRegistry_Create(void)
{
    DefinitionRegistry * registry = malloc(sizeof(DefinitionRegistry));
    if (registry != NULL)
    {
        ListInit(&registry->ObjectDefinition);
    }
    return registry;
}

int DefinitionRegistry_Destroy(DefinitionRegistry * registry)
{
    int result = -1;
    if (registry != NULL)
    {
        DestroyObjectFormatList(&registry->ObjectDefinition);
        free(registry);
        result = 0;
    }
    return result;
}

int Definition_AllocSensibleDefault(const ResourceDefinition * resourceDefinition, const void ** data, int * dataLen)
{
    static const char * defaultString = "";
    static const int64_t defaultInt = 0;
    static const float defaultFloat = 0.0f;
    static const bool defaultBool = false;
    static const AwaObjectLink defaultObjectLink = {0, 0};

    int result = 0;
    if (resourceDefinition != NULL)
    {
        if (data != NULL)
        {
            *data = NULL;
            if (dataLen != NULL)
            {
                *dataLen = 0;
                switch (resourceDefinition->Type)
                {
                    case AwaResourceType_Opaque:     // no break
                    case AwaResourceType_None:
                        break;
                    case AwaResourceType_Integer:
                        *dataLen = sizeof(defaultInt);
                        *data = &defaultInt;
                        break;
                    case AwaResourceType_Float:
                        *dataLen = sizeof(defaultFloat);
                        *data = &defaultFloat;
                        break;
                    case AwaResourceType_Boolean:
                        *dataLen = sizeof(defaultBool);
                        *data = &defaultBool;
                        break;
                    case AwaResourceType_String:
                        *data = defaultString;
                        *dataLen = strlen(*data) + 1;
                        break;
                    case AwaResourceType_Time:
                        *dataLen = sizeof(defaultInt);
                        *data = &defaultInt;
                        break;
                    case AwaResourceType_ObjectLink:
                        *dataLen = sizeof(defaultObjectLink);
                        *data = &defaultObjectLink;
                        break;
                    case AwaResourceType_Invalid:  // no break
                    default:
                        Lwm2m_Error("Invalid resource type %d\n", resourceDefinition->Type);
                        break;
                }
            }
        }
    }
    return result;
}


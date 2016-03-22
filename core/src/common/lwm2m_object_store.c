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


#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "lwm2m_types.h"
#include "lwm2m_limits.h"
#include "lwm2m_object_store.h"
#include "lwm2m_list.h"
#include "lwm2m_debug.h"
#include "lwm2m_util.h"
#include "lwm2m_result.h"

typedef struct
{
    struct ListHead list;             // prev/next pointers

    void * Value;
    int Size;
    int ID;
} ResourceInstance;

typedef struct
{
    struct ListHead list;             // prev/next pointers
    struct ListHead Instance;
    int ID;
} Resource;

typedef struct
{
    struct ListHead list;              // prev/next pointers
    struct ListHead Resource;
    int ID;                            // Instance ID
} ObjectInstance;

typedef struct
{
    struct ListHead list;              // prev/next pointers
    struct ListHead Instance;          // Linked list of object instances
    int ID;
} Object;

static Object * LookupObject(const ObjectStore * store, ObjectIDType objectID)
{
    struct ListHead * i;
    ListForEach(i, &store->objectList)
    {
        Object * object = ListEntry(i, Object, list);
        if (object->ID == objectID)
        {
            return object;
        }
    }
    return NULL;
}

static ObjectInstance * GetObjectInstance(Object * object, ObjectInstanceIDType objectInstanceID)
{
    struct ListHead * i;
    ListForEach(i, &object->Instance)
    {
        ObjectInstance * instance = ListEntry(i, ObjectInstance, list);
        if (instance->ID == objectInstanceID)
        {
            return instance;
        }
    }

    return NULL;
}

static ObjectInstance * LookupObjectInstance(const ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID)
{
    Object * object = LookupObject(store, objectID);
    if (object != NULL)
    {
        return GetObjectInstance(object, objectInstanceID);
    }
    return NULL;
}

// Retrieve a pointer to a Resource from an Object instance
static Resource * GetResource(ObjectInstance * instance, ResourceIDType resourceID)
{
    struct ListHead * i;
    ListForEach(i, &instance->Resource)
    {
        Resource * resource = ListEntry(i, Resource, list);
        if (resource->ID == resourceID)
        {
            return resource;
        }
    }
    return NULL;
}

static Resource * LookupResource(const ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    ObjectInstance * instance = LookupObjectInstance(store, objectID, objectInstanceID);
    if (instance != NULL)
    {
        return GetResource(instance, resourceID);
    }
    return NULL;
}

static Resource * CreateResource(const ObjectStore * store, ObjectInstance * instance, ObjectIDType objectID, ResourceIDType resourceID)
{
    Resource * resource = GetResource(instance, resourceID);
    if (resource)
    {
        // already exists
        AwaResult_SetResult(AwaResult_Success);
        return resource;
    }

    // allocate memory for new resource
    resource = (Resource*)malloc(sizeof(Resource));
    if (resource == NULL)
    {
        AwaResult_SetResult(AwaResult_OutOfMemory);
        return NULL;
    }

    resource->ID = resourceID;
    ListInit(&resource->Instance);

    // Add to instance.
    ListAdd(&resource->list, &instance->Resource);

    AwaResult_SetResult(AwaResult_Success);
    return resource;
}

static ResourceInstance * GetResourceInstance(Resource * resource, ResourceInstanceIDType resourceInstanceID)
{
    int count = 0;
    struct ListHead * i;
    ListForEach(i, &resource->Instance)
    {
        ResourceInstance * resource = ListEntry(i, ResourceInstance, list);
        if (resource->ID == resourceInstanceID)
        {
            return resource;
        }
        count++;
    }
    return NULL;
}

ResourceIDType ObjectStore_GetNextResourceID(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    ObjectInstance * instance = LookupObjectInstance(store, objectID, objectInstanceID);
    if (instance != NULL)
    {
        bool found = (resourceID == -1) ? true : false;
        struct ListHead * i;
        ListForEach(i, &instance->Resource)
        {
            Resource * resource = ListEntry(i, Resource, list);
            if (found)
            {
                AwaResult_SetResult(AwaResult_Success);
                return resource->ID;
            }

            if (resource->ID == resourceID)
            {
                found = true;
            }
        }
    }
    AwaResult_SetResult(AwaResult_NotFound);
    return -1;
}

Object * CreateObject(ObjectStore * store, ObjectIDType objectID)
{
    Object * object = LookupObject(store, objectID);
    if (object != NULL)
    {
        //AwaResult_SetResult(AwaResult_AlreadyCreated);
        AwaResult_SetResult(AwaResult_Success);
        return object;
    }

    object = (Object*)malloc(sizeof(Object));
    if (object == NULL)
    {
        AwaResult_SetResult(AwaResult_OutOfMemory);
        return NULL;
    }

    object->ID = objectID;
    ListInit(&object->Instance);

    ListAdd(&object->list, &store->objectList);

    AwaResult_SetResult(AwaResult_Success);
    return object;
}

static ObjectInstance * CreateObjectInstance(ObjectStore * store, Object * object, ObjectInstanceIDType objectInstanceID)
{
    ObjectInstance * instance = GetObjectInstance(object, objectInstanceID);
    if (instance != NULL)
    {
        AwaResult_SetResult(AwaResult_AlreadyCreated);
        return NULL;
    }

    // Create a new instance
    instance = (ObjectInstance*)malloc(sizeof(ObjectInstance));
    if (instance == NULL)
    {
        AwaResult_SetResult(AwaResult_OutOfMemory);
        return NULL;
    }

    instance->ID = objectInstanceID;
    ListInit(&instance->Resource);

    // Add instance to object
    ListAdd(&instance->list, &object->Instance);

    Lwm2m_Debug("CreateObjectInstance %d %d\n", object->ID, objectInstanceID);

    AwaResult_SetResult(AwaResult_Success);
    return instance;
}

static int DeleteResource(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    Resource * resource = LookupResource(store, objectID, objectInstanceID, resourceID);

    if (resource == NULL)
    {
        return -1;
    }

    struct ListHead * pos, *n;
    ListForEachSafe(pos, n, &resource->Instance)
    {
        ResourceInstance * rInst = ListEntry(pos, ResourceInstance, list);
        free(rInst->Value);
        ListRemove(pos);
        free(rInst);
    }
    ListRemove(&resource->list);
    free(resource);

    return 0;
}

static int DeleteInstance(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID)
{
    ObjectInstance * instance = LookupObjectInstance(store, objectID, objectInstanceID);

    if (instance == NULL)
    {
        return -1;
    }

    struct ListHead * pos, *n;
    ListForEachSafe(pos, n, &instance->Resource)
    {
        Resource * resource = ListEntry(pos, Resource, list);
        DeleteResource(store, objectID, objectInstanceID, resource->ID);
    }
    ListRemove(&instance->list);
    free(instance);

    return 0;
}

static int DeleteObjectInstances(ObjectStore * store, ObjectIDType objectID)
{
    Object * object = LookupObject(store, objectID);

    if (object == NULL)
    {
        return -1;
    }

    struct ListHead * pos, *n;
    ListForEachSafe(pos, n, &object->Instance)
    {
        ObjectInstance * instance = ListEntry(pos, ObjectInstance, list);
        DeleteInstance(store, objectID, instance->ID);
    }
    return 0;
}

int ObjectStore_GetResourceInstanceLength(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID,
        ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID)
{
    int result = -1;
    Resource * resource = LookupResource(store, objectID, objectInstanceID, resourceID);
    if (resource != NULL)
    {
        ResourceInstance * instance = GetResourceInstance(resource, resourceInstanceID);
        if (instance != NULL)
        {
            result = instance->Size;
        }
    }
    return result;
}

int ObjectStore_GetResourceInstanceValue(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID,
                                         ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID, const void ** ValueBuffer, size_t * ValueBufferSize)
{
    Resource * resource = LookupResource(store, objectID, objectInstanceID, resourceID);
    if (resource != NULL)
    {
        ResourceInstance * instance = GetResourceInstance(resource, resourceInstanceID);
        if (instance == NULL)
        {
            AwaResult_SetResult(AwaResult_NotFound);
            return -1;
        }
        else if (ValueBuffer == NULL || ValueBufferSize == NULL)
        {
            Lwm2m_Error("ValueBuffer or ValueBufferSize is NULL\n");
            AwaResult_SetResult(AwaResult_InternalError);
            return -1;
        }

        *ValueBuffer = instance->Value;
        *ValueBufferSize = instance->Size;
        AwaResult_SetResult(AwaResult_Success);
        return instance->Size;
    }
    AwaResult_SetResult(AwaResult_NotFound);
    return -1;
}

int ObjectStore_SetResourceInstanceValue(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID,
                                         ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID, int valueSize,
                                         const void * valueBuffer, int valueBufferPos, int valueBufferLen, bool * changed)
{
    Object * obj;
    ObjectInstance * inst;
    Resource * r;
    ResourceInstance * rInst;

    *changed = false;

    obj = LookupObject(store, objectID);
    if (obj == NULL)
    {
        Lwm2m_Error("Failed to lookup object %d\n", objectID);
        return -1;
    }

    inst = LookupObjectInstance(store, objectID, objectInstanceID);
    if (inst == NULL)
    {
        Lwm2m_Error("Failed to lookup object %d instance %d\n", objectID, objectInstanceID);
        return -1;
    }

    r = LookupResource(store, objectID, objectInstanceID, resourceID);
    if (r == NULL)
    {
        Lwm2m_Error("Failed to lookup object %d instance %d resource %d\n", objectID, objectInstanceID, resourceID);
        return -1;
    }

    // create a new resource instance, or resize the existing one.
    rInst = GetResourceInstance(r, resourceInstanceID);
    if (rInst == NULL)
    {
        rInst = (ResourceInstance*)malloc(sizeof(ResourceInstance));
        if (rInst == NULL)
        {
            Lwm2m_Error("Failed to allocate memory\n");
            AwaResult_SetResult(AwaResult_OutOfMemory);
            return -1;
        }

        rInst->ID = resourceInstanceID;

        rInst->Value = malloc(valueSize);

        if (rInst->Value == NULL)
        {
            free(rInst);
            Lwm2m_Error("Failed to allocate memory\n");
            AwaResult_SetResult(AwaResult_OutOfMemory);
            return -1;
        }

        memset(rInst->Value, 0, valueSize);

        rInst->Size = valueSize;

        ListAdd(&rInst->list, &r->Instance);
    }
    else
    {
        // re-alloc memory if the size has changed.
        if (rInst->Size != valueSize)
        {
            void * temp = realloc(rInst->Value, valueSize);
            if (temp == NULL)
            {
                Lwm2m_Error("Failed to realloc memory\n");
                AwaResult_SetResult(AwaResult_OutOfMemory);
                return -1;
            }

            rInst->Value = temp;
            rInst->Size = valueSize;

            memset(rInst->Value, 0, valueSize);
        }
    }

    if ((valueBufferPos < valueSize && valueBufferPos >= 0) || (valueBufferPos == valueSize && valueSize == 0/*Allow empty opaque data*/))
    {
        if (memcmp((char*)(rInst->Value) + valueBufferPos, valueBuffer, valueBufferLen))
        {
            memcpy((char*)(rInst->Value) + valueBufferPos, valueBuffer, valueBufferLen);
            *changed = true;
        }

        AwaResult_SetResult(AwaResult_Success);

        return valueBufferLen;
    }

    AwaResult_SetResult(AwaResult_BadRequest);
    return -1;
}

ObjectStore * ObjectStore_Create(void)
{
    ObjectStore * store = (ObjectStore*)malloc(sizeof(ObjectStore));
    if (store == NULL)
    {
        AwaResult_SetResult(AwaResult_OutOfMemory);
        return NULL;
    }

    memset(store, 0, sizeof(ObjectStore));

    ListInit(&store->objectList);

    AwaResult_SetResult(AwaResult_Success);
    return store;
}

static void DestroyResourceInstanceList(struct ListHead * resourceInstanceList)
{
    if (resourceInstanceList != NULL)
    {
        struct ListHead * i, * n;
        ListForEachSafe(i, n, resourceInstanceList)
        {
            ResourceInstance * resourceInstance = ListEntry(i, ResourceInstance, list);
            if (resourceInstance != NULL)
            {
                free(resourceInstance->Value);
                free(resourceInstance);
            }
        }
    }
}

static void DestroyResourceList(struct ListHead * resourceList)
{
    if (resourceList != NULL)
    {
        struct ListHead * i, * n;
        ListForEachSafe(i, n, resourceList)
        {
            Resource * resource = ListEntry(i, Resource, list);
            if (resource != NULL)
            {
                DestroyResourceInstanceList(&resource->Instance);
                free(resource);
            }
        }
    }
}

static void DestroyInstanceList(struct ListHead * instanceList)
{
    if (instanceList != NULL)
    {
        struct ListHead * i, * n;
        ListForEachSafe(i, n, instanceList)
        {
            ObjectInstance * objectInstance = ListEntry(i, ObjectInstance, list);
            if (objectInstance != NULL)
            {
                DestroyResourceList(&objectInstance->Resource);
                free(objectInstance);
            }
        }
    }
}

static void DestroyObjectList(struct ListHead * objectList)
{
    if (objectList != NULL)
    {
        struct ListHead * i, * n;
        ListForEachSafe(i, n, objectList)
        {
            Object * object = ListEntry(i, Object, list);
            if (object != NULL)
            {
                DestroyInstanceList(&object->Instance);
                free(object);
            }
        }
    }
}

void ObjectStore_Destroy(ObjectStore * store)
{
    if (store != NULL)
    {
        // loop through all objects and free them
        DestroyObjectList(&store->objectList);
        free(store);
    }
}

int ObjectStore_GetObjectNumInstances(ObjectStore * store, ObjectIDType objectID)
{
    Object * object = LookupObject(store, objectID);
    if (object != NULL)
    {
        return ListCount(&object->Instance);
    }
    return 0;
}

int ObjectStore_GetInstanceNumResources(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID)
{
    ObjectInstance * instance = LookupObjectInstance(store, objectID, objectInstanceID);
    if (instance != NULL)
    {
        return ListCount(&instance->Resource);
    }
    return 0;
}

int ObjectStore_GetResourceNumInstances(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    Resource * resource = LookupResource(store, objectID, objectInstanceID, resourceID);
    if (resource != NULL)
    {
        return ListCount(&resource->Instance);
    }
    return 0;
}

bool ObjectStore_Exists(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    if (objectID == -1)
    {
        return false;
    }
    else if (objectInstanceID == -1)
    {
        return LookupObject(store, objectID) != NULL;
    }
    else if (resourceID == -1)
    {
        return LookupObjectInstance(store, objectID, objectInstanceID) != NULL;
    }
    else
    {
        return LookupResource(store, objectID, objectInstanceID, resourceID) != NULL;
    }
}

ObjectInstanceIDType ObjectStore_GetNextObjectInstanceID(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID)
{
    Object * object = LookupObject(store, objectID);
    if (object != NULL)
    {
        bool found = (objectInstanceID == -1);
        struct ListHead * i;
        ListForEach(i, &object->Instance)
        {
            ObjectInstance * instance = ListEntry(i, ObjectInstance, list);
            if (found)
            {
                AwaResult_SetResult(AwaResult_Success);
                return instance->ID;
            }

            if (instance->ID == objectInstanceID)
            {
                found = true;
            }
        }
    }
    AwaResult_SetResult(AwaResult_NotFound);
    return -1;
}

ResourceInstanceIDType ObjectStore_GetNextResourceInstanceID(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID,
                                                             ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID)
{
    Resource * resource = LookupResource(store, objectID, objectInstanceID, resourceID);
    if (resource != NULL)
    {
        bool found = (resourceInstanceID == -1);
        struct ListHead * i;
        ListForEach(i, &resource->Instance)
        {
            ResourceInstance * instance = ListEntry(i, ResourceInstance, list);
            if (found)
            {
                AwaResult_SetResult(AwaResult_Success);
                return instance->ID;
            }

            if (instance->ID == resourceInstanceID)
            {
                found = true;
            }
        }
    }
    AwaResult_SetResult(AwaResult_NotFound);
    return -1;
}

int ObjectStore_Delete(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    if (resourceID != -1)
    {
        return DeleteResource(store, objectID, objectInstanceID, resourceID);
    }
    else if (objectInstanceID != -1)
    {
        return DeleteInstance(store, objectID, objectInstanceID);
    }
    else if (objectID != -1)
    {
        // If we are deleting an object, remove all instances but keep the object and any subscribers etc
        return DeleteObjectInstances(store, objectID);
    }
    return -1;
}

ResourceIDType ObjectStore_CreateResource(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    ObjectInstance * instance = LookupObjectInstance(store, objectID, objectInstanceID);
    if (instance == NULL)
    {
        Lwm2m_Error("Failed to lookup object instance\n");
        AwaResult_SetResult(AwaResult_NotFound);
        goto error;
    }
    Resource * resource = CreateResource(store, instance, objectID, resourceID);
    if (resource != NULL)
    {
        Lwm2m_Debug("Created new resource ID: %d for object %d instance %d\n", resource->ID, objectID, objectInstanceID);
        return resource->ID;
    }
error:
    return -1;
}

ObjectInstanceIDType ObjectStore_CreateObjectInstance(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, int maxInstances)
{
    AwaResult result = AwaResult_Unspecified;

    Object * obj = CreateObject(store, objectID);
    if (obj == NULL)
    {
        Lwm2m_Error("Failed to create object\n");
        result = AwaResult_GetLastResult();
        goto error;
    }

    if (ObjectStore_GetObjectNumInstances(store, objectID) + 1 > maxInstances)
    {
        Lwm2m_Error("Cannot create object instance: object %d already contains a maximum number of instances\n", objectID);
        result = AwaResult_MethodNotAllowed;
        goto error;
    }

    // Instance number not specified, so find the next available one
    if (objectInstanceID == -1)
    {
        do
        {
            objectInstanceID++;
        }
        while (ObjectStore_Exists(store, objectID, objectInstanceID, -1));

        Lwm2m_Debug("Object instance ID not given, generated %d\n", objectInstanceID);
    }
    else if (ObjectStore_Exists(store, objectID, objectInstanceID, -1))
    {
        Lwm2m_Error("Object instance already exists %d\n", objectID);
        result = AwaResult_MethodNotAllowed;
        goto error;
    }

    // Create the new object instance
    ObjectInstance * instance = CreateObjectInstance(store, obj, objectInstanceID);
    if (instance == NULL)
    {
        Lwm2m_Error("Failed to create object instance\n");
        result = AwaResult_OutOfMemory;
        goto error;
    }
    else
    {
        Lwm2m_Debug("Created new object instance ID: %d for object %d\n", objectInstanceID, objectID);
        result = AwaResult_Success;
        goto done;
    }

error:
    objectInstanceID = -1;
done:
    AwaResult_SetResult(result);
    return objectInstanceID;
}

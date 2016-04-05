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


#ifndef LWM2M_OBJECT_STORE_H
#define LWM2M_OBJECT_STORE_H


#include <stdbool.h>

#include "lwm2m_types.h"
#include "lwm2m_list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    struct ListHead objectList;
} ObjectStore;

ObjectStore * ObjectStore_Create(void);

int ObjectStore_GetResourceInstanceLength(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID,
                                          ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID);
int ObjectStore_GetResourceInstanceValue(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID,
                                         ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID, const void ** ValueBuffer, size_t * ValueBufferSize);
int ObjectStore_SetResourceInstanceValue(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID,
                                         ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID, int valueSize,
                                         const void * valueBuffer, int valueBufferPos, int valueBufferLen, bool * changed);

int ObjectStore_GetResourceNumInstances(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);
bool ObjectStore_Exists(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);
int ObjectStore_GetInstanceNumResources(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID);
int ObjectStore_GetObjectNumInstances(ObjectStore * store, ObjectIDType objectID);
ObjectInstanceIDType ObjectStore_CreateObjectInstance(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, int maxInstances);
ResourceIDType ObjectStore_CreateResource(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);

int ObjectStore_Delete(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);
ResourceIDType ObjectStore_GetNextResourceID(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);
ObjectInstanceIDType ObjectStore_GetNextObjectInstanceID(ObjectStore * store, ObjectIDType  objectID, ObjectInstanceIDType objectInstanceID);
ResourceInstanceIDType ObjectStore_GetNextResourceInstanceID(ObjectStore * store, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID,
                                                             ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID);

ObjectStore * ObjectStore_Create();
void ObjectStore_Destroy(ObjectStore * store);

#ifdef __cplusplus
}
#endif

#endif // LWM2M_OBJECT_STORE_H


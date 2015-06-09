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


#ifndef LWM2M_OBJECT_TREE_H
#define LWM2M_OBJECT_TREE_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>

#include "lwm2m_tree_node.h"
#include "lwm2m_types.h"


typedef struct
{
    Lwm2mTreeNode * RootNode;

} Lwm2mObjectTree;

typedef struct
{
    ObjectIDType ObjectID;
    ObjectInstanceIDType ObjectInstanceID;
    ResourceIDType ResourceID;
    ResourceInstanceIDType ResourceInstanceID;

} Lwm2mObjectTreeIterator;


int Lwm2mObjectTree_Init(Lwm2mObjectTree * objectTree);
int Lwm2mObjectTree_Destroy(Lwm2mObjectTree * objectTree);

int Lwm2mObjectTree_AddObject(Lwm2mObjectTree * objectTree, ObjectIDType objectID);
int Lwm2mObjectTree_AddObjectInstance(Lwm2mObjectTree * objectTree, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID);
int Lwm2mObjectTree_AddResource(Lwm2mObjectTree * objectTree, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);
int Lwm2mObjectTree_AddResourceInstance(Lwm2mObjectTree * objectTree, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID);

int Lwm2mObjectTree_DeleteObject(Lwm2mObjectTree * objectTree, ObjectIDType objectID);
int Lwm2mObjectTree_DeleteObjectInstance(Lwm2mObjectTree * objectTree, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID);
int Lwm2mObjectTree_DeleteResource(Lwm2mObjectTree * objectTree, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);
int Lwm2mObjectTree_DeleteResourceInstance(Lwm2mObjectTree * objectTree, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID);

int Lwm2mObjectTree_InitIterator(Lwm2mObjectTreeIterator * iterator, Lwm2mObjectTree * objectTree, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID);
int Lwm2mObjectTree_GetNextObjectID(Lwm2mObjectTree * objectTree, Lwm2mObjectTreeIterator * iterator);
int Lwm2mObjectTree_GetNextObjectInstanceID(Lwm2mObjectTree * objectTree, Lwm2mObjectTreeIterator * iterator);
int Lwm2mObjectTree_GetNextResourceID(Lwm2mObjectTree * objectTree, Lwm2mObjectTreeIterator * iterator);
int Lwm2mObjectTree_GetNextResourceInstanceID(Lwm2mObjectTree * objectTree, Lwm2mObjectTreeIterator * iterator);

int Lwm2mObjectTree_GetNumObjectInstances(Lwm2mObjectTree * objectTree, ObjectIDType ObjectID);

bool Lwm2mObjectTree_Exists(Lwm2mObjectTree * objectTree, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID);


#ifdef __cplusplus
}
#endif

#endif // LWM2M_OBJECT_TREE_H

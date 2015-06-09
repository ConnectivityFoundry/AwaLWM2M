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


#include <stdlib.h>

#include "lwm2m_object_tree.h"
#include "lwm2m_debug.h"


static Lwm2mTreeNode * Lwm2mTree_LookupNodeFromID(Lwm2mTreeNode * parentNode, uint16_t id)
{
    if (parentNode == NULL)
    {
        return NULL;
    }

    Lwm2mTreeNode * objectNode = Lwm2mTreeNode_GetFirstChild(parentNode);
    while (objectNode != NULL)
    {
        int nodeId;
        Lwm2mTreeNode_GetID(objectNode, &nodeId);

        if (nodeId == id)
        {
            return objectNode;
        }

        objectNode = Lwm2mTreeNode_GetNextChild(parentNode, objectNode);
    }
    return NULL;
}

static Lwm2mTreeNode * Lwm2mObjectTree_CreateNode(Lwm2mTreeNode * parent, uint16_t id, Lwm2mTreeNodeType type)
{
    if (parent == NULL)
    {
        return NULL;
    }

    Lwm2mTreeNode * newNode = Lwm2mTree_LookupNodeFromID(parent, id);
    if (newNode)
    {
        return newNode;
    }

    newNode = Lwm2mTreeNode_Create();
    if (newNode == NULL)
    {
        return NULL;
    }

    Lwm2mTreeNode_SetType(newNode, type);
    Lwm2mTreeNode_SetID(newNode, id);

    Lwm2mTreeNode_AddChild(parent, newNode);
    return newNode;
}

static Lwm2mTreeNode * Lwm2mObjectTree_LookupNodeFromOIR(Lwm2mObjectTree * objectTree, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID)
{
    if (objectTree == NULL)
    {
        return NULL;
    }

    Lwm2mTreeNode * objectNode = Lwm2mTree_LookupNodeFromID(objectTree->RootNode, objectID);
    if (objectNode == NULL)
    {
        return NULL;
    }
    else if (objectInstanceID == -1)
    {
        return objectNode;
    }

    Lwm2mTreeNode * instanceNode = Lwm2mTree_LookupNodeFromID(objectNode, objectInstanceID);
    if (instanceNode == NULL)
    {
        return NULL;
    }
    else if (resourceID == -1)
    {
        return instanceNode;
    }

    Lwm2mTreeNode * resourceNode = Lwm2mTree_LookupNodeFromID(instanceNode, resourceID);
    if (resourceNode == NULL)
    {
        return NULL;
    }
    else if (resourceInstanceID == -1)
    {
        return resourceNode;
    }

    return Lwm2mTree_LookupNodeFromID(resourceNode, resourceInstanceID);
}

int Lwm2mObjectTree_Init(Lwm2mObjectTree * objectTree)
{
    if (objectTree == NULL)
    {
        return -1;
    }

    objectTree->RootNode = Lwm2mTreeNode_Create();
    return 0;
}

int Lwm2mObjectTree_Destroy(Lwm2mObjectTree * objectTree)
{
    if (objectTree == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode_DeleteRecursive(objectTree->RootNode);
    return 0;
}

int Lwm2mObjectTree_AddObject(Lwm2mObjectTree * objectTree, ObjectIDType objectID)
{
    if (objectTree == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode * objectNode = Lwm2mTree_LookupNodeFromID(objectTree->RootNode, objectID);
    if (objectNode != NULL)
    {
        return -1;
    }

    objectNode = Lwm2mObjectTree_CreateNode(objectTree->RootNode, objectID, Lwm2mTreeNodeType_Object);
    if (objectNode == NULL)
    {
        return -1;
    }

    return 0;
}

int Lwm2mObjectTree_AddObjectInstance(Lwm2mObjectTree * objectTree, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID)
{
    if (objectTree == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode * objectNode = Lwm2mTree_LookupNodeFromID(objectTree->RootNode, objectID);
    if (objectNode == NULL)
    {
        objectNode = Lwm2mObjectTree_CreateNode(objectTree->RootNode, objectID, Lwm2mTreeNodeType_Object);
        if (objectNode == NULL)
        {
            return -1;
        }
    }

    Lwm2mTreeNode * instanceNode = Lwm2mTree_LookupNodeFromID(objectNode, objectInstanceID);
    if (instanceNode)
    {
        return -1;
    }

    instanceNode = Lwm2mObjectTree_CreateNode(objectNode, objectInstanceID, Lwm2mTreeNodeType_ObjectInstance);
    if (instanceNode == NULL)
    {
        return -1;
    }

    return 0;
}

int Lwm2mObjectTree_AddResource(Lwm2mObjectTree * objectTree, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    if (objectTree == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode * objectNode = Lwm2mTree_LookupNodeFromID(objectTree->RootNode, objectID);
    if (objectNode == NULL)
    {
        objectNode = Lwm2mObjectTree_CreateNode(objectTree->RootNode, objectID, Lwm2mTreeNodeType_Object);
        if (objectNode == NULL)
        {
            return -1;
        }
    }

    Lwm2mTreeNode * instanceNode = Lwm2mTree_LookupNodeFromID(objectNode, objectInstanceID);
    if (instanceNode == NULL)
    {
        instanceNode = Lwm2mObjectTree_CreateNode(objectNode, objectInstanceID, Lwm2mTreeNodeType_ObjectInstance);
        if (instanceNode == NULL)
        {
            return -1;
        }
    }

    Lwm2mTreeNode * resourceNode = Lwm2mTree_LookupNodeFromID(instanceNode, resourceID);
    if (resourceNode)
    {
        return -1;
    }

    resourceNode = Lwm2mObjectTree_CreateNode(instanceNode, resourceID, Lwm2mTreeNodeType_Resource);
    if (resourceNode == NULL)
    {
        return -1;
    }

    return 0;
}

int Lwm2mObjectTree_AddResourceInstance(Lwm2mObjectTree * objectTree, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID)
{
    if (objectTree == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode * objectNode = Lwm2mTree_LookupNodeFromID(objectTree->RootNode, objectID);
    if (objectNode == NULL)
    {
        objectNode = Lwm2mObjectTree_CreateNode(objectTree->RootNode, objectID, Lwm2mTreeNodeType_Object);
        if (objectNode == NULL)
        {
            return -1;
        }
    }

    Lwm2mTreeNode * instanceNode = Lwm2mTree_LookupNodeFromID(objectNode, objectInstanceID);
    if (instanceNode == NULL)
    {
        instanceNode = Lwm2mObjectTree_CreateNode(objectNode, objectInstanceID, Lwm2mTreeNodeType_ObjectInstance);
        if (instanceNode == NULL)
        {
            return -1;
        }
    }

    Lwm2mTreeNode * resourceNode = Lwm2mTree_LookupNodeFromID(instanceNode, resourceID);
    if (resourceNode == NULL)
    {
        resourceNode = Lwm2mObjectTree_CreateNode(instanceNode, resourceID, Lwm2mTreeNodeType_Resource);
        if (resourceNode == NULL)
        {
            return -1;
        }
    }

    Lwm2mTreeNode * resourceInstanceNode = Lwm2mTree_LookupNodeFromID(resourceNode, resourceInstanceID);
    if (resourceInstanceNode != NULL)
    {
        return -1;
    }

    resourceInstanceNode = Lwm2mObjectTree_CreateNode(resourceNode, resourceInstanceID, Lwm2mTreeNodeType_ResourceInstance);
    if (resourceInstanceNode == NULL)
    {
        return -1;
    }

    return 0;
}

int Lwm2mObjectTree_DeleteObject(Lwm2mObjectTree * objectTree, ObjectIDType objectID)
{
    if (objectTree == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode * objectNode = Lwm2mTree_LookupNodeFromID(objectTree->RootNode, objectID);
    if (objectNode == NULL)
        return -1;

    Lwm2mTreeNode_DeleteRecursive(objectNode);
    return 0;
}

int Lwm2mObjectTree_DeleteObjectInstance(Lwm2mObjectTree * objectTree, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID)
{
    if (objectTree == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode * instanceNode = Lwm2mObjectTree_LookupNodeFromOIR(objectTree, objectID, objectInstanceID, -1, -1);
    if (instanceNode == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode_DeleteRecursive(instanceNode);
    return 0;
}

int Lwm2mObjectTree_DeleteResource(Lwm2mObjectTree * objectTree, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    if (objectTree == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode * resourceNode = Lwm2mObjectTree_LookupNodeFromOIR(objectTree, objectID, objectInstanceID, resourceID, -1);
    if (resourceNode == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode_DeleteRecursive(resourceNode);
    return 0;
}

int Lwm2mObjectTree_DeleteResourceInstance(Lwm2mObjectTree * objectTree, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID)
{
    if (objectTree == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode * resourceInstanceNode = Lwm2mObjectTree_LookupNodeFromOIR(objectTree, objectID, objectInstanceID, resourceID, resourceInstanceID);
    if (resourceInstanceNode == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode_Delete(resourceInstanceNode);
    return 0;
}

int Lwm2mObjectTree_InitIterator(Lwm2mObjectTreeIterator * iterator, Lwm2mObjectTree * objectTree, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID)
{
    iterator->ObjectID = objectID;
    iterator->ObjectInstanceID = objectInstanceID;
    iterator->ResourceID = resourceID;
    iterator->ResourceInstanceID = resourceInstanceID;
    return 0;
}

int Lwm2mObjectTree_GetNextObjectID(Lwm2mObjectTree * objectTree, Lwm2mObjectTreeIterator * iterator)
{
    int id;
    Lwm2mTreeNode * objectNode;

    if (objectTree == NULL)
    {
        return -1;
    }

    if (iterator->ObjectID == -1)
    {
        // Find first object ID
        objectNode = Lwm2mTreeNode_GetFirstChild(objectTree->RootNode);
        if (objectNode == NULL)
        {
            return -1;
        }

        Lwm2mTreeNode_GetID(objectNode, &id);
        iterator->ObjectID = id;
        return iterator->ObjectID;
    }
    else
    {
        objectNode = Lwm2mTree_LookupNodeFromID(objectTree->RootNode, iterator->ObjectID);
        if (objectNode == NULL)
        {
            return -1;
        }
        objectNode = Lwm2mTreeNode_GetNextChild(objectTree->RootNode, objectNode);
    }
    if (objectNode == NULL)
    {
        return -1;
    }
    Lwm2mTreeNode_GetID(objectNode, &id);
    iterator->ObjectID = id;

    return iterator->ObjectInstanceID;
}

int Lwm2mObjectTree_GetNextObjectInstanceID(Lwm2mObjectTree * objectTree, Lwm2mObjectTreeIterator * iterator)
{
    if (objectTree == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode * objectNode = Lwm2mTree_LookupNodeFromID(objectTree->RootNode, iterator->ObjectID);
    if (objectNode == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode * instanceNode;
    int id;
    if (iterator->ObjectInstanceID == -1)
    {
        // Find first object instance id within the current object
        instanceNode = Lwm2mTreeNode_GetFirstChild(objectNode);
    }
    else
    {
        instanceNode = Lwm2mTree_LookupNodeFromID(objectNode, iterator->ObjectInstanceID);
        if (instanceNode == NULL)
        {
            return -1;
        }
        instanceNode = Lwm2mTreeNode_GetNextChild(objectNode, instanceNode);
    }
    if (instanceNode == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode_GetID(instanceNode, &id);
    iterator->ObjectInstanceID = id;
    return iterator->ObjectInstanceID;
}

int Lwm2mObjectTree_GetNextResourceID(Lwm2mObjectTree * objectTree, Lwm2mObjectTreeIterator * iterator)
{
    if (objectTree == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode * objectNode = Lwm2mTree_LookupNodeFromID(objectTree->RootNode, iterator->ObjectID);
    if (objectNode == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode * instanceNode = Lwm2mTree_LookupNodeFromID(objectNode, iterator->ObjectInstanceID);
    if (instanceNode == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode * resourceNode;
    int id;
    if (iterator->ResourceID == -1)
    {
        // Find first resource within the current object instance.
        resourceNode = Lwm2mTreeNode_GetFirstChild(instanceNode);
    }
    else
    {
        resourceNode = Lwm2mTree_LookupNodeFromID(instanceNode, iterator->ResourceID);
        if (resourceNode == NULL)
        {
            return -1;
        }
        resourceNode = Lwm2mTreeNode_GetNextChild(instanceNode, resourceNode);
    }

    if (resourceNode == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode_GetID(resourceNode, &id);
    iterator->ResourceID = id;
    return iterator->ResourceID;
}

int Lwm2mObjectTree_GetNextResourceInstanceID(Lwm2mObjectTree * objectTree, Lwm2mObjectTreeIterator * iterator)
{
    if (objectTree == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode * objectNode = Lwm2mTree_LookupNodeFromID(objectTree->RootNode, iterator->ObjectID);
    if (objectNode == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode * instanceNode = Lwm2mTree_LookupNodeFromID(objectNode, iterator->ObjectInstanceID);
    if (instanceNode == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode * resourceNode = Lwm2mTree_LookupNodeFromID(instanceNode, iterator->ResourceID);
    if (resourceNode == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode * resourceInstanceNode;
    int id;
    if (iterator->ResourceInstanceID == -1)
    {
        // Find first resource within the current object instance.
        resourceInstanceNode = Lwm2mTreeNode_GetFirstChild(resourceNode);
    }
    else
    {
        resourceInstanceNode = Lwm2mTree_LookupNodeFromID(resourceNode, iterator->ResourceInstanceID);
        if (resourceInstanceNode == NULL)
        {
            return -1;
        }
        resourceInstanceNode = Lwm2mTreeNode_GetNextChild(resourceNode, resourceInstanceNode);
    }

    if (resourceInstanceNode == NULL)
    {
        return -1;
    }

    Lwm2mTreeNode_GetID(resourceInstanceNode, &id);
    iterator->ResourceInstanceID = id;
    return iterator->ResourceInstanceID;
}

bool Lwm2mObjectTree_Exists(Lwm2mObjectTree * objectTree, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID)
{
    return (Lwm2mObjectTree_LookupNodeFromOIR(objectTree, objectID, objectInstanceID, resourceID, resourceInstanceID) != NULL);
}

int Lwm2mObjectTree_GetNumObjectInstances(Lwm2mObjectTree * objectTree, ObjectIDType objectID)
{
    Lwm2mTreeNode * objectNode = Lwm2mObjectTree_LookupNodeFromOIR(objectTree, objectID, -1, -1, -1);
    if (objectNode == NULL)
    {
        return 0;
    }
    return Lwm2mTreeNode_GetChildCount(objectNode);
}

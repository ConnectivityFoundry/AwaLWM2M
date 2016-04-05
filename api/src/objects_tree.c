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

#include "lwm2m_xml_serdes.h"

#include "path.h"
#include "xml.h"
#include "error.h"
#include "log.h"
#include "objects_tree.h"

#define MAX_CLIENT_ID_NAME_LEN (64)

TreeNode ObjectsTree_New(void)
{
    TreeNode objectsTree = Xml_CreateNode("Objects");
    if (objectsTree == NULL)
    {
        LogError("Out of memory");
    }
    return objectsTree;
}

void ObjectsTree_Free(TreeNode objectsTree)
{
    Tree_Delete(objectsTree);
}

static TreeNode FindNodeByID(const TreeNode parentNode, int childID, const char * childName, const char * valueName)
{
    // search for a child node that has the specified ID
    uint32_t index = 0;
    TreeNode childNode = NULL;

    while ((childNode = Xml_FindFrom(parentNode, childName, &index)) != NULL)
    {
        int ID = xmlif_GetInteger(childNode, valueName);
        if (ID == childID)
        {
            break;
        }
    }
    return childNode;
}

static TreeNode FindObjectNode(const TreeNode objectsTree, int objectID)
{
    // <Objects> contains several <Object> nodes
    return FindNodeByID(objectsTree, objectID, "Object", "Object/ID");
}

static TreeNode FindInstanceNode(const TreeNode objectNode, int instanceID)
{
    // <Object> contains one <Instance> or <Instances> node
    return FindNodeByID(objectNode, instanceID, "ObjectInstance", "ObjectInstance/ID");
}

static TreeNode FindResourceNode(const TreeNode instanceNode, int propertyID)
{
    return FindNodeByID(instanceNode, propertyID, "Resource", "Resource/ID");
}

TreeNode ObjectsTree_FindOrCreateChildNode(const TreeNode parent, const char * childName, int childID)
{
    char childIDname[MAX_CLIENT_ID_NAME_LEN] = {0};

    sprintf(childIDname, "%s/ID", childName);

    TreeNode child = FindNodeByID(parent, childID, childName, childIDname);
    if (child == NULL)
    {
        child = Xml_CreateNode(childName);
        TreeNode childIDnode = Xml_CreateNodeWithValue("ID", "%d", childID);
        TreeNode_AddChild(child, childIDnode);
        TreeNode_AddChild(parent, child);
    }
    return child;
}

size_t ObjectsTree_GetNumChildrenWithName(const TreeNode parentNode, const char * childName)
{
    uint32_t childIndex = 0;
    size_t childCount = 0;
    while (Xml_FindFrom(parentNode, childName, &childIndex) != NULL)
    {
        ++childCount;
    }
    return childCount;
}

InternalError ObjectsTree_FindPathNode(const TreeNode objectsTree, const char * path, TreeNode * _resultNode)
{
    InternalError result = InternalError_Unspecified;
    TreeNode resultNode = NULL;

    if (objectsTree != NULL)
    {
        if ((TreeNode_GetName(objectsTree) != NULL) && (strcmp("Objects", TreeNode_GetName(objectsTree)) == 0))
        {
            if (path != NULL)
            {
                AwaObjectID objectID = Path_GetObjectID(path);
                AwaObjectInstanceID objectInstanceID = Path_GetObjectInstanceID(path);
                AwaResourceID resourceID = Path_GetResourceID(path);

                if (Path_IsIDValid(objectID))
                {
                    TreeNode objectNode = FindObjectNode(objectsTree, objectID);
                    if (objectNode != NULL)
                    {
                        if (Path_IsIDValid(objectInstanceID))
                        {
                            TreeNode instanceNode = FindInstanceNode(objectNode, objectInstanceID);
                            if (instanceNode != NULL)
                            {
                                if (Path_IsIDValid(resourceID))
                                {
                                    TreeNode propertyNode = FindResourceNode(instanceNode, resourceID);
                                    if (propertyNode != NULL)
                                    {

                                        resultNode = propertyNode;
                                        result = InternalError_Success;
                                    }
                                    else
                                    {
                                        // Resource not found
                                        result = InternalError_Tree;
                                    }
                                }
                                else
                                {
                                    // Path is for object instance
                                    resultNode = instanceNode;
                                    result = InternalError_Success;
                                }
                            }
                            else
                            {
                                // Object instance not found
                                result = InternalError_Tree;
                            }
                        }
                        else
                        {
                            // Path is for object only
                            resultNode = objectNode;
                            result = InternalError_Success;
                        }
                    }
                    else
                    {
                        // Object not found
                        result = InternalError_Tree;
                    }
                }
                else
                {
                    LogError("path %s is invalid", path);
                    result = InternalError_ParameterInvalid;
                }
            }
            else
            {
                LogError("path is NULL");
                result = InternalError_ParameterInvalid;
            }
        }
        else
        {
            LogError("Node %s is not an objectsTree", TreeNode_GetName(objectsTree));
            result = InternalError_ParameterInvalid;
        }
    }
    else
    {
        LogError("objectsTree is NULL");
        result = InternalError_ParameterInvalid;
    }

    if (_resultNode != NULL)
    {
        *_resultNode = resultNode;
    }
    return result;
}

bool ObjectsTree_ContainsPath(const TreeNode objectsTree, const char * path)
{
    TreeNode resultNode = NULL;
    ObjectsTree_FindPathNode(objectsTree, path, &resultNode);
    return resultNode != NULL;
}

// Add a child node to the specified parentNode return pointer to new node
static TreeNode AddChildNode(TreeNode parentNode, const char * tag)
{
    TreeNode node = Xml_CreateNode(tag);
    TreeNode_AddChild(parentNode, node);
    return node;
}

TreeNode ObjectsTreeInternal_AddChildNode(TreeNode parentNode, const char * tag)
{
    return AddChildNode(parentNode, tag);
}


// Add a node with an ID child to the specified parentNode return pointer to new node
static TreeNode AddNodeWithID(TreeNode parentNode, const char * tag, int ID)
{
    TreeNode node = AddChildNode(parentNode, tag);
    TreeNode IDNode = Xml_CreateNodeWithValue("ID", "%d", ID);
    TreeNode_AddChild(node, IDNode);
    return node;
}

TreeNode ObjectsTreeInternal_AddNodeToTree(TreeNode parentNode, int ID, const char * tag)
{
    return AddNodeWithID(parentNode, tag, ID);
}

static TreeNode AddObjectToTree(TreeNode objectsTree, int objectID)
{
    // does Object with this ID already exist as a child of Objects?
    TreeNode objectNode = FindObjectNode(objectsTree, objectID);
    if (objectNode == NULL)
    {
        objectNode = AddNodeWithID(objectsTree, "Object", objectID);
    }
    // else build on this node
    return objectNode;
}

static TreeNode AddInstanceToTree(TreeNode objectNode, int instanceID)
{
    // does Instance with this ID already exist as a child of Object?
    TreeNode instanceNode = FindInstanceNode(objectNode, instanceID);
    return (instanceNode == NULL) ? AddNodeWithID(objectNode, "ObjectInstance", instanceID) : instanceNode;
}

static TreeNode AddResourceToTree(TreeNode instanceNode, int resourceID)
{
    // does Property with this ID already exist as a child of Instance?
    TreeNode resourceNode = FindResourceNode(instanceNode, resourceID);
    return (resourceNode == NULL) ? AddNodeWithID(instanceNode, "Resource", resourceID) : resourceNode;
}

// Using the specified path, build up new nodes in an existing tree to match.
InternalError ObjectsTree_AddPath(TreeNode objectsTree, const char * path, TreeNode * _resultNode)
{
    InternalError result = InternalError_Unspecified;
    TreeNode resultNode = NULL;

    if (objectsTree != NULL)
    {
        if (strcmp("Objects", TreeNode_GetName(objectsTree)) == 0)
        {
            if (path != NULL)
            {
                if (Path_IsValid(path))
                {
                    AwaObjectID objectID = Path_GetObjectID(path);
                    AwaObjectInstanceID objectInstanceID = Path_GetObjectInstanceID(path);
                    AwaResourceID resourceID = Path_GetResourceID(path);

                    // does path specify an Object ID?
                    if (Path_IsIDValid(objectID))
                    {
                        TreeNode objectNode = AddObjectToTree(objectsTree, objectID);
                        // Object node either already exists, or has been added
                        if (objectNode == NULL)
                        {
                            goto error;
                        }
                        resultNode = objectNode;

                        // does path specify an Object Instance ID?
                        if (Path_IsIDValid(objectInstanceID))
                        {
                            TreeNode instanceNode = AddInstanceToTree(objectNode, objectInstanceID);
                            // Instance node either already exists, or has been added
                            if (instanceNode == NULL)
                            {
                                goto error;
                            }
                            resultNode = instanceNode;

                            // does path specify a Resource ID?
                            if (Path_IsIDValid(resourceID))
                            {
                                TreeNode propertyNode = AddResourceToTree(instanceNode, resourceID);
                                // Property node either already exists, or has been added
                                if (propertyNode == NULL)
                                {
                                    goto error;
                                }
                                resultNode = propertyNode;
                            }
                        }
                    }
                    result = InternalError_Success;
                    if (_resultNode != NULL)
                    {
                        *_resultNode = resultNode;
                    }
                }
                else
                {
                    LogError("path %s is invalid", path);
                    result = InternalError_ParameterInvalid;
                }
            }
            else
            {
                LogError("path is NULL");
                result = InternalError_ParameterInvalid;
            }
        }
        else
        {
            LogError("Node %s is not an objectsTree", TreeNode_GetName(objectsTree));
            result = InternalError_ParameterInvalid;
        }
    }
    else
    {
        LogError("node is NULL");
        result = InternalError_ParameterInvalid;
    }
    return result;

error:
    LogError("Error building tree");
    return InternalError_Tree;
}

static bool HasChildNamed(const TreeNode node, const char * name)
{
    return Xml_Find(node, name) != NULL;
}

bool ObjectsTree_IsPathCovered(const TreeNode objectsTree, const char * path, TreeNode * resultNode)
{
    bool result = false;
    if (objectsTree != NULL)
    {
        if (strcmp("Objects", TreeNode_GetName(objectsTree)) == 0)
        {
            if (path != NULL)
            {
                if (Path_IsValid(path))
                {
                    // Search down the tree, looking for a terminal node that matches the path.
                    // If a terminal node is encountered, the node is considered "already added"
                    // find a matching path
                    AwaObjectID objectID = Path_GetObjectID(path);
                    AwaObjectInstanceID objectInstanceID = Path_GetObjectInstanceID(path);
                    AwaResourceID resourceID = Path_GetResourceID(path);

                    if (Path_IsIDValid(objectID))
                    {
                        TreeNode objectNode = FindObjectNode(objectsTree, objectID);
                        if (objectNode != NULL)
                        {
                            if (Path_IsIDValid(objectInstanceID))
                            {
                                TreeNode instanceNode = FindInstanceNode(objectNode, objectInstanceID);
                                if (instanceNode != NULL)
                                {
                                    if (Path_IsIDValid(resourceID))
                                    {
                                        TreeNode propertyNode = FindResourceNode(instanceNode, resourceID);
                                        if (propertyNode != NULL)
                                        {
                                            // Path is for resource

                                            // check if resource is terminal
                                            result = !HasChildNamed(propertyNode, "ResourceInstance");
                                            if (result && resultNode)
                                            {
                                                *resultNode = propertyNode;
                                            }
                                        }
                                        else
                                        {
                                            // Resource not found - path is already covered if parent instance node has no Property children
                                            result = !HasChildNamed(instanceNode, "Resource");
                                            if (result && resultNode)
                                            {
                                                *resultNode = instanceNode;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        // Path is for object instance

                                        // check if object instance is terminal
                                        result = !HasChildNamed(instanceNode, "Resource");
                                        if (result && resultNode)
                                        {
                                            *resultNode = instanceNode;
                                        }
                                    }
                                }
                                else
                                {
                                    // Object instance not found - path is already covered if parent object node has no Instance children
                                    result = !HasChildNamed(objectNode, "ObjectInstance");
                                    if (result && resultNode)
                                    {
                                        *resultNode = objectNode;
                                    }
                                }
                            }
                            else
                            {
                                // Path is for object only

                                // check if object is terminal
                                result = !HasChildNamed(objectNode, "ObjectInstance");
                                if (result && resultNode)
                                {
                                    *resultNode = objectNode;
                                }
                            }
                        }
                        else
                        {
                            // Object not found - path is new
                            result = false;
                        }
                    }
                }
                else
                {
                    LogError("path %s is invalid", path);
                }
            }
            else
            {
                LogError("path is NULL");
            }
        }
        else
        {
            LogError("Node %s is not an objectsTree", TreeNode_GetName(objectsTree));
            result = false;
        }
    }
    else
    {
        LogError("objectsTree is NULL");
    }
    return result;
}

static size_t RemoveChildrenByName(TreeNode node, const char * name)
{
    // search for child nodes that have the specified name and delete them
    uint32_t index = 0;
    size_t count = 0;
    TreeNode childNode = NULL;
    while ((childNode = Xml_FindFrom(node, name, &index)) != NULL)
    {
        LogDebug("delete %s", TreeNode_GetName(childNode));
        Tree_DetachNode(childNode);
        Tree_Delete(childNode);
        ++count;
        --index;
    }
    return count;
}

size_t ObjectsTree_RemovePathNodes(TreeNode node)
{
    size_t result = 0;

    if (node != NULL)
    {
        if (strcmp("Object", TreeNode_GetName(node)) == 0)
        {
            // for an Object node, remove all ObjectInstance nodes
            result = RemoveChildrenByName(node, "ObjectInstance");
        }
        else if (strcmp("ObjectInstance", TreeNode_GetName(node)) == 0)
        {
            // for an ObjectInstance node, remove all Resource nodes
            result = RemoveChildrenByName(node, "Resource");
        }
        else
        {
            LogWarning("Unable to remove Child Path Nodes from %s", TreeNode_GetName(node));
        }
    }
    else
    {
        LogError("node is NULL");
    }
    return result;
}

static int GetNodeID(TreeNode node)
{
    if (node == NULL)
    {
        return -1;
    }
    TreeNode idNode = Xml_Find(node, "ID");
    const uint8_t * value = TreeNode_GetValue(idNode);
    if (value)
    {
        return atoi((const char *)value);
    }
    return -1;
}

void ObjectsTree_GetIDsFromLeafNode(TreeNode node, ObjectIDType * objectID, ObjectInstanceIDType * objectInstanceID, ResourceIDType * resourceID)
{
    const char * currentNodeName = TreeNode_GetName(node);
    if (strcmp("Object", currentNodeName) == 0)
    {
        *objectID = GetNodeID(node);
        *objectInstanceID = -1;
        *resourceID = -1;
    }
    else if (strcmp("ObjectInstance", currentNodeName) == 0)
    {
        TreeNode object = TreeNode_GetParent(node);
        *objectID = GetNodeID(object);
        *objectInstanceID = GetNodeID(node);
        *resourceID = -1;
    }
    else if (strcmp("Resource", currentNodeName) == 0)
    {
        TreeNode objectInstance = TreeNode_GetParent(node);
        TreeNode object = TreeNode_GetParent(objectInstance);
        *objectID = GetNodeID(object);
        *objectInstanceID = GetNodeID(objectInstance);
        *resourceID = GetNodeID(node);
    }
    else
    {
        *objectID = *objectInstanceID = *resourceID = -1;
    }
}

const char * ObjectsTree_GetPath(TreeNode node, char * path, size_t pathLen)
{
    ObjectIDType objectID;
    ObjectInstanceIDType objectInstanceID;
    ResourceIDType resourceID;
    ObjectsTree_GetIDsFromLeafNode(node, &objectID, &objectInstanceID, &resourceID);

    if (!Path_MakePath(path, pathLen, objectID, objectInstanceID, resourceID) == AwaError_Success)
    {
        path = NULL;
    }

    return path;
}

bool ObjectsTree_IsPathNode(TreeNode node)
{
    const char * currentNodeName = TreeNode_GetName(node);
    return ((strcmp("Object", currentNodeName) == 0) ||
            (strcmp("ObjectInstance", currentNodeName) == 0) ||
            (strcmp("Resource", currentNodeName) == 0));
}

bool ObjectsTree_IsObjectNode(TreeNode node)
{
    return strcmp(TreeNode_GetName(node), "Object") == 0;
}

bool ObjectsTree_IsObjectInstanceNode(TreeNode node)
{
    return strcmp(TreeNode_GetName(node), "ObjectInstance") == 0;
}

bool ObjectsTree_IsResourceNode(TreeNode node)
{
    return strcmp(TreeNode_GetName(node), "Resource") == 0;
}

bool ObjectsTree_IsLeafNode(TreeNode node)
{
    return ((Xml_Find(node, "Object") == NULL) &&
            (Xml_Find(node, "ObjectInstance") == NULL) &&
            (Xml_Find(node, "Resource") == NULL));
}

static bool ObjectsTree_IsObjectsNode(TreeNode node)
{
    return (strcmp("Objects", TreeNode_GetName(node)) == 0);
}

TreeNode ObjectsTree_GetNextLeafNode(TreeNode currentNode)
{
    TreeNode nextNode = NULL;
    if (currentNode != NULL)
    {
        TreeNode parentNode = TreeNode_GetParent(currentNode);
        if (parentNode == NULL)
        {
            if (!ObjectsTree_IsObjectsNode(currentNode))
            {
                LogError("No Objects node in XML tree");
                goto done;
            }
        }

        nextNode = currentNode;
        TreeNode nextChildNode = NULL;

        int childIndex = 0;
        bool goingUp = false;
        while(true)
        {
            if (!goingUp)
            {
                // Find the next leaf node, or if at the last child, go up one level
                while(!ObjectsTree_IsLeafNode(nextNode))
                {
                    nextChildNode = TreeNode_GetChild(nextNode, childIndex);
                    if (nextChildNode == NULL)
                    {
                        childIndex = TreeNode_GetID(nextNode) + 1;
                        nextNode = TreeNode_GetParent(nextNode);
                        if (nextNode == NULL)
                        {
                            nextNode = NULL;
                            goto done;
                        }
                        goingUp = true;
                        break;
                    }
                    else if (ObjectsTree_IsPathNode(nextChildNode))
                    {
                        nextNode = nextChildNode;
                        childIndex = 0;
                        if (ObjectsTree_IsLeafNode(nextNode))
                        {
                            goto done;
                        }
                    }
                    else
                    {
                        childIndex++;
                    }
                }
                if (goingUp)
                {
                    goingUp = false;
                    continue;
                }
            }

            // Find the next leaf node, or go up to the next parent
            TreeNode nextNodeParent = TreeNode_GetParent(nextNode);
            int index = TreeNode_GetID(nextNode); //not + 1 because TreeNode ID's start at 1

            while (true)
            {
                nextNode = TreeNode_GetChild(nextNodeParent, index);
                if (nextNode == NULL)
                {
                    nextNode = nextNodeParent;
                    nextNodeParent = TreeNode_GetParent(nextNode);
                    childIndex = TreeNode_GetID(nextNode)+1;
                    goingUp = true;
                    if (nextNodeParent == NULL)
                    {
                        nextNode = NULL;
                        goto done;
                    }
                    break;
                }
                else if (!ObjectsTree_IsPathNode(nextNode))
                {
                    index++;
                }
                else
                {
                    if (ObjectsTree_IsLeafNode(nextNode))
                    {
                        goto done;
                    }
                    else
                    {
                        goingUp = false;
                        childIndex = 0;
                    }
                    break;
                }
            }
        }

    }
done:
    return nextNode;
}

int ObjectsTree_GetLeafCount(TreeNode objectsTree)
{
    int count = 0;
    TreeNode current = objectsTree;
    while ((current = ObjectsTree_GetNextLeafNode(current)) != NULL)
    {
        ++count;
    }
    return count;
}


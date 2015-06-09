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


#ifndef OBJECTS_TREE_V2_H
#define OBJECTS_TREE_V2_H

#include "error.h"
#include "xmltree.h"
#include "lwm2m_types.h"

#ifdef __cplusplus
extern "C" {
#endif

TreeNode ObjectsTree_New(void);

void ObjectsTree_Free(TreeNode objectsNode);

/**
 * @brief Find the node specified by the path, returning it as resultNode.
 * @param[in] objectsNode The root node of an Objects tree.
 * @param[in] path The path used to search the tree.
 * @param[out] resultNode The located path node, if found, else NULL.
 * @return InternalError_Success if the path node is located, resultNode will point to the node.
 * @return InternalError_Tree if the path node is not located, resultNode will point to NULL.
 */
InternalError ObjectsTree_FindPathNode(const TreeNode objectsNode, const char * path, TreeNode * resultNode);

/**
 * @brief Check if a path exists in an object tree.
 * @param[in] objectsNode The root node of an Objects tree to search.
 * @param[in] path The path to check
 * @return true if the path exists in the objects tree
 */
bool ObjectsTree_ContainsPath(const TreeNode objectsNode, const char * path);

/**
 * @brief Add a path to an existing Objects tree.
 * @param[in] objectsNode The root node of an Objects tree to add the path to.
 * @param[in] path The path used to construct nodes.
 * @param[out] resultNode A pointer to a TreeNode that on return references the resulting path node.
 * @return InternalError_Success if the path node is located or created, resultNode will point to the node.
 * @return InternalError_Tree if the path node cannot be located or created, resultNode will point to NULL.
 */
InternalError ObjectsTree_AddPath(TreeNode objectsNode, const char * path, TreeNode * resultNode);

/**
 * @brief Check if a new path is already covered by an existing path in the Objects
 *        tree. A node with no children (terminal node) is considered a node that the
 *        core will act upon.
 * @param[in] objectsNode The root node of an Objects tree to search.
 * @param[in] path The path to check
 * @return true if path is already covered by an existing tree node.
 */
bool ObjectsTree_IsPathCovered(const TreeNode objectsNode, const char * path, TreeNode * resultNode);

/**
 * @brief Remove any child path nodes (Instance, Instances, Property, Properties) from the node.
 * @param[in] node Node to remove child nodes from.
 * @return Number of nodes removed.
 */
size_t ObjectsTree_RemovePathNodes(TreeNode node);

/**
 * @brief Iterate through a tree and find the next leaf node. A leaf node is either an object with no instances,
 *        an object instance with no resources, or a resource.
 * @param[in] currentNode either an Objects Tree or a leaf inside the tree.
 * @return The next leaf node.
 * @return NULL if the objects tree is invalid or the current node is the last leaf in the tree.
 */
TreeNode ObjectsTree_GetNextLeafNode(TreeNode currentNode);

/**
 * @brief Determine and return the number of leaves in an objects tree
 * @param[in] objectsTree an Objects Tree node.
 * @return the number of leaves, or 0 if the objects tree is invalid.
 */
int ObjectsTree_GetLeafCount(TreeNode objectsTree);

/**
 * @brief Determine whether or not a node is valid for a path.
 * @param[in] currentNode
 * @return true if the specified node is either an Object, Object instance or Resource node.
 */
bool ObjectsTree_IsPathNode(TreeNode node);

/**
 * @brief Determine whether or not a node is a leaf node.
 * @param[in] currentNode
 * @return true if the specified node has no children which match an object, object instance, resource or resource instance.
 */
bool ObjectsTree_IsLeafNode(TreeNode node);

/**
 * @brief Determine whether or not a node is an object node.
 * @param[in] node
 * @return true if the specified node is an object node
 */
bool ObjectsTree_IsObjectNode(TreeNode node);

/**
 * @brief Determine whether or not a node is an object instance node.
 * @param[in] node
 * @return true if the specified node is an object instance node
 */
bool ObjectsTree_IsObjectInstanceNode(TreeNode node);

/**
 * @brief Determine whether or not a node is a resource node.
 * @param[in] node
 * @return true if the specified node is a resource node
 */
bool ObjectsTree_IsResourceNode(TreeNode node);

/**
 * @brief Retreive IDs from an Object, Object instance or Resource node.
 * @param[in] currentNode
 */
void ObjectsTree_GetIDsFromLeafNode(TreeNode node, ObjectIDType * objectID, ObjectInstanceIDType * objectInstanceID, ResourceIDType * resourceID);

/**
 * @brief Retreive The path representation of an Object, Object instance or Resource node.
 * @param[in] currentNode
 */
const char * ObjectsTree_GetPath(TreeNode node, char * path, size_t pathLen);

/**
 * @brief Retreive The number of children inside a parent node with the specified name.
 * @param[in] parentNode the name of the parent node.
 * @param[in] childName the name of the child nodes.
 * @return the number of children, or 0 if the parent does not exist.
 */
size_t ObjectsTree_GetNumChildrenWithName(const TreeNode parentNode, const char * childName);

/**
 * @brief Retreive a child node from a specified parent node by ID. If it does not exist, create it.
 * @param[in] parentNode the name of the parent node.
 * @param[in] childName the name of the child node.
 * @param[in] childID the ID of the child
 * @return the existing or newly created child.
 */
TreeNode ObjectsTree_FindOrCreateChildNode(const TreeNode parentNode, const char * childName, int childID);

TreeNode ObjectsTreeInternal_AddNodeToTree(TreeNode parentNode, int ID, const char * tag);
TreeNode ObjectsTreeInternal_AddChildNode(TreeNode parentNode, const char * tag);

#ifdef __cplusplus
}
#endif

#endif // OBJECTS_TREE_V2_H


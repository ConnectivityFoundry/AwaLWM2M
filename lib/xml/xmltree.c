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

#include <stdint.h>
#include <string.h>

#include "xmlparser.h"
#include "xmltree.h"
#include <stdlib.h>

#define INITIAL_TREENODE_CHILD_SLOTS    (16)
#define MAX_TREENODE_CHILDREN           (128)

#define Flow_MemRealloc realloc
#define Flow_MemAlloc malloc
static inline void Flow_MemFree(void **buffer)
{
    if (*buffer) 
    {
        free(*buffer);
        *buffer = NULL;
    }
}

static char *FlowString_DuplicateWithLength(const char *text, int textLength)
{
    char *result = NULL;
    if (text)
    {
        result = (char*)Flow_MemAlloc(textLength+1);
        if (result)
        {
            memcpy(result, text, textLength);
            result[textLength] = '\0';
        }
    }
    return result;
}


struct _TreeNode;

typedef struct
{
    struct TreeNodeImpl* Parent;            // Link to parent
    struct TreeNodeImpl** Children;         // Dynamic array of links to children
    uint32_t ChildCount;                    // Number of children
    uint32_t ChildSlots;                    // Number of potential children before reallocing

    char    *Name;                          // Node name
    uint8_t     *Value;                         // Node value
    uint32_t ChildID;                       // The ID of this child (relative to its parent node). 0 = invalid, 1 ... n = valid

} TreeNodeImpl;


//
// Pointer type
//
typedef TreeNodeImpl* _treeNode;
static TreeNode* currentTreeNode = NULL;

//
// Local functions
//


/* DOM XML-parser setup and callback functions */
void HTTP_xmlDOMBuilder_StartElementHandler(void *userData, const char *nodeName, const char **atts);
void HTTP_xmlDOMBuilder_EndElementHandler(void *userData, const char *nodeName);
void HTTP_xmlDOMBuilder_CharDataHandler(void *userData, const char *s, int len);


bool TreeNode_AddChild(TreeNode node, TreeNode child)
{
    bool result = false;
    _treeNode _node = (_treeNode) node;
    if (_node && child)
    {
        // Check whether we need to resize the children list to add a new child
        if (_node->ChildSlots <= _node->ChildCount)
        {
            TreeNodeImpl **oldChildrenList = (TreeNodeImpl**) _node->Children;

            // Double list capacity
            uint32_t newChildrenListSize = sizeof(TreeNode) * (_node->ChildSlots * 2);
            _node->Children = (struct TreeNodeImpl**) Flow_MemAlloc(newChildrenListSize);
            if (_node->Children)
            {
                _node->ChildSlots = (_node->ChildSlots * 2);
                memset(_node->Children, 0, newChildrenListSize);
                memcpy(_node->Children, oldChildrenList, _node->ChildCount * sizeof(TreeNode));
                Flow_MemFree((void **) &oldChildrenList);
            }
            else
            {
                _node->Children = (struct TreeNodeImpl**) oldChildrenList;
                goto error;
            }

        }

        // Add the new child to the list        
        _node->Children[_node->ChildCount] = child;
        _node->ChildCount++;
        ((_treeNode)child)->ChildID = _node->ChildCount;

        // Make sure the Parent pointer is set so that
        // Tree_Delete will also free the root node.
        ((_treeNode)child)->Parent = (struct TreeNodeImpl *)_node;

        result = true;
    }
error:
    return result;
}

bool Tree_DetachNode(TreeNode node)
{
    bool result = false;
    _treeNode _node = (_treeNode) node;
    if (_node)
    {
        _treeNode _parent = (_treeNode) _node->Parent;
        if (_parent)
        {
            // Detach the node from its parent's children and move all the children down to fill its place, then decrease the parent's child count
            _parent->Children[_node->ChildID - 1] = NULL;

            int i;
            for (i = _node->ChildID-1; i < _parent->ChildCount-1; i++)
            {
                _parent->Children[i] = _parent->Children[i+1];
                if (_parent->Children[i] != NULL)
                {
                    ((_treeNode)_parent->Children[i])->ChildID--;
                }
            }
            _parent->ChildCount--;
            _node->Parent = NULL;

            result = true;
        }
    }
    return result;
}

bool TreeNode_AppendValue(const TreeNode node, const uint8_t* value, const uint32_t length)
{
    bool result = false;
    _treeNode _node = (_treeNode) node;
    if (_node && value)
    {
        if (_node->Value)
        {
            int currentLength = strlen((const char*)_node->Value);
            uint8_t* newBuffer = (uint8_t*)Flow_MemRealloc(_node->Value,currentLength+1+length);
            if (newBuffer)
            {
                memcpy(&newBuffer[currentLength],value, length);
                newBuffer[currentLength+length] = '\0';
                _node->Value = newBuffer;
                result = true;
            }
        }
        else
        {
            _node->Value = (uint8_t *)FlowString_DuplicateWithLength((char *)value, length);
            if (_node->Value)
                result = true;
        }
    }
    return result;
}

TreeNode TreeNode_Create(void)
{
    _treeNode node = (_treeNode) Flow_MemAlloc(sizeof(TreeNodeImpl));
    if (node)
    {
        memset(node, 0 , sizeof(TreeNodeImpl));

        node->ChildSlots = INITIAL_TREENODE_CHILD_SLOTS;
        node->Children = (struct TreeNodeImpl**) Flow_MemAlloc(sizeof(struct TreeNodeImpl*)*node->ChildSlots);
        if (node->Children)
        {
            memset(node->Children, 0, sizeof(TreeNodeImpl*) * node->ChildSlots);
        }
        else
        {
            node->ChildSlots = 0;
            node->Children  = NULL;
        }
    }
    return node;
}

TreeNode TreeNode_CopyTreeNode(TreeNode node)
{
    _treeNode newNode = NULL;

    if (node)
    {
        newNode = TreeNode_Create();
        if (newNode != NULL)
        {
            if (((_treeNode)node)->Name)
                newNode->Name = strdup(((_treeNode)node)->Name);
            if (((_treeNode)node)->Value)
                newNode->Value = strdup(((_treeNode)node)->Value);
        }
    }

    return newNode;
}

bool TreeNode_DeleteSingle(TreeNode node)
{
    // Not a recursive delete -- *only* deletes this node
    // Links to its children will be lost if they have not been
    // 'remembered' externally
    bool result = false;
    _treeNode _node = (_treeNode) node;
    if (_node)
    {
        if(_node->Name)
            Flow_MemFree((void **)&_node->Name);
        if(_node->Value)
            Flow_MemFree((void **)&_node->Value);

        if(_node->Children)
            Flow_MemFree((void **)&_node->Children);

        Flow_MemFree((void **)&_node);
        result = true;
    }
    return result;
}

int TreeNode_GetChildCount(TreeNode node)
{
    _treeNode _node = (_treeNode) node;
    if (node)
    {
        return _node->ChildCount;
    }
    return -1;
}

int TreeNode_GetID(TreeNode node)
{
    _treeNode _node = (_treeNode) node;
    if (_node)
    {
        return _node->ChildID;
    }
    return -1;
}

TreeNode TreeNode_GetChild(TreeNode node, uint32_t index)
{
    TreeNode child = NULL;
    _treeNode _node = (_treeNode) node;
    if (_node && index < _node->ChildCount)
    {
        child = _node->Children[index];
    }
    return child;
}

const char* TreeNode_GetName(const TreeNode node)
{
    _treeNode _node = (_treeNode) node;
    if (_node && _node->Name)
    {
        return (const char*) _node->Name;
    }
    return NULL;
}

TreeNode TreeNode_GetParent(const TreeNode node)
{
    TreeNode parent = NULL;
    _treeNode _node = (_treeNode) node;
    if (_node)
        parent = _node->Parent;
    return parent;
}

const uint8_t* TreeNode_GetValue(const TreeNode node)
{
    _treeNode _node = (_treeNode) node;
    if (_node && _node->Value)
    {
        return (const uint8_t*) _node->Value;
    }
    return NULL;
}

bool TreeNode_HasParent(const TreeNode node)
{
    bool result = false;
    _treeNode _node = (_treeNode) node;

    if (_node && _node->Parent)
        result = true;

    return result;
}

TreeNode TreeNode_Navigate(const TreeNode rootNode, const char* path)
{
    _treeNode currentNode = rootNode;
    _treeNode _node = (_treeNode) rootNode;

    // Validate inputs (Check rootNode & path are not null)
    // Assuming path is null-terminated 
    if (rootNode && path)
    {
        char* _path = (char*) Flow_MemAlloc(sizeof(char) * (strlen((const char*) path)+1) );
        if (_path)
        {
            // Design note: Need to make a copy of path ('_path') as Microchip's implementation
            // of strtok() appears to try to modify the string it's working on
            memset((void*)_path, 0, (sizeof(char) * (strlen((const char*) path)+1) ));
            memcpy((void*) _path, (void*) path, sizeof(char) * strlen((const char*) path) );

            // Check for path separator '/' character
            if (strchr((const char*) _path, '/') == NULL)
            {
                if (strcmp((const char*) _node->Name, (const char*) _path) != 0)
                    currentNode = NULL;
            }
            else
            {
                // Tokenise path and ensure first pathElement matches
                char *pathElement = strtok((char*) _path, (const char*) "/");
                if (pathElement)
                {
                    if (strcmp((const char*) currentNode->Name, (const char*) pathElement) == 0)
                    {
                        bool childFound = false;
                        bool pathError = false;
                        do
                        {
                            // Then, get next path-token and find a match for a child node's name
                            pathElement = strtok(0, "/");
                            if (pathElement)
                            {
                                childFound = false;
                                uint32_t childIndex = 0;
                                for (childIndex = 0; childIndex < currentNode->ChildCount; childIndex++)
                                {
                                    // For each token, check if a child's node name matches the next token in the path
                                    _treeNode thisChild = (_treeNode) currentNode->Children[childIndex];
                                    if (strcmp((const char*) thisChild->Name, pathElement) == 0)
                                    {
                                        currentNode = thisChild;
                                        childFound = true;
                                        break;
                                    }
                                }

                                if (!childFound)
                                {
                                    currentNode = NULL;
                                    pathError = true;
                                }
                            }
                        } while(pathElement && !pathError);
                        if (pathError)
                            currentNode = NULL;
                    }
                }
            }
            Flow_MemFree((void **) &_path);
        }
        else
        {
        }
    }
    return (TreeNode) currentNode;
}

bool TreeNode_SetName(const TreeNode node, const char* name, const uint32_t length)
{
    bool result = false;
    _treeNode _node = (_treeNode) node;
    if (_node && name)
    {
        if (_node->Name)
            Flow_MemFree((void **) &_node->Name);

        _node->Name = Flow_MemAlloc(sizeof(char) * (length+1));
        if (_node->Name)
        {
            if (length > 0)
                memcpy(_node->Name, name, length);
            
            _node->Name[length] = '\0';
            result = true;
        }
    }
    return result;
}

bool TreeNode_SetParent(const TreeNode node, const TreeNode parent)
{
    bool result = false;
    _treeNode _node = (_treeNode) node;
    if (_node)
    {       
        _node->Parent = parent;
        result = true;
    }
    return result;
}

bool TreeNode_SetValue(const TreeNode node, const uint8_t* value, const uint32_t length)
{
    bool result = false;
    _treeNode _node = (_treeNode) node;
    if (_node && value)
    {
        if (_node->Value)
            Flow_MemFree((void **) &_node->Value);

        _node->Value = Flow_MemAlloc(sizeof(uint8_t) * (length+1));
        if (_node->Value)
        {
            if (length > 0)
                memcpy(_node->Value, value, length);
            
            _node->Value[length] = '\0';
            result = true;
        }
    }
    return result;
}

TreeNode Tree_Copy(TreeNode node)
{
    _treeNode _node = (_treeNode) node;
    _treeNode _newNode = NULL;

    if (_node)
    {
        _newNode = TreeNode_CopyTreeNode(node);

        if(_node->ChildCount)
        {
            uint32_t childIndex;
            for (childIndex = 0; childIndex < _node->ChildCount; childIndex++)
            {
                if (_node->Children && _node->Children[childIndex])
                {
                    _treeNode _newChildNode = Tree_Copy(_node->Children[childIndex]);

                    if (_newChildNode)
                    {
                        TreeNode_AddChild(_newNode, _newChildNode);
                    }
                }
            }
        }
    }

    return _newNode;
}

bool Tree_Delete(TreeNode node)
{
    bool result = false;
    _treeNode _rootNode = (_treeNode) node;

    if (_rootNode)
    {
        _treeNode currentNode = _rootNode;
        while (currentNode)
        {
            // Find the end node of a branch
            while (currentNode->ChildCount)
            {
                uint32_t childIndex;

                for (childIndex = 0; childIndex < currentNode->ChildSlots; childIndex++)
                {
                    if (currentNode->Children && currentNode->Children[childIndex])
                    {
                        currentNode = (_treeNode) currentNode->Children[childIndex];
                        childIndex = currentNode->ChildSlots + 1; // Exit 'for' loop
                    }
                }
            }

            //
            // Should be at bottom of a branch
            // (next, free the node)

            // If the currentNode has a parent, break the parent's link to the node
            if (currentNode->Parent)
            {
                _treeNode parentNode = (_treeNode) currentNode->Parent;
                parentNode->Children[currentNode->ChildID-1] = NULL;
                if (((_treeNode) currentNode->Parent)->ChildCount > 0)
                    ((_treeNode) currentNode->Parent)->ChildCount--;
                 currentNode->ChildID = 0;  // The node is no longer a child-node
            }
            // else, must be the 'root' node
            
            // Free the curentNode's name, value & 'children' array
            if (currentNode->Name)
                Flow_MemFree((void **) &currentNode->Name);
            if (currentNode->Value)
                Flow_MemFree((void **) &currentNode->Value);
            if (currentNode->Children)
                Flow_MemFree((void **) &currentNode->Children);

             // Move currentNode up to its parent before freeing this node
            _treeNode tempNode = currentNode;
            currentNode = (_treeNode) currentNode->Parent;
            Flow_MemFree((void **) &tempNode);

            // Rinse and repeat, now that we're at the new end of the old branch
        }
        result = true;
    }

    return result;
}

// Parse an xml document
// -- Creates and sets up an xml parser context
// -- Parses xml doc body into a DOM tree
// -- Destroys the xml parser context when done
// doc should be a char* to the xml document
// length should be the length of th xml document
// whole doc should be set to true if the entire xml doc is contained in the string pointed to by doc
TreeNode TreeNode_ParseXML(uint8_t* doc, uint32_t length, bool wholeDoc)
{
    TreeNode root = NULL;
    if (doc)
    {
        if (length)
        {
            XMLParser_Context bodyParser = XMLParser_Create();
            XMLParser_SetStartHandler(bodyParser, HTTP_xmlDOMBuilder_StartElementHandler);
            XMLParser_SetCharDataHandler(bodyParser, HTTP_xmlDOMBuilder_CharDataHandler);
            XMLParser_SetEndHandler(bodyParser, HTTP_xmlDOMBuilder_EndElementHandler);
            XMLParser_SetUserData(bodyParser, &root);
            if (XMLParser_Parse(bodyParser, (const char*) doc, length, wholeDoc))
            {
                // Parsed ok
            }
            else
            {
                // Parsing failed
                // Clean up tree
                Tree_Delete(root);
                root = NULL;
            }
            XMLParser_Destroy (bodyParser);
            currentTreeNode = NULL;
        }
    }
    return root;
}

void HTTP_xmlDOMBuilder_StartElementHandler(void *userData, const char *nodeName, const char **atts)
{
    TreeNode newNode = TreeNode_Create();
    if (newNode)
    {
        if (nodeName)
        {
            uint32_t namelength = strlen(nodeName);

            if (!TreeNode_SetName(newNode, nodeName, namelength))
            {
                TreeNode_DeleteSingle(newNode);
            }
        }
        // Check if this is the root node
        if (currentTreeNode == NULL)
            *(TreeNode*)userData = newNode;

        // Connect the new node up to its parent
        TreeNode_AddChild(currentTreeNode, newNode);

        currentTreeNode = newNode;
    }
}

void HTTP_xmlDOMBuilder_EndElementHandler(void *userData, const char *nodeName)
{
    // Return back up the tree
    currentTreeNode = TreeNode_GetParent(currentTreeNode);
}

void HTTP_xmlDOMBuilder_CharDataHandler(void *userData, const char *s, int len)
{
    if (currentTreeNode)
    {
        if (s)
        {
            TreeNode_AppendValue(currentTreeNode, (const uint8_t*) s, len);
        }
        else
        {
            // No value for this node (keep as NULL);
        }
    }
}


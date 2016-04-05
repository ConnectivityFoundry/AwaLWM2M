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
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "lwm2m_list.h"
#include "lwm2m_tree_node.h"

typedef struct
{
    struct _Lwm2mTreeNode * Parent;
    struct ListHead _List;
    struct ListHead Children;

    Lwm2mTreeNodeType NodeType;
    void * Definition;                  // if Object or Object Instance, this will point the the object definition
                                        // if Resource or Resource Instance this will point to the resource definition which
                                        // internally will have a reference back to the parent object.
    int ID;
    uint8_t * Value;                    // NULL if not a resource instance?
    uint16_t Length;                    // 0 if not a resource instance.
    bool Create;                        // create flag
    bool Replace;                       // replace flag

} _Lwm2mTreeNode;

static void Lwm2mTreeNode_Init(Lwm2mTreeNode * node)
{
    _Lwm2mTreeNode * _node = (_Lwm2mTreeNode *)node;
    memset(&_node->_List, 0, sizeof(_node->_List));
    _node->NodeType   = Lwm2mTreeNodeType_Unknown;
    _node->Value      = NULL;
    _node->Length     = 0;
    _node->ID         = -1;
    _node->Parent     = NULL;
    _node->Definition = NULL;
    _node->Create     = false;
    _node->Replace    = false;
    ListInit(&_node->Children);
}

Lwm2mTreeNode * Lwm2mTreeNode_Create(void)
{
    _Lwm2mTreeNode * node = malloc(sizeof(_Lwm2mTreeNode));
    if (node == NULL)
    {
        return NULL;
    }

    Lwm2mTreeNode_Init((Lwm2mTreeNode*)node);

    return (Lwm2mTreeNode *)node;
}

int Lwm2mTreeNode_SetType(Lwm2mTreeNode * node, Lwm2mTreeNodeType type)
{
    _Lwm2mTreeNode * _node = (_Lwm2mTreeNode *)node;
    if (node == NULL)
    {
        return -1;
    }

    if ((type < Lwm2mTreeNodeType_ResourceInstance) || (type > Lwm2mTreeNodeType_Unknown))
    {
        return -1;
    }

    _node->NodeType = type;

    return 0;
}

Lwm2mTreeNodeType Lwm2mTreeNode_GetType(Lwm2mTreeNode * node)
{
    _Lwm2mTreeNode * _node = (_Lwm2mTreeNode *)node;
    if (node == NULL)
    {
        return Lwm2mTreeNodeType_Unknown;
    }

    return _node->NodeType;
}

int Lwm2mTreeNode_SetValue(Lwm2mTreeNode * node, const uint8_t * value, uint16_t length)
{
    _Lwm2mTreeNode * _node = (_Lwm2mTreeNode *)node;
    if ((node == NULL) || (value == NULL))
        return -1;

    if (_node->Length != length)
    {
        void * temp = realloc(_node->Value, length);
        if (temp == NULL)
        {
            return -1;
        }
        _node->Value = temp;
    }
    memcpy(_node->Value, value, length);
    _node->Length = length;

    return 0;
}

const uint8_t * Lwm2mTreeNode_GetValue(Lwm2mTreeNode * node, uint16_t * length)
{
    uint8_t * result = NULL;
    _Lwm2mTreeNode * _node = (_Lwm2mTreeNode *)node;
    if ((node != NULL) && (length != NULL))
    {
        *length = _node->Length;
        result = _node->Value;
    }
    else
    {
        if (length != NULL)
        {
            *length = 0;
        }
        result = NULL;
    }
    return result;
}

int Lwm2mTreeNode_SetDefinition(Lwm2mTreeNode * node, void * definition)
{
    _Lwm2mTreeNode * _node = (_Lwm2mTreeNode *)node;
    if ((node == NULL) || (definition == NULL))
    {
        return -1;
    }

    _node->Definition = definition;
    return 0;
}

void * Lwm2mTreeNode_GetDefinition(Lwm2mTreeNode * node)
{
    _Lwm2mTreeNode * _node = (_Lwm2mTreeNode *)node;
    if (node == NULL)
    {
        return NULL;
    }

    return _node->Definition;
}

int Lwm2mTreeNode_SetID(Lwm2mTreeNode * node, int id)
{
    _Lwm2mTreeNode * _node = (_Lwm2mTreeNode *)node;
    if (node == NULL)
    {
        return -1;
    }

    _node->ID = id;
    return 0;
}

int Lwm2mTreeNode_GetID(Lwm2mTreeNode * node, int * id)
{
    _Lwm2mTreeNode * _node = (_Lwm2mTreeNode *)node;
    if ((node == NULL) || (id == NULL))
    {
        return -1;
    }

    *id = _node->ID;
    return 0;
}

int Lwm2mTreeNode_SetCreateFlag(Lwm2mTreeNode * node, bool create)
{
    _Lwm2mTreeNode * _node = (_Lwm2mTreeNode *)node;
    if (node == NULL)
    {
        return -1;
    }

    _node->Create = create;
    return 0;
}
bool Lwm2mTreeNode_IsCreateFlagSet(Lwm2mTreeNode * node)
{
    _Lwm2mTreeNode * _node = (_Lwm2mTreeNode *)node;
    if (node == NULL)
    {
        return false;
    }

    return _node->Create;
}

int Lwm2mTreeNode_SetReplaceFlag(Lwm2mTreeNode * node, bool replace)
{
    _Lwm2mTreeNode * _node = (_Lwm2mTreeNode *)node;
    if (node == NULL)
    {
        return -1;
    }

    _node->Replace = replace;
    return 0;
}
bool Lwm2mTreeNode_IsReplaceFlagSet(Lwm2mTreeNode * node)
{
    _Lwm2mTreeNode * _node = (_Lwm2mTreeNode *)node;
    if (node == NULL)
    {
        return false;
    }

    return _node->Replace;
}

int Lwm2mTreeNode_AddChild(Lwm2mTreeNode * node, Lwm2mTreeNode * child)
{
    _Lwm2mTreeNode * _node = (_Lwm2mTreeNode *)node;
    _Lwm2mTreeNode * _child = (_Lwm2mTreeNode *)child;

    if ((node == NULL) || (child == NULL))
    {
        return -1;
    }

    _child->Parent = (struct _Lwm2mTreeNode *)node;

    ListAdd(&_child->_List, &_node->Children);
    return 0;
}

Lwm2mTreeNode * Lwm2mTreeNode_GetParent(Lwm2mTreeNode * node)
{
    _Lwm2mTreeNode * _node = (_Lwm2mTreeNode*)node;

    if (node == NULL)
    {
        return NULL;
    }

    return (Lwm2mTreeNode*)_node->Parent;
}

bool Lwm2mTreeNode_HasChildren(Lwm2mTreeNode * node)
{
    _Lwm2mTreeNode * _node = (_Lwm2mTreeNode*)node;

    if (node == NULL)
    {
        return NULL;
    }

    if (_node->Children.Next == &_node->Children)
    {
        return false;
    }

    return true;
}

Lwm2mTreeNode * Lwm2mTreeNode_GetFirstChild(Lwm2mTreeNode * node)
{
    _Lwm2mTreeNode * _node = (_Lwm2mTreeNode*)node;

    if (node == NULL)
    {
        return NULL;
    }

    if (_node->Children.Next == &_node->Children)
    {
        return NULL;
    }

    return (Lwm2mTreeNode*)ListEntry(_node->Children.Next, _Lwm2mTreeNode, _List);
}

Lwm2mTreeNode * Lwm2mTreeNode_GetNextChild(Lwm2mTreeNode * node, Lwm2mTreeNode * child)
{
    _Lwm2mTreeNode * _node = (_Lwm2mTreeNode*)node;
    _Lwm2mTreeNode * _child = (_Lwm2mTreeNode*)child;

    if ((node == NULL) || (child == NULL))
    {
        return NULL;
    }

    if (_child->_List.Next == &_node->Children)
    {
        return NULL;
    }

    return (Lwm2mTreeNode*)ListEntry(_child->_List.Next, _Lwm2mTreeNode, _List);
}

int Lwm2mTreeNode_GetChildCount(Lwm2mTreeNode * node)
{
    _Lwm2mTreeNode * _node = (_Lwm2mTreeNode *)node;
    if (node == NULL)
    {
        return -1;
    }

    return ListCount(&_node->Children);
}

int Lwm2mTreeNode_Delete(Lwm2mTreeNode * node)
{
    _Lwm2mTreeNode * _node = (_Lwm2mTreeNode *)node;
    if (node == NULL)
    {
        return -1;
    }

    if (_node->Parent)
    {
        ListRemove(&_node->_List);
    }

    free(_node->Value);
    free(_node);
    return 0;
}

int Lwm2mTreeNode_DeleteRecursive(Lwm2mTreeNode * node)
{
    _Lwm2mTreeNode * _node = (_Lwm2mTreeNode *)node;
    if (node == NULL)
    {
        return -1;
    }

    if (_node->Parent)
    {
        ListRemove(&_node->_List);
    }

    Lwm2mTreeNode * child = Lwm2mTreeNode_GetFirstChild(node);
    while (child != NULL)
    {
        Lwm2mTreeNode * next = Lwm2mTreeNode_GetNextChild(node, child);
        Lwm2mTreeNode_DeleteRecursive(child);
        child = next;
    }

    free(_node->Value);
    free(_node);
    return 0;
}

Lwm2mTreeNode * Lwm2mTreeNode_FindNode(Lwm2mTreeNode * parent, int childID)
{
    Lwm2mTreeNode * child = Lwm2mTreeNode_GetFirstChild(parent);
    while (child != NULL)
    {
        int existingChildID;

        Lwm2mTreeNode_GetID(child, &existingChildID);
        if (existingChildID == childID)
        {
            return child;
        }

        child = Lwm2mTreeNode_GetNextChild(parent, child);
    }
    return NULL;
}

Lwm2mTreeNode * Lwm2mTreeNode_FindOrCreateChildNode(Lwm2mTreeNode * parent, int childID, Lwm2mTreeNodeType childType, void * definition, bool create)
{
    Lwm2mTreeNode * child = NULL;
    if (childID >= 0)
    {
        child = Lwm2mTreeNode_FindNode(parent, childID);
        if (child == NULL)
        {
            child = Lwm2mTreeNode_Create();
            Lwm2mTreeNode_SetID(child, childID);
            Lwm2mTreeNode_SetType(child, childType);
            Lwm2mTreeNode_SetCreateFlag(child, create);
            Lwm2mTreeNode_AddChild(parent, child);
            Lwm2mTreeNode_SetDefinition(child, definition);
        }
    }
    return child;
}

static void Lwm2mTreeNode_CopySingleNode(Lwm2mTreeNode * root, Lwm2mTreeNode * copy)
{
    int rootID;
    Lwm2mTreeNode_GetID(root, &rootID);
    Lwm2mTreeNode_SetID(copy, rootID);

    Lwm2mTreeNode_SetType(copy, Lwm2mTreeNode_GetType(root));
    Lwm2mTreeNode_SetDefinition(copy, Lwm2mTreeNode_GetDefinition(root));

    const uint8_t * rootValue;
    uint16_t rootValueLength;
    rootValue = Lwm2mTreeNode_GetValue(root, &rootValueLength);

    Lwm2mTreeNode_SetValue(copy, rootValue, rootValueLength);
}

static void Lwm2mTreeNode_CopyChildren(Lwm2mTreeNode * parent, Lwm2mTreeNode * parentCopy)
{
    Lwm2mTreeNode * child = Lwm2mTreeNode_GetFirstChild(parent);
    while (child != NULL)
    {
        Lwm2mTreeNode * childCopy = Lwm2mTreeNode_Create();
        Lwm2mTreeNode_AddChild(parentCopy, childCopy);

        Lwm2mTreeNode_CopySingleNode(child, childCopy);
        Lwm2mTreeNode_CopyChildren(child, childCopy);

        child = Lwm2mTreeNode_GetNextChild(parent, child);
    }
}

Lwm2mTreeNode * Lwm2mTreeNode_CopyRecursive(Lwm2mTreeNode * root)
{
    if (root == NULL)
    {
        return NULL;
    }
    Lwm2mTreeNode * copy = Lwm2mTreeNode_Create();

    Lwm2mTreeNode_CopySingleNode(root, copy);
    Lwm2mTreeNode_CopyChildren(root, copy);
    return copy;
}

int Lwm2mTreeNode_CompareRecursive(Lwm2mTreeNode * node1, Lwm2mTreeNode * node2)
{
    int result = 0;
    if (Lwm2mTreeNode_GetChildCount(node1) != Lwm2mTreeNode_GetChildCount(node2))
    {
        result = 1;
        goto done;
    }

    Lwm2mTreeNode * child1 = Lwm2mTreeNode_GetFirstChild(node1);
    Lwm2mTreeNode * child2 = Lwm2mTreeNode_GetFirstChild(node2);
    while (child1)
    {
        int child1ID;
        int child2ID;
        const uint8_t * child1Value;
        const uint8_t * child2Value;
        uint16_t child1ValueLength;
        uint16_t child2ValueLength;

        Lwm2mTreeNode_GetID(child1, &child1ID);
        Lwm2mTreeNode_GetID(child2, &child2ID);

        child1Value = Lwm2mTreeNode_GetValue(child1, &child1ValueLength);
        child2Value = Lwm2mTreeNode_GetValue(child2, &child2ValueLength);

        if (child1ID != child2ID)
        {
            result = 1;
            goto done;
        }
        else if (child1ValueLength != child2ValueLength)
        {
            result = 1;
            goto done;
        }
        else if (child1ValueLength != 0 && memcmp(child1Value, child2Value, child1ValueLength))
        {
            result = 1;
            goto done;
        }
        else if (Lwm2mTreeNode_GetType(child1) != Lwm2mTreeNode_GetType(child2))
        {
            result = 1;
            goto done;
        }
        else if (Lwm2mTreeNode_GetDefinition(child1) != Lwm2mTreeNode_GetDefinition(child2))
        {
            result = 1;
            goto done;
        }
        else if ((result = Lwm2mTreeNode_CompareRecursive(child1, child2)) != 0)
        {
            goto done;
        }
        child1 = Lwm2mTreeNode_GetNextChild(node1, child1);
        child2 = Lwm2mTreeNode_GetNextChild(node2, child2);
    }
done:
    return result;
}


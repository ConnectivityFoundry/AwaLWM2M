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


#ifndef LWM2M_TREE_NODE_H
#define LWM2M_TREE_NODE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    Lwm2mTreeNodeType_ResourceInstance,
    Lwm2mTreeNodeType_Resource,
    Lwm2mTreeNodeType_ObjectInstance,
    Lwm2mTreeNodeType_Object,
    Lwm2mTreeNodeType_Root,  // stores multiple object nodes
    Lwm2mTreeNodeType_Unknown,
} Lwm2mTreeNodeType;

typedef struct _Lwm2mTreeNode Lwm2mTreeNode;

Lwm2mTreeNode * Lwm2mTreeNode_Create(void);

int Lwm2mTreeNode_SetType(Lwm2mTreeNode * node, Lwm2mTreeNodeType type);
Lwm2mTreeNodeType Lwm2mTreeNode_GetType(Lwm2mTreeNode * node);

int Lwm2mTreeNode_SetDefinition(Lwm2mTreeNode * node, void * definition);
void * Lwm2mTreeNode_GetDefinition(Lwm2mTreeNode * node);

int Lwm2mTreeNode_SetValue(Lwm2mTreeNode * node, const uint8_t * value, uint16_t length);
const uint8_t * Lwm2mTreeNode_GetValue(Lwm2mTreeNode * node, uint16_t * length);

int Lwm2mTreeNode_SetID(Lwm2mTreeNode * node, int id);
int Lwm2mTreeNode_GetID(Lwm2mTreeNode * node, int * id);

int Lwm2mTreeNode_SetCreateFlag(Lwm2mTreeNode * node, bool create);
bool Lwm2mTreeNode_IsCreateFlagSet(Lwm2mTreeNode * node);

int Lwm2mTreeNode_SetReplaceFlag(Lwm2mTreeNode * node, bool replace);
bool Lwm2mTreeNode_IsReplaceFlagSet(Lwm2mTreeNode * node);

int Lwm2mTreeNode_AddChild(Lwm2mTreeNode * node, Lwm2mTreeNode * child);
bool Lwm2mTreeNode_HasChildren(Lwm2mTreeNode * node);
int Lwm2mTreeNode_GetChildCount(Lwm2mTreeNode * node);

Lwm2mTreeNode * Lwm2mTreeNode_GetFirstChild(Lwm2mTreeNode * node);
Lwm2mTreeNode * Lwm2mTreeNode_GetNextChild(Lwm2mTreeNode * node, Lwm2mTreeNode * child);

Lwm2mTreeNode * Lwm2mTreeNode_GetParent(Lwm2mTreeNode * node);

Lwm2mTreeNode * Lwm2mTreeNode_FindNode(Lwm2mTreeNode * parent, int id);
Lwm2mTreeNode * Lwm2mTreeNode_FindOrCreateChildNode(Lwm2mTreeNode * parent, int childID, Lwm2mTreeNodeType childType, void * definition, bool create);

int Lwm2mTreeNode_Delete(Lwm2mTreeNode * node);
int Lwm2mTreeNode_DeleteRecursive(Lwm2mTreeNode * node);

int Lwm2mTreeNode_CompareRecursive(Lwm2mTreeNode * node1, Lwm2mTreeNode * node2);

Lwm2mTreeNode * Lwm2mTreeNode_CopyRecursive(Lwm2mTreeNode * root);

#ifdef __cplusplus
}
#endif

#endif // LWM2M_TREE_NODE_H


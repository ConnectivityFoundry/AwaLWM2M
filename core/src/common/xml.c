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


#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "xml.h"

#define IPC_MAX_BUFFER_LEN (65536)

static int _TreeToString(const TreeNode node, char * buffer, size_t bufferSize, int level);

void Xml_TreeToStdout(const TreeNode node, const char * tag)
{
    static const size_t bufferSize = IPC_MAX_BUFFER_LEN;
    char buffer[bufferSize];
    _TreeToString(node, buffer, bufferSize, 0);
    printf("%s:\n%s\n", tag == NULL ? "Xml_TreeToStdout" : tag, buffer);
}

int Xml_TreeToString(const TreeNode node, char * buffer, size_t bufferSize)
{
    return _TreeToString(node, buffer, bufferSize, 0);
}

TreeNode Xml_CreateNode(const char * name)
{
    TreeNode node = TreeNode_Create();
    if (node != NULL)
    {
        if (!TreeNode_SetName(node, name, strlen(name)))
        {
            TreeNode_DeleteSingle(node);
            node = NULL;
        }
    }
    return node;
}

TreeNode Xml_CreateNodeWithValue(const char * name, const char * format, ...)
{
    va_list args;

    TreeNode node = Xml_CreateNode(name);
    if (node == NULL)
    {
        return NULL;
    }
    va_start(args, format);
    int requiredSize = vsnprintf(NULL, 0, format, args);
    va_end(args);

    char * value = malloc(requiredSize + 1);
    if (value == NULL)
    {
        Tree_Delete(node);
        return NULL;
    }

    memset(value, 0, requiredSize + 1);

    va_start(args, format);
    vsprintf(value, format, args);
    va_end(args);

    TreeNode_SetValue(node, (const uint8_t *)value, strlen(value));

    free(value);

    return node;
}

TreeNode Xml_Find(const TreeNode node, const char * name)
{
    uint32_t index = 0;
    return Xml_FindFrom(node, name, &index);
}

TreeNode Xml_FindFrom(const TreeNode node, const char * name, uint32_t * index)
{
    TreeNode child;
    while ((child = TreeNode_GetChild(node, *index)))
    {
        const char * childName = TreeNode_GetName(child);
        if ((childName != NULL) && (strcmp(name, childName) == 0))
        {
            (*index)++;
            return child;
        }
        (*index)++;
    }
    return NULL;
}

TreeNode Xml_FindChildWithGrandchildValue(const TreeNode parentNode, const char * childName, const char * grandchildName, const char * grandchildValue)
{
    TreeNode child;
    int index = 0;
    while ((child = TreeNode_GetChild(parentNode, index)))
    {
        if (strcmp(childName, TreeNode_GetName(child)) == 0)
        {
            TreeNode grandchild;
            if ((grandchild = Xml_Find(child, grandchildName)) != NULL)
            {
                const char * value = (const char * )TreeNode_GetValue(grandchild);
                if (value != NULL)
                {
                    if (strcmp(grandchildValue, value) == 0)
                    {
                        //found
                        break;
                    }
                }
            }
        }
        index++;
    }
    return child;
}

void Xml_Dump(const TreeNode node)
{
    char buffer[65536] = { 0 };
    Xml_TreeToString(node, buffer, sizeof(buffer));
    printf("tree: %s\n", buffer);
}

static int _TreeToString(const TreeNode node, char * buffer, size_t bufferSize, int level)
{
    const char * name = TreeNode_GetName(node);
    int pos = 0;
    int rc;

    if (TreeNode_GetChild(node, 0) != NULL)
    {
        TreeNode child;
        int idx = 0;

        rc = snprintf(buffer, bufferSize, "%*s<%s>\n", level, "", name);
        if ((rc == -1) || (rc >= bufferSize))
        {
            return -1;
        }
        pos += rc;

        while ((child = TreeNode_GetChild(node, idx++)) != NULL)
        {
            rc = _TreeToString(child, &buffer[pos], bufferSize - pos, level + 1);
            if (rc < 0)
            {
                return -1;
            }
            pos += rc;
        }

        rc = snprintf(&buffer[pos], bufferSize - pos, "%*s</%s>\n", level, "", name);
        if ((rc == -1) || (rc >= bufferSize - pos))
        {
            return -1;
        }
        pos += rc;
    }
    else
    {
        const char * value = (const char *)TreeNode_GetValue(node);

        rc = snprintf(buffer, bufferSize, "%*s<%s>%s</%s>\n", level, "", name, value ? value : "", name);
        if ((rc == -1) || (rc >= bufferSize))
        {
            return -1;
        }
        pos += rc;
    }

    return pos;
}


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


#ifndef XML_H
#define XML_H

#include <xmltree.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Print tree to stdout.
 * @param[in] node Root of XML tree.
 * @param[in] tag Optional string to output before dump.
 */
void Xml_TreeToStdout(const TreeNode node, const char * tag);

/**
 * @brief Render XML tree to buffer.
 * @param[in] node Root of XML tree.
 * @param[out] buffer Buffer for resultant string.
 * @param[in] bufferSize Size of buffer for resultant string.
 * @return Resultant string length on success, -1 if buffer overruns.
 */
int Xml_TreeToString(const TreeNode node, char * buffer, size_t bufferSize);

/**
 * @brief Create an XML TreeNode with specified name.
 * @param[in] name Name of the node.
 * @return TreeNode, or NULL on failure.
 */
TreeNode Xml_CreateNode(const char * name);

/**
 * @brief Create an XML TreeNode with specified name and value.
 * @param[in] name Name of the node.
 * @param[in] format Printf-style format string.
 * @param[in] ... Printf-style variadic arguments.
 * @return TreeNode, or NULL on failure.
 */
TreeNode Xml_CreateNodeWithValue(const char * name, const char * format, ...);

/**
 * @brief Find a child element by name.
 * @param[in] node Parent to search.
 * @param[in] name Name of child to search for.
 * @return TreeNode if found, or NULL on failure.
 */
TreeNode Xml_Find(const TreeNode node, const char * name);

/**
 * @brief Find a child element by name, starting at a child index.
 * @param[in] node Parent to search.
 * @param[in] name Name of child to search for.
 * @param[in/out] index Start position, and found position if return is not NULL.
 * @return TreeNode if found, or NULL on failure.
 */
TreeNode Xml_FindFrom(const TreeNode node, const char * name, uint32_t * index);

/**
 * @brief Given a parent node, Find a child element that has a child tag that matches a value.
 * @param[in] parentNode Parent to search.
 * @param[in] childName Name of child to search for.
 * @param[in] grandchildName Name of grandchild which contains a value.
 * @param[in] childID value of the ID tag of the child to search for.
 * @return TreeNode if found, or NULL on failure.
 */
TreeNode Xml_FindChildWithGrandchildValue(const TreeNode parentNode, const char * childName, const char * grandchildName, const char * grandchildValue);

/**
 * @brief Dump a tree to stdout, for debugging purposes.
 * @param[in] node Root of tree to display.
 * @return void
 */
void Xml_Dump(const TreeNode node);

#ifdef __cplusplus
}
#endif

#endif // XML_H


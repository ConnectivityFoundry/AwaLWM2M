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

#include <gtest/gtest.h>

#include "xmltree.h"

namespace FlowCore {

class TestXMLTree : public ::testing::Test {};

TEST_F(TestXMLTree, frees_each_single_child_node)
{
    TreeNode rootNode = TreeNode_Create();
    TreeNode childNode = TreeNode_Create();
    TreeNode_AddChild(rootNode, childNode);
    TreeNode_DeleteSingle(childNode);
    TreeNode_DeleteSingle(rootNode);
}

TEST_F(TestXMLTree, frees_single_child_node_tree)
{
    TreeNode rootNode = TreeNode_Create();
    TreeNode childNode = TreeNode_Create();
    TreeNode_AddChild(rootNode, childNode);
    //TreeNode_SetParent(childNode, rootNode);
    Tree_Delete(rootNode);
}

TEST_F(TestXMLTree, frees_without_detached_node)
{
    TreeNode rootNode = TreeNode_Create();
    TreeNode childNode = TreeNode_Create();
    TreeNode grandchildNode = TreeNode_Create();
    TreeNode_AddChild(rootNode, childNode);
    TreeNode_AddChild(childNode, grandchildNode);

    EXPECT_TRUE(Tree_DetachNode(childNode));
    EXPECT_TRUE(NULL == TreeNode_GetChild(rootNode, 0));
    EXPECT_TRUE(NULL == TreeNode_GetParent(childNode));
    EXPECT_EQ(TreeNode_GetChildCount(rootNode), 0);

    EXPECT_TRUE(Tree_Delete(rootNode));
    EXPECT_EQ(TreeNode_GetChildCount(childNode), 1);

    EXPECT_TRUE(Tree_Delete(childNode));
}

TEST_F(TestXMLTree, copynode_is_valid)
{
    const char * expectedName = "Node Name";
    const char * expectedValue = "Node Value";
    TreeNode newNode = TreeNode_Create();

    TreeNode_SetName(newNode, expectedName, strlen(expectedName));
    TreeNode_SetValue(newNode, (const uint8_t *)expectedValue, strlen(expectedValue));

    TreeNode copyNode = NULL;
    copyNode = TreeNode_CopyTreeNode(newNode);
    ASSERT_TRUE(copyNode != NULL);
    ASSERT_STREQ(expectedName, TreeNode_GetName(copyNode));
    ASSERT_STREQ(expectedValue, (const char *)TreeNode_GetValue(copyNode));

    TreeNode_DeleteSingle(newNode);
    TreeNode_DeleteSingle(copyNode);
}

#define expectedRootNode1Name_str "rootNode"
#define expectedRootchildNode1Name_str "rootchildNode1"
#define expectedRootchildNode2Name_str "rootchildNode2"
#define expectedRootchildchildNode1Name_str "rootchildchildNode1"
#define expectedRootchildchildNode2Name_str "rootchildchildNode2"

TEST_F(TestXMLTree, copytree_is_valid)
{
    const char * expectedRootNode1Name = expectedRootNode1Name_str;
    TreeNode rootNode = TreeNode_Create();
    TreeNode_SetName(rootNode, expectedRootNode1Name, strlen(expectedRootNode1Name));

    const char * expectedRootchildNode1Name = expectedRootchildNode1Name_str;
    TreeNode rootchildNode1 = TreeNode_Create();
    TreeNode_SetName(rootchildNode1, expectedRootchildNode1Name, strlen(expectedRootchildNode1Name));
    TreeNode_AddChild(rootNode, rootchildNode1);

    const char * expectedRootchildNode2Name = expectedRootchildNode2Name_str;
    const char * expectedRootchildNode2Value = "rootchildNode2 Value";
    TreeNode rootchildNode2 = TreeNode_Create();
    TreeNode_SetName(rootchildNode2, expectedRootchildNode2Name, strlen(expectedRootchildNode2Name));
    TreeNode_SetValue(rootchildNode2, (const uint8_t *)expectedRootchildNode2Value, strlen(expectedRootchildNode2Value));
    TreeNode_AddChild(rootNode, rootchildNode2);

    const char * expectedRootchildchildNode1Name = expectedRootchildchildNode1Name_str;
    const char * expectedRootchildchildNode1Value = "rootchildchildNode1 Value";
    TreeNode rootchildchildNode1 = TreeNode_Create();
    TreeNode_SetName(rootchildchildNode1, expectedRootchildchildNode1Name, strlen(expectedRootchildchildNode1Name));
    TreeNode_SetValue(rootchildchildNode1, (const uint8_t *)expectedRootchildchildNode1Value, strlen(expectedRootchildchildNode1Value));
    TreeNode_AddChild(rootchildNode1, rootchildchildNode1);

    const char * expectedRootchildchildNode2Name = expectedRootchildchildNode2Name_str;
    const char * expectedRootchildchildNode2Value = "rootchildchildNode2 Value";
    TreeNode rootchildchildNode2 = TreeNode_Create();
    TreeNode_SetName(rootchildchildNode2, expectedRootchildchildNode2Name, strlen(expectedRootchildchildNode2Name));
    TreeNode_SetValue(rootchildchildNode2, (const uint8_t *)expectedRootchildchildNode2Value, strlen(expectedRootchildchildNode2Value));
    TreeNode_AddChild(rootchildNode1, rootchildchildNode2);

    TreeNode rootNodeCopy = Tree_Copy(rootNode);
    ASSERT_TRUE(NULL != rootNodeCopy);

    TreeNode rootchildNode1Copy = TreeNode_Navigate(rootNodeCopy, expectedRootNode1Name_str "/" expectedRootchildNode1Name_str);
    ASSERT_TRUE(NULL != rootchildNode1Copy);
    ASSERT_STREQ(expectedRootchildNode1Name, TreeNode_GetName(rootchildNode1Copy));

    TreeNode rootchildNode2Copy = TreeNode_Navigate(rootNodeCopy, expectedRootNode1Name_str "/" expectedRootchildNode2Name_str);
    ASSERT_TRUE(NULL != rootchildNode2Copy);
    ASSERT_STREQ(expectedRootchildNode2Name, TreeNode_GetName(rootchildNode2Copy));
    ASSERT_STREQ(expectedRootchildNode2Value, (const char *)TreeNode_GetValue(rootchildNode2Copy));

    TreeNode rootchildchildNode1Copy = TreeNode_Navigate(rootNodeCopy, expectedRootNode1Name_str "/" expectedRootchildNode1Name_str "/" expectedRootchildchildNode1Name_str);
    ASSERT_TRUE(NULL != rootchildchildNode1Copy);
    ASSERT_STREQ(expectedRootchildchildNode1Name, TreeNode_GetName(rootchildchildNode1Copy));
    ASSERT_STREQ(expectedRootchildchildNode1Value, (const char *)TreeNode_GetValue(rootchildchildNode1Copy));

    TreeNode rootchildchildNode2Copy = TreeNode_Navigate(rootNodeCopy, expectedRootNode1Name_str "/" expectedRootchildNode1Name_str "/" expectedRootchildchildNode2Name_str);
    ASSERT_TRUE(NULL != rootchildchildNode2Copy);
    ASSERT_STREQ(expectedRootchildchildNode2Name, TreeNode_GetName(rootchildchildNode2Copy));
    ASSERT_STREQ(expectedRootchildchildNode2Value, (const char *)TreeNode_GetValue(rootchildchildNode2Copy));

    Tree_Delete(rootNode);
    Tree_Delete(rootNodeCopy);
}

} // namespace FlowCore

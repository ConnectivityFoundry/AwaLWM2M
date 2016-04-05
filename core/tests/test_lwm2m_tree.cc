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
#include <string>
#include <stdio.h>
#include "lwm2m_tree_node.h"

class Lwm2mTreeNodeTestSuite : public testing::Test
{
  void SetUp() { }
  void TearDown() { }
};

TEST_F(Lwm2mTreeNodeTestSuite, test_create_delete)
{
    Lwm2mTreeNode * node = Lwm2mTreeNode_Create();
     
    // test delete on NULL pointer
    ASSERT_EQ(-1, Lwm2mTreeNode_DeleteRecursive(NULL));
    // test delete on TreeNode
    ASSERT_EQ(0, Lwm2mTreeNode_DeleteRecursive(node));
}

TEST_F(Lwm2mTreeNodeTestSuite, test_set_get_value)
{
    const char * value = "hello world";
    const char  * larger_value = "this is a larger value";
    const char * short_value = "short";

    uint16_t length;
    const uint8_t * resultValue;

    Lwm2mTreeNode * node = Lwm2mTreeNode_Create();

    // test passing a NULL pointer
    ASSERT_EQ(-1, Lwm2mTreeNode_SetValue(NULL, (const uint8_t*)value, strlen(value)));
    ASSERT_EQ(-1, Lwm2mTreeNode_SetValue(node, NULL, 0));
    ASSERT_EQ(-1, Lwm2mTreeNode_SetValue(NULL, NULL, 0));

    // pass valid value
    ASSERT_EQ(0, Lwm2mTreeNode_SetValue(node, (const uint8_t*)value, strlen(value)));

    // test passing NULL pointers to get value
    ASSERT_EQ(NULL, Lwm2mTreeNode_GetValue(NULL, &length));
    ASSERT_EQ(NULL, Lwm2mTreeNode_GetValue(node, NULL));
    ASSERT_EQ(NULL, Lwm2mTreeNode_GetValue(NULL, NULL));

    // test passing value parameters to get
    resultValue = Lwm2mTreeNode_GetValue(node, &length);
    ASSERT_EQ(strlen(value), length);
    ASSERT_EQ(0, memcmp(value, resultValue, strlen(value)));

    // check we can replace the value with something larger
    ASSERT_EQ(0, Lwm2mTreeNode_SetValue(node, (const uint8_t*)larger_value, strlen(larger_value)));

    // check value has changed
    resultValue = Lwm2mTreeNode_GetValue(node, &length);
    ASSERT_EQ(strlen(larger_value), length);
    ASSERT_EQ(0, memcmp(larger_value, resultValue, strlen(larger_value)));

    // check we can replace the value with something shorter
    ASSERT_EQ(0, Lwm2mTreeNode_SetValue(node, (const uint8_t*)short_value, strlen(short_value)));

    // check value has changed
    resultValue = Lwm2mTreeNode_GetValue(node, &length);
    ASSERT_EQ(strlen(short_value), length);
    ASSERT_EQ(0, memcmp(short_value, resultValue, strlen(short_value)));

    Lwm2mTreeNode_DeleteRecursive(node);
}

TEST_F(Lwm2mTreeNodeTestSuite, test_add_children)
{
    uint16_t len;
    const char * parent_value = "this is the parent";
    const char * child_value = "this is the child";
    const char * child2_value = "this is another child";

    Lwm2mTreeNode * node = Lwm2mTreeNode_Create();
    Lwm2mTreeNode_SetValue(node, (const uint8_t*)parent_value, strlen(parent_value));

    Lwm2mTreeNode * child = Lwm2mTreeNode_Create();
    Lwm2mTreeNode_SetValue(child, (const uint8_t*)child_value, strlen(child_value));

    ASSERT_EQ(-1, Lwm2mTreeNode_AddChild(NULL, child));
    ASSERT_EQ(-1, Lwm2mTreeNode_AddChild(node, NULL));
    ASSERT_EQ(-1, Lwm2mTreeNode_AddChild(NULL, NULL));

    ASSERT_EQ(0, Lwm2mTreeNode_AddChild(node, child));

    Lwm2mTreeNode * child2 = Lwm2mTreeNode_Create();
    Lwm2mTreeNode_SetValue(child2, (const uint8_t*)child2_value, strlen(child2_value));

    ASSERT_EQ(0, Lwm2mTreeNode_AddChild(node, child2));

    Lwm2mTreeNode * temp = Lwm2mTreeNode_GetFirstChild(node);
    ASSERT_EQ(child, temp);
    ASSERT_EQ(0, memcmp(child_value, (const char*)Lwm2mTreeNode_GetValue(temp, &len), strlen(child_value)));

    temp = Lwm2mTreeNode_GetNextChild(node, temp);
    ASSERT_EQ(child2, temp);
    ASSERT_EQ(0, memcmp(child2_value, (const char *)Lwm2mTreeNode_GetValue(temp, &len), strlen(child2_value)));

    temp = Lwm2mTreeNode_GetNextChild(node, temp);
    ASSERT_EQ(NULL, temp);

    Lwm2mTreeNode_DeleteRecursive(child);
    Lwm2mTreeNode_DeleteRecursive(child2);
    Lwm2mTreeNode_DeleteRecursive(node);
}

TEST_F(Lwm2mTreeNodeTestSuite, test_set_get_id)
{
    Lwm2mTreeNode * node = Lwm2mTreeNode_Create();

    int id = 5;

    ASSERT_EQ(-1, Lwm2mTreeNode_GetID(node, NULL));
    ASSERT_EQ(-1, Lwm2mTreeNode_GetID(NULL, NULL));
    ASSERT_EQ(0, Lwm2mTreeNode_GetID(node, &id));
    ASSERT_EQ(-1, id);

    ASSERT_EQ(-1, Lwm2mTreeNode_SetID(NULL, 10));
    ASSERT_EQ(0, Lwm2mTreeNode_SetID(node, 10));

    ASSERT_EQ(0, Lwm2mTreeNode_GetID(node, &id));
    ASSERT_EQ(10, id);
   
    Lwm2mTreeNode_DeleteRecursive(node);
}

TEST_F(Lwm2mTreeNodeTestSuite, test_get_child_count)
{
    const char * parent_value = "this is the parent";
    const char * child_value = "this is the child";
    const char * child2_value = "this is another child";

    Lwm2mTreeNode * node = Lwm2mTreeNode_Create();
    Lwm2mTreeNode_SetValue(node, (const uint8_t*)parent_value, strlen(parent_value));

    Lwm2mTreeNode * child = Lwm2mTreeNode_Create();
    Lwm2mTreeNode_SetValue(child, (const uint8_t*)child_value, strlen(child_value));

    ASSERT_EQ(-1, Lwm2mTreeNode_AddChild(NULL, child));
    ASSERT_EQ(-1, Lwm2mTreeNode_AddChild(node, NULL));
    ASSERT_EQ(-1, Lwm2mTreeNode_AddChild(NULL, NULL));

    ASSERT_EQ(0, Lwm2mTreeNode_GetChildCount(node));

    ASSERT_EQ(0, Lwm2mTreeNode_AddChild(node, child));

    ASSERT_EQ(1, Lwm2mTreeNode_GetChildCount(node));

    Lwm2mTreeNode * child2 = Lwm2mTreeNode_Create();
    Lwm2mTreeNode_SetValue(child2, (const uint8_t*)child2_value, strlen(child2_value));

    ASSERT_EQ(0, Lwm2mTreeNode_AddChild(node, child2));

    ASSERT_EQ(2, Lwm2mTreeNode_GetChildCount(node));


    Lwm2mTreeNode_DeleteRecursive(child);

    ASSERT_EQ(1, Lwm2mTreeNode_GetChildCount(node));

    Lwm2mTreeNode_DeleteRecursive(child2);

    ASSERT_EQ(0, Lwm2mTreeNode_GetChildCount(node));

    Lwm2mTreeNode_DeleteRecursive(node);
}




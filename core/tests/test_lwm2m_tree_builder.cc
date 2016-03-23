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
#include <stdint.h>

// https://meekrosoft.wordpress.com/2009/11/09/unit-testing-c-code-with-the-googletest-framework/
// 1. Define fake functions for the dependencies you want to stub out
// 2. If the module depends on a global (gasp!) you need to define your fake one
// 3. include your module implementation (#include module.c)
// 4. Define a method to reset all the static data to a known state.
// 5. Define your tests

#include "common/lwm2m_tree_node.h"
#include "common/lwm2m_tree_builder.h"
#include "lwm2m_core.h"
#include "common/lwm2m_request_origin.h"

class Lwm2mTreeBuilderTestSuite : public testing::Test
{
    void SetUp() { context = Lwm2mCore_Init(NULL, NULL); }
    void TearDown() { Lwm2mCore_Destroy(context); }

protected:
    Lwm2mContextType * context;
};

TEST_F(Lwm2mTreeBuilderTestSuite, test_build_resource_node)
{
    const char * expected = "coap://bootstrap.example.com:5684/";
    Lwm2m_SetLogLevel(DebugLevel_Debug);

    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 0, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 0, 0, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 0, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 0, 0, 0, 0, (char*)expected, strlen(expected));

    Lwm2mTreeNode * dest;
    TreeBuilder_CreateTreeFromResource(&dest,context, Lwm2mRequestOrigin_Client,0,0,0);

    ASSERT_TRUE(dest != NULL);
    ASSERT_TRUE(Lwm2mTreeNode_GetParent(dest) == NULL);
    ASSERT_EQ(Lwm2mTreeNodeType_Resource, Lwm2mTreeNode_GetType(dest));

    int resourceNodeID;
    ASSERT_EQ(0, Lwm2mTreeNode_GetID(dest, &resourceNodeID));
    EXPECT_EQ(0, resourceNodeID);

    ASSERT_TRUE(Lwm2mTreeNode_GetDefinition(dest) != NULL);
    ResourceDefinition* definition = (ResourceDefinition *)Lwm2mTreeNode_GetDefinition(dest);

    EXPECT_STREQ("Res1", definition->ResourceName);
    EXPECT_EQ(0, definition->ResourceID);

    ASSERT_EQ(1, Lwm2mTreeNode_GetChildCount(dest));

    Lwm2mTreeNode * child = Lwm2mTreeNode_GetFirstChild(dest);
    ASSERT_TRUE(child != NULL);

    int resourceInstanceNodeID;
    ASSERT_EQ(0, Lwm2mTreeNode_GetID(child, &resourceInstanceNodeID));
    EXPECT_EQ(0, resourceInstanceNodeID);

    uint16_t resourceInstanceNodeValueLength;
    char * value = (char *)Lwm2mTreeNode_GetValue(child, &resourceInstanceNodeValueLength);
    ASSERT_TRUE(value != NULL);
    ASSERT_EQ(strlen(expected) , resourceInstanceNodeValueLength);
    ASSERT_TRUE(memcmp(expected, value, resourceInstanceNodeValueLength) == 0);

    Lwm2mTreeNode_DeleteRecursive(dest);
}

// TODO: test_build_[object/object_instance]_node
// TODO: test multiple instances and non-zero IDs


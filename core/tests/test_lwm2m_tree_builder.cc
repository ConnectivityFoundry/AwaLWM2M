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
    ASSERT_TRUE(memcmp(expected, (char *)Lwm2mTreeNode_GetValue(child, &resourceInstanceNodeValueLength),resourceInstanceNodeValueLength) == 0);
    EXPECT_EQ(strlen(expected) , resourceInstanceNodeValueLength);  // +1 for added null terminator

    Lwm2mTreeNode_DeleteRecursive(dest);
}

// TODO: test_build_[object/object_instance]_node
// TODO: test multiple instances and non-zero IDs


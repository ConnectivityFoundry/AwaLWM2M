#include <gtest/gtest.h>
#include "../src/objects_tree.h"

#include "support/support.h"
#include "support/xml_support.h"

#include "awa/client.h"
#include "set_operation.h"
#include "log.h"
#include "set_write_common.h"

namespace Awa {

class TestSetWriteCommon : public TestClientBase {};


///***********************************************************************************************************
// * SetWriteCommon_ AddValue / AddCreate tag tests
// */

TEST_F(TestSetWriteCommon, SetWriteCommon_AddValueToResourceNode_handles_multiple)
{

    TreeNode objectsNode = ObjectsTree_New();
    TreeNode resourcesNode;
    EXPECT_EQ(InternalError_Success, ObjectsTree_AddPath(objectsNode, "/3/0/1", &resourcesNode));

    for(int i = 1; i < 100; i++)
    {
        ASSERT_EQ(InternalError_Success, SetWriteCommon_AddValueToResourceNode(resourcesNode, i, "Lightweight M2M Server"));
    }

    ObjectsTree_Free(objectsNode);
}

TEST_F(TestSetWriteCommon, SetWriteCommon_AddValueToResourceNode_handles_null_node)
{
    EXPECT_EQ(InternalError_ParameterInvalid, SetWriteCommon_AddValueToResourceNode(NULL, 0, NULL));
}

TEST_F(TestSetWriteCommon, SetWriteCommon_AddValueToResourceNode_handles_null_value)
{
    TreeNode objectsNode = ObjectsTree_New();
    TreeNode resourcesNode;
    ASSERT_EQ(InternalError_Success, ObjectsTree_AddPath(objectsNode, "/3/0/1", &resourcesNode));
    EXPECT_EQ(InternalError_ParameterInvalid, SetWriteCommon_AddValueToResourceNode(resourcesNode, 0, NULL));
    ObjectsTree_Free(objectsNode);
}

TEST_F(TestSetWriteCommon, SetWriteCommon_AddValueToResourceNode_handles_non_property_node)
{
    TreeNode objectsNode = ObjectsTree_New();
    ASSERT_EQ(InternalError_ParameterInvalid, SetWriteCommon_AddValueToResourceNode(objectsNode, -1, NULL));
    ObjectsTree_Free(objectsNode);
}

TEST_F(TestSetWriteCommon, SetWriteCommon_AddValueToResourceNode_handles_invalid_resource_instance_id)
{
    TreeNode objectsNode = ObjectsTree_New();
    TreeNode resourcesNode;
    ASSERT_EQ(InternalError_Success, ObjectsTree_AddPath(objectsNode, "/3/0/1", &resourcesNode));
    EXPECT_EQ(InternalError_ParameterInvalid, SetWriteCommon_AddValueToResourceNode(resourcesNode, -1, "Lightweight M2M Server"));
    ObjectsTree_Free(objectsNode);
}

TEST_F(TestSetWriteCommon, SetWriteCommon_AddValueToResourceNode_handles_existing_id)
{

    TreeNode objectsNode = ObjectsTree_New();
    TreeNode resourcesNode;
    EXPECT_EQ(InternalError_Success, ObjectsTree_AddPath(objectsNode, "/3/0/1", &resourcesNode));

    ASSERT_EQ(InternalError_Success, SetWriteCommon_AddValueToResourceNode(resourcesNode, 0, "Lightweight M2M Server"));
    ASSERT_EQ(InternalError_Tree, SetWriteCommon_AddValueToResourceNode(resourcesNode, 0, "Lightweight M2M Server"));

    ObjectsTree_Free(objectsNode);
}

TEST_F(TestSetWriteCommon, SetWriteCommon_AddValueToResourceNode_handles_valid_input)
{
    const char * expectedXML =
                "<Objects>"
                "  <Object>"
                "    <ID>3</ID>"
                "    <ObjectInstance>"
                "      <ID>0</ID>"
                "      <Resource>"
                "        <ID>1</ID>"
                "          <ResourceInstance>"
                "            <ID>0</ID>"
                "            <Value>Lightweight M2M Server</Value>"
                "          </ResourceInstance>"
                "      </Resource>"
                "    </ObjectInstance>"
                "  </Object>"
                "</Objects>";

    TreeNode objectsNode = ObjectsTree_New();
    TreeNode resourcesNode;
    ASSERT_EQ(InternalError_Success, ObjectsTree_AddPath(objectsNode, "/3/0/1", &resourcesNode));
    EXPECT_EQ(InternalError_Success, SetWriteCommon_AddValueToResourceNode(resourcesNode, 0, "Lightweight M2M Server"));
    EXPECT_STREQ(XmlExpected(expectedXML).get(), XmlSerialise(objectsNode).get());
    ObjectsTree_Free(objectsNode);
}

} // namespace Awa


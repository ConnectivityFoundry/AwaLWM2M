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


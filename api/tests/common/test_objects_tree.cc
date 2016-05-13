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

#include "xmltree.h"
#include "lwm2m_xml_serdes.h"
#include "xml.h"

namespace Awa {

class TestObjectsTree : public TestClientBase {};

TEST_F(TestObjectsTree, ObjectsTree_New_Free)
{
    TreeNode objectsNode = ObjectsTree_New();
    ASSERT_TRUE(NULL != objectsNode);
    ObjectsTree_Free(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_Free_handles_null)
{
    ObjectsTree_Free(NULL);
}

TEST_F(TestObjectsTree, ObjectsTree_FindPathNode_handles_null_node)
{
    TreeNode resultNode = (TreeNode)12345;
    ASSERT_EQ(InternalError_ParameterInvalid, ObjectsTree_FindPathNode(NULL, "/3/2/1", &resultNode));
    EXPECT_EQ(NULL, resultNode);
}

TEST_F(TestObjectsTree, ObjectsTree_FindPathNode_handles_null_path)
{
    TreeNode objectsNode = ObjectsTree_New();
    TreeNode resultNode = (TreeNode)12345;
    ASSERT_EQ(InternalError_ParameterInvalid, ObjectsTree_FindPathNode(objectsNode, NULL, &resultNode));
    EXPECT_EQ(NULL, resultNode);
    ObjectsTree_Free(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_FindPathNode_handles_invalid_objects_node)
{
    TreeNode node = Xml_CreateNode("NotObjects");
    ASSERT_EQ(InternalError_ParameterInvalid, ObjectsTree_FindPathNode(node, "/1/2/3", NULL));
    TreeNode_DeleteSingle(node);
}

TEST_F(TestObjectsTree, ObjectsTree_FindPathNode_handles_non_matching_path)
{
    // first, create a suitable tree using ObjectsTree_CreateXMLObjectTree
    TreeNode objectsNode = ObjectsTree_New();
    ASSERT_EQ(InternalError_Success, ObjectsTree_AddPath(objectsNode, "/3/2/1", NULL));

    TreeNode resultNode = (TreeNode)12345;
    ASSERT_EQ(InternalError_Tree, ObjectsTree_FindPathNode(objectsNode, "/4/5/6", &resultNode));
    EXPECT_EQ(NULL, resultNode);
    ObjectsTree_Free(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_FindPathNode_handles_matching_path)
{
    TreeNode objectsNode = ObjectsTree_New();
    ASSERT_EQ(InternalError_Success, ObjectsTree_AddPath(objectsNode, "/3/2/1", NULL));

    TreeNode resultNode = (TreeNode)12345;
    ASSERT_EQ(InternalError_Success, ObjectsTree_FindPathNode(objectsNode, "/3/2/1", &resultNode));
    EXPECT_TRUE(NULL != resultNode);
    ObjectsTree_Free(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_FindPathNode_handles_matching_path_simple1)
{
    const char * xml =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>2</ID>"
            "      <Resource>"
            "        <ID>1</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != objectsNode);

    TreeNode resultNode = (TreeNode)12345;
    ASSERT_EQ(InternalError_Success, ObjectsTree_FindPathNode(objectsNode, "/3/2/1", &resultNode));
    EXPECT_TRUE(NULL != resultNode);
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_FindPathNode_handles_matching_path_multiple_objects)
{
    const char * xml =
            "<Objects>"
            "  <Object>"
            "    <ID>2</ID>"
            "    <ObjectInstance>"
            "      <ID>4</ID>"
            "      <Resource>"
            "        <ID>8</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>2</ID>"
            "      <Resource>"
            "        <ID>1</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != objectsNode);

    TreeNode resultNode = (TreeNode)12345;
    ASSERT_EQ(InternalError_Success, ObjectsTree_FindPathNode(objectsNode, "/3/2/1", &resultNode));
    EXPECT_TRUE(NULL != resultNode);
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_FindPathNode_handles_matching_path_with_instances_singular)
{
    const char * xml =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>2</ID>"
            "      <Resource>"
            "        <ID>1</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != objectsNode);

    TreeNode resultNode = (TreeNode)12345;
    ASSERT_EQ(InternalError_Success, ObjectsTree_FindPathNode(objectsNode, "/3/2/1", &resultNode));
    EXPECT_TRUE(NULL != resultNode);
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_FindPathNode_handles_matching_path_with_instances)
{
    const char * xml =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>1</ID>"
            "      <Resource>"
            "        <ID>4</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "    <ObjectInstance>"
            "      <ID>2</ID>"
            "      <Resource>"
            "        <ID>1</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "    <ObjectInstance>"
            "      <ID>3</ID>"
            "      <Resource>"
            "        <ID>12</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != objectsNode);

    TreeNode resultNode = (TreeNode)12345;
    ASSERT_EQ(InternalError_Success, ObjectsTree_FindPathNode(objectsNode, "/3/2/1", &resultNode));
    EXPECT_TRUE(NULL != resultNode);
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_FindPathNode_handles_matching_path_with_properties_singular)
{
    const char * xml =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>2</ID>"
            "      <Resource>"
            "        <ID>7</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != objectsNode);

    TreeNode resultNode = (TreeNode)12345;
    ASSERT_EQ(InternalError_Success, ObjectsTree_FindPathNode(objectsNode, "/3/2/7", &resultNode));
    EXPECT_TRUE(NULL != resultNode);
    EXPECT_EQ(7, xmlif_GetInteger(resultNode, "Resource/ID"));
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_FindPathNode_handles_matching_path_with_properties)
{
    const char * xml =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "      <ObjectInstance>"
            "      <ID>2</ID>"
            "        <Resource>"
            "          <ID>4</ID>"
            "        </Resource>"
            "        <Resource>"
            "          <ID>3</ID>"
            "        </Resource>"
            "        <Resource>"
            "          <ID>1</ID>"
            "        </Resource>"
            "        <Resource>"
            "          <ID>2</ID>"
            "        </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != objectsNode);

    TreeNode resultNode = (TreeNode)12345;
    ASSERT_EQ(InternalError_Success, ObjectsTree_FindPathNode(objectsNode, "/3/2/1", &resultNode));
    EXPECT_TRUE(NULL != resultNode);
    EXPECT_EQ(1, xmlif_GetInteger(resultNode, "Resource/ID"));
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_FindPathNode_handles_matching_path_complex)
{
    const char * xml =
            "<Objects>"
            "  <Object>"
            "    <ID>2</ID>"
            "    <ObjectInstance>"
            "      <ID>0</ID>"
            "      <Resource>"
            "        <ID>1</ID>"
            "      </Resource>"
            "      <Resource>"
            "        <ID>2</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "    <ObjectInstance>"
            "      <ID>1</ID>"
            "      <Resource>"
            "        <ID>0</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>2</ID>"
            "      <Resource>"
            "        <ID>4</ID>"
            "      </Resource>"
            "      <Resource>"
            "        <ID>3</ID>"
            "      </Resource>"
            "      <Resource>"
            "        <ID>1</ID>"
            "      </Resource>"
            "      <Resource>"
            "        <ID>2</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != objectsNode);

    TreeNode resultNode = (TreeNode)12345;
    ASSERT_EQ(InternalError_Success, ObjectsTree_FindPathNode(objectsNode, "/2/0/1", &resultNode));
    EXPECT_TRUE(NULL != resultNode); EXPECT_EQ(1, xmlif_GetInteger(resultNode, "Resource/ID"));

    ASSERT_EQ(InternalError_Success, ObjectsTree_FindPathNode(objectsNode, "/2/1/0", &resultNode));
    EXPECT_TRUE(NULL != resultNode); EXPECT_EQ(0, xmlif_GetInteger(resultNode, "Resource/ID"));

    ASSERT_EQ(InternalError_Success, ObjectsTree_FindPathNode(objectsNode, "/3/2", &resultNode));
    TreeNode property = TreeNode_Navigate(resultNode, "Instance/Resource");   ASSERT_TRUE(NULL != property);

    Tree_Delete(objectsNode);
}


TEST_F(TestObjectsTree, ObjectsTree_ContainsPath_handles_valid_inputs)
{
    const char * xml =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>2</ID>"
            "      <Resource>"
            "        <ID>7</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != objectsNode);

    ASSERT_TRUE(ObjectsTree_ContainsPath(objectsNode, "/3/2/7"));
    ASSERT_TRUE(ObjectsTree_ContainsPath(objectsNode, "/3/2"));
    ASSERT_TRUE(ObjectsTree_ContainsPath(objectsNode, "/3"));
    ASSERT_FALSE(ObjectsTree_ContainsPath(objectsNode, "/3/2/8"));
    ASSERT_FALSE(ObjectsTree_ContainsPath(objectsNode, "/3/3"));
    ASSERT_FALSE(ObjectsTree_ContainsPath(objectsNode, "/2"));
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_ContainsPath_handles_invalid_inputs)
{
    TreeNode objectsNode = ObjectsTree_New();
    ASSERT_FALSE(ObjectsTree_ContainsPath(NULL, "/2"));
    ASSERT_FALSE(ObjectsTree_ContainsPath(objectsNode, NULL));
    Tree_Delete(objectsNode);
}


TEST_F(TestObjectsTree, ObjectsTree_AddPath_handles_null_node)
{
    EXPECT_EQ(InternalError_ParameterInvalid, ObjectsTree_AddPath(NULL, "/3/2/1", NULL));
}

TEST_F(TestObjectsTree, ObjectsTree_AddPath_handles_invalid_objects_node)
{
    TreeNode node = Xml_CreateNode("NotObjects");
    EXPECT_EQ(InternalError_ParameterInvalid, ObjectsTree_AddPath(node, "/3/2/1", NULL));
    Tree_Delete(node);
}

TEST_F(TestObjectsTree, ObjectsTree_AddPath_handles_null_path)
{
    TreeNode objectsNode = ObjectsTree_New();
    EXPECT_EQ(InternalError_ParameterInvalid, ObjectsTree_AddPath(objectsNode, NULL, NULL));
    ObjectsTree_Free(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_AddPath_handles_invalid_path)
{
    TreeNode objectsNode = ObjectsTree_New();
    EXPECT_EQ(InternalError_ParameterInvalid, ObjectsTree_AddPath(objectsNode, "/an/invalid/path", NULL));
    ObjectsTree_Free(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_AddPath_handles_object_id_out_of_range)
{
    TreeNode objectsNode = ObjectsTree_New();
    EXPECT_EQ(InternalError_ParameterInvalid, ObjectsTree_AddPath(objectsNode, "/999999/0/0", NULL));
    ObjectsTree_Free(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_AddPath_handles_negative_object_id)
{
    TreeNode objectsNode = ObjectsTree_New();
    EXPECT_EQ(InternalError_ParameterInvalid, ObjectsTree_AddPath(objectsNode, "/-1/0/0", NULL));
    ObjectsTree_Free(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_AddPath_handles_object_instance_id_out_of_range)
{
    TreeNode objectsNode = ObjectsTree_New();
    EXPECT_EQ(InternalError_ParameterInvalid, ObjectsTree_AddPath(objectsNode, "/0/999999/0", NULL));
    ObjectsTree_Free(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_AddPath_handles_negative_object_instance_id)
{
    TreeNode objectsNode = ObjectsTree_New();
    EXPECT_EQ(InternalError_ParameterInvalid, ObjectsTree_AddPath(objectsNode, "/0/-1/0", NULL));
    ObjectsTree_Free(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_AddPath_handles_resource_id_out_of_range)
{
    TreeNode objectsNode = ObjectsTree_New();
    EXPECT_EQ(InternalError_ParameterInvalid, ObjectsTree_AddPath(objectsNode, "/0/0/999999", NULL));
    ObjectsTree_Free(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_AddPath_handles_negative_resource_id)
{
    TreeNode objectsNode = ObjectsTree_New();
    EXPECT_EQ(InternalError_ParameterInvalid, ObjectsTree_AddPath(objectsNode, "/0/0/-1", NULL));
    ObjectsTree_Free(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_AddPath_handles_empty_objectsNode)
{
    TreeNode objectsNode = ObjectsTree_New();
    TreeNode resultNode = NULL;
    EXPECT_EQ(InternalError_Success, ObjectsTree_AddPath(objectsNode, "/3/2/1", &resultNode));

    const char * expectedXML =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>2</ID>"
            "      <Resource>"
            "        <ID>1</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    EXPECT_STREQ(XmlExpected(expectedXML).get(), XmlSerialise(objectsNode).get());
    ASSERT_TRUE(NULL != resultNode);
    EXPECT_STREQ("Resource", TreeNode_GetName(resultNode));
    EXPECT_EQ(1, xmlif_GetInteger(resultNode, "Resource/ID"));
    ObjectsTree_Free(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_AddPath_handles_existing_objectsNode_with_non_matching_object)
{
    const char * initialXML =
            "<Objects>"
            "  <Object>"
            "    <ID>2</ID>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);

    TreeNode resultNode = NULL;
    EXPECT_EQ(InternalError_Success, ObjectsTree_AddPath(objectsNode, "/3/2/11", &resultNode));

    const char * expectedXML =
            "<Objects>"
            "  <Object>"
            "    <ID>2</ID>"
            "  </Object>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>2</ID>"
            "      <Resource>"
            "        <ID>11</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    EXPECT_STREQ(XmlExpected(expectedXML).get(), XmlSerialise(objectsNode).get());
    ASSERT_TRUE(NULL != resultNode);
    EXPECT_STREQ("Resource", TreeNode_GetName(resultNode));
    EXPECT_EQ(11, xmlif_GetInteger(resultNode, "Resource/ID"));
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_AddPath_handles_existing_objectsNode_with_matching_object)
{
    const char * initialXML =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);

    TreeNode resultNode = NULL;
    EXPECT_EQ(InternalError_Success, ObjectsTree_AddPath(objectsNode, "/3/2/9", &resultNode));

    const char * expectedXML =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>2</ID>"
            "      <Resource>"
            "        <ID>9</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    EXPECT_STREQ(XmlExpected(expectedXML).get(), XmlSerialise(objectsNode).get());
    ASSERT_TRUE(NULL != resultNode);
    EXPECT_STREQ("Resource", TreeNode_GetName(resultNode));
    EXPECT_EQ(9, xmlif_GetInteger(resultNode, "Resource/ID"));
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_AddPath_handles_existing_objectsNode_with_matching_object_and_instance)
{
    const char * initialXML =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>0</ID>"
            "      <Resource>"
            "        <ID>0</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);

    TreeNode resultNode = NULL;
    EXPECT_EQ(InternalError_Success, ObjectsTree_AddPath(objectsNode, "/3/2/7", &resultNode));

    const char * expectedXML =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>0</ID>"
            "      <Resource>"
            "        <ID>0</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "    <ObjectInstance>"
            "      <ID>2</ID>"
            "      <Resource>"
            "        <ID>7</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    EXPECT_STREQ(XmlExpected(expectedXML).get(), XmlSerialise(objectsNode).get());
    ASSERT_TRUE(NULL != resultNode);
    EXPECT_STREQ("Resource", TreeNode_GetName(resultNode));
    EXPECT_EQ(7, xmlif_GetInteger(resultNode, "Resource/ID"));
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_AddPath_handles_existing_objectsNode_with_matching_property)
{
    // expecting no change

    const char * initialXML =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>2</ID>"
            "      <Resource>"
            "        <ID>1</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);

    TreeNode resultNode = NULL;
    EXPECT_EQ(InternalError_Success, ObjectsTree_AddPath(objectsNode, "/3/2/1", &resultNode));

    const char * expectedXML =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>2</ID>"
            "      <Resource>"
            "        <ID>1</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    EXPECT_STREQ(XmlExpected(expectedXML).get(), XmlSerialise(objectsNode).get());
    ASSERT_TRUE(NULL != resultNode);
    EXPECT_STREQ("Resource", TreeNode_GetName(resultNode));
    EXPECT_EQ(1, xmlif_GetInteger(resultNode, "Resource/ID"));
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_AddPath_handles_existing_objectsNode_with_matching_object_instance)
{
    const char * initialXML =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>2</ID>"
            "      <Resource>"
            "        <ID>0</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);

    TreeNode resultNode = NULL;
    EXPECT_EQ(InternalError_Success, ObjectsTree_AddPath(objectsNode, "/3/2/14", &resultNode));

    const char * expectedXML =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>2</ID>"
            "      <Resource>"
            "        <ID>0</ID>"
            "      </Resource>"
            "      <Resource>"
            "        <ID>14</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    EXPECT_STREQ(XmlExpected(expectedXML).get(), XmlSerialise(objectsNode).get());
    ASSERT_TRUE(NULL != resultNode);
    EXPECT_STREQ("Resource", TreeNode_GetName(resultNode));
    EXPECT_EQ(14, xmlif_GetInteger(resultNode, "Resource/ID"));
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_AddPath_handles_adding_resource_to_existing_object_instance)
{
    const char * initialXML =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>0</ID>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);

    TreeNode resultNode = NULL;
    EXPECT_EQ(InternalError_Success, ObjectsTree_AddPath(objectsNode, "/3/0/0", &resultNode));

    const char * expectedXML =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>0</ID>"
            "      <Resource>"
            "        <ID>0</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    EXPECT_STREQ(XmlExpected(expectedXML).get(), XmlSerialise(objectsNode).get());
    ASSERT_TRUE(NULL != resultNode);
    EXPECT_STREQ("Resource", TreeNode_GetName(resultNode));
    EXPECT_EQ(0, xmlif_GetInteger(resultNode, "Resource/ID"));
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_AddPath_handles_adding_instance_to_matching_tree)
{
    const char * initialXML =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>8</ID>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);

    TreeNode resultNode = NULL;
    EXPECT_EQ(InternalError_Success, ObjectsTree_AddPath(objectsNode, "/3/8", &resultNode));

    const char * expectedXML =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>8</ID>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    EXPECT_STREQ(XmlExpected(expectedXML).get(), XmlSerialise(objectsNode).get());
    ASSERT_TRUE(NULL != resultNode);
    EXPECT_STREQ("ObjectInstance", TreeNode_GetName(resultNode));
    EXPECT_EQ(8, xmlif_GetInteger(resultNode, "ObjectInstance/ID"));
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_AddPath_handles_adding_instance_to_non_matching_tree)
{
    const char * initialXML =
            "<Objects>"
            "  <Object>"
            "    <ID>4</ID>"
            "    <ObjectInstance>"
            "      <ID>7</ID>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);

    TreeNode resultNode = NULL;
    EXPECT_EQ(InternalError_Success, ObjectsTree_AddPath(objectsNode, "/3/8", &resultNode));

    const char * expectedXML =
            "<Objects>"
            "  <Object>"
            "    <ID>4</ID>"
            "    <ObjectInstance>"
            "      <ID>7</ID>"
            "    </ObjectInstance>"
            "  </Object>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>8</ID>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    EXPECT_STREQ(XmlExpected(expectedXML).get(), XmlSerialise(objectsNode).get());
    ASSERT_TRUE(NULL != resultNode);
    EXPECT_STREQ("ObjectInstance", TreeNode_GetName(resultNode));
    EXPECT_EQ(8, xmlif_GetInteger(resultNode, "ObjectInstance/ID"));
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_AddPath_handles_adding_instance_to_partially_matching_tree)
{
    const char * initialXML =
            "<Objects>"
            "  <Object>"
            "    <ID>4</ID>"
            "    <ObjectInstance>"
            "      <ID>7</ID>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);

    TreeNode resultNode = NULL;
    EXPECT_EQ(InternalError_Success, ObjectsTree_AddPath(objectsNode, "/4/8", &resultNode));

    const char * expectedXML =
            "<Objects>"
            "  <Object>"
            "    <ID>4</ID>"
            "    <ObjectInstance>"
            "      <ID>7</ID>"
            "    </ObjectInstance>"
            "    <ObjectInstance>"
            "      <ID>8</ID>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    EXPECT_STREQ(XmlExpected(expectedXML).get(), XmlSerialise(objectsNode).get());
    ASSERT_TRUE(NULL != resultNode);
    EXPECT_STREQ("ObjectInstance", TreeNode_GetName(resultNode));
    EXPECT_EQ(8, xmlif_GetInteger(resultNode, "ObjectInstance/ID"));
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_IsPathCovered_handles_null_objectsNode)
{
    EXPECT_TRUE(false == ObjectsTree_IsPathCovered(NULL, "/3/0/0", NULL));
}

TEST_F(TestObjectsTree, ObjectsTree_IsPathCovered_handles_invalid_objectsNode)
{
    TreeNode node = Xml_CreateNode("NotObjects");
    EXPECT_TRUE(false == ObjectsTree_IsPathCovered(node, "/3/0/0", NULL));
    Tree_Delete(node);
}

TEST_F(TestObjectsTree, ObjectsTree_IsPathCovered_handles_null_path)
{
    TreeNode objectsNode = ObjectsTree_New();
    EXPECT_TRUE(false == ObjectsTree_IsPathCovered(objectsNode, NULL, NULL));
    ObjectsTree_Free(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_IsPathCovered_handles_invalid_path)
{
    TreeNode objectsNode = ObjectsTree_New();
    EXPECT_TRUE(false == ObjectsTree_IsPathCovered(objectsNode, "/an/invalid/path", NULL));
    ObjectsTree_Free(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_IsPathCovered_handles_new_object)
{
    const char * initialXML =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);

    EXPECT_FALSE(ObjectsTree_IsPathCovered(objectsNode, "/4", NULL));
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_IsPathCovered_handles_new_object_instance)
{
    const char * initialXML =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>0</ID>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);

    EXPECT_FALSE(ObjectsTree_IsPathCovered(objectsNode, "/3/1", NULL));
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_IsPathCovered_handles_identical_path)
{
    const char * initialXML =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>0</ID>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);

    EXPECT_TRUE(ObjectsTree_IsPathCovered(objectsNode, "/3/0", NULL));
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_IsPathCovered_handles_shadowed_path)
{
    const char * initialXML =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>0</ID>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);

    EXPECT_TRUE(ObjectsTree_IsPathCovered(objectsNode, "/3/0/0", NULL));
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_RemovePathNodes_handles_null)
{
    ASSERT_EQ(static_cast<size_t>(0), ObjectsTree_RemovePathNodes(NULL));
}

TEST_F(TestObjectsTree, ObjectsTree_RemovePathNodes_handles_object_with_single_instance)
{
    const char * initialXML =
            "<Object>"
            "  <ID>3</ID>"
            "  <ObjectInstance>"
            "    <ID>0</ID>"
            "  </ObjectInstance>"
            "</Object>";

    TreeNode objectNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);
    EXPECT_EQ(static_cast<size_t>(1), ObjectsTree_RemovePathNodes(objectNode));

    const char * expectedXML =
            "<Object>"
            "  <ID>3</ID>"
            "</Object>";

    EXPECT_STREQ(XmlExpected(expectedXML).get(), XmlSerialise(objectNode).get());
    Tree_Delete(objectNode);
}

TEST_F(TestObjectsTree, ObjectsTree_RemovePathNodes_handles_object_with_multiple_instances)
{
    const char * initialXML =
            "<Object>"
            "  <ID>3</ID>"
            "  <ObjectInstance>"
            "    <ID>0</ID>"
            "  </ObjectInstance>"
            "  <ObjectInstance>"
            "    <ID>1</ID>"
            "  </ObjectInstance>"
            "</Object>";

    TreeNode objectNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);
    EXPECT_EQ(static_cast<size_t>(2), ObjectsTree_RemovePathNodes(objectNode));

    const char * expectedXML =
            "<Object>"
            "  <ID>3</ID>"
            "</Object>";

    EXPECT_STREQ(XmlExpected(expectedXML).get(), XmlSerialise(objectNode).get());
    Tree_Delete(objectNode);
}

TEST_F(TestObjectsTree, ObjectsTree_RemovePathNodes_handles_object_with_instances)
{
    const char * initialXML =
            "<Object>"
            "  <ID>3</ID>"
            "  <ObjectInstance>"
            "    <ID>0</ID>"
            "  </ObjectInstance>"
            "  <ObjectInstance>"
            "    <ID>1</ID>"
            "  </ObjectInstance>"
            "</Object>";

    TreeNode objectNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);
    EXPECT_EQ(static_cast<size_t>(2), ObjectsTree_RemovePathNodes(objectNode));

    const char * expectedXML =
            "<Object>"
            "  <ID>3</ID>"
            "</Object>";

    EXPECT_STREQ(XmlExpected(expectedXML).get(), XmlSerialise(objectNode).get());
    Tree_Delete(objectNode);
}

TEST_F(TestObjectsTree, ObjectsTree_RemovePathNodes_handles_instance_with_single_property)
{
    const char * initialXML =
            "<ObjectInstance>"
            "  <ID>7</ID>"
            "  <Resource>"
            "    <ID>42</Resource>"
            "  </Resource>"
            "</ObjectInstance>";

    TreeNode instanceNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);
    EXPECT_EQ(static_cast<size_t>(1), ObjectsTree_RemovePathNodes(instanceNode));

    const char * expectedXML =
            "<ObjectInstance>"
            "  <ID>7</ID>"
            "</ObjectInstance>";

    EXPECT_STREQ(XmlExpected(expectedXML).get(), XmlSerialise(instanceNode).get());
    Tree_Delete(instanceNode);
}

TEST_F(TestObjectsTree, ObjectsTree_RemovePathNodes_handles_instance_with_properties)
{
    const char * initialXML =
            "<ObjectInstance>"
            "  <ID>7</ID>"
            "  <Resource>"
            "    <ID>42</Resource>"
            "  </Resource>"
            "  <Resource>"
            "    <ID>52</Resource>"
            "  </Resource>"
            "  <Resource>"
            "    <ID>62</Resource>"
            "  </Resource>"
            "  <Resource>"
            "    <ID>63</Resource>"
            "  </Resource>"
            "</ObjectInstance>";

    TreeNode instanceNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);
    EXPECT_EQ(static_cast<size_t>(4), ObjectsTree_RemovePathNodes(instanceNode));

    const char * expectedXML =
            "<ObjectInstance>"
            "  <ID>7</ID>"
            "</ObjectInstance>";

    EXPECT_STREQ(XmlExpected(expectedXML).get(), XmlSerialise(instanceNode).get());
    Tree_Delete(instanceNode);
}

TEST_F(TestObjectsTree, ObjectsTree_GetNextLeafNode_handles_null_node)
{
    EXPECT_EQ(NULL, ObjectsTree_GetNextLeafNode(NULL));
}

TEST_F(TestObjectsTree, ObjectsTree_GetNextLeafNode_handles_invalid_node)
{
    const char * initialXML =
            "<NotObjects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>0</ID>"
            "    </ObjectInstance>"
            "  </Object>"
            "</NotObjects>";

    TreeNode objectsNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);
    EXPECT_EQ(NULL, ObjectsTree_GetNextLeafNode(objectsNode));
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_GetNextLeafNode_handles_valid_objects_node_single_resource)
{
    // should return the first path
    const char * initialXML =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>0</ID>"
            "      <Resource>"
            "        <ID>4</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);

    //look for resource /3/0/4
    TreeNode nextNode = ObjectsTree_GetNextLeafNode(objectsNode);
    ASSERT_TRUE(NULL != nextNode);
    EXPECT_STREQ("Resource", TreeNode_GetName(nextNode));
    EXPECT_EQ(4, xmlif_GetInteger(nextNode, "Resource/ID"));

    //should be no more leaves
    ASSERT_TRUE(NULL == ObjectsTree_GetNextLeafNode(nextNode));
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_GetNextLeafNode_handles_valid_objects_node_multiple_resources)
{
    const char * initialXML =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>0</ID>"
            "      <Resource>"
            "        <ID>1</ID>"
            "      </Resource>"
            "      <Resource>"
            "        <ID>4</ID>"
            "      </Resource>"
            "      <Resource>"
            "        <ID>6</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);

    //look for resource /3/0/1
    TreeNode nextNode = ObjectsTree_GetNextLeafNode(objectsNode);
    ASSERT_TRUE(NULL != nextNode);
    EXPECT_STREQ("Resource", TreeNode_GetName(nextNode));
    EXPECT_EQ(1, xmlif_GetInteger(nextNode, "Resource/ID"));

    //look for resource /3/0/4
    nextNode = ObjectsTree_GetNextLeafNode(nextNode);
    ASSERT_TRUE(NULL != nextNode);
    EXPECT_STREQ("Resource", TreeNode_GetName(nextNode));
    EXPECT_EQ(4, xmlif_GetInteger(nextNode, "Resource/ID"));

    //look for resource /3/0/6
    nextNode = ObjectsTree_GetNextLeafNode(nextNode);
    ASSERT_TRUE(NULL != nextNode);
    EXPECT_STREQ("Resource", TreeNode_GetName(nextNode));
    EXPECT_EQ(6, xmlif_GetInteger(nextNode, "Resource/ID"));

    //should be no more leaves
    ASSERT_TRUE(NULL == ObjectsTree_GetNextLeafNode(nextNode));
    Tree_Delete(objectsNode);
}


TEST_F(TestObjectsTree, ObjectsTree_GetNextLeafNode_handles_object_instance_and_resource_leaves)
{
    const char * initialXML =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>0</ID>"
            "      <Resource>"
            "        <ID>1</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "    <ObjectInstance>"
            "      <ID>1</ID>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);

    //look for resource /3/0/1
    TreeNode nextNode = ObjectsTree_GetNextLeafNode(objectsNode);
    ASSERT_TRUE(NULL != nextNode);
    EXPECT_STREQ("Resource", TreeNode_GetName(nextNode));
    EXPECT_EQ(1, xmlif_GetInteger(nextNode, "Resource/ID"));

    //look for object instance /3/1
    nextNode = ObjectsTree_GetNextLeafNode(nextNode);
    ASSERT_TRUE(NULL != nextNode);
    EXPECT_STREQ("ObjectInstance", TreeNode_GetName(nextNode));
    EXPECT_EQ(1, xmlif_GetInteger(nextNode, "ObjectInstance/ID"));

    //should be no more leaves
    ASSERT_TRUE(NULL == ObjectsTree_GetNextLeafNode(nextNode));
    Tree_Delete(objectsNode);
}


TEST_F(TestObjectsTree, ObjectsTree_GetNextLeafNode_handles_object_leaves)
{
    const char * initialXML =
            "<Objects>"
            "  <Object>"
            "    <ID>2</ID>"
            "  </Object>"
            "  <Object>"
            "    <ObjectInstance>"
            "      <ID>0</ID>"
            "      <Resource>"
            "        <ID>1</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "    <ObjectInstance>"
            "      <ID>1</ID>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);

    //look for object /2
    TreeNode nextNode = ObjectsTree_GetNextLeafNode(objectsNode);
    ASSERT_TRUE(NULL != nextNode);
    EXPECT_STREQ("Object", TreeNode_GetName(nextNode));
    EXPECT_EQ(2, xmlif_GetInteger(nextNode, "Object/ID"));

    //look for resource /3/0/1
    nextNode = ObjectsTree_GetNextLeafNode(nextNode);
    ASSERT_TRUE(NULL != nextNode);
    EXPECT_STREQ("Resource", TreeNode_GetName(nextNode));
    EXPECT_EQ(1, xmlif_GetInteger(nextNode, "Resource/ID"));

    //look for object instance /3/1
    nextNode = ObjectsTree_GetNextLeafNode(nextNode);
    ASSERT_TRUE(NULL != nextNode);
    EXPECT_STREQ("ObjectInstance", TreeNode_GetName(nextNode));
    EXPECT_EQ(1, xmlif_GetInteger(nextNode, "ObjectInstance/ID"));

    //should be no more leaves
    ASSERT_TRUE(NULL == ObjectsTree_GetNextLeafNode(nextNode));
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_GetNextLeafNode_handles_valid_large_request)
{
    const char * initialXML =
    "<Objects>"
      "<Object>"
       "<ID>0</ID>"
       "<ObjectInstance>"
        "<ID>1</ID>"
       "</ObjectInstance>"
      "</Object>"
      "<Object>"
       "<ID>1</ID>"
       "<ObjectInstance>"
        "<ID>1</ID>"
       "</ObjectInstance>"
      "</Object>"
      "<Object>"
       "<ID>2</ID>"
       "<ObjectInstance>"
        "<ID>0</ID>"
       "</ObjectInstance>"
       "<ObjectInstance>"
        "<ID>1</ID>"
       "</ObjectInstance>"
       "<ObjectInstance>"
        "<ID>2</ID>"
       "</ObjectInstance>"
       "<ObjectInstance>"
        "<ID>3</ID>"
       "</ObjectInstance>"
      "</Object>"
      "<Object>"
       "<ID>3</ID>"
       "<ObjectInstance>"
       "<ID>0</ID>"
       "</ObjectInstance>"
      "</Object>"
      "<Object>"
       "<ID>4</ID>"
       "<ObjectInstance>"
        "<ID>0</ID>"
       "</ObjectInstance>"
      "</Object>"
      "<Object>"
       "<ID>7</ID>"
       "<ObjectInstance>"
        "<ID>0</ID>"
       "</ObjectInstance>"
      "</Object>"
      "<Object>"
       "<ID>5</ID>"
      "</Object>"
      "<Object>"
       "<ID>6</ID>"
      "</Object>"
     "</Objects>";

    std::vector<const char *> expectedPaths;
    expectedPaths.push_back("/0/1");
    expectedPaths.push_back("/1/1");
    expectedPaths.push_back("/2/0");
    expectedPaths.push_back("/2/1");
    expectedPaths.push_back("/2/2");
    expectedPaths.push_back("/2/3");
    expectedPaths.push_back("/3/0");
    expectedPaths.push_back("/4/0");
    expectedPaths.push_back("/7/0");
    expectedPaths.push_back("/5");
    expectedPaths.push_back("/6");

    TreeNode objectsNode = TreeNode_ParseXML((unsigned char *)initialXML, strlen(initialXML), true);
    TreeNode currentNode = objectsNode;
    for (std::vector<const char *>::iterator iterator = expectedPaths.begin() ; iterator != expectedPaths.end(); ++iterator)
    {
        //Lwm2m_Error("Checking %s\n", *iterator);
        currentNode = ObjectsTree_GetNextLeafNode(currentNode);
        ASSERT_TRUE(NULL != currentNode);
        char path[256];
        EXPECT_STREQ(*iterator, ObjectsTree_GetPath(currentNode, path, sizeof(path)));
    }

    //should be no more leaves
    ASSERT_TRUE(NULL == ObjectsTree_GetNextLeafNode(currentNode));
    Tree_Delete(objectsNode);
}


TEST_F(TestObjectsTree, ObjectsTree_ObjectsTree_GetNumChildrenWithName_handles_null_node)
{
    ASSERT_EQ(0u, ObjectsTree_GetNumChildrenWithName(NULL, "ChildName"));
}
TEST_F(TestObjectsTree, ObjectsTree_ObjectsTree_GetNumChildrenWithName_handles_null_name)
{
    TreeNode parent = TreeNode_Create();
    ASSERT_EQ(0u, ObjectsTree_GetNumChildrenWithName(NULL, NULL));
    Tree_Delete(parent);
}

TEST_F(TestObjectsTree, ObjectsTree_ObjectsTree_GetNumChildrenWithName_handles_valid_resource)
{
    const char * xml =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != objectsNode);

    TreeNode resultNode = (TreeNode)12345;
    ASSERT_EQ(1u, ObjectsTree_GetNumChildrenWithName(objectsNode, "Object"));
    EXPECT_TRUE(NULL != resultNode);
    Tree_Delete(objectsNode);
}

TEST_F(TestObjectsTree, ObjectsTree_ObjectsTree_GetNumChildrenWithName_handles_valid_multiple_resources)
{
    const char * xml =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "  </Object>"
            "  <Object>"
            "    <ID>4</ID>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != objectsNode);

    TreeNode resultNode = (TreeNode)12345;
    ASSERT_EQ(2u, ObjectsTree_GetNumChildrenWithName(objectsNode, "Object"));
    EXPECT_TRUE(NULL != resultNode);
    Tree_Delete(objectsNode);
}

} // namespace Awa

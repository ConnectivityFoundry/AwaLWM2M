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

#include <lwm2m_tree_node.h>
#include "../src/objects_tree.h"

#include "awa/client.h"
#include "awa/common.h"
#include "memalloc.h"
#include "log.h"
#include "arrays.h"
#include "support/support.h"
#include "client_subscribe.h"
#include "client_session.h"
#include "changeset.h"
#include "utils.h"

namespace Awa {

class TestClientChangeSet : public TestClientWithConnectedSession {};
class TestServerChangeSet : public TestServerWithConnectedSession {};

/*
    AwaError ChangeSet_GetExecuteArguments(const AwaChangeSet * changeSet, const char * path, AwaExecuteArguments * arguments);
 */


TEST_F(TestClientChangeSet, Client_AwaChangeSet_New_free_valid_inputs)
{
    TreeNode objectsTree = ObjectsTree_New();
    AwaChangeSet * changeSet = ChangeSet_New(session_, SessionType_Client, objectsTree);
    ASSERT_TRUE(NULL != changeSet);
    ASSERT_EQ(AwaError_Success, ChangeSet_Free(&changeSet));
    ASSERT_TRUE(NULL == changeSet);
    Tree_Delete(objectsTree);
}

TEST_F(TestClientChangeSet, Client_AwaChangeSet_New_free_invalid_inputs)
{
    TreeNode objectsTree = ObjectsTree_New();
    AwaChangeSet * changeSet = ChangeSet_New(session_, SessionType_Client, objectsTree);
    ASSERT_TRUE(NULL != changeSet);
    ASSERT_EQ(AwaError_Success, ChangeSet_Free(&changeSet));
    ASSERT_TRUE(NULL == changeSet);

    //ASSERT_TRUE(NULL == ChangeSet_New(session_, SessionType_Server, objectsTree)); // invalid session type
    ASSERT_TRUE(NULL == ChangeSet_New(session_, SessionType_Client, NULL)); // no objects tree
    ASSERT_TRUE(NULL == ChangeSet_New(NULL, SessionType_Client, objectsTree)); // invalid session

    ASSERT_EQ(AwaError_OperationInvalid, ChangeSet_Free(&changeSet));
    ASSERT_EQ(AwaError_OperationInvalid, ChangeSet_Free(NULL));
    Tree_Delete(objectsTree);
}

TEST_F(TestClientChangeSet, Client_AwaChangeSet_GetObjectsTree_valid_inputs)
{
    TreeNode objectsTree = ObjectsTree_New();
    AwaChangeSet * changeSet = ChangeSet_New(session_, SessionType_Client, objectsTree);
    ASSERT_TRUE(NULL != changeSet);
    ASSERT_TRUE(NULL != ChangeSet_GetObjectsTree(changeSet));

    char buffer[512];
    Xml_TreeToString(ChangeSet_GetObjectsTree(changeSet), buffer, sizeof(buffer));
    ASSERT_STREQ("<Objects></Objects>\n", buffer);
    ASSERT_EQ(AwaError_Success, ChangeSet_Free(&changeSet));
    ASSERT_TRUE(NULL == changeSet);
    Tree_Delete(objectsTree);
}

TEST_F(TestClientChangeSet, Client_AwaChangeSet_GetObjectsTree_invalid_inputs)
{
    AwaChangeSet * changeSet = NULL;
    ASSERT_EQ(NULL, ChangeSet_GetObjectsTree(changeSet));
    ASSERT_EQ(NULL, ChangeSet_GetObjectsTree(NULL));
}

TEST_F(TestClientChangeSet, Client_AwaChangeSet_GetClientSession_handles_valid_inputs)
{
    TreeNode objectsTree = ObjectsTree_New();
    AwaChangeSet * changeSet = ChangeSet_New(session_, SessionType_Client, objectsTree);
    ASSERT_TRUE(NULL != changeSet);
    EXPECT_EQ(session_, AwaChangeSet_GetClientSession(changeSet));
    ASSERT_EQ(AwaError_Success, ChangeSet_Free(&changeSet));
    ASSERT_TRUE(NULL == changeSet);
    Tree_Delete(objectsTree);
}
TEST_F(TestClientChangeSet, Client_AwaChangeSet_GetClientSession_handles_invalid_inputs)
{
    ASSERT_TRUE(NULL == AwaChangeSet_GetClientSession(NULL));
}


TEST_F(TestServerChangeSet, Server_AwaChangeSet_New_free_valid_inputs)
{
    TreeNode objectsTree = ObjectsTree_New();
    AwaChangeSet * changeSet = ChangeSet_NewWithClientID(session_, SessionType_Server, objectsTree, global::clientEndpointName);
    ASSERT_TRUE(NULL != changeSet);
    ASSERT_EQ(AwaError_Success, ChangeSet_Free(&changeSet));
    ASSERT_TRUE(NULL == changeSet);
    Tree_Delete(objectsTree);
}

TEST_F(TestServerChangeSet, Server_AwaChangeSet_New_free_invalid_inputs)
{
    TreeNode objectsTree = ObjectsTree_New();
    AwaChangeSet * changeSet = ChangeSet_NewWithClientID(session_, SessionType_Server, objectsTree, global::clientEndpointName);
    ASSERT_TRUE(NULL != changeSet);
    ASSERT_EQ(AwaError_Success, ChangeSet_Free(&changeSet));
    ASSERT_TRUE(NULL == changeSet);

    //ASSERT_TRUE(NULL == ChangeSet_NewWithClientID(session_, SessionType_Server, objectsTree, NULL)); // invalid clientID
    //ASSERT_TRUE(NULL == ChangeSet_NewWithClientID(session_, SessionType_Client, objectsTree, global::clientEndpointName)); // invalid session type
    ASSERT_TRUE(NULL == ChangeSet_NewWithClientID(session_, SessionType_Server, NULL, global::clientEndpointName)); // no objects tree
    ASSERT_TRUE(NULL == ChangeSet_NewWithClientID(NULL, SessionType_Server, objectsTree, global::clientEndpointName)); // invalid session

    ASSERT_EQ(AwaError_OperationInvalid, ChangeSet_Free(&changeSet));
    ASSERT_EQ(AwaError_OperationInvalid, ChangeSet_Free(NULL));
    Tree_Delete(objectsTree);
}


TEST_F(TestServerChangeSet, Server_AwaChangeSet_GetObjectsTree_valid_inputs)
{
    TreeNode objectsTree = ObjectsTree_New();
    AwaChangeSet * changeSet = ChangeSet_New(session_, SessionType_Server, objectsTree);
    ASSERT_TRUE(NULL != changeSet);
    ASSERT_TRUE(NULL != ChangeSet_GetObjectsTree(changeSet));

    char buffer[512];
    Xml_TreeToString(ChangeSet_GetObjectsTree(changeSet), buffer, sizeof(buffer));
    ASSERT_STREQ("<Objects></Objects>\n", buffer);
    ASSERT_EQ(AwaError_Success, ChangeSet_Free(&changeSet));
    ASSERT_TRUE(NULL == changeSet);
    Tree_Delete(objectsTree);
}

TEST_F(TestServerChangeSet, Server_AwaChangeSet_GetObjectsTree_invalid_inputs)
{
    AwaChangeSet * changeSet = NULL;
    ASSERT_EQ(NULL, ChangeSet_GetObjectsTree(changeSet));
    ASSERT_EQ(NULL, ChangeSet_GetObjectsTree(NULL));
}

TEST_F(TestServerChangeSet, Server_AwaChangeSet_GetExecuteArguments_valid_inputs)
{
    const char * xml =
           "<Objects>"
           "  <Object>"
           "    <ID>3</ID>"
           "    <ObjectInstance>"
           "      <ID>0</ID>"
           "      <Resource>"
           "        <ID>4</ID>"
           "        <Value>SGVsbG8=</Value>" // 'Hello' base64 encoded
           "      </Resource>"
           "    </ObjectInstance>"
           "  </Object>"
           "</Objects>";

    TreeNode objectsTree = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    AwaChangeSet * changeSet = ChangeSet_New(session_, SessionType_Server, objectsTree);
    ASSERT_TRUE(NULL != changeSet);
    AwaExecuteArguments arguments = { 0 };
    EXPECT_EQ(AwaError_Success, ChangeSet_GetExecuteArguments(changeSet, "/3/0/4", &arguments));

    EXPECT_EQ(arguments.Size, 5u);
    ASSERT_TRUE(NULL != arguments.Data);
    EXPECT_EQ(0, memcmp("Hello", (const char *)arguments.Data, 5));

    ASSERT_EQ(AwaError_Success, ChangeSet_Free(&changeSet));
    ASSERT_TRUE(NULL == changeSet);
    Tree_Delete(objectsTree);
}

TEST_F(TestServerChangeSet, Server_AwaChangeSet_GetExecuteArguments_invalid_inputs)
{
    const char * xml =
               "<Objects>"
               "  <Object>"
               "    <ID>3</ID>"
               "    <ObjectInstance>"
               "      <ID>0</ID>"
               "      <Resource>"
               "        <ID>4</ID>"
               "        <Value>SGVsbG8=</Value>" // 'Hello' base64 encoded
               "      </Resource>"
               "    </ObjectInstance>"
               "  </Object>"
               "</Objects>";

    TreeNode objectsTree = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    AwaChangeSet * changeSet = ChangeSet_New(session_, SessionType_Server, objectsTree);
    ASSERT_TRUE(NULL != changeSet);
    AwaExecuteArguments arguments;
    EXPECT_EQ(AwaError_OperationInvalid, ChangeSet_GetExecuteArguments(NULL, "/3/0/4", &arguments));
    EXPECT_EQ(AwaError_PathInvalid, ChangeSet_GetExecuteArguments(changeSet, NULL, &arguments));
    EXPECT_EQ(AwaError_OperationInvalid, ChangeSet_GetExecuteArguments(changeSet, "/3/0/4", NULL));

    ASSERT_EQ(AwaError_Success, ChangeSet_Free(&changeSet));
    ASSERT_TRUE(NULL == changeSet);
    Tree_Delete(objectsTree);
}

TEST_F(TestServerChangeSet, Server_AwaChangeSet_GetExecuteArguments_no_resource_in_response)
{
    const char * xml =
               "<Objects>"
               "  <Object>"
               "    <ID>3</ID>"
               "    <ObjectInstance>"
               "      <ID>0</ID>"
               "    </ObjectInstance>"
               "  </Object>"
               "</Objects>";

    TreeNode objectsTree = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    AwaChangeSet * changeSet = ChangeSet_New(session_, SessionType_Server, objectsTree);
    ASSERT_TRUE(NULL != changeSet);
    AwaExecuteArguments arguments;
    EXPECT_EQ(AwaError_PathNotFound, ChangeSet_GetExecuteArguments(changeSet, "/3/0/4", &arguments));
    ASSERT_EQ(AwaError_Success, ChangeSet_Free(&changeSet));
    ASSERT_TRUE(NULL == changeSet);
    Tree_Delete(objectsTree);
}


TEST_F(TestServerChangeSet, Server_AwaChangeSet_GetServerSession_handles_valid_inputs)
{
    TreeNode objectsTree = ObjectsTree_New();
    AwaChangeSet * changeSet = ChangeSet_New(session_, SessionType_Server, objectsTree);
    ASSERT_TRUE(NULL != changeSet);
    EXPECT_EQ(session_, AwaChangeSet_GetServerSession(changeSet));
    ASSERT_EQ(AwaError_Success, ChangeSet_Free(&changeSet));
    ASSERT_TRUE(NULL == changeSet);
    Tree_Delete(objectsTree);
}
TEST_F(TestServerChangeSet, Server_AwaChangeSet_GetServerSession_handles_invalid_inputs)
{
    ASSERT_TRUE(NULL == AwaChangeSet_GetServerSession(NULL));
}


TEST_F(TestServerChangeSet, Server_AwaChangeSet_GetValueAsIntegerArrayPointer_valid_inputs)
{
    const char * xml =
           "<Objects>"
           "  <Object>"
           "   <ID>3</ID>"
           "   <ObjectInstance>"
           "    <ID>0</ID>"
           "    <Resource>"
           "     <ID>7</ID>"
           "     <Observe></Observe>"
           "     <ResourceInstance>"
           "      <ID>0</ID>"
           "      <Value>2700</Value>"
           "     </ResourceInstance>"
           "     <ResourceInstance>"
           "      <ID>1</ID>"
           "      <Value>5000</Value>"
           "     </ResourceInstance>"
           "    </Resource>"
           "   </ObjectInstance>"
           "  </Object>"
           " </Objects>";

    TreeNode objectsTree = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    AwaChangeSet * changeSet = ChangeSet_New(session_, SessionType_Server, objectsTree);
    ASSERT_TRUE(NULL != changeSet);
    const AwaIntegerArray * array;
    EXPECT_EQ(AwaError_Success, AwaChangeSet_GetValuesAsIntegerArrayPointer(changeSet, "/3/0/7", &array));

    ASSERT_EQ(static_cast<size_t>(2), AwaIntegerArray_GetValueCount(array));

    ASSERT_EQ(AwaError_Success, ChangeSet_Free(&changeSet));
    ASSERT_TRUE(NULL == changeSet);
    Tree_Delete(objectsTree);
}


} // namespace Awa



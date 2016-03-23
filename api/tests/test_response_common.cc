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

#include "response_common.h"
#include "client_session.h"
#include "support/support.h"

namespace Awa {

class TestResponseCommon : public TestClientBase {};

class TestResponseCommonWithConnectedSession : public TestClientWithConnectedSession {};

TEST_F(TestResponseCommon, ResponseCommon_New_handles_null_operation)
{
    TreeNode objectsNode = Xml_CreateNode("Objects");
    ResponseCommon * response = ResponseCommon_New(NULL, objectsNode);
    ASSERT_TRUE(NULL == response);
    ASSERT_TRUE(Tree_Delete(objectsNode));
}
TEST_F(TestResponseCommon, ResponseCommon_New_handles_null_objectsNode)
{
    AwaClientSession * session = (AwaClientSession *)12345;
    OperationCommon * operation = OperationCommon_New(session, SessionType_Client);
    ASSERT_TRUE(NULL != operation);

    ResponseCommon * response = ResponseCommon_New(operation, NULL);
    ASSERT_TRUE(NULL == response);

    OperationCommon_Free(&operation);
}

TEST_F(TestResponseCommonWithConnectedSession, ResponseCommon_New_and_Free_works)
{
    OperationCommon * operation = OperationCommon_New(session_, SessionType_Client);
    TreeNode objectsNode = Xml_CreateNode("Objects");

    ResponseCommon * response = ResponseCommon_New(operation, objectsNode);
    ASSERT_TRUE(NULL != response);
    ASSERT_EQ(operation, ResponseCommon_GetOperation(response));
    //response objectsNode should be equivalent, but copied.
    ASSERT_NE(objectsNode, ResponseCommon_GetObjectsNode(response));
    char buffer1[512];
    char buffer2[512];
    Xml_TreeToString(objectsNode, buffer1, sizeof(buffer1));
    Xml_TreeToString(ResponseCommon_GetObjectsNode(response), buffer2, sizeof(buffer2));
    ASSERT_STREQ(buffer1, "<Objects></Objects>\n");
    ASSERT_STREQ(buffer1, buffer2);

    OperationCommon_Free(&operation);
    ResponseCommon_Free(&response);
    ASSERT_TRUE(NULL == response);
    ASSERT_TRUE(Tree_Delete(objectsNode));
}

TEST_F(TestResponseCommon, ResponseCommon_Free_handles_null_response)
{
    ResponseCommon * response = NULL;
    ASSERT_EQ(AwaError_OperationInvalid, ResponseCommon_Free(&response));
    ASSERT_EQ(AwaError_OperationInvalid, ResponseCommon_Free(NULL));
}

TEST_F(TestResponseCommon, ResponseCommon_GetOperation_invalid)
{
    ASSERT_TRUE(NULL == ResponseCommon_GetOperation(NULL));
}

TEST_F(TestResponseCommon, ResponseCommon_GetContentNode_invalid)
{
    ASSERT_TRUE(NULL == ResponseCommon_GetObjectsNode(NULL));
}

TEST_F(TestResponseCommonWithConnectedSession, ResponseCommon_BuildPathResults_invalid_input)
{
    ASSERT_EQ(AwaError_ResponseInvalid, ResponseCommon_BuildPathResults(NULL));
}

TEST_F(TestResponseCommonWithConnectedSession, ResponseCommon_GetPathResult_invalid_input)
{
    OperationCommon * operation = OperationCommon_New(session_, SessionType_Client);
    TreeNode objectsNode = Xml_CreateNode("Objects");

    ResponseCommon * response = ResponseCommon_New(operation, objectsNode);
    ASSERT_TRUE(NULL != response);

    ResponseCommon_BuildPathResults(response);

    const PathResult * result;
    ASSERT_EQ(AwaError_ResponseInvalid, ResponseCommon_GetPathResult(NULL, "/3/0/1", &result));
    ASSERT_EQ(AwaError_PathInvalid, ResponseCommon_GetPathResult(response, NULL, &result));
    ASSERT_EQ(AwaError_Internal, ResponseCommon_GetPathResult(response, "/3/0/1", NULL));

    OperationCommon_Free(&operation);
    ResponseCommon_Free(&response);
    ASSERT_TRUE(NULL == response);
    ASSERT_TRUE(Tree_Delete(objectsNode));
}

TEST_F(TestResponseCommonWithConnectedSession, ResponseCommon_GetPathResult_valid_input)
{
    OperationCommon * operation = OperationCommon_New(session_, SessionType_Client);

    const char * xml =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>0</ID>"
            "      <Resource>"
            "        <ID>1</ID>"
            "        <Result>"
            "          <Error>AwaError_DefinitionInvalid</Error>"
            "        </Result>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != objectsNode);

    ResponseCommon * response = ResponseCommon_New(operation, objectsNode);
    ASSERT_TRUE(NULL != response);

    const PathResult * result;
    ASSERT_EQ(AwaError_Success, ResponseCommon_GetPathResult(response, "/3/0/1", &result));
    ASSERT_EQ(AwaError_DefinitionInvalid, PathResult_GetError(result));

    OperationCommon_Free(&operation);
    ResponseCommon_Free(&response);
    ASSERT_TRUE(NULL == response);
    ASSERT_TRUE(Tree_Delete(objectsNode));
}

TEST_F(TestResponseCommonWithConnectedSession, ResponseCommon_NewPathIterator_invalid_input)
{
    ASSERT_TRUE(NULL == ResponseCommon_NewPathIterator(NULL));
}

TEST_F(TestResponseCommonWithConnectedSession, ResponseCommon_NewPathIterator_valid_input)
{
    OperationCommon * operation = OperationCommon_New(session_, SessionType_Client);

    const char * xml =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>0</ID>"
            "      <Resource>"
            "        <ID>1</ID>"
            "        <Result>"
            "          <Error>AwaError_DefinitionInvalid</Error>"
            "        </Result>"
            "      </Resource>"
            "      <Resource>"
            "        <ID>2</ID>"
            "        <Result>"
            "          <Error>AwaError_Success</Error>"
            "        </Result>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != objectsNode);

    ResponseCommon * response = ResponseCommon_New(operation, objectsNode);
    ASSERT_TRUE(NULL != response);

    PathIterator * iterator = ResponseCommon_NewPathIterator(response);
    ASSERT_TRUE(NULL != iterator);

    ASSERT_TRUE(PathIterator_Next(iterator));
    ASSERT_STREQ("/3/0/1", PathIterator_Get(iterator));
    ASSERT_TRUE(PathIterator_Next(iterator));
    ASSERT_STREQ("/3/0/2", PathIterator_Get(iterator));
    ASSERT_FALSE(PathIterator_Next(iterator));

    PathIterator_Free(&iterator);
    ASSERT_TRUE(NULL == iterator);

    const PathResult * result;
    ASSERT_EQ(AwaError_Success, ResponseCommon_GetPathResult(response, "/3/0/1", &result));
    ASSERT_EQ(AwaError_DefinitionInvalid, PathResult_GetError(result));

    OperationCommon_Free(&operation);
    ResponseCommon_Free(&response);
    ASSERT_TRUE(NULL == response);
    ASSERT_TRUE(Tree_Delete(objectsNode));
}

TEST_F(TestResponseCommonWithConnectedSession, ResponseCommon_CheckForErrors_handles_null)
{
    EXPECT_EQ(AwaError_ResponseInvalid, ResponseCommon_CheckForErrors(NULL));
}

TEST_F(TestResponseCommonWithConnectedSession, ResponseCommon_CheckForErrors_single_error)
{
    const char * xml =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>0</ID>"
            "      <Resource>"
            "        <ID>1</ID>"
            "        <Result>"
            "          <Error>AwaError_DefinitionInvalid</Error>"
            "        </Result>"
            "      </Resource>"
            "      <Resource>"
            "        <ID>2</ID>"
            "        <Result>"
            "          <Error>AwaError_Success</Error>"
            "        </Result>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";
    TreeNode objectsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    OperationCommon * operation = OperationCommon_New(session_, SessionType_Client);
    ResponseCommon * response = ResponseCommon_New(operation, objectsNode);
    EXPECT_EQ(AwaError_Response, ResponseCommon_CheckForErrors(response));
    OperationCommon_Free(&operation);
    ResponseCommon_Free(&response);
    Tree_Delete(objectsNode);
}

TEST_F(TestResponseCommonWithConnectedSession, ResponseCommon_CheckForErrors_multiple_errors)
{
    const char * xml =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>0</ID>"
            "      <Resource>"
            "        <ID>1</ID>"
            "        <Result>"
            "          <Error>AwaError_DefinitionInvalid</Error>"
            "        </Result>"
            "      </Resource>"
            "      <Resource>"
            "        <ID>2</ID>"
            "        <Result>"
            "          <Error>AwaError_Timeout</Error>"
            "        </Result>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";
    TreeNode objectsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    OperationCommon * operation = OperationCommon_New(session_, SessionType_Client);
    ResponseCommon * response = ResponseCommon_New(operation, objectsNode);
    EXPECT_EQ(AwaError_Response, ResponseCommon_CheckForErrors(response));
    OperationCommon_Free(&operation);
    ResponseCommon_Free(&response);
    Tree_Delete(objectsNode);
}

TEST_F(TestResponseCommonWithConnectedSession, ResponseCommon_CheckForErrors_no_errors)
{
    const char * xml =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>0</ID>"
            "      <Resource>"
            "        <ID>1</ID>"
            "        <Result>"
            "          <Error>AwaError_Success</Error>"
            "        </Result>"
            "      </Resource>"
            "      <Resource>"
            "        <ID>2</ID>"
            "        <Result>"
            "          <Error>AwaError_Success</Error>"
            "        </Result>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";
    TreeNode objectsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    OperationCommon * operation = OperationCommon_New(session_, SessionType_Client);
    ResponseCommon * response = ResponseCommon_New(operation, objectsNode);
    EXPECT_EQ(AwaError_Success, ResponseCommon_CheckForErrors(response));
    OperationCommon_Free(&operation);
    ResponseCommon_Free(&response);
    Tree_Delete(objectsNode);
}

TEST_F(TestResponseCommonWithConnectedSession, ResponseCommon_CheckForErrors_missing_result)
{
    const char * xml =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>0</ID>"
            "      <Resource>"
            "        <ID>1</ID>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";
    TreeNode objectsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    OperationCommon * operation = OperationCommon_New(session_, SessionType_Client);
    ResponseCommon * response = ResponseCommon_New(operation, objectsNode);
    EXPECT_EQ(AwaError_ResponseInvalid, ResponseCommon_CheckForErrors(response));
    OperationCommon_Free(&operation);
    ResponseCommon_Free(&response);
    Tree_Delete(objectsNode);
}

TEST_F(TestResponseCommonWithConnectedSession, ResponseCommon_CheckForErrors_missing_result_error)
{
    const char * xml =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>0</ID>"
            "      <Resource>"
            "        <ID>1</ID>"
            "        <Result>"
            "        </Result>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";
    TreeNode objectsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    OperationCommon * operation = OperationCommon_New(session_, SessionType_Client);
    ResponseCommon * response = ResponseCommon_New(operation, objectsNode);
    EXPECT_EQ(AwaError_ResponseInvalid, ResponseCommon_CheckForErrors(response));
    OperationCommon_Free(&operation);
    ResponseCommon_Free(&response);
    Tree_Delete(objectsNode);
}

TEST_F(TestResponseCommonWithConnectedSession, ResponseCommon_BuildValues_handles_null)
{
    EXPECT_EQ(AwaError_ResponseInvalid, ResponseCommon_BuildValues(NULL));
}

TEST_F(TestResponseCommonWithConnectedSession, ResponseCommon_BuildValues_handles_single_value)
{
    const char * xml =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>0</ID>"
            "      <Resource>"
            "        <ID>9</ID>"
            "        <Value>42</Value>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";
    TreeNode objectsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    OperationCommon * operation = OperationCommon_New(session_, SessionType_Client);
    SessionType sessionType = SessionType_Invalid;
    EXPECT_EQ(session_, OperationCommon_GetSession(operation, &sessionType));
    EXPECT_EQ(SessionType_Client, sessionType);

    ResponseCommon * response = ResponseCommon_New(operation, objectsNode);

    const AwaInteger * valuePtr = NULL;
    ASSERT_EQ(AwaError_Success, ResponseCommon_GetValuePointer(response, "/3/0/9", (const void **)&valuePtr, NULL, AwaResourceType_Integer, sizeof(AwaInteger)));
    EXPECT_EQ(42, *valuePtr);

    OperationCommon_Free(&operation);
    ResponseCommon_Free(&response);
    Tree_Delete(objectsNode);
}


TEST_F(TestResponseCommon, ResponseCommon_HasValue_handles_null_response)
{
    ASSERT_FALSE(ResponseCommon_HasValue(NULL, "/3/0/10"));
}
TEST_F(TestResponseCommonWithConnectedSession, ResponseCommon_HasValue_handles_null_path)
{
    TreeNode objectsNode = ObjectsTree_New();
    OperationCommon * operation = OperationCommon_New(session_, SessionType_Client);
    ResponseCommon * response = ResponseCommon_New(operation, objectsNode);

    ASSERT_FALSE(ResponseCommon_HasValue(response, NULL));

    OperationCommon_Free(&operation);
    ResponseCommon_Free(&response);
    Tree_Delete(objectsNode);
}
TEST_F(TestResponseCommonWithConnectedSession, ResponseCommon_HasValue_handles_single_value)
{
    const char * xml =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>0</ID>"
            "      <Resource>"
            "        <ID>9</ID>"
            "        <Value>42</Value>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";
    TreeNode objectsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    OperationCommon * operation = OperationCommon_New(session_, SessionType_Client);
    ResponseCommon * response = ResponseCommon_New(operation, objectsNode);

    ASSERT_TRUE(ResponseCommon_HasValue(response, "/3/0/9"));
    ASSERT_FALSE(ResponseCommon_HasValue(response, "/3/0/10"));

    OperationCommon_Free(&operation);
    ResponseCommon_Free(&response);
    Tree_Delete(objectsNode);
}


TEST_F(TestResponseCommon, ResponseCommon_ContainsPath_handles_null_response)
{
    ASSERT_FALSE(ResponseCommon_ContainsPath(NULL, "/3/0/10"));
}
TEST_F(TestResponseCommonWithConnectedSession, ResponseCommon_ContainsPath_handles_null_path)
{
    TreeNode objectsNode = ObjectsTree_New();
    OperationCommon * operation = OperationCommon_New(session_, SessionType_Client);
    ResponseCommon * response = ResponseCommon_New(operation, objectsNode);

    ASSERT_FALSE(ResponseCommon_ContainsPath(response, NULL));

    OperationCommon_Free(&operation);
    ResponseCommon_Free(&response);
    Tree_Delete(objectsNode);
}
TEST_F(TestResponseCommonWithConnectedSession, ResponseCommon_ContainsPath_handles_single_value)
{
    const char * xml =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <ID>0</ID>"
            "      <Resource>"
            "        <ID>9</ID>"
            "        <Value>42</Value>"
            "      </Resource>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";
    TreeNode objectsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    OperationCommon * operation = OperationCommon_New(session_, SessionType_Client);
    ResponseCommon * response = ResponseCommon_New(operation, objectsNode);

    ASSERT_TRUE(ResponseCommon_ContainsPath(response, "/3"));
    ASSERT_TRUE(ResponseCommon_ContainsPath(response, "/3/0"));
    ASSERT_TRUE(ResponseCommon_ContainsPath(response, "/3/0/9"));

    ASSERT_FALSE(ResponseCommon_ContainsPath(response, "/2"));
    ASSERT_FALSE(ResponseCommon_ContainsPath(response, "/3/1"));
    ASSERT_FALSE(ResponseCommon_ContainsPath(response, "/3/0/10"));

    OperationCommon_Free(&operation);
    ResponseCommon_Free(&response);
    Tree_Delete(objectsNode);
}


} // namespace Awa

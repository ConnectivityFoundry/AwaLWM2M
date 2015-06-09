#include <gtest/gtest.h>

#include <vector>
#include <string>

#include "server_response.h"
#include "support/support.h"

#include "server_operation.h"
#include "xmltree.h"

namespace Awa {

class TestServerResponse : public TestServerBase {};

class TestServerResponseWithConnectedSession : public TestServerWithConnectedSession {};

TEST_F(TestServerResponseWithConnectedSession, ServerResponse_New_and_Free_work)
{
    const char * xml = "<Clients></Clients>";
    TreeNode clientsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ServerOperation * serverOperation = ServerOperation_New(session_);
    ServerResponse * response = ServerResponse_NewFromServerOperation(serverOperation, clientsNode);
    EXPECT_TRUE(NULL != response);
    EXPECT_EQ(AwaError_Success, ServerResponse_Free(&response));
    EXPECT_EQ(NULL, response);
    ServerOperation_Free(&serverOperation);
    Tree_Delete(clientsNode);
}

TEST_F(TestServerResponseWithConnectedSession, ServerResponse_New_with_bad_clientsNode)
{
    const char * xml = "<Foobar/>";
    TreeNode clientsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ServerOperation * serverOperation = ServerOperation_New(session_);
    ServerResponse * response = ServerResponse_NewFromServerOperation(serverOperation, clientsNode);
    EXPECT_EQ(NULL, response);
    ServerResponse_Free(&response);
    ServerOperation_Free(&serverOperation);
    Tree_Delete(clientsNode);  // must manually free
}

TEST_F(TestServerResponse, ServerResponse_New_handles_null_serverOperation)
{
    TreeNode clientsNode = TreeNode_Create();
    EXPECT_EQ(NULL, ServerResponse_NewFromServerOperation(NULL, clientsNode));
    Tree_Delete(clientsNode);  // must manually free
}

TEST_F(TestServerResponseWithConnectedSession, ServerResponse_New_handles_null_clientsNode)
{
    ServerOperation * serverOperation = ServerOperation_New(session_);
    ServerResponse * response = ServerResponse_NewFromServerOperation(serverOperation, NULL);
    EXPECT_EQ(NULL, response);
    ServerOperation_Free(&serverOperation);
}

TEST_F(TestServerResponseWithConnectedSession, ServerResponse_NewFromServerOperation_and_Free_work)
{
    const char * xml = "<Clients></Clients>";
    TreeNode clientsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ServerOperation * serverOperation = ServerOperation_New(session_);
    ServerResponse * response = ServerResponse_NewFromServerOperation(serverOperation, clientsNode);
    EXPECT_TRUE(NULL != response);
    EXPECT_EQ(AwaError_Success, ServerResponse_Free(&response));
    EXPECT_EQ(NULL, response);
    ServerOperation_Free(&serverOperation);
    Tree_Delete(clientsNode);
}

TEST_F(TestServerResponseWithConnectedSession, ServerResponse_NewFromServerOperation_handles_null_server_operation)
{
    const char * xml = "<Clients></Clients>";
    TreeNode clientsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ServerResponse * response = ServerResponse_NewFromServerOperation(NULL, clientsNode);
    EXPECT_EQ(NULL, response);
    Tree_Delete(clientsNode);
}

TEST_F(TestServerResponseWithConnectedSession, ServerResponse_NewFromServerOperation_handles_null_clients_node)
{
    ServerOperation * serverOperation = ServerOperation_New(session_);
    ServerResponse * response = ServerResponse_NewFromServerOperation(serverOperation, NULL);
    EXPECT_EQ(NULL, response);
    ServerOperation_Free(&serverOperation);
}

TEST_F(TestServerResponse, ServerResponse_Free_handles_null_pointer)
{
    EXPECT_EQ(AwaError_Unspecified, ServerResponse_Free(NULL));
}

TEST_F(TestServerResponse, ServerResponse_Free_handles_pointer_to_null)
{
    ServerResponse * response = NULL;
    EXPECT_EQ(AwaError_Unspecified, ServerResponse_Free(&response));
}

TEST_F(TestServerResponseWithConnectedSession, ServerResponse_GetNextClientID_no_clients)
{
    const char * xml =
            "<Clients>"
            "</Clients>";
    TreeNode clientsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ServerOperation * serverOperation = ServerOperation_New(session_);
    ServerResponse * response = ServerResponse_NewFromServerOperation(serverOperation, clientsNode);
    EXPECT_TRUE(NULL != response);

    const char * clientID = ServerResponse_GetNextClientID(response, NULL);
    EXPECT_EQ(NULL, clientID);

    EXPECT_EQ(AwaError_Success, ServerResponse_Free(&response));
    EXPECT_EQ(NULL, response);
    ServerOperation_Free(&serverOperation);
    Tree_Delete(clientsNode);
}

TEST_F(TestServerResponseWithConnectedSession, ServerResponse_GetNextClientID_one_client)
{
    const char * xml =
            "<Clients>"
            "  <Client>"
            "    <ID>7</ID>"
            "    <Objects/>"
            "  </Client>"
            "</Clients>";
    TreeNode clientsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ServerOperation * serverOperation = ServerOperation_New(session_);
    ServerResponse * response = ServerResponse_NewFromServerOperation(serverOperation, clientsNode);
    EXPECT_TRUE(NULL != response);

    const char * clientID = ServerResponse_GetNextClientID(response, NULL);
    EXPECT_TRUE(NULL != clientID);
    EXPECT_STREQ("7", clientID);

    clientID = ServerResponse_GetNextClientID(response, clientID);
    EXPECT_EQ(NULL, clientID);

    EXPECT_EQ(AwaError_Success, ServerResponse_Free(&response));
    EXPECT_EQ(NULL, response);
    ServerOperation_Free(&serverOperation);
    Tree_Delete(clientsNode);
}

TEST_F(TestServerResponseWithConnectedSession, ServerResponse_GetNextClientID_multiple_clients)
{
    const char * xml =
            "<Clients>"
            "  <Client>"
            "    <ID>7</ID>"
            "  </Client>"
            "  <Client>"
            "    <ID>42</ID>"
            "  </Client>"
            "  <Client>"
            "    <ID>107</ID>"
            "  </Client>"
            "</Clients>";
    TreeNode clientsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ServerOperation * serverOperation = ServerOperation_New(session_);
    ServerResponse * response = ServerResponse_NewFromServerOperation(serverOperation, clientsNode);
    EXPECT_TRUE(NULL != response);

    const char * clientID = ServerResponse_GetNextClientID(response, NULL);
    EXPECT_TRUE(NULL != clientID);
    EXPECT_STREQ("7", clientID);

    clientID = ServerResponse_GetNextClientID(response, clientID);
    EXPECT_TRUE(NULL != clientID);
    EXPECT_STREQ("42", clientID);

    clientID = ServerResponse_GetNextClientID(response, clientID);
    EXPECT_TRUE(NULL != clientID);
    EXPECT_STREQ("107", clientID);

    clientID = ServerResponse_GetNextClientID(response, clientID);
    EXPECT_EQ(NULL, clientID);

    EXPECT_EQ(AwaError_Success, ServerResponse_Free(&response));
    EXPECT_EQ(NULL, response);
    ServerOperation_Free(&serverOperation);
    Tree_Delete(clientsNode);
}

TEST_F(TestServerResponseWithConnectedSession, ServerResponse_GetClientResponse_handles_null_response)
{
    EXPECT_EQ(NULL, ServerResponse_GetClientResponse(NULL, "TestClient1"));
}

TEST_F(TestServerResponseWithConnectedSession, ServerResponse_GetClientResponse_handles_null_clientID)
{
    const char * xml = "<Content/>";
    TreeNode clientsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ServerOperation * serverOperation = ServerOperation_New(session_);
    ServerResponse * response = ServerResponse_NewFromServerOperation(serverOperation, clientsNode);
    EXPECT_EQ(NULL, ServerResponse_GetClientResponse(response, NULL));
    ServerResponse_Free(&response);
    ServerOperation_Free(&serverOperation);
    Tree_Delete(clientsNode);
}

TEST_F(TestServerResponseWithConnectedSession, ServerResponse_GetClientResponse_handles_missing_clientID)
{
    const char * xml =
            "<Clients>"
            "  <Client>"
            "    <ID>7</ID>"
            "  </Client>"
            "  <Client>"
            "    <ID>Foo42</ID>"
            "  </Client>"
            "  <Client>"
            "    <ID>107barbar</ID>"
            "  </Client>"
            "</Clients>";
    TreeNode clientsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ServerOperation * serverOperation = ServerOperation_New(session_);
    ServerResponse * response = ServerResponse_NewFromServerOperation(serverOperation, clientsNode);
    EXPECT_EQ(NULL, ServerResponse_GetClientResponse(response, "TurtleSpinning"));
    EXPECT_EQ(NULL, ServerResponse_GetClientResponse(response, "Billions"));
    EXPECT_EQ(NULL, ServerResponse_GetClientResponse(response, "13"));
    ServerResponse_Free(&response);
    ServerOperation_Free(&serverOperation);
    Tree_Delete(clientsNode);
}

TEST_F(TestServerResponseWithConnectedSession, ServerResponse_GetClientResponse_handles_matching_clientID)
{
    const char * xml =
            "<Clients>"
            "  <Client>"
            "    <ID>7</ID>"
            "    <Objects/>"
            "  </Client>"
            "  <Client>"
            "    <ID>Foo42</ID>"
            "    <Objects/>"
            "  </Client>"
            "  <Client>"
            "    <ID>107barbar</ID>"
            "    <Objects/>"
            "  </Client>"
            "</Clients>";
    TreeNode clientsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ServerOperation * serverOperation = ServerOperation_New(session_);
    ServerResponse * response = ServerResponse_NewFromServerOperation(serverOperation, clientsNode);
    EXPECT_TRUE(NULL != ServerResponse_GetClientResponse(response, "7"));
    EXPECT_TRUE(NULL != ServerResponse_GetClientResponse(response, "Foo42"));
    EXPECT_TRUE(NULL != ServerResponse_GetClientResponse(response, "107barbar"));
    ServerResponse_Free(&response);
    ServerOperation_Free(&serverOperation);
    Tree_Delete(clientsNode);
}

TEST_F(TestServerResponseWithConnectedSession, ServerResponse_CheckForErrors_handles_null)
{
    EXPECT_EQ(AwaError_ResponseInvalid, ServerResponse_CheckForErrors(NULL));
}

TEST_F(TestServerResponseWithConnectedSession, ServerResponse_CheckForErrors_handles_no_errors)
{
    const char * xml =
            "<Clients>"
            "  <Client>"
            "    <ID>TestClient1</ID>"
            "    <Objects>"
            "      <Object>"
            "        <ID>3</ID>"
            "        <ObjectInstance>"
            "          <ID>0</ID>"
            "          <Resource>"
            "            <ID>9</ID>"
            "            <Result>"
            "              <Error>AwaError_Success</Error>"
            "            </Result>"
            "          </Resource>"
            "        </ObjectInstance>"
            "      </Object>"
            "    </Objects>"
            "  </Client>"
            "</Clients>";
    TreeNode clientsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ServerOperation * serverOperation = ServerOperation_New(session_);
    ServerResponse * response = ServerResponse_NewFromServerOperation(serverOperation, clientsNode);
    EXPECT_EQ(AwaError_Success, ServerResponse_CheckForErrors(response));
    ServerResponse_Free(&response);
    ServerOperation_Free(&serverOperation);
    Tree_Delete(clientsNode);
}

TEST_F(TestServerResponseWithConnectedSession, ServerResponse_CheckForErrors_handles_one_error)
{
    const char * xml =
            "<Clients>"
            "  <Client>"
            "    <ID>TestClient1</ID>"
            "    <Objects>"
            "      <Object>"
            "        <ID>3</ID>"
            "        <ObjectInstance>"
            "          <ID>0</ID>"
            "          <Resource>"
            "            <ID>8</ID>"
            "            <Result>"
            "              <Error>AwaError_Success</Error>"
            "            </Result>"
            "          </Resource>"
            "          <Resource>"
            "            <ID>9</ID>"
            "            <Result>"
            "              <Error>AwaError_ClientNotFound</Error>"
            "            </Result>"
            "          </Resource>"
            "        </ObjectInstance>"
            "      </Object>"
            "    </Objects>"
            "  </Client>"
            "</Clients>";
    TreeNode clientsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ServerOperation * serverOperation = ServerOperation_New(session_);
    ServerResponse * response = ServerResponse_NewFromServerOperation(serverOperation, clientsNode);
    EXPECT_EQ(AwaError_Response, ServerResponse_CheckForErrors(response));
    ServerResponse_Free(&response);
    ServerOperation_Free(&serverOperation);
    Tree_Delete(clientsNode);
}

TEST_F(TestServerResponseWithConnectedSession, ServerResponse_CheckForErrors_handles_all_errors_single_client)
{
    const char * xml =
            "<Clients>"
            "  <Client>"
            "    <ID>TestClient1</ID>"
            "    <Objects>"
            "      <Object>"
            "        <ID>3</ID>"
            "        <ObjectInstance>"
            "          <ID>0</ID>"
            "          <Resource>"
            "            <ID>8</ID>"
            "            <Result>"
            "              <Error>AwaError_TypeMismatch</Error>"
            "            </Result>"
            "          </Resource>"
            "          <Resource>"
            "            <ID>9</ID>"
            "            <Result>"
            "              <Error>AwaError_PathNotFound</Error>"
            "            </Result>"
            "          </Resource>"
            "        </ObjectInstance>"
            "      </Object>"
            "    </Objects>"
            "  </Client>"
            "</Clients>";
    TreeNode clientsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ServerOperation * serverOperation = ServerOperation_New(session_);
    ServerResponse * response = ServerResponse_NewFromServerOperation(serverOperation, clientsNode);
    EXPECT_EQ(AwaError_Response, ServerResponse_CheckForErrors(response));
    ServerResponse_Free(&response);
    ServerOperation_Free(&serverOperation);
    Tree_Delete(clientsNode);
}

TEST_F(TestServerResponseWithConnectedSession, ServerResponse_CheckForErrors_handles_all_errors_multiple_clients)
{
    const char * xml =
            "<Clients>"
            "  <Client>"
            "    <ID>TestClient1</ID>"
            "    <Objects>"
            "      <Object>"
            "        <ID>3</ID>"
            "        <ObjectInstance>"
            "          <ID>0</ID>"
            "          <Resource>"
            "            <ID>9</ID>"
            "            <Result>"
            "              <Error>AwaError_PathNotFound</Error>"
            "            </Result>"
            "          </Resource>"
            "        </ObjectInstance>"
            "      </Object>"
            "    </Objects>"
            "  </Client>"
            "  <Client>"
            "    <ID>TestClient2</ID>"
            "    <Objects>"
            "      <Object>"
            "        <ID>3</ID>"
            "        <ObjectInstance>"
            "          <ID>0</ID>"
            "          <Resource>"
            "            <ID>8</ID>"
            "            <Result>"
            "              <Error>AwaError_TypeMismatch</Error>"
            "            </Result>"
            "          </Resource>"
            "        </ObjectInstance>"
            "      </Object>"
            "    </Objects>"
            "  </Client>"
            "</Clients>";
    TreeNode clientsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ServerOperation * serverOperation = ServerOperation_New(session_);
    ServerResponse * response = ServerResponse_NewFromServerOperation(serverOperation, clientsNode);
    EXPECT_EQ(AwaError_Response, ServerResponse_CheckForErrors(response));
    ServerResponse_Free(&response);
    ServerOperation_Free(&serverOperation);
    Tree_Delete(clientsNode);
}

TEST_F(TestServerResponseWithConnectedSession, ServerResponse_CheckForErrors_handles_no_errors_multiple_clients)
{
    const char * xml =
            "<Clients>"
            "  <Client>"
            "    <ID>TestClient1</ID>"
            "    <Objects>"
            "      <Object>"
            "        <ID>3</ID>"
            "        <ObjectInstance>"
            "          <ID>0</ID>"
            "          <Resource>"
            "            <ID>9</ID>"
            "            <Result>"
            "              <Error>AwaError_Success</Error>"
            "            </Result>"
            "          </Resource>"
            "        </ObjectInstance>"
            "      </Object>"
            "    </Objects>"
            "  </Client>"
            "  <Client>"
            "    <ID>TestClient2</ID>"
            "    <Objects>"
            "      <Object>"
            "        <ID>3</ID>"
            "        <ObjectInstance>"
            "          <ID>0</ID>"
            "          <Resource>"
            "            <ID>8</ID>"
            "            <Result>"
            "              <Error>AwaError_Success</Error>"
            "            </Result>"
            "          </Resource>"
            "        </ObjectInstance>"
            "      </Object>"
            "    </Objects>"
            "  </Client>"
            "</Clients>";
    TreeNode clientsNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ServerOperation * serverOperation = ServerOperation_New(session_);
    ServerResponse * response = ServerResponse_NewFromServerOperation(serverOperation, clientsNode);
    EXPECT_EQ(AwaError_Success, ServerResponse_CheckForErrors(response));
    ServerResponse_Free(&response);
    ServerOperation_Free(&serverOperation);
    Tree_Delete(clientsNode);
}

} // namespace Awa

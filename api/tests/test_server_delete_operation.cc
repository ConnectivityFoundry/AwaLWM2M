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
#include <vector>
#include <string>
#include <algorithm>

#include "awa/server.h"
#include "support/support.h"


namespace Awa {

class TestServerDeleteOperation : public TestServerBase {};

class TestServerDeleteOperationWithConnectedSession : public TestServerAndClientWithConnectedSession {};

// TODO: refactor common New/Free tests into type-parameterised tests

TEST_F(TestServerDeleteOperationWithConnectedSession, AwaServerDeleteOperation_New_returns_valid_operation_and_free_works)
{
    // test that AwaServerDeleteOperation_Free works via valgrind
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(server_session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_Free(&operation));
}

TEST_F(TestServerDeleteOperation, AwaServerDeleteOperation_New_handles_null_session)
{
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(NULL);
    ASSERT_EQ(NULL, operation);
}

TEST_F(TestServerDeleteOperation, AwaServerDeleteOperation_New_handles_invalid_session)
{
    // An invalid session is one that is not connected
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(NULL, AwaServerDeleteOperation_New(session));

    // set up IPC - still not enough
    AwaServerSession_SetIPCAsUDP(session, "127.0.0.1", global::serverIpcPort);
    EXPECT_EQ(NULL, AwaServerDeleteOperation_New(session));

    AwaServerSession_Free(&session);
}

TEST_F(TestServerDeleteOperationWithConnectedSession, AwaServerDeleteOperation_Free_nulls_pointer)
{
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(server_session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_Free(&operation));
    ASSERT_EQ(NULL, operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSession, AwaServerDeleteOperation_Free_handles_null)
{
    EXPECT_EQ(AwaError_OperationInvalid, AwaServerDeleteOperation_Free(NULL));
}

TEST_F(TestServerDeleteOperationWithConnectedSession, AwaServerDeleteOperation_Free_handles_null_pointer)
{
    AwaServerDeleteOperation * operation = NULL;
    EXPECT_EQ(AwaError_OperationInvalid, AwaServerDeleteOperation_Free(&operation));
}

TEST_F(TestServerDeleteOperationWithConnectedSession, AwaServerDeleteOperation_Free_handles_invalid_session)
{
    // Session is freed before the operation (use a local session):
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session, "127.0.0.1", global::serverIpcPort));
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(session);
    AwaServerSession_Free(&session);
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_Free(&operation));
    // expect no crash or memory leaks
}

TEST_F(TestServerDeleteOperationWithConnectedSession, AwaServerDeleteOperation_AddPath_handles_null_operation)
{
    EXPECT_EQ(AwaError_OperationInvalid, AwaServerDeleteOperation_AddPath(NULL, global::clientEndpointName, "/1/2/3"));
}

TEST_F(TestServerDeleteOperationWithConnectedSession, AwaServerDeleteOperation_AddPath_handles_null_path)
{
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(server_session_);
    EXPECT_EQ(AwaError_PathInvalid, AwaServerDeleteOperation_AddPath(operation, global::clientEndpointName, NULL));
    AwaServerDeleteOperation_Free(&operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSession, AwaServerDeleteOperation_AddPath_handles_invalid_path)
{
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(server_session_);
    EXPECT_EQ(AwaError_PathInvalid, AwaServerDeleteOperation_AddPath(operation, global::clientEndpointName, "/an/invalid/path"));
    AwaServerDeleteOperation_Free(&operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSession, AwaServerDeleteOperation_AddPath_handles_null_client_id)
{
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(server_session_);
    EXPECT_EQ(AwaError_OperationInvalid, AwaServerDeleteOperation_AddPath(operation, NULL, "/4/0/0"));
    AwaServerDeleteOperation_Free(&operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSession, AwaServerDeleteOperation_AddPath_handles_valid_path)
{
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(server_session_);
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, global::clientEndpointName, "/4/0/0"));
    AwaServerDeleteOperation_Free(&operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSession, AwaServerDeleteOperation_AddPath_handles_dropped_paths)
{
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(server_session_);
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, global::clientEndpointName, "/4/0"));
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, global::clientEndpointName, "/4/0/0"));
    AwaServerDeleteOperation_Free(&operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSession, AwaServerDeleteOperation_AddPath_handles_more_general_path)
{
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(server_session_); ASSERT_TRUE(NULL != operation);
    ASSERT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, global::clientEndpointName, "/4/0/0"));
    ASSERT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, global::clientEndpointName, "/4/0"));
    ASSERT_EQ(AwaError_Success, AwaServerDeleteOperation_Perform(operation, defaults::timeout));
    AwaServerDeleteOperation_Free(&operation);

    // check the result - expect no content for /4/0
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/4/0"));
    ASSERT_EQ(AwaError_Response, AwaServerReadOperation_Perform(readOperation, defaults::timeout));

    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != readResponse);
    const AwaPathResult * result = AwaServerReadResponse_GetPathResult(readResponse, "/4/0");
    EXPECT_EQ(AwaError_LWM2MError, AwaPathResult_GetError(result));
    EXPECT_EQ(AwaLWM2MError_NotFound, AwaPathResult_GetLWM2MError(result));

    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestServerDeleteOperationWithConnectedSession, AwaServerDeleteOperation_Perform_handles_null_operation)
{
    EXPECT_EQ(AwaError_OperationInvalid, AwaServerDeleteOperation_Perform(NULL, defaults::timeout));
}

TEST_F(TestServerDeleteOperationWithConnectedSession, AwaServerDeleteOperation_Perform_handles_empty_operation)
{
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(server_session_);
    // no paths added
    EXPECT_EQ(AwaError_OperationInvalid, AwaServerDeleteOperation_Perform(operation, defaults::timeout));
    AwaServerDeleteOperation_Free(&operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSession, AwaServerDeleteOperation_Perform_single_add_operation_on_resource_should_fail)
{
    // server cannot delete objects or resources, only entire object instances.
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(server_session_);
    ASSERT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, global::clientEndpointName, "/3/0/15"));
    EXPECT_EQ(AwaError_Response, AwaServerDeleteOperation_Perform(operation, defaults::timeout));

    const AwaServerDeleteResponse * deleteResponse = AwaServerDeleteOperation_GetResponse(operation, global::clientEndpointName);
    ASSERT_TRUE(NULL != deleteResponse);
    const AwaPathResult * result = AwaServerDeleteResponse_GetPathResult(deleteResponse, "/3/0/15");
    EXPECT_EQ(AwaError_LWM2MError, AwaPathResult_GetError(result));
    EXPECT_EQ(AwaLWM2MError_MethodNotAllowed, AwaPathResult_GetLWM2MError(result));
    AwaServerDeleteOperation_Free(&operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSession, AwaServerDeleteOperation_Perform_unknown_client)
{
    const char * clientID = "NonExistingClientID";
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(server_session_);
    ASSERT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, clientID, "/4/0"));
    EXPECT_EQ(AwaError_Response, AwaServerDeleteOperation_Perform(operation, defaults::timeout));

    const AwaServerDeleteResponse * deleteResponse = AwaServerDeleteOperation_GetResponse(operation, clientID);
    ASSERT_TRUE(NULL != deleteResponse);
    const AwaPathResult * result = AwaServerDeleteResponse_GetPathResult(deleteResponse, "/4/0");
    EXPECT_EQ(AwaError_ClientNotFound, AwaPathResult_GetError(result));

    AwaServerDeleteOperation_Free(&operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSession, AwaServerDeleteOperation_Perform_handles_single_add_operation_on_object_instance)
{
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(server_session_);
    ASSERT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, global::clientEndpointName, "/4/0"));
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_Perform(operation, defaults::timeout));
    AwaServerDeleteOperation_Free(&operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSession, AwaServerDeleteOperation_Perform_handles_single_add_operation_on_object_should_fail)
{
    // server cannot delete objects or resources, only entire object instances.
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(server_session_);
    ASSERT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, global::clientEndpointName, "/4"));
    EXPECT_EQ(AwaError_Response, AwaServerDeleteOperation_Perform(operation, defaults::timeout));

    const AwaServerDeleteResponse * deleteResponse = AwaServerDeleteOperation_GetResponse(operation, global::clientEndpointName);
    ASSERT_TRUE(NULL != deleteResponse);
    const AwaPathResult * result = AwaServerDeleteResponse_GetPathResult(deleteResponse, "/4");
    EXPECT_EQ(AwaError_LWM2MError, AwaPathResult_GetError(result));
    EXPECT_EQ(AwaLWM2MError_MethodNotAllowed, AwaPathResult_GetLWM2MError(result));

    AwaServerDeleteOperation_Free(&operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSession, DISABLED_AwaServerDeleteOperation_Perform_twice_fails)
{
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(server_session_);
    ASSERT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, global::clientEndpointName, "/4/0"));
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_Perform(operation, defaults::timeout));

    EXPECT_EQ(AwaError_Response, AwaServerDeleteOperation_Perform(operation, defaults::timeout));

    const AwaServerDeleteResponse * deleteResponse = AwaServerDeleteOperation_GetResponse(operation, global::clientEndpointName);
    ASSERT_TRUE(NULL != deleteResponse);
    const AwaPathResult * result = AwaServerDeleteResponse_GetPathResult(deleteResponse, "/4/0");
    EXPECT_EQ(AwaError_LWM2MError, AwaPathResult_GetError(result));

    // FIXME: LibCoAP always returns MethodNotAllowed when the endpoint does not exist.
    EXPECT_EQ(AwaLWM2MError_NotFound, AwaPathResult_GetLWM2MError(result));
    AwaServerDeleteOperation_Free(&operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSession, DISABLED_AwaServerDeleteOperation_Perform_handles_multiple_deletes_on_one_object_instance)
{
    // assume these resources exist by default!
    std::vector<const char *> paths { "/4/0/0", "/4/0/1", "/4/0/9" };

    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(server_session_);

    for (auto it = paths.begin(); it != paths.end(); ++it)
    {
        ASSERT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, global::clientEndpointName, *it));
    }
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_Perform(operation, defaults::timeout));
    AwaServerDeleteOperation_Free(&operation);

    // try to delete them again, one at a time, to check for expected failure
    for (auto it = paths.begin(); it != paths.end(); ++it)
    {
        operation = AwaServerDeleteOperation_New(server_session_);
        ASSERT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, global::clientEndpointName, *it));
        EXPECT_EQ(AwaError_CannotDelete, AwaServerDeleteOperation_Perform(operation, defaults::timeout));
        AwaServerDeleteOperation_Free(&operation);
    }
}

TEST_F(TestServerDeleteOperationWithConnectedSession, DISABLED_AwaServerDeleteOperation_Perform_handles_multiple_deletes_on_multiple_object_instances)
{
    // TODO: create new instances of /4:
    // CreateInstance("/4/7")
    // CreateInstance("/4/8")

    // assume these resources exist by default!
    std::vector<const char *> paths { "/4/0/0", "/4/7/1", "/4/8/9" };

    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(server_session_);

    for (auto it = paths.begin(); it != paths.end(); ++it)
    {
        ASSERT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, global::clientEndpointName, *it));
    }
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_Perform(operation, defaults::timeout));
    AwaServerDeleteOperation_Free(&operation);

    // try to delete them again, one at a time, to check for expected failure
    for (auto it = paths.begin(); it != paths.end(); ++it)
    {
        operation = AwaServerDeleteOperation_New(server_session_);
        ASSERT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, global::clientEndpointName, *it));
        EXPECT_EQ(AwaError_CannotDelete, AwaServerDeleteOperation_Perform(operation, defaults::timeout));
        AwaServerDeleteOperation_Free(&operation);
    }
}

TEST_F(TestServerDeleteOperationWithConnectedSession, DISABLED_AwaServerDeleteOperation_Perform_handles_multiple_deletes_on_multiple_objects)
{
    // assume these resources exist by default!
    std::vector<const char *> paths { "/6/0", "/4/0"};

    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(server_session_);

    for (auto it = paths.begin(); it != paths.end(); ++it)
    {
        ASSERT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, global::clientEndpointName, *it));
    }
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_Perform(operation, defaults::timeout));
    AwaServerDeleteOperation_Free(&operation);

    // try to delete them again, one at a time, to check for expected failure
    for (auto it = paths.begin(); it != paths.end(); ++it)
    {
        operation = AwaServerDeleteOperation_New(server_session_);
        ASSERT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, global::clientEndpointName, *it));
        EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_Perform(operation, defaults::timeout));

        const AwaServerDeleteResponse * response = AwaServerDeleteOperation_GetResponse(operation, global::clientEndpointName);
        EXPECT_TRUE(NULL != response);

        const AwaPathResult * pathResult = AwaServerDeleteResponse_GetPathResult(response, *it);
        EXPECT_TRUE(NULL != pathResult);

        ASSERT_EQ(AwaLWM2MError_MethodNotAllowed, AwaPathResult_GetLWM2MError(pathResult));

        AwaServerDeleteOperation_Free(&operation);
    }
}

TEST_F(TestServerDeleteOperationWithConnectedSession, DISABLED_AwaServerDeleteOperation_Perform_handles_combined)
{
    // Test combined AddPath and AddPathWithArrayRange operations
}

TEST_F(TestServerDeleteOperationWithConnectedSession, AwaServerDeleteOperation_Perform_handles_negative_timeout)
{
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(server_session_);
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, global::clientEndpointName, "/4/0/0"));
    EXPECT_EQ(AwaError_OperationInvalid, AwaServerDeleteOperation_Perform(operation, -1));
    AwaServerDeleteOperation_Free(&operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSession, AwaServerDeleteOperation_AddPath_resource_path_does_not_shadow_instance_path)
{
    // Adding /4/0/0 after adding /4/0 should not disable the delete of /4/0
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(server_session_);
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, global::clientEndpointName, "/4/0"));
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, global::clientEndpointName, "/4/0/0"));
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_Perform(operation, defaults::timeout));
    AwaServerDeleteOperation_Free(&operation);

    // check that the correct resources were deleted
    {
        AwaServerReadOperation * operation = AwaServerReadOperation_New(server_session_);
        AwaServerReadOperation_AddPath(operation, global::clientEndpointName, "/4/0");
        EXPECT_EQ(AwaError_Response, AwaServerReadOperation_Perform(operation, defaults::timeout));
        const AwaServerReadResponse * response = AwaServerReadOperation_GetResponse(operation, global::clientEndpointName);
        EXPECT_TRUE(NULL != response);

        const AwaPathResult * pathResult = AwaServerReadResponse_GetPathResult(response, "/4/0");
        EXPECT_TRUE(NULL != pathResult);

        ASSERT_EQ(AwaLWM2MError_NotFound, AwaPathResult_GetLWM2MError(pathResult));

        AwaServerReadOperation_Free(&operation);
    }
}

TEST_F(TestServerDeleteOperationWithConnectedSession, DISABLED_AwaServerDeleteOperation_Perform_handles_zero_timeout)
{
    // how?
}

TEST_F(TestServerDeleteOperationWithConnectedSession, DISABLED_AwaServerDeleteOperation_Perform_handles_short_timeout)
{
    // how?
}


class TestServerDeleteOperationWithConnectedSessionNoClient : public TestServerWithConnectedSession {};

TEST_F(TestServerDeleteOperationWithConnectedSessionNoClient, AwaServerDeleteOperation_NewClientIterator_handles_null)
{
    EXPECT_EQ(NULL, AwaServerDeleteOperation_NewClientIterator(NULL));
}

TEST_F(TestServerDeleteOperationWithConnectedSessionNoClient, AwaServerDeleteOperation_NewClientIterator_with_no_perform)
{
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(session_);
    // no perform
    AwaClientIterator * iterator = AwaServerDeleteOperation_NewClientIterator(operation);
    EXPECT_EQ(NULL, iterator);
    AwaServerDeleteOperation_Free(&operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSessionNoClient, AwaServerDeleteOperation_NewClientIterator_handles_no_clients)
{
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(session_);
    const char * clientID = "TestClient123";
    const char * path = "/4/0";
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, clientID, path));
    EXPECT_EQ(AwaError_Response, AwaServerDeleteOperation_Perform(operation, defaults::timeout));

    // expect the client ID to be in the response, but with an error on the specified path
    AwaClientIterator * iterator = AwaServerDeleteOperation_NewClientIterator(operation);
    EXPECT_TRUE(NULL != iterator);
    EXPECT_TRUE(AwaClientIterator_Next(iterator));
    EXPECT_STREQ(clientID, AwaClientIterator_GetClientID(iterator));
    EXPECT_FALSE(AwaClientIterator_Next(iterator));   // only one client

    // should be an error:
    const AwaServerDeleteResponse * response = AwaServerDeleteOperation_GetResponse(operation, clientID);
    ASSERT_TRUE(NULL != response);
    const AwaPathResult * pathResult = AwaServerDeleteResponse_GetPathResult(response, path);
    EXPECT_TRUE(NULL != pathResult);
    EXPECT_EQ(AwaError_ClientNotFound, AwaPathResult_GetError(pathResult));

    AwaClientIterator_Free(&iterator);
    AwaServerDeleteOperation_Free(&operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSessionNoClient, AwaServerDeleteOperation_handles_one_client)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000, CURRENT_TEST_DESCRIPTION);
    sleep(1);      // wait for the client to register with the server

    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, "TestClient1", "/4/0"));
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_Perform(operation, defaults::timeout));

    AwaClientIterator * iterator = AwaServerDeleteOperation_NewClientIterator(operation);
    EXPECT_TRUE(NULL != iterator);
    EXPECT_EQ(NULL, AwaClientIterator_GetClientID(iterator));
    EXPECT_TRUE(AwaClientIterator_Next(iterator));
    EXPECT_STREQ("TestClient1", AwaClientIterator_GetClientID(iterator));
    EXPECT_FALSE(AwaClientIterator_Next(iterator));   // only one client

    AwaClientIterator_Free(&iterator);
    AwaServerDeleteOperation_Free(&operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSessionNoClient, DISABLED_AwaServerDeleteOperation_handles_multiple_clients)
{
    // DISABLED because we don't support multiple ClientIDs in a single Delete operation at present.

    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1", "TestClient2", "TestClient3" }, 61000, CURRENT_TEST_DESCRIPTION);
    sleep(1);      // wait for the client to register with the server

    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, "TestClient1", "/4/0"));
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, "TestClient2", "/4/0"));
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, "TestClient3", "/4/0"));
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_Perform(operation, defaults::timeout));

    AwaClientIterator * iterator = AwaServerDeleteOperation_NewClientIterator(operation);
    EXPECT_TRUE(NULL != iterator);
    EXPECT_EQ(NULL, AwaClientIterator_GetClientID(iterator));

    std::vector<std::string> actualClientIDs;

    while (AwaClientIterator_Next(iterator))
    {
        const char * clientID = AwaClientIterator_GetClientID(iterator);
        actualClientIDs.push_back(clientID);
    }
    const std::vector<std::string> &expectedClientIDs = horde.GetClientIDs();

    EXPECT_EQ(expectedClientIDs.size(), actualClientIDs.size());
    if (expectedClientIDs.size() == actualClientIDs.size())
        EXPECT_TRUE(std::is_permutation(expectedClientIDs.begin(), expectedClientIDs.end(), actualClientIDs.begin()));

    AwaClientIterator_Free(&iterator);
    AwaServerDeleteOperation_Free(&operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSessionNoClient, AwaServerDeleteOperation_GetResponse_handles_null_operation)
{
    EXPECT_EQ(NULL, AwaServerDeleteOperation_GetResponse(NULL, "TestClient1"));
}

TEST_F(TestServerDeleteOperationWithConnectedSessionNoClient, AwaServerDeleteOperation_GetResponse_handles_null_clientID)
{
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(session_);
    EXPECT_TRUE(NULL != operation);
    EXPECT_EQ(NULL, AwaServerDeleteOperation_GetResponse(operation, NULL));
    AwaServerDeleteOperation_Free(&operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSessionNoClient, AwaServerDeleteOperation_GetResponse_handles_missing_clientID)
{
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(session_);
    EXPECT_TRUE(NULL != operation);
    EXPECT_EQ(NULL, AwaServerDeleteOperation_GetResponse(operation, "ClientDoesNotExist"));
    AwaServerDeleteOperation_Free(&operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSessionNoClient, AwaServerDeleteOperation_GetResponse_handles_matching_clientID)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000, CURRENT_TEST_DESCRIPTION);
    sleep(1);      // wait for the client to register with the server

    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, "TestClient1", "/4/0"));
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_Perform(operation, defaults::timeout));

    EXPECT_TRUE(NULL != operation);
    EXPECT_TRUE(NULL != AwaServerDeleteOperation_GetResponse(operation, "TestClient1"));
    AwaServerDeleteOperation_Free(&operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSessionNoClient, AwaServerDeleteResponse_NewPathIterator_handles_null_response)
{
    EXPECT_EQ(NULL, AwaServerDeleteResponse_NewPathIterator(NULL));
}

TEST_F(TestServerDeleteOperationWithConnectedSessionNoClient, AwaServerDeleteResponse_NewPathIterator_handles_single_path_response)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000, CURRENT_TEST_DESCRIPTION);
    sleep(1);      // wait for the client to register with the server

    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, "TestClient1", "/4/0"));
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_Perform(operation, defaults::timeout));
    const AwaServerDeleteResponse * response = AwaServerDeleteOperation_GetResponse(operation, "TestClient1");
    ASSERT_TRUE(NULL != response);

    AwaPathIterator * iterator = AwaServerDeleteResponse_NewPathIterator(response);
    ASSERT_TRUE(NULL != iterator);

    // should be one path
    EXPECT_FALSE(AwaPathIterator_Get(iterator));
    EXPECT_TRUE(AwaPathIterator_Next(iterator));
    EXPECT_STREQ("/4/0", AwaPathIterator_Get(iterator));
    EXPECT_FALSE(AwaPathIterator_Next(iterator));
    AwaPathIterator_Free(&iterator);

    AwaServerDeleteOperation_Free(&operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSessionNoClient, DISABLED_AwaServerDeleteResponse_NewPathIterator_handles_multiple_path_response)
{
    // NOTE: DISABLED because we don't support multiple paths per operation, presently.

    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000, CURRENT_TEST_DESCRIPTION);
    sleep(1);      // wait for the client to register with the server

    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, "TestClient1", "/4/0"));
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, "TestClient1", "/4/1"));
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, "TestClient1", "/4/2"));
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_Perform(operation, defaults::timeout));
    const AwaServerDeleteResponse * response = AwaServerDeleteOperation_GetResponse(operation, "TestClient1");
    ASSERT_TRUE(NULL != response);

    AwaPathIterator * iterator = AwaServerDeleteResponse_NewPathIterator(response);
    ASSERT_TRUE(NULL != iterator);

    // should be three paths
    std::vector<std::string> expectedPaths = { "/4/0", "/4/1", "/4/2" };
    std::vector<std::string> actualPaths;
    while (AwaPathIterator_Next(iterator))
    {
        const char * path = AwaPathIterator_Get(iterator);
        actualPaths.push_back(path);
    }

    EXPECT_EQ(expectedPaths.size(), actualPaths.size());
    if (expectedPaths.size() == actualPaths.size())
        EXPECT_TRUE(std::is_permutation(expectedPaths.begin(), expectedPaths.end(), actualPaths.begin()));

    AwaPathIterator_Free(&iterator);
    AwaServerDeleteOperation_Free(&operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSessionNoClient, AwaServerDeleteOperation_handles_success)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000, CURRENT_TEST_DESCRIPTION);
    sleep(1);      // wait for the client to register with the server

    // now try to Delete the mandatory resource:
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, "TestClient1", "/4/0"));
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_Perform(operation, defaults::timeout));

    // check response on /3/0/11
    const AwaServerDeleteResponse * response = AwaServerDeleteOperation_GetResponse(operation, "TestClient1");
    EXPECT_TRUE(NULL != response);

    const AwaPathResult * pathResult = AwaServerDeleteResponse_GetPathResult(response, "/4/0");
    EXPECT_TRUE(NULL != pathResult);
    EXPECT_EQ(AwaError_Success, AwaPathResult_GetError(pathResult));

    AwaServerDeleteOperation_Free(&operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSessionNoClient, AwaServerDeleteOperation_handles_mandatory_instance)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000, CURRENT_TEST_DESCRIPTION);
    sleep(1);      // wait for the client to register with the server

    // now try to Delete the mandatory resource:
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, "TestClient1", "/3/0"));
    EXPECT_EQ(AwaError_Response, AwaServerDeleteOperation_Perform(operation, defaults::timeout));

    // check response on /3/0/11
    const AwaServerDeleteResponse * response = AwaServerDeleteOperation_GetResponse(operation, "TestClient1");
    EXPECT_TRUE(NULL != response);

    const AwaPathResult * pathResult = AwaServerDeleteResponse_GetPathResult(response, "/3/0");
    EXPECT_TRUE(NULL != pathResult);
    EXPECT_EQ(AwaError_LWM2MError, AwaPathResult_GetError(pathResult));
    EXPECT_EQ(AwaLWM2MError_Unauthorized, AwaPathResult_GetLWM2MError(pathResult));

    AwaServerDeleteOperation_Free(&operation);
}

TEST_F(TestServerDeleteOperationWithConnectedSessionNoClient, AwaServerDeleteOperation_handles_resource)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000, CURRENT_TEST_DESCRIPTION);
    sleep(1);      // wait for the client to register with the server

    // now try to Delete the mandatory resource:
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, "TestClient1", "/3/0/1"));
    EXPECT_EQ(AwaError_Response, AwaServerDeleteOperation_Perform(operation, defaults::timeout));

    // check response on /3/0/11
    const AwaServerDeleteResponse * response = AwaServerDeleteOperation_GetResponse(operation, "TestClient1");
    EXPECT_TRUE(NULL != response);

    const AwaPathResult * pathResult = AwaServerDeleteResponse_GetPathResult(response, "/3/0/1");
    EXPECT_TRUE(NULL != pathResult);
    EXPECT_EQ(AwaError_LWM2MError, AwaPathResult_GetError(pathResult));
    EXPECT_EQ(AwaLWM2MError_MethodNotAllowed, AwaPathResult_GetLWM2MError(pathResult));

    AwaServerDeleteOperation_Free(&operation);
}

} // namespace Awa

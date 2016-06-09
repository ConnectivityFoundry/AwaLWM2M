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
#include <cstring>

#include <lwm2m_tree_node.h>

#include "awa/server.h"
#include "awa/common.h"
#include "log.h"
//#include "execute_response.h" // TODO:
#include "arrays.h"
#include "support/support.h"
#include "utils.h"
#include "memalloc.h"

namespace Awa {

class TestExecuteOperation : public TestServerBase {};

class TestExecuteOperationWithConnectedSession : public TestServerWithConnectedSession {};

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_New_returns_valid_operation_and_free_works)
{
    // test that AwaServerExecuteOperation_Free works via valgrind
    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session_);
    ASSERT_TRUE(NULL != executeOperation);
    EXPECT_EQ(AwaError_Success, AwaServerExecuteOperation_Free(&executeOperation));
}

TEST_F(TestExecuteOperation, AwaServerExecuteOperation_New_handles_null_session)
{
    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(NULL);
    ASSERT_EQ(NULL, executeOperation);
}

TEST_F(TestExecuteOperation, AwaServerExecuteOperation_New_handles_invalid_session)
{
    // An invalid session is one that is not connected
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(NULL, AwaServerExecuteOperation_New(session));

    // Set up IPC - still not enough
    AwaServerSession_SetIPCAsUDP(session, "127.0.0.1", global::serverIpcPort);
    EXPECT_EQ(NULL, AwaServerExecuteOperation_New(session));

    AwaServerSession_Free(&session);
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_Free_nulls_pointer)
{
    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session_);
    ASSERT_TRUE(NULL != executeOperation);
    EXPECT_EQ(AwaError_Success, AwaServerExecuteOperation_Free(&executeOperation));
    ASSERT_EQ(NULL, executeOperation);
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_Free_handles_null)
{
    EXPECT_EQ(AwaError_OperationInvalid, AwaServerExecuteOperation_Free(NULL));
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_Free_handles_null_pointer)
{
    AwaServerExecuteOperation * executeOperation = NULL;
    EXPECT_EQ(AwaError_OperationInvalid, AwaServerExecuteOperation_Free(&executeOperation));
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_Free_handles_invalid_session)
{
    // Session is freed before the operation (use a local session):
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session, "127.0.0.1", global::serverIpcPort));
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));
    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session);
    AwaServerSession_Free(&session);
    EXPECT_EQ(AwaError_Success, AwaServerExecuteOperation_Free(&executeOperation));
    // expect no crash or memory leaks
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_AddPath_handles_valid_path)
{
    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session_);
    ASSERT_TRUE(NULL != executeOperation);

    AwaExecuteArguments arguments {NULL, 0};
    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/1000/0/1", &arguments));
    AwaServerExecuteOperation_Free(&executeOperation);
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_AddPath_handles_valid_path_with_payload)
{
    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session_);
    ASSERT_TRUE(NULL != executeOperation);

    const char data[] = {'a','\0', 'b', 'c'};
    AwaExecuteArguments arguments = {(void*)data, sizeof(data)};
    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/1000/0/1", &arguments));
    AwaServerExecuteOperation_Free(&executeOperation);
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_AddPath_handles_invalid_paths)
{
    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session_);
    ASSERT_TRUE(NULL != executeOperation);

    AwaExecuteArguments arguments {NULL, 0};
    EXPECT_EQ(AwaError_PathInvalid, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "a/n in/valid/ path", &arguments));
    EXPECT_EQ(AwaError_PathInvalid, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/3/0", &arguments));
    EXPECT_EQ(AwaError_PathInvalid, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/3", &arguments));
    AwaServerExecuteOperation_Free(&executeOperation);
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_AddPath_handles_null_path)
{
    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session_);
    ASSERT_TRUE(NULL != executeOperation);
    AwaExecuteArguments arguments {NULL, 0};
    ASSERT_EQ(AwaError_PathInvalid, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, NULL, &arguments));
    AwaServerExecuteOperation_Free(&executeOperation);
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_AddPath_handles_null_clientID)
{
    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session_);
    ASSERT_TRUE(NULL != executeOperation);
    AwaExecuteArguments arguments {NULL, 0};
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerExecuteOperation_AddPath(executeOperation, NULL, "/1000/0/1", &arguments));
    AwaServerExecuteOperation_Free(&executeOperation);
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_AddPath_handles_null_arguments)
{
    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session_);
    ASSERT_TRUE(NULL != executeOperation);
    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/1000/0/1", NULL));
    AwaServerExecuteOperation_Free(&executeOperation);
}

TEST_F(TestExecuteOperation, AwaServerExecuteOperation_AddPath_handles_null_operation)
{
    AwaExecuteArguments arguments {NULL, 0};
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerExecuteOperation_AddPath(NULL, global::clientEndpointName, "/1000/0/1", &arguments));
}

//// FIXME: FLOWDM-370
TEST_F(TestExecuteOperationWithConnectedSession, DISABLED_AwaServerExecuteOperation_AddPath_handles_invalid_operation)
{
    // Consider if the parent session has been freed early
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session, "127.0.0.1", global::serverIpcPort));
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));
    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session);
    ASSERT_TRUE(NULL != executeOperation);

    // Free the session
    AwaServerSession_Free(&session);

    // TODO: we need the session to keep track of all operations, and
    // invalidate them when freed so they can detect this situation.

    ASSERT_EQ(AwaError_SessionInvalid, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/1000/0/1", NULL));

    EXPECT_EQ(AwaError_Success, AwaServerExecuteOperation_Free(&executeOperation));
    // expect no crash or memory leaks
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_Perform_handles_valid_operation)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session_);
    ASSERT_TRUE(NULL != executeOperation);

    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/3/0/4", NULL));
    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_Perform(executeOperation, global::timeout));
    AwaServerExecuteOperation_Free(&executeOperation);
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_Perform_handles_valid_operation_with_payload)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session_);
    ASSERT_TRUE(NULL != executeOperation);

    const char data[] = {'a','\0', 'b', 'c'};
    AwaExecuteArguments arguments = {(void*)data, sizeof(data)};
    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/3/0/4", &arguments));
    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_Perform(executeOperation, global::timeout));
    AwaServerExecuteOperation_Free(&executeOperation);
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_Perform_handles_null_operation)
{
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerExecuteOperation_Perform(NULL, global::timeout));
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_Perform_handles_negative_timeout)
{
    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session_);
    ASSERT_TRUE(NULL != executeOperation);

    ASSERT_EQ(AwaError_OperationInvalid, AwaServerExecuteOperation_Perform(executeOperation, -1));
    EXPECT_EQ(AwaError_Success, AwaServerExecuteOperation_Free(&executeOperation));
}

// FIXME: FLOWDM-360
TEST_F(TestExecuteOperationWithConnectedSession, DISABLED_AwaServerExecuteOperation_Perform_handles_zero_timeout)
{
    // how?
}

// FIXME: FLOWDM-360
TEST_F(TestExecuteOperationWithConnectedSession, DISABLED_AwaServerExecuteOperation_Perform_handles_short_timeout)
{
    // how?
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_Perform_honours_server_timeout)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session, "0.0.0.0", global::serverIpcPort));
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));
    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session);
    ASSERT_TRUE(NULL != executeOperation);
    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/3/0/4", NULL));

    // Make server unresponsive
    daemon_.Pause();

    BasicTimer timer;
    timer.Start();
    EXPECT_EQ(AwaError_Timeout, AwaServerExecuteOperation_Perform(executeOperation, global::timeout));
    timer.Stop();
    EXPECT_TRUE(ElapsedTimeExceeds(timer.TimeElapsed_Milliseconds(), global::timeout)) << "Time elapsed: " << timer.TimeElapsed_Milliseconds() << "ms";
    daemon_.Unpause();

    EXPECT_EQ(AwaError_Success, AwaServerExecuteOperation_Free(&executeOperation));
    AwaServerSession_Free(&session);
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_Perform_honours_client_timeout)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session, "0.0.0.0", global::serverIpcPort));
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));
    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session);
    ASSERT_TRUE(NULL != executeOperation);
    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/3/0/4", NULL));

    // Make client unresponsive
    horde.Pause();

    BasicTimer timer;
    timer.Start();
    EXPECT_EQ(AwaError_Timeout, AwaServerExecuteOperation_Perform(executeOperation, global::timeout));
    timer.Stop();
    EXPECT_TRUE(ElapsedTimeExceeds(timer.TimeElapsed_Milliseconds(), global::timeout)) << "Time elapsed: " << timer.TimeElapsed_Milliseconds() << "ms";
    horde.Unpause();

    EXPECT_EQ(AwaError_Success, AwaServerExecuteOperation_Free(&executeOperation));
    AwaServerSession_Free(&session);
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_on_nonexecutable_resource)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session, "0.0.0.0", global::serverIpcPort));
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));
    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session);
    ASSERT_TRUE(NULL != executeOperation);
    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/3/0/1", NULL));

    EXPECT_EQ(AwaError_Response, AwaServerExecuteOperation_Perform(executeOperation, global::timeout));
    const AwaServerExecuteResponse * response = AwaServerExecuteOperation_GetResponse(executeOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != response);
    const AwaPathResult * result = AwaServerExecuteResponse_GetPathResult(response, "/3/0/1");
    ASSERT_TRUE(NULL != result);
    EXPECT_EQ(AwaError_LWM2MError, AwaPathResult_GetError(result));
    EXPECT_EQ(AwaLWM2MError_BadRequest, AwaPathResult_GetLWM2MError(result));

    EXPECT_EQ(AwaError_Success, AwaServerExecuteOperation_Free(&executeOperation));
    AwaServerSession_Free(&session);
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_Perform_handles_invalid_operation_no_content)
{
    // Test behaviour when operation has no content
    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session_);
    ASSERT_TRUE(NULL != executeOperation);

    ASSERT_EQ(AwaError_OperationInvalid, AwaServerExecuteOperation_Perform(executeOperation, global::timeout));
    AwaServerExecuteOperation_Free(&executeOperation);
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_Perform_handles_disconnected_session)
{
    // Test behaviour when parent session has been disconnected
    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session_);
    ASSERT_TRUE(NULL != executeOperation);
    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/3/0/1", NULL));

    AwaServerSession_Disconnect(session_);

    ASSERT_EQ(AwaError_SessionNotConnected, AwaServerExecuteOperation_Perform(executeOperation, global::timeout));
    AwaServerExecuteOperation_Free(&executeOperation);
}

// FIXME: FLOWDM-370
TEST_F(TestExecuteOperationWithConnectedSession, DISABLED_AwaServerExecuteOperation_Perform_handles_invalid_operation_freed_session)
{
    // Test behaviour when parent session has been freed
    ASSERT_FALSE(1);
}


TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_GetResponse_handles_null_operation)
{
    const AwaServerExecuteResponse * executeResponse = AwaServerExecuteOperation_GetResponse(NULL, global::clientEndpointName);
    ASSERT_TRUE(NULL == executeResponse);
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_GetResponse_handles_null_clientID)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session_);
    ASSERT_TRUE(NULL != executeOperation);

    EXPECT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/3/0/4", NULL));
    EXPECT_EQ(AwaError_Success, AwaServerExecuteOperation_Perform(executeOperation, global::timeout));

    const AwaServerExecuteResponse * executeResponse = AwaServerExecuteOperation_GetResponse(executeOperation, NULL);
    EXPECT_TRUE(NULL == executeResponse);

    AwaServerExecuteOperation_Free(&executeOperation);
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_GetResponse_handles_invalid_operation)
{
    //Consider if we haven't called ProcessExecuteOperation
    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session_);
    ASSERT_TRUE(NULL != executeOperation);

    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/3/0/4", NULL));

    //haven't processed execute operation, so execute response should return NULL
    const AwaServerExecuteResponse * executeResponse = AwaServerExecuteOperation_GetResponse(executeOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL == executeResponse);

    AwaServerExecuteOperation_Free(&executeOperation);
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_GetResponse_handles_reusing_operation)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    //Test we can call AwaServerExecuteOperation_GetResponse twice and reuse the same operation
    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session_);
    ASSERT_TRUE(NULL != executeOperation);

    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/3/0/4", NULL));
    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_Perform(executeOperation, global::timeout));

    const AwaServerExecuteResponse * executeResponse = AwaServerExecuteOperation_GetResponse(executeOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != executeResponse);

    const AwaServerExecuteResponse * executeResponse2 = AwaServerExecuteOperation_GetResponse(executeOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != executeResponse2);

    AwaServerExecuteOperation_Free(&executeOperation);
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteOperation_GetResponse_handles_missing_clientID)
{
    AwaServerExecuteOperation * operation = AwaServerExecuteOperation_New(session_);
    EXPECT_TRUE(NULL != operation);
    EXPECT_EQ(NULL, AwaServerExecuteOperation_GetResponse(operation, "ClientDoesNotExist"));
    AwaServerExecuteOperation_Free(&operation);
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteResponse_ContainsPath_handles_null_response)
{
    ASSERT_FALSE(AwaServerExecuteResponse_ContainsPath(NULL, "/3/0/1"));
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteResponse_ContainsPath_handles_null_path)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session_);
    ASSERT_TRUE(NULL != executeOperation);
    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/3/0/4", NULL));
    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_Perform(executeOperation, global::timeout));
    const AwaServerExecuteResponse * executeResponse = AwaServerExecuteOperation_GetResponse(executeOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != executeResponse);
    ASSERT_FALSE(AwaServerExecuteResponse_ContainsPath(executeResponse, NULL));
    AwaServerExecuteOperation_Free(&executeOperation);
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteResponse_ContainsPath_handles_valid_resource_path)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session_);
    ASSERT_TRUE(NULL != executeOperation);
    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/3/0/4", NULL));
    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_Perform(executeOperation, global::timeout));
    const AwaServerExecuteResponse * executeResponse = AwaServerExecuteOperation_GetResponse(executeOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != executeResponse);
    ASSERT_TRUE(AwaServerExecuteResponse_ContainsPath(executeResponse, "/3/0/4"));
    AwaServerExecuteOperation_Free(&executeOperation);
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteResponse_ContainsPath_handles_missing_resource_path)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session_);
    ASSERT_TRUE(NULL != executeOperation);
    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/3/0/4", NULL));
    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_Perform(executeOperation, global::timeout));
    const AwaServerExecuteResponse * executeResponse = AwaServerExecuteOperation_GetResponse(executeOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != executeResponse);
    ASSERT_FALSE(AwaServerExecuteResponse_ContainsPath(executeResponse, "/3/0/2"));
    AwaServerExecuteOperation_Free(&executeOperation);
}

TEST_F(TestExecuteOperation, AwaServerExecuteResponse_NewPathIterator_handles_null_response)
{
    EXPECT_EQ(NULL, AwaServerExecuteResponse_NewPathIterator(NULL));
}

TEST_F(TestExecuteOperationWithConnectedSession, AwaServerExecuteResponse_NewPathIterator_handles_valid_response)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session_);
    ASSERT_TRUE(NULL != executeOperation);
    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/3/0/4", NULL));
    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_Perform(executeOperation, global::timeout));
    const AwaServerExecuteResponse * executeResponse = AwaServerExecuteOperation_GetResponse(executeOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != executeResponse);

    AwaPathIterator * iterator = AwaServerExecuteResponse_NewPathIterator(executeResponse);
    ASSERT_TRUE(NULL != iterator);

    // call Next once before executeting values:
    EXPECT_EQ(true, AwaPathIterator_Next(iterator));
    EXPECT_STREQ("/3/0/4", AwaPathIterator_Get(iterator));

    // not expecting any more paths
    EXPECT_TRUE(false == AwaPathIterator_Next(iterator));
    EXPECT_TRUE(NULL == AwaPathIterator_Get(iterator));

    AwaPathIterator_Free(&iterator);
    EXPECT_EQ(NULL, iterator);
    AwaServerExecuteOperation_Free(&executeOperation);
}

// TODO: support execution of multiple resources in a single request
TEST_F(TestExecuteOperationWithConnectedSession, DISABLED_AwaServerExecuteResponse_NewPathIterator_handles_valid_response_multiple_paths)
{
    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session_); ASSERT_TRUE(NULL != executeOperation);
    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/3/0/0", NULL));
    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/3/0/1", NULL));
    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/3/0/9", NULL));
    ASSERT_EQ(AwaError_Success, AwaServerExecuteOperation_Perform(executeOperation, global::timeout));
    const AwaServerExecuteResponse * executeResponse = AwaServerExecuteOperation_GetResponse(executeOperation, global::clientEndpointName); ASSERT_TRUE(NULL != executeResponse);

    AwaPathIterator * iterator = AwaServerExecuteResponse_NewPathIterator(executeResponse);
    ASSERT_TRUE(NULL != iterator);

    // call Next once before executing values:
    EXPECT_EQ(true, AwaPathIterator_Next(iterator));
    EXPECT_STREQ("/3/0/0", AwaPathIterator_Get(iterator));

    EXPECT_EQ(true, AwaPathIterator_Next(iterator));
    EXPECT_STREQ("/3/0/1", AwaPathIterator_Get(iterator));

    EXPECT_EQ(true, AwaPathIterator_Next(iterator));
    EXPECT_STREQ("/3/0/9", AwaPathIterator_Get(iterator));

    // not expecting any more paths
    EXPECT_TRUE(false == AwaPathIterator_Next(iterator));
    EXPECT_TRUE(NULL == AwaPathIterator_Get(iterator));

    AwaPathIterator_Free(&iterator);
    AwaServerExecuteOperation_Free(&executeOperation);
}

class TestExecuteOperationWithConnectedSessionNoClient : public TestServerWithConnectedSession {};

TEST_F(TestExecuteOperationWithConnectedSessionNoClient, AwaServerExecuteOperation_NewClientIterator_handles_null)
{
    EXPECT_EQ(NULL, AwaServerExecuteOperation_NewClientIterator(NULL));
}

TEST_F(TestExecuteOperationWithConnectedSessionNoClient, AwaServerExecuteOperation_NewClientIterator_with_no_perform)
{
    AwaServerExecuteOperation * operation = AwaServerExecuteOperation_New(session_);
    // no perform
    AwaClientIterator * iterator = AwaServerExecuteOperation_NewClientIterator(operation);
    EXPECT_EQ(NULL, iterator);
    AwaServerExecuteOperation_Free(&operation);
}

TEST_F(TestExecuteOperationWithConnectedSessionNoClient, AwaServerExecuteOperation_NewClientIterator_handles_no_clients)
{
    AwaServerExecuteOperation * operation = AwaServerExecuteOperation_New(session_);
    const char * clientID = "TestClient123";
    const char * path = "/3/0/4";
    EXPECT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(operation, clientID, path, NULL));
    EXPECT_EQ(AwaError_Response, AwaServerExecuteOperation_Perform(operation, global::timeout));

    // expect the client ID to be in the response, but with an error on the specified path
    AwaClientIterator * iterator = AwaServerExecuteOperation_NewClientIterator(operation);
    EXPECT_TRUE(NULL != iterator);
    EXPECT_TRUE(AwaClientIterator_Next(iterator));
    EXPECT_STREQ(clientID, AwaClientIterator_GetClientID(iterator));
    EXPECT_FALSE(AwaClientIterator_Next(iterator));   // only one client

    // should be an error:
    const AwaServerExecuteResponse * response = AwaServerExecuteOperation_GetResponse(operation, clientID);
    ASSERT_TRUE(NULL != response);
    const AwaPathResult * pathResult = AwaServerExecuteResponse_GetPathResult(response, path);
    EXPECT_TRUE(NULL != pathResult);
    EXPECT_EQ(AwaError_ClientNotFound, AwaPathResult_GetError(pathResult));

    AwaClientIterator_Free(&iterator);
    AwaServerExecuteOperation_Free(&operation);
}

TEST_F(TestExecuteOperationWithConnectedSessionNoClient, AwaServerExecuteOperation_handles_one_client)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaServerExecuteOperation * operation = AwaServerExecuteOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(operation, "TestClient1", "/3/0/4", NULL));
    EXPECT_EQ(AwaError_Success, AwaServerExecuteOperation_Perform(operation, global::timeout));

    AwaClientIterator * iterator = AwaServerExecuteOperation_NewClientIterator(operation);
    EXPECT_TRUE(NULL != iterator);
    EXPECT_EQ(NULL, AwaClientIterator_GetClientID(iterator));
    EXPECT_TRUE(AwaClientIterator_Next(iterator));
    EXPECT_STREQ("TestClient1", AwaClientIterator_GetClientID(iterator));
    EXPECT_FALSE(AwaClientIterator_Next(iterator));   // only one client

    AwaClientIterator_Free(&iterator);
    AwaServerExecuteOperation_Free(&operation);
}

TEST_F(TestExecuteOperationWithConnectedSessionNoClient, DISABLED_AwaServerExecuteOperation_handles_multiple_clients)
{
    // DISABLED because we don't support multiple ClientIDs in a single Execute operation at present.

    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1", "TestClient2", "TestClient3" }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaServerExecuteOperation * operation = AwaServerExecuteOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(operation, "TestClient1", "/3/0/0", NULL));
    EXPECT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(operation, "TestClient2", "/3/0/0", NULL));
    EXPECT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(operation, "TestClient3", "/3/0/0", NULL));
    EXPECT_EQ(AwaError_Success, AwaServerExecuteOperation_Perform(operation, global::timeout));

    AwaClientIterator * iterator = AwaServerExecuteOperation_NewClientIterator(operation);
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
    AwaServerExecuteOperation_Free(&operation);
}



} // namespace Awa

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
#include <vector>
#include <tuple>
#include <algorithm>

#include "support/support.h"
#include "support/xml_support.h"

#include "awa/server.h"
#include "log.h"

namespace Awa {

class TestWriteAttributesOperation : public TestServerBase {};
class TestWriteAttributesOperationWithServerDaemon : public TestServerWithDaemonBase {};
class TestWriteAttributesOperationWithConnectedSession : public TestServerWithConnectedSession {};

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_New_returns_valid_operation_and_free_works)
{
    // test that AwaServerWriteAttributesOperation_Free works via valgrind
    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_TRUE(NULL != writeAttributesOperation);
    EXPECT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_Free(&writeAttributesOperation));
}

TEST_F(TestWriteAttributesOperation, AwaServerWriteAttributesOperation_New_handles_null_session)
{
    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(NULL);
    ASSERT_EQ(NULL, writeAttributesOperation);
}

TEST_F(TestWriteAttributesOperation, AwaServerWriteAttributesOperation_New_handles_invalid_session)
{
    // An invalid session is one that is not connected
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(NULL, AwaServerWriteAttributesOperation_New(session));

    // Set up IPC - still not enough
    AwaServerSession_SetIPCAsUDP(session, "127.0.0.1", global::serverIpcPort);
    EXPECT_EQ(NULL, AwaServerWriteAttributesOperation_New(session));

    AwaServerSession_Free(&session);
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_Free_nulls_pointer)
{
    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_TRUE(NULL != writeAttributesOperation);
    EXPECT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_Free(&writeAttributesOperation));
    ASSERT_EQ(NULL, writeAttributesOperation);
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_Free_handles_null)
{
    EXPECT_EQ(AwaError_OperationInvalid, AwaServerWriteAttributesOperation_Free(NULL));
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_Free_handles_null_pointer)
{
    AwaServerWriteAttributesOperation * writeAttributesOperation = NULL;
    EXPECT_EQ(AwaError_OperationInvalid, AwaServerWriteAttributesOperation_Free(&writeAttributesOperation));
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_Free_handles_invalid_session)
{
    // Session is freed before the operation (use a local session):
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session, "127.0.0.1", global::serverIpcPort));
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));
    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session);
    AwaServerSession_Free(&session);
    EXPECT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_Free(&writeAttributesOperation));
    // expect no crash or memory leaks
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_AddAttributeAsInteger_handles_valid_path)
{
    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_TRUE(NULL != writeAttributesOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/13", "gt", 10));
    AwaServerWriteAttributesOperation_Free(&writeAttributesOperation);
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_AddAttributeAsInteger_handles_null_operation)
{
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerWriteAttributesOperation_AddAttributeAsInteger(NULL, global::clientEndpointName, "/3/0/13", "gt", 10));
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_AddAttributeAsInteger_handles_null_clientID)
{
    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_TRUE(NULL != writeAttributesOperation);

    ASSERT_EQ(AwaError_TypeMismatch, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, NULL, "/3/0/13", "gt", 10));
    AwaServerWriteAttributesOperation_Free(&writeAttributesOperation);
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_AddAttributeAsInteger_handles_invalid_path)
{
    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_TRUE(NULL != writeAttributesOperation);

    ASSERT_EQ(AwaError_PathInvalid, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/an_ invalid .path/0", "gt", 10));
    AwaServerWriteAttributesOperation_Free(&writeAttributesOperation);
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_AddAttributeAsInteger_handles_null_link)
{
    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_TRUE(NULL != writeAttributesOperation);

    ASSERT_EQ(AwaError_TypeMismatch, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/13", NULL, 10));
    AwaServerWriteAttributesOperation_Free(&writeAttributesOperation);
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, DISABLED_AwaServerWriteAttributesOperation_AddAttributeAsInteger_handles_invalid_link)
{
    // TODO: How to test this?
    ASSERT_TRUE(false);
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_Perform_handles_valid_operation_single_attribute)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    sleep(1);      // wait for the client to register with the server

    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_TRUE(NULL != writeAttributesOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/13", "gt", 10));

    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_Perform(writeAttributesOperation, defaults::timeout));
    AwaServerWriteAttributesOperation_Free(&writeAttributesOperation);
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_Perform_handles_valid_operation_multiple_attributes)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    sleep(1);      // wait for the client to register with the server

    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_TRUE(NULL != writeAttributesOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/13", "gt", 10));
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/13", "lt", -10));

    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_Perform(writeAttributesOperation, defaults::timeout));
    AwaServerWriteAttributesOperation_Free(&writeAttributesOperation);
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_Perform_handles_setting_invalid_pmin_pmax)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    sleep(1);      // wait for the client to register with the server

    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_TRUE(NULL != writeAttributesOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/13", "pmin", 10));
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/13", "pmax", 5));

    ASSERT_EQ(AwaError_Response, AwaServerWriteAttributesOperation_Perform(writeAttributesOperation, defaults::timeout));
    AwaServerWriteAttributesOperation_Free(&writeAttributesOperation);
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_Perform_handles_setting_valid_pmin_pmax)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    sleep(1);      // wait for the client to register with the server

    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_TRUE(NULL != writeAttributesOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/13", "pmin", 5));
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/13", "pmax", 10));

    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_Perform(writeAttributesOperation, defaults::timeout));
    AwaServerWriteAttributesOperation_Free(&writeAttributesOperation);
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_Perform_handles_invalid_operation_no_attributes)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    sleep(1);      // wait for the client to register with the server

    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_TRUE(NULL != writeAttributesOperation);

    ASSERT_EQ(AwaError_OperationInvalid, AwaServerWriteAttributesOperation_Perform(writeAttributesOperation, defaults::timeout));
    AwaServerWriteAttributesOperation_Free(&writeAttributesOperation);
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_Perform_handles_null_operation)
{
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerWriteAttributesOperation_Perform(NULL, defaults::timeout));
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_Perform_handles_negative_timeout)
{
    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_TRUE(NULL != writeAttributesOperation);
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/13", "gt", 10));

    ASSERT_EQ(AwaError_OperationInvalid, AwaServerWriteAttributesOperation_Perform(writeAttributesOperation, -1));
    EXPECT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_Free(&writeAttributesOperation));
}

//// FIXME: FLOWDM-360
//TEST_F(TestWriteAttributesOperationWithConnectedSession, DISABLED_AwaServerWriteAttributesOperation_Perform_handles_zero_timeout)
//{
//    // how?
//}
//
//// FIXME: FLOWDM-360
//TEST_F(TestWriteAttributesOperationWithConnectedSession, DISABLED_AwaServerWriteAttributesOperation_Perform_handles_short_timeout)
//{
//    // how?
//}
//
TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_Perform_honours_timeout)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    sleep(1);      // wait for the client to register with the server

    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session, "0.0.0.0", global::serverIpcPort));
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));
    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session);
    ASSERT_TRUE(NULL != writeAttributesOperation);
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/13", "gt", 10));

    // Tear down server and connected client
    TestWriteAttributesOperationWithConnectedSession::TearDown();

    BasicTimer timer;
    timer.Start();
    EXPECT_EQ(AwaError_Timeout, AwaServerWriteAttributesOperation_Perform(writeAttributesOperation, defaults::timeout));
    timer.Stop();
    EXPECT_TRUE(ElapsedTimeWithinTolerance(timer.TimeElapsed_Milliseconds(), defaults::timeout, defaults::timeoutTolerance)) << "Time elapsed: " << timer.TimeElapsed_Milliseconds() << "ms";

    EXPECT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_Free(&writeAttributesOperation));
    AwaServerSession_Free(&session);
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_Perform_handles_disconnected_session)
{
    // Test behaviour when parent session has been disconnected
    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_TRUE(NULL != writeAttributesOperation);
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/13", "gt", 10));

    AwaServerSession_Disconnect(session_);

    ASSERT_EQ(AwaError_SessionNotConnected, AwaServerWriteAttributesOperation_Perform(writeAttributesOperation, defaults::timeout));
    AwaServerWriteAttributesOperation_Free(&writeAttributesOperation);
}

// FIXME: FLOWDM-370
TEST_F(TestWriteAttributesOperationWithConnectedSession, DISABLED_AwaServerWriteAttributesOperation_Perform_handles_invalid_operation_freed_session)
{
    // Test behaviour when parent session has been freed
    ASSERT_FALSE(1);
}


TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_GetResponse_handles_null_operation)
{
    const AwaServerWriteAttributesResponse * writeAttributesResponse = AwaServerWriteAttributesOperation_GetResponse(NULL, global::clientEndpointName);
    ASSERT_TRUE(NULL == writeAttributesResponse);
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_GetResponse_handles_null_clientID)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    sleep(1);      // wait for the client to register with the server

    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_TRUE(NULL != writeAttributesOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/13", "gt", 10));
    EXPECT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_Perform(writeAttributesOperation, defaults::timeout));

    const AwaServerWriteAttributesResponse * writeAttributesResponse = AwaServerWriteAttributesOperation_GetResponse(writeAttributesOperation, NULL);
    EXPECT_TRUE(NULL == writeAttributesResponse);

    AwaServerWriteAttributesOperation_Free(&writeAttributesOperation);
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_GetResponse_handles_invalid_operation)
{
    //Consider if we haven't called ProcessWriteAttributesOperation
    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_TRUE(NULL != writeAttributesOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/13", "gt", 10));

    //haven't processed writeAttributes operation, so writeAttributes response should return NULL
    const AwaServerWriteAttributesResponse * writeAttributesResponse = AwaServerWriteAttributesOperation_GetResponse(writeAttributesOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL == writeAttributesResponse);

    AwaServerWriteAttributesOperation_Free(&writeAttributesOperation);
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_GetResponse_handles_reusing_operation)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    sleep(1);      // wait for the client to register with the server

    //Test we can call AwaServerWriteAttributesOperation_GetResponse twice and reuse the same operation
    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_TRUE(NULL != writeAttributesOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/13", "gt", 10));
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_Perform(writeAttributesOperation, defaults::timeout));

    const AwaServerWriteAttributesResponse * writeAttributesResponse = AwaServerWriteAttributesOperation_GetResponse(writeAttributesOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != writeAttributesResponse);

    const AwaServerWriteAttributesResponse * writeAttributesResponse2 = AwaServerWriteAttributesOperation_GetResponse(writeAttributesOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != writeAttributesResponse2);

    AwaServerWriteAttributesOperation_Free(&writeAttributesOperation);
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesOperation_GetResponse_handles_missing_clientID)
{
    AwaServerWriteAttributesOperation * operation = AwaServerWriteAttributesOperation_New(session_);
    EXPECT_TRUE(NULL != operation);
    EXPECT_EQ(NULL, AwaServerWriteAttributesOperation_GetResponse(operation, "ClientDoesNotExist"));
    AwaServerWriteAttributesOperation_Free(&operation);
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesResponse_ContainsPath_handles_null_response)
{
    ASSERT_FALSE(AwaServerWriteAttributesResponse_ContainsPath(NULL, "/3/0/1"));
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesResponse_ContainsPath_handles_null_path)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    sleep(1);      // wait for the client to register with the server

    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_TRUE(NULL != writeAttributesOperation);
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/13", "gt", 10));
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_Perform(writeAttributesOperation, defaults::timeout));
    const AwaServerWriteAttributesResponse * writeAttributesResponse = AwaServerWriteAttributesOperation_GetResponse(writeAttributesOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != writeAttributesResponse);
    ASSERT_FALSE(AwaServerWriteAttributesResponse_ContainsPath(writeAttributesResponse, NULL));
    AwaServerWriteAttributesOperation_Free(&writeAttributesOperation);
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesResponse_ContainsPath_handles_valid_resource_path)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    sleep(1);      // wait for the client to register with the server

    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_TRUE(NULL != writeAttributesOperation);
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/13", "gt", 10));
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_Perform(writeAttributesOperation, defaults::timeout));
    const AwaServerWriteAttributesResponse * writeAttributesResponse = AwaServerWriteAttributesOperation_GetResponse(writeAttributesOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != writeAttributesResponse);
    ASSERT_TRUE(AwaServerWriteAttributesResponse_ContainsPath(writeAttributesResponse, "/3/0/13"));
    AwaServerWriteAttributesOperation_Free(&writeAttributesOperation);
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesResponse_ContainsPath_handles_missing_resource_path)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    sleep(1);      // wait for the client to register with the server

    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_TRUE(NULL != writeAttributesOperation);
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/13", "gt", 10));
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_Perform(writeAttributesOperation, defaults::timeout));
    const AwaServerWriteAttributesResponse * writeAttributesResponse = AwaServerWriteAttributesOperation_GetResponse(writeAttributesOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != writeAttributesResponse);
    ASSERT_FALSE(AwaServerWriteAttributesResponse_ContainsPath(writeAttributesResponse, "/3/0/2"));
    AwaServerWriteAttributesOperation_Free(&writeAttributesOperation);
}

TEST_F(TestWriteAttributesOperation, AwaServerWriteAttributesResponse_NewPathIterator_handles_null_response)
{
    EXPECT_EQ(NULL, AwaServerWriteAttributesResponse_NewPathIterator(NULL));
}

TEST_F(TestWriteAttributesOperationWithConnectedSession, AwaServerWriteAttributesResponse_NewPathIterator_handles_valid_response)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    sleep(1);      // wait for the client to register with the server

    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_TRUE(NULL != writeAttributesOperation);
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/13", "gt", 10));
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_Perform(writeAttributesOperation, defaults::timeout));
    const AwaServerWriteAttributesResponse * writeAttributesResponse = AwaServerWriteAttributesOperation_GetResponse(writeAttributesOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != writeAttributesResponse);

    AwaPathIterator * iterator = AwaServerWriteAttributesResponse_NewPathIterator(writeAttributesResponse);
    ASSERT_TRUE(NULL != iterator);

    // call Next once before writeAttributesting values:
    EXPECT_EQ(true, AwaPathIterator_Next(iterator));
    EXPECT_STREQ("/3/0/13", AwaPathIterator_Get(iterator));

    // not expecting any more paths
    EXPECT_TRUE(false == AwaPathIterator_Next(iterator));
    EXPECT_TRUE(NULL == AwaPathIterator_Get(iterator));

    AwaPathIterator_Free(&iterator);
    EXPECT_EQ(NULL, iterator);
    AwaServerWriteAttributesOperation_Free(&writeAttributesOperation);
}

// TODO: support writing attributes of multiple resources in a single request
TEST_F(TestWriteAttributesOperationWithConnectedSession, DISABLED_AwaServerWriteAttributesResponse_NewPathIterator_handles_valid_response_multiple_paths)
{
    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_); ASSERT_TRUE(NULL != writeAttributesOperation);
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/12", "gt", 10));
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/13", "gt", 10));
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_Perform(writeAttributesOperation, defaults::timeout));
    const AwaServerWriteAttributesResponse * writeAttributesResponse = AwaServerWriteAttributesOperation_GetResponse(writeAttributesOperation, global::clientEndpointName); ASSERT_TRUE(NULL != writeAttributesResponse);

    AwaPathIterator * iterator = AwaServerWriteAttributesResponse_NewPathIterator(writeAttributesResponse);
    ASSERT_TRUE(NULL != iterator);

    // call Next once before executing values:
    EXPECT_EQ(true, AwaPathIterator_Next(iterator));
    EXPECT_STREQ("/3/0/12", AwaPathIterator_Get(iterator));

    EXPECT_EQ(true, AwaPathIterator_Next(iterator));
    EXPECT_STREQ("/3/0/13", AwaPathIterator_Get(iterator));

    // not expecting any more paths
    EXPECT_TRUE(false == AwaPathIterator_Next(iterator));
    EXPECT_TRUE(NULL == AwaPathIterator_Get(iterator));

    AwaPathIterator_Free(&iterator);
    AwaServerWriteAttributesOperation_Free(&writeAttributesOperation);
}

class TestWriteAttributesOperationWithConnectedSessionNoClient : public TestServerWithConnectedSession {};

TEST_F(TestWriteAttributesOperationWithConnectedSessionNoClient, AwaServerWriteAttributesOperation_NewClientIterator_handles_null)
{
    EXPECT_EQ(NULL, AwaServerWriteAttributesOperation_NewClientIterator(NULL));
}

TEST_F(TestWriteAttributesOperationWithConnectedSessionNoClient, AwaServerWriteAttributesOperation_NewClientIterator_with_no_perform)
{
    AwaServerWriteAttributesOperation * operation = AwaServerWriteAttributesOperation_New(session_);
    // no perform
    AwaClientIterator * iterator = AwaServerWriteAttributesOperation_NewClientIterator(operation);
    EXPECT_EQ(NULL, iterator);
    AwaServerWriteAttributesOperation_Free(&operation);
}

TEST_F(TestWriteAttributesOperationWithConnectedSessionNoClient, AwaServerWriteAttributesOperation_NewClientIterator_handles_no_clients)
{
    AwaServerWriteAttributesOperation * operation = AwaServerWriteAttributesOperation_New(session_);
    const char * clientID = "TestClient123";
    const char * path = "/3/0/13";
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(operation, clientID, path, "gt", 10));
    EXPECT_EQ(AwaError_Response, AwaServerWriteAttributesOperation_Perform(operation, defaults::timeout));

    // expect the client ID to be in the response, but with an error on the specified path
    AwaClientIterator * iterator = AwaServerWriteAttributesOperation_NewClientIterator(operation);
    EXPECT_TRUE(NULL != iterator);
    EXPECT_TRUE(AwaClientIterator_Next(iterator));
    EXPECT_STREQ(clientID, AwaClientIterator_GetClientID(iterator));
    EXPECT_FALSE(AwaClientIterator_Next(iterator));   // only one client

    // should be an error:
    const AwaServerWriteAttributesResponse * response = AwaServerWriteAttributesOperation_GetResponse(operation, clientID);
    ASSERT_TRUE(NULL != response);
    const AwaPathResult * pathResult = AwaServerWriteAttributesResponse_GetPathResult(response, path);
    EXPECT_TRUE(NULL != pathResult);
    EXPECT_EQ(AwaError_ClientNotFound, AwaPathResult_GetError(pathResult));

    AwaClientIterator_Free(&iterator);
    AwaServerWriteAttributesOperation_Free(&operation);
}

TEST_F(TestWriteAttributesOperationWithConnectedSessionNoClient, AwaServerWriteAttributesOperation_handles_one_client)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    sleep(1);      // wait for the client to register with the server

    AwaServerWriteAttributesOperation * operation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(operation, global::clientEndpointName, "/3/0/13", "gt", 10));
    EXPECT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_Perform(operation, defaults::timeout));

    AwaClientIterator * iterator = AwaServerWriteAttributesOperation_NewClientIterator(operation);
    EXPECT_TRUE(NULL != iterator);
    EXPECT_EQ(NULL, AwaClientIterator_GetClientID(iterator));
    EXPECT_TRUE(AwaClientIterator_Next(iterator));
    EXPECT_STREQ(global::clientEndpointName, AwaClientIterator_GetClientID(iterator));
    EXPECT_FALSE(AwaClientIterator_Next(iterator));   // only one client

    AwaClientIterator_Free(&iterator);
    AwaServerWriteAttributesOperation_Free(&operation);
}

TEST_F(TestWriteAttributesOperationWithConnectedSessionNoClient, DISABLED_AwaServerWriteAttributesOperation_handles_multiple_clients)
{
    // DISABLED because we don't support multiple ClientIDs in a single WriteAttributes operation at present.

    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1", "TestClient2", "TestClient3" }, 61000);
    sleep(1);      // wait for the client to register with the server

    AwaServerWriteAttributesOperation * operation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(operation, "TestClient1", "/3/0/13", "gt", 10));
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(operation, "TestClient2", "/3/0/13", "gt", 10));
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(operation, "TestClient3", "/3/0/13", "gt", 10));

    EXPECT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_Perform(operation, defaults::timeout));

    AwaClientIterator * iterator = AwaServerWriteAttributesOperation_NewClientIterator(operation);
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
    AwaServerWriteAttributesOperation_Free(&operation);
}


} // namespace Awa


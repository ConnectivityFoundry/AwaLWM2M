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
#include <memory>
#include <algorithm>
#include <unistd.h>

#include "support/support.h"
#include "awa/server.h"

namespace Awa {

class TestListClientsOperation : public TestServerBase {};

class TestListClientsOperationWithConnectedSession : public TestServerWithConnectedSession {};

TEST_F(TestListClientsOperationWithConnectedSession, AwaServerListClientsOperation_New_and_Free_work)
{
    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaServerListClientsOperation_Free(&operation));
}

TEST_F(TestListClientsOperation, AwaServerListClientsOperation_New_handles_null_session)
{
    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(NULL);
    ASSERT_EQ(NULL, operation);
}

TEST_F(TestListClientsOperation, AwaServerListClientsOperation_New_handles_invalid_session)
{
    // An invalid session is one that is not connected
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(NULL, AwaServerListClientsOperation_New(session));

    // set up IPC - still not enough
    AwaServerSession_SetIPCAsUDP(session, "127.0.0.1", global::serverIpcPort);
    EXPECT_EQ(NULL, AwaServerListClientsOperation_New(session));

    AwaServerSession_Free(&session);
}

TEST_F(TestListClientsOperationWithConnectedSession, AwaServerListClientsOperation_Free_nulls_pointer)
{
    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaServerListClientsOperation_Free(&operation));
    ASSERT_EQ(NULL, operation);
}

TEST_F(TestListClientsOperationWithConnectedSession, AwaServerListClientsOperation_Free_handles_null)
{
    EXPECT_EQ(AwaError_OperationInvalid, AwaServerListClientsOperation_Free(NULL));

}

TEST_F(TestListClientsOperationWithConnectedSession, AwaServerListClientsOperation_Free_handles_null_pointer)
{
    AwaServerListClientsOperation * operation = NULL;
    EXPECT_EQ(AwaError_OperationInvalid, AwaServerListClientsOperation_Free(&operation));
}

TEST_F(TestListClientsOperationWithConnectedSession, AwaServerListClientsOperation_Free_handles_invalid_session)
{
    // Session is freed before the operation (use a local session):
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session, "127.0.0.1", global::serverIpcPort));
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));
    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session);
    AwaServerSession_Free(&session);
    EXPECT_EQ(AwaError_Success, AwaServerListClientsOperation_Free(&operation));
    // expect no crash or memory leaks
}

TEST_F(TestListClientsOperation, AwaServerListClientsOperation_Perform_handles_null_operation)
{
    EXPECT_EQ(AwaError_OperationInvalid, AwaServerListClientsOperation_Perform(NULL, global::timeout));
}

TEST_F(TestListClientsOperationWithConnectedSession, AwaServerListClientsOperation_Perform_handles_negative_timeout)
{
    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_EQ(AwaError_OperationInvalid, AwaServerListClientsOperation_Perform(operation, -1));
    AwaServerListClientsOperation_Free(&operation);
}

//DISABLED_AwaServerListClientsOperation_Perform_handles_zero_timeout

//DISABLED_AwaServerListClientsOperation_Perform_handles_short_timeout

TEST_F(TestListClientsOperationWithConnectedSession, AwaServerListClientsOperation_NewClientIterator_handles_null)
{
    EXPECT_EQ(NULL, AwaServerListClientsOperation_NewClientIterator(NULL));
}

TEST_F(TestListClientsOperationWithConnectedSession, AwaServerListClientsOperation_NewClientIterator_with_no_perform)
{
    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    // no perform
    AwaClientIterator * iterator = AwaServerListClientsOperation_NewClientIterator(operation);
    EXPECT_EQ(NULL, iterator);
    AwaServerListClientsOperation_Free(&operation);
}

TEST_F(TestListClientsOperationWithConnectedSession, AwaServerListClientsOperation_NewClientIterator_with_no_clients)
{
    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaServerListClientsOperation_Perform(operation, global::timeout));
    AwaClientIterator * iterator = AwaServerListClientsOperation_NewClientIterator(operation);
    EXPECT_TRUE(NULL != iterator);
    EXPECT_FALSE(AwaClientIterator_Next(iterator));
    AwaClientIterator_Free(&iterator);
    AwaServerListClientsOperation_Free(&operation);
}

TEST_F(TestListClientsOperationWithConnectedSession, AwaServerListClientsOperation_NewClientIterator_with_one_client)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaServerListClientsOperation_Perform(operation, global::timeout));
    AwaClientIterator * iterator = AwaServerListClientsOperation_NewClientIterator(operation);
    EXPECT_TRUE(NULL != iterator);

    // before first Next:
    EXPECT_EQ(NULL, AwaClientIterator_GetClientID(iterator));

    EXPECT_TRUE(AwaClientIterator_Next(iterator));

    EXPECT_STREQ("TestClient1", AwaClientIterator_GetClientID(iterator));

    AwaClientIterator_Free(&iterator);
    AwaServerListClientsOperation_Free(&operation);
}

TEST_F(TestListClientsOperationWithConnectedSession, AwaServerListClientsOperation_Perform_honours_timeout)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session, "0.0.0.0", 61000));
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));

    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session);

    // make client unresponsive
    horde.Pause();
    BasicTimer timer;
    timer.Start();
    EXPECT_EQ(AwaError_Timeout, AwaServerListClientsOperation_Perform(operation, global::timeout));
    timer.Stop();
    EXPECT_TRUE(ElapsedTimeExceeds(timer.TimeElapsed_Milliseconds(), global::timeout)) << "Time elapsed: " << timer.TimeElapsed_Milliseconds() << "ms";
    horde.Unpause();

    AwaServerListClientsOperation_Free(&operation);
    AwaServerSession_Free(&session);
}

TEST_F(TestListClientsOperationWithConnectedSession, AwaServerListClientsOperation_NewClientIterator_with_multiple_clients)
{
    // start a client horde and wait for them to register with the server
    AwaClientDaemonHorde horde( { "IMG123", "TestClient1", "TestClient2", "Imagination0" }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaServerListClientsOperation_Perform(operation, global::timeout));
    AwaClientIterator * iterator = AwaServerListClientsOperation_NewClientIterator(operation);
    EXPECT_TRUE(NULL != iterator);

    // before first Next:
    EXPECT_EQ(NULL, AwaClientIterator_GetClientID(iterator));

    std::vector<std::string> resultClientIDs;
    while (AwaClientIterator_Next(iterator))
    {
        const char * clientID = AwaClientIterator_GetClientID(iterator);
        EXPECT_TRUE(NULL != clientID);
        resultClientIDs.push_back(clientID);
    }

    std::vector<std::string> expectedClientIDs = horde.GetClientIDs();
    EXPECT_EQ(expectedClientIDs.size(), resultClientIDs.size());
    if (expectedClientIDs.size() == resultClientIDs.size())
        EXPECT_TRUE(std::is_permutation(expectedClientIDs.begin(), expectedClientIDs.end(), resultClientIDs.begin()));

    AwaClientIterator_Free(&iterator);
    AwaServerListClientsOperation_Free(&operation);
}

TEST_F(TestListClientsOperationWithConnectedSession, AwaServerListClientsOperation_GetResponse_handles_null)
{
    EXPECT_EQ(NULL, AwaServerListClientsOperation_GetResponse(NULL, global::clientEndpointName));
}

TEST_F(TestListClientsOperationWithConnectedSession, AwaServerListClientsOperation_GetResponse_handles_null_clientID)
{
    EXPECT_EQ(NULL, AwaServerListClientsOperation_GetResponse(NULL, NULL));

    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaServerListClientsOperation_Perform(operation, global::timeout));

    EXPECT_EQ(NULL, AwaServerListClientsOperation_GetResponse(operation, NULL));

    AwaServerListClientsOperation_Free(&operation);
}

TEST_F(TestListClientsOperationWithConnectedSession, AwaServerListClientsOperation_GetResponse_handles_non_matching_clientID)
{
    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaServerListClientsOperation_Perform(operation, global::timeout));
    EXPECT_EQ(NULL, AwaServerListClientsOperation_GetResponse(operation, "ThisIDDoesNotExist"));
    AwaServerListClientsOperation_Free(&operation);
}

TEST_F(TestListClientsOperationWithConnectedSession, AwaServerListClientsOperation_GetResponse_handles_matching_clientID)
{
    // start a client horde and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaServerListClientsOperation_Perform(operation, global::timeout));
    EXPECT_TRUE(NULL != AwaServerListClientsOperation_GetResponse(operation, "TestClient1"));
    AwaServerListClientsOperation_Free(&operation);
}

TEST_F(TestListClientsOperationWithConnectedSession, AwaServerListClientsOperation_GetResponse_handles_matching_clientID_multiple_times)
{
    // start a client horde and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaServerListClientsOperation_Perform(operation, global::timeout));
    // each call should return the same address
    const AwaServerListClientsResponse * response1 = AwaServerListClientsOperation_GetResponse(operation, "TestClient1");
    const AwaServerListClientsResponse * response2 = AwaServerListClientsOperation_GetResponse(operation, "TestClient1");
    const AwaServerListClientsResponse * response3 = AwaServerListClientsOperation_GetResponse(operation, "TestClient1");
    EXPECT_TRUE(NULL != response1);
    EXPECT_EQ(response1, response2);
    EXPECT_EQ(response1, response3);
    AwaServerListClientsOperation_Free(&operation);
}

TEST_F(TestListClientsOperationWithConnectedSession, AwaServerListClientsOperation_GetResponse_handles_no_perform)
{
    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    // no perform before GetResponse
    EXPECT_EQ(NULL, AwaServerListClientsOperation_GetResponse(operation, "TestClient1"));
    AwaServerListClientsOperation_Free(&operation);
}

TEST_F(TestListClientsOperationWithConnectedSession, AwaServerListClientsResponse_NewRegisteredEntityIterator_handles_null)
{
    EXPECT_EQ(NULL, AwaServerListClientsResponse_NewRegisteredEntityIterator(NULL));
}

TEST_F(TestListClientsOperationWithConnectedSession, AwaServerListClientsResponse_NewRegisteredEntityIterator_handles_valid_response)
{
    // start a client horde and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaServerListClientsOperation_Perform(operation, global::timeout));
    const AwaServerListClientsResponse * response = AwaServerListClientsOperation_GetResponse(operation, "TestClient1");
    EXPECT_TRUE(NULL != response);

    AwaRegisteredEntityIterator * iterator = AwaServerListClientsResponse_NewRegisteredEntityIterator(response);
    EXPECT_TRUE(NULL != iterator);

    // attempt to access iterator value without an initial Next should return NULL
    EXPECT_EQ(NULL, AwaRegisteredEntityIterator_GetPath(iterator));

    std::vector<std::string> expectedPaths { /*"/0/1",*/ "/1/0", "/2/0", "/2/1", "/2/2", "/2/3", "/3/0", "/4", "/5", "/6", "/7" };
    std::vector<std::string> actualPaths;

    while (AwaRegisteredEntityIterator_Next(iterator))
    {
        const char * path = AwaRegisteredEntityIterator_GetPath(iterator);
        EXPECT_TRUE(NULL != path);
        actualPaths.push_back(path);
    }

#if 0
    printf("Expected paths: \n");
    for(std::vector<std::string>::iterator it = expectedPaths.begin(); it != expectedPaths.end(); ++it)
    {
        std::cout << *it << " " << std::endl;
    }

    printf("Actual paths: \n");
    for(std::vector<std::string>::iterator it = actualPaths.begin(); it != actualPaths.end(); ++it)
    {
        std::cout << *it << " " << std::endl;
    }
#endif

    EXPECT_EQ(expectedPaths.size(), actualPaths.size());
    if (expectedPaths.size() == actualPaths.size())
        EXPECT_TRUE(std::is_permutation(expectedPaths.begin(), expectedPaths.end(), actualPaths.begin()));

    // test that Free nulls pointer
    AwaRegisteredEntityIterator_Free(&iterator);
    EXPECT_EQ(NULL, iterator);

    AwaServerListClientsOperation_Free(&operation);
}


} // namespace Awa

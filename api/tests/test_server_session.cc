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

#include "awa/server.h"
#include "server_session.h"

#include "log.h"
#include "support/support.h"

namespace Awa {

class TestServerSession : public TestServerBase {};

TEST_F(TestServerSession, AwaServerSession_New_returns_valid_session)
{
    AwaServerSession * session = AwaServerSession_New();
    ASSERT_TRUE(NULL != session);
    AwaServerSession_Free(&session);
}

TEST_F(TestServerSession, AwaServerSession_Free_frees_memory)
{
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_Success, AwaServerSession_Free(&session));
}

TEST_F(TestServerSession, AwaServerSession_Free_nulls_pointer)
{
    AwaServerSession * session = AwaServerSession_New();
    AwaServerSession_Free(&session);
    EXPECT_EQ(NULL, session);
}

TEST_F(TestServerSession, AwaServerSession_Free_handles_null)
{
    EXPECT_EQ(AwaError_SessionInvalid, AwaServerSession_Free(NULL));
}

TEST_F(TestServerSession, AwaServerSession_Free_handles_null_pointer)
{
    AwaServerSession * session = NULL;
    EXPECT_EQ(AwaError_SessionInvalid, AwaServerSession_Free(&session));
}

TEST_F(TestServerSession, AwaServerSession_SetIPCAsUDP_handles_null_session)
{
    EXPECT_EQ(AwaError_SessionInvalid, AwaServerSession_SetIPCAsUDP(NULL, "127.0.0.1", global::serverIpcPort));
}

TEST_F(TestServerSession, AwaServerSession_SetIPCAsUDP_handles_IPv4_address)
{
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session, "127.0.0.1", global::serverIpcPort));
    AwaServerSession_Free(&session);
}

TEST_F(TestServerSession, AwaServerSession_SetIPCAsUDP_handles_IPv6_address)
{
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session, "::1", global::serverIpcPort));
    AwaServerSession_Free(&session);
}

TEST_F(TestServerSession, AwaServerSession_SetIPCAsUDP_handles_invalid_address)
{
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_IPCError, AwaServerSession_SetIPCAsUDP(session, "", global::serverIpcPort));
    EXPECT_EQ(AwaError_IPCError, AwaServerSession_SetIPCAsUDP(session, "1-2-3-4", global::serverIpcPort));
    EXPECT_EQ(AwaError_IPCError, AwaServerSession_SetIPCAsUDP(session, "255.255.255.255.255", global::serverIpcPort));
    EXPECT_EQ(AwaError_IPCError, AwaServerSession_SetIPCAsUDP(session, "1::1::1", 0));
    AwaServerSession_Free(&session);
}

TEST_F(TestServerSession, AwaServerSession_SetIPCAsUDP_handles_null_address)
{
    // NULL address means loopback address
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session, NULL, global::serverIpcPort));
    AwaServerSession_Free(&session);
}

TEST_F(TestServerSession, AwaServerSession_SetIPCAsUDP_handles_hostname)
{
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session, "www.imgtec.com", global::serverIpcPort));
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session, "www.google.com", global::serverIpcPort));
    AwaServerSession_Free(&session);
}

TEST_F(TestServerSession, AwaServerSession_SetIPCAsUDP_handles_unresolvable_hostname)
{
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_IPCError, AwaServerSession_SetIPCAsUDP(session, "www.!.com", global::serverIpcPort));
    EXPECT_EQ(AwaError_IPCError, AwaServerSession_SetIPCAsUDP(session, "a.b.c.d.e", global::serverIpcPort));
    AwaServerSession_Free(&session);
}

TEST_F(TestServerSession, AwaServerSession_Connect_handles_null_session)
{
    AwaServerSession * session = NULL;
    EXPECT_EQ(AwaError_SessionInvalid, AwaServerSession_Connect(session));
    AwaServerSession_Free(&session);
}

TEST_F(TestServerSession, AwaServerSession_Connect_with_default_IPC)
{
    // Start a client daemon on the expected default IPC port
    AwaServerDaemon daemon_;
    daemon_.SetIpcPort(IPC_DEFAULT_SERVER_PORT);
    ASSERT_TRUE(daemon_.Start(""));

    // A session default IPC setup:
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));
    AwaServerSession_Free(&session);
    daemon_.Stop();
}

TEST_F(TestServerSession, AwaServerSession_Connect_handles_invalid_ipc)
{
    // A session with invalid IPC setup:
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_IPCError, AwaServerSession_SetIPCAsUDP(session, "1::1::1", 0));
    AwaServerSession_Free(&session);
}

TEST_F(TestServerSession, AwaServerSession_Connect_handles_unresponsive_UDP_server)
{
    // Connect to daemon - if this test fails, ensure nothing is actually running on 127.0.0.2:serverIpcPort!
    AwaServerSession * session = AwaServerSession_New();
    AwaServerSession_SetIPCAsUDP(session, detail::NonRoutableIPv4Address, global::serverIpcPort);
    EXPECT_EQ(AwaError_Timeout, AwaServerSession_Connect(session));
    AwaServerSession_Free(&session);
}

TEST_F(TestServerSession, AwaServerSession_Disconnect_handles_null_session)
{
    EXPECT_EQ(AwaError_SessionInvalid, AwaServerSession_Disconnect(NULL));
}

TEST_F(TestServerSession, AwaServerSession_Disconnect_handles_invalid_session)
{
    // A session with no IPC setup:
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_IPCError, AwaServerSession_Disconnect(session));
    AwaServerSession_Free(&session);
}

TEST_F(TestServerSession, AwaServerSession_Disconnect_handles_non_connected_session)
{
    // Connect to daemon on IPv4 address
    AwaServerSession * session = AwaServerSession_New();
    AwaServerSession_SetIPCAsUDP(session, "127.0.0.1", global::serverIpcPort);
    EXPECT_EQ(AwaError_SessionNotConnected, AwaServerSession_Disconnect(session));
    AwaServerSession_Free(&session);
}

//FIXME: FLOWDM-371
TEST_F(TestServerSession, DISABLED_AwaServerSession_Free_calls_AwaServerSession_Disconnect)
{
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session, "127.0.0.1", global::serverIpcPort));
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));
    AwaServerSession_Free(&session);


    // TODO:
    ASSERT_TRUE(0);
}

TEST_F(TestServerSession, ServerSession_IsConnected_handles_null)
{
    ASSERT_FALSE(ServerSession_IsConnected(NULL));
}

TEST_F(TestServerSession, ServerSession_IsConnected_returns_false_for_not_connected_session)
{
    AwaServerSession * session = AwaServerSession_New();
    ASSERT_FALSE(ServerSession_IsConnected(session));
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session, "127.0.0.1", global::serverIpcPort));
    ASSERT_FALSE(ServerSession_IsConnected(session));
    AwaServerSession_Free(&session);
}

// Tests that require a spawned daemon:

class TestServerSessionWithDaemon : public TestServerWithDaemonBase {};

TEST_F(TestServerSessionWithDaemon, AwaServerSession_Connect_handles_valid_IPv4_UDP_session)
{
    // Connect to daemon on IPv4 address
    AwaServerSession * session = AwaServerSession_New();
    AwaServerSession_SetIPCAsUDP(session, "127.0.0.1", global::serverIpcPort);
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));
    AwaServerSession_Free(&session);
}

//FIXME: FLOWDM-372
TEST_F(TestServerSessionWithDaemon, DISABLED_AwaServerSession_Connect_handles_valid_IPv6_UDP_session)
{
    // Connect to daemon on IPv6 address
    AwaServerSession * session = AwaServerSession_New();
    AwaServerSession_SetIPCAsUDP(session, "::1", global::serverIpcPort);
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));
    AwaServerSession_Free(&session);
}

TEST_F(TestServerSessionWithDaemon, AwaServerSession_Disconnect_handles_valid_IPv4_UDP_session)
{
    // Connect to daemon on IPv4 address
    AwaServerSession * session = AwaServerSession_New();
    AwaServerSession_SetIPCAsUDP(session, "127.0.0.1", global::serverIpcPort);
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));
    EXPECT_EQ(AwaError_Success, AwaServerSession_Disconnect(session));
    AwaServerSession_Free(&session);
}

//FIXME: FLOWDM-372
TEST_F(TestServerSessionWithDaemon, DISABLED_AwaServerSession_Disconnect_handles_valid_IPv6_UDP_session)
{
    AwaServerSession * session = AwaServerSession_New();
    AwaServerSession_SetIPCAsUDP(session, "::1", global::serverIpcPort);
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));
    EXPECT_EQ(AwaError_Success, AwaServerSession_Disconnect(session));
    AwaServerSession_Free(&session);
}

TEST_F(TestServerSessionWithDaemon, AwaServerSession_Disconnect_handles_unresponsive_UDP_server)
{
    if (!global::spawnServerDaemon)
    {
        // This test is disabled when daemons are not spawned by the test system
        std::cout << " *** SKIPPED ***" << std::endl;
        return;
    }

    AwaServerSession * session = AwaServerSession_New();
    AwaServerSession_SetIPCAsUDP(session, "127.0.0.1", global::serverIpcPort);
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));

    // kill Daemon
    TestServerSessionWithDaemon::TearDown();
    EXPECT_EQ(AwaError_Timeout, AwaServerSession_Disconnect(session));

    AwaServerSession_Free(&session);
}

TEST_F(TestServerSessionWithDaemon, ServerSession_IsConnected_returns_true_for_connected_session)
{
    AwaServerSession * session = AwaServerSession_New();
    AwaServerSession_SetIPCAsUDP(session, "127.0.0.1", global::serverIpcPort);
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));
    ASSERT_TRUE(ServerSession_IsConnected(session));
    EXPECT_EQ(AwaError_Success, AwaServerSession_Disconnect(session));
    AwaServerSession_Free(&session);
}

TEST_F(TestServerSessionWithDaemon, ServerSession_IsConnected_returns_false_for_disconnected_session)
{
    AwaServerSession * session = AwaServerSession_New();
    AwaServerSession_SetIPCAsUDP(session, "127.0.0.1", global::serverIpcPort);
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));
    EXPECT_EQ(AwaError_Success, AwaServerSession_Disconnect(session));
    ASSERT_FALSE(ServerSession_IsConnected(session));
    AwaServerSession_Free(&session);
}

TEST_F(TestServerSessionWithDaemon, AwaServerSession_Connect_handles_call_twice)
{
    // Attempt to connect twice daemon on IPv4 address
    AwaServerSession * session = AwaServerSession_New();
    AwaServerSession_SetIPCAsUDP(session, "127.0.0.1", global::serverIpcPort);
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));
    EXPECT_EQ(AwaError_IPCError, AwaServerSession_Connect(session));
    AwaServerSession_Free(&session);
}

TEST_F(TestServerSession, AwaServerSession_PathToIDs_handles_invalid_session)
{
    AwaObjectID objectID = AWA_INVALID_ID;
    AwaObjectInstanceID objectInstanceID = AWA_INVALID_ID;
    AwaResourceID resourceID = AWA_INVALID_ID;
    ASSERT_EQ(AwaError_SessionInvalid, AwaServerSession_PathToIDs(NULL, "/1/2/3", &objectID, &objectInstanceID, &resourceID));
}

TEST_F(TestServerSession, AwaServerSession_PathToIDs_handles_null_path)
{
    AwaServerSession * session = AwaServerSession_New();
    AwaObjectID objectID = AWA_INVALID_ID;
    AwaObjectInstanceID objectInstanceID = AWA_INVALID_ID;
    AwaResourceID resourceID = AWA_INVALID_ID;
    ASSERT_EQ(AwaError_PathInvalid, AwaServerSession_PathToIDs(session, NULL, &objectID, &objectInstanceID, &resourceID));
    AwaServerSession_Free(&session);
}

TEST_F(TestServerSession, AwaServerSession_NewObjectDefinitionIterator_valid_inputs)
{
    AwaServerSession * session = AwaServerSession_New();
    ASSERT_TRUE(NULL != session);

    AwaObjectDefinitionIterator * iterator = AwaServerSession_NewObjectDefinitionIterator(session);
    ASSERT_TRUE(NULL != iterator);
    AwaObjectDefinitionIterator_Free(&iterator);
    ASSERT_TRUE(NULL == iterator);

    AwaServerSession_Free(&session);
}

TEST_F(TestServerSession, AwaServerSession_NewObjectDefinitionIterator_handles_null)
{
    AwaObjectDefinitionIterator * iterator = AwaServerSession_NewObjectDefinitionIterator(NULL);
    ASSERT_TRUE(NULL == iterator);
    AwaObjectDefinitionIterator_Free(NULL);
}

class TestServerSessionPathInvalidWithParam : public TestServerSession, public ::testing::WithParamInterface<const char *> {};

TEST_P(TestServerSessionPathInvalidWithParam, AwaServerSession_PathToIDs_handles_invalid_path)
{
    AwaServerSession * session = AwaServerSession_New();
    AwaObjectID objectID = AWA_INVALID_ID;
    AwaObjectInstanceID objectInstanceID = AWA_INVALID_ID;
    AwaResourceID resourceID = AWA_INVALID_ID;
    ASSERT_EQ(AwaError_PathInvalid, AwaServerSession_PathToIDs(session, GetParam(), &objectID, &objectInstanceID, &resourceID));
    AwaServerSession_Free(&session);
}

INSTANTIATE_TEST_CASE_P(
        TestServerSessionPathInvalidWithParamInstantiation,
        TestServerSessionPathInvalidWithParam,
        ::testing::Values(
                "",
                "/",
                "0",
                "0/",
                "/0/",
                "/0/1/",
                "0/1/2",
                "0/1/2/",
                "/0/1/2/",
                "//0/1/2",
                "/0/1/2/3",
                "/a/b/c",
                "/3/0/d",
                "a/n in/valid/ path",
                "/70000/0/0",
                "/0/70000/0",
                "/0/0/70000",
                "/0/0/-1",
                "/0/-1/0",
                "/-1/0/0",
                "root/0/1/2",
                "root/sub/0/1/2",
                "/Device/0/Manufacturer",
                "root/Device/0/Manufacturer"
        ));

struct TestServerSessionPathValidWithParamItem
{
    const char * Path;
    AwaObjectID ExpectedObjectID;
    AwaObjectInstanceID ExpectedObjectInstanceID;
    AwaResourceID ExpectedResourceID;
};

::std::ostream& operator<<(::std::ostream& os, const TestServerSessionPathValidWithParamItem& item)
{
  return os << "Item: Path " << item.Path
            << ", ExpectedObjectID " << item.ExpectedObjectID
            << ", ExpectedObjectInstanceID " << item.ExpectedObjectInstanceID
            << ", ExpectedResourceID " << item.ExpectedResourceID;
}

class TestServerSessionPathValidWithParam : public TestServerSession, public ::testing::WithParamInterface<TestServerSessionPathValidWithParamItem> {};

TEST_P(TestServerSessionPathValidWithParam, AwaServerSession_PathToIDs_handles_valid_path)
{
    AwaServerSession * session = AwaServerSession_New();
    AwaObjectID objectID = 42;
    AwaObjectInstanceID objectInstanceID = 19;
    AwaResourceID resourceID = 8;
    ASSERT_EQ(AwaError_Success, AwaServerSession_PathToIDs(session, GetParam().Path, &objectID, &objectInstanceID, &resourceID));
    EXPECT_EQ(GetParam().ExpectedObjectID, objectID);
    EXPECT_EQ(GetParam().ExpectedObjectInstanceID, objectInstanceID);
    EXPECT_EQ(GetParam().ExpectedResourceID, resourceID);
    AwaServerSession_Free(&session);
}

INSTANTIATE_TEST_CASE_P(
        TestServerSessionPathValidWithParamInstantiation,
        TestServerSessionPathValidWithParam,
        ::testing::Values(
            TestServerSessionPathValidWithParamItem { "/3",             3,     AWA_INVALID_ID, AWA_INVALID_ID },
            TestServerSessionPathValidWithParamItem { "/3/2",           3,     2,                  AWA_INVALID_ID },
            TestServerSessionPathValidWithParamItem { "/3/2/1",         3,     2,                  1 },
            TestServerSessionPathValidWithParamItem { "/3/2/65535",     3,     2,                  65535 },
            TestServerSessionPathValidWithParamItem { "/65535/2/65535", 65535, 2,                  65535 },
            TestServerSessionPathValidWithParamItem { "/0/0/0",         0,     0,                  0 }
        ));

} // namespace Awa

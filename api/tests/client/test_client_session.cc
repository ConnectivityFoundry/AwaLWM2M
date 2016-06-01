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

#include "awa/client.h"
#include "client_session.h"

#include "log.h"
#include "support/support.h"

namespace Awa {

class TestClientSession : public TestClientBase {};

TEST_F(TestClientSession, AwaClientSession_New_returns_valid_session)
{
    AwaClientSession * session = AwaClientSession_New();
    ASSERT_TRUE(NULL != session);
    AwaClientSession_Free(&session);
}

TEST_F(TestClientSession, AwaClientSession_Free_frees_memory)
{
    AwaClientSession * session = AwaClientSession_New();
    EXPECT_EQ(AwaError_Success, AwaClientSession_Free(&session));
}

TEST_F(TestClientSession, AwaClientSession_Free_nulls_pointer)
{
    AwaClientSession * session = AwaClientSession_New();
    AwaClientSession_Free(&session);
    EXPECT_EQ(NULL, session);
}

TEST_F(TestClientSession, AwaClientSession_Free_handles_null)
{
    EXPECT_EQ(AwaError_SessionInvalid, AwaClientSession_Free(NULL));
}

TEST_F(TestClientSession, AwaClientSession_Free_handles_null_pointer)
{
    AwaClientSession * session = NULL;
    EXPECT_EQ(AwaError_SessionInvalid, AwaClientSession_Free(&session));
}

TEST_F(TestClientSession, AwaClientSession_SetIPCAsUDP_handles_null_session)
{
    EXPECT_EQ(AwaError_SessionInvalid, AwaClientSession_SetIPCAsUDP(NULL, "127.0.0.1", global::clientIpcPort));
}

TEST_F(TestClientSession, AwaClientSession_SetIPCAsUDP_handles_IPv4_address)
{
    AwaClientSession * session = AwaClientSession_New();
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort));
    AwaClientSession_Free(&session);
}

TEST_F(TestClientSession, AwaClientSession_SetIPCAsUDP_handles_IPv6_address)
{
    AwaClientSession * session = AwaClientSession_New();
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(session, "::1", global::clientIpcPort));
    AwaClientSession_Free(&session);
}

TEST_F(TestClientSession, AwaClientSession_SetIPCAsUDP_handles_invalid_address)
{
    AwaClientSession * session = AwaClientSession_New();
    EXPECT_EQ(AwaError_IPCError, AwaClientSession_SetIPCAsUDP(session, "", global::clientIpcPort));
    EXPECT_EQ(AwaError_IPCError, AwaClientSession_SetIPCAsUDP(session, "1-2-3-4", global::clientIpcPort));
    EXPECT_EQ(AwaError_IPCError, AwaClientSession_SetIPCAsUDP(session, "255.255.255.255.255", global::clientIpcPort));
    EXPECT_EQ(AwaError_IPCError, AwaClientSession_SetIPCAsUDP(session, "1::1::1", 0));
    AwaClientSession_Free(&session);
}

TEST_F(TestClientSession, AwaClientSession_SetIPCAsUDP_handles_null_address)
{
    // NULL address means loopback address
    AwaClientSession * session = AwaClientSession_New();
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(session, NULL, global::clientIpcPort));
    AwaClientSession_Free(&session);
}

TEST_F(TestClientSession, AwaClientSession_SetIPCAsUDP_handles_hostname)
{
    AwaClientSession * session = AwaClientSession_New();
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(session, "www.imgtec.com", global::clientIpcPort));
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(session, "www.google.com", global::clientIpcPort));
    AwaClientSession_Free(&session);
}

TEST_F(TestClientSession, AwaClientSession_SetIPCAsUDP_handles_unresolvable_hostname)
{
    AwaClientSession * session = AwaClientSession_New();
    EXPECT_EQ(AwaError_IPCError, AwaClientSession_SetIPCAsUDP(session, "www.!.com", global::clientIpcPort));
    EXPECT_EQ(AwaError_IPCError, AwaClientSession_SetIPCAsUDP(session, "a.b.c.d.e", global::clientIpcPort));
    AwaClientSession_Free(&session);
}

TEST_F(TestClientSession, AwaClientSession_Connect_handles_null_session)
{
    AwaClientSession * session = NULL;
    EXPECT_EQ(AwaError_SessionInvalid, AwaClientSession_Connect(session));
    AwaClientSession_Free(&session);
}

TEST_F(TestClientSession, AwaClientSession_Connect_with_default_IPC)
{
    // Start a client daemon on the expected default IPC port
    AwaClientDaemon daemon_;
    daemon_.SetIpcPort(IPC_DEFAULT_CLIENT_PORT);
    ASSERT_TRUE(daemon_.Start());

    // A session default IPC setup:
    AwaClientSession * session = AwaClientSession_New();
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(session));
    AwaClientSession_Free(&session);
    daemon_.Stop();
}

TEST_F(TestClientSession, AwaClientSession_Connect_handles_invalid_ipc)
{
    // A session with invalid IPC setup:
    AwaClientSession * session = AwaClientSession_New();
    EXPECT_EQ(AwaError_IPCError, AwaClientSession_SetIPCAsUDP(session, "1::1::1", 0));
    AwaClientSession_Free(&session);
}

TEST_F(TestClientSession, AwaClientSession_Connect_handles_unresponsive_UDP_server)
{
    // Connect to daemon - if this test fails, ensure nothing is actually running on 127.0.0.2:clientIpcPort!
    AwaClientSession * session = AwaClientSession_New();
    AwaClientSession_SetDefaultTimeout(session, 1000);
    AwaClientSession_SetIPCAsUDP(session, detail::NonRoutableIPv4Address, global::clientIpcPort);
    EXPECT_EQ(AwaError_Timeout, AwaClientSession_Connect(session));
    AwaClientSession_Free(&session);
}

TEST_F(TestClientSession, AwaClientSession_Disconnect_handles_null_session)
{
    EXPECT_EQ(AwaError_SessionInvalid, AwaClientSession_Disconnect(NULL));
}

TEST_F(TestClientSession, AwaClientSession_Disconnect_handles_invalid_session)
{
    // A session with no IPC setup:
    AwaClientSession * session = AwaClientSession_New();
    EXPECT_EQ(AwaError_IPCError, AwaClientSession_Disconnect(session));
    AwaClientSession_Free(&session);
}

TEST_F(TestClientSession, AwaClientSession_Disconnect_handles_non_connected_session)
{
    // Connect to daemon on IPv4 address
    AwaClientSession * session = AwaClientSession_New();
    AwaClientSession_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort);
    EXPECT_EQ(AwaError_SessionNotConnected, AwaClientSession_Disconnect(session));
    AwaClientSession_Free(&session);
}

//FIXME: FLOWDM-371
TEST_F(TestClientSession, DISABLED_AwaClientSession_Free_calls_AwaClientSession_Disconnect)
{
    AwaClientSession * session = AwaClientSession_New();
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(session));
    AwaClientSession_Free(&session);


    // TODO:
    ASSERT_TRUE(0);
}

TEST_F(TestClientSession, ClientSession_IsConnected_handles_null)
{
    ASSERT_FALSE(ClientSession_IsConnected(NULL));
}

TEST_F(TestClientSession, ClientSession_IsConnected_returns_false_for_not_connected_session)
{
    AwaClientSession * session = AwaClientSession_New();
    ASSERT_FALSE(ClientSession_IsConnected(session));
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort));
    ASSERT_FALSE(ClientSession_IsConnected(session));
    AwaClientSession_Free(&session);
}

// Tests that require a spawned daemon:

class TestClientSessionWithDaemon : public TestClientWithDaemonBase {};

TEST_F(TestClientSessionWithDaemon, AwaClientSession_Connect_handles_valid_IPv4_UDP_session)
{
    // Connect to daemon on IPv4 address
    AwaClientSession * session = AwaClientSession_New();
    AwaClientSession_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort);
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(session));
    AwaClientSession_Free(&session);
}

//FIXME: FLOWDM-372
TEST_F(TestClientSessionWithDaemon, DISABLED_AwaClientSession_Connect_handles_valid_IPv6_UDP_session)
{
    // Connect to daemon on IPv6 address
    AwaClientSession * session = AwaClientSession_New();
    AwaClientSession_SetIPCAsUDP(session, "::1", global::clientIpcPort);
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(session));
    AwaClientSession_Free(&session);
}

TEST_F(TestClientSessionWithDaemon, AwaClientSession_Disconnect_handles_valid_IPv4_UDP_session)
{
    // Connect to daemon on IPv4 address
    AwaClientSession * session = AwaClientSession_New();
    AwaClientSession_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort);
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(session));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Disconnect(session));
    AwaClientSession_Free(&session);
}

//FIXME: FLOWDM-372
TEST_F(TestClientSessionWithDaemon, DISABLED_AwaClientSession_Disconnect_handles_valid_IPv6_UDP_session)
{
    AwaClientSession * session = AwaClientSession_New();
    AwaClientSession_SetIPCAsUDP(session, "::1", global::clientIpcPort);
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(session));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Disconnect(session));
    AwaClientSession_Free(&session);
}

TEST_F(TestClientSessionWithDaemon, AwaClientSession_Disconnect_handles_unresponsive_UDP_server)
{
    if (!global::spawnClientDaemon)
    {
        // This test is disabled when daemons are not spawned by the test system
        std::cout << " *** SKIPPED ***" << std::endl;
        return;
    }

    AwaClientSession * session = AwaClientSession_New();
    AwaClientSession_SetDefaultTimeout(session, 1000);
    AwaClientSession_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort);
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(session));

    // kill Daemon
    TestClientSessionWithDaemon::TearDown();
    EXPECT_EQ(AwaError_Timeout, AwaClientSession_Disconnect(session));

    AwaClientSession_Free(&session);
}

TEST_F(TestClientSessionWithDaemon, ClientSession_IsConnected_returns_true_for_connected_session)
{
    AwaClientSession * session = AwaClientSession_New();
    AwaClientSession_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort);
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(session));
    ASSERT_TRUE(ClientSession_IsConnected(session));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Disconnect(session));
    AwaClientSession_Free(&session);
}

TEST_F(TestClientSessionWithDaemon, ClientSession_IsConnected_returns_false_for_disconnected_session)
{
    AwaClientSession * session = AwaClientSession_New();
    AwaClientSession_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort);
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(session));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Disconnect(session));
    ASSERT_FALSE(ClientSession_IsConnected(session));
    AwaClientSession_Free(&session);
}

TEST_F(TestClientSessionWithDaemon, AwaClientSession_Connect_handles_call_twice)
{
    // Attempt to connect twice daemon on IPv4 address
    AwaClientSession * session = AwaClientSession_New();
    AwaClientSession_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort);
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(session));
    EXPECT_EQ(AwaError_IPCError, AwaClientSession_Connect(session));
    AwaClientSession_Free(&session);
}

TEST_F(TestClientSession, AwaClientSession_PathToIDs_handles_invalid_session)
{
    AwaObjectID objectID = AWA_INVALID_ID;
    AwaObjectInstanceID objectInstanceID = AWA_INVALID_ID;
    AwaResourceID resourceID = AWA_INVALID_ID;
    ASSERT_EQ(AwaError_SessionInvalid, AwaClientSession_PathToIDs(NULL, "/1/2/3", &objectID, &objectInstanceID, &resourceID));
}

TEST_F(TestClientSession, AwaClientSession_PathToIDs_handles_null_path)
{
    AwaClientSession * session = AwaClientSession_New();
    AwaObjectID objectID = AWA_INVALID_ID;
    AwaObjectInstanceID objectInstanceID = AWA_INVALID_ID;
    AwaResourceID resourceID = AWA_INVALID_ID;
    ASSERT_EQ(AwaError_PathInvalid, AwaClientSession_PathToIDs(session, NULL, &objectID, &objectInstanceID, &resourceID));
    AwaClientSession_Free(&session);
}

TEST_F(TestClientSession, AwaClientSession_NewObjectDefinitionIterator_valid_inputs)
{
    AwaClientSession * session = AwaClientSession_New();
    ASSERT_TRUE(NULL != session);
    
    AwaObjectDefinitionIterator * iterator = AwaClientSession_NewObjectDefinitionIterator(session);
    ASSERT_TRUE(NULL != iterator);
    AwaObjectDefinitionIterator_Free(&iterator);
    ASSERT_TRUE(NULL == iterator);
    
    AwaClientSession_Free(&session);
}

TEST_F(TestClientSession, AwaClientSession_NewObjectDefinitionIterator_invalid_inputs)
{
    AwaObjectDefinitionIterator * iterator = AwaClientSession_NewObjectDefinitionIterator(NULL);
    ASSERT_TRUE(NULL == iterator);
    AwaObjectDefinitionIterator_Free(NULL);
}

TEST_F(TestClientSession, AwaClientSession_SetDefaultTimeout_handles_invalid_timeout)
{
    AwaClientSession * session = AwaClientSession_New();
    EXPECT_EQ(AwaError_Unsupported, AwaClientSession_SetDefaultTimeout(session, 0));
    EXPECT_EQ(AwaError_Unsupported, AwaClientSession_SetDefaultTimeout(session, -1));
    EXPECT_EQ(AwaError_Unsupported, AwaClientSession_SetDefaultTimeout(session, -100));
    AwaClientSession_Free(&session);
}

class TestClientSessionPathInvalidWithParam : public TestClientSession, public ::testing::WithParamInterface<const char *>
{
};

TEST_P(TestClientSessionPathInvalidWithParam, AwaClientSession_PathToIDs_handles_invalid_path)
{
    AwaClientSession * session = AwaClientSession_New();
    AwaObjectID objectID = AWA_INVALID_ID;
    AwaObjectInstanceID objectInstanceID = AWA_INVALID_ID;
    AwaResourceID resourceID = AWA_INVALID_ID;
    ASSERT_EQ(AwaError_PathInvalid, AwaClientSession_PathToIDs(session, GetParam(), &objectID, &objectInstanceID, &resourceID));
    AwaClientSession_Free(&session);
}

INSTANTIATE_TEST_CASE_P(
        TestClientSessionPathInvalidWithParamInstantiation,
        TestClientSessionPathInvalidWithParam,
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

struct TestClientSessionPathValidWithParamItem
{
    const char * Path;
    AwaObjectID ExpectedObjectID;
    AwaObjectInstanceID ExpectedObjectInstanceID;
    AwaResourceID ExpectedResourceID;
};

::std::ostream& operator<<(::std::ostream& os, const TestClientSessionPathValidWithParamItem& item)
{
  return os << "Item: Path " << item.Path
            << ", ExpectedObjectID " << item.ExpectedObjectID
            << ", ExpectedObjectInstanceID " << item.ExpectedObjectInstanceID
            << ", ExpectedResourceID " << item.ExpectedResourceID;
}

class TestClientSessionPathValidWithParam : public TestClientSession, public ::testing::WithParamInterface<TestClientSessionPathValidWithParamItem>
{
};

TEST_P(TestClientSessionPathValidWithParam, AwaClientSession_PathToIDs_handles_valid_path)
{
    AwaClientSession * session = AwaClientSession_New();
    AwaObjectID objectID = 42;
    AwaObjectInstanceID objectInstanceID = 19;
    AwaResourceID resourceID = 8;
    ASSERT_EQ(AwaError_Success, AwaClientSession_PathToIDs(session, GetParam().Path, &objectID, &objectInstanceID, &resourceID));
    EXPECT_EQ(GetParam().ExpectedObjectID, objectID);
    EXPECT_EQ(GetParam().ExpectedObjectInstanceID, objectInstanceID);
    EXPECT_EQ(GetParam().ExpectedResourceID, resourceID);
    AwaClientSession_Free(&session);
}

INSTANTIATE_TEST_CASE_P(
        TestClientSessionPathValidWithParamInstantiation,
        TestClientSessionPathValidWithParam,
        ::testing::Values(
            TestClientSessionPathValidWithParamItem { "/3",             3,     AWA_INVALID_ID, AWA_INVALID_ID },
            TestClientSessionPathValidWithParamItem { "/3/2",           3,     2,                  AWA_INVALID_ID },
            TestClientSessionPathValidWithParamItem { "/3/2/1",         3,     2,                  1 },
            TestClientSessionPathValidWithParamItem { "/3/2/65535",     3,     2,                  65535 },
            TestClientSessionPathValidWithParamItem { "/65535/2/65535", 65535, 2,                  65535 },
            TestClientSessionPathValidWithParamItem { "/0/0/0",         0,     0,                  0 }
        ));

} // namespace Awa

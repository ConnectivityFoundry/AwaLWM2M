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

#include "session_common.h"
#include "support/support.h"

namespace Awa {

class TestSessionCommon : public TestClientBase {};

TEST_F(TestSessionCommon, SessionCommon_New_and_Free_works)
{
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    ASSERT_TRUE(NULL != session);
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommon, SessionCommon_Free_nulls_pointer)
{
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    ASSERT_TRUE(NULL != session);
    SessionCommon_Free(&session);
    ASSERT_EQ(NULL, session);
}

TEST_F(TestSessionCommon, SessionCommon_Free_handles_null)
{
    SessionCommon_Free(NULL);
}

TEST_F(TestSessionCommon, SessionCommon_Free_handles_null_pointer)
{
    SessionCommon * session = NULL;
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommon, SessionCommon_SetIPCAsUDP_handles_null_session)
{
    EXPECT_EQ(AwaError_SessionInvalid, SessionCommon_SetIPCAsUDP(NULL, "127.0.0.1", global::clientIpcPort));
}

TEST_F(TestSessionCommon, SessionCommon_SetIPCAsUDP_handles_IPv4_address)
{
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    EXPECT_EQ(AwaError_Success, SessionCommon_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort));
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommon, SessionCommon_SetIPCAsUDP_handles_IPv6_address)
{
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    EXPECT_EQ(AwaError_Success, SessionCommon_SetIPCAsUDP(session, "::1", global::clientIpcPort));
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommon, SessionCommon_SetIPCAsUDP_handles_invalid_address)
{
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    EXPECT_EQ(AwaError_IPCError, SessionCommon_SetIPCAsUDP(session, "", global::clientIpcPort));
    EXPECT_EQ(AwaError_IPCError, SessionCommon_SetIPCAsUDP(session, "1-2-3-4", global::clientIpcPort));
    EXPECT_EQ(AwaError_IPCError, SessionCommon_SetIPCAsUDP(session, "255.255.255.255.255", global::clientIpcPort));
    EXPECT_EQ(AwaError_IPCError, SessionCommon_SetIPCAsUDP(session, "1::1::1", 0));
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommon, SessionCommon_SetIPCAsUDP_handles_null_address)
{
    // NULL address means loopback address
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    EXPECT_EQ(AwaError_Success, SessionCommon_SetIPCAsUDP(session, NULL, global::clientIpcPort));
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommon, SessionCommon_SetIPCAsUDP_handles_hostname)
{
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    EXPECT_EQ(AwaError_Success, SessionCommon_SetIPCAsUDP(session, "www.imgtec.com", global::clientIpcPort));
    EXPECT_EQ(AwaError_Success, SessionCommon_SetIPCAsUDP(session, "www.google.com", global::clientIpcPort));
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommon, SessionCommon_SetIPCAsUDP_handles_unresolvable_hostname)
{
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    EXPECT_EQ(AwaError_IPCError, SessionCommon_SetIPCAsUDP(session, "www.!.com", global::clientIpcPort));
    EXPECT_EQ(AwaError_IPCError, SessionCommon_SetIPCAsUDP(session, "a.b.c.d.e", global::clientIpcPort));
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommon, SessionCommon_ConnectSession_handles_null_session)
{
    SessionCommon * session = NULL;
    EXPECT_EQ(AwaError_SessionInvalid, SessionCommon_ConnectSession(session));
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommon, SessionCommon_ConnectSession_handles_invalid_session)
{
    // A session with no IPC setup:
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    EXPECT_EQ(AwaError_IPCError, SessionCommon_ConnectSession(session));
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommon, SessionCommon_ConnectSession_handles_invalid_ipc)
{
    // A session with invalid IPC setup:
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    SessionCommon_SetIPCAsUDP(session, "1::1::1", 0);
    EXPECT_EQ(AwaError_IPCError, SessionCommon_ConnectSession(session));
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommon, SessionCommon_ConnectSession_handles_unresponsive_UDP_server)
{
    // Connect to daemon - if this test fails, ensure nothing is actually running on 127.0.0.2:clientIpcPort!
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    SessionCommon_SetDefaultTimeout(session, 1000);
    SessionCommon_SetIPCAsUDP(session, detail::NonRoutableIPv4Address, global::clientIpcPort);
    EXPECT_EQ(AwaError_Timeout, SessionCommon_ConnectSession(session));
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommon, SessionCommon_DisconnectSession_handles_null_session)
{
    EXPECT_EQ(AwaError_SessionInvalid, SessionCommon_DisconnectSession(NULL));
}

TEST_F(TestSessionCommon, SessionCommon_DisconnectSession_handles_invalid_session)
{
    // A session with no IPC setup:
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    EXPECT_EQ(AwaError_IPCError, SessionCommon_DisconnectSession(session));
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommon, SessionCommon_DisconnectSession_handles_non_connected_session)
{
    // Connect to daemon on IPv4 address
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    SessionCommon_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort);
    EXPECT_EQ(AwaError_SessionNotConnected, SessionCommon_DisconnectSession(session));
    SessionCommon_Free(&session);
}

//FIXME: FLOWDM-371
TEST_F(TestSessionCommon, DISABLED_SessionCommon_Free_calls_SessionCommon_DisconnectSession)
{
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    EXPECT_EQ(AwaError_Success, SessionCommon_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort));
    EXPECT_EQ(AwaError_Success, SessionCommon_ConnectSession(session));
    SessionCommon_Free(&session);

    // TODO:
    ASSERT_TRUE(0);
}

TEST_F(TestSessionCommon, SessionCommon_IsConnected_handles_null)
{
    ASSERT_FALSE(SessionCommon_IsConnected(NULL));
}

TEST_F(TestSessionCommon, SessionCommon_IsConnected_returns_false_for_not_connected_session)
{
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    ASSERT_FALSE(SessionCommon_IsConnected(session));
    EXPECT_EQ(AwaError_Success, SessionCommon_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort));
    ASSERT_FALSE(SessionCommon_IsConnected(session));
    SessionCommon_Free(&session);
}

// Tests that require a spawned daemon:

class TestSessionCommonWithDaemon : public TestClientWithDaemonBase {};

TEST_F(TestSessionCommonWithDaemon, SessionCommon_ConnectSession_handles_valid_IPv4_UDP_session)
{
    // Connect to daemon on IPv4 address
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    SessionCommon_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort);
    EXPECT_EQ(AwaError_Success, SessionCommon_ConnectSession(session));
    SessionCommon_Free(&session);
}

//FIXME: FLOWDM-372
TEST_F(TestSessionCommonWithDaemon, DISABLED_SessionCommon_ConnectSession_handles_valid_IPv6_UDP_session)
{
    // Connect to daemon on IPv6 address
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    SessionCommon_SetIPCAsUDP(session, "::1", global::clientIpcPort);
    EXPECT_EQ(AwaError_Success, SessionCommon_ConnectSession(session));
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommonWithDaemon, SessionCommon_DisconnectSession_handles_valid_IPv4_UDP_session)
{
    // Connect to daemon on IPv4 address
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    SessionCommon_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort);
    EXPECT_EQ(AwaError_Success, SessionCommon_ConnectSession(session));
    EXPECT_EQ(AwaError_Success, SessionCommon_DisconnectSession(session));
    SessionCommon_Free(&session);
}

//FIXME: FLOWDM-372
TEST_F(TestSessionCommonWithDaemon, DISABLED_SessionCommon_DisconnectSession_handles_valid_IPv6_UDP_session)
{
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    SessionCommon_SetIPCAsUDP(session, "::1", global::clientIpcPort);
    EXPECT_EQ(AwaError_Success, SessionCommon_ConnectSession(session));
    EXPECT_EQ(AwaError_Success, SessionCommon_DisconnectSession(session));
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommonWithDaemon, SessionCommon_DisconnectSession_handles_unresponsive_UDP_server)
{
    if (!global::spawnClientDaemon)
    {
        // This test is disabled when daemons are not spawned by the test system
        std::cout << " *** SKIPPED ***" << std::endl;
        return;
    }

    SessionCommon * session = SessionCommon_New(SessionType_Client);
    SessionCommon_SetDefaultTimeout(session, 1000);
    SessionCommon_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort);
    EXPECT_EQ(AwaError_Success, SessionCommon_ConnectSession(session));

    // kill Daemon
    TestSessionCommonWithDaemon::TearDown();
    EXPECT_EQ(AwaError_Timeout, SessionCommon_DisconnectSession(session));

    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommonWithDaemon, SessionCommon_IsConnected_returns_true_for_connected_session)
{
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    SessionCommon_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort);
    EXPECT_EQ(AwaError_Success, SessionCommon_ConnectSession(session));
    ASSERT_TRUE(SessionCommon_IsConnected(session));
    EXPECT_EQ(AwaError_Success, SessionCommon_DisconnectSession(session));
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommonWithDaemon, SessionCommon_IsConnected_returns_false_for_disconnected_session)
{
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    SessionCommon_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort);
    EXPECT_EQ(AwaError_Success, SessionCommon_ConnectSession(session));
    EXPECT_EQ(AwaError_Success, SessionCommon_DisconnectSession(session));
    ASSERT_FALSE(SessionCommon_IsConnected(session));
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommonWithDaemon, SessionCommon_ConnectSession_handles_call_twice)
{
    // Attempt to connect twice daemon on IPv4 address
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    SessionCommon_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort);
    EXPECT_EQ(AwaError_Success, SessionCommon_ConnectSession(session));
    EXPECT_EQ(AwaError_IPCError, SessionCommon_ConnectSession(session));
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommon, SessionCommon_PathToIDs_handles_invalid_session)
{
    AwaObjectID objectID = AWA_INVALID_ID;
    AwaObjectInstanceID objectInstanceID = AWA_INVALID_ID;
    AwaResourceID resourceID = AWA_INVALID_ID;
    ASSERT_EQ(AwaError_SessionInvalid, SessionCommon_PathToIDs(NULL, "/1/2/3", &objectID, &objectInstanceID, &resourceID));
}

TEST_F(TestSessionCommon, SessionCommon_PathToIDs_handles_null_path)
{
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    AwaObjectID objectID = AWA_INVALID_ID;
    AwaObjectInstanceID objectInstanceID = AWA_INVALID_ID;
    AwaResourceID resourceID = AWA_INVALID_ID;
    ASSERT_EQ(AwaError_PathInvalid, SessionCommon_PathToIDs(session, NULL, &objectID, &objectInstanceID, &resourceID));
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommon, SessionCommon_NewObjectDefinitionIterator_valid_inputs)
{
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    ASSERT_TRUE(NULL != session);

    AwaObjectDefinitionIterator * iterator = SessionCommon_NewObjectDefinitionIterator(session);
    ASSERT_TRUE(NULL != iterator);
    AwaObjectDefinitionIterator_Free(&iterator);
    ASSERT_TRUE(NULL == iterator);

    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommon, SessionCommon_NewObjectDefinitionIterator_invalid_inputs)
{
    AwaObjectDefinitionIterator * iterator = SessionCommon_NewObjectDefinitionIterator(NULL);
    ASSERT_TRUE(NULL == iterator);
    AwaObjectDefinitionIterator_Free(NULL);
}

class TestSessionCommonPathInvalidWithParam : public TestSessionCommon, public ::testing::WithParamInterface<const char *> {};

TEST_P(TestSessionCommonPathInvalidWithParam, SessionCommon_PathToIDs_handles_invalid_path)
{
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    AwaObjectID objectID = AWA_INVALID_ID;
    AwaObjectInstanceID objectInstanceID = AWA_INVALID_ID;
    AwaResourceID resourceID = AWA_INVALID_ID;
    ASSERT_EQ(AwaError_PathInvalid, SessionCommon_PathToIDs(session, GetParam(), &objectID, &objectInstanceID, &resourceID));
    SessionCommon_Free(&session);
}

INSTANTIATE_TEST_CASE_P(
        TestSessionCommonPathInvalidWithParamInstantiation,
        TestSessionCommonPathInvalidWithParam,
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

struct TestSessionCommonPathValidWithParamItem
{
    const char * Path;
    AwaObjectID ExpectedObjectID;
    AwaObjectInstanceID ExpectedObjectInstanceID;
    AwaResourceID ExpectedResourceID;
};

::std::ostream& operator<<(::std::ostream& os, const TestSessionCommonPathValidWithParamItem& item)
{
  return os << "Item: Path " << item.Path
            << ", ExpectedObjectID " << item.ExpectedObjectID
            << ", ExpectedObjectInstanceID " << item.ExpectedObjectInstanceID
            << ", ExpectedResourceID " << item.ExpectedResourceID;
}

class TestSessionCommonPathValidWithParam : public TestSessionCommon, public ::testing::WithParamInterface<TestSessionCommonPathValidWithParamItem>
{
};

TEST_P(TestSessionCommonPathValidWithParam, SessionCommon_PathToIDs_handles_valid_path)
{
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    AwaObjectID objectID = 42;
    AwaObjectInstanceID objectInstanceID = 19;
    AwaResourceID resourceID = 8;
    ASSERT_EQ(AwaError_Success, SessionCommon_PathToIDs(session, GetParam().Path, &objectID, &objectInstanceID, &resourceID));
    EXPECT_EQ(GetParam().ExpectedObjectID, objectID);
    EXPECT_EQ(GetParam().ExpectedObjectInstanceID, objectInstanceID);
    EXPECT_EQ(GetParam().ExpectedResourceID, resourceID);
    SessionCommon_Free(&session);
}

INSTANTIATE_TEST_CASE_P(
        TestSessionCommonPathValidWithParamInstantiation,
        TestSessionCommonPathValidWithParam,
        ::testing::Values(
            TestSessionCommonPathValidWithParamItem { "/3",             3,     AWA_INVALID_ID, AWA_INVALID_ID },
            TestSessionCommonPathValidWithParamItem { "/3/2",           3,     2,                  AWA_INVALID_ID },
            TestSessionCommonPathValidWithParamItem { "/3/2/1",         3,     2,                  1 },
            TestSessionCommonPathValidWithParamItem { "/3/2/65535",     3,     2,                  65535 },
            TestSessionCommonPathValidWithParamItem { "/65535/2/65535", 65535, 2,                  65535 },
            TestSessionCommonPathValidWithParamItem { "/0/0/0",         0,     0,                  0 }
        ));



// Test with client daemon
class TestSessionCommonWithConnectedSession : public TestSessionCommonWithDaemon
{
protected:
    virtual void SetUp()
    {
        TestSessionCommonWithDaemon::SetUp();
        session_ = SessionCommon_New(SessionType_Client);
        ASSERT_TRUE(NULL != session_);
        ASSERT_EQ(AwaError_Success, SessionCommon_SetIPCAsUDP(session_, "127.0.0.1", global::clientIpcPort));
        ASSERT_EQ(AwaError_Success, SessionCommon_ConnectSession(session_));
    }
    virtual void TearDown()
    {
        ASSERT_EQ(AwaError_Success, SessionCommon_DisconnectSession(session_));
        ASSERT_EQ(AwaError_Success, SessionCommon_Free(&session_));
        TestSessionCommonWithDaemon::TearDown();
    }

    SessionCommon * session_;
};

TEST_F(TestSessionCommon, SessionCommon_GetObjectDefinition_handles_null_session)
{
    ASSERT_EQ(NULL, SessionCommon_GetObjectDefinition(NULL, 3));
}

TEST_F(TestSessionCommonWithConnectedSession, SessionCommon_GetObjectDefinition_handles_invalid_objectID)
{
    ASSERT_EQ(NULL, SessionCommon_GetObjectDefinition(session_, AWA_INVALID_ID));
}

TEST_F(TestSessionCommonWithConnectedSession, SessionCommon_GetObjectDefinition_handles_standard_object)
{
    const AwaObjectDefinition * objectDefinition = SessionCommon_GetObjectDefinition(session_, 3);
    ASSERT_TRUE(NULL != objectDefinition);
    EXPECT_STREQ(AwaObjectDefinition_GetName(objectDefinition), "Device");
}

TEST_F(TestSessionCommonWithConnectedSession, SessionCommon_GetObjectDefinition_handles_undefined_object)
{
    const AwaObjectDefinition * objectDefinition = SessionCommon_GetObjectDefinition(session_, 3333);
    ASSERT_EQ(NULL, objectDefinition);
}

TEST_F(TestSessionCommon, SessionCommon_GetChannel_handles_null_session)
{
    ASSERT_EQ(NULL, SessionCommon_GetChannel(NULL));
}

TEST_F(TestSessionCommon, SessionCommon_GetChannel_handles_not_connected_session)
{
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    ASSERT_EQ(NULL, SessionCommon_GetChannel(NULL));
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommonWithConnectedSession, SessionCommon_GetChannel_handles_connected_session)
{
    ASSERT_TRUE(NULL != SessionCommon_GetChannel(session_));
}

TEST_F(TestSessionCommon, SessionCommon_GetDefinitionRegistry_handles_null_session)
{
    ASSERT_EQ(NULL, SessionCommon_GetDefinitionRegistry(NULL));
}

TEST_F(TestSessionCommon, SessionCommon_GetDefinitionRegistry_handles_not_connected_session)
{
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    // should return a valid definition registry
    ASSERT_TRUE(NULL != SessionCommon_GetDefinitionRegistry(session));
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommonWithConnectedSession, SessionCommon_GetDefinitionRegistry_handles_connected_session)
{
    // should return a valid definition registry
    ASSERT_TRUE(NULL != SessionCommon_GetDefinitionRegistry(session_));
}

TEST_F(TestSessionCommon, SessionCommon_CheckResourceTypeFromPath_handles_null_session)
{
    ASSERT_EQ(AwaError_SessionInvalid, SessionCommon_CheckResourceTypeFromPath(NULL, "/3/0/0", AwaResourceType_String));
}

TEST_F(TestSessionCommon, SessionCommon_CheckResourceTypeFromPath_handles_not_connected_session)
{
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    ASSERT_EQ(AwaError_SessionNotConnected, SessionCommon_CheckResourceTypeFromPath(session, "/3/0/0", AwaResourceType_String));
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommonWithConnectedSession, SessionCommon_CheckResourceTypeFromPath_handles_connected_session_with_valid_path_match)
{
    EXPECT_EQ(AwaError_Success, SessionCommon_CheckResourceTypeFromPath(session_, "/3/0/0", AwaResourceType_String));
    EXPECT_EQ(AwaError_Success, SessionCommon_CheckResourceTypeFromPath(session_, "/3/0/4", AwaResourceType_None));
    EXPECT_EQ(AwaError_Success, SessionCommon_CheckResourceTypeFromPath(session_, "/3/0/8", AwaResourceType_IntegerArray));
    EXPECT_EQ(AwaError_Success, SessionCommon_CheckResourceTypeFromPath(session_, "/3/0/9", AwaResourceType_Integer));
}

TEST_F(TestSessionCommonWithConnectedSession, SessionCommon_CheckResourceTypeFromPath_handles_connected_session_with_valid_path_mismatch)
{
    EXPECT_EQ(AwaError_TypeMismatch, SessionCommon_CheckResourceTypeFromPath(session_, "/3/0/0", AwaResourceType_None));
    EXPECT_EQ(AwaError_TypeMismatch, SessionCommon_CheckResourceTypeFromPath(session_, "/3/0/4", AwaResourceType_String));
    EXPECT_EQ(AwaError_TypeMismatch, SessionCommon_CheckResourceTypeFromPath(session_, "/3/0/8", AwaResourceType_Integer));
    EXPECT_EQ(AwaError_TypeMismatch, SessionCommon_CheckResourceTypeFromPath(session_, "/3/0/9", AwaResourceType_IntegerArray));
}

TEST_F(TestSessionCommonWithConnectedSession, SessionCommon_CheckResourceTypeFromPath_handles_connected_session_with_not_defined_path)
{
    EXPECT_EQ(AwaError_NotDefined, SessionCommon_CheckResourceTypeFromPath(session_, "/333/0/0", AwaResourceType_None));
}

TEST_F(TestSessionCommonWithConnectedSession, SessionCommon_CheckResourceTypeFromPath_handles_connected_session_with_invalid_path)
{
    EXPECT_EQ(AwaError_PathInvalid, SessionCommon_CheckResourceTypeFromPath(session_, "/invalid_path/0...", AwaResourceType_None));
}

TEST_F(TestSessionCommon, SessionCommon_GetResourceDefinitionFromPath_handles_null_session)
{
    ASSERT_EQ(NULL, SessionCommon_GetResourceDefinitionFromPath(NULL, "/3/0/0"));
}

TEST_F(TestSessionCommon, SessionCommon_GetResourceDefinitionFromPath_handles_not_connected_session)
{
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    ASSERT_EQ(NULL, SessionCommon_GetResourceDefinitionFromPath(session, "/3/0/0"));
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommonWithConnectedSession, SessionCommon_GetResourceDefinitionFromPath_handles_connected_session)
{
    const AwaResourceDefinition * definition = SessionCommon_GetResourceDefinitionFromPath(session_, "/3/0/0");
    ASSERT_TRUE(NULL != definition);
}

TEST_F(TestSessionCommon, SessionCommon_IsObjectDefined_handles_null_session_session)
{
    ASSERT_FALSE(SessionCommon_IsObjectDefined(NULL, 3));
}

TEST_F(TestSessionCommon, SessionCommon_IsObjectDefined_handles_not_connected_session)
{
    SessionCommon * session = SessionCommon_New(SessionType_Client);
    ASSERT_FALSE(SessionCommon_IsObjectDefined(NULL, 3));
    SessionCommon_Free(&session);
}

TEST_F(TestSessionCommonWithConnectedSession, SessionCommon_IsObjectDefined_handles_connected_session_valid_object)
{
    EXPECT_TRUE(SessionCommon_IsObjectDefined(session_, 3));
    EXPECT_TRUE(SessionCommon_IsObjectDefined(session_, 4));
}

TEST_F(TestSessionCommonWithConnectedSession, SessionCommon_IsObjectDefined_handles_connected_session_invalid_object)
{
    EXPECT_FALSE(SessionCommon_IsObjectDefined(session_, 333));
    EXPECT_FALSE(SessionCommon_IsObjectDefined(session_, 444));
}

// difficult to unit test SessionCommon_GetSessionType

// difficult to unit test SessionCommon_GetSessionID

} // namespace Awa

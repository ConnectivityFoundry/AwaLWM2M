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

#include "xmltree.h"

#include "ipc.h"
#include "memalloc.h"
#include "error.h"
#include "support/support.h"

namespace Awa {

class TestIPC : public TestClientBase {};

class TestIPCWithDaemon : public TestClientWithDaemonBase {};

TEST_F(TestIPC, IPCInfo_NewUDP_and_Free)
{
    IPCInfo * info = IPCInfo_NewUDP("127.0.0.1", 12345);
    ASSERT_TRUE(NULL != info);
    IPCInfo_Free(&info);
    ASSERT_EQ(NULL, info);
}

TEST_F(TestIPC, IPCInfo_Free_handles_null)
{
    IPCInfo_Free(NULL);
}

TEST_F(TestIPC, IPCMessage_New_and_Free)
{
    IPCMessage * message = IPCMessage_New();
    ASSERT_TRUE(NULL != message);
    IPCMessage_Free(&message);
    ASSERT_EQ(NULL, message);
}

TEST_F(TestIPC, IPCMessage_Free_handles_null)
{
    IPCMessage_Free(NULL);
}

TEST_F(TestIPC, IPCChannel_New_and_Free)
{
    IPCInfo * info = IPCInfo_NewUDP("127.0.0.1", 12345);
    IPCChannel * channel = IPCChannel_New(info);
    ASSERT_TRUE(NULL != channel);
    IPCChannel_Free(&channel);
    ASSERT_EQ(NULL, channel);
    IPCInfo_Free(&info);
}

TEST_F(TestIPC, IPCChannel_New_info_is_null)
{
    IPCChannel * channel = IPCChannel_New(NULL);
    ASSERT_EQ(NULL, channel);
}

TEST_F(TestIPC, IPCChannel_Free_handles_null)
{
    IPCChannel_Free(NULL);
}

TEST_F(TestIPC, IPCMessage_NewPlus_handles_null)
{
    EXPECT_EQ(NULL, IPCMessage_NewPlus(NULL, NULL, 1));
    EXPECT_EQ(NULL, IPCMessage_NewPlus("Request", NULL, 1));
    EXPECT_EQ(NULL, IPCMessage_NewPlus(NULL, "Get", 1));
}

TEST_F(TestIPC, IPCMessage_NewPlus_handles_valid)
{
    IPCMessage * message = IPCMessage_NewPlus("Request", "Get", 1);
    ASSERT_TRUE(NULL != message);
    IPCMessage_Free(&message);
}

TEST_F(TestIPC, IPCMessage_SetType_called_with_null_type)
{
    EXPECT_EQ(InternalError_ParameterInvalid, IPCMessage_SetType(NULL, "type", "subType"));
    EXPECT_EQ(InternalError_ParameterInvalid, IPCMessage_SetType(NULL, "type", NULL));
    EXPECT_EQ(InternalError_ParameterInvalid, IPCMessage_SetType(NULL, NULL, "subType"));
    EXPECT_EQ(InternalError_ParameterInvalid, IPCMessage_SetType(NULL, NULL, NULL));
    IPCMessage * message = IPCMessage_New();
    EXPECT_EQ(InternalError_ParameterInvalid, IPCMessage_SetType(message, NULL, "subType"));
    EXPECT_EQ(InternalError_ParameterInvalid, IPCMessage_SetType(message, "type", NULL));
    EXPECT_EQ(InternalError_ParameterInvalid, IPCMessage_SetType(message, NULL, NULL));
    IPCMessage_Free(&message);
}

TEST_F(TestIPC, IPCMessage_GetType_called_with_null_type)
{
    const char * SubType = NULL;
    const char * Type = NULL;

    EXPECT_EQ(InternalError_ParameterInvalid, IPCMessage_GetType(NULL, &Type, &SubType));
    EXPECT_EQ(InternalError_ParameterInvalid, IPCMessage_GetType(NULL, &Type, NULL));
    EXPECT_EQ(InternalError_ParameterInvalid, IPCMessage_GetType(NULL, NULL, &SubType));
    EXPECT_EQ(InternalError_ParameterInvalid, IPCMessage_GetType(NULL, NULL, NULL));
    IPCMessage * message = IPCMessage_New();
    // type or subType can be NULL, but not both:
    EXPECT_EQ(InternalError_ParameterInvalid, IPCMessage_GetType(message, NULL, NULL));
    IPCMessage_Free(&message);
}

TEST_F(TestIPC, IPCMessage_SetType_GetType_are_equal)
{
    const char * expectedSubType = "ABCDEF";
    const char * expectedType = "123456";
    IPCMessage * message = IPCMessage_New();
    EXPECT_EQ(InternalError_Success, IPCMessage_SetType(message, expectedType, expectedSubType));

    const char * returnedSubType = NULL;
    const char * returnedType = NULL;
    EXPECT_EQ(InternalError_Success, IPCMessage_GetType(message, &returnedType, &returnedSubType));
    EXPECT_STREQ(expectedSubType, returnedSubType);
    EXPECT_STREQ(expectedType, returnedType);

    returnedSubType = NULL;
    returnedType = NULL;
    EXPECT_EQ(InternalError_Success, IPCMessage_GetType(message, &returnedType, NULL));
    EXPECT_STREQ(expectedType, returnedType);

    returnedSubType = NULL;
    returnedType = NULL;
    EXPECT_EQ(InternalError_Success, IPCMessage_GetType(message, NULL, &returnedSubType));
    EXPECT_STREQ(expectedSubType, returnedSubType);

    IPCMessage_Free(&message);
}

TEST_F(TestIPC, IPCMessage_GetType_on_empty_message)
{
    const char * returnedSubType = NULL;
    const char * returnedType = NULL;
    IPCMessage * message = IPCMessage_New();

    ASSERT_EQ(InternalError_InvalidMessage, IPCMessage_GetType(message, &returnedType, &returnedSubType));

    IPCMessage_Free(&message);
}

TEST_F(TestIPC, IPCMessage_SetSessionID_handles_null)
{
    EXPECT_EQ(InternalError_InvalidMessage, IPCMessage_SetSessionID(NULL, 1));
}

TEST_F(TestIPC, IPCMessage_GetSessionID_handles_null)
{
    EXPECT_EQ(-1, IPCMessage_GetSessionID(NULL));
}

TEST_F(TestIPC, IPCMessage_SetSessionID_without_type)
{
    IPCMessage * message = IPCMessage_New();
    ASSERT_TRUE(NULL != message);

    // no call to IPCMessage_SetType
    EXPECT_EQ(InternalError_InvalidMessage, IPCMessage_SetSessionID(message, 42));

    IPCMessage_Free(&message);
}

TEST_F(TestIPC, IPCMessage_SetSessionID_GetSessionID_are_equal)
{
    IPCMessage * message = IPCMessage_New();
    ASSERT_TRUE(NULL != message);
    IPCMessage_SetType(message, "Request", "Get");

    EXPECT_EQ(InternalError_Success, IPCMessage_SetSessionID(message, 42));
    EXPECT_EQ(42, IPCMessage_GetSessionID(message));

    EXPECT_EQ(InternalError_Success, IPCMessage_SetSessionID(message, 99));
    EXPECT_EQ(99, IPCMessage_GetSessionID(message));

    EXPECT_EQ(InternalError_Success, IPCMessage_SetSessionID(message, 711123));
    EXPECT_EQ(711123, IPCMessage_GetSessionID(message));

    IPCMessage_Free(&message);
}

TEST_F(TestIPC, IPCMessage_GetSessionID_default)
{
    IPCMessage * message = IPCMessage_New();
    ASSERT_TRUE(NULL != message);
    IPCMessage_SetType(message, "Request", "Get");

    EXPECT_EQ(-1, IPCMessage_GetSessionID(message));

    IPCMessage_Free(&message);
}

TEST_F(TestIPC, IPCMessage_SetSessionID_invalid_ID_clears)
{
    IPCMessage * message = IPCMessage_New();
    ASSERT_TRUE(NULL != message);
    IPCMessage_SetType(message, "Request", "Get");

    // -1 clears existing ID
    EXPECT_EQ(InternalError_Success, IPCMessage_SetSessionID(message, 56));
    EXPECT_EQ(56, IPCMessage_GetSessionID(message));

    EXPECT_EQ(InternalError_Success, IPCMessage_SetSessionID(message, -1));
    EXPECT_EQ(-1, IPCMessage_GetSessionID(message));

    IPCMessage_Free(&message);
}

TEST_F(TestIPC, IPCMessage_SetSessionID_zero_ID)
{
    IPCMessage * message = IPCMessage_New();
    ASSERT_TRUE(NULL != message);
    IPCMessage_SetType(message, "Request", "Get");

    // 0 behaves like any other valid session ID
    EXPECT_EQ(InternalError_Success, IPCMessage_SetSessionID(message, 0));
    EXPECT_EQ(0, IPCMessage_GetSessionID(message));

    EXPECT_EQ(InternalError_Success, IPCMessage_SetSessionID(message, 56));
    EXPECT_EQ(56, IPCMessage_GetSessionID(message));

    EXPECT_EQ(InternalError_Success, IPCMessage_SetSessionID(message, 0));
    EXPECT_EQ(0, IPCMessage_GetSessionID(message));

    IPCMessage_Free(&message);
}

TEST_F(TestIPC, IPC_SerialiseMessageToXML)
{
    const char * expectedSubType = "ABCDEF";
    IPCMessage * message = IPCMessage_New();
    EXPECT_EQ(InternalError_Success, IPCMessage_SetType(message, IPC_MESSAGE_TYPE_REQUEST, expectedSubType));
    char * buffer = IPC_SerialiseMessageToXML(message);
    const size_t bufferLen = strlen(buffer);
    TreeNode root = TreeNode_ParseXML(reinterpret_cast<uint8_t *>(buffer), bufferLen, true);

    char path[] = "Request/Type";
    TreeNode subTypeNode = TreeNode_Navigate(root, path);
    ASSERT_TRUE(NULL != subTypeNode);
    ASSERT_STREQ(expectedSubType, reinterpret_cast<const char *>(TreeNode_GetValue(subTypeNode)));

    Tree_Delete(root);
    Awa_MemSafeFree(buffer);
    IPCMessage_Free(&message);
}

// Test calling SetType twice
TEST_F(TestIPC, IPC_SerialiseMessageToXML_called_twice)
{
#define EXPECTED_TYPE "ABCDEF"
#define EXPECTED_XML_PATH EXPECTED_TYPE "/Type"

    const char * expectedType = EXPECTED_TYPE;
    const char * expectedSubType = "ZYXWVU";
    IPCMessage * message = IPCMessage_New();

    // call SetType twice
    EXPECT_EQ(InternalError_Success, IPCMessage_SetType(message, "1234354", "subsubsub"));
    EXPECT_EQ(InternalError_Success, IPCMessage_SetType(message, expectedType, expectedSubType));

    char * buffer = IPC_SerialiseMessageToXML(message);
    const size_t bufferLen = strlen(buffer);
    TreeNode root = TreeNode_ParseXML(reinterpret_cast<uint8_t *>(buffer), bufferLen, true);

    char path[] = EXPECTED_XML_PATH;
    TreeNode subTypeNode = TreeNode_Navigate(root, path);
    ASSERT_TRUE(NULL != subTypeNode);
    ASSERT_STREQ(expectedSubType, reinterpret_cast<const char *>(TreeNode_GetValue(subTypeNode)));

    Tree_Delete(root);
    Awa_MemSafeFree(buffer);
    IPCMessage_Free(&message);

#undef EXPECTED_TYPE
#undef EXPECTED_XML_PATH
}

TEST_F(TestIPC, IPC_SendAndReceive_handles_null_channel)
{
    IPCMessage * request = IPCMessage_New();
    IPCMessage * response = NULL;
    IPCMessage_SetType(request, "Request", IPC_MESSAGE_SUB_TYPE_CONNECT);
    EXPECT_EQ(AwaError_IPCError, IPC_SendAndReceive(NULL, request, &response, defaults::timeout));
    IPCMessage_Free(&request);
}

TEST_F(TestIPC, IPC_SendAndReceive_handles_null_request)
{
    IPCInfo * info = IPCInfo_NewUDP(detail::NonRoutableIPv4Address, 55555);  EXPECT_TRUE(NULL != info);
    IPCChannel * channel = IPCChannel_New(info);        EXPECT_TRUE(NULL != channel);
    IPCMessage * response = NULL;
    EXPECT_EQ(AwaError_IPCError, IPC_SendAndReceive(channel, NULL, &response, defaults::timeout));
    IPCChannel_Free(&channel);
    IPCInfo_Free(&info);
}

TEST_F(TestIPC, IPC_SendAndReceive_handles_null_response)
{
    // IPC_SendAndReceive(channel, request, NULL) should still complete successfully,
    // however there will be no attempt to receive a response.
    IPCInfo * info = IPCInfo_NewUDP(detail::NonRoutableIPv4Address, 55555);  EXPECT_TRUE(NULL != info);
    IPCChannel * channel = IPCChannel_New(info);        EXPECT_TRUE(NULL != channel);
    IPCMessage * request = IPCMessage_New();
    IPCMessage_SetType(request, "Request", IPC_MESSAGE_SUB_TYPE_CONNECT);
    EXPECT_EQ(AwaError_Success, IPC_SendAndReceive(channel, request, NULL, defaults::timeout));
    IPCMessage_Free(&request);
    IPCChannel_Free(&channel);
    IPCInfo_Free(&info);
}

//TEST_F(TestIPC, IPC_SendAndReceive_handles_invalid_channel)
//{
//  What is an invalid IPCChannel?
//}

TEST_F(TestIPC, IPC_SendAndReceive_failed_response_is_null)
{
    IPCInfo * info = IPCInfo_NewUDP(detail::NonRoutableIPv4Address, 55555);  EXPECT_TRUE(NULL != info);
    IPCChannel * channel = IPCChannel_New(info);          EXPECT_TRUE(NULL != channel);
    IPCMessage * request = IPCMessage_New();
    IPCMessage * response = NULL;
    IPCMessage_SetType(request, "Request", IPC_MESSAGE_SUB_TYPE_CONNECT);

    EXPECT_NE(AwaError_Success, IPC_SendAndReceive(channel, request, &response, defaults::timeout));
    ASSERT_EQ(NULL, response);

    IPCMessage_Free(&request);
    IPCChannel_Free(&channel);
    IPCInfo_Free(&info);
}


TEST_F(TestIPCWithDaemon, IPC_SendAndReceive_successful_response_is_not_null)
{
    IPCInfo * info = IPCInfo_NewUDP("127.0.0.1", global::clientIpcPort);  EXPECT_TRUE(NULL != info);
    IPCChannel * channel = IPCChannel_New(info);                          EXPECT_TRUE(NULL != channel);
    IPCMessage * request = IPCMessage_New();
    IPCMessage * response = NULL;
    IPCMessage_SetType(request, IPC_MESSAGE_TYPE_REQUEST, IPC_MESSAGE_SUB_TYPE_CONNECT);

    EXPECT_EQ(AwaError_Success, IPC_SendAndReceive(channel, request, &response, defaults::timeout));
    ASSERT_TRUE(NULL != response);

    IPCMessage_Free(&request);
    IPCMessage_Free(&response);
    IPCChannel_Free(&channel);
    IPCInfo_Free(&info);
}

// TODO: test with wrong SetType...

TEST_F(TestIPCWithDaemon, IPC_SendAndReceive_successful_and_correct_type)
{
    IPCInfo * info = IPCInfo_NewUDP("127.0.0.1", global::clientIpcPort);  EXPECT_TRUE(NULL != info);
    IPCChannel * channel = IPCChannel_New(info);                          EXPECT_TRUE(NULL != channel);
    IPCMessage * request = IPCMessage_New();
    IPCMessage * response = NULL;
    IPCMessage_SetType(request, IPC_MESSAGE_TYPE_REQUEST, IPC_MESSAGE_SUB_TYPE_CONNECT);

    EXPECT_EQ(AwaError_Success, IPC_SendAndReceive(channel, request, &response, defaults::timeout));
    EXPECT_TRUE(NULL != response);

    const char * Type = NULL;
    const char * subType = NULL;
    ASSERT_EQ(InternalError_Success, IPCMessage_GetType(response, &Type, &subType));

    ASSERT_STREQ(Type, IPC_MESSAGE_TYPE_RESPONSE);
    ASSERT_STREQ(subType, IPC_MESSAGE_SUB_TYPE_CONNECT);

    IPCMessage_Free(&request);
    IPCMessage_Free(&response);
    IPCChannel_Free(&channel);
    IPCInfo_Free(&info);
}

TEST_F(TestIPCWithDaemon, IPC_SendAndReceive_successful_response_content_is_not_null)
{
    IPCInfo * info = IPCInfo_NewUDP("127.0.0.1", global::clientIpcPort);  EXPECT_TRUE(NULL != info);
    IPCChannel * channel = IPCChannel_New(info);                          EXPECT_TRUE(NULL != channel);
    IPCMessage * request = IPCMessage_New();
    IPCMessage * response = NULL;
    IPCMessage_SetType(request, IPC_MESSAGE_TYPE_REQUEST, IPC_MESSAGE_SUB_TYPE_CONNECT);

    EXPECT_EQ(AwaError_Success, IPC_SendAndReceive(channel, request, &response, defaults::timeout));
    EXPECT_TRUE(NULL != response);

    ASSERT_TRUE(NULL != IPCMessage_GetContentNode(response));

    IPCMessage_Free(&request);
    IPCMessage_Free(&response);
    IPCChannel_Free(&channel);
    IPCInfo_Free(&info);
}

TEST_F(TestIPCWithDaemon, IPC_SendAndReceive_successful_response_code_is_success)
{
    IPCInfo * info = IPCInfo_NewUDP("127.0.0.1", global::clientIpcPort);  EXPECT_TRUE(NULL != info);
    IPCChannel * channel = IPCChannel_New(info);                          EXPECT_TRUE(NULL != channel);
    IPCMessage * request = IPCMessage_New();
    IPCMessage * response = NULL;
    IPCMessage_SetType(request, IPC_MESSAGE_TYPE_REQUEST, IPC_MESSAGE_SUB_TYPE_CONNECT);

    EXPECT_EQ(AwaError_Success, IPC_SendAndReceive(channel, request, &response, defaults::timeout));
    EXPECT_TRUE(NULL != response);

    ASSERT_EQ(IPCResponseCode_Success, IPCMessage_GetResponseCode(response));

    IPCMessage_Free(&request);
    IPCMessage_Free(&response);
    IPCChannel_Free(&channel);
    IPCInfo_Free(&info);
}

} // namespace Awa

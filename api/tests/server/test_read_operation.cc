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
#include "arrays.h"
#include "support/support.h"
#include "utils.h"
#include "memalloc.h"
#include "support/definition.h"

namespace Awa {

class TestReadOperation : public TestServerBase {};

class TestReadOperationWithConnectedSession : public TestServerAndClientWithConnectedSession {};

class TestReadOperationWithConnectedServerAndClientSession : public TestServerAndClientWithConnectedSession {};

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadOperation_New_returns_valid_operation_and_free_works)
{
    // test that AwaServerReadOperation_Free works via valgrind
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Free(&readOperation));
}

TEST_F(TestReadOperation, AwaServerReadOperation_New_handles_null_session)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(NULL);
    ASSERT_EQ(NULL, readOperation);
}

TEST_F(TestReadOperation, AwaServerReadOperation_New_handles_invalid_session)
{
    // An invalid session is one that is not connected
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(NULL, AwaServerReadOperation_New(session));

    // Set up IPC - still not enough
    AwaServerSession_SetIPCAsUDP(session, "127.0.0.1", global::serverIpcPort);
    EXPECT_EQ(NULL, AwaServerReadOperation_New(session));

    AwaServerSession_Free(&session);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadOperation_Free_nulls_pointer)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Free(&readOperation));
    ASSERT_EQ(NULL, readOperation);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadOperation_Free_handles_null)
{
    EXPECT_EQ(AwaError_OperationInvalid, AwaServerReadOperation_Free(NULL));
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadOperation_Free_handles_null_pointer)
{
    AwaServerReadOperation * readOperation = NULL;
    EXPECT_EQ(AwaError_OperationInvalid, AwaServerReadOperation_Free(&readOperation));
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadOperation_Free_handles_invalid_session)
{
    // Session is freed before the operation (use a local session):
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session, "127.0.0.1", global::serverIpcPort));
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(session);
    AwaServerSession_Free(&session);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Free(&readOperation));
    // expect no crash or memory leaks
}

// TODO

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadOperation_AddPath_handles_valid_path)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);

    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/1000/0/1"));
    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadOperation_AddPath_handles_invalid_path)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);

    EXPECT_EQ(AwaError_PathInvalid, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "a/n in/valid/ path"));
    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadOperation_AddPath_handles_null_path)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);
    ASSERT_EQ(AwaError_PathInvalid, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, NULL));
    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadOperation_AddPath_handles_null_clientID)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerReadOperation_AddPath(readOperation, NULL, "/1000/0/1"));
    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestReadOperation, AwaServerReadOperation_AddPath_handles_null_operation)
{
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerReadOperation_AddPath(NULL, global::clientEndpointName, "/1000/0/1"));
}

// FIXME: FLOWDM-370
TEST_F(TestReadOperationWithConnectedSession, DISABLED_AwaServerReadOperation_AddPath_handles_invalid_operation)
{
    // Consider if the parent session has been freed early
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session, "127.0.0.1", global::serverIpcPort));
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(session);
    ASSERT_TRUE(NULL != readOperation);

    // Free the session
    AwaServerSession_Free(&session);

    // TODO: we need the session to keep track of all operations, and
    // invalidate them when freed so they can detect this situation.

    ASSERT_EQ(AwaError_SessionInvalid, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/1000/0/1"));

    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Free(&readOperation));
    // expect no crash or memory leaks
}

TEST_F(TestReadOperationWithConnectedSession, DISABLED_AwaServerReadOperation_AddPath_handles_more_general_path)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_); ASSERT_TRUE(NULL != readOperation);
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/0"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));
    const AwaServerReadResponse * readResponse = NULL;//AwaServerReadOperation_GetResponse(readOperation); ASSERT_TRUE(NULL != readResponse);

    EXPECT_TRUE(AwaServerReadResponse_ContainsPath(readResponse, "/3/0/0"));
    EXPECT_TRUE(AwaServerReadResponse_ContainsPath(readResponse, "/3/0/1"));
    EXPECT_TRUE(AwaServerReadResponse_ContainsPath(readResponse, "/3/0/2"));
    // ... expecting the rest of /3/0

    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadOperation_Perform_handles_valid_operation)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);

    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));
    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadOperation_Perform_handles_null_operation)
{
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerReadOperation_Perform(NULL, global::timeout));
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadOperation_Perform_handles_negative_timeout)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);

    ASSERT_EQ(AwaError_OperationInvalid, AwaServerReadOperation_Perform(readOperation, -1));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Free(&readOperation));
}

// FIXME: FLOWDM-360
TEST_F(TestReadOperationWithConnectedSession, DISABLED_AwaServerReadOperation_Perform_handles_zero_timeout)
{
    // how?
}

// FIXME: FLOWDM-360
TEST_F(TestReadOperationWithConnectedSession, DISABLED_AwaServerReadOperation_Perform_handles_short_timeout)
{
    // how?
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadOperation_Perform_honours_timeout)
{
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session, "0.0.0.0", global::serverIpcPort));
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(session);
    ASSERT_TRUE(NULL != readOperation);
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/1"));

    // Make the server unresponsive
    TestServerWithDaemonBase::daemon_.Pause();
    BasicTimer timer;
    timer.Start();
    EXPECT_EQ(AwaError_Timeout, AwaServerReadOperation_Perform(readOperation, global::timeout));
    timer.Stop();
    EXPECT_TRUE(ElapsedTimeExceeds(timer.TimeElapsed_Milliseconds(), global::timeout)) << "Time elapsed: " << timer.TimeElapsed_Milliseconds() << "ms";
    TestServerWithDaemonBase::daemon_.Unpause();

    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Free(&readOperation));
    AwaServerSession_Free(&session);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadOperation_Perform_handles_invalid_operation_no_content)
{
    // Test behaviour when operation has no content
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);

    ASSERT_EQ(AwaError_OperationInvalid, AwaServerReadOperation_Perform(readOperation, global::timeout));
    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadOperation_Perform_handles_disconnected_session)
{
    // Test behaviour when parent session has been disconnected
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/1"));

    AwaServerSession_Disconnect(server_session_);

    ASSERT_EQ(AwaError_SessionNotConnected, AwaServerReadOperation_Perform(readOperation, global::timeout));
    AwaServerReadOperation_Free(&readOperation);
}

// FIXME: FLOWDM-370
TEST_F(TestReadOperationWithConnectedSession, DISABLED_AwaServerReadOperation_Perform_handles_invalid_operation_freed_session)
{
    // Test behaviour when parent session has been freed
    ASSERT_FALSE(1);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadOperation_Perform_handles_resource)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);

    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));

    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != readResponse);
    ASSERT_TRUE(AwaServerReadResponse_ContainsPath(readResponse, "/3/0/1"));

    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestReadOperationWithConnectedServerAndClientSession, AwaServerReadOperation_Perform_handles_Read_only_resource)
{
    // should fail - resource is write only.

    ObjectDescription object = { 1000, "Object1000", 0, 1, {
            ResourceDescription(0, "Resource0", AwaResourceType_Time, 0, 1, AwaResourceOperations_WriteOnly),
        }};
    EXPECT_EQ(AwaError_Success, Define(client_session_, object));
    EXPECT_EQ(AwaError_Success, Define(server_session_, object));

    WaitForClientDefinition(AwaObjectDefinition_GetID(object.GetDefinition()));

    //create the object instance on the client
    AwaClientSetOperation * clientSet = AwaClientSetOperation_New(client_session_);
    EXPECT_TRUE(clientSet != NULL);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(clientSet, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(clientSet, "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(clientSet, global::timeout));
    AwaClientSetOperation_Free(&clientSet);

    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);

    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/1000/0/0"));
    EXPECT_EQ(AwaError_Response, AwaServerReadOperation_Perform(readOperation, global::timeout));

    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != readResponse);
    EXPECT_TRUE(AwaServerReadResponse_ContainsPath(readResponse, "/1000/0/0"));

    const AwaPathResult * pathResult = AwaServerReadResponse_GetPathResult(readResponse, "/1000/0/0");
    EXPECT_EQ(AwaError_LWM2MError, AwaPathResult_GetError(pathResult));
    EXPECT_EQ(AwaLWM2MError_MethodNotAllowed, AwaPathResult_GetLWM2MError(pathResult));
    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadOperation_Perform_handles_object_instance)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);

    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));

    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != readResponse);
    ASSERT_TRUE(AwaServerReadResponse_ContainsPath(readResponse, "/3/0"));

    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadOperation_Perform_handles_object)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);

    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));

    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != readResponse);
    ASSERT_TRUE(AwaServerReadResponse_ContainsPath(readResponse, "/3"));

    AwaServerReadOperation_Free(&readOperation);
}


TEST_F(TestReadOperationWithConnectedSession, AwaServerReadOperation_GetResponse_handles_null_operation)
{
    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(NULL, global::clientEndpointName);
    ASSERT_TRUE(NULL == readResponse);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadOperation_GetResponse_handles_null_clientID)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);

    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));

    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, NULL);
    ASSERT_TRUE(NULL == readResponse);

    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadOperation_GetResponse_handles_invalid_operation)
{
    //Consider if we haven't called ProcessReadOperation
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);

    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/1"));

    //haven't processed read operation, so read response should return NULL
    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL == readResponse);

    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadOperation_GetResponse_handles_reusing_operation)
{
    //Test we can call AwaServerReadOperation_GetResponse twice and reuse the same operation
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);

    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));

    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != readResponse);

    const AwaServerReadResponse * readResponse2 = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != readResponse2);

    AwaServerReadOperation_Free(&readOperation);
}



///***********************************************************************************************************
// * ReadValue parameterised tests
// */

namespace readDetail
{

struct TestReadResource
{
    AwaError expectedResult;
    const void * expectedValue;
    int expectedValueSize;
    const char * path;
    bool UseResponse;
    AwaResourceType type;
};

::std::ostream& operator<<(::std::ostream& os, const TestReadResource& item)
{
  return os << "Item: expectedResult " << item.expectedResult
            << ", expectedValue " << item.expectedValue
            << ", expectedValueSize " << item.expectedValueSize
            << ", path " << item.path
            << ", UseResponse " << item.UseResponse
            << ", type " << item.type;
}

const AwaInteger expectedInteger1 = 100;
const char * expectedString1 = "Lightweight M2M Server";
const AwaFloat expectedFloat1 = 1.337;
const AwaTime expectedTime1 = 0xA20AD72B;
const AwaBoolean expectedBoolean1 = true;

const char dummyOpaqueData[] = {'a',0,'x','\0', 123};
const AwaOpaque expectedOpaque1 = {(void*) dummyOpaqueData, sizeof(dummyOpaqueData)};
const AwaObjectLink expectedObjectLink1 = { 3, 0 };

// Add a TestReadResource for each type except the type given, so we can ensure the type doesn't match
void BuildReadTypeMismatchContainer(std::vector<readDetail::TestReadResource>& container, const char * path, AwaResourceType skip)
{
    for (int type = AwaResourceType_String; type != AwaResourceType_StringArray; type++)
    {
        if (type == skip)
            continue;
        container.push_back(readDetail::TestReadResource {AwaError_TypeMismatch,            NULL,     -1,    path,   true,   static_cast<AwaResourceType>(type)});
    }
}

const std::vector<readDetail::TestReadResource> BuildReadTypeMismatchContainers()
{
    std::vector<readDetail::TestReadResource> container;
    BuildReadTypeMismatchContainer(container, "/10000/0/1", AwaResourceType_String);
    BuildReadTypeMismatchContainer(container, "/10000/0/2", AwaResourceType_Integer);
    BuildReadTypeMismatchContainer(container, "/10000/0/3", AwaResourceType_Float);
    BuildReadTypeMismatchContainer(container, "/10000/0/4", AwaResourceType_Boolean);
    BuildReadTypeMismatchContainer(container, "/10000/0/5", AwaResourceType_Opaque);
    BuildReadTypeMismatchContainer(container, "/10000/0/6", AwaResourceType_Time);
    BuildReadTypeMismatchContainer(container, "/10000/0/7", AwaResourceType_ObjectLink);
    return container;
}

} // namespace readDetail

class TestReadValue : public TestReadOperationWithConnectedSession, public ::testing::WithParamInterface< readDetail::TestReadResource>
{
protected:

    TestReadValue() :
        readOperation_(nullptr),
        readResponse_(nullptr) {}

    void SetUp() {
        TestReadOperationWithConnectedSession::SetUp();

        //Define and set our custom objects - TODO define in some common class for read, set etc?
        AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(server_session_);
        AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(client_session_);
        EXPECT_TRUE(serverDefineOperation != NULL);
        EXPECT_TRUE(clientDefineOperation != NULL);

        AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(10000, "Test Object", 0, 1);
        ASSERT_TRUE(NULL != customObjectDefinition);

        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsNoType(customObjectDefinition, 0, "Test None Resource", false, AwaResourceOperations_Execute));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsString(customObjectDefinition, 1, "Test String Resource", true, AwaResourceOperations_ReadWrite, readDetail::expectedString1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(customObjectDefinition, 2, "Test Integer Resource", true, AwaResourceOperations_ReadWrite, readDetail::expectedInteger1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloat(customObjectDefinition, 3, "Test Float Resource", true, AwaResourceOperations_ReadWrite, readDetail::expectedFloat1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBoolean(customObjectDefinition, 4, "Test Boolean Resource", true, AwaResourceOperations_ReadWrite, readDetail::expectedBoolean1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque(customObjectDefinition, 5, "Test Opaque Resource", true, AwaResourceOperations_ReadWrite, readDetail::expectedOpaque1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTime(customObjectDefinition, 6, "Test Time Resource", true, AwaResourceOperations_ReadWrite, readDetail::expectedTime1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLink(customObjectDefinition, 7, "Test ObjectLink Resource", true, AwaResourceOperations_ReadWrite, readDetail::expectedObjectLink1));

        // define another object that we can use for "path not in result" tests
        AwaObjectDefinition * customObjectDefinition2 = AwaObjectDefinition_New(10001, "Test Object 2", 0, 1);
        ASSERT_TRUE(NULL != customObjectDefinition2);

        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsNoType(customObjectDefinition2, 0, "Test None Resource", false, AwaResourceOperations_Execute));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsString(customObjectDefinition2, 1, "Test String Resource", true, AwaResourceOperations_ReadWrite, readDetail::expectedString1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(customObjectDefinition2, 2, "Test Integer Resource", true, AwaResourceOperations_ReadWrite, readDetail::expectedInteger1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloat(customObjectDefinition2, 3, "Test Float Resource", true, AwaResourceOperations_ReadWrite, readDetail::expectedFloat1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBoolean(customObjectDefinition2, 4, "Test Boolean Resource", true, AwaResourceOperations_ReadWrite, readDetail::expectedBoolean1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque(customObjectDefinition2, 5, "Test Opaque Resource", true, AwaResourceOperations_ReadWrite, readDetail::expectedOpaque1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTime(customObjectDefinition2, 6, "Test Time Resource", true, AwaResourceOperations_ReadWrite, readDetail::expectedTime1));

        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLink(customObjectDefinition2, 7, "Test ObjectLink Resource", true, AwaResourceOperations_ReadWrite, readDetail::expectedObjectLink1));
        EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition));
        EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition2));
        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition2));
        ASSERT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, global::timeout));
        ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, global::timeout));

        AwaObjectDefinition_Free(&customObjectDefinition);
        AwaObjectDefinition_Free(&customObjectDefinition2);
        AwaServerDefineOperation_Free(&serverDefineOperation);
        AwaClientDefineOperation_Free(&clientDefineOperation);

        //Create a basic set operation to create our custom objects - TODO remove once we can set default values
        AwaClientSetOperation * setOperation = AwaClientSetOperation_New(client_session_);
        ASSERT_TRUE(NULL != setOperation);

        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));

        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsCString(setOperation, "/10000/0/1", readDetail::expectedString1));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsInteger(setOperation, "/10000/0/2", readDetail::expectedInteger1));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsFloat(setOperation, "/10000/0/3", readDetail::expectedFloat1));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsBoolean(setOperation, "/10000/0/4", readDetail::expectedBoolean1));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsOpaque(setOperation, "/10000/0/5", readDetail::expectedOpaque1));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsTime(setOperation, "/10000/0/6", readDetail::expectedTime1));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsObjectLink(setOperation, "/10000/0/7", readDetail::expectedObjectLink1));

        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, global::timeout));
        AwaClientSetOperation_Free(&setOperation);


        readOperation_ = AwaServerReadOperation_New(server_session_);
        ASSERT_TRUE(NULL != readOperation_);

        // list of paths to objects we require
        ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation_, global::clientEndpointName, "/10000"));

        ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation_, 0));
        readResponse_ = AwaServerReadOperation_GetResponse(readOperation_, global::clientEndpointName);
        ASSERT_TRUE(NULL != readResponse_);

    }

    void TearDown() {
        TestReadOperationWithConnectedSession::TearDown();
        AwaServerReadOperation_Free(&readOperation_);
    }

    AwaServerReadOperation * readOperation_;
    const AwaServerReadResponse * readResponse_;
};

TEST_P(TestReadValue, TestReadValueInstantiation)
{
    readDetail::TestReadResource data = GetParam();
    const AwaServerReadResponse * readResponse = data.UseResponse ? this->readResponse_ : NULL;
    void * value = (void *)1;  // not null

    AwaObjectLink receivedObjectLink = {0, 0};
    AwaOpaque receivedOpaque = {NULL, 0};

    switch (data.type)
    {
    case AwaResourceType_String:
        ASSERT_EQ(data.expectedResult, AwaServerReadResponse_GetValueAsCStringPointer(readResponse, data.path, (const char **)&value));
        break;
    case AwaResourceType_Integer:
        ASSERT_EQ(data.expectedResult, AwaServerReadResponse_GetValueAsIntegerPointer(readResponse, data.path, (const AwaInteger **)&value));
        break;
    case AwaResourceType_Float:
        ASSERT_EQ(data.expectedResult, AwaServerReadResponse_GetValueAsFloatPointer(readResponse, data.path, (const AwaFloat **)&value));
        break;
    case AwaResourceType_Boolean:
        ASSERT_EQ(data.expectedResult, AwaServerReadResponse_GetValueAsBooleanPointer(readResponse, data.path, (const AwaBoolean **)&value));
        break;
    case AwaResourceType_Opaque:
        ASSERT_EQ(data.expectedResult, AwaServerReadResponse_GetValueAsOpaque(readResponse, data.path, &receivedOpaque));
        ASSERT_EQ(data.expectedResult, AwaServerReadResponse_GetValueAsOpaquePointer(readResponse, data.path, (const AwaOpaque **)&value));
        break;
    case AwaResourceType_Time:
        ASSERT_EQ(data.expectedResult, AwaServerReadResponse_GetValueAsTimePointer(readResponse, data.path, (const AwaTime **)&value));
        break;
    case AwaResourceType_ObjectLink:
        ASSERT_EQ(data.expectedResult, AwaServerReadResponse_GetValueAsObjectLink(readResponse, data.path, &receivedObjectLink));
        ASSERT_EQ(data.expectedResult, AwaServerReadResponse_GetValueAsObjectLinkPointer(readResponse, data.path, (const AwaObjectLink **)&value));
        break;
    default:
        ASSERT_TRUE(false);
        break;
    }

    if (data.expectedResult == AwaError_Success)
    {
        ASSERT_TRUE(NULL != value);
        switch(data.type)
        {
            case AwaResourceType_String:
            {
                ASSERT_STREQ((char*) data.expectedValue, (char*) value);
                break;
            }
            case AwaResourceType_Opaque:
            {
                AwaOpaque * expectedOpaque = (AwaOpaque *) data.expectedValue;
                AwaOpaque * receivedOpaquePointer = (AwaOpaque *) value;

                ASSERT_EQ(expectedOpaque->Size, receivedOpaquePointer->Size);
                ASSERT_EQ(expectedOpaque->Size, receivedOpaque.Size);
                ASSERT_EQ(0, memcmp(expectedOpaque->Data, receivedOpaquePointer->Data, receivedOpaquePointer->Size));
                ASSERT_EQ(0, memcmp(expectedOpaque->Data, receivedOpaque.Data, expectedOpaque->Size));
                break;
            }
            case AwaResourceType_Integer:
                ASSERT_EQ(*static_cast<const AwaInteger *>(data.expectedValue), *static_cast<AwaInteger *>(value));
                break;
            case AwaResourceType_Float:
                ASSERT_EQ(*static_cast<const AwaFloat *>(data.expectedValue), *static_cast<AwaFloat *>(value));
                break;
            case AwaResourceType_Boolean:
                ASSERT_EQ(*static_cast<const AwaBoolean *>(data.expectedValue), *static_cast<AwaBoolean *>(value));
                break;
            case AwaResourceType_Time:
                ASSERT_EQ(*static_cast<const AwaTime *>(data.expectedValue), *static_cast<AwaTime *>(value));
                break;
            case AwaResourceType_ObjectLink:
            {
                const AwaObjectLink * expectedObjectLink = static_cast<const AwaObjectLink *>(data.expectedValue);
                const AwaObjectLink * receivedObjectLinkPointer = static_cast<const AwaObjectLink *>(value);
                ASSERT_EQ(0, memcmp(expectedObjectLink, receivedObjectLinkPointer, sizeof(AwaObjectLink)));
                ASSERT_EQ(0, memcmp(expectedObjectLink, &receivedObjectLink, sizeof(AwaObjectLink)));
                break;
            }
            default:
                ASSERT_TRUE(false);
                break;
        }
    }
    else
    {
        ASSERT_EQ(NULL, value);
    }
}

INSTANTIATE_TEST_CASE_P(
        TestReadValueSuccessInstantiation,
        TestReadValue,
        ::testing::Values(
          readDetail::TestReadResource {AwaError_Success,            (void *)readDetail::expectedString1, -1,                              "/10000/0/1",   true,   AwaResourceType_String},
          readDetail::TestReadResource {AwaError_Success,            &readDetail::expectedInteger1,       sizeof(AwaInteger),          "/10000/0/2",   true,   AwaResourceType_Integer},
          readDetail::TestReadResource {AwaError_Success,            &readDetail::expectedFloat1,         sizeof(AwaFloat),            "/10000/0/3",   true,   AwaResourceType_Float},
          readDetail::TestReadResource {AwaError_Success,            &readDetail::expectedBoolean1,       sizeof(AwaBoolean),          "/10000/0/4",   true,   AwaResourceType_Boolean},
          readDetail::TestReadResource {AwaError_Success,            &readDetail::expectedOpaque1,        sizeof(readDetail::expectedOpaque1), "/10000/0/5",   true,   AwaResourceType_Opaque},
          readDetail::TestReadResource {AwaError_Success,            &readDetail::expectedTime1,          sizeof(AwaTime),             "/10000/0/6",   true,   AwaResourceType_Time},
          readDetail::TestReadResource {AwaError_Success,            &readDetail::expectedObjectLink1,    sizeof(AwaObjectLink),       "/10000/0/7",   true,   AwaResourceType_ObjectLink}
        ));


//FIXME: add tests for NULL, invalid, not defined, non-existing instance etc
INSTANTIATE_TEST_CASE_P(
        TestReadValueInvalidPathInstantiation,
        TestReadValue,
        ::testing::Values(
          readDetail::TestReadResource {AwaError_PathInvalid,      (void *)readDetail::expectedString1, -1, NULL,          true,  AwaResourceType_String},  // handles null path
          readDetail::TestReadResource {AwaError_PathNotFound,     (void *)readDetail::expectedString1, -1, "/10001/0/1",  true,  AwaResourceType_String},  // handles path not in response
          readDetail::TestReadResource {AwaError_OperationInvalid, (void *)readDetail::expectedString1, -1, "/10000/0/1",  false, AwaResourceType_String},   // handles invalid response

          readDetail::TestReadResource {AwaError_PathInvalid,      &readDetail::expectedInteger1, sizeof(AwaInteger), NULL,          true,  AwaResourceType_Integer},  // handles null path
          readDetail::TestReadResource {AwaError_PathNotFound,     &readDetail::expectedInteger1, sizeof(AwaInteger), "/10001/0/2",  true,  AwaResourceType_Integer},  // handles path not in response
          readDetail::TestReadResource {AwaError_OperationInvalid, &readDetail::expectedInteger1, sizeof(AwaInteger), "/10000/0/2",  false, AwaResourceType_Integer},  // handles invalid response

          readDetail::TestReadResource {AwaError_PathInvalid,      &readDetail::expectedFloat1, sizeof(AwaFloat), NULL,          true,  AwaResourceType_Float},  // handles null path
          readDetail::TestReadResource {AwaError_PathNotFound,     &readDetail::expectedFloat1, sizeof(AwaFloat), "/10001/0/3",  true,  AwaResourceType_Float},  // handles path not in response
          readDetail::TestReadResource {AwaError_OperationInvalid, &readDetail::expectedFloat1, sizeof(AwaFloat), "/10000/0/3",  false, AwaResourceType_Float},  // handles invalid response

          readDetail::TestReadResource {AwaError_PathInvalid,      &readDetail::expectedFloat1, sizeof(AwaBoolean), NULL,          true,  AwaResourceType_Boolean},  // handles null path
          readDetail::TestReadResource {AwaError_PathNotFound,     &readDetail::expectedFloat1, sizeof(AwaBoolean), "/10001/0/4",  true,  AwaResourceType_Boolean},  // handles path not in response
          readDetail::TestReadResource {AwaError_OperationInvalid, &readDetail::expectedFloat1, sizeof(AwaBoolean), "/10000/0/4",  false, AwaResourceType_Boolean},  // handles invalid response

          readDetail::TestReadResource {AwaError_PathInvalid,      &readDetail::expectedOpaque1, sizeof(readDetail::expectedOpaque1), NULL,          true,  AwaResourceType_Opaque},  // handles null path
          readDetail::TestReadResource {AwaError_PathNotFound,     &readDetail::expectedOpaque1, sizeof(readDetail::expectedOpaque1), "/10001/0/5",  true,  AwaResourceType_Opaque},  // handles path not in response
          readDetail::TestReadResource {AwaError_OperationInvalid, &readDetail::expectedOpaque1, sizeof(readDetail::expectedOpaque1), "/10000/0/5",  false, AwaResourceType_Opaque},  // handles invalid response

          readDetail::TestReadResource {AwaError_PathInvalid,      &readDetail::expectedTime1, sizeof(AwaTime), NULL,          true,  AwaResourceType_Time},  // handles null path
          readDetail::TestReadResource {AwaError_PathNotFound,     &readDetail::expectedTime1, sizeof(AwaTime), "/10001/0/6",  true,  AwaResourceType_Time},  // handles path not in response
          readDetail::TestReadResource {AwaError_OperationInvalid, &readDetail::expectedTime1, sizeof(AwaTime), "/10000/0/6",  false, AwaResourceType_Time},  // handles invalid response

          readDetail::TestReadResource {AwaError_PathInvalid,      &readDetail::expectedObjectLink1, sizeof(AwaObjectLink), NULL,          true,  AwaResourceType_ObjectLink},  // handles null path
          readDetail::TestReadResource {AwaError_PathNotFound,     &readDetail::expectedObjectLink1, sizeof(AwaObjectLink), "/10001/0/7",  true,  AwaResourceType_ObjectLink},  // handles path not in response
          readDetail::TestReadResource {AwaError_OperationInvalid, &readDetail::expectedObjectLink1, sizeof(AwaObjectLink), "/10000/0/7",  false, AwaResourceType_ObjectLink}   // handles invalid response

        ));

INSTANTIATE_TEST_CASE_P(
        TestReadValueTypeMismatchInstantiation,
        TestReadValue,
        ::testing::ValuesIn(readDetail::BuildReadTypeMismatchContainers()));

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_GetValueAsIntegerPointer_handles_null_value)
{
    const AwaInteger * value = NULL;
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerReadResponse_GetValueAsIntegerPointer(NULL, "/3/0/9", &value));
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_GetValueAsIntegerPointer_sets_pointer_to_null_on_error)
{
    const AwaInteger * value = (const AwaInteger *)1;
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerReadResponse_GetValueAsIntegerPointer(NULL, "/3/0/9", &value));
    EXPECT_EQ(NULL, value);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_HasValue_handles_null_response)
{
    ASSERT_FALSE(AwaServerReadResponse_HasValue(NULL, "/3/0/1"));
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_HasValue_handles_null_path)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));
    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != readResponse);
    ASSERT_FALSE(AwaServerReadResponse_HasValue(readResponse, NULL));
    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_HasValue_handles_invalid_path)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));
    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != readResponse);
    ASSERT_FALSE(AwaServerReadResponse_HasValue(readResponse, "/3/0/9"));
    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_HasValue_handles_non_resource_path)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));
    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != readResponse);
    ASSERT_FALSE(AwaServerReadResponse_HasValue(readResponse, "/3/0"));
    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_HasValue_handles_valid_path)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));
    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != readResponse);
    ASSERT_TRUE(AwaServerReadResponse_HasValue(readResponse, "/3/0/1"));
    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_ContainsPath_handles_null_response)
{
    ASSERT_FALSE(AwaServerReadResponse_ContainsPath(NULL, "/3/0/1"));
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_ContainsPath_handles_null_path)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));
    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != readResponse);
    ASSERT_FALSE(AwaServerReadResponse_ContainsPath(readResponse, NULL));
    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_ContainsPath_handles_valid_resource_path)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));
    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != readResponse);
    ASSERT_TRUE(AwaServerReadResponse_ContainsPath(readResponse, "/3/0/1"));
    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestReadOperationWithConnectedSession, DISABLED_AwaServerReadResponse_ContainsPath_handles_valid_resource_path_from_entire_object)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));
    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != readResponse);
    ASSERT_TRUE(AwaServerReadResponse_ContainsPath(readResponse, "/3/0/1"));
    AwaServerReadOperation_Free(&readOperation);
}


TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_ContainsPath_handles_valid_object_instance_path)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));
    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != readResponse);
    ASSERT_TRUE(AwaServerReadResponse_ContainsPath(readResponse, "/3/0"));
    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_ContainsPath_handles_valid_object_path)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));
    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != readResponse);
    ASSERT_TRUE(AwaServerReadResponse_ContainsPath(readResponse, "/3"));
    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_ContainsPath_handles_invalid_resource_path)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));
    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != readResponse);
    ASSERT_FALSE(AwaServerReadResponse_ContainsPath(readResponse, "/3/0/2"));
    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestReadOperation, AwaServerReadResponse_NewPathIterator_handles_null_response)
{
    EXPECT_EQ(NULL, AwaServerReadResponse_NewPathIterator(NULL));
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_NewPathIterator_handles_valid_response)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/9"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));
    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != readResponse);

    AwaPathIterator * iterator = AwaServerReadResponse_NewPathIterator(readResponse);
    ASSERT_TRUE(NULL != iterator);

    // call Next once before readting values:
    EXPECT_EQ(true, AwaPathIterator_Next(iterator));
    EXPECT_STREQ("/3/0/9", AwaPathIterator_Get(iterator));

    // not expecting any more paths
    EXPECT_TRUE(false == AwaPathIterator_Next(iterator));
    EXPECT_TRUE(NULL == AwaPathIterator_Get(iterator));

    AwaPathIterator_Free(&iterator);
    EXPECT_EQ(NULL, iterator);
    AwaServerReadOperation_Free(&readOperation);
}

// TODO: support retrieving to multiple resources in a single request
TEST_F(TestReadOperationWithConnectedSession, DISABLED_AwaServerReadResponse_NewPathIterator_handles_valid_response_multiple_paths)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_); ASSERT_TRUE(NULL != readOperation);
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/0"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/9"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));
    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName); ASSERT_TRUE(NULL != readResponse);

    AwaPathIterator * iterator = AwaServerReadResponse_NewPathIterator(readResponse);
    ASSERT_TRUE(NULL != iterator);

    // call Next once before reading values:
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
    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_NewPathIterator_handles_valid_response_object_instance)
{
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_); ASSERT_TRUE(NULL != readOperation);
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));
    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName); ASSERT_TRUE(NULL != readResponse);


    EXPECT_TRUE(AwaServerReadResponse_ContainsPath(readResponse, "/3/0/0"));
    EXPECT_TRUE(AwaServerReadResponse_ContainsPath(readResponse, "/3/0/1"));
    EXPECT_TRUE(AwaServerReadResponse_ContainsPath(readResponse, "/3/0/2"));
    // ... expecting the rest of /3/0

    AwaServerReadOperation_Free(&readOperation);
}


/***********************************************************************************************************
 * ReadValueArray parameterised tests
 */

namespace readDetail
{

struct TestReadResourceArray
{
    AwaError expectedResult;
    const char * path;
    bool UseResponse;
    AwaResourceType type;
};

::std::ostream& operator<<(::std::ostream& os, const TestReadResourceArray& item)
{
  return os << "Item: expectedResult " << item.expectedResult
            << ", path " << item.path
            << ", UseResponse " << item.UseResponse
            << ", type " << item.type;
}

/*
const AwaInteger expectedInteger1 = 100;
const char * expectedString1 = "Lightweight M2M Server";
AwaFloat expectedFloat1 = 1.337;
AwaTime expectedTime1 = 0xA20AD72B;
AwaBoolean expectedBoolean1 = true;

static char dummyOpaqueData[] = {'a',0,'x','\0', 123};
AwaOpaque expectedOpaque1 = {(void*) dummyOpaqueData, sizeof(dummyOpaqueData), sizeof(dummyOpaqueData)};
AwaObjectLink expectedObjectLink1 = { 3, 0 };*/

}

class TestReadValueArray : public TestReadOperationWithConnectedSession, public ::testing::WithParamInterface< readDetail::TestReadResourceArray>
{
protected:

    TestReadValueArray() :
        readOperation_(nullptr),
        readResponse_(nullptr),
        expectedStringArray_(nullptr),
        expectedIntegerArray_(nullptr),
        expectedFloatArray_(nullptr),
        expectedBooleanArray_(nullptr),
        expectedOpaqueArray_(nullptr),
        expectedTimeArray_(nullptr),
        expectedObjectLinkArray_(nullptr) {}

    void SetUp() {
        TestReadOperationWithConnectedSession::SetUp();

        //Define and set our custom objects - TODO define in some common class for read, set etc?
        AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(server_session_);
        AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(client_session_);
        EXPECT_TRUE(serverDefineOperation != NULL);
        EXPECT_TRUE(clientDefineOperation != NULL);

        AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(10000, "Test Object", 0, 1);
        ASSERT_TRUE(NULL != customObjectDefinition);

        expectedStringArray_ = AwaStringArray_New();
        expectedIntegerArray_ = AwaIntegerArray_New();
        expectedFloatArray_ = AwaFloatArray_New();
        expectedBooleanArray_ = AwaBooleanArray_New();
        expectedOpaqueArray_ = AwaOpaqueArray_New();
        expectedTimeArray_ = AwaTimeArray_New();
        expectedObjectLinkArray_ = AwaObjectLinkArray_New();

        for (int i = 0; i < 10; i++)
        {
            char * stringValue;
            msprintf(&stringValue, "%d", i*2);
            AwaStringArray_SetValueAsCString(expectedStringArray_, i, (const char *)stringValue);
            Awa_MemSafeFree(stringValue);

            AwaIntegerArray_SetValue(expectedIntegerArray_, i, i*2);
            AwaFloatArray_SetValue(expectedFloatArray_, i, i*2.5);
            AwaBooleanArray_SetValue(expectedBooleanArray_, i, i%2==0);

            static char dummyOpaqueData[] = {'a',0,'x','\0', (char)i};

            AwaOpaque opaqueValue = {(void*) dummyOpaqueData, sizeof(dummyOpaqueData)};

            AwaOpaqueArray_SetValue(expectedOpaqueArray_, i, opaqueValue);
            AwaTimeArray_SetValue(expectedTimeArray_, i, i*100);

            AwaObjectLink objectLinkValue = { 3, i };
            AwaObjectLinkArray_SetValue(expectedObjectLinkArray_, i, objectLinkValue);
        }

        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsStringArray(customObjectDefinition, 1, "Test String Array Resource", 0, AWA_MAX_ID, AwaResourceOperations_ReadWrite, expectedStringArray_));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsIntegerArray(customObjectDefinition, 2, "Test Integer Array Resource", 0, AWA_MAX_ID, AwaResourceOperations_ReadWrite, expectedIntegerArray_));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloatArray(customObjectDefinition, 3, "Test Float Array Resource", 0, AWA_MAX_ID, AwaResourceOperations_ReadWrite, expectedFloatArray_));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBooleanArray(customObjectDefinition, 4, "Test Boolean Array Resource", 0, AWA_MAX_ID, AwaResourceOperations_ReadWrite, expectedBooleanArray_));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaqueArray(customObjectDefinition, 5, "Test Opaque Array Resource", 0, AWA_MAX_ID, AwaResourceOperations_ReadWrite, expectedOpaqueArray_));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTimeArray(customObjectDefinition, 6, "Test Time Array Resource", 0, AWA_MAX_ID, AwaResourceOperations_ReadWrite, expectedTimeArray_));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLinkArray(customObjectDefinition, 7, "Test ObjectLink Array Resource", 0, AWA_MAX_ID, AwaResourceOperations_ReadWrite, expectedObjectLinkArray_));


        EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition));
        ASSERT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, global::timeout));
        ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, global::timeout));

        AwaObjectDefinition_Free(&customObjectDefinition);
        AwaServerDefineOperation_Free(&serverDefineOperation);
        AwaClientDefineOperation_Free(&clientDefineOperation);

        //Create a basic set operation to create our custom objects - TODO remove setting values once we can set default values
        AwaClientSetOperation * setOperation = AwaClientSetOperation_New(client_session_);
        ASSERT_TRUE(NULL != setOperation);

        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));

        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10000/0/1"));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10000/0/2"));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10000/0/3"));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10000/0/4"));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10000/0/5"));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10000/0/6"));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10000/0/7"));

        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsStringArray(setOperation, "/10000/0/1", expectedStringArray_));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsIntegerArray(setOperation, "/10000/0/2", expectedIntegerArray_));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsFloatArray(setOperation, "/10000/0/3", expectedFloatArray_));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsBooleanArray(setOperation, "/10000/0/4", expectedBooleanArray_));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsOpaqueArray(setOperation, "/10000/0/5", expectedOpaqueArray_));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsTimeArray(setOperation, "/10000/0/6", expectedTimeArray_));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsObjectLinkArray(setOperation, "/10000/0/7", expectedObjectLinkArray_));

        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, global::timeout));
        AwaClientSetOperation_Free(&setOperation);

        readOperation_ = AwaServerReadOperation_New(server_session_);
        ASSERT_TRUE(NULL != readOperation_);

        // list of paths to objects we require
        ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation_, global::clientEndpointName, "/10000"));

        ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation_, global::timeout));
        readResponse_ = AwaServerReadOperation_GetResponse(readOperation_, global::clientEndpointName);
        ASSERT_TRUE(NULL != readResponse_);
    }

    void TearDown()
    {
        TestReadOperationWithConnectedSession::TearDown();
        AwaServerReadOperation_Free(&readOperation_);
        AwaStringArray_Free(&expectedStringArray_);
        AwaIntegerArray_Free(&expectedIntegerArray_);
        AwaFloatArray_Free(&expectedFloatArray_);
        AwaBooleanArray_Free(&expectedBooleanArray_);
        AwaOpaqueArray_Free(&expectedOpaqueArray_);
        AwaTimeArray_Free(&expectedTimeArray_);
        AwaObjectLinkArray_Free(&expectedObjectLinkArray_);
    }

    AwaServerReadOperation * readOperation_;
    const AwaServerReadResponse * readResponse_;
    AwaStringArray * expectedStringArray_;
    AwaIntegerArray * expectedIntegerArray_;
    AwaFloatArray * expectedFloatArray_;
    AwaBooleanArray * expectedBooleanArray_;
    AwaOpaqueArray * expectedOpaqueArray_;
    AwaTimeArray * expectedTimeArray_;
    AwaObjectLinkArray * expectedObjectLinkArray_;
};

TEST_P(TestReadValueArray, TestReadValueArrayInstantiation)
{
    readDetail::TestReadResourceArray data = GetParam();
    const AwaServerReadResponse * readResponse = data.UseResponse ? this->readResponse_ : NULL;
    AwaArray * array = NULL;

    AwaArray * expectedArray = NULL;

    switch(data.type)
    {
    case AwaResourceType_StringArray:
    {
        expectedArray = (AwaArray *)this->expectedStringArray_;
        ASSERT_EQ(data.expectedResult, AwaServerReadResponse_GetValuesAsStringArrayPointer(readResponse, data.path, (const AwaStringArray **)&array));
        break;
    }
    case AwaResourceType_IntegerArray:
    {
        expectedArray = (AwaArray *)this->expectedIntegerArray_;
        ASSERT_EQ(data.expectedResult, AwaServerReadResponse_GetValuesAsIntegerArrayPointer(readResponse, data.path, (const AwaIntegerArray **)&array));
        break;
    }
    case AwaResourceType_FloatArray:
    {
        expectedArray = (AwaArray *)this->expectedFloatArray_;
        ASSERT_EQ(data.expectedResult, AwaServerReadResponse_GetValuesAsFloatArrayPointer(readResponse, data.path, (const AwaFloatArray **)&array));
        break;
    }
    case AwaResourceType_BooleanArray:
    {
        expectedArray = (AwaArray *)this->expectedBooleanArray_;
        ASSERT_EQ(data.expectedResult, AwaServerReadResponse_GetValuesAsBooleanArrayPointer(readResponse, data.path, (const AwaBooleanArray **)&array));
        break;
    }
    case AwaResourceType_OpaqueArray:
    {
        expectedArray = (AwaArray *)this->expectedOpaqueArray_;
        ASSERT_EQ(data.expectedResult, AwaServerReadResponse_GetValuesAsOpaqueArrayPointer(readResponse, data.path, (const AwaOpaqueArray **)&array));
        break;
    }
    case AwaResourceType_TimeArray:
    {
        expectedArray = (AwaArray *)this->expectedTimeArray_;
        ASSERT_EQ(data.expectedResult, AwaServerReadResponse_GetValuesAsTimeArrayPointer(readResponse, data.path, (const AwaTimeArray **)&array));
        break;
    }
    case AwaResourceType_ObjectLinkArray:
    {
        expectedArray = (AwaArray *)this->expectedObjectLinkArray_;
        ASSERT_EQ(data.expectedResult, AwaServerReadResponse_GetValuesAsObjectLinkArrayPointer(readResponse, data.path, (const AwaObjectLinkArray **)&array));
        break;
    }
    default:
        ASSERT_TRUE(false);
        break;
    }
    ASSERT_TRUE((data.expectedResult == AwaError_Success) == (array != NULL));
    if (array != NULL)
    {
        ASSERT_EQ(0, Array_Compare(expectedArray, array, data.type));
    }
}

INSTANTIATE_TEST_CASE_P(
        TestReadValueArraySuccessInstantiation,
        TestReadValueArray,
        ::testing::Values(
          readDetail::TestReadResourceArray {AwaError_Success,           "/10000/0/1",   true,   AwaResourceType_StringArray},
          readDetail::TestReadResourceArray {AwaError_Success,           "/10000/0/2",   true,   AwaResourceType_IntegerArray},
          readDetail::TestReadResourceArray {AwaError_Success,           "/10000/0/3",   true,   AwaResourceType_FloatArray},
          readDetail::TestReadResourceArray {AwaError_Success,           "/10000/0/4",   true,   AwaResourceType_BooleanArray},
          readDetail::TestReadResourceArray {AwaError_Success,           "/10000/0/5",   true,   AwaResourceType_OpaqueArray},
          readDetail::TestReadResourceArray {AwaError_Success,           "/10000/0/6",   true,   AwaResourceType_TimeArray},
          readDetail::TestReadResourceArray {AwaError_Success,           "/10000/0/7",   true,   AwaResourceType_ObjectLinkArray}
        ));

INSTANTIATE_TEST_CASE_P(
        TestReadValueArrayInvalidResponseInstantiation,
        TestReadValueArray,
        ::testing::Values(
          readDetail::TestReadResourceArray {AwaError_OperationInvalid,           "/10000/0/1",   false,   AwaResourceType_StringArray},
          readDetail::TestReadResourceArray {AwaError_OperationInvalid,           "/10000/0/2",   false,   AwaResourceType_IntegerArray},
          readDetail::TestReadResourceArray {AwaError_OperationInvalid,           "/10000/0/3",   false,   AwaResourceType_FloatArray},
          readDetail::TestReadResourceArray {AwaError_OperationInvalid,           "/10000/0/4",   false,   AwaResourceType_BooleanArray},
          readDetail::TestReadResourceArray {AwaError_OperationInvalid,           "/10000/0/5",   false,   AwaResourceType_OpaqueArray},
          readDetail::TestReadResourceArray {AwaError_OperationInvalid,           "/10000/0/6",   false,   AwaResourceType_TimeArray},
          readDetail::TestReadResourceArray {AwaError_OperationInvalid,            "/10000/0/7",   false,   AwaResourceType_ObjectLinkArray}
        ));

INSTANTIATE_TEST_CASE_P(
        TestReadValueArrayInvalidPathInstantiation,
        TestReadValueArray,
        ::testing::Values(
          readDetail::TestReadResourceArray {AwaError_PathNotFound,           "/10000/1/1",   true,   AwaResourceType_StringArray},
          readDetail::TestReadResourceArray {AwaError_PathNotFound,           "/10000/1/2",   true,   AwaResourceType_IntegerArray},
          readDetail::TestReadResourceArray {AwaError_PathNotFound,           "/10000/1/3",   true,   AwaResourceType_FloatArray},
          readDetail::TestReadResourceArray {AwaError_PathNotFound,           "/10000/1/4",   true,   AwaResourceType_BooleanArray},
          readDetail::TestReadResourceArray {AwaError_PathNotFound,           "/10000/1/5",   true,   AwaResourceType_OpaqueArray},
          readDetail::TestReadResourceArray {AwaError_PathNotFound,           "/10000/1/6",   true,   AwaResourceType_TimeArray},
          readDetail::TestReadResourceArray {AwaError_PathNotFound,            "/10000/1/7",   true,   AwaResourceType_ObjectLinkArray}
        ));

////TODO AwaError_TypeMismatch
////TODO AwaError_NotDefined
//
////template <typename T>
////class AwaServerReadResponse_ReadValuesAsArrayPointer
////{
////
////};
////
////TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_ReadValuesAsStringArrayPointer_handles_null_response)
////{
////
////}
////
////TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_ReadValuesAsStringArrayPointer_handles_null_path)
////{
////
////}
////
////TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_ReadValuesAsStringArrayPointer_handles_null_array)
////{
////
////}
////
////TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_ReadValuesAsStringArrayPointer_handles_invalid_type)
////{
////
////}
//
//
//TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_ReadValuesAsIntegerArrayPointer_handles_null_response)
//{
//    const AwaIntegerArray * array = NULL;
//    ASSERT_EQ(AwaError_OperationInvalid, AwaServerReadResponse_ReadValuesAsIntegerArrayPointer(NULL, "/3/0/6", &array));
//    ASSERT_TRUE(NULL == array);
//}
//
//TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_ReadValuesAsIntegerArrayPointer_handles_null_path)
//{
//    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
//    ASSERT_TRUE(NULL != readOperation);
//    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/6"));
//    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));
//    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation);
//    ASSERT_TRUE(NULL != readResponse);
//
//    const AwaIntegerArray * array = NULL;
//    ASSERT_EQ(AwaError_PathInvalid, AwaServerReadResponse_ReadValuesAsIntegerArrayPointer(readResponse, NULL, &array));
//
//    AwaServerReadOperation_Free(&readOperation);
//}
//
//TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_ReadValuesAsIntegerArrayPointer_handles_null_array)
//{
//    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
//    ASSERT_TRUE(NULL != readOperation);
//    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/6"));
//    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));
//    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation);
//    ASSERT_TRUE(NULL != readResponse);
//
//    ASSERT_EQ(AwaError_OperationInvalid, AwaServerReadResponse_ReadValuesAsIntegerArrayPointer(readResponse, "/3/0/6", NULL));
//
//    AwaServerReadOperation_Free(&readOperation);
//}
//
//TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_ReadValuesAsIntegerArrayPointer_handles_invalid_type)
//{
//    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
//    ASSERT_TRUE(NULL != readOperation);
//    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/1"));
//    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));
//    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation);
//    ASSERT_TRUE(NULL != readResponse);
//
//    const AwaIntegerArray * array = NULL;
//    ASSERT_EQ(AwaError_TypeMismatch, AwaServerReadResponse_ReadValuesAsIntegerArrayPointer(readResponse, "/3/0/1", &array));
//
//    AwaServerReadOperation_Free(&readOperation);
//}
//
//TEST_F(TestReadOperationWithConnectedSession, AwaServerReadResponse_ReadValuesAsIntegerArrayPointer_handles_valid_operation)
//{
//    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
//    ASSERT_TRUE(NULL != readOperation);
//    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/3/0/6"));
//    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));
//    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation);
//    ASSERT_TRUE(NULL != readResponse);
//
//    const AwaIntegerArray * array = NULL;
//    ASSERT_EQ(AwaError_Success, AwaServerReadResponse_ReadValuesAsIntegerArrayPointer(readResponse, "/3/0/6", &array));
//    ASSERT_TRUE(NULL != array);
//
//    AwaIntegerArrayIterator * iterator = AwaIntegerArray_NewIntegerArrayIterator(array);
//    ASSERT_TRUE(AwaIntegerArrayIterator_Next(iterator));
//    ASSERT_EQ(1, AwaIntegerArrayIterator_ReadValue(iterator));
//    ASSERT_TRUE(AwaIntegerArrayIterator_Next(iterator));
//    ASSERT_EQ(5, AwaIntegerArrayIterator_ReadValue(iterator));
//    ASSERT_FALSE(AwaIntegerArrayIterator_Next(iterator));
//    AwaIntegerArrayIterator_Free(&iterator);
//
//    AwaServerReadOperation_Free(&readOperation);
//}

class TestReadOperationWithConnectedSessionNoClient : public TestServerWithConnectedSession {};

TEST_F(TestReadOperationWithConnectedSessionNoClient, AwaServerReadOperation_NewClientIterator_handles_null)
{
    EXPECT_EQ(NULL, AwaServerReadOperation_NewClientIterator(NULL));
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, AwaServerReadOperation_NewClientIterator_with_no_perform)
{
    AwaServerReadOperation * operation = AwaServerReadOperation_New(session_);
    // no perform
    AwaClientIterator * iterator = AwaServerReadOperation_NewClientIterator(operation);
    EXPECT_EQ(NULL, iterator);
    AwaServerReadOperation_Free(&operation);
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, AwaServerReadOperation_NewClientIterator_handles_no_clients)
{
    AwaServerReadOperation * operation = AwaServerReadOperation_New(session_);
    const char * clientID = "TestClient123";
    const char * path = "/3/0/9";
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, clientID, path));
    EXPECT_EQ(AwaError_Response, AwaServerReadOperation_Perform(operation, global::timeout));

    // expect the client ID to be in the response, but with an error on the specified path
    AwaClientIterator * iterator = AwaServerReadOperation_NewClientIterator(operation);
    EXPECT_TRUE(NULL != iterator);
    EXPECT_TRUE(AwaClientIterator_Next(iterator));
    EXPECT_STREQ(clientID, AwaClientIterator_GetClientID(iterator));
    EXPECT_FALSE(AwaClientIterator_Next(iterator));   // only one client

    // should be an error:
    const AwaServerReadResponse * response = AwaServerReadOperation_GetResponse(operation, clientID);
    ASSERT_TRUE(NULL != response);
    const AwaPathResult * pathResult = AwaServerReadResponse_GetPathResult(response, path);
    EXPECT_TRUE(NULL != pathResult);
    EXPECT_EQ(AwaError_ClientNotFound, AwaPathResult_GetError(pathResult));

    AwaClientIterator_Free(&iterator);
    AwaServerReadOperation_Free(&operation);
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, AwaServerReadOperation_handles_one_client)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaServerReadOperation * operation = AwaServerReadOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/1/0/0"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Perform(operation, global::timeout));

    AwaClientIterator * iterator = AwaServerReadOperation_NewClientIterator(operation);
    EXPECT_TRUE(NULL != iterator);
    EXPECT_EQ(NULL, AwaClientIterator_GetClientID(iterator));
    EXPECT_TRUE(AwaClientIterator_Next(iterator));
    EXPECT_STREQ("TestClient1", AwaClientIterator_GetClientID(iterator));
    EXPECT_FALSE(AwaClientIterator_Next(iterator));   // only one client

    AwaClientIterator_Free(&iterator);
    AwaServerReadOperation_Free(&operation);
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, DISABLED_AwaServerReadOperation_handles_multiple_clients)
{
    // DISABLED because we don't support multiple ClientIDs in a single Read operation at present.

    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1", "TestClient2", "TestClient3" }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaServerReadOperation * operation = AwaServerReadOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/1/0/0"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient2", "/1/0/0"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient3", "/1/0/0"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Perform(operation, global::timeout));

    AwaClientIterator * iterator = AwaServerReadOperation_NewClientIterator(operation);
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
    AwaServerReadOperation_Free(&operation);
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, AwaServerReadOperation_GetResponse_handles_null_operation)
{
    EXPECT_EQ(NULL, AwaServerReadOperation_GetResponse(NULL, "TestClient1"));
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, AwaServerReadOperation_GetResponse_handles_null_clientID)
{
    AwaServerReadOperation * operation = AwaServerReadOperation_New(session_);
    EXPECT_TRUE(NULL != operation);
    EXPECT_EQ(NULL, AwaServerReadOperation_GetResponse(operation, NULL));
    AwaServerReadOperation_Free(&operation);
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, AwaServerReadOperation_GetResponse_handles_missing_clientID)
{
    AwaServerReadOperation * operation = AwaServerReadOperation_New(session_);
    EXPECT_TRUE(NULL != operation);
    EXPECT_EQ(NULL, AwaServerReadOperation_GetResponse(operation, "ClientDoesNotExist"));
    AwaServerReadOperation_Free(&operation);
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, AwaServerReadOperation_GetResponse_handles_matching_clientID)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaServerReadOperation * operation = AwaServerReadOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/1/0/0"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Perform(operation, global::timeout));

    EXPECT_TRUE(NULL != operation);
    EXPECT_TRUE(NULL != AwaServerReadOperation_GetResponse(operation, "TestClient1"));
    AwaServerReadOperation_Free(&operation);
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, AwaServerReadResponse_NewPathIterator_handles_null_response)
{
    EXPECT_EQ(NULL, AwaServerReadResponse_NewPathIterator(NULL));
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, AwaServerReadResponse_NewPathIterator_handles_single_path_response)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 5000));

    AwaServerReadOperation * operation = AwaServerReadOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/1/0/0"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Perform(operation, global::timeout));
    const AwaServerReadResponse * response = AwaServerReadOperation_GetResponse(operation, "TestClient1");
    ASSERT_TRUE(NULL != response);

    AwaPathIterator * iterator = AwaServerReadResponse_NewPathIterator(response);
    ASSERT_TRUE(NULL != iterator);

    // should be one path
    EXPECT_FALSE(AwaPathIterator_Get(iterator));
    EXPECT_TRUE(AwaPathIterator_Next(iterator));
    EXPECT_STREQ("/1/0/0", AwaPathIterator_Get(iterator));
    EXPECT_FALSE(AwaPathIterator_Next(iterator));
    AwaPathIterator_Free(&iterator);

    AwaServerReadOperation_Free(&operation);
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, DISABLED_AwaServerReadResponse_NewPathIterator_handles_multiple_path_response)
{
    // NOTE: DISABLED because we don't support multiple paths per operation, presently.

    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaServerReadOperation * operation = AwaServerReadOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/1/0/0"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/1/0/1"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/1/0/2"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Perform(operation, global::timeout));
    const AwaServerReadResponse * response = AwaServerReadOperation_GetResponse(operation, "TestClient1");
    ASSERT_TRUE(NULL != response);

    AwaPathIterator * iterator = AwaServerReadResponse_NewPathIterator(response);
    ASSERT_TRUE(NULL != iterator);

    // should be three paths
    std::vector<std::string> expectedPaths = { "/1/0/0", "/1/0/1", "/1/0/2" };
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
    AwaServerReadOperation_Free(&operation);
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, AwaServerReadResponse_GetValueAsIntegerPointer_handles_null_response)
{
    const AwaInteger * value = NULL;
    EXPECT_EQ(AwaError_OperationInvalid, AwaServerReadResponse_GetValueAsIntegerPointer(NULL, "/3/0/0", &value));
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, AwaServerReadResponse_GetValueAsIntegerPointer_handles_null_path)
{
    AwaServerReadOperation * operation = AwaServerReadOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/3/0/9"));
    EXPECT_EQ(AwaError_Response, AwaServerReadOperation_Perform(operation, global::timeout));
    const AwaServerReadResponse * response = AwaServerReadOperation_GetResponse(operation, "TestClient1");
    ASSERT_TRUE(NULL != response);

    const AwaInteger * value = 0;
    EXPECT_EQ(AwaError_PathInvalid, AwaServerReadResponse_GetValueAsIntegerPointer(response, NULL, &value));
    AwaServerReadOperation_Free(&operation);
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, AwaServerReadResponse_GetValueAsIntegerPointer_handles_null_value_pointer)
{
    AwaServerReadOperation * operation = AwaServerReadOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/3/0/9"));
    EXPECT_EQ(AwaError_Response, AwaServerReadOperation_Perform(operation, global::timeout));
    const AwaServerReadResponse * response = AwaServerReadOperation_GetResponse(operation, "TestClient1");
    ASSERT_TRUE(NULL != response);

    EXPECT_EQ(AwaError_OperationInvalid, AwaServerReadResponse_GetValueAsIntegerPointer(response, "/3/0/0", NULL));
    AwaServerReadOperation_Free(&operation);
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, AwaServerReadResponse_GetValueAsIntegerPointer_handles_non_matching_path)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaServerReadOperation * operation = AwaServerReadOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/1/0/0"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Perform(operation, global::timeout));
    const AwaServerReadResponse * response = AwaServerReadOperation_GetResponse(operation, "TestClient1");
    ASSERT_TRUE(NULL != response);

    const AwaInteger * value = 0;
    EXPECT_EQ(AwaError_PathNotFound, AwaServerReadResponse_GetValueAsIntegerPointer(response, "/99/9/9", &value));

    AwaServerReadOperation_Free(&operation);
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, AwaServerReadResponse_GetValueAsIntegerPointer_handles_valid_integer_path)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    // use the Client API to modify a resource, so that the server's
    // attempt to read the resource will encounter an expected value.
    AwaClientSession * clientSession = AwaClientSession_New();
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(clientSession, "127.0.0.1", 61000));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(clientSession));
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(clientSession);
    ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsInteger(setOperation, "/1/0/1", 56));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, global::timeout));
    AwaClientSession_Disconnect(clientSession);
    AwaClientSetOperation_Free(&setOperation);
    AwaClientSession_Free(&clientSession);
    //

    AwaServerReadOperation * operation = AwaServerReadOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/1/0/1"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Perform(operation, global::timeout));
    const AwaServerReadResponse * response = AwaServerReadOperation_GetResponse(operation, "TestClient1");
    ASSERT_TRUE(NULL != response);

    const AwaInteger * value = 0;
    ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsIntegerPointer(response, "/1/0/1", &value));
    EXPECT_EQ(56, *value);

    AwaServerReadOperation_Free(&operation);
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, AwaServerReadResponse_GetValueAsIntegerPointer_handles_valid_string_path)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    // use the Client API to modify a resource, so that the server's
    // attempt to read the resource will encounter an expected value.
    AwaClientSession * clientSession = AwaClientSession_New();
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(clientSession, "127.0.0.1", 61000));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(clientSession));
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(clientSession);
    ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsCString(setOperation, "/1/0/7", "UQ"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, global::timeout));
    AwaClientSession_Disconnect(clientSession);
    AwaClientSetOperation_Free(&setOperation);
    AwaClientSession_Free(&clientSession);
    //

    AwaServerReadOperation * operation = AwaServerReadOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/1/0/7"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Perform(operation, global::timeout));
    const AwaServerReadResponse * response = AwaServerReadOperation_GetResponse(operation, "TestClient1");
    ASSERT_TRUE(NULL != response);

    const char * value = 0;
    ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsCStringPointer(response, "/1/0/7", &value));
    EXPECT_STREQ("UQ", value);

    AwaServerReadOperation_Free(&operation);
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, AwaServerReadResponse_GetValueAsIntegerPointer_handles_valid_boolean_path)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    // use the Client API to create a boolean resource, so that the server's
    // attempt to read the resource will encounter an expected value.
    AwaClientSession * clientSession = AwaClientSession_New();
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(clientSession, "127.0.0.1", 61000));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(clientSession));

    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(1000, "TestObject1000", 1, 1);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBoolean(objectDefinition, 0, "Resource0", true, AwaResourceOperations_ReadOnly, false));

    AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(clientSession);  ASSERT_TRUE(NULL != clientDefineOperation);
    AwaClientDefineOperation_Add(clientDefineOperation, objectDefinition);
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, global::timeout));
    AwaClientDefineOperation_Free(&clientDefineOperation);

    AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(session_);       ASSERT_TRUE(NULL != serverDefineOperation);
    AwaServerDefineOperation_Add(serverDefineOperation, objectDefinition);
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, global::timeout));
    AwaServerDefineOperation_Free(&serverDefineOperation);
    AwaObjectDefinition_Free(&objectDefinition);

    // set to known value
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(clientSession);
    ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsBoolean(setOperation, "/1000/0/0", true));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, global::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaServerReadOperation * operation = AwaServerReadOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Perform(operation, global::timeout));
    const AwaServerReadResponse * response = AwaServerReadOperation_GetResponse(operation, "TestClient1");
    ASSERT_TRUE(NULL != response);

    const AwaBoolean * value = 0;
    ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsBooleanPointer(response, "/1000/0/0", &value));
    EXPECT_EQ(true, *value);
    AwaServerReadOperation_Free(&operation);

    // change and read again
    setOperation = AwaClientSetOperation_New(clientSession);
    ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsBoolean(setOperation, "/1000/0/0", false));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, global::timeout));
    AwaClientSetOperation_Free(&setOperation);

    operation = AwaServerReadOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Perform(operation, global::timeout));
    response = AwaServerReadOperation_GetResponse(operation, "TestClient1");
    ASSERT_TRUE(NULL != response);

    value = 0;
    ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsBooleanPointer(response, "/1000/0/0", &value));
    EXPECT_TRUE(false ==  *value);
    AwaServerReadOperation_Free(&operation);

    AwaClientSession_Disconnect(clientSession);
    AwaClientSession_Free(&clientSession);
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, AwaServerReadResponse_GetValueAsIntegerPointer_handles_valid_time_path)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    // use the Client API to create a boolean resource, so that the server's
    // attempt to read the resource will encounter an expected value.
    AwaClientSession * clientSession = AwaClientSession_New();
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(clientSession, "127.0.0.1", 61000));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(clientSession));

    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(1000, "TestObject1000", 1, 1);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTime(objectDefinition, 0, "Resource0", true, AwaResourceOperations_ReadOnly, 0));

    AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(clientSession);  ASSERT_TRUE(NULL != clientDefineOperation);
    AwaClientDefineOperation_Add(clientDefineOperation, objectDefinition);
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, global::timeout));
    AwaClientDefineOperation_Free(&clientDefineOperation);

    AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(session_);       ASSERT_TRUE(NULL != serverDefineOperation);
    AwaServerDefineOperation_Add(serverDefineOperation, objectDefinition);
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, global::timeout));
    AwaServerDefineOperation_Free(&serverDefineOperation);
    AwaObjectDefinition_Free(&objectDefinition);

    // set to known value
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(clientSession);
    ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsTime(setOperation, "/1000/0/0", 12345678));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, global::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaServerReadOperation * operation = AwaServerReadOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Perform(operation, global::timeout));
    const AwaServerReadResponse * response = AwaServerReadOperation_GetResponse(operation, "TestClient1");
    ASSERT_TRUE(NULL != response);

    const AwaTime * value = 0;
    ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsTimePointer(response, "/1000/0/0", &value));
    EXPECT_EQ(12345678, *value);
    AwaServerReadOperation_Free(&operation);

    // change and try again
    setOperation = AwaClientSetOperation_New(clientSession);
    ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsTime(setOperation, "/1000/0/0", 87654321));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, global::timeout));
    AwaClientSetOperation_Free(&setOperation);

    operation = AwaServerReadOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Perform(operation, global::timeout));
    response = AwaServerReadOperation_GetResponse(operation, "TestClient1");
    ASSERT_TRUE(NULL != response);

    value = 0;
    ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsTimePointer(response, "/1000/0/0", &value));
    EXPECT_EQ(87654321, *value);
    AwaServerReadOperation_Free(&operation);

    AwaClientSession_Disconnect(clientSession);
    AwaClientSession_Free(&clientSession);
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, AwaServerReadResponse_GetValueAsIntegerPointer_handles_valid_float_path)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    // use the Client API to create a boolean resource, so that the server's
    // attempt to read the resource will encounter an expected value.
    AwaClientSession * clientSession = AwaClientSession_New();
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(clientSession, "127.0.0.1", 61000));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(clientSession));

    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(1000, "TestObject1000", 1, 1);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloat(objectDefinition, 0, "Resource0", true, AwaResourceOperations_ReadOnly, 0));

    AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(clientSession);  ASSERT_TRUE(NULL != clientDefineOperation);
    AwaClientDefineOperation_Add(clientDefineOperation, objectDefinition);
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, global::timeout));
    AwaClientDefineOperation_Free(&clientDefineOperation);

    AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(session_);       ASSERT_TRUE(NULL != serverDefineOperation);
    AwaServerDefineOperation_Add(serverDefineOperation, objectDefinition);
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, global::timeout));
    AwaServerDefineOperation_Free(&serverDefineOperation);
    AwaObjectDefinition_Free(&objectDefinition);

    // set to known value
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(clientSession);
    ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsFloat(setOperation, "/1000/0/0", -42.236e17));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, global::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaServerReadOperation * operation = AwaServerReadOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Perform(operation, global::timeout));
    const AwaServerReadResponse * response = AwaServerReadOperation_GetResponse(operation, "TestClient1");
    ASSERT_TRUE(NULL != response);

    const AwaFloat * value = 0;
    ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsFloatPointer(response, "/1000/0/0", &value));
    EXPECT_EQ(-42.236e17, *value);
    AwaServerReadOperation_Free(&operation);

    // change and try again
    setOperation = AwaClientSetOperation_New(clientSession);
    ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsFloat(setOperation, "/1000/0/0", 1001.00002));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, global::timeout));
    AwaClientSetOperation_Free(&setOperation);

    operation = AwaServerReadOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Perform(operation, global::timeout));
    response = AwaServerReadOperation_GetResponse(operation, "TestClient1");
    ASSERT_TRUE(NULL != response);

    value = 0;
    ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsFloatPointer(response, "/1000/0/0", &value));
    EXPECT_EQ(1001.00002, *value);
    AwaServerReadOperation_Free(&operation);

    // change and try again
    setOperation = AwaClientSetOperation_New(clientSession);
    ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsFloat(setOperation, "/1000/0/0", -1.234567e30));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, global::timeout));
    AwaClientSetOperation_Free(&setOperation);

    operation = AwaServerReadOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Perform(operation, global::timeout));
    response = AwaServerReadOperation_GetResponse(operation, "TestClient1");
    ASSERT_TRUE(NULL != response);

    value = 0;
    ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsFloatPointer(response, "/1000/0/0", &value));
    EXPECT_EQ(-1.234567e30, *value);
    AwaServerReadOperation_Free(&operation);

    AwaClientSession_Disconnect(clientSession);
    AwaClientSession_Free(&clientSession);
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, AwaServerReadResponse_GetValueAsIntegerPointer_handles_valid_opaque_path)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    AwaOpaque defaultValue = { NULL, 0 };

    // use the Client API to create a boolean resource, so that the server's
    // attempt to read the resource will encounter an expected value.
    AwaClientSession * clientSession = AwaClientSession_New();
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(clientSession, "127.0.0.1", 61000));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(clientSession));

    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(1000, "TestObject1000", 1, 1);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque(objectDefinition, 0, "Resource0", true, AwaResourceOperations_ReadOnly, defaultValue));

    AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(clientSession);  ASSERT_TRUE(NULL != clientDefineOperation);
    AwaClientDefineOperation_Add(clientDefineOperation, objectDefinition);
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, global::timeout));
    AwaClientDefineOperation_Free(&clientDefineOperation);

    AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(session_);       ASSERT_TRUE(NULL != serverDefineOperation);
    AwaServerDefineOperation_Add(serverDefineOperation, objectDefinition);
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, global::timeout));
    AwaServerDefineOperation_Free(&serverDefineOperation);
    AwaObjectDefinition_Free(&objectDefinition);

    // set to known value
    const int expectedData[] = { 0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89 };
    LogHex(LOG_LEVEL_DEBUG, stdout, expectedData, sizeof(expectedData));
    AwaOpaque opaque = { (void *)expectedData, sizeof(expectedData) };

    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(clientSession);
    ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsOpaque(setOperation, "/1000/0/0", opaque));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, global::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaServerReadOperation * operation = AwaServerReadOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Perform(operation, global::timeout));
    const AwaServerReadResponse * response = AwaServerReadOperation_GetResponse(operation, "TestClient1");
    ASSERT_TRUE(NULL != response);

    AwaOpaque value = { 0 };
    ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsOpaque(response, "/1000/0/0", &value));
    EXPECT_EQ(opaque.Size, value.Size);
    ASSERT_TRUE(NULL != value.Data);
    EXPECT_EQ(0, std::memcmp(expectedData, value.Data, sizeof(expectedData)));
    //LogHex(LOG_LEVEL_DEBUG, stdout, opaque.Data, opaque.Size);
    AwaServerReadOperation_Free(&operation);

    // change and try again
    const int expectedData2[] = { 0, 2, 5, 9, 17, 33, 65, 129, 257, 513, 1025, 2049, 8193, 16537 };
    LogHex(LOG_LEVEL_DEBUG, stdout, expectedData2, sizeof(expectedData2));
    opaque = { (void *)expectedData2, sizeof(expectedData2) };
    setOperation = AwaClientSetOperation_New(clientSession);
    ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsOpaque(setOperation, "/1000/0/0", opaque));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, global::timeout));
    AwaClientSetOperation_Free(&setOperation);

    operation = AwaServerReadOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Perform(operation, global::timeout));
    response = AwaServerReadOperation_GetResponse(operation, "TestClient1");
    ASSERT_TRUE(NULL != response);

    std::memset(&value, 0, sizeof(value));
    ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsOpaque(response, "/1000/0/0", &value));
    EXPECT_EQ(opaque.Size, value.Size);
    EXPECT_EQ(0, std::memcmp(expectedData2, value.Data, sizeof(expectedData2)));
    //LogHex(LOG_LEVEL_DEBUG, stdout, opaque.Data, opaque.Size);
    AwaServerReadOperation_Free(&operation);

    AwaClientSession_Disconnect(clientSession);
    AwaClientSession_Free(&clientSession);
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, DISABLED_AwaServerReadResponse_GetValueAsIntegerPointer_handles_multiple_valid_paths)
{
    // NOTE: DISABLED because we don't support multiple paths per operation, presently.

    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    // use the Client API to modify a resource, so that the server's
    // attempt to read the resource will encounter an expected value.
    AwaClientSession * clientSession = AwaClientSession_New();
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(clientSession, "127.0.0.1", 61000));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(clientSession));
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(clientSession);
    ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsCString(setOperation, "/3/0/0", "Imagination Technologies"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsCString(setOperation, "/3/0/1", "Ci40"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsInteger(setOperation, "/3/0/9", 42));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, global::timeout));
    AwaClientSession_Disconnect(clientSession);
    AwaClientSetOperation_Free(&setOperation);
    AwaClientSession_Free(&clientSession);
    //

    AwaServerReadOperation * operation = AwaServerReadOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/3/0"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Perform(operation, global::timeout));
    const AwaServerReadResponse * response = AwaServerReadOperation_GetResponse(operation, "TestClient1");
    ASSERT_TRUE(NULL != response);

    const char * value0 = NULL;
    const char * value1 = NULL;
    const AwaInteger * value9 = 0;

    ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsCStringPointer(response, "/3/0/0", &value0));
    EXPECT_STREQ("Imagination Technologies", value0);
    ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsCStringPointer(response, "/3/0/1", &value1));
    EXPECT_STREQ("Ci40", value1);
    ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsIntegerPointer(response, "/3/0/9", &value9));
    EXPECT_EQ(56, *value9);

    AwaServerReadOperation_Free(&operation);
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, AwaServerReadOperation_handles_success)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    // now try to read the mandatory resource:
    AwaServerReadOperation * operation = AwaServerReadOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/3/0/11"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Perform(operation, global::timeout));

    // check response on /3/0/11
    const AwaServerReadResponse * response = AwaServerReadOperation_GetResponse(operation, "TestClient1");
    EXPECT_TRUE(NULL != response);

    const AwaPathResult * pathResult = AwaServerReadResponse_GetPathResult(response, "/3/0/11");
    EXPECT_TRUE(NULL != pathResult);
    EXPECT_EQ(AwaError_Success, AwaPathResult_GetError(pathResult));

    AwaServerReadOperation_Free(&operation);
}

TEST_F(TestReadOperationWithConnectedSessionNoClient, AwaServerReadOperation_handles_error)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    ASSERT_TRUE(WaitForRegistration(session_, horde.GetClientIDs(), 1000));

    // use the Client API to delete an optional resource, so that the server's
    // attempt to read the resource will fail.
    AwaClientSession * clientSession = AwaClientSession_New();
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(clientSession, "127.0.0.1", 61000));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(clientSession));
    AwaClientDeleteOperation * deleteOperation = AwaClientDeleteOperation_New(clientSession);
    ASSERT_TRUE(NULL != deleteOperation);
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(deleteOperation, "/1/0/5"));
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_Perform(deleteOperation, global::timeout));
    AwaClientSession_Disconnect(clientSession);
    AwaClientDeleteOperation_Free(&deleteOperation);
    AwaClientSession_Free(&clientSession);
    //

    // now try to read the deleted resource:
    AwaServerReadOperation * operation = AwaServerReadOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(operation, "TestClient1", "/1/0/5"));
    EXPECT_EQ(AwaError_Response, AwaServerReadOperation_Perform(operation, global::timeout));

    // check response on /3/0/0
    const AwaServerReadResponse * response = AwaServerReadOperation_GetResponse(operation, "TestClient1");
    EXPECT_TRUE(NULL != response);

    const AwaPathResult * pathResult = AwaServerReadResponse_GetPathResult(response, "/1/0/5");
    EXPECT_TRUE(NULL != pathResult);
    EXPECT_EQ(AwaError_LWM2MError, AwaPathResult_GetError(pathResult));
// TODO    EXPECT_EQ(AwaLWM2MError_NotFound, AwaPathResult_GetLWM2MError(pathResult));
    AwaServerReadOperation_Free(&operation);
}

} // namespace Awa

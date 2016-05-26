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

#include <lwm2m_tree_node.h>

#include "awa/client.h"
#include "awa/common.h"
#include "log.h"
#include "get_response.h"
#include "arrays.h"
#include "support/support.h"
#include "utils.h"

namespace Awa {

class TestGetOperation : public TestClientBase {};

class TestGetOperationWithConnectedSession : public TestClientWithConnectedSession {};

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_New_returns_valid_operation_and_free_works)
{
    // test that AwaClientGetOperation_Free works via valgrind
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Free(&getOperation));
}

TEST_F(TestGetOperation, AwaClientGetOperation_New_handles_null_session)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(NULL);
    ASSERT_EQ(NULL, getOperation);
}

TEST_F(TestGetOperation, AwaClientGetOperation_New_handles_invalid_session)
{
    // An invalid session is one that is not connected
    AwaClientSession * session = AwaClientSession_New();
    EXPECT_EQ(NULL, AwaClientGetOperation_New(session));

    // Set up IPC - still not enough
    AwaClientSession_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort);
    EXPECT_EQ(NULL, AwaClientGetOperation_New(session));

    AwaClientSession_Free(&session);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_Free_nulls_pointer)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Free(&getOperation));
    ASSERT_EQ(NULL, getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_Free_handles_null)
{
    EXPECT_EQ(AwaError_OperationInvalid, AwaClientGetOperation_Free(NULL));
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_Free_handles_null_pointer)
{
    AwaClientGetOperation * getOperation = NULL;
    EXPECT_EQ(AwaError_OperationInvalid, AwaClientGetOperation_Free(&getOperation));
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_Free_handles_invalid_session)
{
    // Session is freed before the operation (use a local session):
    AwaClientSession * session = AwaClientSession_New();
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(session));
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session);
    AwaClientSession_Free(&session);
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Free(&getOperation));
    // expect no crash or memory leaks
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_AddPath_handles_valid_path)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);

    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000/0/1"));
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_AddPath_handles_invalid_path)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);

    EXPECT_EQ(AwaError_PathInvalid, AwaClientGetOperation_AddPath(getOperation, "a/n in/valid/ path"));
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_AddPath_handles_null_path)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_PathInvalid, AwaClientGetOperation_AddPath(getOperation, NULL));
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperation, AwaClientGetOperation_AddPath_handles_null_operation)
{
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientGetOperation_AddPath(NULL, "/1000/0/1"));
}

// FIXME: FLOWDM-370
TEST_F(TestGetOperationWithConnectedSession, DISABLED_AwaClientGetOperation_AddPath_handles_invalid_operation)
{
    // Consider if the parent session has been freed early
    AwaClientSession * session = AwaClientSession_New();
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(session));
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session);
    ASSERT_TRUE(NULL != getOperation);

    // Free the session
    AwaClientSession_Free(&session);

    // TODO: we need the session to keep track of all operations, and
    // invalidate them when freed so they can detect this situation.

    ASSERT_EQ(AwaError_SessionInvalid, AwaClientGetOperation_AddPath(getOperation, "/1000/0/1"));

    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Free(&getOperation));
    // expect no crash or memory leaks
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_AddPath_handles_more_general_path)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_); ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/0"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation); ASSERT_TRUE(NULL != getResponse);

    EXPECT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/3/0/0"));
    EXPECT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/3/0/1"));
    EXPECT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/3/0/2"));
    // ... expecting the rest of /3/0

    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_AddPathWithArrayRange_handles_null_operation)
{
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientGetOperation_AddPathWithArrayRange(NULL, "/3/0/6", 0, 1));
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_AddPathWithArrayRange_handles_null_path)
{
    AwaClientGetOperation * operation = AwaClientGetOperation_New(session_);
    ASSERT_EQ(AwaError_PathInvalid, AwaClientGetOperation_AddPathWithArrayRange(operation, NULL, 0, 1));
    AwaClientGetOperation_Free(&operation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_AddPathWithArrayRange_handles_invalid_path)
{
    AwaClientGetOperation * operation = AwaClientGetOperation_New(session_);
    ASSERT_EQ(AwaError_PathInvalid, AwaClientGetOperation_AddPathWithArrayRange(operation, "/inv.alid/path///", 0, 1));

    // Object Instance paths are not valid:
    ASSERT_EQ(AwaError_PathInvalid, AwaClientGetOperation_AddPathWithArrayRange(operation, "/3/0", 0, 1));

    // Object paths are not valid:
    ASSERT_EQ(AwaError_PathInvalid, AwaClientGetOperation_AddPathWithArrayRange(operation, "/3", 0, 1));

    AwaClientGetOperation_Free(&operation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_AddPathWithArrayRange_handles_invalid_range)
{
    AwaClientGetOperation * operation = AwaClientGetOperation_New(session_);

    // overflow the ID range:
    EXPECT_EQ(AwaError_AddInvalid, AwaClientGetOperation_AddPathWithArrayRange(operation, "/3/0/6", 1, SIZE_MAX));
    EXPECT_EQ(AwaError_AddInvalid, AwaClientGetOperation_AddPathWithArrayRange(operation, "/3/0/6", SIZE_MAX, 1));

    AwaClientGetOperation_Free(&operation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_AddPathWithArrayRange_handles_invalid_count)
{
    AwaClientGetOperation * operation = AwaClientGetOperation_New(session_);
    // Zero is an invalid count value:
    ASSERT_EQ(AwaError_AddInvalid, AwaClientGetOperation_AddPathWithArrayRange(operation, "/3/0/6", 0, 0));

    AwaClientGetOperation_Free(&operation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_AddPathWithArrayRange_handles_single_resource_instance)
{
    AwaClientGetOperation * operation = AwaClientGetOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPathWithArrayRange(operation, "/3/0/6", 0, 1));
    AwaClientGetOperation_Free(&operation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_AddPathWithArrayRange_handles_multiple_resource_instances)
{
    AwaClientGetOperation * operation = AwaClientGetOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPathWithArrayRange(operation, "/3/0/6", 0, 2));
    AwaClientGetOperation_Free(&operation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_AddPathWithArrayRange_handles_valid_inputs_multiple_resources)
{
    AwaClientGetOperation * operation = AwaClientGetOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPathWithArrayRange(operation, "/3/0/6", 0, 2));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPathWithArrayRange(operation, "/3/0/7", 0, 2));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPathWithArrayRange(operation, "/3/0/8", 0, 2));
    AwaClientGetOperation_Free(&operation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_AddPathWithArrayRange_handles_multiple_ranges_on_same_resource)
{
    AwaClientGetOperation * operation = AwaClientGetOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPathWithArrayRange(operation, "/3/0/6", 0, 2));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPathWithArrayRange(operation, "/3/0/6", 10, 12));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPathWithArrayRange(operation, "/3/0/6", 100, 200));
    AwaClientGetOperation_Free(&operation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_Perform_handles_valid_operation)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);

    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_Perform_handles_null_operation)
{
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientGetOperation_Perform(NULL, defaults::timeout));
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_Perform_handles_negative_timeout)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);

    ASSERT_EQ(AwaError_OperationInvalid, AwaClientGetOperation_Perform(getOperation, -1));
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Free(&getOperation));
}

// FIXME: FLOWDM-360
TEST_F(TestGetOperationWithConnectedSession, DISABLED_AwaClientGetOperation_Perform_handles_zero_timeout)
{
    // how?
}

// FIXME: FLOWDM-360
TEST_F(TestGetOperationWithConnectedSession, DISABLED_AwaClientGetOperation_Perform_handles_short_timeout)
{
    // how?
}

TEST_F(TestGetOperation, AwaClientGetOperation_Perform_honours_timeout)
{
    // start a client
    const char * clientID = "TestClient1";

    AwaClientDaemonHorde * horde_ = new AwaClientDaemonHorde( { clientID }, 61001);
    sleep(1);      // wait for the client to register with the server

    AwaClientSession * session = AwaClientSession_New();
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(session, "0.0.0.0", 61001));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(session));

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session);
    ASSERT_TRUE(NULL != getOperation);
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/0"));

    // Tear down client process
    delete horde_;

    BasicTimer timer;
    timer.Start();
    EXPECT_EQ(AwaError_Timeout, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    timer.Stop();
    EXPECT_TRUE(ElapsedTimeWithinTolerance(timer.TimeElapsed_Milliseconds(), defaults::timeout, defaults::timeoutTolerance)) << "Time elapsed: " << timer.TimeElapsed_Milliseconds() << "ms";

    AwaClientGetOperation_Free(&getOperation);
    AwaClientSession_Free(&session);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_Perform_handles_invalid_operation_no_content)
{
    // Test behaviour when operation has no content
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);

    ASSERT_EQ(AwaError_OperationInvalid, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_Perform_handles_disconnected_session)
{
    // Test behaviour when parent session has been disconnected
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/1"));

    AwaClientSession_Disconnect(session_);

    ASSERT_EQ(AwaError_SessionNotConnected, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    AwaClientGetOperation_Free(&getOperation);
}

// FIXME: FLOWDM-370
TEST_F(TestGetOperationWithConnectedSession, DISABLED_AwaClientGetOperation_Perform_handles_invalid_operation_freed_session)
{
    // Test behaviour when parent session has been freed
    ASSERT_FALSE(1);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_GetResponse_handles_resource)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);

    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));

    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);

    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_Perform_handles_object_instance)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);

    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));

    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);

    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_Perform_handles_object)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);

    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));

    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);

    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_GetResponse_handles_null_operation)
{
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(NULL);
    ASSERT_TRUE(NULL == getResponse);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_GetResponse_handles_invalid_operation)
{
    //Consider if we haven't called ProcessGetOperation
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);

    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/1"));

    //haven't processed get operation, so get response should return NULL
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL == getResponse);

    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_GetResponse_handles_reusing_operation)
{
    //Test we can call AwaClientGetOperation_GetResponse twice and reuse the same operation
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);

    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));

    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);

    const AwaClientGetResponse * getResponse2 = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse2);

    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_handles_single_resource_instance)
{
    AwaClientGetOperation * operation = AwaClientGetOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPathWithArrayRange(operation, "/3/0/6", 0, 1));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(operation, defaults::timeout));
    const AwaClientGetResponse * response = AwaClientGetOperation_GetResponse(operation); ASSERT_TRUE(NULL != response);
    ASSERT_TRUE(NULL != response);
    // check that /3/0/6 is in the response
    EXPECT_TRUE(AwaClientGetResponse_HasValue(response, "/3/0/6"));

    // and that it's an array with elements within the specified range
    const AwaIntegerArray * array = NULL;
    AwaClientGetResponse_GetValuesAsIntegerArrayPointer(response, "/3/0/6", &array);

    AwaIntegerArrayIterator * iterator = AwaIntegerArray_NewIntegerArrayIterator(array);
    EXPECT_TRUE(AwaIntegerArrayIterator_Next(iterator));
    EXPECT_EQ(static_cast<AwaArrayIndex>(0), AwaIntegerArrayIterator_GetIndex(iterator));

    EXPECT_FALSE(AwaIntegerArrayIterator_Next(iterator));
    AwaIntegerArrayIterator_Free(&iterator);
    EXPECT_TRUE(NULL == iterator);

    AwaClientGetOperation_Free(&operation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetOperation_handles_invalid_array_range)
{
    AwaClientGetOperation * operation = AwaClientGetOperation_New(session_);
    ASSERT_EQ(AwaError_AddInvalid, AwaClientGetOperation_AddPathWithArrayRange(operation, "/3/0/6", 1, 0));
    AwaClientGetOperation_Free(&operation);
}

/***********************************************************************************************************
 * GetValue parameterised tests
 */

namespace detail
{

struct TestGetResource
{
    AwaError expectedResult;
    const void * expectedValue;
    int expectedValueSize;
    const char * path;
    bool UseResponse;
    AwaResourceType type;
};

::std::ostream& operator<<(::std::ostream& os, const TestGetResource& item)
{
  return os << "Item: expectedResult " << item.expectedResult
            << ", expectedValue " << item.expectedValue
            << ", expectedValueSize " << item.expectedValueSize
            << ", path " << item.path
            << ", UseResponse " << item.UseResponse
            << ", type " << item.type;
}

const AwaInteger expectedInteger1 = 100;
const char * expectedString1 = "Lightweight M2M Client";
const AwaFloat expectedFloat1 = 1.337;
const AwaTime expectedTime1 = 0xA20AD72B;
const AwaBoolean expectedBoolean1 = true;

const char dummyOpaqueData[] = {'a',0,'x','\0', 123};
const AwaOpaque expectedOpaque1 = {(void*) dummyOpaqueData, sizeof(dummyOpaqueData)};
const AwaObjectLink expectedObjectLink1 = { 4, 123 };

// Add a TestGetResource for each type except the type given, so we can ensure the type doesn't match
void BuildTypeMismatchContainer(std::vector<detail::TestGetResource>& container, const char * path, AwaResourceType skip)
{
    for (int type = AwaResourceType_String; type != AwaResourceType_StringArray; type++)
    {
        if (type == skip)
            continue;
        container.push_back(detail::TestGetResource {AwaError_TypeMismatch,            NULL,     -1,    path,   true,   static_cast<AwaResourceType>(type)});
    }
}

const std::vector<detail::TestGetResource> BuildTypeMismatchContainers()
{
    std::vector<detail::TestGetResource> container;
    BuildTypeMismatchContainer(container, "/10000/0/1", AwaResourceType_String);
    BuildTypeMismatchContainer(container, "/10000/0/2", AwaResourceType_Integer);
    BuildTypeMismatchContainer(container, "/10000/0/3", AwaResourceType_Float);
    BuildTypeMismatchContainer(container, "/10000/0/4", AwaResourceType_Boolean);
    BuildTypeMismatchContainer(container, "/10000/0/5", AwaResourceType_Opaque);
    BuildTypeMismatchContainer(container, "/10000/0/6", AwaResourceType_Time);
    BuildTypeMismatchContainer(container, "/10000/0/7", AwaResourceType_ObjectLink);
    return container;
}

} // namespace detail

class TestGetValue : public TestGetOperationWithConnectedSession, public ::testing::WithParamInterface< detail::TestGetResource>
{
protected:

    void SetUp() {
        TestGetOperationWithConnectedSession::SetUp();

        //Define and set our custom objects - TODO define in some common class for get, set etc?
        AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
        EXPECT_TRUE(defineOperation != NULL);


        AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(10000, "Test Object", 0, 1);
        ASSERT_TRUE(NULL != customObjectDefinition);

        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsNoType(customObjectDefinition, 0, "Test None Resource", false, AwaResourceOperations_Execute));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsString(customObjectDefinition, 1, "Test String Resource", true, AwaResourceOperations_ReadWrite, detail::expectedString1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(customObjectDefinition, 2, "Test Integer Resource", true, AwaResourceOperations_ReadWrite, detail::expectedInteger1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloat(customObjectDefinition, 3, "Test Float Resource", true, AwaResourceOperations_ReadWrite, detail::expectedFloat1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBoolean(customObjectDefinition, 4, "Test Boolean Resource", true, AwaResourceOperations_ReadWrite, detail::expectedBoolean1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque(customObjectDefinition, 5, "Test Opaque Resource", true, AwaResourceOperations_ReadWrite, detail::expectedOpaque1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTime(customObjectDefinition, 6, "Test Time Resource", true, AwaResourceOperations_ReadWrite, detail::expectedTime1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLink(customObjectDefinition, 7, "Test ObjectLink Resource", true, AwaResourceOperations_ReadWrite, detail::expectedObjectLink1));

        // define another object that we can use for "path not in result" tests
        AwaObjectDefinition * customObjectDefinition2 = AwaObjectDefinition_New(10001, "Test Object 2", 0, 1);
        ASSERT_TRUE(NULL != customObjectDefinition2);

        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsNoType(customObjectDefinition2, 0, "Test None Resource", false, AwaResourceOperations_Execute));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsString(customObjectDefinition2, 1, "Test String Resource", true, AwaResourceOperations_ReadWrite, detail::expectedString1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(customObjectDefinition2, 2, "Test Integer Resource", true, AwaResourceOperations_ReadWrite, detail::expectedInteger1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloat(customObjectDefinition2, 3, "Test Float Resource", true, AwaResourceOperations_ReadWrite, detail::expectedFloat1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBoolean(customObjectDefinition2, 4, "Test Boolean Resource", true, AwaResourceOperations_ReadWrite, detail::expectedBoolean1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque(customObjectDefinition2, 5, "Test Opaque Resource", true, AwaResourceOperations_ReadWrite, detail::expectedOpaque1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTime(customObjectDefinition2, 6, "Test Time Resource", true, AwaResourceOperations_ReadWrite, detail::expectedTime1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLink(customObjectDefinition2, 7, "Test ObjectLink Resource", true, AwaResourceOperations_ReadWrite, detail::expectedObjectLink1));

        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition2));
        ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

        AwaObjectDefinition_Free(&customObjectDefinition);
        AwaObjectDefinition_Free(&customObjectDefinition2);
        AwaClientDefineOperation_Free(&defineOperation);

        //Create a basic set operation to create our custom objects - TODO remove once we can set default values
        AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
        ASSERT_TRUE(NULL != setOperation);

        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));

        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsCString(setOperation, "/10000/0/1", detail::expectedString1));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsInteger(setOperation, "/10000/0/2", detail::expectedInteger1));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsFloat(setOperation, "/10000/0/3", detail::expectedFloat1));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsBoolean(setOperation, "/10000/0/4", detail::expectedBoolean1));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsOpaque(setOperation, "/10000/0/5", detail::expectedOpaque1));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsTime(setOperation, "/10000/0/6", detail::expectedTime1));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsObjectLink(setOperation, "/10000/0/7", detail::expectedObjectLink1));

        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
        AwaClientSetOperation_Free(&setOperation);


        getOperation_ = AwaClientGetOperation_New(session_);
        ASSERT_TRUE(NULL != getOperation_);

        // list of paths to objects we require
        ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation_, "/10000"));

        ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation_, 0));
        getResponse_ = AwaClientGetOperation_GetResponse(getOperation_);
        ASSERT_TRUE(NULL != getResponse_);

    }

    void TearDown() {
        TestGetOperationWithConnectedSession::TearDown();
        AwaClientGetOperation_Free(&getOperation_);
    }

    AwaClientGetOperation * getOperation_;
    const AwaClientGetResponse * getResponse_;
};

TEST_P(TestGetValue, TestGetValueInstantiation)
{
    detail::TestGetResource data = GetParam();
    const AwaClientGetResponse * getResponse = data.UseResponse ? this->getResponse_ : NULL;
    void * value = NULL;

    AwaObjectLink receivedObjectLink = {0, 0};
    AwaOpaque receivedOpaque = {NULL, 0};

    switch(data.type)
    {
    case AwaResourceType_String:
        ASSERT_EQ(data.expectedResult, AwaClientGetResponse_GetValueAsCStringPointer(getResponse, data.path, (const char **)&value));
        break;
    case AwaResourceType_Integer:
        ASSERT_EQ(data.expectedResult, AwaClientGetResponse_GetValueAsIntegerPointer(getResponse, data.path, (const AwaInteger **)&value));
        break;
    case AwaResourceType_Float:
        ASSERT_EQ(data.expectedResult, AwaClientGetResponse_GetValueAsFloatPointer(getResponse, data.path, (const AwaFloat **)&value));
        break;
    case AwaResourceType_Boolean:
        ASSERT_EQ(data.expectedResult, AwaClientGetResponse_GetValueAsBooleanPointer(getResponse, data.path, (const AwaBoolean **)&value));
        break;
    case AwaResourceType_Opaque:
        ASSERT_EQ(data.expectedResult, AwaClientGetResponse_GetValueAsOpaque(getResponse, data.path, &receivedOpaque));
        ASSERT_EQ(data.expectedResult, AwaClientGetResponse_GetValueAsOpaquePointer(getResponse, data.path, (const AwaOpaque **)&value));
        break;
    case AwaResourceType_Time:
        ASSERT_EQ(data.expectedResult, AwaClientGetResponse_GetValueAsTimePointer(getResponse, data.path, (const AwaTime **)&value));
        break;
    case AwaResourceType_ObjectLink:
        ASSERT_EQ(data.expectedResult, AwaClientGetResponse_GetValueAsObjectLink(getResponse, data.path, &receivedObjectLink));
        ASSERT_EQ(data.expectedResult, AwaClientGetResponse_GetValueAsObjectLinkPointer(getResponse, data.path, (const AwaObjectLink **)&value));
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
                ASSERT_EQ(0, memcmp(expectedOpaque->Data, receivedOpaquePointer->Data, expectedOpaque->Size));
                ASSERT_EQ(0, memcmp(expectedOpaque->Data, receivedOpaque.Data, receivedOpaque.Size));
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
                const AwaObjectLink * receivedObjectLinkPointer = static_cast<AwaObjectLink *>(value);
                ASSERT_EQ(0, memcmp(expectedObjectLink, receivedObjectLinkPointer, sizeof(AwaObjectLink)));
                ASSERT_EQ(0, memcmp(expectedObjectLink, &receivedObjectLink, sizeof(AwaObjectLink)));
                break;
            }
            default:
                ASSERT_TRUE(false);
        }
    }
}

INSTANTIATE_TEST_CASE_P(
        TestGetValueSuccessInstantiation,
        TestGetValue,
        ::testing::Values(
          detail::TestGetResource {AwaError_Success,            (void *)detail::expectedString1, -1,                              "/10000/0/1",   true,   AwaResourceType_String},
          detail::TestGetResource {AwaError_Success,            &detail::expectedInteger1,       sizeof(AwaInteger),          "/10000/0/2",   true,   AwaResourceType_Integer},
          detail::TestGetResource {AwaError_Success,            &detail::expectedFloat1,         sizeof(AwaFloat),            "/10000/0/3",   true,   AwaResourceType_Float},
          detail::TestGetResource {AwaError_Success,            &detail::expectedBoolean1,       sizeof(AwaBoolean),          "/10000/0/4",   true,   AwaResourceType_Boolean},
          detail::TestGetResource {AwaError_Success,            &detail::expectedOpaque1,        sizeof(detail::expectedOpaque1), "/10000/0/5",   true,   AwaResourceType_Opaque},
          detail::TestGetResource {AwaError_Success,            &detail::expectedTime1,          sizeof(AwaTime),             "/10000/0/6",   true,   AwaResourceType_Time},
          detail::TestGetResource {AwaError_Success,            &detail::expectedObjectLink1,    sizeof(AwaObjectLink),       "/10000/0/7",   true,   AwaResourceType_ObjectLink}
        ));


//FIXME: add tests for NULL, invalid, not defined, non-existing instance etc
INSTANTIATE_TEST_CASE_P(
        TestGetValueInvalidPathInstantiation,
        TestGetValue,
        ::testing::Values(
          detail::TestGetResource {AwaError_PathInvalid,      (void *)detail::expectedString1, -1, NULL,          true,  AwaResourceType_String},  // handles null path
          detail::TestGetResource {AwaError_PathNotFound,     (void *)detail::expectedString1, -1, "/10001/0/1",  true,  AwaResourceType_String},  // handles path not in response
          detail::TestGetResource {AwaError_OperationInvalid, (void *)detail::expectedString1, -1, "/10000/0/1",  false, AwaResourceType_String},   // handles invalid response

          detail::TestGetResource {AwaError_PathInvalid,      &detail::expectedInteger1, sizeof(AwaInteger), NULL,          true,  AwaResourceType_Integer},  // handles null path
          detail::TestGetResource {AwaError_PathNotFound,     &detail::expectedInteger1, sizeof(AwaInteger), "/10001/0/2",  true,  AwaResourceType_Integer},  // handles path not in response
          detail::TestGetResource {AwaError_OperationInvalid, &detail::expectedInteger1, sizeof(AwaInteger), "/10000/0/2",  false, AwaResourceType_Integer},  // handles invalid response

          detail::TestGetResource {AwaError_PathInvalid,      &detail::expectedFloat1, sizeof(AwaFloat), NULL,          true,  AwaResourceType_Float},  // handles null path
          detail::TestGetResource {AwaError_PathNotFound,     &detail::expectedFloat1, sizeof(AwaFloat), "/10001/0/3",  true,  AwaResourceType_Float},  // handles path not in response
          detail::TestGetResource {AwaError_OperationInvalid, &detail::expectedFloat1, sizeof(AwaFloat), "/10000/0/3",  false, AwaResourceType_Float},  // handles invalid response

          detail::TestGetResource {AwaError_PathInvalid,      &detail::expectedFloat1, sizeof(AwaBoolean), NULL,          true,  AwaResourceType_Boolean},  // handles null path
          detail::TestGetResource {AwaError_PathNotFound,     &detail::expectedFloat1, sizeof(AwaBoolean), "/10001/0/4",  true,  AwaResourceType_Boolean},  // handles path not in response
          detail::TestGetResource {AwaError_OperationInvalid, &detail::expectedFloat1, sizeof(AwaBoolean), "/10000/0/4",  false, AwaResourceType_Boolean},  // handles invalid response

          detail::TestGetResource {AwaError_PathInvalid,      &detail::expectedOpaque1, sizeof(detail::expectedOpaque1), NULL,          true,  AwaResourceType_Opaque},  // handles null path
          detail::TestGetResource {AwaError_PathNotFound,     &detail::expectedOpaque1, sizeof(detail::expectedOpaque1), "/10001/0/5",  true,  AwaResourceType_Opaque},  // handles path not in response
          detail::TestGetResource {AwaError_OperationInvalid, &detail::expectedOpaque1, sizeof(detail::expectedOpaque1), "/10000/0/5",  false, AwaResourceType_Opaque},  // handles invalid response

          detail::TestGetResource {AwaError_PathInvalid,      &detail::expectedTime1, sizeof(AwaTime), NULL,          true,  AwaResourceType_Time},  // handles null path
          detail::TestGetResource {AwaError_PathNotFound,     &detail::expectedTime1, sizeof(AwaTime), "/10001/0/6",  true,  AwaResourceType_Time},  // handles path not in response
          detail::TestGetResource {AwaError_OperationInvalid, &detail::expectedTime1, sizeof(AwaTime), "/10000/0/6",  false, AwaResourceType_Time},  // handles invalid response

          detail::TestGetResource {AwaError_PathInvalid,      &detail::expectedObjectLink1, sizeof(AwaObjectLink), NULL,          true,  AwaResourceType_ObjectLink},  // handles null path
          detail::TestGetResource {AwaError_PathNotFound,     &detail::expectedObjectLink1, sizeof(AwaObjectLink), "/10001/0/7",  true,  AwaResourceType_ObjectLink},  // handles path not in response
          detail::TestGetResource {AwaError_OperationInvalid, &detail::expectedObjectLink1, sizeof(AwaObjectLink), "/10000/0/7",  false, AwaResourceType_ObjectLink}   // handles invalid response

        ));

INSTANTIATE_TEST_CASE_P(
        TestGetValueTypeMismatchInstantiation,
        TestGetValue,
        ::testing::ValuesIn(detail::BuildTypeMismatchContainers()));

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_GetValueAsIntegerPointer_handles_null_value)
{
    const AwaInteger * value = NULL;
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientGetResponse_GetValueAsIntegerPointer(NULL, "/3/0/9", &value));
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_GetValueAsIntegerPointer_handles_null_response)
{
    const AwaInteger * value = NULL;
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientGetResponse_GetValueAsIntegerPointer(NULL, "/3/0/9", &value));
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_HasValue_handles_null_response)
{
    ASSERT_FALSE(AwaClientGetResponse_HasValue(NULL, "/3/0/1"));
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_HasValue_handles_null_path)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    ASSERT_FALSE(AwaClientGetResponse_HasValue(getResponse, NULL));
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_HasValue_handles_invalid_path)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    ASSERT_FALSE(AwaClientGetResponse_HasValue(getResponse, "/3/0/9"));
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_HasValue_handles_non_resource_path)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    ASSERT_FALSE(AwaClientGetResponse_HasValue(getResponse, "/3/0"));
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_HasValue_handles_valid_path)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    ASSERT_TRUE(AwaClientGetResponse_HasValue(getResponse, "/3/0/1"));
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_ContainsPath_handles_null_response)
{
    ASSERT_FALSE(AwaClientGetResponse_ContainsPath(NULL, "/3/0/1"));
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_ContainsPath_handles_null_path)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    ASSERT_FALSE(AwaClientGetResponse_ContainsPath(getResponse, NULL));
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_ContainsPath_handles_valid_resource_path)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    ASSERT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/3/0/1"));
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_ContainsPath_handles_valid_resource_path_from_entire_object)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    ASSERT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/3/0/1"));
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_ContainsPath_handles_valid_object_instance_path)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    ASSERT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/3/0"));
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_ContainsPath_handles_valid_object_path)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    ASSERT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/3"));
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_ContainsPath_handles_invalid_resource_path)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    ASSERT_FALSE(AwaClientGetResponse_ContainsPath(getResponse, "/3/0/2"));
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperation, AwaClientGetResponse_NewPathIterator_handles_null_response)
{
    EXPECT_EQ(NULL, AwaClientGetResponse_NewPathIterator(NULL));
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_NewPathIterator_handles_valid_response)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/9"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);

    AwaPathIterator * iterator = AwaClientGetResponse_NewPathIterator(getResponse);
    ASSERT_TRUE(NULL != iterator);

    // call Next once before getting values:
    EXPECT_EQ(true, AwaPathIterator_Next(iterator));
    EXPECT_STREQ("/3/0/9", AwaPathIterator_Get(iterator));

    // not expecting any more paths
    EXPECT_TRUE(false == AwaPathIterator_Next(iterator));
    EXPECT_TRUE(NULL == AwaPathIterator_Get(iterator));

    AwaPathIterator_Free(&iterator);
    EXPECT_EQ(NULL, iterator);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_NewPathIterator_handles_valid_response_multiple_paths)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_); ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/0"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/9"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation); ASSERT_TRUE(NULL != getResponse);

    AwaPathIterator * iterator = AwaClientGetResponse_NewPathIterator(getResponse);
    ASSERT_TRUE(NULL != iterator);

    // call Next once before getting values:
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
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_NewPathIterator_handles_valid_response_object_instance)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_); ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation); ASSERT_TRUE(NULL != getResponse);


    EXPECT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/3/0/0"));
    EXPECT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/3/0/1"));
    EXPECT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/3/0/2"));
    // ... expecting the rest of /3/0

    AwaClientGetOperation_Free(&getOperation);
}


/***********************************************************************************************************
 * GetValueArray parameterised tests
 */

namespace detail
{

struct TestGetResourceArray
{
    AwaError expectedResult;
    const char * path;
    bool UseResponse;
    AwaResourceType type;
};

::std::ostream& operator<<(::std::ostream& os, const TestGetResourceArray& item)
{
  return os << "Item: expectedResult " << item.expectedResult
            << ", path " << item.path
            << ", UseResponse " << item.UseResponse
            << ", type " << item.type;
}

/*
const AwaInteger expectedInteger1 = 100;
const char * expectedString1 = "Lightweight M2M Client";
AwaFloat expectedFloat1 = 1.337;
AwaTime expectedTime1 = 0xA20AD72B;
AwaBoolean expectedBoolean1 = true;

static char dummyOpaqueData[] = {'a',0,'x','\0', 123};
AwaOpaque expectedOpaque1 = {(void*) dummyOpaqueData, sizeof(dummyOpaqueData), sizeof(dummyOpaqueData)};
AwaObjectLink expectedObjectLink1 = { 3, 0 };*/

}

class TestGetValueArray : public TestGetOperationWithConnectedSession, public ::testing::WithParamInterface< detail::TestGetResourceArray>
{
protected:

    void SetUp() {
        TestGetOperationWithConnectedSession::SetUp();

        //Define and set our custom objects - TODO define in some common class for get, set etc?
        AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
        EXPECT_TRUE(defineOperation != NULL);


        AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(10000, "Test Object", 0, 1);
        ASSERT_TRUE(NULL != customObjectDefinition);

        expectedStringArray_ = AwaStringArray_New();
        expectedIntegerArray_ = AwaIntegerArray_New();
        expectedFloatArray_ = AwaFloatArray_New();
        expectedBooleanArray_ = AwaBooleanArray_New();
        expectedOpaqueArray_ = AwaOpaqueArray_New();
        expectedTimeArray_ = AwaTimeArray_New();
        expectedObjectLinkArray_ = AwaObjectLinkArray_New();

        for (int i = 0; i < 3; i++)
        {
            char * stringValue;
            msprintf(&stringValue, "%d", i*2);
            AwaStringArray_SetValueAsCString(expectedStringArray_, i, (const char *)stringValue);
            Awa_MemSafeFree(stringValue);

            AwaIntegerArray_SetValue(expectedIntegerArray_, i, i*2);
            AwaFloatArray_SetValue(expectedFloatArray_, i, i*2.5);
            AwaBooleanArray_SetValue(expectedBooleanArray_, i, i%2==0);

            static char dummyOpaqueData[] = {(char)i, 'a',0,'x','\0', (char)i};

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


        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
        ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

        AwaObjectDefinition_Free(&customObjectDefinition);
        AwaClientDefineOperation_Free(&defineOperation);

        //Create a basic set operation to create our custom objects - TODO remove setting values once we can set default values
        AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
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

        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
        AwaClientSetOperation_Free(&setOperation);

        getOperation_ = AwaClientGetOperation_New(session_);
        ASSERT_TRUE(NULL != getOperation_);

        // list of paths to objects we require
        ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation_, "/10000"));

        ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation_, defaults::timeout));
        getResponse_ = AwaClientGetOperation_GetResponse(getOperation_);
        ASSERT_TRUE(NULL != getResponse_);
    }

    void TearDown()
    {
        TestGetOperationWithConnectedSession::TearDown();
        AwaClientGetOperation_Free(&getOperation_);
        AwaStringArray_Free(&expectedStringArray_);
        AwaIntegerArray_Free(&expectedIntegerArray_);
        AwaFloatArray_Free(&expectedFloatArray_);
        AwaBooleanArray_Free(&expectedBooleanArray_);
        AwaOpaqueArray_Free(&expectedOpaqueArray_);
        AwaTimeArray_Free(&expectedTimeArray_);
        AwaObjectLinkArray_Free(&expectedObjectLinkArray_);
    }

    AwaClientGetOperation * getOperation_;
    const AwaClientGetResponse * getResponse_;
    AwaStringArray * expectedStringArray_;
    AwaIntegerArray * expectedIntegerArray_;
    AwaFloatArray * expectedFloatArray_;
    AwaBooleanArray * expectedBooleanArray_;
    AwaOpaqueArray * expectedOpaqueArray_;
    AwaTimeArray * expectedTimeArray_;
    AwaObjectLinkArray * expectedObjectLinkArray_;
};

TEST_P(TestGetValueArray, TestGetValueArrayInstantiation)
{
    detail::TestGetResourceArray data = GetParam();
    const AwaClientGetResponse * getResponse = data.UseResponse ? this->getResponse_ : NULL;
    AwaArray * array = NULL;

    AwaArray * expectedArray = NULL;

    switch(data.type)
    {
        case AwaResourceType_StringArray:
        {
            expectedArray = (AwaArray *)this->expectedStringArray_;
            ASSERT_EQ(data.expectedResult, AwaClientGetResponse_GetValuesAsStringArrayPointer(getResponse, data.path, (const AwaStringArray **)&array));
            break;
        }
        case AwaResourceType_IntegerArray:
        {
            expectedArray = (AwaArray *)this->expectedIntegerArray_;
            ASSERT_EQ(data.expectedResult, AwaClientGetResponse_GetValuesAsIntegerArrayPointer(getResponse, data.path, (const AwaIntegerArray **)&array));
            break;
        }
        case AwaResourceType_FloatArray:
        {
            expectedArray = (AwaArray *)this->expectedFloatArray_;
            ASSERT_EQ(data.expectedResult, AwaClientGetResponse_GetValuesAsFloatArrayPointer(getResponse, data.path, (const AwaFloatArray **)&array));
            break;
        }
        case AwaResourceType_BooleanArray:
        {
            expectedArray = (AwaArray *)this->expectedBooleanArray_;
            ASSERT_EQ(data.expectedResult, AwaClientGetResponse_GetValuesAsBooleanArrayPointer(getResponse, data.path, (const AwaBooleanArray **)&array));
            break;
        }
        case AwaResourceType_OpaqueArray:
        {
            expectedArray = (AwaArray *)this->expectedOpaqueArray_;
            ASSERT_EQ(data.expectedResult, AwaClientGetResponse_GetValuesAsOpaqueArrayPointer(getResponse, data.path, (const AwaOpaqueArray **)&array));
            break;
        }
        case AwaResourceType_TimeArray:
        {
            expectedArray = (AwaArray *)this->expectedTimeArray_;
            ASSERT_EQ(data.expectedResult, AwaClientGetResponse_GetValuesAsTimeArrayPointer(getResponse, data.path, (const AwaTimeArray **)&array));
            break;
        }
        case AwaResourceType_ObjectLinkArray:
        {
            expectedArray = (AwaArray *)this->expectedObjectLinkArray_;
            ASSERT_EQ(data.expectedResult, AwaClientGetResponse_GetValuesAsObjectLinkArrayPointer(getResponse, data.path, (const AwaObjectLinkArray **)&array));
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
        TestGetValueArraySuccessInstantiation,
        TestGetValueArray,
        ::testing::Values(
          detail::TestGetResourceArray {AwaError_Success,           "/10000/0/1",   true,   AwaResourceType_StringArray},
          detail::TestGetResourceArray {AwaError_Success,           "/10000/0/2",   true,   AwaResourceType_IntegerArray},
          detail::TestGetResourceArray {AwaError_Success,           "/10000/0/3",   true,   AwaResourceType_FloatArray},
          detail::TestGetResourceArray {AwaError_Success,           "/10000/0/4",   true,   AwaResourceType_BooleanArray},
          detail::TestGetResourceArray {AwaError_Success,           "/10000/0/5",   true,   AwaResourceType_OpaqueArray},
          detail::TestGetResourceArray {AwaError_Success,           "/10000/0/6",   true,   AwaResourceType_TimeArray},
          detail::TestGetResourceArray {AwaError_Success,           "/10000/0/7",   true,   AwaResourceType_ObjectLinkArray}
        ));

INSTANTIATE_TEST_CASE_P(
        TestGetValueArrayInvalidResponseInstantiation,
        TestGetValueArray,
        ::testing::Values(
          detail::TestGetResourceArray {AwaError_OperationInvalid,           "/10000/0/1",   false,   AwaResourceType_StringArray},
          detail::TestGetResourceArray {AwaError_OperationInvalid,           "/10000/0/2",   false,   AwaResourceType_IntegerArray},
          detail::TestGetResourceArray {AwaError_OperationInvalid,                "/10000/0/3",   false,   AwaResourceType_FloatArray},
          detail::TestGetResourceArray {AwaError_OperationInvalid,           "/10000/0/4",   false,   AwaResourceType_BooleanArray},
          detail::TestGetResourceArray {AwaError_OperationInvalid,           "/10000/0/5",   false,   AwaResourceType_OpaqueArray},
          detail::TestGetResourceArray {AwaError_OperationInvalid,           "/10000/0/6",   false,   AwaResourceType_TimeArray},
          detail::TestGetResourceArray {AwaError_OperationInvalid,            "/10000/0/7",   false,   AwaResourceType_ObjectLinkArray}
        ));

INSTANTIATE_TEST_CASE_P(
        TestGetValueArrayPathNotFoundInstantiation,
        TestGetValueArray,
        ::testing::Values(
          detail::TestGetResourceArray {AwaError_PathNotFound,           "/10000/1/1",   true,   AwaResourceType_StringArray},
          detail::TestGetResourceArray {AwaError_PathNotFound,           "/10000/1/2",   true,   AwaResourceType_IntegerArray},
          detail::TestGetResourceArray {AwaError_PathNotFound,                "/10000/1/3",   true,   AwaResourceType_FloatArray},
          detail::TestGetResourceArray {AwaError_PathNotFound,           "/10000/1/4",   true,   AwaResourceType_BooleanArray},
          detail::TestGetResourceArray {AwaError_PathNotFound,           "/10000/1/5",   true,   AwaResourceType_OpaqueArray},
          detail::TestGetResourceArray {AwaError_PathNotFound,           "/10000/1/6",   true,   AwaResourceType_TimeArray},
          detail::TestGetResourceArray {AwaError_PathNotFound,            "/10000/1/7",   true,   AwaResourceType_ObjectLinkArray}
        ));

//TODO AwaError_TypeMismatch
//TODO AwaError_NotDefined

//template <typename T>
//class AwaClientGetResponse_GetValuesAsArrayPointer
//{
//
//};
//
//TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_GetValuesAsStringArrayPointer_handles_null_response)
//{
//
//}
//
//TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_GetValuesAsStringArrayPointer_handles_null_path)
//{
//
//}
//
//TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_GetValuesAsStringArrayPointer_handles_null_array)
//{
//
//}
//
//TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_GetValuesAsStringArrayPointer_handles_invalid_type)
//{
//
//}


TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_GetValuesAsIntegerArrayPointer_handles_null_response)
{
    const AwaIntegerArray * array = NULL;
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientGetResponse_GetValuesAsIntegerArrayPointer(NULL, "/3/0/6", &array));
    ASSERT_TRUE(NULL == array);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_GetValuesAsIntegerArrayPointer_handles_null_path)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/6"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);

    const AwaIntegerArray * array = NULL;
    ASSERT_EQ(AwaError_PathInvalid, AwaClientGetResponse_GetValuesAsIntegerArrayPointer(getResponse, NULL, &array));

    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_GetValuesAsIntegerArrayPointer_handles_null_array)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/6"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);

    ASSERT_EQ(AwaError_OperationInvalid, AwaClientGetResponse_GetValuesAsIntegerArrayPointer(getResponse, "/3/0/6", NULL));

    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_GetValuesAsIntegerArrayPointer_handles_invalid_type)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);

    const AwaIntegerArray * array = NULL;
    ASSERT_EQ(AwaError_TypeMismatch, AwaClientGetResponse_GetValuesAsIntegerArrayPointer(getResponse, "/3/0/1", &array));

    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestGetOperationWithConnectedSession, AwaClientGetResponse_GetValuesAsIntegerArrayPointer_handles_valid_operation)
{
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/6"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);

    const AwaIntegerArray * array = NULL;
    ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsIntegerArrayPointer(getResponse, "/3/0/6", &array));
    ASSERT_TRUE(NULL != array);

    AwaIntegerArrayIterator * iterator = AwaIntegerArray_NewIntegerArrayIterator(array);
    ASSERT_TRUE(AwaIntegerArrayIterator_Next(iterator));
    ASSERT_EQ(1, AwaIntegerArrayIterator_GetValue(iterator));
    ASSERT_TRUE(AwaIntegerArrayIterator_Next(iterator));
    ASSERT_EQ(5, AwaIntegerArrayIterator_GetValue(iterator));
    ASSERT_FALSE(AwaIntegerArrayIterator_Next(iterator));
    AwaIntegerArrayIterator_Free(&iterator);

    AwaClientGetOperation_Free(&getOperation);
}

} // namespace Awa

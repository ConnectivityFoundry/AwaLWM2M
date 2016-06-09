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

#include "awa/client.h"
#include "support/support.h"

namespace Awa {

class TestClientDeleteOperation : public TestClientBase {};

class TestClientDeleteOperationWithConnectedSession : public TestClientWithConnectedSession {};

// TODO: refactor common New/Free tests into type-parameterised tests

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_New_returns_valid_operation_and_free_works)
{
    // test that AwaClientDeleteOperation_Free works via valgrind
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_Free(&operation));
}

TEST_F(TestClientDeleteOperation, AwaClientDeleteOperation_New_handles_null_session)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(NULL);
    ASSERT_EQ(NULL, operation);
}

TEST_F(TestClientDeleteOperation, AwaClientDeleteOperation_New_handles_invalid_session)
{
    // An invalid session is one that is not connected
    AwaClientSession * session = AwaClientSession_New();
    EXPECT_EQ(NULL, AwaClientDeleteOperation_New(session));

    // set up IPC - still not enough
    AwaClientSession_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort);
    EXPECT_EQ(NULL, AwaClientDeleteOperation_New(session));

    AwaClientSession_Free(&session);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_Free_nulls_pointer)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_Free(&operation));
    ASSERT_EQ(NULL, operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_Free_handles_null)
{
    EXPECT_EQ(AwaError_OperationInvalid, AwaClientDeleteOperation_Free(NULL));
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_Free_handles_null_pointer)
{
    AwaClientDeleteOperation * operation = NULL;
    EXPECT_EQ(AwaError_OperationInvalid, AwaClientDeleteOperation_Free(&operation));
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_Free_handles_invalid_session)
{
    // Session is freed before the operation (use a local session):
    AwaClientSession * session = AwaClientSession_New();
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(session));
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session);
    AwaClientSession_Free(&session);
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_Free(&operation));
    // expect no crash or memory leaks
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_AddPath_handles_null_operation)
{
    EXPECT_EQ(AwaError_OperationInvalid, AwaClientDeleteOperation_AddPath(NULL, "/1/2/3"));
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_AddPath_handles_null_path)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    EXPECT_EQ(AwaError_PathInvalid, AwaClientDeleteOperation_AddPath(operation, NULL));
    AwaClientDeleteOperation_Free(&operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_AddPath_handles_invalid_path)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    EXPECT_EQ(AwaError_PathInvalid, AwaClientDeleteOperation_AddPath(operation, "/an/invalid/path"));
    AwaClientDeleteOperation_Free(&operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_AddPath_handles_valid_path)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/3/0/0"));
    AwaClientDeleteOperation_Free(&operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_AddPath_handles_dropped_paths)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/3/0"));
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/3/0/0"));
    AwaClientDeleteOperation_Free(&operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_AddPath_handles_more_general_path)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_); ASSERT_TRUE(NULL != operation);
    ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/3/0/0"));
    ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/3/0"));
    ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_Perform(operation, global::timeout));
    AwaClientDeleteOperation_Free(&operation);

    // check the result - expect not found for /3/0
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0"));
    ASSERT_EQ(AwaError_Response, AwaClientGetOperation_Perform(getOperation, global::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation); ASSERT_TRUE(NULL != getResponse);
    AwaPathIterator * iterator = AwaClientGetResponse_NewPathIterator(getResponse);
    ASSERT_TRUE(NULL != iterator);

    EXPECT_TRUE(AwaPathIterator_Next(iterator));
    EXPECT_STREQ("/3/0", AwaPathIterator_Get(iterator));
    EXPECT_FALSE(AwaPathIterator_Next(iterator));

    const AwaPathResult * result = AwaClientGetResponse_GetPathResult(getResponse, "/3/0");
    ASSERT_TRUE(NULL != result);
    EXPECT_EQ(AwaError_PathNotFound, AwaPathResult_GetError(result));

    AwaPathIterator_Free(&iterator);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_AddPathWithArrayRange_handles_null_operation)
{
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientDeleteOperation_AddPathWithArrayRange(NULL, "/3/0/6", 0, 1));
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_AddPathWithArrayRange_handles_null_path)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    ASSERT_EQ(AwaError_PathInvalid, AwaClientDeleteOperation_AddPathWithArrayRange(operation, NULL, 0, 1));
    AwaClientDeleteOperation_Free(&operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_AddPathWithArrayRange_handles_invalid_path)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    ASSERT_EQ(AwaError_PathInvalid, AwaClientDeleteOperation_AddPathWithArrayRange(operation, "/inv.alid/path///", 0, 1));

    // Object Instance paths are not valid:
    ASSERT_EQ(AwaError_PathInvalid, AwaClientDeleteOperation_AddPathWithArrayRange(operation, "/3/0", 0, 1));

    // Object paths are not valid:
    ASSERT_EQ(AwaError_PathInvalid, AwaClientDeleteOperation_AddPathWithArrayRange(operation, "/3", 0, 1));

    AwaClientDeleteOperation_Free(&operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, DISABLED_AwaClientDeleteOperation_AddPathWithArrayRange_handles_invalid_start)
{
    // TODO: not sure what constitutes an invalid startIndex
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_AddPathWithArrayRange_handles_invalid_count)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    // Zero is an invalid count value:
    ASSERT_EQ(AwaError_AddInvalid, AwaClientDeleteOperation_AddPathWithArrayRange(operation, "/3/0/6", 0, 0));

    // TODO: consider size_t overflow case:
    ASSERT_EQ(AwaError_AddInvalid, AwaClientDeleteOperation_AddPathWithArrayRange(operation, "/3/0/6", SIZE_MAX, 1));

    AwaClientDeleteOperation_Free(&operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_AddPathWithArrayRange_handles_single_resource_instance)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPathWithArrayRange(operation, "/3/0/6", 0, 1));
    AwaClientDeleteOperation_Free(&operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_AddPathWithArrayRange_handles_multiple_resource_instances)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPathWithArrayRange(operation, "/3/0/6", 0, 2));
    AwaClientDeleteOperation_Free(&operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_AddPathWithArrayRange_handles_valid_inputs_multiple_resources)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPathWithArrayRange(operation, "/3/0/6", 0, 2));
    ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPathWithArrayRange(operation, "/3/0/7", 0, 2));
    ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPathWithArrayRange(operation, "/3/0/8", 0, 2));
    AwaClientDeleteOperation_Free(&operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, DISABLED_AwaClientDeleteOperation_AddPathWithArrayRange_handles_multiple_ranges_on_same_resource)
{
    // TODO
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_Perform_handles_null_operation)
{
    EXPECT_EQ(AwaError_OperationInvalid, AwaClientDeleteOperation_Perform(NULL, global::timeout));
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_Perform_handles_empty_operation)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    // no paths added
    EXPECT_EQ(AwaError_OperationInvalid, AwaClientDeleteOperation_Perform(operation, global::timeout));
    AwaClientDeleteOperation_Free(&operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_Perform_handles_single_add_operation)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/3/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_Perform(operation, global::timeout));
    AwaClientDeleteOperation_Free(&operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_Perform_twice_fails)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/3/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_Perform(operation, global::timeout));
    EXPECT_EQ(AwaError_Response, AwaClientDeleteOperation_Perform(operation, global::timeout));
    const AwaClientDeleteResponse * response = AwaClientDeleteOperation_GetResponse(operation);
    const AwaPathResult * pathResult = AwaClientDeleteResponse_GetPathResult(response, "/3/0/0");
    EXPECT_EQ(AwaError_PathNotFound, AwaPathResult_GetError(pathResult));
    AwaClientDeleteOperation_Free(&operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_Perform_handles_multiple_deletes_on_one_object_instance)
{
    // assume these resources exist by default!
    std::vector<const char *> paths { "/3/0/0", "/3/0/1", "/3/0/9" };

    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);

    for (auto it = paths.begin(); it != paths.end(); ++it)
    {
        ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, *it));
    }
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_Perform(operation, global::timeout));
    AwaClientDeleteOperation_Free(&operation);

    // try to delete them again, one at a time, to check for expected failure
    for (auto it = paths.begin(); it != paths.end(); ++it)
    {
        operation = AwaClientDeleteOperation_New(session_);
        ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, *it));
        EXPECT_EQ(AwaError_Response, AwaClientDeleteOperation_Perform(operation, global::timeout));

        const AwaClientDeleteResponse * response = AwaClientDeleteOperation_GetResponse(operation);
        const AwaPathResult * pathResult = AwaClientDeleteResponse_GetPathResult(response, *it);
        EXPECT_EQ(AwaError_PathNotFound, AwaPathResult_GetError(pathResult));

        AwaClientDeleteOperation_Free(&operation);
    }
}

TEST_F(TestClientDeleteOperationWithConnectedSession, DISABLED_AwaClientDeleteOperation_Perform_handles_multiple_deletes_on_multiple_object_instances)
{
    // TODO: create new instances of /3:
    // CreateInstance("/3/7")
    // CreateInstance("/3/8")

    // assume these resources exist by default!
    std::vector<const char *> paths { "/3/0/0", "/3/7/1", "/3/8/9" };

    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);

    for (auto it = paths.begin(); it != paths.end(); ++it)
    {
        ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, *it));
    }
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_Perform(operation, global::timeout));
    AwaClientDeleteOperation_Free(&operation);

    // try to delete them again, one at a time, to check for expected failure
    for (auto it = paths.begin(); it != paths.end(); ++it)
    {
        operation = AwaClientDeleteOperation_New(session_);
        ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, *it));
        EXPECT_EQ(AwaError_CannotDelete, AwaClientDeleteOperation_Perform(operation, global::timeout));
        AwaClientDeleteOperation_Free(&operation);
    }
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_Perform_handles_multiple_deletes_on_multiple_objects)
{
    // assume these resources exist by default!
    std::vector<const char *> paths { "/3/0/0", "/4/0/0", "/2/0/0" };

    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);

    for (auto it = paths.begin(); it != paths.end(); ++it)
    {
        ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, *it));
    }
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_Perform(operation, global::timeout));
    AwaClientDeleteOperation_Free(&operation);

    // try to delete them again, one at a time, to check for expected failure
    for (auto it = paths.begin(); it != paths.end(); ++it)
    {
        operation = AwaClientDeleteOperation_New(session_);
        ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, *it));
        EXPECT_EQ(AwaError_Response, AwaClientDeleteOperation_Perform(operation, global::timeout));

        const AwaClientDeleteResponse * response = AwaClientDeleteOperation_GetResponse(operation);
        const AwaPathResult * pathResult = AwaClientDeleteResponse_GetPathResult(response, *it);
        EXPECT_EQ(AwaError_PathNotFound, AwaPathResult_GetError(pathResult));

        AwaClientDeleteOperation_Free(&operation);
    }
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_Perform_handles_valid_array_range)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPathWithArrayRange(operation, "/3/0/6", 0, 2));
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_Perform(operation, global::timeout));
    AwaClientDeleteOperation_Free(&operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_Perform_handles_split_array_range)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPathWithArrayRange(operation, "/3/0/6", 0, 2));
    ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPathWithArrayRange(operation, "/3/0/6", 10, 2));
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_Perform(operation, global::timeout));
    AwaClientDeleteOperation_Free(&operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, DISABLED_AwaClientDeleteOperation_Perform_handles_split_array_range_over_object_instances)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPathWithArrayRange(operation, "/3/0/6", 0, 2));
    ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPathWithArrayRange(operation, "/3/1/6", 0, 2));
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_Perform(operation, global::timeout));
    AwaClientDeleteOperation_Free(&operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_Perform_handles_split_array_range_over_single_object_instance)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPathWithArrayRange(operation, "/3/0/1", 0, 2));
    ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPathWithArrayRange(operation, "/3/0/2", 0, 2));
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_Perform(operation, global::timeout));
    AwaClientDeleteOperation_Free(&operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_Perform_handles_split_array_range_over_objects)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPathWithArrayRange(operation, "/3/0/1", 0, 2));
    ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPathWithArrayRange(operation, "/4/0/1", 0, 2));
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_Perform(operation, global::timeout));
    AwaClientDeleteOperation_Free(&operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, DISABLED_AwaClientDeleteOperation_Perform_handles_combined)
{
    // Test combined AddPath and AddPathWithArrayRange operations
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_Perform_handles_negative_timeout)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/3/0/0"));
    EXPECT_EQ(AwaError_OperationInvalid, AwaClientDeleteOperation_Perform(operation, -1));
    AwaClientDeleteOperation_Free(&operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, DISABLED_DA_AwaClientDeleteOperation_AddPath_resource_path_does_not_shadow_instance_path)
{
    // Adding /3/0/0 after adding /3/0 should not disable the delete of /3/0
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/3/0"));
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/3/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_Perform(operation, global::timeout));
    AwaClientDeleteOperation_Free(&operation);

    // check that the correct resources were deleted
    {
        AwaClientGetOperation * operation = AwaClientGetOperation_New(session_);
        AwaClientGetOperation_AddPath(operation, "/3/0");
        ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(operation, global::timeout));
        const AwaClientGetResponse * response = AwaClientGetOperation_GetResponse(operation);
        AwaPathIterator * iterator = AwaClientGetResponse_NewPathIterator(response);

        // the response should be empty!
        EXPECT_TRUE(false == AwaPathIterator_Next(iterator));

        AwaPathIterator_Free(&iterator);
        AwaClientGetOperation_Free(&operation);
    }
}

TEST_F(TestClientDeleteOperationWithConnectedSession, DISABLED_DA_AwaClientDeleteOperation_AddPath_instance_path_does_not_shadow_object_path)
{
    // define Object 888 for this test
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_); ASSERT_TRUE(NULL != defineOperation);
    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(888, "Test Object 888", 0, 1); ASSERT_TRUE(NULL != objectDefinition);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefinition, 1, "Test Resource Integer", false, AwaResourceOperations_ReadWrite, 0));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, objectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, 5));
    AwaObjectDefinition_Free(&objectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    // create an instance of 888
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/888/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, 5));
    AwaClientSetOperation_Free(&setOperation);

    // Adding /888/0 after adding /888/0 should not disable the delete of /888
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/888"));
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/888/0"));
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_Perform(operation, 5));
    AwaClientDeleteOperation_Free(&operation);

    // check that the correct resources were deleted
    {
        AwaClientGetOperation * operation = AwaClientGetOperation_New(session_);
        AwaClientGetOperation_AddPath(operation, "/888");
        ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(operation, global::timeout));
        const AwaClientGetResponse * response = AwaClientGetOperation_GetResponse(operation);
        AwaPathIterator * iterator = AwaClientGetResponse_NewPathIterator(response);

        // the response should be empty!
        EXPECT_TRUE(false == AwaPathIterator_Next(iterator));

        AwaPathIterator_Free(&iterator);
        AwaClientGetOperation_Free(&operation);
    }
}

TEST_F(TestClientDeleteOperationWithConnectedSession, DISABLED_DA_AwaClientDeleteOperation_AddPath_handles_complex_operation)
{
    // define Objects for this test
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_); ASSERT_TRUE(NULL != defineOperation);
    {
        // define Object 888 for this test
        AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(888, "Test Object 888", 0, 1); ASSERT_TRUE(NULL != objectDefinition);
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefinition, 0, "Test Resource Integer 0", false, AwaResourceOperations_ReadWrite, 0));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefinition, 1, "Test Resource Integer 1", false, AwaResourceOperations_ReadWrite, 0));
        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, objectDefinition));
        AwaObjectDefinition_Free(&objectDefinition);
    }
    {
        // define Object 889 for this test
        AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(889, "Test Object 889", 0, 1); ASSERT_TRUE(NULL != objectDefinition);
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefinition, 0, "Test Resource Integer 0", false, AwaResourceOperations_ReadWrite, 0));
        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, objectDefinition));
        AwaObjectDefinition_Free(&objectDefinition);
    }
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, 5));
    AwaClientDefineOperation_Free(&defineOperation);

    // create two instances of 888 and 889
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/888/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/888/1"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/889/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/889/1"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, 5));
    AwaClientSetOperation_Free(&setOperation);

    // Delete /888/0/0, /888/1, /889, should leave /888/0/1 remaining

    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/888/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/888/1/0"));  // check that /888/1 removes this!
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/888/1"));
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/889"));
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/889/0/0"));  // check that /889 causes this to be dropped
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_Perform(operation, 5));
    AwaClientDeleteOperation_Free(&operation);

    // check that the correct resources were deleted in 888
    {
        AwaClientGetOperation * operation = AwaClientGetOperation_New(session_);
        AwaClientGetOperation_AddPath(operation, "/888");
        AwaClientGetOperation_AddPath(operation, "/889");
        ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(operation, global::timeout));
        const AwaClientGetResponse * response = AwaClientGetOperation_GetResponse(operation);
        AwaPathIterator * iterator = AwaClientGetResponse_NewPathIterator(response);

        EXPECT_TRUE(AwaPathIterator_Next(iterator));
        EXPECT_STREQ("/888/0/0", AwaPathIterator_Get(iterator));

        EXPECT_FALSE(AwaPathIterator_Next(iterator));

        AwaPathIterator_Free(&iterator);
        AwaClientGetOperation_Free(&operation);
    }
}

TEST_F(TestClientDeleteOperationWithConnectedSession, DISABLED_AwaClientDeleteOperation_Perform_handles_zero_timeout)
{
    // how?
}

TEST_F(TestClientDeleteOperationWithConnectedSession, DISABLED_AwaClientDeleteOperation_Perform_handles_short_timeout)
{
    // how?
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_GetResponse_handles_null_operation)
{
    EXPECT_EQ(NULL, AwaClientDeleteOperation_GetResponse(NULL));
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteOperation_GetResponse_handles_valid_operation)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/4/0"));
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_Perform(operation, global::timeout));

    EXPECT_TRUE(NULL != operation);
    EXPECT_TRUE(NULL != AwaClientDeleteOperation_GetResponse(operation));
    AwaClientDeleteOperation_Free(&operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteResponse_NewPathIterator_handles_null_response)
{
    EXPECT_EQ(NULL, AwaClientDeleteResponse_NewPathIterator(NULL));
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteResponse_NewPathIterator_handles_single_path_response)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/4/0"));
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_Perform(operation, global::timeout));
    const AwaClientDeleteResponse * response = AwaClientDeleteOperation_GetResponse(operation);
    ASSERT_TRUE(NULL != response);

    AwaPathIterator * iterator = AwaClientDeleteResponse_NewPathIterator(response);
    ASSERT_TRUE(NULL != iterator);

    // should be one path
    EXPECT_FALSE(AwaPathIterator_Get(iterator));
    EXPECT_TRUE(AwaPathIterator_Next(iterator));
    EXPECT_STREQ("/4/0", AwaPathIterator_Get(iterator));
    EXPECT_FALSE(AwaPathIterator_Next(iterator));
    AwaPathIterator_Free(&iterator);

    AwaClientDeleteOperation_Free(&operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteResponse_NewPathIterator_handles_multiple_path_response)
{
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/3/0"));
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/4/0"));
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/5/0"));
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_Perform(operation, global::timeout));
    const AwaClientDeleteResponse * response = AwaClientDeleteOperation_GetResponse(operation);
    ASSERT_TRUE(NULL != response);

    AwaPathIterator * iterator = AwaClientDeleteResponse_NewPathIterator(response);
    ASSERT_TRUE(NULL != iterator);

    // should be three paths
    std::vector<std::string> expectedPaths = { "/3/0", "/4/0", "/5/0" };
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
    AwaClientDeleteOperation_Free(&operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteResponse_GetPathResult_handles_success)
{
    // now try to Delete the mandatory resource:
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/4/0"));
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_Perform(operation, global::timeout));

    // check response on /3/0/11
    const AwaClientDeleteResponse * response = AwaClientDeleteOperation_GetResponse(operation);
    EXPECT_TRUE(NULL != response);

    const AwaPathResult * pathResult = AwaClientDeleteResponse_GetPathResult(response, "/4/0");
    EXPECT_TRUE(NULL != pathResult);
    EXPECT_EQ(AwaError_Success, AwaPathResult_GetError(pathResult));

    AwaClientDeleteOperation_Free(&operation);
}

TEST_F(TestClientDeleteOperationWithConnectedSession, AwaClientDeleteResponse_GetPathResult_handles_error)
{
    // now try to Delete the mandatory resource:
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, "/3132/0"));
    EXPECT_EQ(AwaError_Response, AwaClientDeleteOperation_Perform(operation, global::timeout));

    // check response on /3/0/11
    const AwaClientDeleteResponse * response = AwaClientDeleteOperation_GetResponse(operation);
    EXPECT_TRUE(NULL != response);

    const AwaPathResult * pathResult = AwaClientDeleteResponse_GetPathResult(response, "/3132/0");
    EXPECT_TRUE(NULL != pathResult);
    EXPECT_EQ(AwaError_PathNotFound, AwaPathResult_GetError(pathResult));

    AwaClientDeleteOperation_Free(&operation);
}

} // namespace Awa

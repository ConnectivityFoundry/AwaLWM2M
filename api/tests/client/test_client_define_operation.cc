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
#include "support/support.h"

namespace Awa {

namespace detail {
    const AwaObjectID TEST_OBJECT_ID_VALID = 3;
    const AwaObjectID TEST_OBJECT_ID_INVALID = 65000;

    const AwaObjectID TEST_TEMPORARY_OBJECT_ID = 9999;

    const int TEST_MULTIPLE_INSTANCES_MAX = 20;
} // namespace detail

class TestClientDefineOperationWithConnectedSession : public TestClientWithConnectedSession {};

TEST_F(TestClientDefineOperationWithConnectedSession, AwaClientSession_GetObjectDefinition_handles_valid_inputs)
{
    const AwaObjectDefinition * definition = AwaClientSession_GetObjectDefinition(session_, detail::TEST_OBJECT_ID_VALID);
    ASSERT_TRUE(NULL != definition);
}

TEST_F(TestClientDefineOperationWithConnectedSession, AwaClientSession_GetObjectDefinition_handles_null_session)
{
    const AwaObjectDefinition * definition = AwaClientSession_GetObjectDefinition(NULL, detail::TEST_OBJECT_ID_VALID);
    ASSERT_TRUE(NULL == definition);
}

TEST_F(TestClientDefineOperationWithConnectedSession, AwaClientSession_GetObjectDefinition_handles_invalid_session)
{
    // An invalid session is one that is not connected
    AwaClientSession * session = AwaClientSession_New();
    const AwaObjectDefinition * definition = AwaClientSession_GetObjectDefinition(session, detail::TEST_OBJECT_ID_VALID);
    ASSERT_TRUE(NULL == definition);
    AwaClientSession_Free(&session);
}

TEST_F(TestClientDefineOperationWithConnectedSession, AwaClientSession_GetObjectDefinition_handles_invalid_object_ID)
{
    const AwaObjectDefinition * definition = AwaClientSession_GetObjectDefinition(session_, detail::TEST_OBJECT_ID_INVALID);
    ASSERT_EQ(NULL, definition);
}

TEST_F(TestClientDefineOperationWithConnectedSession, AwaObjectDefinition_GetResourceDefinition_handles_valid_inputs)
{
    const AwaObjectDefinition * definition = AwaClientSession_GetObjectDefinition(session_, detail::TEST_OBJECT_ID_VALID);
    EXPECT_TRUE(NULL != definition);

    const AwaResourceDefinition * resourceDefinition = AwaObjectDefinition_GetResourceDefinition(definition, 0);
    ASSERT_TRUE(NULL != resourceDefinition);
}

TEST_F(TestClientDefineOperationWithConnectedSession, AwaObjectDefinition_GetResourceDefinition_handles_null_object_definition)
{
    const AwaResourceDefinition * resourceDefinition = AwaObjectDefinition_GetResourceDefinition(NULL, 0);
    ASSERT_EQ(NULL, resourceDefinition);
}

TEST_F(TestClientDefineOperationWithConnectedSession, AwaObjectDefinition_GetResourceDefinition_handles_invalid_resource_ID)
{
    const AwaObjectDefinition * definition = AwaClientSession_GetObjectDefinition(session_, detail::TEST_OBJECT_ID_VALID);
    EXPECT_TRUE(NULL != definition);

    const AwaResourceDefinition * resourceDefinition = AwaObjectDefinition_GetResourceDefinition(definition, AWA_MAX_ID);
    ASSERT_EQ(NULL, resourceDefinition);
}

TEST_F(TestClientDefineOperationWithConnectedSession, AwaResourceDefinition_GetType_handles_valid_inputs)
{
    const AwaObjectDefinition * definition = AwaClientSession_GetObjectDefinition(session_, detail::TEST_OBJECT_ID_VALID);
    EXPECT_TRUE(NULL != definition);

    const AwaResourceDefinition * resourceDefinition = AwaObjectDefinition_GetResourceDefinition(definition, 0);
    EXPECT_TRUE(NULL != resourceDefinition);

    ASSERT_EQ(AwaResourceType_String, AwaResourceDefinition_GetType(resourceDefinition));
}

TEST_F(TestClientDefineOperationWithConnectedSession, AwaResourceDefinition_GetType_handles_null_resource_definition)
{
    ASSERT_EQ(AwaResourceType_Invalid, AwaResourceDefinition_GetType(NULL));
}

TEST_F(TestClientDefineOperationWithConnectedSession, AwaClientSession_GetObjectDefinition_device_object_valid)
{
    const AwaObjectDefinition * definition = AwaClientSession_GetObjectDefinition(session_, detail::TEST_OBJECT_ID_VALID);
    EXPECT_TRUE(NULL != definition);

    ASSERT_EQ(1, AwaObjectDefinition_GetMaximumInstances(definition));
    ASSERT_EQ(1, AwaObjectDefinition_GetMinimumInstances(definition));
}

TEST_F(TestClientDefineOperationWithConnectedSession, AwaClientDefineOperation_New_and_Free_handles_valid_inputs)
{
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    ASSERT_TRUE(defineOperation != NULL);

    const AwaObjectDefinition * definition = AwaClientSession_GetObjectDefinition(session_, detail::TEST_OBJECT_ID_VALID);
    EXPECT_TRUE(NULL != definition);

    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, definition));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Free(&defineOperation));
}

TEST_F(TestClientDefineOperationWithConnectedSession, AwaClientDefineOperation_Free_nulls_pointer)
{
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Free(&defineOperation));
    ASSERT_EQ(NULL, defineOperation);
}

TEST_F(TestClientDefineOperationWithConnectedSession, AwaClientDefineOperation_Free_handles_null)
{
    EXPECT_EQ(AwaError_OperationInvalid, AwaClientDefineOperation_Free(NULL));
}

TEST_F(TestClientDefineOperationWithConnectedSession, AwaClientDefineOperation_Process_handles_valid_inputs)
{
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    AwaObjectDefinition * definition = AwaObjectDefinition_New(detail::TEST_TEMPORARY_OBJECT_ID, "Test Object", 1, 1);
    EXPECT_TRUE(NULL != definition);

    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsNoType(definition, 1, "Test Resource No Type", false, AwaResourceOperations_Execute));

    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, definition));

    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, global::timeout));

    AwaObjectDefinition_Free(&definition);
    AwaClientDefineOperation_Free(&defineOperation);
}

TEST_F(TestClientDefineOperationWithConnectedSession, AwaClientDefineOperation_Process_handles_duplicate_object_id)
{
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    ASSERT_TRUE(defineOperation != NULL);

    AwaObjectDefinition * objectDefinition0 = AwaObjectDefinition_New(1000, "Test Object 0", 0, 1);
    ASSERT_TRUE(objectDefinition0 != NULL);
    AwaObjectDefinition * objectDefinition1 = AwaObjectDefinition_New(1000, "Test Object 1", 0, 1);
    ASSERT_TRUE(objectDefinition1 != NULL);

    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, objectDefinition0));
    ASSERT_EQ(AwaError_DefinitionInvalid, AwaClientDefineOperation_Add(defineOperation, objectDefinition1));

    AwaObjectDefinition_Free(&objectDefinition0);
    AwaObjectDefinition_Free(&objectDefinition1);
    AwaClientDefineOperation_Free(&defineOperation);
}

TEST_F(TestClientDefineOperationWithConnectedSession, AwaClientDefineOperation_Process_handles_adding_object_definition_twice)
{
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    ASSERT_TRUE(defineOperation != NULL);

    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(1000, "Test Object 0", 0, 1);
    ASSERT_TRUE(objectDefinition != NULL);

    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, objectDefinition));
    ASSERT_EQ(AwaError_DefinitionInvalid, AwaClientDefineOperation_Add(defineOperation, objectDefinition));

    AwaObjectDefinition_Free(&objectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);
}

TEST_F(TestClientDefineOperationWithConnectedSession, AwaClientDefineOperation_Process_handles_negative_timeout)
{
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_EQ(AwaError_OperationInvalid, AwaClientDefineOperation_Perform(defineOperation, -1));
    AwaClientDefineOperation_Free(&defineOperation);
}

TEST_F(TestClientDefineOperationWithConnectedSession, AwaObjectDefinitionIterator_Can_iterate_valid_inputs)
{
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    AwaObjectDefinition * definition = AwaObjectDefinition_New(5001, "Object1", 1, 1);
    EXPECT_TRUE(NULL != definition);
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, definition));

    AwaObjectDefinition * definition2 = AwaObjectDefinition_New(5002, "Object2", 1, 1);
    EXPECT_TRUE(NULL != definition2);
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, definition2));

    AwaObjectDefinition * definition3 = AwaObjectDefinition_New(5003, "Object3", 1, 1);
    EXPECT_TRUE(NULL != definition3);
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, definition3));

    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, global::timeout));

    AwaObjectDefinition_Free(&definition);
    AwaObjectDefinition_Free(&definition2);
    AwaObjectDefinition_Free(&definition3);
    AwaClientDefineOperation_Free(&defineOperation);

    // create iterator
    AwaObjectDefinitionIterator * iterator = AwaClientSession_NewObjectDefinitionIterator(session_);
    ASSERT_TRUE(NULL != iterator);

    const AwaObjectDefinition * objectDefinition;

    // As we have no idea how many objects are registered already, loop up to 20 times to find object "5001"
    for (int i = 0; i < 20; i++)
    {
        ASSERT_EQ(true, AwaObjectDefinitionIterator_Next(iterator));
        objectDefinition = AwaObjectDefinitionIterator_Get(iterator);
        EXPECT_TRUE(NULL != objectDefinition);
        if (5001 == AwaObjectDefinition_GetID(objectDefinition))
            break;
    }

    ASSERT_EQ(true, AwaObjectDefinitionIterator_Next(iterator));
    objectDefinition = AwaObjectDefinitionIterator_Get(iterator);
    EXPECT_TRUE(NULL != objectDefinition);
    ASSERT_EQ(5002, AwaObjectDefinition_GetID(objectDefinition));

    ASSERT_EQ(true, AwaObjectDefinitionIterator_Next(iterator));
    objectDefinition = AwaObjectDefinitionIterator_Get(iterator);
    EXPECT_TRUE(NULL != objectDefinition);
    ASSERT_EQ(5003, AwaObjectDefinition_GetID(objectDefinition));

    ASSERT_EQ(0, AwaObjectDefinitionIterator_Next(iterator));
    objectDefinition = AwaObjectDefinitionIterator_Get(iterator);
    EXPECT_TRUE(NULL == objectDefinition);

    EXPECT_TRUE(NULL == objectDefinition);

    AwaObjectDefinitionIterator_Free(&iterator);
    ASSERT_TRUE(NULL == iterator);
}

//FIXME: FLOWDM-360
TEST_F(TestClientDefineOperationWithConnectedSession, DISABLED_AwaClientDefineOperation_Process_handles_zero_timeout)
{
    // how?
}

//FIXME: FLOWDM-360
TEST_F(TestClientDefineOperationWithConnectedSession, DISABLED_AwaClientDefineOperation_Process_handles_short_timeout)
{
    // how?
}

//FIXME: FLOWDM-370
TEST_F(TestClientDefineOperationWithConnectedSession, DISABLED_AwaClientDefineOperation_Process_handles_freed_session)
{
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    AwaObjectDefinition * definition = AwaObjectDefinition_New(detail::TEST_TEMPORARY_OBJECT_ID, "Test Object", 1, 1);
    EXPECT_TRUE(NULL != definition);

    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsNoType(definition, 1, "Test Resource No Type", false, AwaResourceOperations_Execute));

    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, definition));

    EXPECT_EQ(AwaError_Success, AwaClientSession_Free(&session_));

    ASSERT_EQ(AwaError_SessionInvalid, AwaClientDefineOperation_Perform(defineOperation, global::timeout));

    AwaObjectDefinition_Free(&definition);
    AwaClientDefineOperation_Free(&defineOperation);
}

TEST_F(TestClientDefineOperationWithConnectedSession, AwaClientDefineOperation_Process_handles_null_operation)
{
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientDefineOperation_Perform(NULL, global::timeout));
}

TEST_F(TestClientDefineOperationWithConnectedSession, AwaClientDefineOperation_Process_handles_disconnected_session)
{
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_); EXPECT_TRUE(defineOperation != NULL);
    AwaObjectDefinition * definition = AwaObjectDefinition_New(detail::TEST_TEMPORARY_OBJECT_ID, "Test Object", 1, 1); EXPECT_TRUE(NULL != definition);

    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsNoType(definition, 1, "Test Resource No Type", false, AwaResourceOperations_Execute));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, definition));

    // disconnect the session
    EXPECT_EQ(AwaError_Success, AwaClientSession_Disconnect(session_));

    ASSERT_EQ(AwaError_SessionNotConnected, AwaClientDefineOperation_Perform(defineOperation, global::timeout));

    AwaObjectDefinition_Free(&definition);
    AwaClientDefineOperation_Free(&defineOperation);
}

TEST_F(TestClientDefineOperationWithConnectedSession, redefining_existing_standard_object_fails)
{
    // define a new object definition for the standard object 3 (Device), with a single integer resource 0:
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);  ASSERT_TRUE(defineOperation != NULL);
    AwaObjectDefinition * definition = AwaObjectDefinition_New(3, "New Device", 1, 1);   ASSERT_TRUE(NULL != definition);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(definition, 0, "New Manufacturer", false, AwaResourceOperations_ReadWrite, 0));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, definition));

    // TODO: this should eventually return AlreadyDefined, but for now will return IPCError
    //ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Process(defineOperation, global::timeout));
    EXPECT_EQ(AwaError_AlreadyDefined, AwaClientDefineOperation_Perform(defineOperation, global::timeout));

    // check that the session's object definition has not changed:
    {
        const AwaObjectDefinition * resultDefinition = AwaClientSession_GetObjectDefinition(session_, 3);
        EXPECT_TRUE(AwaObjectDefinition_IsResourceDefined(resultDefinition, 1));   // resource 1 should still exist
        const AwaResourceDefinition * resource0Definition = AwaObjectDefinition_GetResourceDefinition(resultDefinition, 0);
        EXPECT_EQ(AwaResourceType_String, AwaResourceDefinition_GetType(resource0Definition));  // resource 0 should still be a string
    }

    // disconnect, reconnect to refresh the session's definition data:
    EXPECT_EQ(AwaError_Success, AwaClientSession_Disconnect(session_));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(session_));

    // then check again:
    {
        const AwaObjectDefinition * resultDefinition = AwaClientSession_GetObjectDefinition(session_, 3);
        EXPECT_TRUE(AwaObjectDefinition_IsResourceDefined(resultDefinition, 1));   // resource 1 should still exist
        const AwaResourceDefinition * resource0Definition = AwaObjectDefinition_GetResourceDefinition(resultDefinition, 0);
        EXPECT_EQ(AwaResourceType_String, AwaResourceDefinition_GetType(resource0Definition));  // resource 0 should still be a string
    }

    AwaObjectDefinition_Free(&definition);
    AwaClientDefineOperation_Free(&defineOperation);
}

class TestClientDefineOperationRedefinition : public TestClientDefineOperationWithConnectedSession, public ::testing::WithParamInterface<bool> {};

TEST_P(TestClientDefineOperationRedefinition, redefine_existing_standard_object_with_new_object)
{
    // what happens if we construct two object definitions, the first of which is invalid (duplicate of 3)?

    // define a new object definition for the standard object 3 (Device), with a single integer resource 0:
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);  ASSERT_TRUE(defineOperation != NULL);
    AwaObjectDefinition * definition3 = AwaObjectDefinition_New(3, "New Device", 1, 1);   ASSERT_TRUE(NULL != definition3);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(definition3, 0, "New Manufacturer", false, AwaResourceOperations_ReadWrite, 0));

    // define a new object definition that doesn't match an existing definition:
    AwaObjectDefinition * definition42 = AwaObjectDefinition_New(42, "Valid Test Object", 1, 1); ASSERT_TRUE(NULL != definition42);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(definition42, 0, "Test Resource Integer", false, AwaResourceOperations_ReadWrite, 0));

    if (GetParam())
    {
        // add invalid definition first
        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, definition3));
        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, definition42));
    }
    else
    {
        // add invalid definition second
        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, definition42));
        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, definition3));
    }
    // TODO: other error code may be more appropriate
    //EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Process(defineOperation, global::timeout));
    EXPECT_EQ(AwaError_AlreadyDefined, AwaClientDefineOperation_Perform(defineOperation, global::timeout));

    // check that the session's object definition has not changed for object 3:
    {
        const AwaObjectDefinition * objectDefinition = AwaClientSession_GetObjectDefinition(session_, 3);
        EXPECT_TRUE(AwaObjectDefinition_IsResourceDefined(objectDefinition, 1));   // resource 1 should still exist
        const AwaResourceDefinition * resourceDefinition = AwaObjectDefinition_GetResourceDefinition(objectDefinition, 0);
        EXPECT_EQ(AwaResourceType_String, AwaResourceDefinition_GetType(resourceDefinition));  // resource 0 should still be a string
    }

    // check that the session's object definition has been created for object 42:
    {
        const AwaObjectDefinition * objectDefinition = AwaClientSession_GetObjectDefinition(session_, 42);
        EXPECT_TRUE(AwaObjectDefinition_IsResourceDefined(objectDefinition, 0));
        const AwaResourceDefinition * resourceDefinition = AwaObjectDefinition_GetResourceDefinition(objectDefinition, 0);
        EXPECT_EQ(AwaResourceType_Integer, AwaResourceDefinition_GetType(resourceDefinition));
        EXPECT_STREQ("Test Resource Integer", AwaResourceDefinition_GetName(resourceDefinition));
    }

    // disconnect, reconnect to refresh the session's definition data:
    EXPECT_EQ(AwaError_Success, AwaClientSession_Disconnect(session_));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(session_));

    // then check again:
    {
        const AwaObjectDefinition * objectDefinition = AwaClientSession_GetObjectDefinition(session_, 3);
        EXPECT_TRUE(AwaObjectDefinition_IsResourceDefined(objectDefinition, 1));   // resource 1 should still exist
        const AwaResourceDefinition * resourceDefinition = AwaObjectDefinition_GetResourceDefinition(objectDefinition, 0);
        EXPECT_EQ(AwaResourceType_String, AwaResourceDefinition_GetType(resourceDefinition));  // resource 0 should still be a string
    }
    {
        const AwaObjectDefinition * objectDefinition = AwaClientSession_GetObjectDefinition(session_, 42);
        EXPECT_TRUE(AwaObjectDefinition_IsResourceDefined(objectDefinition, 0));
        const AwaResourceDefinition * resourceDefinition = AwaObjectDefinition_GetResourceDefinition(objectDefinition, 0);
        EXPECT_EQ(AwaResourceType_Integer, AwaResourceDefinition_GetType(resourceDefinition));
        EXPECT_STREQ("Test Resource Integer", AwaResourceDefinition_GetName(resourceDefinition));
    }

    AwaObjectDefinition_Free(&definition3);
    AwaObjectDefinition_Free(&definition42);
    AwaClientDefineOperation_Free(&defineOperation);
}

// TODO: FLOWDM-379 : the 'false' case fails unexpectedly
//INSTANTIATE_TEST_CASE_P(
//        TestClientDefineOperationRedefinition,
//        TestClientDefineOperationRedefinition,
//        ::testing::Values(true, false));

} // namespace Awa

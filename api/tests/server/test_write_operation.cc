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
#include "support/definition.h"

#include "../src/arrays.h"

#include "awa/server.h"
#include "write_mode.h"
#include "log.h"
#include "path.h"

namespace Awa {

class TestWriteOperation : public TestServerBase {};

class TestWriteOperationWithServerDaemon : public TestServerWithDaemonBase {};


class TestWriteOperationWithConnectedSession : public TestServerWithConnectedSession {};
class TestWriteOperationWithConnectedServerAndClientSession : public TestServerAndClientWithConnectedSession {};

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_New_returns_valid_operation_and_free_works)
{
    // test that AwaServerWriteOperation_Free works via valgrind
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Replace);
    ASSERT_TRUE(NULL != writeOperation);
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Free(&writeOperation));
}

TEST_F(TestWriteOperation, AwaServerWriteOperation_New_handles_null_session)
{
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(NULL, AwaWriteMode_Replace);
    ASSERT_EQ(NULL, writeOperation);
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_New_handles_invalid_write_mode)
{
    ASSERT_EQ(NULL, AwaServerWriteOperation_New(session_, (AwaWriteMode)-1));
    ASSERT_EQ(NULL, AwaServerWriteOperation_New(session_, (AwaWriteMode)123));
}

TEST_F(TestWriteOperation, AwaServerWriteOperation_New_handles_invalid_session)
{
    // An invalid session is one that is not connected
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(NULL, AwaServerWriteOperation_New(session, AwaWriteMode_Update));

    // write up IPC - still not enough
    AwaServerSession_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort);
    EXPECT_EQ(NULL, AwaServerWriteOperation_New(session, AwaWriteMode_Replace));

    AwaServerSession_Free(&session);
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_Free_nulls_pointer)
{
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Free(&writeOperation));
    ASSERT_EQ(NULL, writeOperation);
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_Free_handles_null)
{
    EXPECT_EQ(AwaError_OperationInvalid, AwaServerWriteOperation_Free(NULL));
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_Free_handles_null_pointer)
{
    AwaServerWriteOperation * writeOperation = NULL;
    EXPECT_EQ(AwaError_OperationInvalid, AwaServerWriteOperation_Free(&writeOperation));
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_Free_handles_invalid_session)
{
    // Session is freed before the operation (use a local session):
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Replace);
    AwaServerSession_Free(&session_);
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Free(&writeOperation));
    // expect no crash or memory leaks
}

///***********************************************************************************************************
// * AwaServerWriteOperation_AddValueAsInteger tests
// */
//
TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_AddValueAsInteger_handles_valid_integer_resource)
{
    // A valid path refers to a Resource
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Replace); ASSERT_TRUE(NULL != writeOperation);
    AwaInteger value = 123456789;
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/3/0/9", value));
    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_AddValueAsInteger_handles_undefined_resource)
{
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    AwaInteger value = 123456789;
    ASSERT_EQ(AwaError_NotDefined, AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/99/0/9", value));
    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_AddValueAsInteger_handles_valid_path_for_resource_with_wrong_type)
{
    // A valid path refers to a Resource
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    AwaInteger value = 123456789;

    // assume /3/0/0 is a String type
    ASSERT_EQ(AwaError_TypeMismatch, AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/3/0/0", value));
    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperation, AwaServerWriteOperation_AddValueAsInteger_handles_null_operation)
{
    AwaInteger value = 123456789;
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerWriteOperation_AddValueAsInteger(NULL, "1000/0/1", value));
}

TEST_F(TestWriteOperationWithConnectedSession, DISABLED_AwaServerWriteOperation_AddValueAsInteger_handles_invalid_operation)
{
    // Consider if the parent session has been freed early
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Replace);

    // Free the session
    AwaServerSession_Free(&session_);

    // TODO: we need the session to keep track of all operations, and
    // invalidate them when freed so they can detect this situation.
    AwaInteger value = 123456789;
    ASSERT_EQ(AwaError_SessionInvalid, AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/1000/0/1", value));

    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Free(&writeOperation));
    // expect no crash or memory leaks
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_AddValueAsInteger_handles_null_path)
{
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    AwaInteger value = 123456789;
    ASSERT_EQ(AwaError_PathInvalid, AwaServerWriteOperation_AddValueAsInteger(writeOperation, NULL, value));
    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_AddValueAsInteger_handles_invalid_path)
{
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    AwaInteger value = 123456789;
    EXPECT_EQ(AwaError_PathInvalid, AwaServerWriteOperation_AddValueAsInteger(writeOperation, "a/n in/valid/ path", value));
    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_AddValueAsInteger_handles_non_resource_path)
{
    // Paths are valid, but do not refer to resources. WriteOperation requires resource paths only.
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    AwaInteger value = 123456789;
    EXPECT_EQ(AwaError_PathInvalid, AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/0", value));
    EXPECT_EQ(AwaError_PathInvalid, AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/1/12", value));
    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_AddValueAsCString_handles_null_value)
{
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    ASSERT_EQ(AwaError_TypeMismatch, AwaServerWriteOperation_AddValueAsCString(writeOperation, "/3/0/1", NULL));
    AwaServerWriteOperation_Free(&writeOperation);
}

///***********************************************************************************************************
// * AwaServerWriteOperation_ProcessWriteOperation tests
// */

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_Perform_handles_valid_operation)
{
    // start a client
    const char * clientID = "TestClient1";
    AwaClientDaemonHorde horde( { clientID }, 61000);
    sleep(1);      // wait for the client to register with the server

    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    AwaTime value = 123456789;
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsTime(writeOperation, "/3/0/13", value));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, clientID, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_Perform_handles_read_only_resource)
{
    // start a client
    const char * clientID = "TestClient1";
    AwaClientDaemonHorde horde( { clientID }, 61000);
    sleep(1);      // wait for the client to register with the server

    const char * path = "/3/0/9";
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    AwaInteger value = 123456789;
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, path, value));
    EXPECT_EQ(AwaError_Response, AwaServerWriteOperation_Perform(writeOperation, clientID, defaults::timeout));

    // check response - should be method not allowed.

    const AwaServerWriteResponse * response = AwaServerWriteOperation_GetResponse(writeOperation, "TestClient1");
    EXPECT_TRUE(NULL != response);
    const AwaPathResult * pathResult = AwaServerWriteResponse_GetPathResult(response, path);
    EXPECT_EQ(AwaError_LWM2MError, AwaPathResult_GetError(pathResult));
    EXPECT_EQ(AwaLWM2MError_MethodNotAllowed, AwaPathResult_GetLWM2MError(pathResult));

    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperationWithConnectedServerAndClientSession, AwaServerWriteOperation_Perform_handles_write_only_resource)
{
    // should succeed - resource is writable.

    ObjectDescription object = { 1000, "Object1000", 0, 1, {
            ResourceDescription(0, "Resource0", AwaResourceType_Time, 0, 1, AwaResourceOperations_WriteOnly),
        }};
    EXPECT_EQ(AwaError_Success, Define(client_session_, object));
    EXPECT_EQ(AwaError_Success, Define(server_session_, object));

    WaitForClientDefinition(AwaObjectDefinition_GetID(object.GetDefinition()));

    //create the object instance on the client
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(client_session_);
    EXPECT_TRUE(setOperation != NULL);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    AwaTime value = 123456789;
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsTime(writeOperation, "/1000/0/0", value));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperationWithConnectedServerAndClientSession, AwaServerWriteOperation_Perform_put_existing_object_instance_should_succeed)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1,
    {
        ResourceDescription(0, "Resource0", AwaResourceType_Integer, 0, 1, AwaResourceOperations_ReadWrite),
        ResourceDescription(1, "Resource1", AwaResourceType_Integer, 0, 1, AwaResourceOperations_ReadWrite),
    }};
    EXPECT_EQ(AwaError_Success, Define(client_session_, object));
    EXPECT_EQ(AwaError_Success, Define(server_session_, object));

    WaitForClientDefinition(AwaObjectDefinition_GetID(object.GetDefinition()));

    //create the object instance on the client
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(client_session_);
    EXPECT_TRUE(setOperation != NULL);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);


    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Replace); ASSERT_TRUE(NULL != writeOperation);
    AwaInteger value = 123456789;
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/1000/0/0", value));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/1000/0/1", value));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));

    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperationWithConnectedServerAndClientSession, AwaServerWriteOperation_Perform_put_on_mandatory_object_instance_should_succeed)
{
    ObjectDescription object = { 1000, "Object1000", 1, 1,
    {
        ResourceDescription(0, "Resource0", AwaResourceType_Integer, 0, 1, AwaResourceOperations_ReadWrite),
        ResourceDescription(1, "Resource1", AwaResourceType_Integer, 0, 1, AwaResourceOperations_ReadWrite),
    }};
    EXPECT_EQ(AwaError_Success, Define(client_session_, object));
    EXPECT_EQ(AwaError_Success, Define(server_session_, object));

    WaitForClientDefinition(AwaObjectDefinition_GetID(object.GetDefinition()));

    //create the object instance on the client
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(client_session_);
    EXPECT_TRUE(setOperation != NULL);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Replace); ASSERT_TRUE(NULL != writeOperation);
    AwaInteger value = 123456789;
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/1000/0/0", value));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/1000/0/1", value));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));

    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperationWithConnectedServerAndClientSession, AwaServerWriteOperation_Perform_put_on_mandatory_resource_should_succeed)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1,
    {
        ResourceDescription(0, "Resource0", AwaResourceType_Integer, 1, 1, AwaResourceOperations_ReadWrite),
    }};
    EXPECT_EQ(AwaError_Success, Define(client_session_, object));
    EXPECT_EQ(AwaError_Success, Define(server_session_, object));

    WaitForClientDefinition(AwaObjectDefinition_GetID(object.GetDefinition()));

    //create the object instance and mandatory resource on the client
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(client_session_);
    EXPECT_TRUE(setOperation != NULL);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Replace); ASSERT_TRUE(NULL != writeOperation);
    AwaInteger value = 123456789;
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/1000/0/0", value));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));

    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperationWithConnectedServerAndClientSession, AwaServerWriteOperation_Perform_put_with_optional_resource_should_not_create_optional_resource)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1,
    {
        ResourceDescription(0, "MandatoryResource", AwaResourceType_Integer, 1, 1, AwaResourceOperations_ReadWrite),
        ResourceDescription(1, "OptionalResource", AwaResourceType_Integer, 0, 1, AwaResourceOperations_ReadWrite),
    }};
    EXPECT_EQ(AwaError_Success, Define(client_session_, object));
    EXPECT_EQ(AwaError_Success, Define(server_session_, object));

    WaitForClientDefinition(AwaObjectDefinition_GetID(object.GetDefinition()));

    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Replace); ASSERT_TRUE(NULL != writeOperation);
    AwaInteger value = 123456789;
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/1000/0/0", value));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);


    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(client_session_);
    EXPECT_TRUE(getOperation != NULL);
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));

    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    EXPECT_TRUE(getResponse != NULL);

    ASSERT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/1000/0/0"));
    ASSERT_FALSE(AwaClientGetResponse_ContainsPath(getResponse, "/1000/0/1"));

    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestWriteOperationWithConnectedServerAndClientSession, AwaServerWriteOperation_Perform_post_existing_object_instance_should_succeed)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1,
    {
        ResourceDescription(0, "Resource0", AwaResourceType_Integer, 0, 1, AwaResourceOperations_ReadWrite),
        ResourceDescription(1, "Resource1", AwaResourceType_Integer, 0, 1, AwaResourceOperations_ReadWrite),
    }};
    EXPECT_EQ(AwaError_Success, Define(client_session_, object));
    EXPECT_EQ(AwaError_Success, Define(server_session_, object));

    WaitForClientDefinition(AwaObjectDefinition_GetID(object.GetDefinition()));

    //create the object instance on the client
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(client_session_);
    EXPECT_TRUE(setOperation != NULL);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    AwaInteger value = 123456789;
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/1000/0/0", value));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/1000/0/1", value));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));

    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperationWithConnectedServerAndClientSession, AwaServerWriteOperation_Perform_put_non_existent_object_instance_should_fail)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1,
    {
        ResourceDescription(0, "Resource0", AwaResourceType_Integer, 0, 1, AwaResourceOperations_ReadWrite),
    }};
    EXPECT_EQ(AwaError_Success, Define(client_session_, object));
    EXPECT_EQ(AwaError_Success, Define(server_session_, object));

    WaitForClientDefinition(AwaObjectDefinition_GetID(object.GetDefinition()));

    const char * path = "/1000/0/0";
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Replace); ASSERT_TRUE(NULL != writeOperation);
    AwaInteger value = 123456789;
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, path, value));
    EXPECT_EQ(AwaError_Response, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));

    // check response - should be AwaLWM2MError_BadRequest
    const AwaServerWriteResponse * response = AwaServerWriteOperation_GetResponse(writeOperation, global::clientEndpointName);
    EXPECT_TRUE(NULL != response);
    const AwaPathResult * pathResult = AwaServerWriteResponse_GetPathResult(response, path);
    EXPECT_EQ(AwaError_LWM2MError, AwaPathResult_GetError(pathResult));
    EXPECT_EQ(AwaLWM2MError_MethodNotAllowed, AwaPathResult_GetLWM2MError(pathResult));

    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperationWithConnectedServerAndClientSession, AwaServerWriteOperation_Perform_post_non_existent_object_instance_should_fail)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1,
    {
        ResourceDescription(0, "Resource0", AwaResourceType_Integer, 0, 1, AwaResourceOperations_ReadWrite),
    }};
    EXPECT_EQ(AwaError_Success, Define(client_session_, object));
    EXPECT_EQ(AwaError_Success, Define(server_session_, object));

    WaitForClientDefinition(AwaObjectDefinition_GetID(object.GetDefinition()));

    const char * path = "/1000/0/0";
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    AwaInteger value = 123456789;
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, path, value));
    EXPECT_EQ(AwaError_Response, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));

    // check response - should be AwaLWM2MError_BadRequest
    const AwaServerWriteResponse * response = AwaServerWriteOperation_GetResponse(writeOperation, global::clientEndpointName);
    EXPECT_TRUE(NULL != response);
    const AwaPathResult * pathResult = AwaServerWriteResponse_GetPathResult(response, path);
    EXPECT_EQ(AwaError_LWM2MError, AwaPathResult_GetError(pathResult));
    EXPECT_EQ(AwaLWM2MError_MethodNotAllowed, AwaPathResult_GetLWM2MError(pathResult));

    AwaServerWriteOperation_Free(&writeOperation);
}


TEST_F(TestWriteOperationWithConnectedServerAndClientSession, AwaServerWriteOperation_Perform_put_non_existent_resource_should_fail)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1,
    {
        ResourceDescription(0, "Resource0", AwaResourceType_Integer, 0, 1, AwaResourceOperations_ReadWrite),
    }};
    EXPECT_EQ(AwaError_Success, Define(client_session_, object));
    EXPECT_EQ(AwaError_Success, Define(server_session_, object));

    WaitForClientDefinition(AwaObjectDefinition_GetID(object.GetDefinition()));

    //create the object instance on the client
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(client_session_);
    EXPECT_TRUE(setOperation != NULL);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    const char * path = "/1000/0/0";
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Replace); ASSERT_TRUE(NULL != writeOperation);
    AwaInteger value = 123456789;
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, path, value));
    EXPECT_EQ(AwaError_Response, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));

    // check response - should fail
    const AwaServerWriteResponse * response = AwaServerWriteOperation_GetResponse(writeOperation, global::clientEndpointName);
    EXPECT_TRUE(NULL != response);
    const AwaPathResult * pathResult = AwaServerWriteResponse_GetPathResult(response, path);
    EXPECT_EQ(AwaError_LWM2MError, AwaPathResult_GetError(pathResult));
    EXPECT_EQ(AwaLWM2MError_MethodNotAllowed, AwaPathResult_GetLWM2MError(pathResult));

    AwaServerWriteOperation_Free(&writeOperation);
}
TEST_F(TestWriteOperationWithConnectedServerAndClientSession, AwaServerWriteOperation_Perform_post_non_existent_resource_should_succeed)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1,
    {
        ResourceDescription(0, "Resource0", AwaResourceType_Integer, 0, 1, AwaResourceOperations_ReadWrite),
    }};
    EXPECT_EQ(AwaError_Success, Define(client_session_, object));
    EXPECT_EQ(AwaError_Success, Define(server_session_, object));

    WaitForClientDefinition(AwaObjectDefinition_GetID(object.GetDefinition()));

    //create the object instance on the client
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(client_session_);
    EXPECT_TRUE(setOperation != NULL);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    const char * path = "/1000/0/0";
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    AwaInteger value = 123456789;
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, path, value));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));

    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperationWithConnectedServerAndClientSession, AwaServerWriteOperation_Perform_put_on_multiple_instance_resource_instance_should_replace)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1,
    {
        ResourceDescription(0, "Resource0", AwaResourceType_IntegerArray, 0, 10, AwaResourceOperations_ReadWrite),
    }};
    EXPECT_EQ(AwaError_Success, Define(client_session_, object));
    EXPECT_EQ(AwaError_Success, Define(server_session_, object));

    WaitForClientDefinition(AwaObjectDefinition_GetID(object.GetDefinition()));

    //create the object instance on the client and set the initial value
    AwaClientSetOperation * clientSet = AwaClientSetOperation_New(client_session_);
    EXPECT_TRUE(clientSet != NULL);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(clientSet, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(clientSet, "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddArrayValueAsInteger(clientSet, "/1000/0/0", 0, 12345));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(clientSet, defaults::timeout));
    AwaClientSetOperation_Free(&clientSet);
    AwaInteger expectedValue = 123456789;

    {
        AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Replace); ASSERT_TRUE(NULL != writeOperation);

        AwaIntegerArray * array = AwaIntegerArray_New();
        AwaIntegerArray_SetValue(array, 1, expectedValue);
        EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsIntegerArray(writeOperation, "/1000/0/0", array));
        EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
        AwaIntegerArray_Free(&array);
        AwaServerWriteOperation_Free(&writeOperation);
    }

    // should have replaced the entire array

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(client_session_);
    EXPECT_TRUE(getOperation != NULL);

    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));

    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    EXPECT_TRUE(getResponse != NULL);

    const AwaIntegerArray * array;
    AwaClientGetResponse_GetValuesAsIntegerArrayPointer(getResponse, "/1000/0/0", &array);
    EXPECT_TRUE(NULL != array);

    AwaIntegerArrayIterator * iterator = AwaIntegerArray_NewIntegerArrayIterator(array);

    ASSERT_TRUE(AwaIntegerArrayIterator_Next(iterator));
    EXPECT_EQ(1u, AwaIntegerArrayIterator_GetIndex(iterator));
    EXPECT_EQ(expectedValue, AwaIntegerArrayIterator_GetValue(iterator));

    EXPECT_FALSE(AwaIntegerArrayIterator_Next(iterator));

    AwaIntegerArrayIterator_Free(&iterator);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestWriteOperationWithConnectedServerAndClientSession, AwaServerWriteOperation_Perform_post_on_multiple_instance_resource_instance_should_update)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1,
    {
        ResourceDescription(0, "Resource0", AwaResourceType_IntegerArray, 0, 10, AwaResourceOperations_ReadWrite),
    }};
    EXPECT_EQ(AwaError_Success, Define(client_session_, object));
    EXPECT_EQ(AwaError_Success, Define(server_session_, object));

    WaitForClientDefinition(AwaObjectDefinition_GetID(object.GetDefinition()));

    //create the object instance on the client and set the initial value
    AwaClientSetOperation * clientSet = AwaClientSetOperation_New(client_session_);
    EXPECT_TRUE(clientSet != NULL);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(clientSet, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(clientSet, "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddArrayValueAsInteger(clientSet, "/1000/0/0", 0, 12345));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(clientSet, defaults::timeout));
    AwaClientSetOperation_Free(&clientSet);

    {
        AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);

        AwaIntegerArray * array = AwaIntegerArray_New();
        AwaIntegerArray_SetValue(array, 1, 67890);
        EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsIntegerArray(writeOperation, "/1000/0/0", array));
        EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
        AwaServerWriteOperation_Free(&writeOperation);
        AwaIntegerArray_Free(&array);
    }

    // A get on the resource should contain both the new and initial value.

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(client_session_);
    EXPECT_TRUE(getOperation != NULL);

    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));

    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    EXPECT_TRUE(getResponse != NULL);

    const AwaIntegerArray * array;
    AwaClientGetResponse_GetValuesAsIntegerArrayPointer(getResponse, "/1000/0/0", &array);
    EXPECT_TRUE(NULL != array);

    EXPECT_EQ(2u, AwaIntegerArray_GetValueCount(array));

    EXPECT_EQ(12345, AwaIntegerArray_GetValue(array, 0u));
    EXPECT_EQ(67890, AwaIntegerArray_GetValue(array, 1u));

    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_Perform_handles_invalid_operation_no_content)
{
    // Test behaviour when operation has no content
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    EXPECT_EQ(AwaError_OperationInvalid, AwaServerWriteOperation_Perform(writeOperation, "TestClient1", defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperationWithConnectedSession, DISABLED_AwaServerWriteOperation_Perform_handles_invalid_operation_invalid_session)
{
    // Test behaviour when parent session has been disconnected

    // Test behaviour when parent session has been freed

    ASSERT_FALSE(1);
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_Perform_handles_multiple_resources_same_object_instance)
{
    // start a client
    const char * clientID = "TestClient1";
    AwaClientDaemonHorde horde( { clientID }, 61000);
    sleep(1);      // wait for the client to register with the server

    // Test behaviour when we add multiple paths to write
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    AwaTime timeValue = 123456789;
    const char * stringValue = "hello";
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsTime(writeOperation, "/3/0/13", timeValue));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsCString(writeOperation, "/3/0/14", stringValue));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, clientID, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperationWithConnectedSession, DISABLED_AwaServerWriteOperation_Perform_handles_multiple_resources_different_instance)
{
    // start a client
    const char * clientID = "TestClient1";
    AwaClientDaemonHorde horde( { clientID }, 61000);
    sleep(1);      // wait for the client to register with the server

    // Test behaviour when we add multiple paths to write
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    AwaTime value = 123456789;
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsTime(writeOperation, "/3/0/13", value));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsTime(writeOperation, "/3/1/13", value));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, clientID, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_Perform_handles_null_operation)
{
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerWriteOperation_Perform(NULL, "TestClient1", defaults::timeout));
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_Perform_handles_negative_timeout)
{
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerWriteOperation_Perform(NULL, "TestClient1", -1));
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_Perform_handles_no_paths)
{
    const char * clientID = "TestClient1";
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    EXPECT_EQ(AwaError_OperationInvalid, AwaServerWriteOperation_Perform(writeOperation, clientID, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperationWithConnectedSession, DISABLED_AwaServerWriteOperation_Perform_handles_zero_timeout)
{
    // how?
}

TEST_F(TestWriteOperationWithConnectedSession, DISABLED_AwaServerWriteOperation_Perform_handles_short_timeout)
{
    // how?
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_Perform_honours_timeout)
{
    // start a client
    const char * clientID = "TestClient1";
    AwaClientDaemonHorde * horde_ = new AwaClientDaemonHorde( { clientID }, 61001);
    sleep(1);      // wait for the client to register with the server

    AwaServerSession * session = AwaServerSession_New();
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session, "0.0.0.0", global::serverIpcPort));
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);
    AwaTime value = 123456789;
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsTime(writeOperation, "/3/0/13", value));

    // Tear down server
    TestWriteOperationWithConnectedSession::TearDown();

    BasicTimer timer;
    timer.Start();
    EXPECT_EQ(AwaError_Timeout, AwaServerWriteOperation_Perform(writeOperation, clientID, defaults::timeout));
    timer.Stop();
    EXPECT_TRUE(ElapsedTimeWithinTolerance(timer.TimeElapsed_Milliseconds(), defaults::timeout, defaults::timeoutTolerance)) << "Time elapsed: " << timer.TimeElapsed_Milliseconds() << "ms";

    delete horde_;
    AwaServerWriteOperation_Free(&writeOperation);
    AwaServerSession_Free(&session);
}

TEST_F(TestWriteOperationWithConnectedSession, DISABLED_AwaServerWriteOperation_Perform_handles_disconnected_session)
{
    // Test behaviour when parent session has been disconnected
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    AwaInteger value = 123456789;
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/3/0/9", value));

    AwaServerSession_Disconnect(session_);

    EXPECT_EQ(AwaError_SessionNotConnected, AwaServerWriteOperation_Perform(writeOperation, "TestClient1", defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);
}


TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_CreateObjectInstance_handles_valid_operation_object_instance_id_specified)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    sleep(1);      // wait for the client to register with the server

    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/2/10");

    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_CreateObjectInstance_handles_valid_operation_object_instance_id_unspecified)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    sleep(1);      // wait for the client to register with the server


    // first do a read to see which instances already exist.
    std::vector<std::string> existingPaths;
    {
        AwaServerReadOperation * readOperation = AwaServerReadOperation_New(session_);
        ASSERT_TRUE(NULL != readOperation);

        ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/2"));
        ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, defaults::timeout));

        const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
        ASSERT_TRUE(NULL != readResponse);

        AwaPathIterator * iterator = AwaServerReadResponse_NewPathIterator(readResponse);
        while (AwaPathIterator_Next(iterator))
        {
            const char * path = AwaPathIterator_Get(iterator);
            if (Path_GetObjectInstanceID(path) != AWA_INVALID_ID && Path_GetResourceID(path) == AWA_INVALID_ID)
                existingPaths.push_back(path);
        }
        AwaPathIterator_Free(&iterator);
        //ASSERT_TRUE(AwaServerReadResponse_ContainsPath(readResponse, "/3/0/1"));

        AwaServerReadOperation_Free(&readOperation);
    }


    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/2");

    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));

    const AwaServerWriteResponse * writeResponse = AwaServerWriteOperation_GetResponse(writeOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != writeResponse);

    AwaPathIterator * iterator = AwaServerWriteResponse_NewPathIterator(writeResponse);
    const char * newInstancePath = NULL;
    while (AwaPathIterator_Next(iterator))
    {
        const char * path = AwaPathIterator_Get(iterator);
        if (Path_GetObjectInstanceID(path) != AWA_INVALID_ID && Path_GetResourceID(path) == AWA_INVALID_ID)
        {
            if (std::find(existingPaths.begin(), existingPaths.end(), path) == existingPaths.end())
            {
                // a new instance exists that wasn't there in the read operation
                newInstancePath = path;
                break;
            }
        }
    }

    ASSERT_TRUE(NULL != newInstancePath);
    //printf("Found new instance: %s\n", newInstancePath);

    AwaPathIterator_Free(&iterator);
    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_CreateObjectInstance_object_instance_id_specified_handles_writing_resource_values)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    sleep(1);      // wait for the client to register with the server

    AwaInteger expected = 13232;
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/2/10");
    AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/2/10/3", expected);
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(session_);
    EXPECT_TRUE(NULL != readOperation);

    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/2/10/3"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, defaults::timeout));

    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    EXPECT_TRUE(NULL != readResponse);

    const AwaInteger * value;
    EXPECT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsIntegerPointer(readResponse, "/2/10/3", &value));

    EXPECT_EQ(expected, *value);

    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestWriteOperationWithConnectedSession, DISABLED_AwaServerWriteOperation_CreateObjectInstance_object_instance_id_unspecified_handles_writing_resource_values)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    sleep(1);      // wait for the client to register with the server

    AwaInteger expected = 13232;
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/2");
    AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/2", expected);  // FIXME: Have to allow writing values to /O in this case
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(session_);
    EXPECT_TRUE(NULL != readOperation);

    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/2/10/3"));
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, defaults::timeout));

    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    EXPECT_TRUE(NULL != readResponse);

    const AwaInteger * value;
    EXPECT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsIntegerPointer(readResponse, "/2/10/3", &value));

    EXPECT_EQ(expected, *value);

    AwaServerReadOperation_Free(&readOperation);
}


TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_CreateObjectInstance_handles_null_operation)
{
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerWriteOperation_CreateObjectInstance(NULL, "/3"));
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_CreateObjectInstance_handles_null_path)
{
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);

    ASSERT_EQ(AwaError_PathInvalid, AwaServerWriteOperation_CreateObjectInstance(writeOperation, NULL));

    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_CreateObjectInstance_handles_invalid_path)
{
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);

    ASSERT_EQ(AwaError_PathInvalid, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/3/0/1"));

    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_CreateObjectInstance_handles_existing_instance)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    sleep(1);      // wait for the client to register with the server

    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/3/0"));
    EXPECT_EQ(AwaError_Response, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));

    const AwaServerWriteResponse * response = AwaServerWriteOperation_GetResponse(writeOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != response);
    const AwaPathResult * pathResult = AwaServerWriteResponse_GetPathResult(response, "/3/0");
    EXPECT_TRUE(NULL != pathResult);
    EXPECT_EQ(AwaError_LWM2MError, AwaPathResult_GetError(pathResult));
    EXPECT_EQ(AwaLWM2MError_BadRequest, AwaPathResult_GetLWM2MError(pathResult));

    AwaServerWriteOperation_Free(&writeOperation);
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_CreateObjectInstance_handles_maximum_instances)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, 61000);
    sleep(1);      // wait for the client to register with the server

    // Test we cannot create more object instances than the object definition allows
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/3"));

    EXPECT_EQ(AwaError_Response, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));

    const AwaServerWriteResponse * response = AwaServerWriteOperation_GetResponse(writeOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != response);
    const AwaPathResult * pathResult = AwaServerWriteResponse_GetPathResult(response, "/3");
    EXPECT_TRUE(NULL != pathResult);
    EXPECT_EQ(AwaError_LWM2MError, AwaPathResult_GetError(pathResult));
    EXPECT_EQ(AwaLWM2MError_MethodNotAllowed, AwaPathResult_GetLWM2MError(pathResult));

    AwaServerWriteOperation_Free(&writeOperation);
}



TEST_F(TestWriteOperationWithConnectedSession, Consecutive_Writes_to_Custom_Object)
{
    // start a client
    const char * clientID = "TestClient1";
    AwaClientDaemonHorde horde( { clientID }, 61000);
    sleep(1);      // wait for the client to register with the server

    //Create client session
    AwaClientSession * clientSession = AwaClientSession_New();
    EXPECT_TRUE(clientSession != NULL);

    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(clientSession, "0.0.0.0", 61000));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(clientSession));

    //Create custom object definition
    AwaObjectDefinition * customObject = AwaObjectDefinition_New(9999, "testObject", 0, AWA_MAX_ID);
    EXPECT_TRUE(customObject != NULL);

    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBoolean(customObject, 1, "testResource", true, AwaResourceOperations_ReadWrite, false));

    //client define...
    {
        AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(clientSession);
        EXPECT_TRUE(defineOperation != NULL);

        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObject));
        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

        AwaClientDefineOperation_Free(&defineOperation);
    }

    //server define...
    {
        AwaServerDefineOperation * defineOperation = AwaServerDefineOperation_New(session_);
        EXPECT_TRUE(defineOperation != NULL);

        EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(defineOperation, customObject));
        EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(defineOperation, defaults::timeout));

        AwaServerDefineOperation_Free(&defineOperation);
    }

    AwaObjectDefinition_Free(&customObject);

    //FIXME: FLOWDM-498: server api should create the object instance
    AwaClientSetOperation * clientSet = AwaClientSetOperation_New(clientSession);
    EXPECT_TRUE(clientSet != NULL);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(clientSet, "/9999/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(clientSet, defaults::timeout));
    AwaClientSetOperation_Free(&clientSet);

    //First write and get
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    EXPECT_TRUE(writeOperation != NULL);
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(clientSession);
    EXPECT_TRUE(getOperation != NULL);

    AwaBoolean value = false;
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsBoolean(writeOperation, "/9999/0/1", value));
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/9999/0/1"));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, clientID, defaults::timeout));
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));

    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    EXPECT_TRUE(getResponse != NULL);

    const AwaBoolean * valueResult = NULL;
    EXPECT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsBooleanPointer(getResponse, "/9999/0/1", &valueResult));

    ASSERT_TRUE(valueResult != NULL);
    ASSERT_EQ(value, *valueResult);

    AwaServerWriteOperation_Free(&writeOperation);

    //Second write and get
    writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    EXPECT_TRUE(writeOperation != NULL);
    //Change the /9999/0/1 respource to true;
    value = true;
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsBoolean(writeOperation, "/9999/0/1", value));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, clientID, defaults::timeout));

    //We can perform the get twice without creating a new operation...
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    getResponse = AwaClientGetOperation_GetResponse(getOperation);
    EXPECT_TRUE(getResponse != NULL);
    EXPECT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsBooleanPointer(getResponse, "/9999/0/1", &valueResult));

    //Should see true here!
    ASSERT_TRUE(valueResult != NULL);
    ASSERT_EQ(value, *valueResult);

    AwaServerWriteOperation_Free(&writeOperation);
    AwaClientGetOperation_Free(&getOperation);

    AwaClientSession_Free(&clientSession);
}

///***********************************************************************************************************
// * WriteValue parameterised tests
// */

namespace writeDetail
{

struct TestWriteResource
{
    bool CreateSetOperation;
    bool PopulateArrayWithInitialValues;
    AwaError ExpectedAddResult;
    AwaError ExpectedProcessResult;

    AwaObjectID ObjectID;
    AwaObjectInstanceID ObjectInstanceID;
    AwaResourceID ResourceID;

    const void * Value;
    const size_t ValueCount;
    AwaResourceType Type;
};

::std::ostream& operator<<(::std::ostream& os, const TestWriteResource& item)
{
  return os << "Item: expectedAddResult " << item.ExpectedAddResult
            << ", expectedProcessResult " << item.ExpectedProcessResult
            << ", UseSetOperation " << item.CreateSetOperation
            << ", PopulateArrayWithInitialValues " << item.PopulateArrayWithInitialValues
            << ", objectID " << item.ObjectID
            << ", objectInstanceID " << item.ObjectInstanceID
            << ", resourceID " << item.ResourceID
            << ", value " << item.Value
            << ", valueCount " << item.ValueCount
            << ", type" << item.Type;
}

const char * clientID = "TestClient1";

static AwaInteger dummyInteger1 = 123456;
static const char * dummyString1 = "Lightweight M2M Server";
static AwaFloat dummyFloat1 = 1.0;
static AwaTime dummyTime1 = 0xA20AD72B;
static AwaBoolean dummyBoolean1 = true;

//static int dummyObjLinkData[] = {-1,-1};

AwaObjectLink dummyObjectLink1 = { 3, 5 };
AwaObjectLink dummyObjectLink2 = { 1, 7 };
AwaObjectLink dummyObjectLink3 = { 0, 1 };

const char * dummyStringArray1[] = {"Lightweight M2M Server", "test1", ""};
const char * dummyStringArray2[] = {"Lightweight M2M Server", "test1", " ", " ", " ", " "};
const AwaInteger dummyIntegerArray1[] = {55, 8732, 11};
const AwaInteger dummyIntegerArray2[] = {55, 8732, 11, 55, 8732, 11};
const AwaFloat dummyFloatArray1[] = {55.0, 0.0008, 11e10};
const AwaFloat dummyFloatArray2[] = {55.0, 0.0008, 11e10, 55.0, 0.0008, 11e10};
const AwaBoolean dummyBooleanArray1[] = {true, false, true};
const AwaBoolean dummyBooleanArray2[] = {true, false, true, true, false, true};


static char dummyOpaqueData1[] = {'a',0,'x','\0', 123};
static char dummyOpaqueData2[] = {'a',0,'x','\0', 123, 'c', '2'};
static char dummyOpaqueData3[] = {'a',0,'x','\0', 123, 1, 5, 0, 6};

AwaOpaque dummyOpaque1 = {(void*) dummyOpaqueData1, sizeof(dummyOpaqueData1)};
AwaOpaque dummyOpaque2 = {(void*) dummyOpaqueData2, sizeof(dummyOpaqueData2)};
AwaOpaque dummyOpaque3 = {(void*) dummyOpaqueData3, sizeof(dummyOpaqueData3)};
const AwaOpaque * dummyOpaqueArray1[] = {&dummyOpaque1, &dummyOpaque2, &dummyOpaque3};
const AwaOpaque * dummyOpaqueArray2[] = {&dummyOpaque1, &dummyOpaque2, &dummyOpaque3, &dummyOpaque1, &dummyOpaque2, &dummyOpaque3};

const AwaInteger dummyTimeArray1[] = {16000, 8732222, 1111};
const AwaInteger dummyTimeArray2[] = {16000, 8732222, 1111, 16000, 8732222, 1111};


const AwaObjectLink * dummyObjectLinkArray1[] = {&dummyObjectLink1, &dummyObjectLink2, &dummyObjectLink3};
const AwaObjectLink * dummyObjectLinkArray2[] = {&dummyObjectLink1, &dummyObjectLink2, &dummyObjectLink3, &dummyObjectLink1, &dummyObjectLink2, &dummyObjectLink3};

const AwaObjectID TEST_OBJECT_NON_ARRAY_TYPES = 10000;
const AwaResourceID TEST_RESOURCE_STRING = 1;
const AwaResourceID TEST_RESOURCE_INTEGER = 2;
const AwaResourceID TEST_RESOURCE_FLOAT = 3;
const AwaResourceID TEST_RESOURCE_BOOLEAN = 4;
const AwaResourceID TEST_RESOURCE_OPAQUE = 5;
const AwaResourceID TEST_RESOURCE_TIME = 6;
const AwaResourceID TEST_RESOURCE_OBJECTLINK = 7;

const AwaObjectID TEST_OBJECT_ARRAY_TYPES = 10001;
const AwaResourceID TEST_RESOURCE_STRINGARRAY = 1;
const AwaResourceID TEST_RESOURCE_INTEGERARRAY = 2;
const AwaResourceID TEST_RESOURCE_FLOATARRAY = 3;
const AwaResourceID TEST_RESOURCE_BOOLEANARRAY = 4;
const AwaResourceID TEST_RESOURCE_OPAQUEARRAY = 5;
const AwaResourceID TEST_RESOURCE_TIMEARRAY = 6;
const AwaResourceID TEST_RESOURCE_OBJECTLINKARRAY = 7;

}

class TestWriteValue : public TestWriteOperationWithServerDaemon, public ::testing::WithParamInterface< writeDetail::TestWriteResource>
{
protected:

    void SetUp() {
        TestWriteOperationWithServerDaemon::SetUp();
        writeDetail::TestWriteResource data = GetParam();
        // start a client
        // TO RUN with debug, don't create the client, and specify the clientIpcPort, serverIpcPort, bootstrapConfig
        horde_ = global::spawnClientDaemon ? new AwaClientDaemonHorde( { writeDetail::clientID }, 61000) : NULL;
        sleep(1);      // wait for the client to register with the server

        // set up a valid, connected session:
        clientSession_ = AwaClientSession_New(); ASSERT_TRUE(NULL != clientSession_);
        serverSession_ = AwaServerSession_New(); ASSERT_TRUE(NULL != serverSession_);
        ASSERT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(clientSession_, "127.0.0.1", global::spawnClientDaemon? 61000 : global::clientIpcPort));
        ASSERT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(serverSession_, "127.0.0.1", global::serverIpcPort));
        ASSERT_EQ(AwaError_Success, AwaClientSession_Connect(clientSession_));
        ASSERT_EQ(AwaError_Success, AwaServerSession_Connect(serverSession_));


        AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(clientSession_);
        AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(serverSession_);
        EXPECT_TRUE(clientDefineOperation != NULL);
        EXPECT_TRUE(serverDefineOperation != NULL);

        AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(writeDetail::TEST_OBJECT_NON_ARRAY_TYPES, "Test Object Single", 0, 1);
        EXPECT_TRUE(NULL != customObjectDefinition);

        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsString     (customObjectDefinition, writeDetail::TEST_RESOURCE_STRING,     "Test String Resource",      true, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger    (customObjectDefinition, writeDetail::TEST_RESOURCE_INTEGER,    "Test Integer Resource",     true, AwaResourceOperations_ReadWrite, 0));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloat      (customObjectDefinition, writeDetail::TEST_RESOURCE_FLOAT,      "Test Float Resource",       true, AwaResourceOperations_ReadWrite, 0.0));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBoolean    (customObjectDefinition, writeDetail::TEST_RESOURCE_BOOLEAN,    "Test Boolean Resource",     true, AwaResourceOperations_ReadWrite, false));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque     (customObjectDefinition, writeDetail::TEST_RESOURCE_OPAQUE,     "Test Opaque Resource",      true, AwaResourceOperations_ReadWrite, AwaOpaque {0}));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTime       (customObjectDefinition, writeDetail::TEST_RESOURCE_TIME,       "Test Time Resource",        true, AwaResourceOperations_ReadWrite, 0));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLink (customObjectDefinition, writeDetail::TEST_RESOURCE_OBJECTLINK, "Test Object Link Resource", true, AwaResourceOperations_ReadWrite, AwaObjectLink {0}));

        EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition));
        AwaObjectDefinition_Free(&customObjectDefinition);

        customObjectDefinition = AwaObjectDefinition_New(writeDetail::TEST_OBJECT_ARRAY_TYPES, "Test Object Array", 0, 1);
        EXPECT_TRUE(NULL != customObjectDefinition);

        int maximumArrayInstances = 5;

        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsStringArray    (customObjectDefinition, writeDetail::TEST_RESOURCE_STRING,     "Test String Array Resource",      0, maximumArrayInstances, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsIntegerArray   (customObjectDefinition, writeDetail::TEST_RESOURCE_INTEGER,    "Test Integer Array Resource",     0, maximumArrayInstances, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloatArray     (customObjectDefinition, writeDetail::TEST_RESOURCE_FLOAT,      "Test Float Array Resource",       0, maximumArrayInstances, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBooleanArray   (customObjectDefinition, writeDetail::TEST_RESOURCE_BOOLEAN,    "Test Boolean Array Resource",     0, maximumArrayInstances, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaqueArray    (customObjectDefinition, writeDetail::TEST_RESOURCE_OPAQUE,     "Test Opaque Array Resource",      0, maximumArrayInstances, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTimeArray      (customObjectDefinition, writeDetail::TEST_RESOURCE_TIME,       "Test Time Array Resource",        0, maximumArrayInstances, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLinkArray(customObjectDefinition, writeDetail::TEST_RESOURCE_OBJECTLINK, "Test Object Link Array Resource", 0, maximumArrayInstances, AwaResourceOperations_ReadWrite, NULL));

        EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition));
        EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, defaults::timeout));
        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, defaults::timeout));

        AwaObjectDefinition_Free(&customObjectDefinition);
        AwaClientDefineOperation_Free(&clientDefineOperation);
        AwaServerDefineOperation_Free(&serverDefineOperation);


        //FIXME: FLOWDM-498: this creates the object instance for the dummy object on the client as the server api does not support object/resource creation yet
        //comment this code to find tests that aren't LWM2M compliant
        AwaClientSetOperation * setOperation = AwaClientSetOperation_New(clientSession_);
        EXPECT_TRUE(setOperation != NULL);
        EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
        EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10001/0"));

        EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10001/0/1"));
        EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10001/0/2"));
        EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10001/0/3"));
        EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10001/0/4"));
        EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10001/0/5"));
        EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10001/0/6"));
        EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10001/0/7"));

        if (data.PopulateArrayWithInitialValues)
        {
            for (int i = maximumArrayInstances/2; i < maximumArrayInstances; i++)
            {
                char tempString[8];
                sprintf(tempString, "%d", i * 123);

                char tempOpaqueData[] = {'2',static_cast<char>(i),0,'a','c','\n'};

                ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddArrayValueAsCString(setOperation, "/10001/0/1", i, tempString));
                ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddArrayValueAsInteger(setOperation, "/10001/0/2", i, i*222));
                ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddArrayValueAsFloat(setOperation, "/10001/0/3", i, i*0.2));
                ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddArrayValueAsBoolean(setOperation, "/10001/0/4", i, i == 0));
                ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddArrayValueAsOpaque(setOperation, "/10001/0/5", i, {tempOpaqueData, sizeof(tempOpaqueData)}));
                ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddArrayValueAsTime(setOperation, "/10001/0/6", i, i*10030));
                ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddArrayValueAsObjectLink(setOperation, "/10001/0/7", i, {i, i*10}));
            }
        }

        EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
        AwaClientSetOperation_Free(&setOperation);
    }

    void TearDown() {

        AwaClientSession_Disconnect(clientSession_);
        AwaServerSession_Disconnect(serverSession_);

        AwaClientSession_Free(&clientSession_);
        AwaServerSession_Free(&serverSession_);
        if (horde_ != NULL)
        {
            delete horde_;
        }
        TestWriteOperationWithServerDaemon::TearDown();
    }

    AwaClientSession * clientSession_;
    AwaServerSession * serverSession_;
    AwaClientDaemonHorde * horde_;

};

class TestWriteValueArray : public TestWriteValue {};

TEST_P(TestWriteValueArray, TestWriteValueArray)
{
    writeDetail::TestWriteResource data = GetParam();

    AwaServerWriteOperation * writeOperation = data.CreateSetOperation? AwaServerWriteOperation_New(serverSession_, AwaWriteMode_Replace) : NULL;
    char path[128] = {0};

    if(data.ObjectID == AWA_INVALID_ID)
    {
        sprintf(path, "a/n in/valid/ path");
    }
    else
    {
        EXPECT_EQ(AwaError_Success, AwaAPI_MakePath(path, sizeof(path), data.ObjectID, data.ObjectInstanceID, data.ResourceID));
    }

    AwaArray * setArray = NULL;

    switch(data.Type)
    {
        case AwaResourceType_StringArray:
            {
                AwaStringArray * array = AwaStringArray_New();
                EXPECT_TRUE(NULL != array);

                for(size_t i = 0; i < data.ValueCount; i++)
                {
                    AwaStringArray_SetValueAsCString(array, i, ((const char **)data.Value)[i]);
                }

                ASSERT_EQ(data.ExpectedAddResult, AwaServerWriteOperation_AddValueAsStringArray(writeOperation, path, array));
                setArray = (AwaArray *)array;
            }
            break;
        case AwaResourceType_IntegerArray:
            {
                AwaIntegerArray * array = AwaIntegerArray_New();
                EXPECT_TRUE(NULL != array);

                for(size_t i = 0; i < data.ValueCount; i++)
                {
                    AwaIntegerArray_SetValue(array, i, ((const AwaInteger *)data.Value)[i]);
                }

                ASSERT_EQ(data.ExpectedAddResult, AwaServerWriteOperation_AddValueAsIntegerArray(writeOperation, path, array));
                setArray = (AwaArray *)array;
            }
            break;
        case AwaResourceType_FloatArray:
            {
                AwaFloatArray * array = AwaFloatArray_New();
                EXPECT_TRUE(NULL != array);

                for(size_t i = 0; i < data.ValueCount; i++)
                {
                    AwaFloatArray_SetValue(array, i, ((const AwaFloat *)data.Value)[i]);
                }

                ASSERT_EQ(data.ExpectedAddResult, AwaServerWriteOperation_AddValueAsFloatArray(writeOperation, path, array));
                setArray = (AwaArray *)array;
            }
            break;
        case AwaResourceType_BooleanArray:
            {
                AwaBooleanArray * array = AwaBooleanArray_New();
                EXPECT_TRUE(NULL != array);

                for(size_t i = 0; i < data.ValueCount; i++)
                {
                    AwaBooleanArray_SetValue(array, i, ((const AwaBoolean *)data.Value)[i]);
                }

                ASSERT_EQ(data.ExpectedAddResult, AwaServerWriteOperation_AddValueAsBooleanArray(writeOperation, path, array));
                setArray = (AwaArray *)array;
            }
            break;
        case AwaResourceType_OpaqueArray:
            {
                AwaOpaqueArray * array = AwaOpaqueArray_New();
                EXPECT_TRUE(NULL != array);

                for(size_t i = 0; i < data.ValueCount; i++)
                {
                    AwaOpaqueArray_SetValue(array, i, *((AwaOpaque **)data.Value)[i]);
                }

                ASSERT_EQ(data.ExpectedAddResult, AwaServerWriteOperation_AddValueAsOpaqueArray(writeOperation, path, array));
                setArray = (AwaArray *)array;
            }
            break;
        case AwaResourceType_TimeArray:
            {
                AwaTimeArray * array = AwaTimeArray_New();
                EXPECT_TRUE(NULL != array);

                for(size_t i = 0; i < data.ValueCount; i++)
                {
                    AwaTimeArray_SetValue(array, i, ((const AwaTime *)data.Value)[i]);
                }

                ASSERT_EQ(data.ExpectedAddResult, AwaServerWriteOperation_AddValueAsTimeArray(writeOperation, path, array));
                setArray = (AwaArray *)array;
            }
            break;
        case AwaResourceType_ObjectLinkArray:
            {
                AwaObjectLinkArray * array = AwaObjectLinkArray_New();
                EXPECT_TRUE(NULL != array);

                for(size_t i = 0; i < data.ValueCount; i++)
                {
                    AwaObjectLinkArray_SetValue(array, i, *((AwaObjectLink **)data.Value)[i]);
                }

                ASSERT_EQ(data.ExpectedAddResult, AwaServerWriteOperation_AddValueAsObjectLinkArray(writeOperation, path, array));
                setArray = (AwaArray *)array;
            }
            break;
        default:
            ASSERT_TRUE(false);
            break;
    }
    ASSERT_TRUE(NULL != setArray);

    if (data.ExpectedAddResult == AwaError_Success)
    {
        EXPECT_EQ(data.ExpectedProcessResult, AwaServerWriteOperation_Perform(writeOperation, writeDetail::clientID, defaults::timeout));
    }

    AwaServerWriteOperation_Free(&writeOperation);


    if (data.ExpectedProcessResult == AwaError_Success)
    {
        // Confirm the value was set correctly
        AwaClientGetOperation * getOperation = AwaClientGetOperation_New(clientSession_);
        ASSERT_TRUE(NULL != getOperation);
        ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, path));
        ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
        const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
        ASSERT_TRUE(NULL != getResponse);

        AwaArray * getArray = NULL;

        switch(data.Type)
        {
            case AwaResourceType_StringArray:
            {
                ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsStringArrayPointer(getResponse, path, (const AwaStringArray **)&getArray));
                break;
            }
            case AwaResourceType_IntegerArray:
            {
                ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsIntegerArrayPointer(getResponse, path, (const AwaIntegerArray **)&getArray));
                break;
            }
            case AwaResourceType_FloatArray:
            {
                ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsFloatArrayPointer(getResponse, path, (const AwaFloatArray **)&getArray));
                break;
            }
            case AwaResourceType_BooleanArray:
            {
                ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsBooleanArrayPointer(getResponse, path, (const AwaBooleanArray **)&getArray));
                break;
            }
            case AwaResourceType_OpaqueArray:
            {
                ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsOpaqueArrayPointer(getResponse, path, (const AwaOpaqueArray **)&getArray));
                break;
            }
            case AwaResourceType_TimeArray:
            {
                ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsTimeArrayPointer(getResponse, path, (const AwaTimeArray **)&getArray));
                break;
            }
            case AwaResourceType_ObjectLinkArray:
            {
                ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsObjectLinkArrayPointer(getResponse, path, (const AwaObjectLinkArray **)&getArray));
                break;
            }
            default:
                ASSERT_TRUE(false);
                break;
        }

        EXPECT_EQ(0, Array_Compare(setArray, getArray, data.Type));
        AwaClientGetOperation_Free(&getOperation);
    }


    switch(data.Type)
    {
        case AwaResourceType_StringArray:
            AwaStringArray_Free((AwaStringArray **)&setArray);
            break;
        case AwaResourceType_IntegerArray:
            AwaIntegerArray_Free((AwaIntegerArray **)&setArray);
            break;
        case AwaResourceType_FloatArray:
            AwaFloatArray_Free((AwaFloatArray **)&setArray);
            break;
        case AwaResourceType_BooleanArray:
            AwaBooleanArray_Free((AwaBooleanArray **)&setArray);
            break;
        case AwaResourceType_OpaqueArray:
            AwaOpaqueArray_Free((AwaOpaqueArray **)&setArray);
            break;
        case AwaResourceType_TimeArray:
            AwaTimeArray_Free((AwaTimeArray **)&setArray);
            break;
        case AwaResourceType_ObjectLinkArray:
            AwaObjectLinkArray_Free((AwaObjectLinkArray **)&setArray);
            break;
        default:
            ASSERT_TRUE(false);
            break;
    }
}

INSTANTIATE_TEST_CASE_P(
        TestWriteValueArray1,
        TestWriteValueArray,
        ::testing::Values(

          writeDetail::TestWriteResource {true, false, AwaError_Success,          AwaError_Success,          writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_STRINGARRAY,     &writeDetail::dummyStringArray1,3,     AwaResourceType_StringArray},
          writeDetail::TestWriteResource {true, true, AwaError_Success,          AwaError_Success,          writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_STRINGARRAY,     &writeDetail::dummyStringArray1,3,     AwaResourceType_StringArray},
          writeDetail::TestWriteResource {true, false, AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,                    0,                  writeDetail::TEST_RESOURCE_STRINGARRAY,     &writeDetail::dummyStringArray1,3,     AwaResourceType_StringArray},
          writeDetail::TestWriteResource {true, false, AwaError_TypeMismatch,     AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_INTEGERARRAY,    &writeDetail::dummyStringArray1,3,     AwaResourceType_StringArray},
          writeDetail::TestWriteResource {false, false, AwaError_OperationInvalid, AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_STRINGARRAY,     &writeDetail::dummyStringArray1,3,     AwaResourceType_StringArray},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  AWA_INVALID_ID,                         &writeDetail::dummyStringArray1,3,     AwaResourceType_StringArray},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,                         &writeDetail::dummyStringArray1,3,     AwaResourceType_StringArray},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,                    AWA_INVALID_ID, AWA_INVALID_ID,                         &writeDetail::dummyStringArray1,3,     AwaResourceType_StringArray},
          writeDetail::TestWriteResource {true, false, AwaError_AddInvalid,       AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_STRINGARRAY,     &writeDetail::dummyStringArray2,6,     AwaResourceType_StringArray},

          writeDetail::TestWriteResource {true, false, AwaError_Success,          AwaError_Success,          writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_INTEGERARRAY,    writeDetail::dummyIntegerArray1,3,     AwaResourceType_IntegerArray},
          writeDetail::TestWriteResource {true, true, AwaError_Success,          AwaError_Success,          writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_INTEGERARRAY,    writeDetail::dummyIntegerArray1,3,     AwaResourceType_IntegerArray},
          writeDetail::TestWriteResource {true, false, AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,                    0,                  writeDetail::TEST_RESOURCE_INTEGERARRAY,    writeDetail::dummyIntegerArray1,3,     AwaResourceType_IntegerArray},
          writeDetail::TestWriteResource {true, false, AwaError_TypeMismatch,     AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_STRINGARRAY,     writeDetail::dummyIntegerArray1,3,     AwaResourceType_IntegerArray},
          writeDetail::TestWriteResource {false, false, AwaError_OperationInvalid, AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_INTEGERARRAY,    writeDetail::dummyIntegerArray1,3,     AwaResourceType_IntegerArray},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  AWA_INVALID_ID,                         writeDetail::dummyIntegerArray1,3,     AwaResourceType_IntegerArray},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,                         writeDetail::dummyIntegerArray1,3,     AwaResourceType_IntegerArray},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,                    AWA_INVALID_ID, AWA_INVALID_ID,                         writeDetail::dummyIntegerArray1,3,     AwaResourceType_IntegerArray},
          writeDetail::TestWriteResource {true, false, AwaError_AddInvalid,       AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_INTEGERARRAY,    writeDetail::dummyIntegerArray2,6,     AwaResourceType_IntegerArray},

          writeDetail::TestWriteResource {true, false, AwaError_Success,          AwaError_Success,          writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_FLOATARRAY,      &writeDetail::dummyFloatArray1,3,      AwaResourceType_FloatArray},
          writeDetail::TestWriteResource {true, true, AwaError_Success,          AwaError_Success,          writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_FLOATARRAY,      &writeDetail::dummyFloatArray1,3,      AwaResourceType_FloatArray},
          writeDetail::TestWriteResource {true, false, AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,                    0,                  writeDetail::TEST_RESOURCE_FLOATARRAY,      &writeDetail::dummyFloatArray1,3,      AwaResourceType_FloatArray},
          writeDetail::TestWriteResource {true, false, AwaError_TypeMismatch,     AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_STRINGARRAY,     &writeDetail::dummyFloatArray1,3,      AwaResourceType_FloatArray},
          writeDetail::TestWriteResource {false, false, AwaError_OperationInvalid, AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_FLOATARRAY,      &writeDetail::dummyFloatArray1,3,      AwaResourceType_FloatArray},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  AWA_INVALID_ID,                         &writeDetail::dummyFloatArray1,3,      AwaResourceType_FloatArray},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,                         &writeDetail::dummyFloatArray1,3,      AwaResourceType_FloatArray},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,                    AWA_INVALID_ID, AWA_INVALID_ID,                         &writeDetail::dummyFloatArray1,3,      AwaResourceType_FloatArray},
          writeDetail::TestWriteResource {true, false, AwaError_AddInvalid,       AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_FLOATARRAY,      &writeDetail::dummyFloatArray2,6,      AwaResourceType_FloatArray},

          writeDetail::TestWriteResource {true, false, AwaError_Success,          AwaError_Success,          writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_BOOLEANARRAY,    &writeDetail::dummyBooleanArray1,3,    AwaResourceType_BooleanArray},
          writeDetail::TestWriteResource {true, true, AwaError_Success,          AwaError_Success,          writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_BOOLEANARRAY,    &writeDetail::dummyBooleanArray1,3,    AwaResourceType_BooleanArray},
          writeDetail::TestWriteResource {true, false, AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,                    0,                  writeDetail::TEST_RESOURCE_BOOLEANARRAY,    &writeDetail::dummyBooleanArray1,3,    AwaResourceType_BooleanArray},
          writeDetail::TestWriteResource {true, false, AwaError_TypeMismatch,     AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_STRINGARRAY,     &writeDetail::dummyBooleanArray1,3,    AwaResourceType_BooleanArray},
          writeDetail::TestWriteResource {false, false, AwaError_OperationInvalid, AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_BOOLEANARRAY,    &writeDetail::dummyBooleanArray1,3,    AwaResourceType_BooleanArray},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  AWA_INVALID_ID,                         &writeDetail::dummyBooleanArray1,3,    AwaResourceType_BooleanArray},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,                         &writeDetail::dummyBooleanArray1,3,    AwaResourceType_BooleanArray},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,                    AWA_INVALID_ID, AWA_INVALID_ID,                         &writeDetail::dummyBooleanArray1,3,    AwaResourceType_BooleanArray},
          writeDetail::TestWriteResource {true, false, AwaError_AddInvalid,       AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_BOOLEANARRAY,    &writeDetail::dummyBooleanArray2,6,    AwaResourceType_BooleanArray}

          ));

INSTANTIATE_TEST_CASE_P(
        TestWriteValueArray2,
        TestWriteValueArray,
        ::testing::Values(

          writeDetail::TestWriteResource {true, false, AwaError_Success,          AwaError_Success,          writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_OPAQUEARRAY,     &writeDetail::dummyOpaqueArray1,3,     AwaResourceType_OpaqueArray},
          writeDetail::TestWriteResource {true, true, AwaError_Success,          AwaError_Success,          writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_OPAQUEARRAY,     &writeDetail::dummyOpaqueArray1,3,     AwaResourceType_OpaqueArray},
          writeDetail::TestWriteResource {true, false, AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,                    0,                  writeDetail::TEST_RESOURCE_OPAQUEARRAY,     &writeDetail::dummyOpaqueArray1,3,     AwaResourceType_OpaqueArray},
          writeDetail::TestWriteResource {true, false, AwaError_TypeMismatch,     AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_STRINGARRAY,     &writeDetail::dummyOpaqueArray1,3,     AwaResourceType_OpaqueArray},
          writeDetail::TestWriteResource {false, false, AwaError_OperationInvalid, AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_OPAQUEARRAY,     &writeDetail::dummyOpaqueArray1,3,     AwaResourceType_OpaqueArray},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  AWA_INVALID_ID,                         &writeDetail::dummyOpaqueArray1,3,     AwaResourceType_OpaqueArray},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,                         &writeDetail::dummyOpaqueArray1,3,     AwaResourceType_OpaqueArray},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,                    AWA_INVALID_ID, AWA_INVALID_ID,                         &writeDetail::dummyOpaqueArray1,3,     AwaResourceType_OpaqueArray},
          writeDetail::TestWriteResource {true, false, AwaError_AddInvalid,       AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_BOOLEANARRAY,    &writeDetail::dummyBooleanArray2,6,    AwaResourceType_BooleanArray},

          writeDetail::TestWriteResource {true, false, AwaError_Success,          AwaError_Success,          writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_TIMEARRAY,       &writeDetail::dummyTimeArray1,3,       AwaResourceType_TimeArray},
          writeDetail::TestWriteResource {true, true, AwaError_Success,          AwaError_Success,          writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_TIMEARRAY,       &writeDetail::dummyTimeArray1,3,       AwaResourceType_TimeArray},
          writeDetail::TestWriteResource {true, false, AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,                    0,                  writeDetail::TEST_RESOURCE_TIMEARRAY,       &writeDetail::dummyTimeArray1,3,       AwaResourceType_TimeArray},
          writeDetail::TestWriteResource {true, false, AwaError_TypeMismatch,     AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_STRINGARRAY,     &writeDetail::dummyTimeArray1,3,       AwaResourceType_TimeArray},
          writeDetail::TestWriteResource {false, false, AwaError_OperationInvalid, AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_TIMEARRAY,       &writeDetail::dummyTimeArray1,3,       AwaResourceType_TimeArray},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  AWA_INVALID_ID,                         &writeDetail::dummyTimeArray1,3,       AwaResourceType_TimeArray},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,                         &writeDetail::dummyTimeArray1,3,       AwaResourceType_TimeArray},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,                    AWA_INVALID_ID, AWA_INVALID_ID,                         &writeDetail::dummyTimeArray1,3,       AwaResourceType_TimeArray},
          writeDetail::TestWriteResource {true, false, AwaError_AddInvalid,       AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_TIMEARRAY,       &writeDetail::dummyTimeArray2,6,       AwaResourceType_TimeArray},

          writeDetail::TestWriteResource {true, false, AwaError_Success,          AwaError_Success,          writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_OBJECTLINKARRAY, &writeDetail::dummyObjectLinkArray1,3, AwaResourceType_ObjectLinkArray},
          writeDetail::TestWriteResource {true, true, AwaError_Success,          AwaError_Success,          writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_OBJECTLINKARRAY, &writeDetail::dummyObjectLinkArray1,3, AwaResourceType_ObjectLinkArray},
          writeDetail::TestWriteResource {true, false, AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,                    0,                  writeDetail::TEST_RESOURCE_OBJECTLINKARRAY, &writeDetail::dummyObjectLinkArray1,3, AwaResourceType_ObjectLinkArray},
          writeDetail::TestWriteResource {true, false, AwaError_TypeMismatch,     AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_STRINGARRAY,     &writeDetail::dummyObjectLinkArray1,3, AwaResourceType_ObjectLinkArray},
          writeDetail::TestWriteResource {false, false, AwaError_OperationInvalid, AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_OBJECTLINKARRAY, &writeDetail::dummyObjectLinkArray1,3, AwaResourceType_ObjectLinkArray},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  AWA_INVALID_ID,                         &writeDetail::dummyObjectLinkArray1,3, AwaResourceType_ObjectLinkArray},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid, writeDetail::TEST_OBJECT_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,                         &writeDetail::dummyObjectLinkArray1,3, AwaResourceType_ObjectLinkArray},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,                    AWA_INVALID_ID, AWA_INVALID_ID,                         &writeDetail::dummyObjectLinkArray1,3, AwaResourceType_ObjectLinkArray},
          writeDetail::TestWriteResource {true, false, AwaError_AddInvalid,          AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_OBJECTLINKARRAY, &writeDetail::dummyObjectLinkArray2,6, AwaResourceType_ObjectLinkArray}
        ));

class TestWriteValueSingle : public TestWriteValue {};

TEST_P(TestWriteValueSingle, TestWriteValueSingle)
{
    writeDetail::TestWriteResource data = GetParam();
    AwaServerWriteOperation * writeOperation = data.CreateSetOperation? AwaServerWriteOperation_New(serverSession_, AwaWriteMode_Update) : NULL;

    char path[128] = {0};

    if(data.ObjectID == AWA_INVALID_ID)
    {
        sprintf(path, "a/n in/valid/ path");
    }
    else
    {
        EXPECT_EQ(AwaError_Success, AwaAPI_MakePath(path, sizeof(path), data.ObjectID, data.ObjectInstanceID, data.ResourceID));
    }

    switch(data.Type)
    {
        case AwaResourceType_String:
            ASSERT_EQ(data.ExpectedAddResult, AwaServerWriteOperation_AddValueAsCString(writeOperation, path, (const char *)data.Value));
            break;
        case AwaResourceType_Integer:
            ASSERT_EQ(data.ExpectedAddResult, AwaServerWriteOperation_AddValueAsInteger(writeOperation, path, *((AwaInteger*)data.Value)));
            break;
        case AwaResourceType_Float:
            ASSERT_EQ(data.ExpectedAddResult, AwaServerWriteOperation_AddValueAsFloat(writeOperation, path, *((AwaFloat*)data.Value)));
            break;
        case AwaResourceType_Boolean:
            ASSERT_EQ(data.ExpectedAddResult, AwaServerWriteOperation_AddValueAsBoolean(writeOperation, path, *((AwaBoolean*)data.Value)));
            break;
        case AwaResourceType_Opaque:
            ASSERT_EQ(data.ExpectedAddResult, AwaServerWriteOperation_AddValueAsOpaque(writeOperation, path, *((AwaOpaque*)data.Value)));
            break;
        case AwaResourceType_Time:
            ASSERT_EQ(data.ExpectedAddResult, AwaServerWriteOperation_AddValueAsTime(writeOperation, path, *((AwaTime*)data.Value)));
            break;
        case AwaResourceType_ObjectLink:
            ASSERT_EQ(data.ExpectedAddResult, AwaServerWriteOperation_AddValueAsObjectLink(writeOperation, path, *((AwaObjectLink*)data.Value)));
            break;
        default:
            ASSERT_TRUE(false);
            break;
    }

    ASSERT_EQ(data.ExpectedProcessResult, AwaServerWriteOperation_Perform(writeOperation, writeDetail::clientID, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    if (data.ExpectedProcessResult == AwaError_Success)
    {
        // Confirm the value was set correctly
        AwaClientGetOperation * getOperation = AwaClientGetOperation_New(clientSession_);
        ASSERT_TRUE(NULL != getOperation);
        ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, path));
        ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
        const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
        ASSERT_TRUE(NULL != getResponse);

        void * value = NULL;

        AwaObjectLink receivedObjectLink = {0, 0};
        AwaOpaque receivedOpaque = {NULL, 0};

        switch(data.Type)
        {
        case AwaResourceType_String:
            ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsCStringPointer(getResponse, path, (const char **)&value));
            break;
        case AwaResourceType_Integer:
            ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsIntegerPointer(getResponse, path, (const AwaInteger **)&value));
            break;
        case AwaResourceType_Float:
            ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsFloatPointer(getResponse, path, (const AwaFloat **)&value));
            break;
        case AwaResourceType_Boolean:
            ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsBooleanPointer(getResponse, path, (const AwaBoolean **)&value));
            break;
        case AwaResourceType_Opaque:
            ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsOpaque(getResponse, path, &receivedOpaque));
            ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsOpaquePointer(getResponse, path, (const AwaOpaque **)&value));
            break;
        case AwaResourceType_Time:
            ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsTimePointer(getResponse, path, (const AwaTime **)&value));
            break;
        case AwaResourceType_ObjectLink:
            ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsObjectLink(getResponse, path, &receivedObjectLink));
            ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsObjectLinkPointer(getResponse, path, (const AwaObjectLink **)&value));
            break;
        default:
            ASSERT_TRUE(false);
            break;
        }


        switch(data.Type)
        {
            case AwaResourceType_String:
            {
                ASSERT_STREQ((char*) data.Value, (char*) value);
                break;
            }
            case AwaResourceType_Opaque:
            {
                AwaOpaque * expectedOpaque = (AwaOpaque *) data.Value;
                AwaOpaque * receivedOpaquePointer = (AwaOpaque *) value;

                ASSERT_EQ(expectedOpaque->Size, receivedOpaquePointer->Size);
                ASSERT_EQ(expectedOpaque->Size, receivedOpaque.Size);
                ASSERT_EQ(0, memcmp(expectedOpaque->Data, receivedOpaquePointer->Data, expectedOpaque->Size));
                ASSERT_EQ(0, memcmp(expectedOpaque->Data, receivedOpaque.Data, receivedOpaque.Size));
                break;
            }
            case AwaResourceType_Integer:
                ASSERT_EQ(*static_cast<const AwaInteger *>(data.Value), *static_cast<AwaInteger *>(value));
                break;
            case AwaResourceType_Float:
                ASSERT_EQ(*static_cast<const AwaFloat *>(data.Value), *static_cast<AwaFloat *>(value));
                break;
            case AwaResourceType_Boolean:
                ASSERT_EQ(*static_cast<const AwaBoolean *>(data.Value), *static_cast<AwaBoolean *>(value));
                break;
            case AwaResourceType_Time:
                ASSERT_EQ(*static_cast<const AwaTime *>(data.Value), *static_cast<AwaTime *>(value));
                break;
            case AwaResourceType_ObjectLink:
            {
                const AwaObjectLink * expectedObjectLink = static_cast<const AwaObjectLink *>(data.Value);
                const AwaObjectLink * receivedObjectLinkPointer = static_cast<AwaObjectLink *>(value);
                ASSERT_EQ(0, memcmp(expectedObjectLink, receivedObjectLinkPointer, sizeof(AwaObjectLink)));
                ASSERT_EQ(0, memcmp(expectedObjectLink, &receivedObjectLink, sizeof(AwaObjectLink)));
                break;
            }
            default:
                ASSERT_TRUE(false);
        }


        AwaClientGetOperation_Free(&getOperation);
    }
}

INSTANTIATE_TEST_CASE_P(
        TestWriteValueSingle,
        TestWriteValueSingle,
        ::testing::Values(
          writeDetail::TestWriteResource {true, false, AwaError_Success,          AwaError_Success,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_STRING,     writeDetail::dummyString1,1,     AwaResourceType_String},
          writeDetail::TestWriteResource {true, false, AwaError_NotDefined,       AwaError_OperationInvalid,          AWA_MAX_ID - 1,                   0,                  writeDetail::TEST_RESOURCE_STRING,     writeDetail::dummyString1,1,     AwaResourceType_String},
          writeDetail::TestWriteResource {true, false, AwaError_TypeMismatch,     AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_INTEGER,    writeDetail::dummyString1,1,     AwaResourceType_String},
          writeDetail::TestWriteResource {false, false, AwaError_OperationInvalid, AwaError_OperationInvalid, writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_STRING,     writeDetail::dummyString1,1,     AwaResourceType_String},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  AWA_INVALID_ID,               writeDetail::dummyString1,1,     AwaResourceType_String},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,               writeDetail::dummyString1,1,     AwaResourceType_String},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid,          AWA_INVALID_ID,                   AWA_INVALID_ID, AWA_INVALID_ID,               writeDetail::dummyString1,1,     AwaResourceType_String},

          writeDetail::TestWriteResource {true, false, AwaError_Success,          AwaError_Success,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_INTEGER,    &writeDetail::dummyInteger1,1,    AwaResourceType_Integer},
          writeDetail::TestWriteResource {true, false, AwaError_NotDefined,       AwaError_OperationInvalid,          AWA_MAX_ID - 1,                   0,                  writeDetail::TEST_RESOURCE_INTEGER,    &writeDetail::dummyInteger1,1,    AwaResourceType_Integer},
          writeDetail::TestWriteResource {true, false, AwaError_TypeMismatch,     AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_STRING,     &writeDetail::dummyInteger1,1,    AwaResourceType_Integer},
          writeDetail::TestWriteResource {false, false, AwaError_OperationInvalid, AwaError_OperationInvalid, writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_INTEGER,    &writeDetail::dummyInteger1,1,    AwaResourceType_Integer},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  AWA_INVALID_ID,               &writeDetail::dummyInteger1,1,    AwaResourceType_Integer},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,               &writeDetail::dummyInteger1,1,    AwaResourceType_Integer},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid,          AWA_INVALID_ID,                   AWA_INVALID_ID, AWA_INVALID_ID,               &writeDetail::dummyInteger1,1,    AwaResourceType_Integer},

          writeDetail::TestWriteResource {true, false, AwaError_Success,          AwaError_Success,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_FLOAT,      &writeDetail::dummyFloat1,1,      AwaResourceType_Float},
          writeDetail::TestWriteResource {true, false, AwaError_NotDefined,       AwaError_OperationInvalid,          AWA_MAX_ID - 1,                   0,                  writeDetail::TEST_RESOURCE_FLOAT,      &writeDetail::dummyFloat1,1,      AwaResourceType_Float},
          writeDetail::TestWriteResource {true, false, AwaError_TypeMismatch,     AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_STRING,     &writeDetail::dummyFloat1,1,      AwaResourceType_Float},
          writeDetail::TestWriteResource {false, false, AwaError_OperationInvalid, AwaError_OperationInvalid, writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_FLOAT,      &writeDetail::dummyFloat1,1,      AwaResourceType_Float},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  AWA_INVALID_ID,               &writeDetail::dummyFloat1,1,      AwaResourceType_Float},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,               &writeDetail::dummyFloat1,1,      AwaResourceType_Float},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid,          AWA_INVALID_ID,                   AWA_INVALID_ID, AWA_INVALID_ID,               &writeDetail::dummyFloat1,1,      AwaResourceType_Float},

          writeDetail::TestWriteResource {true, false, AwaError_Success,          AwaError_Success,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_BOOLEAN,    &writeDetail::dummyBoolean1,1,    AwaResourceType_Boolean},
          writeDetail::TestWriteResource {true, false, AwaError_NotDefined,       AwaError_OperationInvalid,          AWA_MAX_ID - 1,                   0,                  writeDetail::TEST_RESOURCE_BOOLEAN,    &writeDetail::dummyBoolean1,1,    AwaResourceType_Boolean},
          writeDetail::TestWriteResource {true, false, AwaError_TypeMismatch,     AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_STRING,     &writeDetail::dummyBoolean1,1,    AwaResourceType_Boolean},
          writeDetail::TestWriteResource {false, false, AwaError_OperationInvalid, AwaError_OperationInvalid, writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_BOOLEAN,    &writeDetail::dummyBoolean1,1,    AwaResourceType_Boolean},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  AWA_INVALID_ID,               &writeDetail::dummyBoolean1,1,    AwaResourceType_Boolean},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,               &writeDetail::dummyBoolean1,1,    AwaResourceType_Boolean},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid,          AWA_INVALID_ID,                   AWA_INVALID_ID, AWA_INVALID_ID,               &writeDetail::dummyBoolean1,1,    AwaResourceType_Boolean},

          writeDetail::TestWriteResource {true, false, AwaError_Success,          AwaError_Success,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_OPAQUE,     &writeDetail::dummyOpaque1,1,     AwaResourceType_Opaque},
          writeDetail::TestWriteResource {true, false, AwaError_NotDefined,       AwaError_OperationInvalid,          AWA_MAX_ID - 1,                   0,                  writeDetail::TEST_RESOURCE_OPAQUE,     &writeDetail::dummyOpaque1,1,     AwaResourceType_Opaque},
          writeDetail::TestWriteResource {true, false, AwaError_TypeMismatch,     AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_STRING,     &writeDetail::dummyOpaque1,1,     AwaResourceType_Opaque},
          writeDetail::TestWriteResource {false, false, AwaError_OperationInvalid, AwaError_OperationInvalid, writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_OPAQUE,     &writeDetail::dummyOpaque1,1,     AwaResourceType_Opaque},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  AWA_INVALID_ID,               &writeDetail::dummyOpaque1,1,     AwaResourceType_Opaque},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,               &writeDetail::dummyOpaque1,1,     AwaResourceType_Opaque},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid,          AWA_INVALID_ID,                   AWA_INVALID_ID, AWA_INVALID_ID,               &writeDetail::dummyOpaque1,1,     AwaResourceType_Opaque},

          writeDetail::TestWriteResource {true, false, AwaError_Success,          AwaError_Success,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_TIME,       &writeDetail::dummyTime1,1,       AwaResourceType_Time},
          writeDetail::TestWriteResource {true, false, AwaError_NotDefined,       AwaError_OperationInvalid,          AWA_MAX_ID - 1,                   0,                  writeDetail::TEST_RESOURCE_TIME,       &writeDetail::dummyTime1,1,       AwaResourceType_Time},
          writeDetail::TestWriteResource {true, false, AwaError_TypeMismatch,     AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_STRING,     &writeDetail::dummyTime1,1,       AwaResourceType_Time},
          writeDetail::TestWriteResource {false, false, AwaError_OperationInvalid, AwaError_OperationInvalid, writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_TIME,       &writeDetail::dummyTime1,1,       AwaResourceType_Time},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  AWA_INVALID_ID,               &writeDetail::dummyTime1,1,       AwaResourceType_Time},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,               &writeDetail::dummyTime1,1,       AwaResourceType_Time},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid,          AWA_INVALID_ID,                   AWA_INVALID_ID, AWA_INVALID_ID,               &writeDetail::dummyTime1,1,       AwaResourceType_Time},

          writeDetail::TestWriteResource {true, false, AwaError_Success,          AwaError_Success,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_OBJECTLINK, &writeDetail::dummyObjectLink1,1, AwaResourceType_ObjectLink},
          writeDetail::TestWriteResource {true, false, AwaError_NotDefined,       AwaError_OperationInvalid,          AWA_MAX_ID - 1,                   0,                  writeDetail::TEST_RESOURCE_OBJECTLINK, &writeDetail::dummyObjectLink1,1, AwaResourceType_ObjectLink},
          writeDetail::TestWriteResource {true, false, AwaError_TypeMismatch,     AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_STRING,     &writeDetail::dummyObjectLink1,1, AwaResourceType_ObjectLink},
          writeDetail::TestWriteResource {false, false, AwaError_OperationInvalid, AwaError_OperationInvalid, writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  writeDetail::TEST_RESOURCE_OBJECTLINK, &writeDetail::dummyObjectLink1,1, AwaResourceType_ObjectLink},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  AWA_INVALID_ID,               &writeDetail::dummyObjectLink1,1, AwaResourceType_ObjectLink},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid,          writeDetail::TEST_OBJECT_NON_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,               &writeDetail::dummyObjectLink1,1, AwaResourceType_ObjectLink},
          writeDetail::TestWriteResource {true, false, AwaError_PathInvalid,      AwaError_OperationInvalid,          AWA_INVALID_ID,                   AWA_INVALID_ID, AWA_INVALID_ID,               &writeDetail::dummyObjectLink1,1, AwaResourceType_ObjectLink}

        ));

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_NewClientIterator_handles_null)
{
    EXPECT_EQ(NULL, AwaServerWriteOperation_NewClientIterator(NULL));
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_NewClientIterator_with_no_perform)
{
    AwaServerWriteOperation * operation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    // no perform
    AwaClientIterator * iterator = AwaServerWriteOperation_NewClientIterator(operation);
    EXPECT_EQ(NULL, iterator);
    AwaServerWriteOperation_Free(&operation);
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_handles_no_clients)
{
    AwaServerWriteOperation * operation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    const char * clientID = "TestClient123";
    const char * path = "/3/0/9";
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(operation, path, 42));
    EXPECT_EQ(AwaError_Response, AwaServerWriteOperation_Perform(operation, clientID, defaults::timeout));

    // expect the client ID to be in the response, but with an error on the specified path
    AwaClientIterator * iterator = AwaServerWriteOperation_NewClientIterator(operation);
    EXPECT_TRUE(NULL != iterator);
    EXPECT_TRUE(AwaClientIterator_Next(iterator));
    EXPECT_STREQ(clientID, AwaClientIterator_GetClientID(iterator));
    EXPECT_FALSE(AwaClientIterator_Next(iterator));   // only one client

    // should be an error:
    const AwaServerWriteResponse * response = AwaServerWriteOperation_GetResponse(operation, clientID);
    ASSERT_TRUE(NULL != response);
    const AwaPathResult * pathResult = AwaServerWriteResponse_GetPathResult(response, path);
    EXPECT_TRUE(NULL != pathResult);
    EXPECT_EQ(AwaError_ClientNotFound, AwaPathResult_GetError(pathResult));

    AwaClientIterator_Free(&iterator);
    AwaServerWriteOperation_Free(&operation);
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_handles_one_client)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    sleep(1);      // wait for the client to register with the server

    AwaServerWriteOperation * operation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsCString(operation, "/3/0/15", "Europe/London"));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(operation, "TestClient1", defaults::timeout));

    AwaClientIterator * iterator = AwaServerWriteOperation_NewClientIterator(operation);
    EXPECT_TRUE(NULL != iterator);
    EXPECT_EQ(NULL, AwaClientIterator_GetClientID(iterator));
    EXPECT_TRUE(AwaClientIterator_Next(iterator));
    EXPECT_STREQ("TestClient1", AwaClientIterator_GetClientID(iterator));
    EXPECT_FALSE(AwaClientIterator_Next(iterator));   // only one client

    AwaClientIterator_Free(&iterator);
    AwaServerWriteOperation_Free(&operation);
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_handles_LWM2M_error)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    sleep(1);      // wait for the client to register with the server

    AwaServerWriteOperation * operation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(operation, "/3/0/9", 53));
    EXPECT_EQ(AwaError_Response, AwaServerWriteOperation_Perform(operation, "TestClient1", defaults::timeout));

    // resource is read-only, should see a LWM2M error (MethodNotAllowed):
    const AwaServerWriteResponse * response = AwaServerWriteOperation_GetResponse(operation, "TestClient1");
    ASSERT_TRUE(NULL != response);
    const AwaPathResult * pathResult = AwaServerWriteResponse_GetPathResult(response, "/3/0/9");
    EXPECT_TRUE(NULL != pathResult);
    EXPECT_EQ(AwaError_LWM2MError, AwaPathResult_GetError(pathResult));
    EXPECT_EQ(AwaLWM2MError_MethodNotAllowed, AwaPathResult_GetLWM2MError(pathResult));

    AwaServerWriteOperation_Free(&operation);
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteResponse_NewPathIterator_handles_null)
{
    EXPECT_EQ(NULL, AwaServerWriteResponse_NewPathIterator(NULL));
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteResponse_NewPathIterator_handles_valid_response)
{
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    sleep(1);      // wait for the client to register with the server

    AwaServerWriteOperation * operation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsCString(operation, "/3/0/15", "Europe/London"));  // expect Success
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(operation, "TestClient1", defaults::timeout));
    const AwaServerWriteResponse * response = AwaServerWriteOperation_GetResponse(operation, "TestClient1");
    EXPECT_TRUE(NULL != response);
    AwaPathIterator * pathIterator =  AwaServerWriteResponse_NewPathIterator(response);
    EXPECT_TRUE(NULL != pathIterator);
    EXPECT_TRUE(AwaPathIterator_Next(pathIterator));
    const char * path = AwaPathIterator_Get(pathIterator);
    const AwaPathResult * pathResult = AwaServerWriteResponse_GetPathResult(response, path);
    EXPECT_STREQ("/3/0/15", path);
    EXPECT_EQ(AwaError_Success, AwaPathResult_GetError(pathResult));
    AwaPathIterator_Free(&pathIterator);
    AwaServerWriteOperation_Free(&operation);
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_handles_multiple_errors)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    sleep(1);      // wait for the client to register with the server

    AwaServerWriteOperation * operation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);

    // NOTE: because the server sends these three resources as a single CoAP message,
    // the write to /3/0/15 reports an error, even though it may have succeeded.
    // This is a flaw with LWM2M and is difficult to avoid.
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(operation, "/3/0/9", 53));      // expect LWM2M error
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsCString(operation, "/3/0/0", "ACME"));  // expect LWM2M error
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsCString(operation, "/3/0/15", "Europe/London"));  // expect LWM2M error

    // at least one error occurred:
    EXPECT_EQ(AwaError_Response, AwaServerWriteOperation_Perform(operation, "TestClient1", defaults::timeout));

    // only one client:
    AwaClientIterator * clientIterator = AwaServerWriteOperation_NewClientIterator(operation);
    EXPECT_TRUE(AwaClientIterator_Next(clientIterator));
    EXPECT_STREQ("TestClient1", AwaClientIterator_GetClientID(clientIterator));

    // check response
    const AwaServerWriteResponse * response = AwaServerWriteOperation_GetResponse(operation, "TestClient1");
    EXPECT_TRUE(NULL != response);
    AwaPathIterator * pathIterator =  AwaServerWriteResponse_NewPathIterator(response);
    EXPECT_TRUE(NULL != pathIterator);

    typedef std::tuple<std::string, AwaError> PathResult;
    std::vector<PathResult> expectedResults {
        PathResult { "/3/0/0", AwaError_LWM2MError },
        PathResult { "/3/0/9", AwaError_LWM2MError },
        PathResult { "/3/0/15", AwaError_LWM2MError },  // error - see note above
    };
    std::vector<PathResult> actualResults;

    while (AwaPathIterator_Next(pathIterator))
    {
        const char * path = AwaPathIterator_Get(pathIterator);
        const AwaPathResult * pathResult = AwaServerWriteResponse_GetPathResult(response, path);
        actualResults.push_back(PathResult { path, AwaPathResult_GetError(pathResult) } );
    }

//    for (auto it = actualResults.begin(); it != actualResults.end(); ++it)
//    {
//        std::cout << std::get<0>(*it) << ", " << std::get<1>(*it) << std::endl;
//    }

    EXPECT_EQ(expectedResults.size(), actualResults.size());
    if (expectedResults.size() == actualResults.size())
        EXPECT_TRUE(std::is_permutation(expectedResults.begin(), expectedResults.end(), actualResults.begin()));

    EXPECT_FALSE(AwaClientIterator_Next(clientIterator));   // only one client
    AwaClientIterator_Free(&clientIterator);
    AwaPathIterator_Free(&pathIterator);
    AwaServerWriteOperation_Free(&operation);
}

TEST_F(TestWriteOperationWithConnectedSession, AwaServerWriteOperation_handles_multiple_successes)
{
    // start a client and wait for them to register with the server
    AwaClientDaemonHorde horde( { "TestClient1" }, 61000);
    sleep(1);      // wait for the client to register with the server

    AwaServerWriteOperation * operation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);

    // NOTE: because the server sends these three resources as a single CoAP message,
    // the write to /3/0/15 reports an error, even though it may have succeeded.
    // This is a flaw with LWM2M and is difficult to avoid.
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsTime(operation, "/3/0/13", 12345678));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsCString(operation, "/3/0/14", "+1:00"));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsCString(operation, "/3/0/15", "Europe/London"));

    // no errors expected
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(operation, "TestClient1", defaults::timeout));

    // only one client:
    AwaClientIterator * clientIterator = AwaServerWriteOperation_NewClientIterator(operation);
    EXPECT_TRUE(AwaClientIterator_Next(clientIterator));
    EXPECT_STREQ("TestClient1", AwaClientIterator_GetClientID(clientIterator));

    // check response
    const AwaServerWriteResponse * response = AwaServerWriteOperation_GetResponse(operation, "TestClient1");
    EXPECT_TRUE(NULL != response);
    AwaPathIterator * pathIterator =  AwaServerWriteResponse_NewPathIterator(response);
    EXPECT_TRUE(NULL != pathIterator);

    typedef std::tuple<std::string, AwaError> PathResult;
    std::vector<PathResult> expectedResults {
        PathResult { "/3/0/13", AwaError_Success },
        PathResult { "/3/0/14", AwaError_Success },
        PathResult { "/3/0/15", AwaError_Success },
    };
    std::vector<PathResult> actualResults;

    while (AwaPathIterator_Next(pathIterator))
    {
        const char * path = AwaPathIterator_Get(pathIterator);
        const AwaPathResult * pathResult = AwaServerWriteResponse_GetPathResult(response, path);
        actualResults.push_back(PathResult { path, AwaPathResult_GetError(pathResult) } );
    }

//    for (auto it = actualResults.begin(); it != actualResults.end(); ++it)
//    {
//        std::cout << std::get<0>(*it) << ", " << std::get<1>(*it) << std::endl;
//    }

    EXPECT_EQ(expectedResults.size(), actualResults.size());
    if (expectedResults.size() == actualResults.size())
        EXPECT_TRUE(std::is_permutation(expectedResults.begin(), expectedResults.end(), actualResults.begin()));

    EXPECT_FALSE(AwaClientIterator_Next(clientIterator));   // only one client
    AwaClientIterator_Free(&clientIterator);
    AwaPathIterator_Free(&pathIterator);
    AwaServerWriteOperation_Free(&operation);
}




TEST_F(TestWriteOperation, AwaWriteMode_Strings_are_consistent)
{
    ASSERT_EQ(static_cast<size_t>(AwaWriteMode_LAST), WriteMode_GetNumberOfWriteModeStrings());
}

TEST_F(TestWriteOperation, AwaWriteMode_ToString)
{
    EXPECT_STREQ("AwaWriteMode_Replace", AwaWriteMode_ToString(AwaWriteMode_Replace));
    EXPECT_STREQ("AwaWriteMode_Update", AwaWriteMode_ToString(AwaWriteMode_Update));
}

TEST_F(TestWriteOperation, AwaWriteMode_FromString)
{
    EXPECT_EQ(AwaWriteMode_Replace, AwaWriteMode_FromString("AwaWriteMode_Replace"));
    EXPECT_EQ(AwaWriteMode_Update, AwaWriteMode_FromString("AwaWriteMode_Update"));
}



} // namespace Awa


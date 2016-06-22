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

#include <pthread.h>
#include <gtest/gtest.h>
#include "awa/static.h"
#include "awa/server.h"
#include "support/static_api_support.h"

namespace Awa {

class TestStaticClientCreateDeleteWithServer : public TestStaticClientWithServer {};

TEST_F(TestStaticClientCreateDeleteWithServer, AwaStaticClient_CreateDelete_Object_valid_inputs)
{
    AwaInteger i = 0;
    EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineObject(client_, 7997, "TestObject", 0, 1)); // valid
    EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResource(client_, 7997,  1, "Resource", AwaResourceType_Integer, 0, 1, AwaResourceOperations_ReadWrite));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetResourceStorageWithPointer(client_, 7997,  1, &i, sizeof(i), 0));

    SingleStaticClientWaitCondition bootstrap_condition(client_, session_, global::clientEndpointName, global::timeout);
    EXPECT_TRUE(bootstrap_condition.Wait());

    ASSERT_EQ(AwaError_Success, AwaStaticClient_CreateObjectInstance(client_, 7997, 0));

    SingleStaticClientObjectWaitCondition create_condition(client_, session_, global::clientEndpointName, "/7997/0", global::timeout);
    ASSERT_TRUE(create_condition.Wait());

    ASSERT_EQ(AwaError_Success, AwaStaticClient_DeleteObjectInstance(client_, 7997, 0));

    SingleStaticClientObjectWaitCondition delete_condition(client_, session_, global::clientEndpointName, "/7997/0", global::timeout, true);
    ASSERT_TRUE(delete_condition.Wait());
}

TEST_F(TestStaticClientCreateDeleteWithServer, AwaStaticClient_CreateDelete_invalid_inputs)
{
    AwaInteger i = 0;
    EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineObject(client_, 7997, "TestObject", 0, 1)); // valid
    EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResource(client_, 7997,  1, "Resource", AwaResourceType_Integer, 0, 1, AwaResourceOperations_ReadWrite));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetResourceStorageWithPointer(client_, 7997,  1, &i, sizeof(i), 0));

    ASSERT_EQ(AwaError_CannotCreate , AwaStaticClient_CreateObjectInstance(client_, 7987, 0));
    ASSERT_EQ(AwaError_CannotCreate , AwaStaticClient_CreateObjectInstance(client_, 7997, 1));
    ASSERT_EQ(AwaError_CannotDelete, AwaStaticClient_DeleteObjectInstance(client_, 7997, 0));
    ASSERT_EQ(AwaError_CannotDelete, AwaStaticClient_DeleteObjectInstance(client_, 7987, 0));
    ASSERT_EQ(AwaError_CannotDelete, AwaStaticClient_DeleteObjectInstance(client_, 7997, 1));

    ASSERT_EQ(AwaError_CannotCreate, AwaStaticClient_CreateResource(client_, 7997, 0, 1));

    EXPECT_EQ(AwaError_Success , AwaStaticClient_CreateObjectInstance(client_, 7997, 0));

    ASSERT_EQ(AwaError_CannotDelete, AwaStaticClient_DeleteResource(client_, 7997, 0, 1));

    ASSERT_EQ(AwaError_CannotCreate, AwaStaticClient_CreateResource(client_, 7997, 0, 2));

    EXPECT_EQ(AwaError_Success, AwaStaticClient_CreateResource(client_, 7997, 0, 1));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_DeleteResource(client_, 7997, 0, 1));
    ASSERT_EQ(AwaError_CannotDelete, AwaStaticClient_DeleteResource(client_, 7997, 0, 1));

    EXPECT_EQ(AwaError_Success, AwaStaticClient_DeleteObjectInstance(client_, 7997, 0));
    ASSERT_EQ(AwaError_CannotDelete, AwaStaticClient_DeleteObjectInstance(client_, 7997, 0));
}

TEST_F(TestStaticClientCreateDeleteWithServer, AwaStaticClient_CreateDelete_Resource_valid_inputs)
{
    AwaInteger i = 9999;
    EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineObject(client_, 7997, "TestObject", 0, 1)); // valid
    EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResource(client_, 7997, 1, "Resource", AwaResourceType_Integer, 0, 1, AwaResourceOperations_ReadWrite));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetResourceStorageWithPointer(client_, 7997,  1, &i, sizeof(i), 0));


    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(7997, "TestObject", 0, 1);
    EXPECT_TRUE(NULL != objectDefinition);

    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefinition, 1, "Resource", false, AwaResourceOperations_ReadWrite, static_cast<AwaInteger>(0)));


    AwaServerDefineOperation * defineOperation = AwaServerDefineOperation_New(session_);
    EXPECT_TRUE(NULL != defineOperation);

    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(defineOperation, objectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(defineOperation, global::timeout));

    AwaServerDefineOperation_Free(&defineOperation);
    AwaObjectDefinition_Free(&objectDefinition);

    SingleStaticClientWaitCondition bootstrap_condition(client_, session_, global::clientEndpointName, global::timeout);
    EXPECT_TRUE(bootstrap_condition.Wait());

    EXPECT_EQ(AwaError_Success, AwaStaticClient_CreateObjectInstance(client_, 7997, 0));

    SingleStaticClientObjectWaitCondition create_condition(client_, session_, global::clientEndpointName, "/7997/0", global::timeout);
    EXPECT_TRUE(create_condition.Wait());

    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(session_);
    EXPECT_TRUE(NULL != readOperation);

    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/7997/0/1"));

    ASSERT_EQ(AwaError_Success, AwaStaticClient_CreateResource(client_, 7997, 0, 1));

    StaticClientProccessInfo processInfo = { .Run = true, .StaticClient = client_ };
    pthread_t processThread;
    pthread_create(&processThread, NULL, do_static_client_process, &processInfo);
    AwaServerReadOperation_Perform(readOperation, global::timeout * 20);
    processInfo.Run = false;
    pthread_join(processThread, NULL);

    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    EXPECT_TRUE(NULL != readResponse);

    const AwaInteger * responseValue = NULL;
    EXPECT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsIntegerPointer(readResponse, "/7997/0/1", &responseValue));
    ASSERT_TRUE(NULL != responseValue);
    ASSERT_EQ(i, *responseValue);

    ASSERT_EQ(AwaError_Success, AwaStaticClient_DeleteResource(client_, 7997, 0, 1));

    processInfo.Run = true;
    pthread_create(&processThread, NULL, do_static_client_process, &processInfo);
    AwaServerReadOperation_Perform(readOperation, global::timeout * 20);
    processInfo.Run = false;
    pthread_join(processThread, NULL);

    readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    EXPECT_TRUE(NULL != readResponse);

    ASSERT_EQ(AwaLWM2MError_NotFound, AwaPathResult_GetLWM2MError(AwaServerReadResponse_GetPathResult(readResponse, "/7997/0/1")));
    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestStaticClientCreateDeleteWithServer, AwaStaticClient_CreateDelete_mandatory_without_storage)
{
    EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineObject(client_, 7997, "TestObject", 0, 1)); // valid
    EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResource(client_, 7997,  1, "Resource", AwaResourceType_Integer, 1, 1, AwaResourceOperations_ReadWrite));

    EXPECT_EQ(AwaError_CannotCreate, AwaStaticClient_CreateObjectInstance(client_, 7997, 0));
}

TEST_F(TestStaticClientCreateDeleteWithServer, AwaStaticClient_CreateDelete_optional_without_storage)
{
    EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineObject(client_, 7997, "TestObject", 0, 1)); // valid
    EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResource(client_, 7997,  1, "Resource", AwaResourceType_Integer, 0, 1, AwaResourceOperations_ReadWrite));

    EXPECT_EQ(AwaError_Success, AwaStaticClient_CreateObjectInstance(client_, 7997, 0));
    EXPECT_EQ(AwaError_CannotCreate, AwaStaticClient_CreateResource(client_, 7997, 0, 1));
}


} // namespace Awa

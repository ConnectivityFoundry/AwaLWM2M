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

#include <awa/client.h>
#include <awa/common.h>
#include "log.h"
#include "get_response.h"
#include "support/support.h"
#include "support/definition.h"
#include "arrays.h"

namespace Awa {

class TestServerDefineDefaults : public TestClientBase {};

class TestServerDefineDefaultsWithDaemon : public TestServerAndClientWithConnectedSession {};


TEST_F(TestServerDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_string_resource)
{
    //Define our custom object
    AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(client_session_);
    AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(server_session_);
    EXPECT_TRUE(clientDefineOperation != NULL);
    EXPECT_TRUE(serverDefineOperation != NULL);

    const char * expected = "abcdefg";
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsString(customObjectDefinition, 0, "Test Resource", true, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, defaults::timeout));
    ASSERT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&clientDefineOperation);
    AwaServerDefineOperation_Free(&serverDefineOperation);

    WaitForClientDefinition(customObjectID);

    // Create a write operation to create the custom object.
    // The write operation should add the defaults to the CoAP create packet.
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(client_session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/10000/0/0"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    const char * value = NULL;
    ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsCStringPointer(getResponse, "/10000/0/0", &value));
    ASSERT_TRUE(NULL != value);
    EXPECT_STREQ(expected, value);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestServerDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_integer_resource)
{
    //Define our custom object
    AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(client_session_);
    AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(server_session_);
    EXPECT_TRUE(clientDefineOperation != NULL);
    EXPECT_TRUE(serverDefineOperation != NULL);

    AwaInteger expected = 12345;
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(customObjectDefinition, 0, "Test Resource", true, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, defaults::timeout));
    ASSERT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&clientDefineOperation);
    AwaServerDefineOperation_Free(&serverDefineOperation);

    WaitForClientDefinition(customObjectID);

    // Create a write operation to create the custom object.
    // The write operation should add the defaults to the CoAP create packet.
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(client_session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/10000/0/0"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    const AwaInteger * value = NULL;
    ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsIntegerPointer(getResponse, "/10000/0/0", &value));
    ASSERT_TRUE(NULL != value);
    EXPECT_EQ(expected, *value);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestServerDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_float_resource)
{
    //Define our custom object
    AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(client_session_);
    AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(server_session_);
    EXPECT_TRUE(clientDefineOperation != NULL);
    EXPECT_TRUE(serverDefineOperation != NULL);

    AwaFloat expected = 123.321;
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloat(customObjectDefinition, 0, "Test Resource", true, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, defaults::timeout));
    ASSERT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&clientDefineOperation);
    AwaServerDefineOperation_Free(&serverDefineOperation);

    WaitForClientDefinition(customObjectID);

    // Create a write operation to create the custom object.
    // The write operation should add the defaults to the CoAP create packet.
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(client_session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/10000/0/0"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    const AwaFloat * value = NULL;
    ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsFloatPointer(getResponse, "/10000/0/0", &value));
    ASSERT_TRUE(NULL != value);
    EXPECT_EQ(expected, *value);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestServerDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_boolean_resource)
{
    for (int i = 0; i < 2; i++)
    {
        //Define our custom object
        AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(client_session_);
        AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(server_session_);
        EXPECT_TRUE(clientDefineOperation != NULL);
        EXPECT_TRUE(serverDefineOperation != NULL);

        AwaBoolean expected = (i == 0);
        int customObjectID = 10000 + i;
        char instancePath[16];
        char resourcePath[16];
        sprintf(instancePath, "/%d/0", customObjectID);
        sprintf(resourcePath, "/%d/0/0", customObjectID);

        AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
        ASSERT_TRUE(NULL != customObjectDefinition);

        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBoolean(customObjectDefinition, 0, "Test Resource", true, AwaResourceOperations_ReadWrite, expected));
        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition));
        EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
        ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, defaults::timeout));
        ASSERT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, defaults::timeout));

        AwaObjectDefinition_Free(&customObjectDefinition);
        AwaClientDefineOperation_Free(&clientDefineOperation);
        AwaServerDefineOperation_Free(&serverDefineOperation);

        WaitForClientDefinition(customObjectID);

        // Create a write operation to create the custom object.
        // The write operation should add the defaults to the CoAP create packet.
        AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Update);
        ASSERT_TRUE(NULL != writeOperation);

        ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, instancePath));
        ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
        AwaServerWriteOperation_Free(&writeOperation);

        AwaClientGetOperation * getOperation = AwaClientGetOperation_New(client_session_);
        ASSERT_TRUE(NULL != getOperation);
        ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, resourcePath));
        ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
        const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
        ASSERT_TRUE(NULL != getResponse);
        const AwaBoolean * value = NULL;
        ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsBooleanPointer(getResponse, resourcePath, &value));
        ASSERT_TRUE(NULL != value);
        EXPECT_EQ(expected, *value);
        AwaClientGetOperation_Free(&getOperation);
    }
}

TEST_F(TestServerDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_opaque_resource)
{
    //Define our custom object
    AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(client_session_);
    AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(server_session_);
    EXPECT_TRUE(clientDefineOperation != NULL);
    EXPECT_TRUE(serverDefineOperation != NULL);

    const char dummyData[] = {'a', 0, 'x', 123, '4'};
    AwaOpaque expected {(void*)dummyData, sizeof(dummyData)};
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque(customObjectDefinition, 0, "Test Resource", true, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, defaults::timeout));
    ASSERT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&clientDefineOperation);
    AwaServerDefineOperation_Free(&serverDefineOperation);

    WaitForClientDefinition(customObjectID);

    // Create a write operation to create the custom object.
    // The write operation should add the defaults to the CoAP create packet.
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(client_session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/10000/0/0"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    AwaOpaque value = { 0 };
    ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsOpaque(getResponse, "/10000/0/0", &value));
    EXPECT_EQ(expected.Size, value.Size);
    ASSERT_TRUE(NULL != value.Data);
    EXPECT_EQ(0, memcmp(expected.Data, value.Data, expected.Size));
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestServerDefineDefaultsWithDaemon, AwaClient_get_empty_default_value_from_created_custom_object_instance_opaque_resource)
{
    //Define our custom object
    AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(client_session_);
    AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(server_session_);
    EXPECT_TRUE(clientDefineOperation != NULL);
    EXPECT_TRUE(serverDefineOperation != NULL);

    AwaOpaque defaultOpaque = (AwaOpaque){0};//{NULL, 0};
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque(customObjectDefinition, 0, "Test Resource", true, AwaResourceOperations_ReadWrite, defaultOpaque));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, defaults::timeout));
    ASSERT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&clientDefineOperation);
    AwaServerDefineOperation_Free(&serverDefineOperation);

    WaitForClientDefinition(customObjectID);

    // Create a write operation to create the custom object.
    // The write operation should add the defaults to the CoAP create packet.
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(client_session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/10000/0/0"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    AwaOpaque value = { 0 };
    ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsOpaque(getResponse, "/10000/0/0", &value));
    EXPECT_EQ(defaultOpaque.Size, value.Size);
    ASSERT_TRUE(NULL == value.Data);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestServerDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_time_resource)
{
    //Define our custom object
    AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(client_session_);
    AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(server_session_);
    EXPECT_TRUE(clientDefineOperation != NULL);
    EXPECT_TRUE(serverDefineOperation != NULL);

    AwaTime expected = 12324321;
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTime(customObjectDefinition, 0, "Test Resource", true, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, defaults::timeout));
    ASSERT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&clientDefineOperation);
    AwaServerDefineOperation_Free(&serverDefineOperation);

    WaitForClientDefinition(customObjectID);

    // Create a write operation to create the custom object.
    // The write operation should add the defaults to the CoAP create packet.
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(client_session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/10000/0/0"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    const AwaTime * value = NULL;
    ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsTimePointer(getResponse, "/10000/0/0", &value));
    ASSERT_TRUE(NULL != value);
    EXPECT_EQ(expected, *value);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestServerDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_objectlink_resource)
{
    //Define our custom object
    AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(client_session_);
    AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(server_session_);
    EXPECT_TRUE(clientDefineOperation != NULL);
    EXPECT_TRUE(serverDefineOperation != NULL);

    AwaObjectLink expected = {9, 7};
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLink(customObjectDefinition, 0, "Test Resource", true, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, defaults::timeout));
    ASSERT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&clientDefineOperation);
    AwaServerDefineOperation_Free(&serverDefineOperation);

    WaitForClientDefinition(customObjectID);

    // Create a write operation to create the custom object.
    // The write operation should add the defaults to the CoAP create packet.
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(client_session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/10000/0/0"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    AwaObjectLink value;
    ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsObjectLink(getResponse, "/10000/0/0", &value));
    EXPECT_EQ(expected.ObjectID, value.ObjectID);
    EXPECT_EQ(expected.ObjectInstanceID, value.ObjectInstanceID);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestServerDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_string_array_resource)
{
    //Define our custom object
    AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(client_session_);
    AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(server_session_);
    EXPECT_TRUE(clientDefineOperation != NULL);
    EXPECT_TRUE(serverDefineOperation != NULL);

    AwaStringArray * expected = AwaStringArray_New();
    AwaStringArray_SetValueAsCString(expected, 0u, "123456787");
    AwaStringArray_SetValueAsCString(expected, 1u, "987654321");
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsStringArray(customObjectDefinition, 0, "Test Resource", 2, 10, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, defaults::timeout));
    ASSERT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&clientDefineOperation);
    AwaServerDefineOperation_Free(&serverDefineOperation);

    WaitForClientDefinition(customObjectID);

    // Create a write operation to create the custom object.
    // The write operation should add the defaults to the CoAP create packet.
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(client_session_);

    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/10000/0/0"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    const AwaStringArray * value = NULL;
    ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsStringArrayPointer(getResponse, "/10000/0/0", &value));
    ASSERT_TRUE(NULL != value);
    EXPECT_EQ(0, Array_Compare((AwaArray *)expected, (AwaArray *)value, AwaResourceType_StringArray));

    AwaStringArray_Free(&expected);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestServerDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_integer_array_resource)
{
    //Define our custom object
    AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(client_session_);
    AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(server_session_);
    EXPECT_TRUE(clientDefineOperation != NULL);
    EXPECT_TRUE(serverDefineOperation != NULL);

    AwaIntegerArray * expected = AwaIntegerArray_New();
    AwaIntegerArray_SetValue(expected, 0u, 123456787);
    AwaIntegerArray_SetValue(expected, 1u, 987654321);
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsIntegerArray(customObjectDefinition, 0, "Test Resource", 2, 10, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, defaults::timeout));
    ASSERT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&clientDefineOperation);
    AwaServerDefineOperation_Free(&serverDefineOperation);

    // Create a write operation to create the custom object.
    // The write operation should add the defaults to the CoAP create packet.
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);

    WaitForClientDefinition(customObjectID);

    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(client_session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/10000/0/0"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    const AwaIntegerArray * value = NULL;
    ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsIntegerArrayPointer(getResponse, "/10000/0/0", &value));
    ASSERT_TRUE(NULL != value);
    EXPECT_EQ(0, Array_Compare((AwaArray *)expected, (AwaArray *)value, AwaResourceType_IntegerArray));

    AwaIntegerArray_Free(&expected);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestServerDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_float_array_resource)
{
    //Define our custom object
    AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(client_session_);
    AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(server_session_);
    EXPECT_TRUE(clientDefineOperation != NULL);
    EXPECT_TRUE(serverDefineOperation != NULL);

    AwaFloatArray * expected = AwaFloatArray_New();
    AwaFloatArray_SetValue(expected, 0u, 1234.5678);
    AwaFloatArray_SetValue(expected, 1u, 9876.5432);
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloatArray(customObjectDefinition, 0, "Test Resource", 2, 10, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, defaults::timeout));
    ASSERT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&clientDefineOperation);
    AwaServerDefineOperation_Free(&serverDefineOperation);

    WaitForClientDefinition(customObjectID);

    // Create a write operation to create the custom object.
    // The write operation should add the defaults to the CoAP create packet.
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(client_session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/10000/0/0"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    const AwaFloatArray * value = NULL;
    ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsFloatArrayPointer(getResponse, "/10000/0/0", &value));
    ASSERT_TRUE(NULL != value);
    EXPECT_EQ(0, Array_Compare((AwaArray *)expected, (AwaArray *)value, AwaResourceType_FloatArray));

    AwaFloatArray_Free(&expected);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestServerDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_boolean_array_resource)
{
    //Define our custom object
    AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(client_session_);
    AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(server_session_);
    EXPECT_TRUE(clientDefineOperation != NULL);
    EXPECT_TRUE(serverDefineOperation != NULL);

    AwaBooleanArray * expected = AwaBooleanArray_New();
    AwaBooleanArray_SetValue(expected, 0u, true);
    AwaBooleanArray_SetValue(expected, 1u, false);
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBooleanArray(customObjectDefinition, 0, "Test Resource", 2, 10, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, defaults::timeout));
    ASSERT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&clientDefineOperation);
    AwaServerDefineOperation_Free(&serverDefineOperation);

    WaitForClientDefinition(customObjectID);

    // Create a write operation to create the custom object.
    // The write operation should add the defaults to the CoAP create packet.
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(client_session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/10000/0/0"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    const AwaBooleanArray * value = NULL;
    ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsBooleanArrayPointer(getResponse, "/10000/0/0", &value));
    ASSERT_TRUE(NULL != value);
    EXPECT_EQ(0, Array_Compare((AwaArray *)expected, (AwaArray *)value, AwaResourceType_BooleanArray));

    AwaBooleanArray_Free(&expected);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestServerDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_opaque_array_resource)
{
    //Define our custom object
    AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(client_session_);
    AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(server_session_);
    EXPECT_TRUE(clientDefineOperation != NULL);
    EXPECT_TRUE(serverDefineOperation != NULL);

    const char dummyData1[] = {'a', 0, 'x', 123, '4'};
    const char dummyData2[] = {'b', 0, 's', 22};

    AwaOpaqueArray * expected = AwaOpaqueArray_New();
    AwaOpaqueArray_SetValue(expected, 0u, {(void*)dummyData1, sizeof(dummyData1)});
    AwaOpaqueArray_SetValue(expected, 1u, {(void*)dummyData2, sizeof(dummyData2)});
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaqueArray(customObjectDefinition, 0, "Test Resource", 2, 10, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, defaults::timeout));
    ASSERT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&clientDefineOperation);
    AwaServerDefineOperation_Free(&serverDefineOperation);

    WaitForClientDefinition(customObjectID);

    // Create a write operation to create the custom object.
    // The write operation should add the defaults to the CoAP create packet.
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(client_session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/10000/0/0"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    const AwaOpaqueArray * value = NULL;
    ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsOpaqueArrayPointer(getResponse, "/10000/0/0", &value));
    ASSERT_TRUE(NULL != value);
    EXPECT_EQ(0, Array_Compare((AwaArray *)expected, (AwaArray *)value, AwaResourceType_OpaqueArray));

    AwaOpaqueArray_Free(&expected);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestServerDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_time_array_resource)
{
    //Define our custom object
    AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(client_session_);
    AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(server_session_);
    EXPECT_TRUE(clientDefineOperation != NULL);
    EXPECT_TRUE(serverDefineOperation != NULL);

    AwaTimeArray * expected = AwaTimeArray_New();
    AwaTimeArray_SetValue(expected, 0u, 123456787);
    AwaTimeArray_SetValue(expected, 1u, 987654321);
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTimeArray(customObjectDefinition, 0, "Test Resource", 2, 10, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, defaults::timeout));
    ASSERT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&clientDefineOperation);
    AwaServerDefineOperation_Free(&serverDefineOperation);

    WaitForClientDefinition(customObjectID);

    // Create a write operation to create the custom object.
    // The write operation should add the defaults to the CoAP create packet.
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(client_session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/10000/0/0"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    const AwaTimeArray * value = NULL;
    ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsTimeArrayPointer(getResponse, "/10000/0/0", &value));
    ASSERT_TRUE(NULL != value);
    EXPECT_EQ(0, Array_Compare((AwaArray *)expected, (AwaArray *)value, AwaResourceType_TimeArray));

    AwaTimeArray_Free(&expected);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestServerDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_objectlink_array_resource)
{
    //Define our custom object
    AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(client_session_);
    AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(server_session_);
    EXPECT_TRUE(clientDefineOperation != NULL);
    EXPECT_TRUE(serverDefineOperation != NULL);

    AwaObjectLinkArray * expected = AwaObjectLinkArray_New();
    AwaObjectLinkArray_SetValue(expected, 0u, {9, 7});
    AwaObjectLinkArray_SetValue(expected, 1u, {3, 2});
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLinkArray(customObjectDefinition, 0, "Test Resource", 2, 10, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, defaults::timeout));
    ASSERT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&clientDefineOperation);
    AwaServerDefineOperation_Free(&serverDefineOperation);

    WaitForClientDefinition(customObjectID);

    // Create a write operation to create the custom object.
    // The write operation should add the defaults to the CoAP create packet.
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(client_session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/10000/0/0"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    const AwaObjectLinkArray * value = NULL;
    ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsObjectLinkArrayPointer(getResponse, "/10000/0/0", &value));
    ASSERT_TRUE(NULL != value);
    EXPECT_EQ(0, Array_Compare((AwaArray *)expected, (AwaArray *)value, AwaResourceType_ObjectLinkArray));

    AwaObjectLinkArray_Free(&expected);
    AwaClientGetOperation_Free(&getOperation);
}

} // namespace Awa

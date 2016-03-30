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

class TestClientDefineDefaults : public TestClientBase {};

class TestClientDefineDefaultsWithDaemon : public TestClientWithConnectedSession {};


TEST_F(TestClientDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_string_resource)
{
    //Define our custom object
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    const char * expected = "abcdefg";
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsString(customObjectDefinition, 0, "Test Resource", true, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    //Create a basic set operation to create our custom objects
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
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

TEST_F(TestClientDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_integer_resource)
{
    //Define our custom object
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    AwaInteger expected = 123456787;
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(customObjectDefinition, 0, "Test Resource", true, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    //Create a basic set operation to create our custom objects
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
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

TEST_F(TestClientDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_float_resource)
{
    //Define our custom object
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    AwaFloat expected = 123.321;
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloat(customObjectDefinition, 0, "Test Resource", true, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    //Create a basic set operation to create our custom objects
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
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

TEST_F(TestClientDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_boolean_resource)
{
    for (int i = 0; i < 2; i++)
    {
        //Define our custom object
        AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
        EXPECT_TRUE(defineOperation != NULL);

        AwaBoolean expected = (i == 0);
        int customObjectID = 10000 + i;
        char instancePath[16];
        char resourcePath[16];
        sprintf(instancePath, "/%d/0", customObjectID);
        sprintf(resourcePath, "/%d/0/0", customObjectID);

        AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
        ASSERT_TRUE(NULL != customObjectDefinition);

        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBoolean(customObjectDefinition, 0, "Test Resource", true, AwaResourceOperations_ReadWrite, expected));
        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
        ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

        AwaObjectDefinition_Free(&customObjectDefinition);
        AwaClientDefineOperation_Free(&defineOperation);

        //Create a basic set operation to create our custom objects
        AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
        ASSERT_TRUE(NULL != setOperation);

        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, instancePath));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
        AwaClientSetOperation_Free(&setOperation);

        AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
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

TEST_F(TestClientDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_opaque_resource)
{
    //Define our custom object
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    const char dummyData[] = {'a', 0, 'x', 123, '4'};
    AwaOpaque expected {(void*)dummyData, sizeof(dummyData)};
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque(customObjectDefinition, 0, "Test Resource", true, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    //Create a basic set operation to create our custom objects
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
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

TEST_F(TestClientDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_time_resource)
{
    //Define our custom object
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    AwaTime expected = 12324321;
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTime(customObjectDefinition, 0, "Test Resource", true, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    //Create a basic set operation to create our custom objects
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
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

TEST_F(TestClientDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_objectlink_resource)
{
    //Define our custom object
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    AwaObjectLink expected = {9, 7};
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLink(customObjectDefinition, 0, "Test Resource", true, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    //Create a basic set operation to create our custom objects
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
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

TEST_F(TestClientDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_string_array_resource)
{
    //Define our custom object
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    AwaStringArray * expected = AwaStringArray_New();
    AwaStringArray_SetValueAsCString(expected, 0u, "123456787");
    AwaStringArray_SetValueAsCString(expected, 1u, "987654321");
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    // in order for DEFAULTS to be created, minimum instances must be >= 1
    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsStringArray(customObjectDefinition, 0, "Test Resource", 2, 10, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    //Create a basic set operation to create our custom objects
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
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

TEST_F(TestClientDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_integer_array_resource)
{
    //Define our custom object
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    AwaIntegerArray * expected = AwaIntegerArray_New();
    AwaIntegerArray_SetValue(expected, 0u, 123456787);
    AwaIntegerArray_SetValue(expected, 1u, 987654321);
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    // in order for DEFAULTS to be created, minimum instances must be >= 1
    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsIntegerArray(customObjectDefinition, 0, "Test Resource", 2, 10, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    //Create a basic set operation to create our custom objects
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
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

TEST_F(TestClientDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_float_array_resource)
{
    //Define our custom object
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    AwaFloatArray * expected = AwaFloatArray_New();
    AwaFloatArray_SetValue(expected, 0u, 1234.5678);
    AwaFloatArray_SetValue(expected, 1u, 9876.5432);
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    // in order for DEFAULTS to be created, minimum instances must be >= 1
    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloatArray(customObjectDefinition, 0, "Test Resource", 2, 10, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    //Create a basic set operation to create our custom objects
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
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

TEST_F(TestClientDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_boolean_array_resource)
{
    //Define our custom object
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    AwaBooleanArray * expected = AwaBooleanArray_New();
    AwaBooleanArray_SetValue(expected, 0u, true);
    AwaBooleanArray_SetValue(expected, 1u, false);
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    // in order for DEFAULTS to be created, minimum instances must be >= 1
    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBooleanArray(customObjectDefinition, 0, "Test Resource", 2, 10, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    //Create a basic set operation to create our custom objects
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
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

TEST_F(TestClientDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_opaque_array_resource)
{
    //Define our custom object
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    const char dummyData1[] = {'a', 0, 'x', 123, '4'};
    const char dummyData2[] = {'b', 0, 's', 22};

    AwaOpaqueArray * expected = AwaOpaqueArray_New();
    AwaOpaqueArray_SetValue(expected, 0u, {(void*)dummyData1, sizeof(dummyData1)});
    AwaOpaqueArray_SetValue(expected, 1u, {(void*)dummyData2, sizeof(dummyData2)});
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    // in order for DEFAULTS to be created, minimum instances must be >= 1
    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaqueArray(customObjectDefinition, 0, "Test Resource", 2, 10, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    //Create a basic set operation to create our custom objects
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
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

TEST_F(TestClientDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_time_array_resource)
{
    //Define our custom object
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    AwaTimeArray * expected = AwaTimeArray_New();
    AwaTimeArray_SetValue(expected, 0u, 123456787);
    AwaTimeArray_SetValue(expected, 1u, 987654321);
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    // in order for DEFAULTS to be created, minimum instances must be >= 1
    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTimeArray(customObjectDefinition, 0, "Test Resource", 2, 10, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    //Create a basic set operation to create our custom objects
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
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

TEST_F(TestClientDefineDefaultsWithDaemon, AwaClient_get_default_value_from_created_custom_object_instance_objectlink_array_resource)
{
    //Define our custom object
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    AwaObjectLinkArray * expected = AwaObjectLinkArray_New();
    AwaObjectLinkArray_SetValue(expected, 0u, {9, 7});
    AwaObjectLinkArray_SetValue(expected, 1u, {3, 2});
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    // in order for DEFAULTS to be created, minimum instances must be >= 1
    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLinkArray(customObjectDefinition, 0, "Test Resource", 2, 10, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    //Create a basic set operation to create our custom objects
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TEST SENSIBLE RESOURCE DEFAULTS
TEST_F(TestClientDefineDefaultsWithDaemon, optional_string_resource_has_sensible_default)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1, {
            ResourceDescription(0, "Resource0", AwaResourceType_String, 0, 1, AwaResourceOperations_ReadWrite),
        }};
    EXPECT_EQ(AwaError_Success, Define(session_, object));

    // create an instance of this object and the optional object
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    // check the default value of the optional resource
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_); ASSERT_TRUE(NULL != getOperation);
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation); ASSERT_TRUE(NULL != getResponse);
    const char * value = NULL;
    EXPECT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsCStringPointer(getResponse, "/1000/0/0", &value));
    EXPECT_STREQ("", value);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestClientDefineDefaultsWithDaemon, optional_integer_resource_has_sensible_default)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1, {
            ResourceDescription(0, "Resource0", AwaResourceType_Integer, 0, 1, AwaResourceOperations_ReadWrite),
        }};
    EXPECT_EQ(AwaError_Success, Define(session_, object));

    // create an instance of this object and the optional object
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    // check the default value of the optional resource
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_); ASSERT_TRUE(NULL != getOperation);
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation); ASSERT_TRUE(NULL != getResponse);
    const AwaInteger * value = NULL;
    EXPECT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsIntegerPointer(getResponse, "/1000/0/0", &value));
    EXPECT_EQ(0, *value);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestClientDefineDefaultsWithDaemon, optional_float_resource_has_sensible_default)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1, {
            ResourceDescription(0, "Resource0", AwaResourceType_Float, 0, 1, AwaResourceOperations_ReadWrite),
        }};
    EXPECT_EQ(AwaError_Success, Define(session_, object));

    // create an instance of this object and the optional object
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    // check the default value of the optional resource
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_); ASSERT_TRUE(NULL != getOperation);
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation); ASSERT_TRUE(NULL != getResponse);
    const AwaFloat * value = NULL;
    EXPECT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsFloatPointer(getResponse, "/1000/0/0", &value));
    EXPECT_EQ(0.0, *value);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestClientDefineDefaultsWithDaemon, optional_boolean_resource_has_sensible_default)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1, {
            ResourceDescription(0, "Resource0", AwaResourceType_Boolean, 0, 1, AwaResourceOperations_ReadWrite),
        }};
    EXPECT_EQ(AwaError_Success, Define(session_, object));

    // create an instance of this object and the optional object
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    // check the default value of the optional resource
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_); ASSERT_TRUE(NULL != getOperation);
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation); ASSERT_TRUE(NULL != getResponse);
    const AwaBoolean * value = NULL;
    EXPECT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsBooleanPointer(getResponse, "/1000/0/0", &value));
    EXPECT_FALSE(*value);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestClientDefineDefaultsWithDaemon, optional_time_resource_has_sensible_default)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1, {
            ResourceDescription(0, "Resource0", AwaResourceType_Time, 0, 1, AwaResourceOperations_ReadWrite),
        }};
    EXPECT_EQ(AwaError_Success, Define(session_, object));

    // create an instance of this object and the optional object
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    // check the default value of the optional resource
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_); ASSERT_TRUE(NULL != getOperation);
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation); ASSERT_TRUE(NULL != getResponse);
    const AwaTime * value = NULL;
    EXPECT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsTimePointer(getResponse, "/1000/0/0", &value));
    EXPECT_EQ(0, *value);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestClientDefineDefaultsWithDaemon, optional_opaque_resource_has_sensible_default)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1, {
            ResourceDescription(0, "Resource0", AwaResourceType_Opaque, 0, 1, AwaResourceOperations_ReadWrite),
        }};
    EXPECT_EQ(AwaError_Success, Define(session_, object));

    // create an instance of this object and the optional object
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    // check the default value of the optional resource
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_); ASSERT_TRUE(NULL != getOperation);
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation); ASSERT_TRUE(NULL != getResponse);
    AwaOpaque value = { .Data = (void *)0xffffffff, .Size = 1024 };
    EXPECT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsOpaque(getResponse, "/1000/0/0", &value));
    EXPECT_EQ(NULL, value.Data);
    EXPECT_EQ(0u, value.Size);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestClientDefineDefaultsWithDaemon, optional_objectlink_resource_has_sensible_default)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1, {
            ResourceDescription(0, "Resource0", AwaResourceType_ObjectLink, 0, 1, AwaResourceOperations_ReadWrite),
        }};
    EXPECT_EQ(AwaError_Success, Define(session_, object));

    // create an instance of this object and the optional object
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    // check the default value of the optional resource
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_); ASSERT_TRUE(NULL != getOperation);
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation); ASSERT_TRUE(NULL != getResponse);
    AwaObjectLink value = { .ObjectID = AWA_INVALID_ID, .ObjectInstanceID = AWA_INVALID_ID };
    EXPECT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsObjectLink(getResponse, "/1000/0/0", &value));
    EXPECT_EQ(0, value.ObjectID);
    EXPECT_EQ(0, value.ObjectInstanceID);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestClientDefineDefaultsWithDaemon, optional_string_array_resource_has_sensible_default)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1, {
            ResourceDescription(0, "Resource0", AwaResourceType_StringArray, 0, 10, AwaResourceOperations_ReadWrite),
        }};
    EXPECT_EQ(AwaError_Success, Define(session_, object));

    // create an instance of this object and the optional object
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    // check the default value of the optional resource
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_); ASSERT_TRUE(NULL != getOperation);
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation); ASSERT_TRUE(NULL != getResponse);

    const AwaStringArray * valueArray = NULL;
    EXPECT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsStringArrayPointer(getResponse, "/1000/0/0", &valueArray));
    EXPECT_EQ(0u, AwaStringArray_GetValueCount(valueArray));
    //EXPECT_STREQ("", AwaStringArray_GetValueAsCString(valueArray, 0));

    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestClientDefineDefaultsWithDaemon, optional_integer_array_resource_has_sensible_default)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1, {
            ResourceDescription(0, "Resource0", AwaResourceType_IntegerArray, 0, 10, AwaResourceOperations_ReadWrite),
        }};
    EXPECT_EQ(AwaError_Success, Define(session_, object));

    // create an instance of this object and the optional object
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    // check the default value of the optional resource
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_); ASSERT_TRUE(NULL != getOperation);
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation); ASSERT_TRUE(NULL != getResponse);

    const AwaIntegerArray * valueArray = NULL;
    EXPECT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsIntegerArrayPointer(getResponse, "/1000/0/0", &valueArray));
    EXPECT_EQ(0u, AwaIntegerArray_GetValueCount(valueArray));
    //EXPECT_EQ(0, AwaIntegerArray_GetValue(valueArray, 0));

    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestClientDefineDefaultsWithDaemon, optional_float_array_resource_has_sensible_default)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1, {
            ResourceDescription(0, "Resource0", AwaResourceType_FloatArray, 0, 10, AwaResourceOperations_ReadWrite),
        }};
    EXPECT_EQ(AwaError_Success, Define(session_, object));

    // create an instance of this object and the optional object
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    // check the default value of the optional resource
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_); ASSERT_TRUE(NULL != getOperation);
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation); ASSERT_TRUE(NULL != getResponse);

    const AwaFloatArray * valueArray = NULL;
    EXPECT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsFloatArrayPointer(getResponse, "/1000/0/0", &valueArray));
    EXPECT_EQ(0u, AwaFloatArray_GetValueCount(valueArray));
    //EXPECT_EQ(0.0f, AwaFloatArray_GetValue(valueArray, 0));

    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestClientDefineDefaultsWithDaemon, optional_boolean_array_resource_has_sensible_default)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1, {
            ResourceDescription(0, "Resource0", AwaResourceType_BooleanArray, 0, 10, AwaResourceOperations_ReadWrite),
        }};
    EXPECT_EQ(AwaError_Success, Define(session_, object));

    // create an instance of this object and the optional object
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    // check the default value of the optional resource
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_); ASSERT_TRUE(NULL != getOperation);
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation); ASSERT_TRUE(NULL != getResponse);

    const AwaBooleanArray * valueArray = NULL;
    EXPECT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsBooleanArrayPointer(getResponse, "/1000/0/0", &valueArray));
    EXPECT_EQ(0u, AwaBooleanArray_GetValueCount(valueArray));
    //EXPECT_FALSE(AwaBooleanArray_GetValue(valueArray, 0));

    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestClientDefineDefaultsWithDaemon, optional_time_array_resource_has_sensible_default)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1, {
            ResourceDescription(0, "Resource0", AwaResourceType_TimeArray, 0, 10, AwaResourceOperations_ReadWrite),
        }};
    EXPECT_EQ(AwaError_Success, Define(session_, object));

    // create an instance of this object and the optional object
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    // check the default value of the optional resource
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_); ASSERT_TRUE(NULL != getOperation);
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation); ASSERT_TRUE(NULL != getResponse);

    const AwaTimeArray * valueArray = NULL;
    EXPECT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsTimeArrayPointer(getResponse, "/1000/0/0", &valueArray));
    EXPECT_EQ(0u, AwaTimeArray_GetValueCount(valueArray));
    //EXPECT_EQ(0, AwaTimeArray_GetValue(valueArray, 0));

    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestClientDefineDefaultsWithDaemon, optional_opaque_array_resource_has_sensible_default)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1, {
            ResourceDescription(0, "Resource0", AwaResourceType_OpaqueArray, 0, 10, AwaResourceOperations_ReadWrite),
        }};
    EXPECT_EQ(AwaError_Success, Define(session_, object));

    // create an instance of this object and the optional object
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    // check the default value of the optional resource
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_); ASSERT_TRUE(NULL != getOperation);
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation); ASSERT_TRUE(NULL != getResponse);

    const AwaOpaqueArray * valueArray = NULL;
    EXPECT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsOpaqueArrayPointer(getResponse, "/1000/0/0", &valueArray));
    EXPECT_EQ(0u, AwaOpaqueArray_GetValueCount(valueArray));
//    AwaOpaque value = AwaOpaqueArray_GetValue(valueArray, 0);
//    EXPECT_EQ(NULL, value.Data);
//    EXPECT_EQ(0u, value.Size);

    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestClientDefineDefaultsWithDaemon, optional_objectlink_array_resource_has_sensible_default)
{
    ObjectDescription object = { 1000, "Object1000", 0, 1, {
            ResourceDescription(0, "Resource0", AwaResourceType_ObjectLinkArray, 0, 10, AwaResourceOperations_ReadWrite),
        }};
    EXPECT_EQ(AwaError_Success, Define(session_, object));

    // create an instance of this object and the optional object
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    // check the default value of the optional resource
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_); ASSERT_TRUE(NULL != getOperation);
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation); ASSERT_TRUE(NULL != getResponse);

    const AwaObjectLinkArray * valueArray = NULL;
    EXPECT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsObjectLinkArrayPointer(getResponse, "/1000/0/0", &valueArray));
    EXPECT_EQ(0u, AwaObjectLinkArray_GetValueCount(valueArray));
//    AwaObjectLink value = AwaObjectLinkArray_GetValue(valueArray, 0);
//    EXPECT_EQ(0, value.ObjectID);
//    EXPECT_EQ(0, value.ObjectInstanceID);

    AwaClientGetOperation_Free(&getOperation);
}


TEST_F(TestClientDefineDefaultsWithDaemon, AwaClientGetOperation_receive_and_iterate_default_string_array_values)
{
    // Test that we can get the default values that an object was defined with
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(10000, "Test Object", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    AwaStringArray * defaultArray = AwaStringArray_New();
    for (int i = 0; i < 10; i++)
    {
        char value[8];
        sprintf(value, "%d", i * 10);
        AwaStringArray_SetValueAsCString(defaultArray, static_cast<size_t>(i), value);
    }

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsStringArray(customObjectDefinition, 1, "Test String Array Resource", 0, 10, AwaResourceOperations_ReadWrite, defaultArray));

    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    // Create a basic set operation to create the object instance / resource
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10000/0/1"));

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    // confirm we can iterate through the default value
    AwaClientGetOperation * operation = AwaClientGetOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(operation, "/10000/0/1"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(operation, defaults::timeout));
    const AwaClientGetResponse * response = AwaClientGetOperation_GetResponse(operation); ASSERT_TRUE(NULL != response);
    ASSERT_TRUE(NULL != response);


    const AwaStringArray * array = NULL;
    AwaClientGetResponse_GetValuesAsStringArrayPointer(response, "/10000/0/1", &array);
    ASSERT_TRUE(NULL != array);
    EXPECT_EQ(0, Array_Compare((AwaArray *)defaultArray, (AwaArray *)array, AwaResourceType_StringArray));

    AwaCStringArrayIterator * iterator = AwaStringArray_NewCStringArrayIterator(array);
    for (int i = 0; i < 10; i++)
    {
        EXPECT_TRUE(AwaCStringArrayIterator_Next(iterator));
        EXPECT_EQ(static_cast<size_t>(i), AwaCStringArrayIterator_GetIndex(iterator));
        EXPECT_STREQ(AwaStringArray_GetValueAsCString(defaultArray, i), AwaCStringArrayIterator_GetValueAsCString(iterator));
    }
    AwaCStringArrayIterator_Free(&iterator);

    AwaClientGetOperation_Free(&operation);
    AwaStringArray_Free(&defaultArray);
}

TEST_F(TestClientDefineDefaultsWithDaemon, AwaClientGetOperation_receive_and_iterate_default_integer_array_values)
{
    // Test that we can get the default values that an object was defined with
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(10000, "Test Object", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    AwaIntegerArray * defaultArray = AwaIntegerArray_New();
    for (int i = 0; i < 10; i++)
    {
        AwaIntegerArray_SetValue(defaultArray, static_cast<size_t>(i), i * 10);
    }

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsIntegerArray(customObjectDefinition, 1, "Test Integer Array Resource", 0, 10, AwaResourceOperations_ReadWrite, defaultArray));

    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    // Create a basic set operation to create the object instance / resource
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10000/0/1"));

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    // confirm we can iterate through the default value
    AwaClientGetOperation * operation = AwaClientGetOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(operation, "/10000/0/1"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(operation, defaults::timeout));
    const AwaClientGetResponse * response = AwaClientGetOperation_GetResponse(operation); ASSERT_TRUE(NULL != response);
    ASSERT_TRUE(NULL != response);


    const AwaIntegerArray * array = NULL;
    AwaClientGetResponse_GetValuesAsIntegerArrayPointer(response, "/10000/0/1", &array);
    ASSERT_TRUE(NULL != array);
    EXPECT_EQ(0, Array_Compare((AwaArray *)defaultArray, (AwaArray *)array, AwaResourceType_IntegerArray));

    AwaIntegerArrayIterator * iterator = AwaIntegerArray_NewIntegerArrayIterator(array);
    for (int i = 0; i < 10; i++)
    {
        EXPECT_TRUE(AwaIntegerArrayIterator_Next(iterator));
        EXPECT_EQ(static_cast<size_t>(i), AwaIntegerArrayIterator_GetIndex(iterator));
        EXPECT_EQ(AwaIntegerArray_GetValue(defaultArray, i), AwaIntegerArrayIterator_GetValue(iterator));
    }
    AwaIntegerArrayIterator_Free(&iterator);

    AwaClientGetOperation_Free(&operation);
    AwaIntegerArray_Free(&defaultArray);
}

TEST_F(TestClientDefineDefaultsWithDaemon, AwaClientGetOperation_receive_and_iterate_default_float_array_values)
{
    // Test that we can get the default values that an object was defined with
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(10000, "Test Object", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    AwaFloatArray * defaultArray = AwaFloatArray_New();
    for (int i = 0; i < 10; i++)
    {
        AwaFloatArray_SetValue(defaultArray, static_cast<size_t>(i), i * 10.1);
    }

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloatArray(customObjectDefinition, 1, "Test Float Array Resource", 0, 10, AwaResourceOperations_ReadWrite, defaultArray));

    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    // Create a basic set operation to create the object instance / resource
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10000/0/1"));

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    // confirm we can iterate through the default value
    AwaClientGetOperation * operation = AwaClientGetOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(operation, "/10000/0/1"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(operation, defaults::timeout));
    const AwaClientGetResponse * response = AwaClientGetOperation_GetResponse(operation); ASSERT_TRUE(NULL != response);
    ASSERT_TRUE(NULL != response);


    const AwaFloatArray * array = NULL;
    AwaClientGetResponse_GetValuesAsFloatArrayPointer(response, "/10000/0/1", &array);
    ASSERT_TRUE(NULL != array);
    EXPECT_EQ(0, Array_Compare((AwaArray *)defaultArray, (AwaArray *)array, AwaResourceType_FloatArray));

    AwaFloatArrayIterator * iterator = AwaFloatArray_NewFloatArrayIterator(array);
    for (int i = 0; i < 10; i++)
    {
        EXPECT_TRUE(AwaFloatArrayIterator_Next(iterator));
        EXPECT_EQ(static_cast<size_t>(i), AwaFloatArrayIterator_GetIndex(iterator));
        EXPECT_EQ(AwaFloatArray_GetValue(defaultArray, i), AwaFloatArrayIterator_GetValue(iterator));
    }
    AwaFloatArrayIterator_Free(&iterator);

    AwaClientGetOperation_Free(&operation);
    AwaFloatArray_Free(&defaultArray);
}

TEST_F(TestClientDefineDefaultsWithDaemon, AwaClientGetOperation_receive_and_iterate_default_boolean_array_values)
{
    // Test that we can get the default values that an object was defined with
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(10000, "Test Object", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    AwaBooleanArray * defaultArray = AwaBooleanArray_New();
    for (int i = 0; i < 10; i++)
    {
        AwaBooleanArray_SetValue(defaultArray, static_cast<size_t>(i), i % 2 == 0);
    }

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBooleanArray(customObjectDefinition, 1, "Test Boolean Array Resource", 0, 10, AwaResourceOperations_ReadWrite, defaultArray));

    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    // Create a basic set operation to create the object instance / resource
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10000/0/1"));

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    // confirm we can iterate through the default value
    AwaClientGetOperation * operation = AwaClientGetOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(operation, "/10000/0/1"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(operation, defaults::timeout));
    const AwaClientGetResponse * response = AwaClientGetOperation_GetResponse(operation); ASSERT_TRUE(NULL != response);
    ASSERT_TRUE(NULL != response);


    const AwaBooleanArray * array = NULL;
    AwaClientGetResponse_GetValuesAsBooleanArrayPointer(response, "/10000/0/1", &array);
    ASSERT_TRUE(NULL != array);
    EXPECT_EQ(0, Array_Compare((AwaArray *)defaultArray, (AwaArray *)array, AwaResourceType_BooleanArray));

    AwaBooleanArrayIterator * iterator = AwaBooleanArray_NewBooleanArrayIterator(array);
    for (int i = 0; i < 10; i++)
    {
        EXPECT_TRUE(AwaBooleanArrayIterator_Next(iterator));
        EXPECT_EQ(static_cast<size_t>(i), AwaBooleanArrayIterator_GetIndex(iterator));
        EXPECT_EQ(AwaBooleanArray_GetValue(defaultArray, i), AwaBooleanArrayIterator_GetValue(iterator));
    }
    AwaBooleanArrayIterator_Free(&iterator);

    AwaClientGetOperation_Free(&operation);
    AwaBooleanArray_Free(&defaultArray);
}

TEST_F(TestClientDefineDefaultsWithDaemon, AwaClientGetOperation_receive_and_iterate_default_time_array_values)
{
    // Test that we can get the default values that an object was defined with
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(10000, "Test Object", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    AwaTimeArray * defaultArray = AwaTimeArray_New();
    for (int i = 0; i < 10; i++)
    {
        AwaTimeArray_SetValue(defaultArray, static_cast<size_t>(i), i * 10);
    }

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTimeArray(customObjectDefinition, 1, "Test Time Array Resource", 0, 10, AwaResourceOperations_ReadWrite, defaultArray));

    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    // Create a basic set operation to create the object instance / resource
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10000/0/1"));

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    // confirm we can iterate through the default value
    AwaClientGetOperation * operation = AwaClientGetOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(operation, "/10000/0/1"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(operation, defaults::timeout));
    const AwaClientGetResponse * response = AwaClientGetOperation_GetResponse(operation); ASSERT_TRUE(NULL != response);
    ASSERT_TRUE(NULL != response);


    const AwaTimeArray * array = NULL;
    AwaClientGetResponse_GetValuesAsTimeArrayPointer(response, "/10000/0/1", &array);
    ASSERT_TRUE(NULL != array);
    EXPECT_EQ(0, Array_Compare((AwaArray *)defaultArray, (AwaArray *)array, AwaResourceType_TimeArray));

    AwaTimeArrayIterator * iterator = AwaTimeArray_NewTimeArrayIterator(array);
    for (int i = 0; i < 10; i++)
    {
        EXPECT_TRUE(AwaTimeArrayIterator_Next(iterator));
        EXPECT_EQ(static_cast<size_t>(i), AwaTimeArrayIterator_GetIndex(iterator));
        EXPECT_EQ(AwaTimeArray_GetValue(defaultArray, i), AwaTimeArrayIterator_GetValue(iterator));
    }
    AwaTimeArrayIterator_Free(&iterator);

    AwaClientGetOperation_Free(&operation);
    AwaTimeArray_Free(&defaultArray);
}

TEST_F(TestClientDefineDefaultsWithDaemon, AwaClientGetOperation_receive_and_iterate_default_objectlink_array_values)
{
    // Test that we can get the default values that an object was defined with
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(10000, "Test Object", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    AwaObjectLinkArray * defaultArray = AwaObjectLinkArray_New();
    for (int i = 0; i < 10; i++)
    {
        AwaObjectLink objectLink = {i, 10 - i};
        AwaObjectLinkArray_SetValue(defaultArray, static_cast<size_t>(i), objectLink);
    }

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLinkArray(customObjectDefinition, 1, "Test ObjectLink Array Resource", 0, 10, AwaResourceOperations_ReadWrite, defaultArray));

    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    // Create a basic set operation to create the object instance / resource
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10000/0/1"));

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    // confirm we can iterate through the default value
    AwaClientGetOperation * operation = AwaClientGetOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(operation, "/10000/0/1"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(operation, defaults::timeout));
    const AwaClientGetResponse * response = AwaClientGetOperation_GetResponse(operation); ASSERT_TRUE(NULL != response);
    ASSERT_TRUE(NULL != response);


    const AwaObjectLinkArray * array = NULL;
    AwaClientGetResponse_GetValuesAsObjectLinkArrayPointer(response, "/10000/0/1", &array);
    ASSERT_TRUE(NULL != array);
    EXPECT_EQ(0, Array_Compare((AwaArray *)defaultArray, (AwaArray *)array, AwaResourceType_ObjectLinkArray));

    AwaObjectLinkArrayIterator * iterator = AwaObjectLinkArray_NewObjectLinkArrayIterator(array);
    for (int i = 0; i < 10; i++)
    {
        EXPECT_TRUE(AwaObjectLinkArrayIterator_Next(iterator));
        EXPECT_EQ(static_cast<size_t>(i), AwaObjectLinkArrayIterator_GetIndex(iterator));
        AwaObjectLink defaultObjectLink = AwaObjectLinkArray_GetValue(defaultArray, i);
        AwaObjectLink receivedObjectLink = AwaObjectLinkArrayIterator_GetValue(iterator);

        EXPECT_EQ(0, memcmp(&defaultObjectLink, &receivedObjectLink, sizeof(AwaObjectLink)));
    }
    AwaObjectLinkArrayIterator_Free(&iterator);

    AwaClientGetOperation_Free(&operation);
    AwaObjectLinkArray_Free(&defaultArray);
}

TEST_F(TestClientDefineDefaultsWithDaemon, AwaClientGetOperation_receive_and_iterate_default_opaque_array_values)
{
    // Test that we can get the default values that an object was defined with
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(10000, "Test Object", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    AwaOpaqueArray * defaultArray = AwaOpaqueArray_New();
    for (int i = 0; i < 10; i++)
    {
        char dummyData[] = {'a', static_cast<char>(i), 0, 'c', '\0', 123};
        AwaOpaque opaque = {dummyData, sizeof(dummyData)};
        AwaOpaqueArray_SetValue(defaultArray, static_cast<size_t>(i), opaque);
    }

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaqueArray(customObjectDefinition, 1, "Test Opaque Array Resource", 0, 10, AwaResourceOperations_ReadWrite, defaultArray));

    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    // Create a basic set operation to create the object instance / resource
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10000/0/1"));

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    // confirm we can iterate through the default value
    AwaClientGetOperation * operation = AwaClientGetOperation_New(session_);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(operation, "/10000/0/1"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(operation, defaults::timeout));
    const AwaClientGetResponse * response = AwaClientGetOperation_GetResponse(operation); ASSERT_TRUE(NULL != response);
    ASSERT_TRUE(NULL != response);


    const AwaOpaqueArray * array = NULL;
    AwaClientGetResponse_GetValuesAsOpaqueArrayPointer(response, "/10000/0/1", &array);
    ASSERT_TRUE(NULL != array);
    EXPECT_EQ(0, Array_Compare((AwaArray *)defaultArray, (AwaArray *)array, AwaResourceType_OpaqueArray));

    AwaOpaqueArrayIterator * iterator = AwaOpaqueArray_NewOpaqueArrayIterator(array);
    for (int i = 0; i < 10; i++)
    {
        EXPECT_TRUE(AwaOpaqueArrayIterator_Next(iterator));
        EXPECT_EQ(static_cast<size_t>(i), AwaOpaqueArrayIterator_GetIndex(iterator));

        AwaOpaque defaultOpaque = AwaOpaqueArray_GetValue(defaultArray, i);
        AwaOpaque receivedOpaque = AwaOpaqueArrayIterator_GetValue(iterator);

        EXPECT_EQ(defaultOpaque.Size, receivedOpaque.Size);
        EXPECT_EQ(0, memcmp(defaultOpaque.Data, receivedOpaque.Data, defaultOpaque.Size));
    }
    AwaOpaqueArrayIterator_Free(&iterator);

    AwaClientGetOperation_Free(&operation);
    AwaOpaqueArray_Free(&defaultArray);
}



} // namespace Awa

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

    // in order for DEFAULTS to be created, minimum instances must be > 1
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

    // in order for DEFAULTS to be created, minimum instances must be > 1
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

    // in order for DEFAULTS to be created, minimum instances must be > 1
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

    // in order for DEFAULTS to be created, minimum instances must be > 1
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

    // in order for DEFAULTS to be created, minimum instances must be > 1
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

    // in order for DEFAULTS to be created, minimum instances must be > 1
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

    // in order for DEFAULTS to be created, minimum instances must be > 1
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

} // namespace Awa

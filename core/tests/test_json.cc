#include <gtest/gtest.h>
#include <string>
#include <stdio.h>
#include <stdint.h>

// https://meekrosoft.wordpress.com/2009/11/09/unit-testing-c-code-with-the-googletest-framework/
// 1. Define fake functions for the dependencies you want to stub out
// 2. If the module depends on a global (gasp!) you need to define your fake one
// 3. include your module implementation (#include module.c)
// 4. Define a method to reset all the static data to a known state.
// 5. Define your tests

#include "common/lwm2m_object_store.h"
#include "common/lwm2m_json.c"
#include "common/lwm2m_tree_node.h"
#include "common/lwm2m_tree_builder.h"
#include "lwm2m_core.h"

class JsonTestSuite : public testing::Test
{
    void SetUp() { context = Lwm2mCore_Init(NULL, NULL); }
    void TearDown() { Lwm2mCore_Destroy(context); }

protected:
    Lwm2mContextType * context;
};

TEST_F(JsonTestSuite, test_serialise_string)
{
    Lwm2m_SetLogLevel(DebugLevel_Debug);

    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 0, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 0, 0, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_RW, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 0, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 0, 0, 0, 0, (char*)"Open Mobile Alliance", strlen("Open Mobile Alliance"));

    uint8_t buffer[512];
    memset(buffer, 0, 512);

    char * expected = (char*)"{\"e\":[\n"
    "{\"n\":\"0/0\",\"sv\":\"Open Mobile Alliance\"}]\n"
    "}\n";

    Lwm2mTreeNode * dest;
    TreeBuilder_CreateTreeFromObject(&dest, context, Lwm2mRequestOrigin_Client, 0);

    SerdesContext serdesContext = NULL;
    int len = JsonSerialiseObject(&serdesContext, dest, 0, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    EXPECT_EQ(static_cast<int>(strlen(expected)), len);
    EXPECT_EQ(0, memcmp(buffer, expected, strlen(expected)));
}

TEST_F(JsonTestSuite, test_serialise_string_multiple)
{
    Lwm2m_SetLogLevel(DebugLevel_Debug);

    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 0, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 0, 0, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_RW, &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res2", 0, 1, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_RW, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 0, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 0, 0, 0, 0, (char*)"Open Mobile Alliance", strlen("Open Mobile Alliance"));
    Lwm2mCore_SetResourceInstanceValue(context, 0, 0, 1, 0, (char*)"Lightweight M2M Client", strlen("Lightweight M2M Client"));

    uint8_t buffer[512];
    memset(buffer, 0, 512);

    char * expected = (char*)"{\"e\":[\n"
    "{\"n\":\"0/0\",\"sv\":\"Open Mobile Alliance\"},\n"
    "{\"n\":\"0/1\",\"sv\":\"Lightweight M2M Client\"}]\n"
    "}\n";

    Lwm2mTreeNode * dest;

    TreeBuilder_CreateTreeFromObject(&dest, context, Lwm2mRequestOrigin_Client, 0);

    SerdesContext serdesContext = NULL;
    int len = JsonSerialiseObject(&serdesContext, dest, 0, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    EXPECT_EQ(static_cast<int>(strlen(expected)), len);
    EXPECT_EQ(0, memcmp(buffer, expected, strlen(expected))) << std::endl << expected << " != "<< std::endl << buffer;
}

TEST_F(JsonTestSuite, test_serialise_opaque)
{
    Lwm2m_SetLogLevel(DebugLevel_Debug);

    char * decoded = (char*)"Open Mobile Alliance";
    //char * encoded = "T3BlbiBNb2JpbGUgQWxsaWFuY2U=";

    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 0, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 0, 0, ResourceTypeEnum_TypeOpaque, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_RW, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 0, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 0, 0, 0, 0, decoded, strlen(decoded));

    uint8_t buffer[512];
    memset(buffer, 0, 512);

    char * expected = (char*)"{\"e\":[\n"
    "{\"n\":\"0/0\",\"sv\":\"T3BlbiBNb2JpbGUgQWxsaWFuY2U=\"}]\n"
    "}\n";

    Lwm2mTreeNode * dest;

    TreeBuilder_CreateTreeFromObject(&dest, context, Lwm2mRequestOrigin_Client, 0);
  
    SerdesContext serdesContext = NULL;
    int len = JsonSerialiseObject(&serdesContext, dest, 0, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    EXPECT_EQ(static_cast<int>(strlen(expected)), len);
    EXPECT_EQ(0, memcmp(buffer, expected, strlen(expected)));
}

TEST_F(JsonTestSuite, test_serialise_boolean)
{
    Lwm2m_SetLogLevel(DebugLevel_Debug);

    bool value = true;
    //char * encoded = "T3BlbiBNb2JpbGUgQWxsaWFuY2U=";

    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 0, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 0, 0, ResourceTypeEnum_TypeBoolean, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_RW, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 0, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 0, 0, 0, 0, &value, sizeof(value));

    uint8_t buffer[512];
    memset(buffer, 0, 512);

    char * expected = (char*)"{\"e\":[\n"
    "{\"n\":\"0/0\",\"bv\":\"true\"}]\n"
    "}\n";

    Lwm2mTreeNode * dest;

    TreeBuilder_CreateTreeFromObject(&dest, context, Lwm2mRequestOrigin_Client, 0);

    //TODO: change other tests to use single instance of an object.
    SerdesContext serdesContext = NULL;
    int len = JsonSerialiseObject(&serdesContext, dest, 0, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    EXPECT_EQ(static_cast<int>(strlen(expected)), len) << expected << std::endl << buffer;
    EXPECT_EQ(0, memcmp(buffer, expected, strlen(expected)));
}

TEST_F(JsonTestSuite, test_serialise_float)
{
    Lwm2m_SetLogLevel(DebugLevel_Debug);

    double value = 5.23;
    //char * encoded = "T3BlbiBNb2JpbGUgQWxsaWFuY2U=";

    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 0, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 0, 0, ResourceTypeEnum_TypeFloat, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_RW, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 0, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 0, 0, 0, 0, &value, sizeof(value));

    uint8_t buffer[512];
    memset(buffer, 0, 512);

    char * expected = (char*)"{\"e\":[\n"
    "{\"n\":\"0/0\",\"v\":5.230000}]\n"
    "}\n";

    Lwm2mTreeNode * dest;

    TreeBuilder_CreateTreeFromObject(&dest, context, Lwm2mRequestOrigin_Client, 0);

    SerdesContext serdesContext = NULL;
    int len = JsonSerialiseObject(&serdesContext, dest, 0, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    EXPECT_EQ(static_cast<int>(strlen(expected)), len) << expected << std::endl << buffer;
    EXPECT_EQ(0, memcmp(buffer, expected, strlen(expected)));
}

TEST_F(JsonTestSuite, test_serialise_integer)
{
    Lwm2m_SetLogLevel(DebugLevel_Debug);

    int64_t value = 9001;

    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 0, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 0, 0, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_RW, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 0, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 0, 0, 0, 0, &value, sizeof(value));

    uint8_t buffer[512];
    memset(buffer, 0, 512);

    char * expected = (char*)"{\"e\":[\n"
    "{\"n\":\"0/0\",\"v\":9001}]\n"
    "}\n";

    Lwm2mTreeNode * dest;

    TreeBuilder_CreateTreeFromObject(&dest, context, Lwm2mRequestOrigin_Client, 0);

    SerdesContext serdesContext = NULL;
    int len = JsonSerialiseObject(&serdesContext, dest, 0, buffer, sizeof(buffer));
 
    Lwm2mTreeNode_DeleteRecursive(dest);

    EXPECT_EQ(static_cast<int>(strlen(expected)), len) << expected << std::endl << buffer;
    EXPECT_EQ(0, memcmp(buffer, expected, strlen(expected)));
}



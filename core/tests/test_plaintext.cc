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
#include "common/lwm2m_plaintext.c"
#include "common/lwm2m_tree_node.h"
#include "common/lwm2m_tree_builder.h"
#include "lwm2m_core.h"
#include "common/lwm2m_request_origin.h"

class PlainTextTestSuite : public testing::Test
{
    void SetUp() { context = Lwm2mCore_Init(NULL, NULL); }
    void TearDown() { Lwm2mCore_Destroy(context); }

protected:
    Lwm2mContextType * context;
};

TEST_F(PlainTextTestSuite, test_serialise_string)
{
    Lwm2m_SetLogLevel(DebugLevel_Debug);

    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 0, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 0, 0, AwaStaticResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_RW, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 0, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 0, 0, 0, 0, (char*)"Open Mobile Alliance", strlen("Open Mobile Alliance"));

    uint8_t buffer[512];
    memset(buffer, 0, 512);

    char * expected = (char*)"Open Mobile Alliance";

    Lwm2mTreeNode * dest;
    int OIR[] = {0,0,0};
    TreeBuilder_CreateTreeFromOIR(&dest, context, Lwm2mRequestOrigin_Client, OIR, 3);
    
    SerdesContext serdesContext;
    int len = PTSerialiseResource(&serdesContext, dest, 0, 0, 0, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    EXPECT_EQ(static_cast<int>(strlen(expected)), len);
    EXPECT_EQ(0, memcmp(buffer, expected, strlen(expected)));
}

TEST_F(PlainTextTestSuite, test_deserialise_string)
{
    Lwm2m_SetLogLevel(DebugLevel_Debug);

    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 0, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 0, 0, AwaStaticResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_RW, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 0, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 0, 0, 0, 0, (char*)"Open Mobile Alliance", strlen("Open Mobile Alliance"));

    const char * expected = "Open Mobile Alliance";

    Lwm2mTreeNode * dest;
    SerdesContext serdesContext;
    int result = PTDeserialiseResource(&serdesContext, &dest, Lwm2mCore_GetDefinitions(context), 0, 0, 0, (const uint8_t * )expected, strlen(expected));

    Lwm2mTreeNode * resourceInstanceNode = Lwm2mTreeNode_GetFirstChild(dest);

    uint16_t length;
    const uint8_t * value = Lwm2mTreeNode_GetValue(resourceInstanceNode, &length);

    EXPECT_EQ(0, result);
    EXPECT_EQ(static_cast<int>(strlen(expected)), length);
    EXPECT_EQ(0, memcmp(value, expected, strlen(expected))) << value << " != " << expected;

    Lwm2mTreeNode_DeleteRecursive(dest);
}

TEST_F(PlainTextTestSuite, test_serialise_float)
{
    Lwm2m_SetLogLevel(DebugLevel_Debug);

    double expected = 0.12345;

    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 0, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 0, 0, AwaStaticResourceType_Float, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_RW, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 0, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 0, 0, 0, 0, &expected, sizeof(expected));

    uint8_t buffer[512];
    memset(buffer, 0, 512);

    Lwm2mTreeNode * dest;
    int OIR[] = {0,0,0};
    TreeBuilder_CreateTreeFromOIR(&dest, context, Lwm2mRequestOrigin_Client, OIR, 3);

    SerdesContext serdesContext;
    PTSerialiseResource(&serdesContext, dest, 0, 0, 0, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    EXPECT_EQ(expected, atof((char*)buffer)) << "BUFFER IS " << buffer;
}

TEST_F(PlainTextTestSuite, test_deserialise_float)
{
    Lwm2m_SetLogLevel(DebugLevel_Debug);

    double expected = 0.12345;
    char buffer[512];
    snprintf(buffer,sizeof(buffer),"%lf",expected);
    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 0, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 0, 0, AwaStaticResourceType_Float, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_RW, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 0, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 0, 0, 0, 0, &expected, sizeof(expected));


    Lwm2mTreeNode * dest;
    SerdesContext serdesContext;
    int result = PTDeserialiseResource(&serdesContext, &dest, Lwm2mCore_GetDefinitions(context), 0, 0, 0, (const uint8_t * )buffer, strlen(buffer));

    Lwm2mTreeNode * resourceInstanceNode = Lwm2mTreeNode_GetFirstChild(dest);

    uint16_t length;
    const uint8_t * value = Lwm2mTreeNode_GetValue(resourceInstanceNode, &length);

    EXPECT_EQ(0, result);
    EXPECT_EQ(strlen(buffer), length);
    EXPECT_EQ(expected, *(double*)value);

   Lwm2mTreeNode_DeleteRecursive(dest);

}





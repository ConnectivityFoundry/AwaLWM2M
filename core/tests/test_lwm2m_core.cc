#include <gtest/gtest.h>
#include <string>
#include <stdio.h>

// https://meekrosoft.wordpress.com/2009/11/09/unit-testing-c-code-with-the-googletest-framework/
// 1. Define fake functions for the dependencies you want to stub out
// 2. If the module depends on a global (gasp!) you need to define your fake one
// 3. include your module implementation (#include module.c)
// 4. Define a method to reset all the static data to a known state.
// 5. Define your tests

#include "common/lwm2m_object_store.h"
#include "lwm2m_core.h"

#include "common/lwm2m_object_store.h"
#include "common/lwm2m_object_defs.h"

class Lwm2mCoreTestSuite : public testing::Test
{
  void SetUp() { context = Lwm2mCore_Init(NULL, (char *)"123456"); }
  void TearDown() { Lwm2mCore_Destroy(context); }
protected:
  Lwm2mContextType * context;
};

#if 0
TEST_F(Lwm2mCoreTestSuite, test_handle_valid_get_request)
{
    //Lwm2m_SetLogLevel(DebugLevel_Emerg);  // disable output
    //Lwm2m_SetLogLevel(DebugLevel_

    const char * expected = "coap://bootstrap.example.com:5684/";
    int expectedLength = strlen(expected);

    Definition_RegisterObjectType(context->Store, (char*)"Test", 0, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory);
    Definition_RegisterResourceType(context->Store, (char*)"Res1", 0, 0, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_RW);
    ObjectStore_SetResourceInstanceValue(context->Store, 0, 0, 0, 0, static_cast<const char*>(expected), strlen(expected));

    char responseBuffer[512] = { 0 };
    int responseBufferLen = sizeof(responseBuffer);
    int responseCode = 0;
    ContentType responseContentType = ContentType_None;

    char path[] = "/0/0/0";  // passing the path as a variable avoids an annoying C++ warning
        AddressType addr;
        char query[] = "";

    Lwm2mCore_HandleGetRequest(context, &addr, path, query, ContentType_ApplicationPlainText, NULL, 0, &responseContentType, responseBuffer, &responseBufferLen, &responseCode);

    // TODO: add more assertions
    EXPECT_EQ(ContentType_ApplicationPlainText, responseContentType);
    EXPECT_EQ(expectedLength, responseBufferLen);
    EXPECT_STREQ(expected, responseBuffer);
}
#endif

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

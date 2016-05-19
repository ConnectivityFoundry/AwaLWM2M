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

#include "lwm2m_core.h"

class ObjectStoreInterfaceTestSuite : public testing::Test
{
  void SetUp() { context = Lwm2mCore_Init(NULL, NULL); }
  void TearDown() { Lwm2mCore_Destroy(context); }
protected:
  Lwm2mContextType * context;
};

TEST_F(ObjectStoreInterfaceTestSuite, test_WriteAndReadSingleResource)
{
    const char * expected = "coap://bootstrap.example.com:5684/";

    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 0, 1, 0, &defaultObjectOperationHandlers);
    Definition_RegisterResourceType(Lwm2mCore_GetDefinitions(context), (char*)"Res1", 0, 0, AwaResourceType_String, 1, 1, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers, NULL);
    Lwm2mCore_CreateObjectInstance(context, 0, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 0, 0, 0, 0, static_cast<const char*>(expected), strlen(expected));

    const char * buffer;
    size_t bufferSize = 0;
    int len = Lwm2mCore_GetResourceInstanceValue(context, 0, 0, 0, 0, (const void **)&buffer, &bufferSize);

    EXPECT_EQ(static_cast<size_t>(len), strlen(expected));
    ASSERT_TRUE(len >= 0);
    EXPECT_TRUE(memcmp(expected, buffer, static_cast<size_t>(len)) == 0);
}

//TEST_F(ObjectStoreInterfaceTestSuite, DISABLED_test_EnumerateObjectsAndResources)
//{
//  printf("Store size: %d\n", ObjectStatic_GetTotalSize());
//
//  ObjectIDType obj = -1;
//  do
//  {
//      obj = ObjectStatic_GetNextObjectType(obj);
//      if(obj != -1)
//      {
//          ResourceIDType res = -1;
////            printf("Object ID: %d\n", obj);
//          do
//          {
//              res = ObjectStatic_GetNextResourceType(obj, res);
//
//              if(res != -1)
//              {
////                    printf("\tResource ID: %d\n", res);
//              }
//          } while(res != -1);
//      }
//
//  } while(obj != -1);
//}

TEST_F(ObjectStoreInterfaceTestSuite, test_WriteAndReadMultipleResources)
{
    std::vector<std::string> expected = {
            "coap://bootstrap.example.com:5684/",
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
            "The Quick Brown Fox Jumped Over tHe LaZy DoG!?...",
            "FOUR",
            "1 0 01 1 0 01 10 11 101 11 1011 1011 1000 1010000 11 101 10",
            "first line\nsecond line",
    };

    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), static_cast<const char*>("Test"), 0, 1, 0, &defaultObjectOperationHandlers);

    // register string resources
    int i = 0;
    for(auto it = expected.begin(); it < expected.end(); ++it, ++i)
    {
        char resourceName[100];
        snprintf(resourceName, sizeof(resourceName), "Res%d", i);
        Definition_RegisterResourceType(Lwm2mCore_GetDefinitions(context), static_cast<const char*>(resourceName), 0, i, AwaResourceType_String, 1, 1, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers, NULL);

    }

    Lwm2mCore_CreateObjectInstance(context, 0, 0);

    i = 0;
    for(auto it = expected.begin(); it < expected.end(); ++it, ++i)
    {
        Lwm2mCore_SetResourceInstanceValue(context, 0, 0, i, 0, (*it).c_str(), (*it).length());
    }

    // read back
    i = 0;
    for(auto it = expected.begin(); it < expected.end(); ++it, ++i)
    {
        const char * buffer = NULL;
        size_t len = 0;
        Lwm2mCore_GetResourceInstanceValue(context, 0, 0, i, 0, (const void **)&buffer, &len);
        ASSERT_TRUE(buffer != NULL);
    }
}

TEST_F(ObjectStoreInterfaceTestSuite, test_WriteAndReadMultipleResourcesWithSparseIds)
{
    struct Items {
        Items(std::string s_, int id_) : s(s_), id(id_) {}
        std::string s;
        int id;
    };

    std::vector<Items> expected = {
            Items("coap://bootstrap.example.com:5684/", 17),
            Items("ABCDEFGHIJKLMNOPQRSTUVWXYZ", 25),
            Items("The Quick Brown Fox Jumped Over tHe LaZy DoG!?...", 500),
            Items("FOUR", 501),
            Items("1 0 01 1 0 01 10 11 101 11 1011 1011 1000 1010000 11 101 10", 9999),
            Items("first line\nsecond line", 32767),
    };

    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), static_cast<const char*>("Test"), 0, 1, 0, &defaultObjectOperationHandlers);

    // register string resources
    int i = 0;
    for(auto it = expected.begin(); it < expected.end(); ++it, ++i)
    {
        char resourceName[100];
        snprintf(resourceName, sizeof(resourceName), "Res%d", i);
        Definition_RegisterResourceType(Lwm2mCore_GetDefinitions(context), static_cast<const char*>(resourceName), 0, it->id, AwaResourceType_String, 1, 1, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers, NULL);
    }

    Lwm2mCore_CreateObjectInstance(context, 0, 0);

    // write the values
    i = 0;
    for(auto it = expected.begin(); it < expected.end(); ++it, ++i)
    {
        Lwm2mCore_SetResourceInstanceValue(context, 0, 0, it->id, 0, it->s.c_str(), it->s.length());
    }

    // read back
    i = 0;
    for(auto it = expected.begin(); it < expected.end(); ++it, ++i)
    {
        const char * buffer = NULL;
        size_t len = 0;
        Lwm2mCore_GetResourceInstanceValue(context, 0, 0, it->id, 0, (const void **)&buffer, &len);
        ASSERT_TRUE(buffer != NULL);

        const char * expected = (it->s).c_str();
        EXPECT_EQ(static_cast<size_t>(len), strlen(expected));
        EXPECT_TRUE(memcmp(expected, buffer, len) == 0);
    }
}

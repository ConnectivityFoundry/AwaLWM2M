
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
    Definition_RegisterResourceType(Lwm2mCore_GetDefinitions(context), (char*)"Res1", 0, 0, AwaResourceType_String, 1, 1, Operations_RW, &defaultResourceOperationHandlers, NULL);
    Lwm2mCore_CreateObjectInstance(context, 0, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 0, 0, 0, 0, static_cast<const char*>(expected), strlen(expected));

    const char * buffer;
    int bufferSize = 0;
    int len = Lwm2mCore_GetResourceInstanceValue(context, 0, 0, 0, 0, (const void **)&buffer, &bufferSize);

    EXPECT_EQ(static_cast<size_t>(len), strlen(buffer) + 1);
    EXPECT_EQ(strlen(expected), strlen(buffer));
    EXPECT_STREQ(expected, buffer);
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
        Definition_RegisterResourceType(Lwm2mCore_GetDefinitions(context), static_cast<const char*>(resourceName), 0, i, AwaResourceType_String, 1, 1, Operations_RW, &defaultResourceOperationHandlers, NULL);

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
        int len = 0;
        Lwm2mCore_GetResourceInstanceValue(context, 0, 0, i, 0, (const void **)&buffer, &len);
        ASSERT_TRUE(buffer != NULL);
        std::string actual(buffer);
        //std::cout << i << ": expected " << *it << ", actual " << actual << std::endl;
        EXPECT_EQ(static_cast<size_t>(len), actual.length() + 1);
        EXPECT_EQ((*it).length(), actual.length());
        EXPECT_EQ(*it, std::string(buffer));
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
        Definition_RegisterResourceType(Lwm2mCore_GetDefinitions(context), static_cast<const char*>(resourceName), 0, it->id, AwaResourceType_String, 1, 1, Operations_RW, &defaultResourceOperationHandlers, NULL);
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
        int len = 0;
        Lwm2mCore_GetResourceInstanceValue(context, 0, 0, it->id, 0, (const void **)&buffer, &len);
        ASSERT_TRUE(buffer != NULL);

        std::string actual(buffer);
        //std::cout << i << ": expected " << *it << ", actual " << actual << std::endl;
        EXPECT_EQ(static_cast<size_t>(len), actual.length() + 1);
        EXPECT_EQ(it->s.length(), actual.length());
        EXPECT_EQ(it->s, std::string(buffer));
    }
}

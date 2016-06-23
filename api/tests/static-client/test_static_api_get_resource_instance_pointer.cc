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

namespace TestStaticClientGetResourceInstancePointerDetail
{

static AwaInteger dummyInteger1 = 123456;
static const char * dummyString1 = "Lightweight M2M Server";
static AwaFloat dummyFloat1 = 1.0;
static AwaTime dummyTime1 = 0xA20AD72B;
static AwaBoolean dummyBoolean1 = true;
//static char dummyOpaqueData[] = {'a',0,'x','\0', 123};
//static int dummyObjLinkData[] = {-1,-1};

AwaObjectLink dummyObjectLink1 = { 3, 5 };

const char * dummyStringArray1[] = {"Lightweight M2M Server", "test1", ""};
const char * dummyStringArray2[] = {"Lightweight M2M Server", "test1", "", "", "", ""};
const AwaInteger dummyIntegerArray1[] = {55, 8732, 11};
const AwaInteger dummyIntegerArray2[] = {55, 8732, 11, 55, 8732, 11};
const AwaFloat dummyFloatArray1[] = {55.0, 0.0008732, 11e10};
const AwaFloat dummyFloatArray2[] = {55.0, 0.0008732, 11e10, 55.0, 0.0008732, 11e10};
const AwaBoolean dummyBooleanArray1[] = {true, false, true};
const AwaBoolean dummyBooleanArray2[] = {true, false, true, true, false, true};

const AwaInteger dummyTimeArray1[] = {16000, 8732222, 1111};
const AwaInteger dummyTimeArray2[] = {16000, 8732222, 1111, 16000, 8732222, 1111};

const AwaObjectID TEST_OBJECT_NON_ARRAY_TYPES = 10000;
const AwaResourceID TEST_RESOURCE_STRING = 1;
const AwaResourceID TEST_RESOURCE_INTEGER = 2;
const AwaResourceID TEST_RESOURCE_FLOAT = 3;
const AwaResourceID TEST_RESOURCE_BOOLEAN = 4;
const AwaResourceID TEST_RESOURCE_OPAQUE = 5;
const AwaResourceID TEST_RESOURCE_TIME = 6;
const AwaResourceID TEST_RESOURCE_OBJECTLINK = 7;

const AwaObjectID TEST_OBJECT_ARRAY_TYPES = 10001;
const AwaResourceID TEST_RESOURCE_STRINGARRAY = 1;
const AwaResourceID TEST_RESOURCE_INTEGERARRAY = 2;
const AwaResourceID TEST_RESOURCE_FLOATARRAY = 3;
const AwaResourceID TEST_RESOURCE_BOOLEANARRAY = 4;
const AwaResourceID TEST_RESOURCE_OPAQUEARRAY = 5;
const AwaResourceID TEST_RESOURCE_TIMEARRAY = 6;
const AwaResourceID TEST_RESOURCE_OBJECTLINKARRAY = 7;

} // namespace TestStaticClientGetResourceInstancePointerDetail

struct TestGetResourceInstancePointerData
{
    AwaObjectID ObjectID;
    AwaObjectInstanceID ObjectInstanceID;
    AwaResourceID ResourceID;

    const void * Value;
    size_t ValueSize;
    AwaResourceType Type;
};

::std::ostream& operator<<(::std::ostream& os, const TestGetResourceInstancePointerData& item)
{
  return os << ", ObjectID " << item.ObjectID
            << ", ObjectInstanceID " << item.ObjectInstanceID
            << ", ResourceID " << item.ResourceID
            << ", Value " << item.Value
            << ", ValueCount " << item.ValueSize
            << ", Type " << item.Type;
}

class TestStaticClientGetResourceInstancePointer : public TestStaticClientWithServer, public ::testing::WithParamInterface< TestGetResourceInstancePointerData >
{

protected:

    void SetUp() {
        TestStaticClientWithServer::SetUp();
        TestGetResourceInstancePointerData data = GetParam();

        EXPECT_EQ(AwaError_Success,AwaStaticClient_DefineObject(client_, data.ObjectID, "TestObject", 0, 1));

        StaticClientAllocedValue_ = malloc(data.ValueSize);
        EXPECT_TRUE(StaticClientAllocedValue_ != NULL);
        memset(StaticClientAllocedValue_, 0, data.ValueSize);
        EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResource(client_, data.ObjectID, data.ResourceID, "Test Resource", data.Type, 1, 1, AwaResourceOperations_ReadWrite));
        EXPECT_EQ(AwaError_Success, AwaStaticClient_SetResourceStorageWithPointer(client_, data.ObjectID, data.ResourceID, StaticClientAllocedValue_, data.ValueSize, 0));

        EXPECT_EQ(AwaError_Success, AwaStaticClient_CreateObjectInstance(client_, data.ObjectID, 0));

        SingleStaticClientWaitCondition condition(client_, session_, global::clientEndpointName, global::timeout);
        EXPECT_TRUE(condition.Wait());

        AwaServerDefineOperation * defineOperation = AwaServerDefineOperation_New(session_);
        EXPECT_TRUE(defineOperation != NULL);
        AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(data.ObjectID, "TestObject", 0, 1);
        EXPECT_TRUE(objectDefinition != NULL);

        switch (data.Type)
        {
            case AwaResourceType_String:
                EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, data.ResourceID, "Test Resource", true, AwaResourceOperations_ReadWrite, (const char *)data.Value));
                break;
            case AwaResourceType_Integer:
                EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefinition, data.ResourceID, "Test Resource", true, AwaResourceOperations_ReadWrite, *((AwaInteger *)data.Value)));
                break;
            case AwaResourceType_Float:
                EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloat(objectDefinition, data.ResourceID, "Test Resource", true, AwaResourceOperations_ReadWrite, *((AwaFloat *)data.Value)));
                break;
            case AwaResourceType_Boolean:
                EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBoolean(objectDefinition, data.ResourceID, "Test Resource", true, AwaResourceOperations_ReadWrite, *((AwaBoolean *)data.Value)));
                break;
            case AwaResourceType_Opaque:
            {
                AwaOpaque opaque = {(void *)data.Value, data.ValueSize};
                EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque(objectDefinition, data.ResourceID, "Test Resource", true, AwaResourceOperations_ReadWrite, opaque));
                break;
            }
            case AwaResourceType_Time:
                EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTime(objectDefinition, data.ResourceID, "Test Resource", true, AwaResourceOperations_ReadWrite, *((AwaTime *)data.Value)));
                break;
            case AwaResourceType_ObjectLink:
                EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLink(objectDefinition, data.ResourceID, "Test Resource", true, AwaResourceOperations_ReadWrite, *((AwaObjectLink *)data.Value)));
                break;
            default:
                ASSERT_TRUE(false);
        }

        EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(defineOperation, objectDefinition));
        EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(defineOperation, global::timeout));

        AwaObjectDefinition_Free(&objectDefinition);
        AwaServerDefineOperation_Free(&defineOperation);

        writeOperation_ = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
        EXPECT_TRUE(NULL != writeOperation_);

        readOperation_ = AwaServerReadOperation_New(session_);
        EXPECT_TRUE(NULL != readOperation_);


        EXPECT_EQ(AwaError_Success, AwaAPI_MakeResourcePath(StaticClientResourcePath_, sizeof(StaticClientResourcePath_), data.ObjectID, data.ObjectInstanceID, data.ResourceID));
        AwaServerReadOperation_AddPath(readOperation_, global::clientEndpointName, StaticClientResourcePath_);
    }

    void TearDown() {
        free(StaticClientAllocedValue_);
        AwaServerWriteOperation_Free(&writeOperation_);
        AwaServerReadOperation_Free(&readOperation_);
        TestStaticClientWithServer::TearDown();
    }

    AwaServerWriteOperation * writeOperation_;
    AwaServerReadOperation * readOperation_;
    void * StaticClientAllocedValue_;
    char StaticClientResourcePath_[256] = {0};
};


TEST_P(TestStaticClientGetResourceInstancePointer, TestGetResourceInstancePointer)
{
    TestGetResourceInstancePointerData data = GetParam();

    size_t ValueSize = 0;
    memcpy(StaticClientAllocedValue_, data.Value, data.ValueSize);
    ASSERT_EQ(StaticClientAllocedValue_, AwaStaticClient_GetResourceInstancePointer(client_, data.ObjectID, data.ObjectInstanceID, data.ResourceID, 0, &ValueSize));

    //ValueSize is returned as strlen so need to +1 to get total memory
    if(data.Type == AwaResourceType_String)
        ValueSize++;
    ASSERT_EQ(data.ValueSize, ValueSize);

    StaticClientProccessInfo processInfo = { .Run = true, .StaticClient = client_ };
    pthread_t processThread;

    pthread_create(&processThread, NULL, do_static_client_process, &processInfo);
    AwaServerReadOperation_Perform(readOperation_, global::timeout * 20);
    processInfo.Run = false;
    pthread_join(processThread, NULL);

    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation_, global::clientEndpointName);
    EXPECT_TRUE(readResponse != NULL);

    const void * readValue = NULL;

    switch(data.Type)
    {
        case AwaResourceType_String:
            EXPECT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsCStringPointer(readResponse, StaticClientResourcePath_, (const char **)&readValue));
            break;
        case AwaResourceType_Integer:
            EXPECT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsIntegerPointer(readResponse, StaticClientResourcePath_, (const AwaInteger **)&readValue));
            break;
        case AwaResourceType_Float:
            EXPECT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsFloatPointer(readResponse, StaticClientResourcePath_, (const AwaFloat **)&readValue));
            break;
        case AwaResourceType_Boolean:
            EXPECT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsBooleanPointer(readResponse, StaticClientResourcePath_, (const AwaBoolean **)&readValue));
            break;
        case AwaResourceType_Time:
            EXPECT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsTimePointer(readResponse, StaticClientResourcePath_, (const AwaTime **)&readValue));
            break;
        case AwaResourceType_ObjectLink:
            EXPECT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsObjectLinkPointer(readResponse, StaticClientResourcePath_, (const AwaObjectLink **)&readValue));
            break;
        default:
            ASSERT_TRUE(false);
            break;
    }

    ASSERT_EQ(0, memcmp(readValue, StaticClientAllocedValue_, data.ValueSize));
    memset( StaticClientAllocedValue_, 0, data.ValueSize);

    switch(data.Type)
    {
        case AwaResourceType_String:
            EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsCString(writeOperation_, StaticClientResourcePath_, (const char *)data.Value));
            break;
        case AwaResourceType_Integer:
            EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation_, StaticClientResourcePath_, *((AwaInteger *)data.Value)));
            break;
        case AwaResourceType_Float:
            EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsFloat(writeOperation_, StaticClientResourcePath_, *((AwaFloat *)data.Value)));
            break;
        case AwaResourceType_Boolean:
            EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsBoolean(writeOperation_, StaticClientResourcePath_, *((AwaBoolean *)data.Value)));
            break;
        case AwaResourceType_Time:
            EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsTime(writeOperation_, StaticClientResourcePath_, *((AwaTime *)data.Value)));
            break;
        case AwaResourceType_ObjectLink:
            EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsObjectLink(writeOperation_, StaticClientResourcePath_, *((AwaObjectLink *)data.Value)));
            break;
        default:
            ASSERT_TRUE(false);
            break;
    }

    processInfo.Run = true;
    pthread_create(&processThread, NULL, do_static_client_process, &processInfo);
    AwaServerWriteOperation_Perform(writeOperation_, global::clientEndpointName, global::timeout * 20);
    processInfo.Run = false;
    pthread_join(processThread, NULL);


    ASSERT_EQ(0, memcmp(data.Value, StaticClientAllocedValue_, data.ValueSize));
}

INSTANTIATE_TEST_CASE_P(
        TestStaticClientGetResourceInstancePointer,
        TestStaticClientGetResourceInstancePointer,
        ::testing::Values(
          TestGetResourceInstancePointerData { TestStaticClientGetResourceInstancePointerDetail::TEST_OBJECT_NON_ARRAY_TYPES, 0, TestStaticClientGetResourceInstancePointerDetail::TEST_RESOURCE_STRING,     TestStaticClientGetResourceInstancePointerDetail::dummyString1, strlen(TestStaticClientGetResourceInstancePointerDetail::dummyString1) + 1, AwaResourceType_String},
          TestGetResourceInstancePointerData { TestStaticClientGetResourceInstancePointerDetail::TEST_OBJECT_NON_ARRAY_TYPES, 0, TestStaticClientGetResourceInstancePointerDetail::TEST_RESOURCE_INTEGER,    &TestStaticClientGetResourceInstancePointerDetail::dummyInteger1, sizeof(AwaInteger), AwaResourceType_Integer },
          TestGetResourceInstancePointerData { TestStaticClientGetResourceInstancePointerDetail::TEST_OBJECT_NON_ARRAY_TYPES, 0, TestStaticClientGetResourceInstancePointerDetail::TEST_RESOURCE_INTEGER,    &TestStaticClientGetResourceInstancePointerDetail::dummyFloat1, sizeof(AwaFloat), AwaResourceType_Float },
          TestGetResourceInstancePointerData { TestStaticClientGetResourceInstancePointerDetail::TEST_OBJECT_NON_ARRAY_TYPES, 0, TestStaticClientGetResourceInstancePointerDetail::TEST_RESOURCE_INTEGER,    &TestStaticClientGetResourceInstancePointerDetail::dummyBoolean1, sizeof(AwaBoolean), AwaResourceType_Boolean },
          TestGetResourceInstancePointerData { TestStaticClientGetResourceInstancePointerDetail::TEST_OBJECT_NON_ARRAY_TYPES, 0, TestStaticClientGetResourceInstancePointerDetail::TEST_RESOURCE_INTEGER,    &TestStaticClientGetResourceInstancePointerDetail::dummyTime1, sizeof(AwaTime), AwaResourceType_Time },
          TestGetResourceInstancePointerData { TestStaticClientGetResourceInstancePointerDetail::TEST_OBJECT_NON_ARRAY_TYPES, 0, TestStaticClientGetResourceInstancePointerDetail::TEST_RESOURCE_OBJECTLINK, &TestStaticClientGetResourceInstancePointerDetail::dummyObjectLink1, sizeof(AwaObjectLink), AwaResourceType_ObjectLink}
        ));

} // namespace Awa

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

class TestStaticClientWithPointerWithServer : public TestStaticClientWithServer {};

TEST_F(TestStaticClientWithPointerWithServer, AwaStaticClient_Define_Invalid)
{
  EXPECT_EQ(AwaError_StaticClientInvalid, AwaStaticClient_DefineObject(NULL, 7997, "TestObject", 0, 1));
    EXPECT_EQ(AwaError_DefinitionInvalid,   AwaStaticClient_DefineObject(client_, 7997, NULL, 0, 1));
    EXPECT_EQ(AwaError_DefinitionInvalid,   AwaStaticClient_DefineObject(client_, 7997, "TestObject", 2, 1));
    EXPECT_EQ(AwaError_Success,             AwaStaticClient_DefineObject(client_, 7997, "TestObject", 0, 1)); // valid

    EXPECT_EQ(AwaError_StaticClientInvalid, AwaStaticClient_DefineResource(NULL,    7997,  1, "TestResource", AwaResourceType_Opaque, 1, 1, AwaResourceOperations_ReadWrite));
    EXPECT_EQ(AwaError_DefinitionInvalid,   AwaStaticClient_DefineResource(client_, 300,   1, "TestResource", AwaResourceType_Opaque, 1, 1, AwaResourceOperations_ReadWrite));
    EXPECT_EQ(AwaError_DefinitionInvalid,   AwaStaticClient_DefineResource(client_, 7997,  1, "TestResource", AwaResourceType_Opaque, 2, 1, AwaResourceOperations_ReadWrite));
    EXPECT_EQ(AwaError_DefinitionInvalid,   AwaStaticClient_DefineResource(client_, 7997,  1, NULL,           AwaResourceType_Opaque, 1, 1, AwaResourceOperations_ReadWrite));
}

TEST_F(TestStaticClientWithPointerWithServer, AwaStaticClient_SetResourceStorageWithPointer_Invalid)
{
    uint8_t o[10];
    ASSERT_EQ(AwaError_Success,             AwaStaticClient_DefineObject(client_, 7997, "TestObject", 0, 1));
    ASSERT_EQ(AwaError_Success,             AwaStaticClient_DefineResource(client_, 7997, 1, "TestResource", AwaResourceType_Opaque, 1, 1, AwaResourceOperations_ReadWrite));
    EXPECT_EQ(AwaError_StaticClientInvalid, AwaStaticClient_SetResourceStorageWithPointer(NULL,    7997,  1, &o,   sizeof(o), 0));
    EXPECT_EQ(AwaError_DefinitionInvalid,   AwaStaticClient_SetResourceStorageWithPointer(client_, 7996,  1, &o,   sizeof(o), 0));
    EXPECT_EQ(AwaError_DefinitionInvalid,   AwaStaticClient_SetResourceStorageWithPointer(client_, 7997,  2, &o,   sizeof(o), 0));
    EXPECT_EQ(AwaError_DefinitionInvalid,   AwaStaticClient_SetResourceStorageWithPointer(client_, 7997,  1, &o,   0, 0));
    EXPECT_EQ(AwaError_DefinitionInvalid,   AwaStaticClient_SetResourceStorageWithPointer(client_, 7997,  1, NULL, 0, 0));
    EXPECT_EQ(AwaError_StaticClientInvalid, AwaStaticClient_SetResourceStorageWithPointer(NULL,    7996,  2, NULL, 0, 0));
}

TEST_F(TestStaticClientWithPointerWithServer, AwaStaticClient_SetResourceStorageWithPointer_Success)
{
    uint8_t o[10];
    ASSERT_EQ(AwaError_Success,             AwaStaticClient_DefineObject(client_, 7997, "TestObject", 0, 1));
    ASSERT_EQ(AwaError_Success,             AwaStaticClient_DefineResource(client_, 7997, 1, "TestResource", AwaResourceType_Opaque, 1, 1, AwaResourceOperations_ReadWrite));
    EXPECT_EQ(AwaError_Success,             AwaStaticClient_SetResourceStorageWithPointer(client_,  7997,  1, &o, sizeof(o), 0));
}

TEST_F(TestStaticClientWithPointerWithServer, AwaStaticClient_SetResourceStorageWithPointerArray_Invalid)
{
    uint8_t o1[10];
    uint8_t o2[10];
    uint8_t o3[10];
    void * pointers[] = {&o1, &o2, &o3, NULL};

    ASSERT_EQ(AwaError_Success, AwaStaticClient_DefineObject(client_, 7996, "TestObject", 0, 1));
    ASSERT_EQ(AwaError_Success, AwaStaticClient_DefineResource(client_, 7996, 1, "TestResource", AwaResourceType_Opaque, 1, 3, AwaResourceOperations_ReadWrite));

    EXPECT_EQ(AwaError_StaticClientInvalid, AwaStaticClient_SetResourceStorageWithPointerArray(NULL,    7996, 1, pointers, sizeof(o1)));
    EXPECT_EQ(AwaError_DefinitionInvalid,   AwaStaticClient_SetResourceStorageWithPointerArray(client_, 7995, 1, pointers, sizeof(o1)));
    EXPECT_EQ(AwaError_DefinitionInvalid,   AwaStaticClient_SetResourceStorageWithPointerArray(client_, 7996, 2, pointers, sizeof(o1)));
    EXPECT_EQ(AwaError_DefinitionInvalid,   AwaStaticClient_SetResourceStorageWithPointerArray(client_, 7996, 1, NULL,     sizeof(o1)));
    EXPECT_EQ(AwaError_DefinitionInvalid,   AwaStaticClient_SetResourceStorageWithPointerArray(client_, 7996, 1, pointers, 0));
    EXPECT_EQ(AwaError_StaticClientInvalid, AwaStaticClient_SetResourceStorageWithPointerArray(NULL,    7995, 2, NULL,     0));
}

TEST_F(TestStaticClientWithPointerWithServer, AwaStaticClient_SetResourceStorageWithPointerArray_Success)
{
    uint8_t o1[10];
    uint8_t o2[10];
    uint8_t o3[10];
    void * pointers[] = {&o1, &o2, &o3, NULL};

    ASSERT_EQ(AwaError_Success, AwaStaticClient_DefineObject(client_, 7996, "TestObject", 0, 1));
    ASSERT_EQ(AwaError_Success, AwaStaticClient_DefineResource(client_, 7996, 1, "TestResource", AwaResourceType_Opaque, 1, 3, AwaResourceOperations_ReadWrite));

    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetResourceStorageWithPointerArray(client_, 7996,  1, pointers, sizeof(o1)));
}

TEST_F(TestStaticClientWithPointerWithServer, AwaStaticClient_CreateObjectInstance_Resource_Invalid)
{
    AwaInteger i = 0;

    EXPECT_EQ(AwaError_StaticClientInvalid, AwaStaticClient_CreateObjectInstance(NULL, 9999, 0));

    EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineObject(client_, 205, "TestObject", 0, 1));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResource(client_, 205, 1, "Resource", AwaResourceType_Integer, 0, 1, AwaResourceOperations_ReadWrite));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetResourceStorageWithPointer(client_, 205, 1, &i, sizeof(i), 0));

    EXPECT_EQ(AwaError_Success, AwaStaticClient_CreateObjectInstance(client_, 205, 0));
    EXPECT_EQ(AwaError_CannotCreate, AwaStaticClient_CreateObjectInstance(client_, 205, 0));

    EXPECT_EQ(AwaError_StaticClientInvalid, AwaStaticClient_CreateResource(NULL, 9999, 0, 1));
    EXPECT_EQ(AwaError_StaticClientInvalid, AwaStaticClient_CreateResource(NULL, 9999, 0, 1));
    EXPECT_EQ(AwaError_StaticClientInvalid, AwaStaticClient_CreateResource(NULL, 9999, 0, 1));

    EXPECT_EQ(AwaError_Success, AwaStaticClient_CreateResource(client_, 205, 0, 1));
    EXPECT_EQ(AwaError_CannotCreate, AwaStaticClient_CreateResource(client_, 205, 0, 1));
}

TEST_F(TestStaticClientWithPointerWithServer, AwaStaticClient_WithPointer_Create_and_Write_Operation_for_Object_and_Resource)
{
    AwaInteger i = 10;

    EXPECT_EQ(AwaError_Success,AwaStaticClient_DefineObject(client_, 7999, "TestObject", 0, 1));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResource(client_, 7999, 1, "TestResource", AwaResourceType_Integer, 1, 1, AwaResourceOperations_ReadWrite));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetResourceStorageWithPointer(client_, 7999, 1, &i, sizeof(i), 0));

    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_TRUE(NULL != operation);
    SingleStaticClientPollCondition condition(client_, operation, global::clientEndpointName, 10);
    EXPECT_TRUE(condition.Wait());
    AwaServerListClientsOperation_Free(&operation);

    AwaServerDefineOperation * defineOperation = AwaServerDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);
    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(7999, "TestObject", 0, 1);
    EXPECT_TRUE(objectDefinition != NULL);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefinition, 1, "TestResource", true, AwaResourceOperations_ReadWrite, 0));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(defineOperation, objectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(defineOperation, defaults::timeout));
    AwaServerDefineOperation_Free(&defineOperation);
    AwaObjectDefinition_Free(&objectDefinition);

    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    EXPECT_TRUE(writeOperation != NULL);
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/7999/0"));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/7999/0/1", 5));

    pthread_t writeThread;
    pthread_create(&writeThread, NULL, do_write_operation, (void *)writeOperation);
    AwaStaticClient_Process(client_);
    AwaStaticClient_Process(client_);
    pthread_join(writeThread, NULL);

    AwaServerWriteOperation_Free(&writeOperation);

    AwaStaticClient_Process(client_);

    ASSERT_EQ(5, i);
}

TEST_F(TestStaticClientWithPointerWithServer, AwaStaticClient_WithPointer_Create_and_Write_Operation_CoAPtimeout)
{
    // Static client definition
    uint8_t opaque[16] = {0};
    EXPECT_EQ(AwaError_Success,AwaStaticClient_DefineObject(client_, 7998, "TestObject", 0, 1));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResource(client_, 7998, 1, "TestResource", AwaResourceType_Opaque, 1, 1, AwaResourceOperations_ReadWrite));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetResourceStorageWithPointer(client_, 7998, 1, &opaque, sizeof(opaque), 0));

    // Server definition
    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_TRUE(NULL != operation);
    SingleStaticClientPollCondition condition(client_, operation, global::clientEndpointName, 10);
    EXPECT_TRUE(condition.Wait());
    AwaServerListClientsOperation_Free(&operation);

    AwaServerDefineOperation * defineOpertaion = AwaServerDefineOperation_New(session_);
    EXPECT_TRUE(defineOpertaion != NULL);
    AwaObjectDefinition * objectDefintion = AwaObjectDefinition_New(7998, "TestObject", 1, 1);
    EXPECT_TRUE(objectDefintion != NULL);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque(objectDefintion, 1, "TestResource", true, AwaResourceOperations_ReadWrite, AwaOpaque {0}));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(defineOpertaion, objectDefintion));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(defineOpertaion, defaults::timeout));
    AwaServerDefineOperation_Free(&defineOpertaion);
    AwaObjectDefinition_Free(&objectDefintion);

    // Write
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    EXPECT_TRUE(writeOperation != NULL);
    AwaOpaque o = { (void *)"Hello", 5 };
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/7998/0"));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsOpaque(writeOperation, "/7998/0/1", o));

    AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, 60 * 1000);

    const AwaServerWriteResponse * response = AwaServerWriteOperation_GetResponse(writeOperation, "TestIMG1");
    EXPECT_TRUE(NULL != response);
    const AwaPathResult * pathResult = AwaServerWriteResponse_GetPathResult(response, "/7998/0/1");
    EXPECT_EQ(AwaError_LWM2MError, AwaPathResult_GetError(pathResult));
    EXPECT_EQ(AwaLWM2MError_Timeout, AwaPathResult_GetLWM2MError(pathResult));

    AwaServerWriteOperation_Free(&writeOperation);
}


TEST_F(TestStaticClientWithPointerWithServer, AwaStaticClient_WithPointer_Create_and_Write_Operation_for_Object_and_Opaque_Resource)
{
    // Static client definition
    uint8_t opaque[16] = {0};
    EXPECT_EQ(AwaError_Success,AwaStaticClient_DefineObject(client_, 7998, "TestObject", 0, 1));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResource(client_, 7998, 1, "TestResource", AwaResourceType_Opaque, 1, 1, AwaResourceOperations_ReadWrite));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetResourceStorageWithPointer(client_, 7998, 1, opaque, sizeof(opaque), 0));

    // Server definition
    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_TRUE(NULL != operation);
    SingleStaticClientPollCondition condition(client_, operation, global::clientEndpointName, 10);
    EXPECT_TRUE(condition.Wait());
    AwaServerListClientsOperation_Free(&operation);

    AwaServerDefineOperation * defineOpertaion = AwaServerDefineOperation_New(session_);
    EXPECT_TRUE(defineOpertaion != NULL);
    AwaObjectDefinition * objectDefintion = AwaObjectDefinition_New(7998, "TestObject", 1, 1);
    EXPECT_TRUE(objectDefintion != NULL);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque(objectDefintion, 1, "TestResource", true, AwaResourceOperations_ReadWrite, AwaOpaque {0}));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(defineOpertaion, objectDefintion));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(defineOpertaion, defaults::timeout));
    AwaServerDefineOperation_Free(&defineOpertaion);
    AwaObjectDefinition_Free(&objectDefintion);

    // Write
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    EXPECT_TRUE(writeOperation != NULL);
    AwaOpaque o = { (void *)"Hello", 5 };
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/7998/0"));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsOpaque(writeOperation, "/7998/0/1", o));

    pthread_t writeThread;
    pthread_create(&writeThread, NULL, do_write_operation, (void *)writeOperation);
    AwaStaticClient_Process(client_);
    AwaStaticClient_Process(client_);
    pthread_join(writeThread, NULL);

    AwaServerWriteOperation_Free(&writeOperation);

    ASSERT_EQ(0, memcmp(opaque, o.Data, o.Size));

    AwaStaticClient_Process(client_);
    AwaStaticClient_Process(client_);

    // Read
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(session_);
    EXPECT_TRUE(readOperation != NULL);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/7998/0/1"));

    pthread_t readThread;
    pthread_create(&readThread, NULL, do_read_operation, (void *)readOperation);
    AwaStaticClient_Process(client_);
    AwaStaticClient_Process(client_);
    pthread_join(readThread, NULL);

    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    EXPECT_TRUE(readResponse != NULL);

    AwaOpaque * value;
    ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsOpaquePointer(readResponse, "/7998/0/1", (const AwaOpaque **)&value));
    ASSERT_EQ(sizeof(opaque), value->Size);
    ASSERT_TRUE(memcmp(value->Data, "Hello", 5) == 0);

    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestStaticClientWithPointerWithServer, AwaStaticClient_WithPointer_Create_and_Write_Operation_for_Object_and_String_Resource)
{
    // Static client definition
    char stringData[128] = {0};
    EXPECT_EQ(AwaError_Success,AwaStaticClient_DefineObject(client_, 7998, "TestObject", 0, 1));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResource(client_, 7998, 1, "TestResource", AwaResourceType_String, 1, 1, AwaResourceOperations_ReadWrite));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetResourceStorageWithPointer(client_, 7998, 1, &stringData, sizeof(stringData), 0));

    // Server definition
    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_TRUE(NULL != operation);
    SingleStaticClientPollCondition condition(client_, operation, global::clientEndpointName, 10);
    EXPECT_TRUE(condition.Wait());
    AwaServerListClientsOperation_Free(&operation);

    AwaServerDefineOperation * defineOpertaion = AwaServerDefineOperation_New(session_);
    EXPECT_TRUE(defineOpertaion != NULL);
    AwaObjectDefinition * objectDefintion = AwaObjectDefinition_New(7998, "TestObject", 1, 1);
    EXPECT_TRUE(objectDefintion != NULL);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsString(objectDefintion, 1, "TestResource", true, AwaResourceOperations_ReadWrite, ""));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(defineOpertaion, objectDefintion));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(defineOpertaion, defaults::timeout));
    AwaServerDefineOperation_Free(&defineOpertaion);
    AwaObjectDefinition_Free(&objectDefintion);

    // Write
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    EXPECT_TRUE(writeOperation != NULL);
    const char * writeData = "Hello";
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/7998/0"));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsCString(writeOperation, "/7998/0/1", writeData));

    pthread_t writeThread;
    pthread_create(&writeThread, NULL, do_write_operation, (void *)writeOperation);
    AwaStaticClient_Process(client_);
    AwaStaticClient_Process(client_);
    pthread_join(writeThread, NULL);

    AwaServerWriteOperation_Free(&writeOperation);

    ASSERT_EQ(strlen(writeData), strlen(stringData));
    ASSERT_EQ(0, memcmp(writeData, stringData, strlen(writeData)));

    AwaStaticClient_Process(client_);
    AwaStaticClient_Process(client_);

    // Read
    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(session_);
    EXPECT_TRUE(readOperation != NULL);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/7998/0/1"));

    pthread_t readThread;
    pthread_create(&readThread, NULL, do_read_operation, (void *)readOperation);
    AwaStaticClient_Process(client_);
    AwaStaticClient_Process(client_);
    pthread_join(readThread, NULL);

    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    EXPECT_TRUE(readResponse != NULL);

    char * value;
    ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsCStringPointer(readResponse, "/7998/0/1", (const char **)&value));
    ASSERT_EQ(strlen(writeData), strlen(value));
    ASSERT_EQ(0, memcmp(writeData, value, strlen(writeData)));

    AwaServerReadOperation_Free(&readOperation);
}

namespace observeDetail
{

struct TestObserveResource
{
    AwaError ExpectedResult;

    AwaObjectID ObjectID;
    AwaObjectInstanceID ObjectInstanceID;
    AwaResourceID ResourceID;

    const void * InitialValue;
    const void * ExpectedValue;
    AwaResourceType Type;

    bool UseOperation;
};

::std::ostream& operator<<(::std::ostream& os, const TestObserveResource& item)
{
  return os << "Item: ExpectedResult " << item.ExpectedResult
            << ", ObjectID " << item.ObjectID
            << ", ObjectInstanceID " << item.ObjectInstanceID
            << ", ResourceID " << item.ResourceID
            << ", InitialValue " << item.InitialValue
            << ", ExpectedValue " << item.ExpectedValue
            << ", Type " << item.Type
            << ", UseOperation " << item.UseOperation;
}

static const char * dummyInitialString1 = "Lightweight M2M Server";
static AwaInteger dummyInitialInteger1 = 123456;
static AwaFloat dummyInitialFloat1 = 1.0;
static AwaTime dummyInitialTime1 = 0xA20AD72B;
static AwaBoolean dummyInitialBoolean1 = true;
static char dummyInitialOpaqueData[] = {'a',0,'x','\0', 123};
AwaObjectLink dummyInitialObjectLink1 = { 3, 5 };

static const char * dummyExpectedString1 = "Heavyweight M2M Server";
static AwaInteger dummyExpectedInteger1 = 654321;
static AwaFloat dummyExpectedFloat1 = 2.3;
static AwaTime dummyExpectedTime1 = 0xEFAA5721;
static AwaBoolean dummyExpectedBoolean1 = false;
AwaObjectLink dummyExpectedObjectLink1 = {-1, -1};
static char dummyExpectedOpaqueData[] = {'c',0,'3','\0', 76};

const AwaObjectID TEST_OBJECT_NON_ARRAY_TYPES = 9500;
const AwaResourceID TEST_RESOURCE_STRING = 1;
const AwaResourceID TEST_RESOURCE_INTEGER = 2;
const AwaResourceID TEST_RESOURCE_FLOAT = 3;
const AwaResourceID TEST_RESOURCE_BOOLEAN = 4;
const AwaResourceID TEST_RESOURCE_OPAQUE = 5;
const AwaResourceID TEST_RESOURCE_TIME = 6;
const AwaResourceID TEST_RESOURCE_OBJECTLINK = 7;

} // namespace observeDetail

typedef struct
{
    AwaServerObserveOperation * ObserveOperation;
    volatile bool * ObserveThreadAlive;
} ServerObserveThreadContext;

static void * do_observe_operation(void * attr)
{
    ServerObserveThreadContext * context = (ServerObserveThreadContext *)attr;
    AwaError result = AwaServerObserveOperation_Perform(context->ObserveOperation, defaults::timeout);
    Lwm2m_Debug("AwaServerObserveOperation_Perform: %s\n", AwaError_ToString(result));
    *(context)->ObserveThreadAlive = false;
    return 0;
}

struct TestObserveStaticResource
{
    AwaObjectID ObjectID;
    AwaObjectInstanceID ObjectInstanceID;
    AwaResourceID ResourceID;

    void * Value;
    void * ExpectedValue;
    const size_t ValueSize;
    AwaResourceType Type;
};

::std::ostream& operator<<(::std::ostream& os, const TestObserveStaticResource& item)
{
  return os << "Item: ObjectID " << item.ObjectID
            << ", ObjectInstanceID " << item.ObjectInstanceID
            << ", ResourceID " << item.ResourceID
            << ", Value " << item.Value
            << ", ExpectedValue " << item.ExpectedValue
            << ", ValueSize " << item.ValueSize
            << ", Type " << item.Type;
}

class TestStaticClientObserveValue : public TestStaticClientWithServer, public ::testing::WithParamInterface< TestObserveStaticResource >
{
public:

    virtual void SetUp()
    {
        TestStaticClientWithServer::SetUp();
        cbHandler_ = new callback1(client_, 20, session_);
    }

    virtual void TearDown()
    {
        delete cbHandler_;
        TestStaticClientWithServer::TearDown();
    }

    struct callback1 : public StaticClientCallbackPollCondition
    {
        callback1(AwaStaticClient * StaticClient, int maxCount, AwaServerSession * session) : StaticClientCallbackPollCondition(StaticClient, maxCount),
                                                                                              session_(session){};
        virtual bool Check()
        {
            if (global::logLevel == AwaLogLevel_Debug)
                std::cout << "Check..." << std::endl;
            if (!observeThreadAlive_)
            {
                // only process the server session after we have successfully performed the observe operation.
                AwaServerSession_Process(session_, defaults::timeout);
                AwaServerSession_DispatchCallbacks(session_);
            }
            AwaStaticClient_Process(StaticClient);

            return complete;
        }
    public:
        volatile bool observeThreadAlive_ = true;
    private:
        AwaServerSession * session_;
    };

    void callbackHandler(const AwaChangeSet * changeSet)
    {
        Lwm2m_Debug("Received notification %d\n", notificationCount_);
        TestObserveStaticResource data = GetParam();
        const void * value = 0;

        char path[64];
        AwaAPI_MakeResourcePath(path, sizeof(path), data.ObjectID, data.ObjectInstanceID, data.ResourceID);

        switch(data.Type)
        {
            case AwaResourceType_Integer:
                AwaChangeSet_GetValueAsIntegerPointer(changeSet, path, (const AwaInteger **)&value);
                break;
            case AwaResourceType_String:
                AwaChangeSet_GetValueAsCStringPointer(changeSet, path, (const char **)&value);
                break;
            case AwaResourceType_Float:
                AwaChangeSet_GetValueAsFloatPointer(changeSet, path, (const AwaFloat **)&value);
                break;
            case AwaResourceType_Boolean:
                AwaChangeSet_GetValueAsBooleanPointer(changeSet, path, (const AwaBoolean **)&value);
                break;
            case AwaResourceType_Opaque:
                AwaChangeSet_GetValueAsOpaquePointer(changeSet, path, (const AwaOpaque **)&value);
                break;
            case AwaResourceType_Time:
                AwaChangeSet_GetValueAsTimePointer(changeSet, path, (const AwaTime **)&value);
                break;
            case AwaResourceType_ObjectLink:
                AwaChangeSet_GetValueAsObjectLinkPointer(changeSet, path, (const AwaObjectLink **)&value);
                break;
            default:
                ASSERT_TRUE(false);
                break;
        }
        //Lwm2m_Error("Received value: %zu\n", *value);

        switch(data.Type)
        {
            case AwaResourceType_String:
                EXPECT_STREQ((const char *)data.Value, (const char *)value);
                break;
            case AwaResourceType_Opaque:
            {
                AwaOpaque expected = {opaque_, sizeof(opaque_)};
                AwaOpaque * actual = (AwaOpaque *)value;
                EXPECT_EQ(expected.Size, actual->Size);
                EXPECT_EQ(0, memcmp(expected.Data, actual->Data, expected.Size));
                break;
            }
            default:
                EXPECT_EQ(0, memcmp(data.Value, value, data.ValueSize));
                break;
        }

        if (notificationCount_ == 0)
        {
            switch(data.Type)
            {
            case AwaResourceType_Opaque:
                memcpy(opaque_, data.ExpectedValue, data.ValueSize);
                break;
            default:
                data.Value = data.ExpectedValue;
                break;
            }
            EXPECT_EQ(AwaError_Success, AwaStaticClient_ResourceChanged(client_, data.ObjectID, data.ObjectInstanceID, data.ResourceID));
        }
        else
        {
            // received changed value
            cbHandler_->complete = true;
        }
        notificationCount_++;
    };

protected:
    int notificationCount_ = 0;
    //void * integerResourceValue_ = 0;
    //int expectedValue_ = 12345;
    callback1 * cbHandler_;
    uint8_t opaque_[64];
};

static void (ChangeCallbackRunner)(const AwaChangeSet * changeSet, void * context)
{
    if (context)
    {
        auto * that = static_cast<TestStaticClientObserveValue*>(context);
        that->callbackHandler(changeSet);
    }
}

TEST_P(TestStaticClientObserveValue, TestObserveValueSingle)
{
    TestObserveStaticResource data = GetParam();

    ASSERT_TRUE(sizeof(opaque_) >= data.ValueSize);
    memcpy(opaque_, data.Value, data.ValueSize);
    memset(opaque_, 0, sizeof(opaque_));

    AwaServerObserveOperation * observeOperation = AwaServerObserveOperation_New(session_);

    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetApplicationContext(client_, &cbHandler_));
    EXPECT_EQ(AwaError_Success,AwaStaticClient_DefineObject(client_, observeDetail::TEST_OBJECT_NON_ARRAY_TYPES, "TestObject", 0, 1));

    switch(data.Type)
    {
        case AwaResourceType_Opaque:
        {
          EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResource(client_, data.ObjectID, data.ResourceID, "Test Resource", data.Type, 1, 1, AwaResourceOperations_ReadOnly));
            EXPECT_EQ(AwaError_Success, AwaStaticClient_SetResourceStorageWithPointer(client_, data.ObjectID, data.ResourceID, opaque_, sizeof(opaque_), 0));
            break;
        }
        default:
          EXPECT_EQ(AwaError_Success, AwaStaticClient_DefineResource(client_, data.ObjectID, data.ResourceID, "Test Resource", data.Type, 1, 1, AwaResourceOperations_ReadOnly));
            EXPECT_EQ(AwaError_Success, AwaStaticClient_SetResourceStorageWithPointer(client_, data.ObjectID, data.ResourceID, data.Value, data.ValueSize, 0));
            break;
    }
    EXPECT_EQ(AwaError_Success, AwaStaticClient_CreateObjectInstance(client_, observeDetail::TEST_OBJECT_NON_ARRAY_TYPES, 0));

    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_TRUE(NULL != operation);
    SingleStaticClientPollCondition condition(client_, operation, global::clientEndpointName, 10);
    EXPECT_TRUE(condition.Wait());
    AwaServerListClientsOperation_Free(&operation);

    AwaServerDefineOperation * defineOperation = AwaServerDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);
    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(data.ObjectID, "TestObject", 0, 1);
    EXPECT_TRUE(objectDefinition != NULL);

    switch (data.Type)
    {
        case AwaResourceType_String:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, data.ResourceID, "Test Resource", true, AwaResourceOperations_ReadOnly, (const char *)data.Value));
            break;
        case AwaResourceType_Integer:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefinition, data.ResourceID, "Test Resource", true, AwaResourceOperations_ReadOnly, *((AwaInteger *)data.Value)));
            break;
        case AwaResourceType_Float:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloat(objectDefinition, data.ResourceID, "Test Resource", true, AwaResourceOperations_ReadOnly, *((AwaFloat *)data.Value)));
            break;
        case AwaResourceType_Boolean:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBoolean(objectDefinition, data.ResourceID, "Test Resource", true, AwaResourceOperations_ReadOnly, *((AwaBoolean *)data.Value)));
            break;
        case AwaResourceType_Opaque:
        {
            AwaOpaque opaque = {data.Value, data.ValueSize};
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque(objectDefinition, data.ResourceID, "Test Resource", true, AwaResourceOperations_ReadOnly, opaque));
            break;
        }
        case AwaResourceType_Time:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTime(objectDefinition, data.ResourceID, "Test Resource", true, AwaResourceOperations_ReadOnly, *((AwaTime *)data.Value)));
            break;
        case AwaResourceType_ObjectLink:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLink(objectDefinition, data.ResourceID, "Test Resource", true, AwaResourceOperations_ReadOnly, *((AwaObjectLink *)data.Value)));
            break;
        default:
            ASSERT_TRUE(false);
    }
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(defineOperation, objectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(defineOperation, defaults::timeout));
    AwaServerDefineOperation_Free(&defineOperation);
    AwaObjectDefinition_Free(&objectDefinition);

    char path[64];
    AwaAPI_MakeResourcePath(path, sizeof(path), data.ObjectID, data.ObjectInstanceID, data.ResourceID);
    AwaServerObservation * observation = AwaServerObservation_New(global::clientEndpointName, path, ChangeCallbackRunner, this);

    EXPECT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(observeOperation, observation));

    Lwm2m_Debug("Performing Observe Operation\n");

    pthread_t t;
    ServerObserveThreadContext serverObserveContext_;
    memset(&serverObserveContext_, 0, sizeof(serverObserveContext_));
    serverObserveContext_.ObserveOperation = observeOperation;
    serverObserveContext_.ObserveThreadAlive = &cbHandler_->observeThreadAlive_;
    pthread_create(&t, NULL, do_observe_operation, (void *)&serverObserveContext_);

    ASSERT_TRUE(cbHandler_->Wait());

    pthread_join(t, NULL);

    EXPECT_EQ(notificationCount_, 2);

    EXPECT_EQ(AwaError_Success, AwaServerObservation_Free(&observation));
    EXPECT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&observeOperation));
}

INSTANTIATE_TEST_CASE_P(
        TestStaticClientObserveValue,
        TestStaticClientObserveValue,
        ::testing::Values(
        TestObserveStaticResource {observeDetail::TEST_OBJECT_NON_ARRAY_TYPES, 0, observeDetail::TEST_RESOURCE_STRING, (void *)observeDetail::dummyInitialString1, (void *)observeDetail::dummyExpectedString1, strlen(observeDetail::dummyInitialString1) + 1,     AwaResourceType_String},
        TestObserveStaticResource {observeDetail::TEST_OBJECT_NON_ARRAY_TYPES, 0, observeDetail::TEST_RESOURCE_INTEGER,    &observeDetail::dummyInitialInteger1, &observeDetail::dummyExpectedInteger1, sizeof(observeDetail::dummyInitialInteger1),    AwaResourceType_Integer},
        TestObserveStaticResource {observeDetail::TEST_OBJECT_NON_ARRAY_TYPES, 0, observeDetail::TEST_RESOURCE_FLOAT,      &observeDetail::dummyInitialFloat1, &observeDetail::dummyExpectedFloat1,       sizeof(observeDetail::dummyInitialFloat1),      AwaResourceType_Float},
        TestObserveStaticResource {observeDetail::TEST_OBJECT_NON_ARRAY_TYPES, 0, observeDetail::TEST_RESOURCE_BOOLEAN,    &observeDetail::dummyInitialBoolean1, &observeDetail::dummyExpectedBoolean1,     sizeof(observeDetail::dummyInitialBoolean1),    AwaResourceType_Boolean},
        TestObserveStaticResource {observeDetail::TEST_OBJECT_NON_ARRAY_TYPES, 0, observeDetail::TEST_RESOURCE_OPAQUE,     observeDetail::dummyInitialOpaqueData, observeDetail::dummyExpectedOpaqueData,    sizeof(observeDetail::dummyInitialOpaqueData),  AwaResourceType_Opaque},
        TestObserveStaticResource {observeDetail::TEST_OBJECT_NON_ARRAY_TYPES, 0, observeDetail::TEST_RESOURCE_TIME,       &observeDetail::dummyInitialTime1, &observeDetail::dummyExpectedTime1,       sizeof(observeDetail::dummyInitialTime1),       AwaResourceType_Time},
        TestObserveStaticResource {observeDetail::TEST_OBJECT_NON_ARRAY_TYPES, 0, observeDetail::TEST_RESOURCE_OBJECTLINK, &observeDetail::dummyInitialObjectLink1, &observeDetail::dummyExpectedObjectLink1, sizeof(observeDetail::dummyInitialObjectLink1), AwaResourceType_ObjectLink}
        ));

} // namespace Awa

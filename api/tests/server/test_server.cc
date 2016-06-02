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

#include <awa/server.h>
#include <awa/common.h>
#include "log.h"
#include "get_response.h"
#include "support/support.h"
#include "arrays.h"

namespace Awa {

class TestServer : public TestServerAndClientWithConnectedSession {};
class TestServerWithDummyObjects : public TestServerAndClientWithConnectedSessionWithDummyObjects {};


TEST_F(TestServer, AwaServer_read_default_value_from_created_custom_object_instance_integer_resource)
{
    //Define our custom object with a mandatory integer resource
    AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(client_session_);
    AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(server_session_);
    ASSERT_TRUE(clientDefineOperation != NULL);
    ASSERT_TRUE(serverDefineOperation != NULL);

    AwaInteger expected = 123456787;
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(customObjectDefinition, 0, "Test Resource", true, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, global::timeout));
    ASSERT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, global::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&clientDefineOperation);
    AwaServerDefineOperation_Free(&serverDefineOperation);


    //Create a basic set operation to create our custom objects
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(client_session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, global::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/10000/0/0"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));
    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != readResponse);
    const AwaInteger * value = NULL;
    ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsIntegerPointer(readResponse, "/10000/0/0", &value));
    ASSERT_TRUE(NULL != value);
    EXPECT_EQ(expected, *value);
    AwaServerReadOperation_Free(&readOperation);
}

TEST_F(TestServer, AwaServer_read_default_value_from_created_custom_object_instance_integer_array_resource)
{
    //Define our custom object with a mandatory integer resource
    AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(client_session_);
    AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(server_session_);
    ASSERT_TRUE(clientDefineOperation != NULL);
    ASSERT_TRUE(serverDefineOperation != NULL);

    AwaIntegerArray * expected = AwaIntegerArray_New();
    AwaIntegerArray_SetValue(expected, 0u, 123456787);
    AwaIntegerArray_SetValue(expected, 1u, 987654321);
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsIntegerArray(customObjectDefinition, 0, "Test Resource", 2, 10, AwaResourceOperations_ReadWrite, expected));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, global::timeout));
    ASSERT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, global::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&clientDefineOperation);
    AwaServerDefineOperation_Free(&serverDefineOperation);


    //Create a basic set operation to create our custom objects
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(client_session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, global::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/10000/0/0"));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));
    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != readResponse);
    const AwaIntegerArray * value = NULL;
    ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValuesAsIntegerArrayPointer(readResponse, "/10000/0/0", &value));
    ASSERT_TRUE(NULL != value);
    EXPECT_EQ(0, Array_Compare((AwaArray *)expected, (AwaArray *)value, AwaResourceType_IntegerArray));

    AwaIntegerArray_Free(&expected);
    AwaServerReadOperation_Free(&readOperation);
}



namespace writeReadDetail
{
    struct TestWriteReadResource
    {
        const char * path;
        const void * value;
        AwaResourceType type;
    };

    ::std::ostream& operator<<(::std::ostream& os, const TestWriteReadResource& item)
    {
      return os << "Item: path " << item.path
                << ", value " << item.value
                << ", type" << item.type;
    }

    AwaInteger dummyInteger1 = 123456;
    const char * dummyString1 = "Lightweight M2M Client";
    AwaFloat dummyFloat1 = 1.23;
    AwaTime dummyTime1 = 0xA20AD72B;
    AwaBoolean dummyBoolean1 = true;

    static char dummyOpaqueData[] = {'a',0,'x','\0', 123};
    AwaOpaque dummyOpaque1 = {(void*) dummyOpaqueData, sizeof(dummyOpaqueData)};
    AwaObjectLink dummyObjectLink1 = { 2, 5 };
}

class TestWriteReadValueSingle : public TestServerWithDummyObjects, public ::testing::WithParamInterface< writeReadDetail::TestWriteReadResource> {};

TEST_P(TestWriteReadValueSingle, TestWriteReadValueSingle)
{
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Update);
    EXPECT_TRUE(NULL != writeOperation);
    writeReadDetail::TestWriteReadResource data = GetParam();

    switch(data.type)
    {
        case AwaResourceType_String:
            ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsCString(writeOperation, data.path, (const char *)data.value));
            break;
        case AwaResourceType_Integer:
            ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, data.path, *((AwaInteger*)data.value)));
            break;
        case AwaResourceType_Float:
            ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsFloat(writeOperation, data.path, *((AwaFloat*)data.value)));
            break;
        case AwaResourceType_Boolean:
            ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsBoolean(writeOperation, data.path, *((AwaBoolean*)data.value)));
            break;
        case AwaResourceType_Opaque:
            ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsOpaque(writeOperation, data.path, *((AwaOpaque*)data.value)));
            break;
        case AwaResourceType_Time:
            ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsTime(writeOperation, data.path, *((AwaTime*)data.value)));
            break;
        case AwaResourceType_ObjectLink:
            ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsObjectLink(writeOperation, data.path, *((AwaObjectLink*)data.value)));
            break;
        default:
            ASSERT_TRUE(false);
            break;
    }

    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, global::timeout));

    AwaServerWriteOperation_Free(&writeOperation);
    EXPECT_TRUE(NULL == writeOperation);

    // TODO: read to ensure the value was set

    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, data.path));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));
    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
    ASSERT_TRUE(NULL != readResponse);

    switch(data.type)
    {
        case AwaResourceType_String:
            {
                const char * value = NULL;
                ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsCStringPointer(readResponse, data.path, &value));
                ASSERT_TRUE(NULL != value);
                ASSERT_STREQ((const char*)data.value, value);
            }
            break;
        case AwaResourceType_Integer:
            {
                const AwaInteger * value = NULL;
                ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsIntegerPointer(readResponse, data.path, &value));
                ASSERT_TRUE(NULL != value);
                ASSERT_EQ(*((AwaInteger*)data.value), *value);
            }
            break;
        case AwaResourceType_Float:
            {
                const AwaFloat * value = NULL;
                ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsFloatPointer(readResponse, data.path, &value));
                ASSERT_TRUE(NULL != value);
                ASSERT_EQ(*((AwaFloat*)data.value), *value);
            }
            break;
        case AwaResourceType_Boolean:
        {
            const AwaBoolean * value = NULL;
            ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsBooleanPointer(readResponse, data.path, &value));
            ASSERT_TRUE(NULL != value);
            ASSERT_EQ(*((AwaBoolean*)data.value) == true? 1 : 0, *value == true? 1 : 0);
        }
            break;
        case AwaResourceType_Opaque:
            {
                AwaOpaque value;
                ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsOpaque(readResponse, data.path, &value));
                ASSERT_EQ(((AwaOpaque*)data.value)->Size, value.Size);
                ASSERT_EQ(0, memcmp(((AwaOpaque*)data.value)->Data, value.Data, value.Size));
            }
            break;
        case AwaResourceType_Time:
            {
                const AwaTime * value = NULL;
                ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsTimePointer(readResponse, data.path, &value));
                ASSERT_TRUE(NULL != value);
                ASSERT_EQ(*((AwaTime*)data.value), *value);
            }
            break;
        case AwaResourceType_ObjectLink:
            {
                AwaObjectLink value;
                ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsObjectLink(readResponse, data.path, &value));
                ASSERT_EQ(((AwaObjectLink*)data.value)->ObjectID, value.ObjectID);
                ASSERT_EQ(((AwaObjectLink*)data.value)->ObjectInstanceID, value.ObjectInstanceID);
            }
            break;
        default:
            ASSERT_TRUE(false);
            break;
    }

    AwaServerReadOperation_Free(&readOperation);
}

INSTANTIATE_TEST_CASE_P(
        TestWriteReadValueSingle,
        TestWriteReadValueSingle,
        ::testing::Values(
            writeReadDetail::TestWriteReadResource {"/10000/0/1", writeReadDetail::dummyString1, AwaResourceType_String},
            writeReadDetail::TestWriteReadResource {"/10000/0/2", &writeReadDetail::dummyInteger1, AwaResourceType_Integer},
            writeReadDetail::TestWriteReadResource {"/10000/0/3", &writeReadDetail::dummyFloat1, AwaResourceType_Float},
            writeReadDetail::TestWriteReadResource {"/10000/0/4", &writeReadDetail::dummyBoolean1, AwaResourceType_Boolean},
            writeReadDetail::TestWriteReadResource {"/10000/0/5", &writeReadDetail::dummyOpaque1, AwaResourceType_Opaque},
            writeReadDetail::TestWriteReadResource {"/10000/0/6", &writeReadDetail::dummyTime1, AwaResourceType_Time},
            writeReadDetail::TestWriteReadResource {"/10000/0/7", &writeReadDetail::dummyObjectLink1, AwaResourceType_ObjectLink}
        ));



TEST_F(TestServer, AwaServer_read_delete_read)
{
    //first read should succeed, but the second should fail "Not Found" after the delete

    {
        AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
        ASSERT_TRUE(NULL != readOperation);
        ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/4"));
        ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, global::timeout));

        AwaServerReadOperation_Free(&readOperation);
    }

    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(server_session_);
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(operation, global::clientEndpointName, "/4/0"));
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_Perform(operation, global::timeout));

    AwaServerDeleteOperation_Free(&operation);

    {
        AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
        ASSERT_TRUE(NULL != readOperation);
        ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/4"));
        ASSERT_EQ(AwaError_Response, AwaServerReadOperation_Perform(readOperation, global::timeout));
        const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation, global::clientEndpointName);
        ASSERT_TRUE(NULL != readResponse);
        const AwaPathResult * result = AwaServerReadResponse_GetPathResult(readResponse, "/4");
        EXPECT_EQ(AwaError_LWM2MError, AwaPathResult_GetError(result));
        EXPECT_EQ(AwaLWM2MError_NotFound, AwaPathResult_GetLWM2MError(result));
        AwaServerReadOperation_Free(&readOperation);
    }
}



//TODO: Observation with write attributes set tests

//TODO: Interaction between functions: Observe, Read, Write, Define, Delete, Discover

//Delete write
//Delete read
//Observe set
//Observe delete
//Define Discover
//Define write read
//etc.

} // namespace Awa

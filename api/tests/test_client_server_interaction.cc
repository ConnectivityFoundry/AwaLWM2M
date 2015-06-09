#include <gtest/gtest.h>

#include <lwm2m_tree_node.h>

#include <awa/client.h>
#include <awa/common.h>
#include "log.h"
#include "get_response.h"
#include "support/support.h"

namespace Awa {

class TestClientServerInteraction : public TestServerAndClientWithConnectedSessionWithDummyObjects {};

TEST_F(TestClientServerInteraction, write_get_valid_integer_resource)
{
    const char * path = "/10000/0/2";
    AwaInteger expectedValue = 123456789;

    // Do Server WRITE
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, path, expectedValue));
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, defaults::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    // Do Client GET, ensure value has been written.
    AwaClientGetOperation * readOperation = AwaClientGetOperation_New(client_session_);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(readOperation, path));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(readOperation, defaults::timeout));
    const AwaClientGetResponse * readResponse = AwaClientGetOperation_GetResponse(readOperation); ASSERT_TRUE(NULL != readResponse);
    ASSERT_TRUE(AwaClientGetResponse_HasValue(readResponse, path));
    const AwaInteger * getResponseValue = NULL;
    ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsIntegerPointer(readResponse, path, &getResponseValue));
    ASSERT_EQ(expectedValue, *getResponseValue);
    AwaClientGetOperation_Free(&readOperation);
}

TEST_F(TestClientServerInteraction, DISABLED_set_read_valid_integer_resource)
{
    const char * path = "/10000/0/2";
    AwaInteger expectedValue = 123456789;

    // Do Client SET
    AwaClientSetOperation * writeOperation = AwaClientSetOperation_New(client_session_); ASSERT_TRUE(NULL != writeOperation);
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsInteger(writeOperation, path, expectedValue));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(writeOperation, defaults::timeout));
    AwaClientSetOperation_Free(&writeOperation);

    // Do Server READ, ensure value has been written.
    /*AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, defaults::clientEndpointName, path));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, defaults::timeout));
    const AwaServerReadResponse * readResponse = AwaServerReadOperation_ReadResponse(readOperation); ASSERT_TRUE(NULL != readResponse);
    ASSERT_TRUE(AwaServerReadResponse_HasValue(readResponse, path));
    const AwaInteger * getResponseValue = NULL;
    ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsIntegerPointer(readResponse, path, &getResponseValue));
    ASSERT_EQ(expectedValue, *getResponseValue);
    AwaServerReadOperation_Free(&readOperation);*/
    ASSERT_TRUE(false);
}



TEST_F(TestClientServerInteraction, DISABLED_client_delete_server_read)
{
    // First, write a value to the optional resource instance, so it is created.

    const char * path = "/10000/0/102";
    AwaInteger expected = 123456789;

    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(server_session_, AwaWriteMode_Update); ASSERT_TRUE(NULL != writeOperation);

    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, path, expected));
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    // Read it to ensure it is actually readable

    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, path));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, defaults::timeout));
    /*const AwaServerReadResponse * readResponse = AwaServerReadOperation_ReadResponse(readOperation);
    ASSERT_TRUE(NULL != readResponse);
    const AwaInteger * value = NULL;
    ASSERT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsIntegerPointer(readResponse, path, &value));
    ASSERT_TRUE(NULL != value);
    ASSERT_EQ(expected, *value);
    AwaServerReadOperation_Free(&readOperation);
    EXPECT_TRUE(NULL == readOperation);*/
    ASSERT_TRUE(false);

    // Delete the resource
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(client_session_); ASSERT_TRUE(NULL != operation);
    ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_AddPath(operation, path));
    ASSERT_EQ(AwaError_Success, AwaClientDeleteOperation_Perform(operation, defaults::timeout));
    AwaClientDeleteOperation_Free(&operation);


    // Read after delete should fail.

    readOperation = AwaServerReadOperation_New(server_session_);
    ASSERT_TRUE(NULL != readOperation);
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, path));
    ASSERT_EQ(AwaError_Success, AwaServerReadOperation_Perform(readOperation, defaults::timeout));
    /*const AwaServerReadResponse * readResponse = AwaServerReadOperation_ReadResponse(readOperation);
    ASSERT_TRUE(NULL != readResponse);

    const AwaPathResult * result = NULL;
    AwaServerReadResponse_GetPathResult(readResponse, path, &result);
    ASSERT_EQ(AwaError_LWM2MError, AwaPathResult_GetError(result));
    ASSERT_EQ(AwaLWM2MError_BadRequest, AwaPathResult_GetLWM2MError(result));

    const AwaInteger * value = NULL;
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerReadResponse_GetValueAsIntegerPointer(readResponse, path, &value));
    ASSERT_TRUE(NULL == value);
    AwaServerReadOperation_Free(&readOperation);*/
    ASSERT_TRUE(false);

}

//TODO: Interaction between functions: Observe, Subscribe, Set, Read, Get, Write, Define, Delete, Discover, Explore.

//Server-Delete set
//Client-Delete write
//Server-Delete get
//Client-Delete read
//Observe set
//Subscribe write
//Observe delete
//Define Discover
//Define write read
//etc.

} // namespace Awa

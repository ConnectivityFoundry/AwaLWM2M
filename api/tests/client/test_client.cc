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

#include <awa/client.h>
#include <awa/common.h>
#include "log.h"
#include "get_response.h"
#include "support/support.h"
#include "support/definition.h"
#include "arrays.h"

namespace Awa {

class TestClient : public TestClientBase {};

class TestClientWithDaemon : public TestClientWithConnectedSession {};

TEST_F(TestClientWithDaemon, AwaClient_set_get_valid_integer_resource)
{
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    AwaInteger expected = 123456789;
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsInteger(setOperation, "/3/0/9", expected));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, global::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/9"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, global::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    const AwaInteger * value = NULL;
    ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsIntegerPointer(getResponse, "/3/0/9", &value));
    ASSERT_TRUE(NULL != value);
    ASSERT_EQ(expected, *value);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestClientWithDaemon, AwaClient_set_get_valid_string_resource)
{
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    const char * expected = "abcdefgh";
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsCString(setOperation, "/3/0/1", expected));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, global::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, global::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    const char * value = NULL;
    ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsCStringPointer(getResponse, "/3/0/1", &value));
    ASSERT_TRUE(NULL != value);
    ASSERT_STREQ(expected, value);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestClientWithDaemon, AwaClient_get_set_multiple_valid_resources)
{
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    AwaInteger expectedInteger = 123456789;
    const char * expectedString = "abcdefgh";
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsInteger(setOperation, "/3/0/9", expectedInteger));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsCString(setOperation, "/3/0/1", expectedString));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, global::timeout));
    AwaClientSetOperation_Free(&setOperation);

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/9"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3/0/1"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, global::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    const AwaInteger * integerValue = NULL;
    const char * stringValue = NULL;
    ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsIntegerPointer(getResponse, "/3/0/9", &integerValue));
    ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsCStringPointer(getResponse, "/3/0/1", &stringValue));
    ASSERT_TRUE(NULL != integerValue);
    ASSERT_TRUE(NULL != stringValue);
    ASSERT_EQ(expectedInteger, *integerValue);
    ASSERT_STREQ(expectedString, stringValue);
    AwaClientGetOperation_Free(&getOperation);
}


TEST_F(TestClientWithDaemon, AwaClient_get_resource_value_from_created_custom_object_instance)
{
    //Define our custom object
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    AwaInteger expected = 123456787;
    int customObjectID = 10000;

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(customObjectID, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(customObjectDefinition, 0, "Test Resource", false, AwaResourceOperations_ReadWrite, 0));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, global::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    //Create a basic set operation to create our custom objects
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10000/0/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsInteger(setOperation, "/10000/0/0", expected));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, global::timeout));
    AwaClientSetOperation_Free(&setOperation);


    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/10000/0/0"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, global::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    const AwaInteger * value = NULL;
    ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsIntegerPointer(getResponse, "/10000/0/0", &value));
    ASSERT_TRUE(NULL != value);
    ASSERT_EQ(expected, *value);
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestClientWithDaemon, test_object_0_is_defined_regression)
{
    // Test for regression, where object 0 was never listed as defined.
    ASSERT_TRUE(AwaClientSession_IsObjectDefined(session_, 0));
}


/***********************************************************************************************************
 * Complex GET tests
 */

TEST_F(TestClientWithDaemon, get_on_undefined_object_should_return_path_not_found)
{
    // Test a get on an undefined object.
    ASSERT_FALSE(AwaClientSession_IsObjectDefined(session_, 1000));

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000"));
    EXPECT_EQ(AwaError_Response, AwaClientGetOperation_Perform(getOperation, global::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    const AwaPathResult * result = AwaClientGetResponse_GetPathResult(getResponse, "/1000");
    ASSERT_TRUE(NULL != result);
    EXPECT_EQ(AwaError_PathNotFound, AwaPathResult_GetError(result));

    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestClientWithDaemon, get_on_defined_and_undefined_objects_should_return_appropriate_response)
{
    // Test a get on a mix of defined and undefined objects.
    // Should return a response that contains both objects,
    // with AwaError_Success on the defined object and AwaError_PathNotFound on the undefined object.

    // 1000 is not defined

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/3"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000"));
    ASSERT_EQ(AwaError_Response, AwaClientGetOperation_Perform(getOperation, global::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);

    const AwaPathResult * undefinedResult = AwaClientGetResponse_GetPathResult(getResponse, "/1000");
    ASSERT_TRUE(NULL != undefinedResult);
    EXPECT_EQ(AwaError_PathNotFound, AwaPathResult_GetError(undefinedResult));

    const AwaPathResult * definedResult = AwaClientGetResponse_GetPathResult(getResponse, "/3");
    ASSERT_TRUE(NULL != definedResult);
    EXPECT_EQ(AwaError_Success, AwaPathResult_GetError(definedResult));

    EXPECT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/3"));
    EXPECT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/3/0"));
    EXPECT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/1000"));
    EXPECT_FALSE(AwaClientGetResponse_ContainsPath(getResponse, "/1000/0"));
    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestClientWithDaemon, get_on_empty_object_instance_has_correct_paths_in_response)
{
    // Test a get on a valid object with a new object instance, but no mandatory resources or set resources.
    // Should return a response that contains the object instance, but no resource paths.
    AwaError result = AwaError_Unspecified;

    // check /1000 is not defined on the daemon
    {
        AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
        ASSERT_TRUE(NULL != getOperation);
        EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000"));
        EXPECT_EQ(AwaError_Response, AwaClientGetOperation_Perform(getOperation, global::timeout));
        const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
        ASSERT_TRUE(NULL != getResponse);
        const AwaPathResult * result = AwaClientGetResponse_GetPathResult(getResponse, "/1000");
        ASSERT_TRUE(NULL != result);
        EXPECT_EQ(AwaError_PathNotFound, AwaPathResult_GetError(result));
        AwaClientGetOperation_Free(&getOperation);
    }

    // define a test object 1000, with single integer resource 0
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);                  ASSERT_TRUE(defineOperation != NULL);
    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(1000, "Test Object 0", 0, 1);  ASSERT_TRUE(customObjectDefinition != NULL);
    result = AwaObjectDefinition_AddResourceDefinitionAsInteger(customObjectDefinition, 0, "Resource0", false, AwaResourceOperations_ReadWrite, 0L); ASSERT_EQ(AwaError_Success, result);
    result = AwaClientDefineOperation_Add(defineOperation, customObjectDefinition);                           ASSERT_EQ(AwaError_Success, result);
    result = AwaClientDefineOperation_Perform(defineOperation, global::timeout);                            ASSERT_EQ(AwaError_Success, result);
    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    // create instance 0
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);                           ASSERT_TRUE(NULL != setOperation);
    result = AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0");                             ASSERT_EQ(AwaError_Success, result);
    result = AwaClientSetOperation_Perform(setOperation, global::timeout);                                  ASSERT_EQ(AwaError_Success, result);
    AwaClientSetOperation_Free(&setOperation);



    // /1000 and /1000/0 should be in the response, but /1000/0/0 should not be
    {
        AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
        ASSERT_TRUE(NULL != getOperation);
        EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000"));
        EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, global::timeout));
        const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
        ASSERT_TRUE(NULL != getResponse);
        ASSERT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/1000"));
        ASSERT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/1000/0"));
        ASSERT_FALSE(AwaClientGetResponse_ContainsPath(getResponse, "/1000/0/0"));
        AwaClientGetOperation_Free(&getOperation);
    }
}

TEST_F(TestClientWithDaemon, get_on_defined_and_undefined_resources_should_return_appropriate_response)
{
    // Test a get on a mix of defined and undefined resources.
    // Should return a response that contains all requested paths, but errors on the undefined resources.
    // The response should contain object and object instance for both.
    AwaError result = AwaError_Unspecified;

    // /2/0/1000 is not defined

    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);                           ASSERT_TRUE(NULL != getOperation);
    result = AwaClientGetOperation_AddPath(getOperation, "/3/0/1");                                           ASSERT_EQ(AwaError_Success, result);
    result = AwaClientGetOperation_AddPath(getOperation, "/2/0/1000");                                        ASSERT_EQ(AwaError_Success, result);
    result = AwaClientGetOperation_Perform(getOperation, global::timeout);                                  ASSERT_EQ(AwaError_Response, result);
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);           ASSERT_TRUE(NULL != getResponse);
    ASSERT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/3"));
    ASSERT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/3/0"));
    ASSERT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/3/0/1"));
    ASSERT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/2"));
    ASSERT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/2/0"));
    ASSERT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/2/0/1000"));

    const AwaPathResult * undefinedResult = AwaClientGetResponse_GetPathResult(getResponse, "/2/0/1000");
    ASSERT_TRUE(NULL != undefinedResult);
    EXPECT_EQ(AwaError_PathNotFound, AwaPathResult_GetError(undefinedResult));

    const AwaPathResult * definedResult = AwaClientGetResponse_GetPathResult(getResponse, "/3/0/1");
    ASSERT_TRUE(NULL != definedResult);
    EXPECT_EQ(AwaError_Success, AwaPathResult_GetError(definedResult));

    AwaClientGetOperation_Free(&getOperation);
}

TEST_F(TestClientWithDaemon, object_with_single_optional_resource_can_be_created)
{
    // define an object with a single optional resource
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);                  ASSERT_TRUE(defineOperation != NULL);
    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(1000, "Test Object 0", 0, 1);  ASSERT_TRUE(customObjectDefinition != NULL);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(customObjectDefinition, 0, "Resource0", false, AwaResourceOperations_ReadWrite, 42L));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, global::timeout));
    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    // create an instance of this object
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, global::timeout));
    AwaClientSetOperation_Free(&setOperation);

    // a get should succeed but have no resource paths (optional resource not created)
    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_); ASSERT_TRUE(NULL != getOperation);
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, global::timeout));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation); ASSERT_TRUE(NULL != getResponse);

    // /1000 and /1000/0 should be in the response, but /1000/0/0 should not be
    EXPECT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/1000"));
    EXPECT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/1000/0"));
    EXPECT_FALSE(AwaClientGetResponse_ContainsPath(getResponse, "/1000/0/0"));
    EXPECT_FALSE(AwaClientGetResponse_HasValue(getResponse, "/1000/0/0"));
    AwaClientGetOperation_Free(&getOperation);

    // create the optional resource
    setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/1000/0/0"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, global::timeout));
    AwaClientSetOperation_Free(&setOperation);

    // /1000/0/0 should be in the response, with a (default) value
    getOperation = AwaClientGetOperation_New(session_); ASSERT_TRUE(NULL != getOperation);
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/1000/0"));
    EXPECT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, global::timeout));
    getResponse = AwaClientGetOperation_GetResponse(getOperation); ASSERT_TRUE(NULL != getResponse);
    EXPECT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/1000"));
    EXPECT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/1000/0"));
    EXPECT_TRUE(AwaClientGetResponse_ContainsPath(getResponse, "/1000/0/0"));
    EXPECT_TRUE(AwaClientGetResponse_HasValue(getResponse, "/1000/0/0"));

    //TODO: check value matches default

    AwaClientGetOperation_Free(&getOperation);
}





//TODO: Interaction between functions: Subscribe, Set, Get, Define, Delete, Explore

//Delete set
//Delete get
//Subscribe set
//Subscribe delete
//Define explore
//Define set get
//etc.


} // namespace Awa

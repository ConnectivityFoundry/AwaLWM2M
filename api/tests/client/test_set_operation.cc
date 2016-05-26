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
#include <vector>
#include <string>
#include <algorithm>

#include "../src/objects_tree.h"
#include "../src/arrays.h"
#include "support/support.h"
#include "support/xml_support.h"

#include "awa/client.h"
#include "set_operation.h"
#include "log.h"
#include "set_write_common.h"

namespace Awa {

class TestSetOperation : public TestClientBase {};

class TestSetOperationWithConnectedSession : public TestClientWithConnectedSession {};

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_New_returns_valid_operation_and_free_works)
{
    // test that AwaClientSetOperation_Free works via valgrind
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Free(&setOperation));
}

TEST_F(TestSetOperation, AwaClientSetOperation_New_handles_null_session)
{
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(NULL);
    ASSERT_EQ(NULL, setOperation);
}

TEST_F(TestSetOperation, AwaClientSetOperation_New_handles_invalid_session)
{
    // An invalid session is one that is not connected
    AwaClientSession * session = AwaClientSession_New();
    EXPECT_EQ(NULL, AwaClientSetOperation_New(session));

    // set up IPC - still not enough
    AwaClientSession_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort);
    EXPECT_EQ(NULL, AwaClientSetOperation_New(session));

    AwaClientSession_Free(&session);
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_Free_nulls_pointer)
{
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Free(&setOperation));
    ASSERT_EQ(NULL, setOperation);
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_Free_handles_null)
{
    EXPECT_EQ(AwaError_OperationInvalid, AwaClientSetOperation_Free(NULL));
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_Free_handles_null_pointer)
{
    AwaClientSetOperation * setOperation = NULL;
    EXPECT_EQ(AwaError_OperationInvalid, AwaClientSetOperation_Free(&setOperation));
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_Free_handles_invalid_session)
{
    // Session is freed before the operation (use a local session):
    AwaClientSession * session = AwaClientSession_New();
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(session));
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session);
    AwaClientSession_Free(&session);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Free(&setOperation));
    // expect no crash or memory leaks
}

/***********************************************************************************************************
 * ClientSetOperation_ AddCreate tag tests
 */

TEST_F(TestSetOperation, ClientSetOperation_AddCreate_handles_null_node)
{
    EXPECT_EQ(InternalError_ParameterInvalid, ClientSetOperation_AddCreate(NULL));
}

TEST_F(TestSetOperation, ClientSetOperation_AddCreate_handles_invalid_node)
{
    //input must be either an object or object instance node
    TreeNode objectsNode = ObjectsTree_New();
    EXPECT_EQ(InternalError_ParameterInvalid, ClientSetOperation_AddCreate(objectsNode));
    ObjectsTree_Free(objectsNode);
}

TEST_F(TestSetOperation, ClientSetOperation_AddCreate_handles_valid_object_node)
{
    const char * expectedXML =
            "<Objects>"
            "  <Object>"
            "    <ID>3</ID>"
            "    <ObjectInstance>"
            "      <Create/>"
            "    </ObjectInstance>"
            "  </Object>"
            "</Objects>";

    TreeNode objectsNode = ObjectsTree_New();
    TreeNode objectNode;
    ASSERT_EQ(InternalError_Success, ObjectsTree_AddPath(objectsNode, "/3", &objectNode));
    EXPECT_EQ(InternalError_Success, ClientSetOperation_AddCreate(objectNode));
    EXPECT_STREQ(XmlExpected(expectedXML).get(), XmlSerialise(objectsNode).get());
    ObjectsTree_Free(objectsNode);
}

TEST_F(TestSetOperation, ClientSetOperation_AddCreate_handles_valid_object_instance_node)
{
    const char * expectedXML =
                "<Objects>"
                "  <Object>"
                "    <ID>3</ID>"
                "    <ObjectInstance>"
                "      <ID>0</ID>"
                "      <Create/>"
                "    </ObjectInstance>"
                "  </Object>"
                "</Objects>";

    TreeNode objectsNode = ObjectsTree_New();
    TreeNode objectInstanceNode;
    ASSERT_EQ(InternalError_Success, ObjectsTree_AddPath(objectsNode, "/3/0", &objectInstanceNode));
    EXPECT_EQ(InternalError_Success, ClientSetOperation_AddCreate(objectInstanceNode));
    EXPECT_STREQ(XmlExpected(expectedXML).get(), XmlSerialise(objectsNode).get());
    ObjectsTree_Free(objectsNode);
}

TEST_F(TestSetOperation, ClientSetOperation_AddCreate_works_with_AddValue)
{
    const char * expectedXML =
                "<Objects>"
                "  <Object>"
                "    <ID>3</ID>"
                "    <ObjectInstance>"
                "      <ID>0</ID>"
                "      <Create/>"
                "      <Resource>"
                "        <ID>1</ID>"
                "        <ResourceInstance>"
                "          <ID>0</ID>"
                "          <Value>Lightweight M2M Client</Value>"
                "         </ResourceInstance>"
                "       </Resource>"
                "    </ObjectInstance>"
                "  </Object>"
                "</Objects>";

    TreeNode objectsNode = ObjectsTree_New();
    TreeNode objectInstanceNode;
    TreeNode resourceNode;
    ASSERT_EQ(InternalError_Success, ObjectsTree_AddPath(objectsNode, "/3/0", &objectInstanceNode));
    EXPECT_EQ(InternalError_Success, ClientSetOperation_AddCreate(objectInstanceNode));
    ASSERT_EQ(InternalError_Success, ObjectsTree_AddPath(objectsNode, "/3/0/1", &resourceNode));
    EXPECT_EQ(InternalError_Success, SetWriteCommon_AddValueToResourceNode(resourceNode, 0, "Lightweight M2M Client"));
    EXPECT_STREQ(XmlExpected(expectedXML).get(), XmlSerialise(objectsNode).get());
    ObjectsTree_Free(objectsNode);
}

/***********************************************************************************************************
 * AwaClientSetOperation_AddValueAsInteger tests
 */

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_AddValueAsInteger_handles_valid_integer_resource)
{
    // A valid path refers to a Resource
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    AwaInteger value = 123456789;
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsInteger(setOperation, "/3/0/9", value));
    AwaClientSetOperation_Free(&setOperation);
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_AddValueAsInteger_handles_undefined_resource)
{
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    AwaInteger value = 123456789;
    ASSERT_EQ(AwaError_NotDefined, AwaClientSetOperation_AddValueAsInteger(setOperation, "/99/0/9", value));
    AwaClientSetOperation_Free(&setOperation);
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_AddValueAsInteger_handles_valid_path_for_resource_with_wrong_type)
{
    // A valid path refers to a Resource
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    AwaInteger value = 123456789;

    // assume /3/0/0 is a String type
    ASSERT_EQ(AwaError_TypeMismatch, AwaClientSetOperation_AddValueAsInteger(setOperation, "/3/0/0", value));
    AwaClientSetOperation_Free(&setOperation);
}

TEST_F(TestSetOperation, AwaClientSetOperation_AddValueAsInteger_handles_null_operation)
{
    AwaInteger value = 123456789;
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientSetOperation_AddValueAsInteger(NULL, "1000/0/1", value));
}

TEST_F(TestSetOperationWithConnectedSession, DISABLED_AwaClientSetOperation_AddValueAsInteger_handles_invalid_operation)
{
    // Consider if the parent session has been freed early
    AwaClientSession * session = AwaClientSession_New();
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(session, "127.0.0.1", global::clientIpcPort));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(session));
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session);

    // Free the session
    AwaClientSession_Free(&session);

    // TODO: we need the session to keep track of all operations, and
    // invalidate them when freed so they can detect this situation.
    AwaInteger value = 123456789;
    ASSERT_EQ(AwaError_SessionInvalid, AwaClientSetOperation_AddValueAsInteger(setOperation, "/3/0/1", value));

    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Free(&setOperation));
    // expect no crash or memory leaks
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_AddValueAsInteger_handles_null_path)
{
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    AwaInteger value = 123456789;
    ASSERT_EQ(AwaError_PathInvalid, AwaClientSetOperation_AddValueAsInteger(setOperation, NULL, value));
    AwaClientSetOperation_Free(&setOperation);
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_AddValueAsInteger_handles_invalid_path)
{
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    AwaInteger value = 123456789;
    EXPECT_EQ(AwaError_PathInvalid, AwaClientSetOperation_AddValueAsInteger(setOperation, "a/n in/valid/ path", value));
    AwaClientSetOperation_Free(&setOperation);
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_AddValueAsInteger_handles_non_resource_path)
{
    // Paths are valid, but do not refer to resources. SetOperation requires resource paths only.
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    AwaInteger value = 123456789;
    EXPECT_EQ(AwaError_PathInvalid, AwaClientSetOperation_AddValueAsInteger(setOperation, "/0", value));
    EXPECT_EQ(AwaError_PathInvalid, AwaClientSetOperation_AddValueAsInteger(setOperation, "/1/12", value));
    AwaClientSetOperation_Free(&setOperation);
}


TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_AddValueAsCString_handles_null_value)
{
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    ASSERT_EQ(AwaError_TypeMismatch, AwaClientSetOperation_AddValueAsCString(setOperation, "/3/0/1", NULL));
    AwaClientSetOperation_Free(&setOperation);
}


/***********************************************************************************************************
 * AwaClientSetOperation_CreateObjectInstance tests
 */

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_CreateObjectInstance_handles_null_operation)
{
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientSetOperation_CreateObjectInstance(NULL, "/3"));
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_CreateObjectInstance_handles_null_path)
{
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_PathInvalid, AwaClientSetOperation_CreateObjectInstance(setOperation, NULL));

    AwaClientSetOperation_Free(&setOperation);
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_CreateObjectInstance_handles_invalid_path)
{
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_PathInvalid, AwaClientSetOperation_CreateObjectInstance(setOperation, "/3/0/1"));

    AwaClientSetOperation_Free(&setOperation);
}

TEST_F(TestSetOperationWithConnectedSession, DISABLED_AwaClientSetOperation_CreateObjectInstance_handles_existing_instance)
{
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/3/0"));

    EXPECT_EQ(AwaError_CannotCreate, AwaClientSetOperation_Perform(setOperation, defaults::timeout));

    AwaClientSetOperation_Free(&setOperation);
}

TEST_F(TestSetOperationWithConnectedSession, DISABLED_AwaClientSetOperation_CreateObjectInstance_handles_maximum_instances)
{
    // Test we cannot create more object instances than the object definition allows
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/3"));

    EXPECT_EQ(AwaError_CannotCreate, AwaClientSetOperation_Perform(setOperation, defaults::timeout));

    AwaClientSetOperation_Free(&setOperation);
}

TEST_F(TestSetOperationWithConnectedSession, DISABLED_AwaClientSetOperation_CreateObjectInstance_handles_valid_operation_specified_instance_id)
{
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1/123"));

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));

    //TODO check in response iterator
    EXPECT_EQ(true, false);

    AwaClientSetOperation_Free(&setOperation);
}

TEST_F(TestSetOperationWithConnectedSession, DISABLED_AwaClientSetOperation_CreateObjectInstance_handles_valid_operation_unspecified_instance_id)
{
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/1"));

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));

    //TODO check in response iterator
    EXPECT_EQ(true, false);

    AwaClientSetOperation_Free(&setOperation);
}

TEST_F(TestSetOperationWithConnectedSession, DISABLED_AwaClientSetOperation_CreateObjectInstance_does_not_create_optional_resources)
{
    //1. define a custom object with an optional resource
    //2. use a set operation to create a custom object instance
    //3. ensure we cannot SET (or get) on an optional resource of that object instance without creating it first
    ASSERT_TRUE(false);
}

TEST_F(TestSetOperationWithConnectedSession, DISABLED_AwaClientSetOperation_CreateObjectInstance_handles_existing_resource)
{
    ASSERT_FALSE(true);
}

TEST_F(TestSetOperationWithConnectedSession, DISABLED_AwaClientSetOperation_CreateObjectInstance_handles_mandatory_resource)
{
    ASSERT_FALSE(true);
}


/***********************************************************************************************************
 * AwaClientSetOperation_CreateOptionalResource tests
 */

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_CreateOptionalResource_handles_null_operation)
{
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientSetOperation_CreateOptionalResource(NULL, "/3"));
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_CreateOptionalResource_handles_null_path)
{
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_PathInvalid, AwaClientSetOperation_CreateOptionalResource(setOperation, NULL));

    AwaClientSetOperation_Free(&setOperation);
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_CreateOptionalResource_handles_invalid_path)
{
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_PathInvalid, AwaClientSetOperation_CreateOptionalResource(setOperation, "/3/0"));

    AwaClientSetOperation_Free(&setOperation);
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_CreateOptionalResource_handles_valid_operation)
{
    //Define our custom object
    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(10000, "Test Object 0", 0, 1);
    ASSERT_TRUE(NULL != customObjectDefinition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(customObjectDefinition, 0, "Test Resource", false, AwaResourceOperations_ReadWrite, 0));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&customObjectDefinition);
    AwaClientDefineOperation_Free(&defineOperation);

    int expected = 12345;

    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10000/0/0"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsInteger(setOperation, "/10000/0/0", expected));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));

    AwaClientSetOperation_Free(&setOperation);


    AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
    ASSERT_TRUE(NULL != getOperation);
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, "/10000/0/0"));
    ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, 0));
    const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
    ASSERT_TRUE(NULL != getResponse);
    const AwaInteger * value = NULL;
    ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsIntegerPointer(getResponse, "/10000/0/0", &value));
    ASSERT_TRUE(NULL != value);
    ASSERT_EQ(expected, *value);
    AwaClientGetOperation_Free(&getOperation);

}

TEST_F(TestSetOperationWithConnectedSession, DISABLED_AwaClientSetOperation_CreateOptionalResource_handles_valid_operation)
{
    ASSERT_FALSE(true);
}


/***********************************************************************************************************
 * AwaClientSetOperation_ProcessSetOperation tests
 */

TEST_F(TestSetOperationWithConnectedSession, DISABLED_AwaClientSetOperation_Perform_handles_valid_operation)
{
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    AwaInteger value = 123456789;
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsInteger(setOperation, "/3/0/9", value));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_Perform_handles_invalid_operation_no_content)
{
    // Test behaviour when operation has no content
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);
}

TEST_F(TestSetOperationWithConnectedSession, DISABLED_AwaClientSetOperation_Perform_handles_invalid_operation_invalid_session)
{
    // Test behaviour when parent session has been disconnected

    // Test behaviour when parent session has been freed

    ASSERT_FALSE(1);
}

TEST_F(TestSetOperationWithConnectedSession, DISABLED_AwaClientSetOperation_Perform_handles_multiple_paths)
{
    // Test behaviour when we add multiple paths to set
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsInteger(setOperation, "/3/0/9", 12345));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsCString(setOperation, "/3/0/1", "abcde"));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_Perform_handles_null_operation)
{
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientSetOperation_Perform(NULL, defaults::timeout));
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_Perform_handles_negative_timeout)
{
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientSetOperation_Perform(NULL, -1));
}

TEST_F(TestSetOperationWithConnectedSession, DISABLED_AwaClientSetOperation_Perform_handles_zero_timeout)
{
    // how?
}

TEST_F(TestSetOperationWithConnectedSession, DISABLED_AwaClientSetOperation_Perform_handles_short_timeout)
{
    // how?
}

TEST_F(TestSetOperation, AwaClientSetOperation_Perform_honours_timeout)
{
    // start a client
    const char * clientID = "TestClient1";

    AwaClientDaemonHorde * horde_ = new AwaClientDaemonHorde( { clientID }, 61001);
    sleep(1);      // wait for the client to register with the server

    AwaClientSession * session = AwaClientSession_New();
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(session, "0.0.0.0", 61001));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(session));

    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session); ASSERT_TRUE(NULL != setOperation);
    ASSERT_TRUE(NULL != setOperation);
    AwaInteger value = 123456789;
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsInteger(setOperation, "/3/0/9", value));

    // Tear down client process
    delete horde_;

    BasicTimer timer;
    timer.Start();
    EXPECT_EQ(AwaError_Timeout, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    timer.Stop();
    EXPECT_TRUE(ElapsedTimeWithinTolerance(timer.TimeElapsed_Milliseconds(), defaults::timeout, defaults::timeoutTolerance)) << "Time elapsed: " << timer.TimeElapsed_Milliseconds() << "ms";

    AwaClientSetOperation_Free(&setOperation);
    AwaClientSession_Free(&session);
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_Perform_handles_disconnected_session)
{
    // Test behaviour when parent session has been disconnected
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_); ASSERT_TRUE(NULL != setOperation);
    AwaInteger value = 123456789;
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsInteger(setOperation, "/3/0/9", value));

    AwaClientSession_Disconnect(session_);

    ASSERT_EQ(AwaError_SessionNotConnected, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);
}

/***********************************************************************************************************
 * SetValue parameterised tests
 */

namespace detail
{

struct TestSetResource
{
    bool CreateSetOperation;
    bool PopulateArrayWithInitialValues;
    bool PerformSetOperation;
    AwaError ExpectedAddResult;
    AwaError ExpectedProcessResult;

    AwaObjectID ObjectID;
    AwaObjectInstanceID ObjectInstanceID;
    AwaResourceID ResourceID;

    const void * Value;
    const size_t ValueCount;
    AwaResourceType Type;
};

::std::ostream& operator<<(::std::ostream& os, const TestSetResource& item)
{
  return os << "Item: expectedAddResult " << item.ExpectedAddResult
            << ", expectedProcessResult " << item.ExpectedProcessResult
            << ", UseSetOperation " << item.CreateSetOperation
            << ", PopulateArrayWithInitialValues " << item.PopulateArrayWithInitialValues
            << ", objectID " << item.ObjectID
            << ", objectInstanceID " << item.ObjectInstanceID
            << ", resourceID " << item.ResourceID
            << ", value " << item.Value
            << ", valueCount " << item.ValueCount
            << ", type" << item.Type;
}

AwaInteger dummyInteger1 = 123456;
const char * dummyString1 = "Lightweight M2M Client";
AwaFloat dummyFloat1 = 1.0;
AwaTime dummyTime1 = 0xA20AD72B;
AwaBoolean dummyBoolean1 = true;

static char dummyOpaqueData[] = {'a',0,'x','\0', 123};
//static int dummyObjLinkData[] = {-1,-1};

AwaOpaque dummyOpaque1 = {(void*) dummyOpaqueData, sizeof(dummyOpaqueData)};
AwaObjectLink dummyObjectLink1 = { 3, 5 };
AwaObjectLink dummyObjectLink2 = { 1, 7 };
AwaObjectLink dummyObjectLink3 = { 0, 1 };


const char * dummyStringArray1[] = {"Lightweight M2M Client", "test1", ""};
const char * dummyStringArray2[] = {"Lightweight M2M Client", "test1", " ", " ", " ", " "};
const AwaInteger dummyIntegerArray1[] = {55, 8732, 11};
const AwaInteger dummyIntegerArray2[] = {55, 8732, 11, 55, 8732, 11};
const AwaFloat dummyFloatArray1[] = {55.0, 0.0008, 11e10};
const AwaFloat dummyFloatArray2[] = {55.0, 0.0008, 11e10, 55.0, 0.0008, 11e10};
const AwaBoolean dummyBooleanArray1[] = {true, false, true};
const AwaBoolean dummyBooleanArray2[] = {true, false, true, true, false, true};

AwaOpaque dummyOpaque2 = {(void*) dummyOpaqueData, sizeof(dummyOpaqueData)};
AwaOpaque dummyOpaque3 = {(void*) dummyOpaqueData, sizeof(dummyOpaqueData)};
const AwaOpaque * dummyOpaqueArray1[] = {&dummyOpaque1, &dummyOpaque2, &dummyOpaque3};
const AwaOpaque * dummyOpaqueArray2[] = {&dummyOpaque1, &dummyOpaque2, &dummyOpaque3, &dummyOpaque1, &dummyOpaque2, &dummyOpaque3};

const AwaInteger dummyTimeArray1[] = {16000, 8732222, 1111};
const AwaInteger dummyTimeArray2[] = {16000, 8732222, 1111, 16000, 8732222, 1111};


const AwaObjectLink * dummyObjectLinkArray1[] = {&dummyObjectLink1, &dummyObjectLink2, &dummyObjectLink3};

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

}

class TestSetValue : public TestSetOperationWithConnectedSession, public ::testing::WithParamInterface< detail::TestSetResource>
{
protected:

    void SetUp() {

        detail::TestSetResource data = GetParam();
        TestSetOperationWithConnectedSession::SetUp();

        AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
        EXPECT_TRUE(defineOperation != NULL);

        AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(detail::TEST_OBJECT_NON_ARRAY_TYPES, "Test Object Single", 0, 1);
        EXPECT_TRUE(NULL != customObjectDefinition);

        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsString     (customObjectDefinition, detail::TEST_RESOURCE_STRING,     "Test String Resource",      true, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger    (customObjectDefinition, detail::TEST_RESOURCE_INTEGER,    "Test Integer Resource",     true, AwaResourceOperations_ReadWrite, 0));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloat      (customObjectDefinition, detail::TEST_RESOURCE_FLOAT,      "Test Float Resource",       true, AwaResourceOperations_ReadWrite, 0.0));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBoolean    (customObjectDefinition, detail::TEST_RESOURCE_BOOLEAN,    "Test Boolean Resource",     true, AwaResourceOperations_ReadWrite, false));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque     (customObjectDefinition, detail::TEST_RESOURCE_OPAQUE,     "Test Opaque Resource",      true, AwaResourceOperations_ReadWrite, AwaOpaque {0}));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTime       (customObjectDefinition, detail::TEST_RESOURCE_TIME,       "Test Time Resource",        true, AwaResourceOperations_ReadWrite, 0));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLink (customObjectDefinition, detail::TEST_RESOURCE_OBJECTLINK, "Test Object Link Resource", true, AwaResourceOperations_ReadWrite, AwaObjectLink {0}));

        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
        AwaObjectDefinition_Free(&customObjectDefinition);

        customObjectDefinition = AwaObjectDefinition_New(detail::TEST_OBJECT_ARRAY_TYPES, "Test Object Array", 0, 1);
        EXPECT_TRUE(NULL != customObjectDefinition);

        int maximumArrayInstances = 5;

        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsStringArray    (customObjectDefinition, detail::TEST_RESOURCE_STRING,     "Test String Array Resource",      0, maximumArrayInstances, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsIntegerArray   (customObjectDefinition, detail::TEST_RESOURCE_INTEGER,    "Test Integer Array Resource",     0, maximumArrayInstances, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloatArray     (customObjectDefinition, detail::TEST_RESOURCE_FLOAT,      "Test Float Array Resource",       0, maximumArrayInstances, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBooleanArray   (customObjectDefinition, detail::TEST_RESOURCE_BOOLEAN,    "Test Boolean Array Resource",     0, maximumArrayInstances, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaqueArray    (customObjectDefinition, detail::TEST_RESOURCE_OPAQUE,     "Test Opaque Array Resource",      0, maximumArrayInstances, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTimeArray      (customObjectDefinition, detail::TEST_RESOURCE_TIME,       "Test Time Array Resource",        0, maximumArrayInstances, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLinkArray(customObjectDefinition, detail::TEST_RESOURCE_OBJECTLINK, "Test Object Link Array Resource", 0, maximumArrayInstances, AwaResourceOperations_ReadWrite, NULL));

        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

        AwaObjectDefinition_Free(&customObjectDefinition);
        AwaClientDefineOperation_Free(&defineOperation);

        AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
        EXPECT_TRUE(NULL != setOperation);
        EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
        EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10001/0"));

        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10001/0/1"));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10001/0/2"));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10001/0/3"));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10001/0/4"));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10001/0/5"));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10001/0/6"));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10001/0/7"));

        if (data.PopulateArrayWithInitialValues)
        {
            for (int i = maximumArrayInstances/2; i < maximumArrayInstances; i++)
            {
                char tempString[8];
                sprintf(tempString, "%d", i * 123);

                char tempOpaqueData[] = {'2',static_cast<char>(i),0,'a','c','\n'};

                ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddArrayValueAsCString(setOperation, "/10001/0/1", i, tempString));
                ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddArrayValueAsInteger(setOperation, "/10001/0/2", i, i*222));
                ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddArrayValueAsFloat(setOperation, "/10001/0/3", i, i*0.2));
                ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddArrayValueAsBoolean(setOperation, "/10001/0/4", i, i == 0));
                ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddArrayValueAsOpaque(setOperation, "/10001/0/5", i, {tempOpaqueData, sizeof(tempOpaqueData)}));
                ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddArrayValueAsTime(setOperation, "/10001/0/6", i, i*10030));
                ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddArrayValueAsObjectLink(setOperation, "/10001/0/7", i, {i, i*10}));
            }
        }

        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, 0));

        AwaClientSetOperation_Free(&setOperation);
    }

    void TearDown() {
        TestSetOperationWithConnectedSession::TearDown();

    }
};

class TestSetValueArray : public TestSetValue {};

TEST_P(TestSetValueArray, TestSetValueArray)
{
    detail::TestSetResource data = GetParam();
    AwaClientSetOperation * setOperation = data.CreateSetOperation? AwaClientSetOperation_New(session_) : NULL;
    char path[128] = {0};

    if(data.ObjectID == AWA_INVALID_ID)
    {
        sprintf(path, "a/n in/valid/ path");
    }
    else
    {
        EXPECT_EQ(AwaError_Success, AwaAPI_MakePath(path, sizeof(path), data.ObjectID, data.ObjectInstanceID, data.ResourceID));
    }

    AwaArray * setArray = NULL;

    switch(data.Type)
    {
        case AwaResourceType_StringArray:
            {
                AwaStringArray * array = AwaStringArray_New();
                EXPECT_TRUE(NULL != array);

                for(size_t i = 0; i < data.ValueCount; i++)
                {
                    AwaStringArray_SetValueAsCString(array, i, ((const char **)data.Value)[i]);
                }

                ASSERT_EQ(data.ExpectedAddResult, AwaClientSetOperation_AddValueAsStringArray(setOperation, path, array));
                setArray = (AwaArray *)array;
            }
            break;
        case AwaResourceType_IntegerArray:
            {
                AwaIntegerArray * array = AwaIntegerArray_New();
                EXPECT_TRUE(NULL != array);

                for(size_t i = 0; i < data.ValueCount; i++)
                {
                    AwaIntegerArray_SetValue(array, i, ((const AwaInteger *)data.Value)[i]);
                }

                ASSERT_EQ(data.ExpectedAddResult, AwaClientSetOperation_AddValueAsIntegerArray(setOperation, path, array));
                setArray = (AwaArray *)array;
            }
            break;
        case AwaResourceType_FloatArray:
            {
                AwaFloatArray * array = AwaFloatArray_New();
                EXPECT_TRUE(NULL != array);

                for(size_t i = 0; i < data.ValueCount; i++)
                {
                    AwaFloatArray_SetValue(array, i, ((const AwaFloat *)data.Value)[i]);
                }

                ASSERT_EQ(data.ExpectedAddResult, AwaClientSetOperation_AddValueAsFloatArray(setOperation, path, array));
                setArray = (AwaArray *)array;
            }
            break;
        case AwaResourceType_BooleanArray:
            {
                AwaBooleanArray * array = AwaBooleanArray_New();
                EXPECT_TRUE(NULL != array);

                for(size_t i = 0; i < data.ValueCount; i++)
                {
                    AwaBooleanArray_SetValue(array, i, ((const AwaBoolean *)data.Value)[i]);
                }

                ASSERT_EQ(data.ExpectedAddResult, AwaClientSetOperation_AddValueAsBooleanArray(setOperation, path, array));
                setArray = (AwaArray *)array;
            }
            break;
        case AwaResourceType_OpaqueArray:
            {
                AwaOpaqueArray * array = AwaOpaqueArray_New();
                EXPECT_TRUE(NULL != array);

                for(size_t i = 0; i < data.ValueCount; i++)
                {
                    AwaOpaqueArray_SetValue(array, i, *((AwaOpaque **)data.Value)[i]);
                }

                ASSERT_EQ(data.ExpectedAddResult, AwaClientSetOperation_AddValueAsOpaqueArray(setOperation, path, array));
                setArray = (AwaArray *)array;
            }
            break;
        case AwaResourceType_TimeArray:
            {
                AwaTimeArray * array = AwaTimeArray_New();
                EXPECT_TRUE(NULL != array);

                for(size_t i = 0; i < data.ValueCount; i++)
                {
                    AwaTimeArray_SetValue(array, i, ((const AwaTime *)data.Value)[i]);
                }

                ASSERT_EQ(data.ExpectedAddResult, AwaClientSetOperation_AddValueAsTimeArray(setOperation, path, array));
                setArray = (AwaArray *)array;
            }
            break;
        case AwaResourceType_ObjectLinkArray:
            {
                AwaObjectLinkArray * array = AwaObjectLinkArray_New();
                EXPECT_TRUE(NULL != array);

                for(size_t i = 0; i < data.ValueCount; i++)
                {
                    AwaObjectLinkArray_SetValue(array, i, *((AwaObjectLink **)data.Value)[i]);
                }

                ASSERT_EQ(data.ExpectedAddResult, AwaClientSetOperation_AddValueAsObjectLinkArray(setOperation, path, array));
                setArray = (AwaArray *)array;
            }
            break;
        default:
            ASSERT_TRUE(false);
            break;
    }
    ASSERT_TRUE(NULL != setArray);
    if (data.PerformSetOperation)
    {
        EXPECT_EQ(data.ExpectedProcessResult, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    }
    AwaClientSetOperation_Free(&setOperation);

    if (data.ExpectedProcessResult == AwaError_Success)
    {
        // Confirm the value was set correctly
        AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
        ASSERT_TRUE(NULL != getOperation);
        ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, path));
        ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
        const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
        ASSERT_TRUE(NULL != getResponse);

        AwaArray * getArray = NULL;

        switch(data.Type)
        {
            case AwaResourceType_StringArray:
            {
                ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsStringArrayPointer(getResponse, path, (const AwaStringArray **)&getArray));
                break;
            }
            case AwaResourceType_IntegerArray:
            {
                ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsIntegerArrayPointer(getResponse, path, (const AwaIntegerArray **)&getArray));
                break;
            }
            case AwaResourceType_FloatArray:
            {
                ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsFloatArrayPointer(getResponse, path, (const AwaFloatArray **)&getArray));
                break;
            }
            case AwaResourceType_BooleanArray:
            {
                ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsBooleanArrayPointer(getResponse, path, (const AwaBooleanArray **)&getArray));
                break;
            }
            case AwaResourceType_OpaqueArray:
            {
                ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsOpaqueArrayPointer(getResponse, path, (const AwaOpaqueArray **)&getArray));
                break;
            }
            case AwaResourceType_TimeArray:
            {
                ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsTimeArrayPointer(getResponse, path, (const AwaTimeArray **)&getArray));
                break;
            }
            case AwaResourceType_ObjectLinkArray:
            {
                ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsObjectLinkArrayPointer(getResponse, path, (const AwaObjectLinkArray **)&getArray));
                break;
            }
            default:
                ASSERT_TRUE(false);
                break;
        }

        EXPECT_EQ(0, Array_Compare(setArray, getArray, data.Type));
        AwaClientGetOperation_Free(&getOperation);
    }


    switch(data.Type)
    {
        case AwaResourceType_StringArray:
            AwaStringArray_Free((AwaStringArray **)&setArray);
            break;
        case AwaResourceType_IntegerArray:
            AwaIntegerArray_Free((AwaIntegerArray **)&setArray);
            break;
        case AwaResourceType_FloatArray:
            AwaFloatArray_Free((AwaFloatArray **)&setArray);
            break;
        case AwaResourceType_BooleanArray:
            AwaBooleanArray_Free((AwaBooleanArray **)&setArray);
            break;
        case AwaResourceType_OpaqueArray:
            AwaOpaqueArray_Free((AwaOpaqueArray **)&setArray);
            break;
        case AwaResourceType_TimeArray:
            AwaTimeArray_Free((AwaTimeArray **)&setArray);
            break;
        case AwaResourceType_ObjectLinkArray:
            AwaObjectLinkArray_Free((AwaObjectLinkArray **)&setArray);
            break;
        default:
            ASSERT_TRUE(false);
            break;
    }


}

INSTANTIATE_TEST_CASE_P(
        TestSetValueArray1,
        TestSetValueArray,
        ::testing::Values(

          detail::TestSetResource {true, false, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRINGARRAY,     &detail::dummyStringArray1,3,     AwaResourceType_StringArray},
          detail::TestSetResource {true, true, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRINGARRAY,     &detail::dummyStringArray1,3,     AwaResourceType_StringArray},
          detail::TestSetResource {true, false, true,AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,               0,                  detail::TEST_RESOURCE_STRINGARRAY,     &detail::dummyStringArray1,3,     AwaResourceType_StringArray},
          detail::TestSetResource {true, false, true,AwaError_TypeMismatch,     AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_INTEGERARRAY,    &detail::dummyStringArray1,3,     AwaResourceType_StringArray},
          detail::TestSetResource {false, false, true,AwaError_OperationInvalid, AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRINGARRAY,     &detail::dummyStringArray1,3,     AwaResourceType_StringArray},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  AWA_INVALID_ID,                    &detail::dummyStringArray1,3,     AwaResourceType_StringArray},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyStringArray1,3,     AwaResourceType_StringArray},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,               AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyStringArray1,3,     AwaResourceType_StringArray},
          detail::TestSetResource {true, false, false,AwaError_AddInvalid,       AwaError_OperationInvalid,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRINGARRAY,     &detail::dummyStringArray2,6,     AwaResourceType_StringArray},

          detail::TestSetResource {true, false, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_INTEGERARRAY,    detail::dummyIntegerArray1,3,    AwaResourceType_IntegerArray},
          detail::TestSetResource {true, true, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_INTEGERARRAY,    detail::dummyIntegerArray1,3,    AwaResourceType_IntegerArray},
          detail::TestSetResource {true, false, true,AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,               0,                  detail::TEST_RESOURCE_INTEGERARRAY,    detail::dummyIntegerArray1,3,    AwaResourceType_IntegerArray},
          detail::TestSetResource {true, false, true,AwaError_TypeMismatch,     AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRINGARRAY,     detail::dummyIntegerArray1,3,    AwaResourceType_IntegerArray},
          detail::TestSetResource {false, false, true,AwaError_OperationInvalid, AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_INTEGERARRAY,    detail::dummyIntegerArray1,3,    AwaResourceType_IntegerArray},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  AWA_INVALID_ID,                    detail::dummyIntegerArray1,3,    AwaResourceType_IntegerArray},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,                    detail::dummyIntegerArray1,3,    AwaResourceType_IntegerArray},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,               AWA_INVALID_ID, AWA_INVALID_ID,                    detail::dummyIntegerArray1,3,    AwaResourceType_IntegerArray},
          detail::TestSetResource {true, false, false,AwaError_AddInvalid,       AwaError_OperationInvalid,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_INTEGERARRAY,    detail::dummyIntegerArray2,6,     AwaResourceType_IntegerArray},

          detail::TestSetResource {true, false, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_FLOATARRAY,      &detail::dummyFloatArray1,3,     AwaResourceType_FloatArray},
          detail::TestSetResource {true, true, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_FLOATARRAY,      &detail::dummyFloatArray1,3,     AwaResourceType_FloatArray},
          detail::TestSetResource {true, false, true,AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,               0,                  detail::TEST_RESOURCE_FLOATARRAY,      &detail::dummyFloatArray1,3,     AwaResourceType_FloatArray},
          detail::TestSetResource {true, false, true,AwaError_TypeMismatch,     AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRINGARRAY,     &detail::dummyFloatArray1,3,     AwaResourceType_FloatArray},
          detail::TestSetResource {false, false, true,AwaError_OperationInvalid, AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_FLOATARRAY,      &detail::dummyFloatArray1,3,     AwaResourceType_FloatArray},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  AWA_INVALID_ID,                    &detail::dummyFloatArray1,3,     AwaResourceType_FloatArray},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyFloatArray1,3,     AwaResourceType_FloatArray},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,               AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyFloatArray1,3,     AwaResourceType_FloatArray},
          detail::TestSetResource {true, false, false,AwaError_AddInvalid,       AwaError_OperationInvalid,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_FLOATARRAY,      &detail::dummyFloatArray2,6,      AwaResourceType_FloatArray},

          detail::TestSetResource {true, false, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_BOOLEANARRAY,    &detail::dummyBooleanArray1,3,    AwaResourceType_BooleanArray},
          detail::TestSetResource {true, true, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_BOOLEANARRAY,    &detail::dummyBooleanArray1,3,    AwaResourceType_BooleanArray},
          detail::TestSetResource {true, false, true,AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,               0,                  detail::TEST_RESOURCE_BOOLEANARRAY,    &detail::dummyBooleanArray1,3,    AwaResourceType_BooleanArray},
          detail::TestSetResource {true, false, true,AwaError_TypeMismatch,     AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRINGARRAY,     &detail::dummyBooleanArray1,3,    AwaResourceType_BooleanArray},
          detail::TestSetResource {false, false, true,AwaError_OperationInvalid, AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_BOOLEANARRAY,    &detail::dummyBooleanArray1,3,    AwaResourceType_BooleanArray},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  AWA_INVALID_ID,                    &detail::dummyBooleanArray1,3,    AwaResourceType_BooleanArray},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyBooleanArray1,3,    AwaResourceType_BooleanArray},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,               AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyBooleanArray1,3,    AwaResourceType_BooleanArray},
          detail::TestSetResource {true, false, false,AwaError_AddInvalid,       AwaError_OperationInvalid,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_BOOLEANARRAY,    &detail::dummyBooleanArray2,6,    AwaResourceType_BooleanArray}

          ));

INSTANTIATE_TEST_CASE_P(
        TestSetValueArray2,
        TestSetValueArray,
        ::testing::Values(

          detail::TestSetResource {true, false, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_OPAQUEARRAY,     &detail::dummyOpaqueArray1,3,     AwaResourceType_OpaqueArray},
          detail::TestSetResource {true, true, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_OPAQUEARRAY,     &detail::dummyOpaqueArray1,3,     AwaResourceType_OpaqueArray},
          detail::TestSetResource {true, false, true,AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,               0,                  detail::TEST_RESOURCE_OPAQUEARRAY,     &detail::dummyOpaqueArray1,3,     AwaResourceType_OpaqueArray},
          detail::TestSetResource {true, false, true,AwaError_TypeMismatch,     AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRINGARRAY,     &detail::dummyOpaqueArray1,3,     AwaResourceType_OpaqueArray},
          detail::TestSetResource {false, false, true,AwaError_OperationInvalid, AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_OPAQUEARRAY,     &detail::dummyOpaqueArray1,3,     AwaResourceType_OpaqueArray},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  AWA_INVALID_ID,                    &detail::dummyOpaqueArray1,3,     AwaResourceType_OpaqueArray},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyOpaqueArray1,3,     AwaResourceType_OpaqueArray},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,               AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyOpaqueArray1,3,     AwaResourceType_OpaqueArray},
          detail::TestSetResource {true, false, false,AwaError_AddInvalid,       AwaError_OperationInvalid,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_BOOLEANARRAY,    &detail::dummyBooleanArray2,6,    AwaResourceType_BooleanArray},

          detail::TestSetResource {true, false, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_TIMEARRAY,       &detail::dummyTimeArray1,3,       AwaResourceType_TimeArray},
          detail::TestSetResource {true, true, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_TIMEARRAY,       &detail::dummyTimeArray1,3,       AwaResourceType_TimeArray},
          detail::TestSetResource {true, false, true,AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,               0,                  detail::TEST_RESOURCE_TIMEARRAY,       &detail::dummyTimeArray1,3,       AwaResourceType_TimeArray},
          detail::TestSetResource {true, false, true,AwaError_TypeMismatch,     AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRINGARRAY,     &detail::dummyTimeArray1,3,       AwaResourceType_TimeArray},
          detail::TestSetResource {false, false, true,AwaError_OperationInvalid, AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_TIMEARRAY,       &detail::dummyTimeArray1,3,       AwaResourceType_TimeArray},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  AWA_INVALID_ID,                    &detail::dummyTimeArray1,3,       AwaResourceType_TimeArray},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyTimeArray1,3,       AwaResourceType_TimeArray},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,               AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyTimeArray1,3,       AwaResourceType_TimeArray},

          detail::TestSetResource {true, false, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_OBJECTLINKARRAY, &detail::dummyObjectLinkArray1,3, AwaResourceType_ObjectLinkArray},
          detail::TestSetResource {true, true, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_OBJECTLINKARRAY, &detail::dummyObjectLinkArray1,3, AwaResourceType_ObjectLinkArray},
          detail::TestSetResource {true, false, true,AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,               0,                  detail::TEST_RESOURCE_OBJECTLINKARRAY, &detail::dummyObjectLinkArray1,3, AwaResourceType_ObjectLinkArray},
          detail::TestSetResource {true, false, true,AwaError_TypeMismatch,     AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRINGARRAY,     &detail::dummyObjectLinkArray1,3, AwaResourceType_ObjectLinkArray},
          detail::TestSetResource {false, false, true,AwaError_OperationInvalid, AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_OBJECTLINKARRAY, &detail::dummyObjectLinkArray1,3, AwaResourceType_ObjectLinkArray},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  AWA_INVALID_ID,                    &detail::dummyObjectLinkArray1,3, AwaResourceType_ObjectLinkArray},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyObjectLinkArray1,3, AwaResourceType_ObjectLinkArray},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,               AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyObjectLinkArray1,3, AwaResourceType_ObjectLinkArray}

        ));

class TestSetValueSingle : public TestSetValue {};

TEST_P(TestSetValueSingle, TestSetValueSingle)
{
    detail::TestSetResource data = GetParam();

    AwaClientSetOperation * setOperation = data.CreateSetOperation ? AwaClientSetOperation_New(session_) : NULL;
    char path[128] = {0};

    if(data.ObjectID == AWA_INVALID_ID)
    {
        sprintf(path, "a/n in/valid/ path");
    }
    else
    {
        EXPECT_EQ(AwaError_Success, AwaAPI_MakePath(path, sizeof(path), data.ObjectID, data.ObjectInstanceID, data.ResourceID));
    }

    switch(data.Type)
    {
        case AwaResourceType_String:
            ASSERT_EQ(data.ExpectedAddResult, AwaClientSetOperation_AddValueAsCString(setOperation, path, (const char *)data.Value));
            break;
        case AwaResourceType_Integer:
            ASSERT_EQ(data.ExpectedAddResult, AwaClientSetOperation_AddValueAsInteger(setOperation, path, *((AwaInteger*)data.Value)));
            break;
        case AwaResourceType_Float:
            ASSERT_EQ(data.ExpectedAddResult, AwaClientSetOperation_AddValueAsFloat(setOperation, path, *((AwaFloat*)data.Value)));
            break;
        case AwaResourceType_Boolean:
            ASSERT_EQ(data.ExpectedAddResult, AwaClientSetOperation_AddValueAsBoolean(setOperation, path, *((AwaBoolean*)data.Value)));
            break;
        case AwaResourceType_Opaque:
            ASSERT_EQ(data.ExpectedAddResult, AwaClientSetOperation_AddValueAsOpaque(setOperation, path, *((AwaOpaque*)data.Value)));
            break;
        case AwaResourceType_Time:
            ASSERT_EQ(data.ExpectedAddResult, AwaClientSetOperation_AddValueAsTime(setOperation, path, *((AwaTime*)data.Value)));
            break;
        case AwaResourceType_ObjectLink:
            ASSERT_EQ(data.ExpectedAddResult, AwaClientSetOperation_AddValueAsObjectLink(setOperation, path, *((AwaObjectLink*)data.Value)));
            break;
        default:
            ASSERT_TRUE(false);
            break;
    }

    EXPECT_EQ(data.ExpectedProcessResult, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    if (data.ExpectedProcessResult == AwaError_Success)
    {
        // Confirm the value was set correctly
        AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
        ASSERT_TRUE(NULL != getOperation);
        ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, path));
        ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
        const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
        ASSERT_TRUE(NULL != getResponse);

        void * value = NULL;

        AwaObjectLink receivedObjectLink = {0, 0};
        AwaOpaque receivedOpaque = {NULL, 0};

        switch(data.Type)
        {
        case AwaResourceType_String:
            ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsCStringPointer(getResponse, path, (const char **)&value));
            break;
        case AwaResourceType_Integer:
            ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsIntegerPointer(getResponse, path, (const AwaInteger **)&value));
            break;
        case AwaResourceType_Float:
            ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsFloatPointer(getResponse, path, (const AwaFloat **)&value));
            break;
        case AwaResourceType_Boolean:
            ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsBooleanPointer(getResponse, path, (const AwaBoolean **)&value));
            break;
        case AwaResourceType_Opaque:
            ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsOpaque(getResponse, path, &receivedOpaque));
            ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsOpaquePointer(getResponse, path, (const AwaOpaque **)&value));
            break;
        case AwaResourceType_Time:
            ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsTimePointer(getResponse, path, (const AwaTime **)&value));
            break;
        case AwaResourceType_ObjectLink:
            ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsObjectLink(getResponse, path, &receivedObjectLink));
            ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValueAsObjectLinkPointer(getResponse, path, (const AwaObjectLink **)&value));
            break;
        default:
            ASSERT_TRUE(false);
            break;
        }


        switch(data.Type)
        {
            case AwaResourceType_String:
            {
                ASSERT_STREQ((char*) data.Value, (char*) value);
                break;
            }
            case AwaResourceType_Opaque:
            {
                AwaOpaque * expectedOpaque = (AwaOpaque *) data.Value;
                AwaOpaque * receivedOpaquePointer = (AwaOpaque *) value;

                ASSERT_EQ(expectedOpaque->Size, receivedOpaquePointer->Size);
                ASSERT_EQ(expectedOpaque->Size, receivedOpaque.Size);
                ASSERT_EQ(0, memcmp(expectedOpaque->Data, receivedOpaquePointer->Data, expectedOpaque->Size));
                ASSERT_EQ(0, memcmp(expectedOpaque->Data, receivedOpaque.Data, receivedOpaque.Size));
                break;
            }
            case AwaResourceType_Integer:
                ASSERT_EQ(*static_cast<const AwaInteger *>(data.Value), *static_cast<AwaInteger *>(value));
                break;
            case AwaResourceType_Float:
                ASSERT_EQ(*static_cast<const AwaFloat *>(data.Value), *static_cast<AwaFloat *>(value));
                break;
            case AwaResourceType_Boolean:
                ASSERT_EQ(*static_cast<const AwaBoolean *>(data.Value), *static_cast<AwaBoolean *>(value));
                break;
            case AwaResourceType_Time:
                ASSERT_EQ(*static_cast<const AwaTime *>(data.Value), *static_cast<AwaTime *>(value));
                break;
            case AwaResourceType_ObjectLink:
            {
                const AwaObjectLink * expectedObjectLink = static_cast<const AwaObjectLink *>(data.Value);
                const AwaObjectLink * receivedObjectLinkPointer = static_cast<AwaObjectLink *>(value);
                ASSERT_EQ(0, memcmp(expectedObjectLink, receivedObjectLinkPointer, sizeof(AwaObjectLink)));
                ASSERT_EQ(0, memcmp(expectedObjectLink, &receivedObjectLink, sizeof(AwaObjectLink)));
                break;
            }
            default:
                ASSERT_TRUE(false);
        }


        AwaClientGetOperation_Free(&getOperation);
    }
}

INSTANTIATE_TEST_CASE_P(
        TestSetValueSingle,
        TestSetValueSingle,
        ::testing::Values(

          detail::TestSetResource {true, false, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRING,     detail::dummyString1,1,     AwaResourceType_String},
          detail::TestSetResource {true, false, true,AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,                   0,                  detail::TEST_RESOURCE_STRING,     detail::dummyString1,1,     AwaResourceType_String},
          detail::TestSetResource {true, false, true,AwaError_TypeMismatch,     AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_INTEGER,    detail::dummyString1,1,     AwaResourceType_String},
          detail::TestSetResource {false, false, true,AwaError_OperationInvalid, AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRING,     detail::dummyString1,1,     AwaResourceType_String},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  AWA_INVALID_ID,               detail::dummyString1,1,     AwaResourceType_String},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,               detail::dummyString1,1,     AwaResourceType_String},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,                   AWA_INVALID_ID, AWA_INVALID_ID,               detail::dummyString1,1,     AwaResourceType_String},

          detail::TestSetResource {true, false, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_INTEGER,    &detail::dummyInteger1,1,    AwaResourceType_Integer},
          detail::TestSetResource {true, false, true,AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,                   0,                  detail::TEST_RESOURCE_INTEGER,    &detail::dummyInteger1,1,    AwaResourceType_Integer},
          detail::TestSetResource {true, false, true,AwaError_TypeMismatch,     AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRING,     &detail::dummyInteger1,1,    AwaResourceType_Integer},
          detail::TestSetResource {false, false, true,AwaError_OperationInvalid, AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_INTEGER,    &detail::dummyInteger1,1,    AwaResourceType_Integer},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  AWA_INVALID_ID,               &detail::dummyInteger1,1,    AwaResourceType_Integer},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,               &detail::dummyInteger1,1,    AwaResourceType_Integer},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,                   AWA_INVALID_ID, AWA_INVALID_ID,               &detail::dummyInteger1,1,    AwaResourceType_Integer},

          detail::TestSetResource {true, false, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_FLOAT,      &detail::dummyFloat1,1,      AwaResourceType_Float},
          detail::TestSetResource {true, false, true,AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,                   0,                  detail::TEST_RESOURCE_FLOAT,      &detail::dummyFloat1,1,      AwaResourceType_Float},
          detail::TestSetResource {true, false, true,AwaError_TypeMismatch,     AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRING,     &detail::dummyFloat1,1,      AwaResourceType_Float},
          detail::TestSetResource {false, false, true,AwaError_OperationInvalid, AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_FLOAT,      &detail::dummyFloat1,1,      AwaResourceType_Float},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  AWA_INVALID_ID,               &detail::dummyFloat1,1,      AwaResourceType_Float},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,               &detail::dummyFloat1,1,      AwaResourceType_Float},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,                   AWA_INVALID_ID, AWA_INVALID_ID,               &detail::dummyFloat1,1,      AwaResourceType_Float},

          detail::TestSetResource {true, false, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_BOOLEAN,    &detail::dummyBoolean1,1,    AwaResourceType_Boolean},
          detail::TestSetResource {true, false, true,AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,                   0,                  detail::TEST_RESOURCE_BOOLEAN,    &detail::dummyBoolean1,1,    AwaResourceType_Boolean},
          detail::TestSetResource {true, false, true,AwaError_TypeMismatch,     AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRING,     &detail::dummyBoolean1,1,    AwaResourceType_Boolean},
          detail::TestSetResource {false, false, true,AwaError_OperationInvalid, AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_BOOLEAN,    &detail::dummyBoolean1,1,    AwaResourceType_Boolean},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  AWA_INVALID_ID,               &detail::dummyBoolean1,1,    AwaResourceType_Boolean},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,               &detail::dummyBoolean1,1,    AwaResourceType_Boolean},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,                   AWA_INVALID_ID, AWA_INVALID_ID,               &detail::dummyBoolean1,1,    AwaResourceType_Boolean},

          detail::TestSetResource {true, false, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_OPAQUE,     &detail::dummyOpaque1,1,     AwaResourceType_Opaque},
          detail::TestSetResource {true, false, true,AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,                   0,                  detail::TEST_RESOURCE_OPAQUE,     &detail::dummyOpaque1,1,     AwaResourceType_Opaque},
          detail::TestSetResource {true, false, true,AwaError_TypeMismatch,     AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRING,     &detail::dummyOpaque1,1,     AwaResourceType_Opaque},
          detail::TestSetResource {false, false, true,AwaError_OperationInvalid, AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_OPAQUE,     &detail::dummyOpaque1,1,     AwaResourceType_Opaque},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  AWA_INVALID_ID,               &detail::dummyOpaque1,1,     AwaResourceType_Opaque},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,               &detail::dummyOpaque1,1,     AwaResourceType_Opaque},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,                   AWA_INVALID_ID, AWA_INVALID_ID,               &detail::dummyOpaque1,1,     AwaResourceType_Opaque},

          detail::TestSetResource {true, false, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_TIME,       &detail::dummyTime1,1,       AwaResourceType_Time},
          detail::TestSetResource {true, false, true,AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,                   0,                  detail::TEST_RESOURCE_TIME,       &detail::dummyTime1,1,       AwaResourceType_Time},
          detail::TestSetResource {true, false, true,AwaError_TypeMismatch,     AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRING,     &detail::dummyTime1,1,       AwaResourceType_Time},
          detail::TestSetResource {false, false, true,AwaError_OperationInvalid, AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_TIME,       &detail::dummyTime1,1,       AwaResourceType_Time},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  AWA_INVALID_ID,               &detail::dummyTime1,1,       AwaResourceType_Time},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,               &detail::dummyTime1,1,       AwaResourceType_Time},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,                   AWA_INVALID_ID, AWA_INVALID_ID,               &detail::dummyTime1,1,       AwaResourceType_Time},

          detail::TestSetResource {true, false, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_OBJECTLINK, &detail::dummyObjectLink1,1, AwaResourceType_ObjectLink},
          detail::TestSetResource {true, false, true,AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,                   0,                  detail::TEST_RESOURCE_OBJECTLINK, &detail::dummyObjectLink1,1, AwaResourceType_ObjectLink},
          detail::TestSetResource {true, false, true,AwaError_TypeMismatch,     AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRING,     &detail::dummyObjectLink1,1, AwaResourceType_ObjectLink},
          detail::TestSetResource {false, false, true,AwaError_OperationInvalid, AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_OBJECTLINK, &detail::dummyObjectLink1,1, AwaResourceType_ObjectLink},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  0,                  AWA_INVALID_ID,               &detail::dummyObjectLink1,1, AwaResourceType_ObjectLink},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_NON_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,               &detail::dummyObjectLink1,1, AwaResourceType_ObjectLink},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,                   AWA_INVALID_ID, AWA_INVALID_ID,               &detail::dummyObjectLink1,1, AwaResourceType_ObjectLink}

        ));

class TestSetValueArraySingle : public TestSetValue {};

TEST_P(TestSetValueArraySingle, TestSetValueArraySingle)
{
    detail::TestSetResource data = GetParam();
    AwaClientSetOperation * setOperation = data.CreateSetOperation? AwaClientSetOperation_New(session_) : NULL;
    char path[128] = {0};

    if(data.ObjectID == AWA_INVALID_ID)
    {
        sprintf(path, "a/n in/valid/ path");
    }
    else
    {
        EXPECT_EQ(AwaError_Success, AwaAPI_MakePath(path, sizeof(path), data.ObjectID, data.ObjectInstanceID, data.ResourceID));
    }

    AwaArrayIndex resourceInstanceID = 1;

    switch(data.Type)
    {
        case AwaResourceType_String:
            ASSERT_EQ(data.ExpectedAddResult, AwaClientSetOperation_AddArrayValueAsCString(setOperation, path, resourceInstanceID, (const char *)data.Value));
            break;
        case AwaResourceType_Integer:
            ASSERT_EQ(data.ExpectedAddResult, AwaClientSetOperation_AddArrayValueAsInteger(setOperation, path, resourceInstanceID, *((AwaInteger*)data.Value)));
            break;
        case AwaResourceType_Float:
            ASSERT_EQ(data.ExpectedAddResult, AwaClientSetOperation_AddArrayValueAsFloat(setOperation, path, resourceInstanceID, *((AwaFloat*)data.Value)));
            break;
        case AwaResourceType_Boolean:
            ASSERT_EQ(data.ExpectedAddResult, AwaClientSetOperation_AddArrayValueAsBoolean(setOperation, path, resourceInstanceID, *((AwaBoolean*)data.Value)));
            break;
        case AwaResourceType_Opaque:
            ASSERT_EQ(data.ExpectedAddResult, AwaClientSetOperation_AddArrayValueAsOpaque(setOperation, path, resourceInstanceID, *((AwaOpaque*)data.Value)));
            break;
        case AwaResourceType_Time:
            ASSERT_EQ(data.ExpectedAddResult, AwaClientSetOperation_AddArrayValueAsTime(setOperation, path, resourceInstanceID, *((AwaTime*)data.Value)));
            break;
        case AwaResourceType_ObjectLink:
            ASSERT_EQ(data.ExpectedAddResult, AwaClientSetOperation_AddArrayValueAsObjectLink(setOperation, path, resourceInstanceID, *((AwaObjectLink*)data.Value)));
            break;
        default:
            ASSERT_TRUE(false);
            break;
    }

    EXPECT_EQ(data.ExpectedProcessResult, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    if (data.ExpectedProcessResult == AwaError_Success)
    {
        // Confirm the value was set correctly
        AwaClientGetOperation * getOperation = AwaClientGetOperation_New(session_);
        ASSERT_TRUE(NULL != getOperation);
        ASSERT_EQ(AwaError_Success, AwaClientGetOperation_AddPath(getOperation, path));
        ASSERT_EQ(AwaError_Success, AwaClientGetOperation_Perform(getOperation, defaults::timeout));
        const AwaClientGetResponse * getResponse = AwaClientGetOperation_GetResponse(getOperation);
        ASSERT_TRUE(NULL != getResponse);

        AwaArray * getArray = NULL;

        switch(data.Type)
        {
            case AwaResourceType_String:
            {
                ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsStringArrayPointer(getResponse, path, (const AwaStringArray **)&getArray));
                EXPECT_STREQ(static_cast<const char *>(data.Value), AwaStringArray_GetValueAsCString((const AwaStringArray *)getArray, resourceInstanceID));
                break;
            }
            case AwaResourceType_Integer:
            {
                ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsIntegerArrayPointer(getResponse, path, (const AwaIntegerArray **)&getArray));
                EXPECT_EQ(*((AwaInteger *)data.Value), AwaIntegerArray_GetValue((const AwaIntegerArray *)getArray, resourceInstanceID));
                break;
            }
            case AwaResourceType_Float:
            {
                ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsFloatArrayPointer(getResponse, path, (const AwaFloatArray **)&getArray));
                EXPECT_EQ(*((AwaFloat *)data.Value), AwaFloatArray_GetValue((const AwaFloatArray *)getArray, resourceInstanceID));
                break;
            }
            case AwaResourceType_Boolean:
            {
                ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsBooleanArrayPointer(getResponse, path, (const AwaBooleanArray **)&getArray));
                EXPECT_EQ(*((AwaBoolean *)data.Value), AwaBooleanArray_GetValue((const AwaBooleanArray *)getArray, resourceInstanceID));
                break;
            }
            case AwaResourceType_Opaque:
            {
                AwaOpaque * expected = (AwaOpaque *)data.Value;

                ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsOpaqueArrayPointer(getResponse, path, (const AwaOpaqueArray **)&getArray));
                AwaOpaque actual = AwaOpaqueArray_GetValue((const AwaOpaqueArray *)getArray, resourceInstanceID);

                EXPECT_EQ(expected->Size, actual.Size);
                EXPECT_EQ(0, memcmp(expected->Data, actual.Data, expected->Size));
                break;
            }
            case AwaResourceType_Time:
            {
                ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsTimeArrayPointer(getResponse, path, (const AwaTimeArray **)&getArray));
                EXPECT_EQ(*((AwaTime *)data.Value), AwaTimeArray_GetValue((const AwaTimeArray *)getArray, resourceInstanceID));
                break;
            }
            case AwaResourceType_ObjectLink:
            {
                AwaObjectLink * expected = (AwaObjectLink *)data.Value;
                ASSERT_EQ(AwaError_Success, AwaClientGetResponse_GetValuesAsObjectLinkArrayPointer(getResponse, path, (const AwaObjectLinkArray **)&getArray));
                AwaObjectLink actual = AwaObjectLinkArray_GetValue((const AwaObjectLinkArray *)getArray, resourceInstanceID);

                EXPECT_EQ(0, memcmp(expected, &actual, sizeof(*expected)));
                break;
            }
            default:
                ASSERT_TRUE(false);
                break;
        }

        AwaClientGetOperation_Free(&getOperation);
    }
}

INSTANTIATE_TEST_CASE_P(
        TestSetValueArraySingle,
        TestSetValueArraySingle,
        ::testing::Values(
          detail::TestSetResource {true, false, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRINGARRAY,     detail::dummyString1,1,     AwaResourceType_String},
          detail::TestSetResource {true, false, true,AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,               0,                  detail::TEST_RESOURCE_STRINGARRAY,     detail::dummyString1,1,     AwaResourceType_String},
          detail::TestSetResource {true, false, true,AwaError_TypeMismatch,     AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_INTEGERARRAY,    detail::dummyString1,1,     AwaResourceType_String},
          detail::TestSetResource {false, false, true,AwaError_OperationInvalid, AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRINGARRAY,     detail::dummyString1,1,     AwaResourceType_String},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  AWA_INVALID_ID,                    detail::dummyString1,1,     AwaResourceType_String},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,                    detail::dummyString1,1,     AwaResourceType_String},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,               AWA_INVALID_ID, AWA_INVALID_ID,                    detail::dummyString1,1,     AwaResourceType_String},

          detail::TestSetResource {true, false, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_INTEGERARRAY,    &detail::dummyInteger1,1,    AwaResourceType_Integer},
          detail::TestSetResource {true, false, true,AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,               0,                  detail::TEST_RESOURCE_INTEGERARRAY,    &detail::dummyInteger1,1,    AwaResourceType_Integer},
          detail::TestSetResource {true, false, true,AwaError_TypeMismatch,     AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRINGARRAY,     &detail::dummyInteger1,1,    AwaResourceType_Integer},
          detail::TestSetResource {false, false, true,AwaError_OperationInvalid, AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_INTEGERARRAY,    &detail::dummyInteger1,1,    AwaResourceType_Integer},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  AWA_INVALID_ID,                    &detail::dummyInteger1,1,    AwaResourceType_Integer},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyInteger1,1,    AwaResourceType_Integer},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,               AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyInteger1,1,    AwaResourceType_Integer},

          detail::TestSetResource {true, false, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_FLOATARRAY,      &detail::dummyFloat1,1,      AwaResourceType_Float},
          detail::TestSetResource {true, false, true,AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,               0,                  detail::TEST_RESOURCE_FLOATARRAY,      &detail::dummyFloat1,1,      AwaResourceType_Float},
          detail::TestSetResource {true, false, true,AwaError_TypeMismatch,     AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRINGARRAY,     &detail::dummyFloat1,1,      AwaResourceType_Float},
          detail::TestSetResource {false, false, true,AwaError_OperationInvalid, AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_FLOATARRAY,      &detail::dummyFloat1,1,      AwaResourceType_Float},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  AWA_INVALID_ID,                    &detail::dummyFloat1,1,      AwaResourceType_Float},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyFloat1,1,      AwaResourceType_Float},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,               AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyFloat1,1,      AwaResourceType_Float},

          detail::TestSetResource {true, false, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_BOOLEANARRAY,    &detail::dummyBoolean1,1,    AwaResourceType_Boolean},
          detail::TestSetResource {true, false, true,AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,               0,                  detail::TEST_RESOURCE_BOOLEANARRAY,    &detail::dummyBoolean1,1,    AwaResourceType_Boolean},
          detail::TestSetResource {true, false, true,AwaError_TypeMismatch,     AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRINGARRAY,     &detail::dummyBoolean1,1,    AwaResourceType_Boolean},
          detail::TestSetResource {false, false, true,AwaError_OperationInvalid, AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_BOOLEANARRAY,    &detail::dummyBoolean1,1,    AwaResourceType_Boolean},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  AWA_INVALID_ID,                    &detail::dummyBoolean1,1,    AwaResourceType_Boolean},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyBoolean1,1,    AwaResourceType_Boolean},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,               AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyBoolean1,1,    AwaResourceType_Boolean},

          detail::TestSetResource {true, false, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_OPAQUEARRAY,     &detail::dummyOpaque1,1,     AwaResourceType_Opaque},
          detail::TestSetResource {true, false, true,AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,               0,                  detail::TEST_RESOURCE_OPAQUEARRAY,     &detail::dummyOpaque1,1,     AwaResourceType_Opaque},
          detail::TestSetResource {true, false, true,AwaError_TypeMismatch,     AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRINGARRAY,     &detail::dummyOpaque1,1,     AwaResourceType_Opaque},
          detail::TestSetResource {false, false, true,AwaError_OperationInvalid, AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_OPAQUEARRAY,     &detail::dummyOpaque1,1,     AwaResourceType_Opaque},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  AWA_INVALID_ID,                    &detail::dummyOpaque1,1,     AwaResourceType_Opaque},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyOpaque1,1,     AwaResourceType_Opaque},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,               AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyOpaque1,1,     AwaResourceType_Opaque},

          detail::TestSetResource {true, false, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_TIMEARRAY,       &detail::dummyTime1,1,       AwaResourceType_Time},
          detail::TestSetResource {true, false, true,AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,               0,                  detail::TEST_RESOURCE_TIMEARRAY,       &detail::dummyTime1,1,       AwaResourceType_Time},
          detail::TestSetResource {true, false, true,AwaError_TypeMismatch,     AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRINGARRAY,     &detail::dummyTime1,1,       AwaResourceType_Time},
          detail::TestSetResource {false, false, true,AwaError_OperationInvalid, AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_TIMEARRAY,       &detail::dummyTime1,1,       AwaResourceType_Time},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  AWA_INVALID_ID,                    &detail::dummyTime1,1,       AwaResourceType_Time},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyTime1,1,       AwaResourceType_Time},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,               AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyTime1,1,       AwaResourceType_Time},

          detail::TestSetResource {true, false, true,AwaError_Success,          AwaError_Success,          detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_OBJECTLINKARRAY, &detail::dummyObjectLink1,1, AwaResourceType_ObjectLink},
          detail::TestSetResource {true, false, true,AwaError_NotDefined,       AwaError_OperationInvalid, AWA_MAX_ID - 1,               0,                  detail::TEST_RESOURCE_OBJECTLINKARRAY, &detail::dummyObjectLink1,1, AwaResourceType_ObjectLink},
          detail::TestSetResource {true, false, true,AwaError_TypeMismatch,     AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_STRINGARRAY,     &detail::dummyObjectLink1,1, AwaResourceType_ObjectLink},
          detail::TestSetResource {false, false, true,AwaError_OperationInvalid, AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  detail::TEST_RESOURCE_OBJECTLINKARRAY, &detail::dummyObjectLink1,1, AwaResourceType_ObjectLink},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  0,                  AWA_INVALID_ID,                    &detail::dummyObjectLink1,1, AwaResourceType_ObjectLink},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, detail::TEST_OBJECT_ARRAY_TYPES,  AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyObjectLink1,1, AwaResourceType_ObjectLink},
          detail::TestSetResource {true, false, true,AwaError_PathInvalid,      AwaError_OperationInvalid, AWA_INVALID_ID,               AWA_INVALID_ID, AWA_INVALID_ID,                    &detail::dummyObjectLink1,1, AwaResourceType_ObjectLink}

        ));



TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_GetResponse_handles_null_operation)
{
    EXPECT_EQ(NULL, AwaClientSetOperation_GetResponse(NULL));
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetOperation_GetResponse_handles_valid_operation)
{
    AwaClientSetOperation * operation = AwaClientSetOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsCString(operation, "/3/0/15", "hello"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(operation, defaults::timeout));

    EXPECT_TRUE(NULL != operation);
    EXPECT_TRUE(NULL != AwaClientSetOperation_GetResponse(operation));
    AwaClientSetOperation_Free(&operation);
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetResponse_NewPathIterator_handles_null_response)
{
    EXPECT_EQ(NULL, AwaClientSetResponse_NewPathIterator(NULL));
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetResponse_NewPathIterator_handles_single_path_response)
{
    AwaClientSetOperation * operation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsCString(operation, "/3/0/15", "hello"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(operation, defaults::timeout));
    const AwaClientSetResponse * response = AwaClientSetOperation_GetResponse(operation);
    ASSERT_TRUE(NULL != response);

    AwaPathIterator * iterator = AwaClientSetResponse_NewPathIterator(response);
    ASSERT_TRUE(NULL != iterator);

    // should be one path
    EXPECT_FALSE(AwaPathIterator_Get(iterator));
    EXPECT_TRUE(AwaPathIterator_Next(iterator));
    EXPECT_STREQ("/3/0/15", AwaPathIterator_Get(iterator));
    EXPECT_FALSE(AwaPathIterator_Next(iterator));
    AwaPathIterator_Free(&iterator);

    AwaClientSetOperation_Free(&operation);
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetResponse_NewPathIterator_handles_multiple_path_response)
{
    AwaClientSetOperation * operation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsCString(operation, "/3/0/14", "hello"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsCString(operation, "/3/0/15", "world"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(operation, defaults::timeout));
    const AwaClientSetResponse * response = AwaClientSetOperation_GetResponse(operation);
    ASSERT_TRUE(NULL != response);

    AwaPathIterator * iterator = AwaClientSetResponse_NewPathIterator(response);
    ASSERT_TRUE(NULL != iterator);

    // should be three paths
    std::vector<std::string> expectedPaths = { "/3/0/14", "/3/0/15" };
    std::vector<std::string> actualPaths;
    while (AwaPathIterator_Next(iterator))
    {
        const char * path = AwaPathIterator_Get(iterator);
        actualPaths.push_back(path);
    }

    EXPECT_EQ(expectedPaths.size(), actualPaths.size());
    if (expectedPaths.size() == actualPaths.size())
        EXPECT_TRUE(std::is_permutation(expectedPaths.begin(), expectedPaths.end(), actualPaths.begin()));

    AwaPathIterator_Free(&iterator);
    AwaClientSetOperation_Free(&operation);
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetResponse_GetPathResult_handles_success)
{
    // now try to Set the mandatory resource:
    AwaClientSetOperation * operation = AwaClientSetOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsCString(operation, "/3/0/14", "hello"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(operation, defaults::timeout));

    // check response on /3/0/11
    const AwaClientSetResponse * response = AwaClientSetOperation_GetResponse(operation);
    EXPECT_TRUE(NULL != response);

    const AwaPathResult * pathResult = AwaClientSetResponse_GetPathResult(response, "/3/0/14");
    EXPECT_TRUE(NULL != pathResult);
    EXPECT_EQ(AwaError_Success, AwaPathResult_GetError(pathResult));

    AwaClientSetOperation_Free(&operation);
}

TEST_F(TestSetOperationWithConnectedSession, AwaClientSetResponse_GetPathResult_handles_error)
{
    // now try to Set the mandatory resource:
    AwaClientSetOperation * operation = AwaClientSetOperation_New(session_);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsCString(operation, "/3/2/14", "hello"));
    EXPECT_EQ(AwaError_Response, AwaClientSetOperation_Perform(operation, defaults::timeout));

    // check response on /3/0/11
    const AwaClientSetResponse * response = AwaClientSetOperation_GetResponse(operation);
    EXPECT_TRUE(NULL != response);

    const AwaPathResult * pathResult = AwaClientSetResponse_GetPathResult(response, "/3/2/14");
    EXPECT_TRUE(NULL != pathResult);
    EXPECT_EQ(AwaError_PathInvalid, AwaPathResult_GetError(pathResult));

    AwaClientSetOperation_Free(&operation);
}

} // namespace Awa


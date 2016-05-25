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

// TODO: Test subscribe to multiple objects in a single request, then cancel one of the subscriptions without the others being broken

#include <gtest/gtest.h>

#include <lwm2m_tree_node.h>

#include "awa/client.h"
#include "awa/common.h"
#include "memalloc.h"
#include "log.h"
#include "arrays.h"
#include "support/support.h"
#include "client_subscribe.h"
#include "client_session.h"
#include "utils.h"

namespace Awa {

class TestSubscribe : public TestClientBase {};

class TestSubscribeToChangeWithConnectedSession : public TestClientWithConnectedSession 
{
public:
    virtual void callbackHandler(const AwaChangeSet * changeSet) {};
};


void (ChangeCallbackRunner)(const AwaChangeSet * changeSet, void * context)
{
    if (context)
    {
        auto * that = static_cast<TestSubscribeToChangeWithConnectedSession*>(context);
        that->callbackHandler(changeSet);
    }
}


void (EmptyChangeCallback)(const AwaChangeSet * changeSet, void * context)
{
}

void EmptyExecuteCallback(const AwaExecuteArguments * arguments, void * context)
{
}


TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeOperation_New_free_valid_inputs)
{
    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeOperation_New_free_invalid_inputs)
{
    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(NULL);
    ASSERT_TRUE(NULL == operation);
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientSubscribeOperation_Free(&operation));
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientSubscribeOperation_Free(NULL));
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientChangeSubscription_New_free_valid_inputs)
{
    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    AwaClientChangeSubscription * observation = AwaClientChangeSubscription_New("/3/0/1", EmptyChangeCallback, NULL);
    ASSERT_TRUE(NULL != observation);
    ASSERT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&observation));
    ASSERT_TRUE(NULL == observation);
    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientChangeSubscription_New_free_invalid_inputs)
{
    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session_);
    EXPECT_TRUE(NULL != operation);

    EXPECT_TRUE(NULL == AwaClientChangeSubscription_New(NULL, EmptyChangeCallback, NULL));
    EXPECT_TRUE(NULL == AwaClientChangeSubscription_New("/3/0/1", NULL, NULL));

    AwaClientChangeSubscription * observation = NULL;
    EXPECT_EQ(AwaError_SubscriptionInvalid, AwaClientChangeSubscription_Free(&observation));
    EXPECT_EQ(AwaError_SubscriptionInvalid, AwaClientChangeSubscription_Free(NULL));

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    EXPECT_TRUE(NULL == operation);
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeOperation_AddRemoveChangeSubscription_valid_inputs)
{
    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaClientChangeSubscription * changeSubscription = AwaClientChangeSubscription_New("/3/0/1", EmptyChangeCallback, NULL);

    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddChangeSubscription(operation, changeSubscription));

    ASSERT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&changeSubscription));
    ASSERT_TRUE(NULL == changeSubscription);
    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeOperation_AddCancelChangeSubscription_null_inputs)
{
    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    ASSERT_EQ(AwaError_SubscriptionInvalid, AwaClientSubscribeOperation_AddChangeSubscription(operation, NULL));
    ASSERT_EQ(AwaError_SubscriptionInvalid, AwaClientSubscribeOperation_AddCancelChangeSubscription(operation, NULL));

    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);

    ASSERT_EQ(AwaError_OperationInvalid, AwaClientSubscribeOperation_AddChangeSubscription(operation, NULL));
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientSubscribeOperation_AddCancelChangeSubscription(operation, NULL));

    AwaClientChangeSubscription * changeSubscription = AwaClientChangeSubscription_New("/3/0/1", EmptyChangeCallback, NULL);

    ASSERT_EQ(AwaError_OperationInvalid, AwaClientSubscribeOperation_AddChangeSubscription(NULL, changeSubscription));
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientSubscribeOperation_AddCancelChangeSubscription(NULL, changeSubscription));

    ASSERT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&changeSubscription));
    ASSERT_TRUE(NULL == changeSubscription);
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeOperation_AddCancelExecuteSubscription_null_inputs)
{
    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    ASSERT_EQ(AwaError_SubscriptionInvalid, AwaClientSubscribeOperation_AddExecuteSubscription(operation, NULL));
    ASSERT_EQ(AwaError_SubscriptionInvalid, AwaClientSubscribeOperation_AddCancelExecuteSubscription(operation, NULL));

    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);

    ASSERT_EQ(AwaError_OperationInvalid, AwaClientSubscribeOperation_AddExecuteSubscription(operation, NULL));
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientSubscribeOperation_AddCancelExecuteSubscription(operation, NULL));

    AwaClientExecuteSubscription * executeSubscription = AwaClientExecuteSubscription_New("/3/0/4", EmptyExecuteCallback, NULL);

    ASSERT_EQ(AwaError_OperationInvalid, AwaClientSubscribeOperation_AddExecuteSubscription(NULL, executeSubscription));
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientSubscribeOperation_AddCancelExecuteSubscription(NULL, executeSubscription));

    ASSERT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&executeSubscription));
    ASSERT_TRUE(NULL == executeSubscription);
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeOperation_AddChangeSubscription_valid_inputs)
{
    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaClientChangeSubscription * changeSubscription = AwaClientChangeSubscription_New("/3/0/1", EmptyChangeCallback, NULL);

    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddChangeSubscription(operation, changeSubscription));

    ASSERT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&changeSubscription));
    ASSERT_TRUE(NULL == changeSubscription);

    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}



TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeOperation_AddSubscription_duplicate_path)
{
    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaClientChangeSubscription * changeSubscription = AwaClientChangeSubscription_New("/3/0/1", EmptyChangeCallback, NULL);

    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddChangeSubscription(operation, changeSubscription));
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientSubscribeOperation_AddChangeSubscription(operation, changeSubscription));
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientSubscribeOperation_AddCancelChangeSubscription(operation, changeSubscription));

    ASSERT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&changeSubscription));
    ASSERT_TRUE(NULL == changeSubscription);

    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeOperation_Perform_handles_invalid_inputs)
{
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientSubscribeOperation_Perform(NULL, 0));

    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientSubscribeOperation_Perform(NULL, -1));

    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
    ASSERT_EQ(AwaError_OperationInvalid, AwaClientSubscribeOperation_Perform(operation, 0));
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeOperation_Perform_handles_valid_operation_with_callback)
{
    struct ChangeCallbackHandler1 : public TestSubscribeToChangeWithConnectedSession
    {
        int count;

        ChangeCallbackHandler1() : count(0) {}

        void callbackHandler(const AwaChangeSet * changeSet)
        {
            count ++;

            ASSERT_TRUE(NULL != changeSet);
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3"));
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3/0"));
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3/0/16"));
            EXPECT_FALSE(AwaChangeSet_ContainsPath(changeSet, "/3000/0/0"));
            EXPECT_TRUE(AwaChangeSet_HasValue(changeSet, "/3/0/16"));
            EXPECT_FALSE(AwaChangeSet_HasValue(changeSet, "/3000/0/0"));

            const char * value = NULL;
            AwaChangeSet_GetValueAsCStringPointer(changeSet, "/3/0/16", &value);
            ASSERT_TRUE(NULL != value);
            EXPECT_STREQ("123414123", value);

            // TODO: need to add support for the other cases
            EXPECT_EQ(AwaChangeType_ResourceModified, AwaChangeSet_GetChangeType(changeSet, "/3/0/16"));
        }
        void TestBody() {}
    };
    ChangeCallbackHandler1 cbHandler;

    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaClientChangeSubscription * changeSubscription = AwaClientChangeSubscription_New("/3/0/16", ChangeCallbackRunner, &cbHandler);

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddChangeSubscription(operation, changeSubscription));

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Perform(operation, defaults::timeout));

    // set via client api to trigger notification.
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsCString(setOperation, "/3/0/16", "123414123"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    EXPECT_EQ(AwaError_Success, AwaClientSession_Process(session_, defaults::timeout));

    cbHandler.count = 0;
    AwaClientSession_DispatchCallbacks(session_);
    EXPECT_EQ(1, cbHandler.count);

    EXPECT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&changeSubscription));
    EXPECT_TRUE(NULL == changeSubscription);

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    EXPECT_TRUE(NULL == operation);
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeOperation_ChangeSet_handles_wrong_session_type)
{
    struct ChangeCallbackHandler1 : public TestSubscribeToChangeWithConnectedSession
    {
        int count;

        ChangeCallbackHandler1() : count(0) {}

        void callbackHandler(const AwaChangeSet * changeSet)
        {
            count ++;

            const AwaClientSession * clientSession = AwaChangeSet_GetClientSession(changeSet);
            EXPECT_TRUE(NULL != clientSession);
            const AwaServerSession * serverSession = AwaChangeSet_GetServerSession(changeSet);
            EXPECT_TRUE(NULL == serverSession);
        }
        void TestBody() {}
    };
    ChangeCallbackHandler1 cbHandler;

    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaClientChangeSubscription * changeSubscription = AwaClientChangeSubscription_New("/3/0/16", ChangeCallbackRunner, &cbHandler);

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddChangeSubscription(operation, changeSubscription));

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Perform(operation, defaults::timeout));

    // set via client api to trigger notification.
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsCString(setOperation, "/3/0/16", "123414123"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    EXPECT_EQ(AwaError_Success, AwaClientSession_Process(session_, defaults::timeout));

    cbHandler.count = 0;
    AwaClientSession_DispatchCallbacks(session_);
    EXPECT_EQ(1, cbHandler.count);

    EXPECT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&changeSubscription));
    EXPECT_TRUE(NULL == changeSubscription);

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    EXPECT_TRUE(NULL == operation);
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeOperation_Perform_handles_valid_operation_on_multiple_instance_resource_with_callback)
{
    struct ChangeCallbackHandler1 : public TestSubscribeToChangeWithConnectedSession
    {
        int count;

        ChangeCallbackHandler1() : count(0) {}

        void callbackHandler(const AwaChangeSet * changeSet)
        {
            count ++;

            ASSERT_TRUE(NULL != changeSet);
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3"));
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3/0"));
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3/0/6"));
            EXPECT_FALSE(AwaChangeSet_ContainsPath(changeSet, "/3000/0/0"));
            EXPECT_TRUE(AwaChangeSet_HasValue(changeSet, "/3/0/6"));
            EXPECT_FALSE(AwaChangeSet_HasValue(changeSet, "/3000/0/0"));

            // TODO: need to add support for the other cases
            EXPECT_EQ(AwaChangeType_ResourceModified, AwaChangeSet_GetChangeType(changeSet, "/3/0/6"));
            const AwaIntegerArray * array = NULL;
            AwaChangeSet_GetValuesAsIntegerArrayPointer(changeSet, "/3/0/6", &array);
            EXPECT_TRUE(NULL != array);
            EXPECT_EQ(1234, AwaIntegerArray_GetValue(array, 0));
        }
        void TestBody() {}
    };
    ChangeCallbackHandler1 cbHandler;

    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaClientChangeSubscription * changeSubscription = AwaClientChangeSubscription_New("/3/0/6", ChangeCallbackRunner, &cbHandler);

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddChangeSubscription(operation, changeSubscription));

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Perform(operation, defaults::timeout));

    // set via client api to trigger notification.
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddArrayValueAsInteger(setOperation, "/3/0/6", 0, 1234));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    EXPECT_EQ(AwaError_Success, AwaClientSession_Process(session_, defaults::timeout));

    cbHandler.count = 0;
    AwaClientSession_DispatchCallbacks(session_);
    EXPECT_EQ(1, cbHandler.count);

    EXPECT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&changeSubscription));
    EXPECT_TRUE(NULL == changeSubscription);

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    EXPECT_TRUE(NULL == operation);
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeOperation_Perform_free_handles_operation_freed_after_subscription)
{
    struct ChangeCallbackHandler1 : public TestSubscribeToChangeWithConnectedSession
    {
        int count;

        ChangeCallbackHandler1() : count(0) {}

        void callbackHandler(const AwaChangeSet * changeSet)
        {
            count ++;
        }
        void TestBody() {}
    };
    ChangeCallbackHandler1 cbHandler;

    AwaClientSubscribeOperation * subscribeOperation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != subscribeOperation);

    AwaClientChangeSubscription * changeSubscription = AwaClientChangeSubscription_New("/3/0/16", ChangeCallbackRunner, &cbHandler);

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddChangeSubscription(subscribeOperation, changeSubscription));
    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Perform(subscribeOperation, defaults::timeout));

    // set via client api to trigger notification.
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsCString(setOperation, "/3/0/16", "123414123"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    EXPECT_EQ(AwaError_Success, AwaClientSession_Process(session_, defaults::timeout));

    cbHandler.count = 0;
    AwaClientSession_DispatchCallbacks(session_);
    EXPECT_EQ(1, cbHandler.count);

    EXPECT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&changeSubscription));
    EXPECT_TRUE(NULL == changeSubscription);

    EXPECT_EQ(0u, Map_Length(ClientSubscribeOperation_GetSubscribers(subscribeOperation)));

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&subscribeOperation));
    EXPECT_TRUE(NULL == subscribeOperation);


}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeOperation_Perform_free_handles_subscription_freed_after_operation)
{
    struct ChangeCallbackHandler1 : public TestSubscribeToChangeWithConnectedSession
    {
        int count;

        ChangeCallbackHandler1() : count(0) {}

        void callbackHandler(const AwaChangeSet * changeSet)
        {
            count ++;
        }
        void TestBody() {}
    };
    ChangeCallbackHandler1 cbHandler;

    AwaClientSubscribeOperation * subscribeOperation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != subscribeOperation);

    AwaClientChangeSubscription * changeSubscription = AwaClientChangeSubscription_New("/3/0/16", ChangeCallbackRunner, &cbHandler);

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddChangeSubscription(subscribeOperation, changeSubscription));
    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Perform(subscribeOperation, defaults::timeout));

    // set via client api to trigger notification.
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsCString(setOperation, "/3/0/16", "123414123"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    EXPECT_EQ(AwaError_Success, AwaClientSession_Process(session_, defaults::timeout));

    cbHandler.count = 0;
    AwaClientSession_DispatchCallbacks(session_);
    EXPECT_EQ(1, cbHandler.count);

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&subscribeOperation));
    EXPECT_TRUE(NULL == subscribeOperation);

    EXPECT_EQ(0u, List_Length(ClientSubscription_GetOperations(changeSubscription)));

    EXPECT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&changeSubscription));
    EXPECT_TRUE(NULL == changeSubscription);
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeOperation_Perform_free_handles_subscription_freed_before_perform)
{
    AwaClientSubscribeOperation * subscribeOperation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != subscribeOperation);

    AwaClientChangeSubscription * changeSubscription1 = AwaClientChangeSubscription_New("/3/0/15", EmptyChangeCallback, NULL);
    AwaClientChangeSubscription * changeSubscription2 = AwaClientChangeSubscription_New("/3/0/16", EmptyChangeCallback, NULL);

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddChangeSubscription(subscribeOperation, changeSubscription1));
    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddChangeSubscription(subscribeOperation, changeSubscription2));

    EXPECT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&changeSubscription2));
    EXPECT_TRUE(NULL == changeSubscription2);

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Perform(subscribeOperation, defaults::timeout));

    const AwaClientSubscribeResponse * response = AwaClientSubscribeOperation_GetResponse(subscribeOperation);
    ASSERT_TRUE(response != NULL);

    EXPECT_TRUE(AwaClientSubscribeResponse_GetPathResult(response, "/3/0/15") != NULL);
    EXPECT_TRUE(AwaClientSubscribeResponse_GetPathResult(response, "/3/0/16") == NULL);

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&subscribeOperation));
    EXPECT_TRUE(NULL == subscribeOperation);

    EXPECT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&changeSubscription1));
    EXPECT_TRUE(NULL == changeSubscription1);
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeOperation_subscription_handles_multiple_operations)
{
    struct ChangeCallbackHandler1 : public TestSubscribeToChangeWithConnectedSession
    {
        int count;

        ChangeCallbackHandler1() : count(0) {}

        void callbackHandler(const AwaChangeSet * changeSet)
        {
            count ++;
        }
        void TestBody() {}
    };
    ChangeCallbackHandler1 cbHandler;

    AwaClientChangeSubscription * changeSubscription = AwaClientChangeSubscription_New("/3/0/16", ChangeCallbackRunner, &cbHandler);


    // Create a successful subscribe operation, write and wait for a notify
    AwaClientSubscribeOperation * subscribeOperation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != subscribeOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddChangeSubscription(subscribeOperation, changeSubscription));
    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Perform(subscribeOperation, defaults::timeout));
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsCString(setOperation, "/3/0/16", "123414123"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSession_Process(session_, defaults::timeout));
    cbHandler.count = 0;
    AwaClientSession_DispatchCallbacks(session_);
    EXPECT_EQ(1, cbHandler.count);
    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&subscribeOperation));
    EXPECT_TRUE(NULL == subscribeOperation);

    // Create a cancel subscribe operation.
    AwaClientSubscribeOperation * cancelSubscribeOperation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != cancelSubscribeOperation);
    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddCancelChangeSubscription(cancelSubscribeOperation, changeSubscription));
    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Perform(cancelSubscribeOperation, defaults::timeout));
    AwaClientSession_DispatchCallbacks(session_);
    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&cancelSubscribeOperation));
    EXPECT_TRUE(NULL == cancelSubscribeOperation);

    EXPECT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&changeSubscription));
    EXPECT_TRUE(NULL == changeSubscription);
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeOperation_Perform_honours_timeout)
{
    struct ChangeCallbackHandler1 : public TestSubscribeToChangeWithConnectedSession
    {
        int count;

        ChangeCallbackHandler1() : count(0) {}

        void callbackHandler(const AwaChangeSet * changeSet)
        {
            count ++;

            ASSERT_TRUE(NULL != changeSet);
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3"));
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3/0"));
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3/0/16"));
            EXPECT_FALSE(AwaChangeSet_ContainsPath(changeSet, "/3000/0/0"));
            EXPECT_TRUE(AwaChangeSet_HasValue(changeSet, "/3/0/16"));
            EXPECT_FALSE(AwaChangeSet_HasValue(changeSet, "/3000/0/0"));

            // TODO: need to add support for the other cases
            EXPECT_EQ(AwaChangeType_ResourceModified, AwaChangeSet_GetChangeType(changeSet, "/3/0/16"));
        }
        void TestBody() {}
    };
    ChangeCallbackHandler1 cbHandler;

    const char * clientID = "TestClient1";

    AwaClientDaemonHorde * horde_ = new AwaClientDaemonHorde( { clientID }, 61001, CURRENT_TEST_DESCRIPTION);
    sleep(1);      // wait for the client to register with the server

    AwaClientSession * session = AwaClientSession_New();
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(session, "0.0.0.0", 61001));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(session));

    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session);
    ASSERT_TRUE(NULL != operation);
    AwaClientChangeSubscription * changeSubscription = AwaClientChangeSubscription_New("/3/0/16", ChangeCallbackRunner, &cbHandler);
    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddChangeSubscription(operation, changeSubscription));

    // Tear down client process
    delete horde_;

    BasicTimer timer;
    timer.Start();
    EXPECT_EQ(AwaError_Timeout, AwaClientSubscribeOperation_Perform(operation, defaults::timeout));
    timer.Stop();
    EXPECT_TRUE(ElapsedTimeWithinTolerance(timer.TimeElapsed_Milliseconds(), defaults::timeout, defaults::timeoutTolerance)) << "Time elapsed: " << timer.TimeElapsed_Milliseconds() << "ms";

    EXPECT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&changeSubscription));
    EXPECT_TRUE(NULL == changeSubscription);

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    EXPECT_TRUE(NULL == operation);

    AwaClientSession_Free(&session);
    EXPECT_TRUE(NULL == session);
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeOperation_Perform_handles_valid_operation_multiple_paths)
{
    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaClientChangeSubscription * changeSubscription = AwaClientChangeSubscription_New("/3/0/1", EmptyChangeCallback, NULL);
    AwaClientChangeSubscription * changeSubscription2 = AwaClientChangeSubscription_New("/3/0/0", EmptyChangeCallback, NULL);
    AwaClientExecuteSubscription * executeSubscription = AwaClientExecuteSubscription_New("/3/0/4", EmptyExecuteCallback, NULL);

    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddChangeSubscription(operation, changeSubscription));
    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddChangeSubscription(operation, changeSubscription2));
    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddExecuteSubscription(operation, executeSubscription));

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Perform(operation, defaults::timeout));

    EXPECT_EQ(3u, Map_Length(ClientSession_GetSubscribers(session_)));

    ASSERT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&changeSubscription));
    ASSERT_TRUE(NULL == changeSubscription);

    ASSERT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&changeSubscription2));
    ASSERT_TRUE(NULL == changeSubscription);

    ASSERT_EQ(AwaError_Success, AwaClientExecuteSubscription_Free(&executeSubscription));
    ASSERT_TRUE(NULL == executeSubscription);

    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeOperation_AddSubscription_add_subscription_to_non_existent_resource)
{
    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    ASSERT_EQ(0u, Map_Length(ClientSession_GetSubscribers(session_)));
    AwaClientChangeSubscription * changeSubscription = AwaClientChangeSubscription_New("/3/0/9000", EmptyChangeCallback, NULL);

    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddChangeSubscription(operation, changeSubscription));

    EXPECT_EQ(AwaError_Response, AwaClientSubscribeOperation_Perform(operation, defaults::timeout));

    const AwaClientSubscribeResponse * response = AwaClientSubscribeOperation_GetResponse(operation);
    ASSERT_TRUE(NULL != response);

    const AwaPathResult * result = AwaClientSubscribeResponse_GetPathResult(response, "/3/0/9000");
    EXPECT_EQ(AwaError_PathNotFound, AwaPathResult_GetError(result));

    EXPECT_EQ(0u, Map_Length(ClientSession_GetSubscribers(session_)));

    EXPECT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&changeSubscription));
    EXPECT_TRUE(NULL == changeSubscription);

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    EXPECT_TRUE(NULL == operation);
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeOperation_AddCancelSubscription_cancel_existing_change_subscription)
{
    // Do operation twice, first time creating the subscription, second time canceling the subscription.
    for (int i = 0; i < 2; i++)
    {
        AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session_);
        ASSERT_TRUE(NULL != operation);
        AwaClientChangeSubscription * changeSubscription = AwaClientChangeSubscription_New("/3/0/1", EmptyChangeCallback, NULL);

        if (i == 0)
            ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddChangeSubscription(operation, changeSubscription));
        else
            ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddCancelChangeSubscription(operation, changeSubscription));

        EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Perform(operation, defaults::timeout));
        const AwaClientSubscribeResponse * response = AwaClientSubscribeOperation_GetResponse(operation);
        ASSERT_TRUE(NULL != response);
        const AwaPathResult * result = AwaClientSubscribeResponse_GetPathResult(response, "/3/0/1");
        EXPECT_EQ(AwaError_Success, AwaPathResult_GetError(result));
        ASSERT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&changeSubscription));
        ASSERT_TRUE(NULL == changeSubscription);
        ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
        ASSERT_TRUE(NULL == operation);
    }
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeOperation_AddSubscription_replace_existing_subscription)
{
    // Do operation twice, second should succeed (replace)
    for (int i = 0; i < 2; i++)
    {
        AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session_);
        ASSERT_TRUE(NULL != operation);
        AwaClientChangeSubscription * changeSubscription = AwaClientChangeSubscription_New("/3/0/1", EmptyChangeCallback, NULL);
        ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddChangeSubscription(operation, changeSubscription));
        EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Perform(operation, defaults::timeout));
        const AwaClientSubscribeResponse * response = AwaClientSubscribeOperation_GetResponse(operation);
        ASSERT_TRUE(NULL != response);
        const AwaPathResult * result = AwaClientSubscribeResponse_GetPathResult(response, "/3/0/1");
        EXPECT_EQ(AwaError_Success, AwaPathResult_GetError(result));
        ASSERT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&changeSubscription));
        ASSERT_TRUE(NULL == changeSubscription);
        ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
        ASSERT_TRUE(NULL == operation);
    }
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeOperation_AddCancelSubscription_cancel_non_existing_subscription)
{
    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaClientChangeSubscription * changeSubscription = AwaClientChangeSubscription_New("/3/0/1", EmptyChangeCallback, NULL);
    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddCancelChangeSubscription(operation, changeSubscription));

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Perform(operation, defaults::timeout));
    const AwaClientSubscribeResponse * response = AwaClientSubscribeOperation_GetResponse(operation);
    ASSERT_TRUE(NULL != response);
    const AwaPathResult * result = AwaClientSubscribeResponse_GetPathResult(response, "/3/0/1");

    EXPECT_EQ(AwaError_SubscriptionInvalid, AwaPathResult_GetError(result));
    ASSERT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&changeSubscription));
    ASSERT_TRUE(NULL == changeSubscription);
    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}


TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeResponse_NewPathIterator_handles_null_response)
{
    EXPECT_EQ(NULL, AwaClientSubscribeResponse_NewPathIterator(NULL));
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeResponse_NewPathIterator_handles_valid_response)
{
    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaClientChangeSubscription * changeSubscription = AwaClientChangeSubscription_New("/3/0/0", EmptyChangeCallback, NULL);
    AwaClientChangeSubscription * changeSubscription2 = AwaClientChangeSubscription_New("/3/0/1", EmptyChangeCallback, NULL);
    AwaClientExecuteSubscription * executeSubscription = AwaClientExecuteSubscription_New("/3/0/4", EmptyExecuteCallback, NULL);

    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddChangeSubscription(operation, changeSubscription));
    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddChangeSubscription(operation, changeSubscription2));
    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddExecuteSubscription(operation, executeSubscription));

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Perform(operation, defaults::timeout));

    const AwaClientSubscribeResponse * response = AwaClientSubscribeOperation_GetResponse(operation);
    ASSERT_TRUE(NULL != response);

    AwaPathIterator * iterator = AwaClientSubscribeResponse_NewPathIterator(response);
    ASSERT_TRUE(NULL != iterator);

    //ensure the subscription paths are in our response
    EXPECT_EQ(true, AwaPathIterator_Next(iterator));
    EXPECT_STREQ("/3/0/0", AwaPathIterator_Get(iterator));

    EXPECT_EQ(true, AwaPathIterator_Next(iterator));
    EXPECT_STREQ("/3/0/1", AwaPathIterator_Get(iterator));

    EXPECT_EQ(true, AwaPathIterator_Next(iterator));
    EXPECT_STREQ("/3/0/4", AwaPathIterator_Get(iterator));

    // not expecting any more paths
    EXPECT_TRUE(false == AwaPathIterator_Next(iterator));
    EXPECT_TRUE(NULL == AwaPathIterator_Get(iterator));

    AwaPathIterator_Free(&iterator);
    EXPECT_EQ(NULL, iterator);

    ASSERT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&changeSubscription));
    ASSERT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&changeSubscription2));
    ASSERT_EQ(AwaError_Success, AwaClientExecuteSubscription_Free(&executeSubscription));
    ASSERT_TRUE(NULL == changeSubscription);
    ASSERT_TRUE(NULL == changeSubscription);
    ASSERT_TRUE(NULL == executeSubscription);

    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaChangeSubscription_New_free_valid_inputs)
{
    int stuff_to_pass;

    AwaClientChangeSubscription * subscription = AwaClientChangeSubscription_New("/3/0/1", EmptyChangeCallback, (void*)&stuff_to_pass);
    ASSERT_TRUE(NULL != subscription);
    ASSERT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&subscription));
    ASSERT_TRUE(NULL == subscription); 

    subscription = AwaClientChangeSubscription_New("/3/0/1", EmptyChangeCallback, NULL);
    ASSERT_TRUE(NULL != subscription);
    ASSERT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&subscription));
    ASSERT_TRUE(NULL == subscription); 
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaChangeSubscription_New_free_invalid)
{
    int stuff_to_pass;

    AwaClientChangeSubscription * subscription = AwaClientChangeSubscription_New(NULL, EmptyChangeCallback, (void*)&stuff_to_pass);
    ASSERT_TRUE(NULL == subscription);
    subscription = AwaClientChangeSubscription_New("/3/0/1", NULL, (void*)&stuff_to_pass);
    ASSERT_TRUE(NULL == subscription);
    ASSERT_EQ(AwaError_SubscriptionInvalid, AwaClientChangeSubscription_Free(NULL));
    AwaClientChangeSubscription ** subscription2 = NULL;
    ASSERT_EQ(AwaError_SubscriptionInvalid, AwaClientChangeSubscription_Free(subscription2));
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaChangeSubscription_GetPath_valid_invalid)
{
    ASSERT_TRUE(NULL == AwaClientChangeSubscription_GetPath(NULL));
    AwaClientChangeSubscription * subscription = AwaClientChangeSubscription_New("/3/0/4", EmptyChangeCallback, NULL);
    ASSERT_TRUE(NULL != subscription);
    ASSERT_STREQ("/3/0/4", AwaClientChangeSubscription_GetPath(subscription));
    ASSERT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&subscription));
    ASSERT_TRUE(NULL == subscription);
    ASSERT_TRUE(NULL == AwaClientChangeSubscription_GetPath(subscription));
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSession_Process_nothing_pending)
{
    ASSERT_EQ(AwaError_Success, AwaClientSession_Process(session_, defaults::timeout));
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSession_DispatchCallbacks_nothing_in_queue)
{
    AwaClientSession_DispatchCallbacks(session_);
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaChangeSet_ContainsPath_invalid_input)
{
    EXPECT_FALSE(AwaChangeSet_ContainsPath(NULL, NULL));
    EXPECT_FALSE(AwaChangeSet_ContainsPath(NULL, "/3/2/1"));
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaChangeSet_HasValue_invalid_input)
{
    EXPECT_FALSE(AwaChangeSet_HasValue(NULL, NULL));
    EXPECT_FALSE(AwaChangeSet_HasValue(NULL, "/3/2/1"));
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaChangeSet_GetChangeType_invalid_input)
{
    EXPECT_EQ(AwaChangeType_Invalid, AwaChangeSet_GetChangeType(NULL, NULL));
    EXPECT_EQ(AwaChangeType_Invalid, AwaChangeSet_GetChangeType(NULL, "/3/2/1"));
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSession_AwaChangeSet_NewPathIterator_invalid_inputs)
{
    EXPECT_EQ(NULL, AwaChangeSet_NewPathIterator(NULL));
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSession_AwaChangeSet_NewPathIterator_valid_inputs)
{
    struct ChangeCallbackHandler2 : public TestSubscribeToChangeWithConnectedSession
    {
        int count;

        ChangeCallbackHandler2() : count(0) {}

        void callbackHandler(const AwaChangeSet * changeSet)
        {
            count ++;

            ASSERT_TRUE(NULL != changeSet);
            AwaPathIterator * iterator = AwaChangeSet_NewPathIterator(changeSet);
            ASSERT_TRUE(NULL != iterator);

            // FIXME: the changeset should only contain paths to resources/objects that have changed!
            // call Next once before getting values:
            //EXPECT_EQ(true, AwaPathIterator_Next(iterator));
            //EXPECT_STREQ("/3/0/16", AwaPathIterator_Get(iterator));

            // not expecting any more paths
            //EXPECT_TRUE(false == AwaPathIterator_Next(iterator));
            //EXPECT_TRUE(NULL == AwaPathIterator_Get(iterator));

            AwaPathIterator_Free(&iterator);

        }
        void TestBody() {}
    };
    ChangeCallbackHandler2 cbHandler;

    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaClientChangeSubscription * changeSubscription = AwaClientChangeSubscription_New("/3/0/16", ChangeCallbackRunner, &cbHandler);

    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddChangeSubscription(operation, changeSubscription));

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Perform(operation, defaults::timeout));

    // set via client api to trigger notification.
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsCString(setOperation, "/3/0/16", "123414123"));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSession_Process(session_, defaults::timeout));

    cbHandler.count = 0;
    AwaClientSession_DispatchCallbacks(session_);
    ASSERT_EQ(1, cbHandler.count);

    ASSERT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&changeSubscription));
    ASSERT_TRUE(NULL == changeSubscription);

    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}

TEST_F(TestSubscribeToChangeWithConnectedSession, AwaClientSubscribeOperation_Subscribe_to_multiple_instance_resource_changes)
{
    struct ChangeCallbackHandler2 : public TestSubscribeToChangeWithConnectedSession
    {
        int count;

        ChangeCallbackHandler2() : count(0) {}

        void callbackHandler(const AwaChangeSet * changeSet)
        {
            count ++;

            ASSERT_TRUE(NULL != changeSet);
            AwaPathIterator * iterator = AwaChangeSet_NewPathIterator(changeSet);
            ASSERT_TRUE(NULL != iterator);
            EXPECT_TRUE(AwaPathIterator_Next(iterator));
            EXPECT_STREQ("/3/0/6", AwaPathIterator_Get(iterator));
            EXPECT_FALSE(AwaPathIterator_Next(iterator));

            const AwaIntegerArray * valueArray = NULL;
            AwaChangeSet_GetValuesAsIntegerArrayPointer(changeSet, "/3/0/6", &valueArray);
            ASSERT_TRUE(NULL != valueArray);
            EXPECT_EQ(12345, AwaIntegerArray_GetValue(valueArray, 0));
            EXPECT_EQ(54321, AwaIntegerArray_GetValue(valueArray, 1));

            AwaPathIterator_Free(&iterator);

        }
        void TestBody() {}
    };
    ChangeCallbackHandler2 cbHandler;

    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaClientChangeSubscription * changeSubscription = AwaClientChangeSubscription_New("/3/0/6", ChangeCallbackRunner, &cbHandler);

    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddChangeSubscription(operation, changeSubscription));

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Perform(operation, defaults::timeout));

    // set via client api to trigger notification.
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddArrayValueAsInteger(setOperation, "/3/0/6", 0, 12345));
    ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddArrayValueAsInteger(setOperation, "/3/0/6", 1, 54321));
    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSession_Process(session_, defaults::timeout));

    cbHandler.count = 0;
    AwaClientSession_DispatchCallbacks(session_);
    ASSERT_EQ(1, cbHandler.count);


    ASSERT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&changeSubscription));
    ASSERT_TRUE(NULL == changeSubscription);

    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}



// Test ChangeSet functions
//------------------------------
namespace changeSetDetail
{

struct TestChangeSetResource
{
    AwaError expectedResult;
    const void * expectedValue;
    int expectedValueSize;
    const char * path;
    AwaResourceType type;
};

::std::ostream& operator<<(::std::ostream& os, const TestChangeSetResource& item)
{
  return os << "Item: expectedResult " << item.expectedResult
            << ", expectedValue " << item.expectedValue
            << ", expectedValueSize " << item.expectedValueSize
            << ", path " << item.path
            << ", type " << item.type;
}

// initial object values
const AwaInteger initialInteger1 = 100;
const char * initialString1 = "Lightweight M2M Client";
const AwaFloat initialFloat1 = 1.337;
const AwaTime initialTime1 = 0xA20AD72B;
const AwaBoolean initialBoolean1 = true;
const char dummyOpaqueData2[] = {'a',0,'x','\0', 123};
const AwaOpaque initialOpaque1 = {(void*) dummyOpaqueData2, sizeof(dummyOpaqueData2)};
const AwaObjectLink initialObjectLink1 = { 3, 0 };

// change to this to trigger notification.
const AwaInteger expectedInteger1 = 150;
const char * expectedString1 = "Heavyweight M2M Client";
const AwaFloat expectedFloat1 = 7.331;
const AwaTime expectedTime1 = 0xB67AD72B;
const AwaBoolean expectedBoolean1 = false;

const char dummyOpaqueData[] = {'c',0,'d','\0', 124};
const AwaOpaque expectedOpaque1 = {(void*) dummyOpaqueData, sizeof(dummyOpaqueData)};
const AwaObjectLink expectedObjectLink1 = { 4, 0 };

} // namespace detail

class TestAwaChangeSet : public TestSubscribeToChangeWithConnectedSession, public ::testing::WithParamInterface< changeSetDetail::TestChangeSetResource>
{
protected:

    void SetUp() {
        TestSubscribeToChangeWithConnectedSession::SetUp();

        AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
        EXPECT_TRUE(defineOperation != NULL);

        AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(10000, "Test Object", 0, 1);
        ASSERT_TRUE(NULL != customObjectDefinition);

        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsNoType(customObjectDefinition, 0, "Test None Resource", false, AwaResourceOperations_Execute));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsString(customObjectDefinition, 1, "Test String Resource", true, AwaResourceOperations_ReadWrite, changeSetDetail::expectedString1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(customObjectDefinition, 2, "Test Integer Resource", true, AwaResourceOperations_ReadWrite, changeSetDetail::expectedInteger1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloat(customObjectDefinition, 3, "Test Float Resource", true, AwaResourceOperations_ReadWrite, changeSetDetail::expectedFloat1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBoolean(customObjectDefinition, 4, "Test Boolean Resource", true, AwaResourceOperations_ReadWrite, changeSetDetail::expectedBoolean1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque(customObjectDefinition, 5, "Test Opaque Resource", true, AwaResourceOperations_ReadWrite, changeSetDetail::expectedOpaque1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTime(customObjectDefinition, 6, "Test Time Resource", true, AwaResourceOperations_ReadWrite, changeSetDetail::expectedTime1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLink(customObjectDefinition, 7, "Test ObjectLink Resource", true, AwaResourceOperations_ReadWrite, changeSetDetail::expectedObjectLink1));

        // define another object that we can use for "path not in result" tests
        AwaObjectDefinition * customObjectDefinition2 = AwaObjectDefinition_New(10001, "Test Object 2", 0, 1);
        ASSERT_TRUE(NULL != customObjectDefinition2);

        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsNoType(customObjectDefinition2, 0, "Test None Resource", false, AwaResourceOperations_Execute));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsString(customObjectDefinition2, 1, "Test String Resource", true, AwaResourceOperations_ReadWrite, changeSetDetail::expectedString1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(customObjectDefinition2, 2, "Test Integer Resource", true, AwaResourceOperations_ReadWrite, changeSetDetail::expectedInteger1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloat(customObjectDefinition2, 3, "Test Float Resource", true, AwaResourceOperations_ReadWrite, changeSetDetail::expectedFloat1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBoolean(customObjectDefinition2, 4, "Test Boolean Resource", true, AwaResourceOperations_ReadWrite, changeSetDetail::expectedBoolean1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque(customObjectDefinition2, 5, "Test Opaque Resource", true, AwaResourceOperations_ReadWrite, changeSetDetail::expectedOpaque1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTime(customObjectDefinition2, 6, "Test Time Resource", true, AwaResourceOperations_ReadWrite, changeSetDetail::expectedTime1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLink(customObjectDefinition2, 7, "Test ObjectLink Resource", true, AwaResourceOperations_ReadWrite, changeSetDetail::expectedObjectLink1));

        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition2));
        ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

        AwaObjectDefinition_Free(&customObjectDefinition);
        AwaObjectDefinition_Free(&customObjectDefinition2);
        AwaClientDefineOperation_Free(&defineOperation);

        //Create a basic set operation to create our custom objects - TODO remove once we can set default values
        AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
        ASSERT_TRUE(NULL != setOperation);

        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));

        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsCString(setOperation, "/10000/0/1", changeSetDetail::initialString1));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsInteger(setOperation, "/10000/0/2", changeSetDetail::initialInteger1));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsFloat(setOperation, "/10000/0/3", changeSetDetail::initialFloat1));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsBoolean(setOperation, "/10000/0/4", changeSetDetail::initialBoolean1));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsOpaque(setOperation, "/10000/0/5", changeSetDetail::initialOpaque1));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsTime(setOperation, "/10000/0/6", changeSetDetail::initialTime1));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsObjectLink(setOperation, "/10000/0/7", changeSetDetail::initialObjectLink1));

        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
        AwaClientSetOperation_Free(&setOperation);
    }

    void TearDown() {
        TestSubscribeToChangeWithConnectedSession::TearDown();
    }

};

TEST_P(TestAwaChangeSet, TestAwaChangeSetInstantiation)
{
    changeSetDetail::TestChangeSetResource data = GetParam();

    struct ChangeCallbackHandler3 : public TestSubscribeToChangeWithConnectedSession
    {
        int count;
        changeSetDetail::TestChangeSetResource data;

        explicit ChangeCallbackHandler3(changeSetDetail::TestChangeSetResource data) : count(0), data(data) {}

        void callbackHandler(const AwaChangeSet * changeSet)
        {
            void * value = NULL;
            AwaObjectLink receivedObjectLink = {0, 0};
            AwaOpaque receivedOpaque = {NULL, 0};

            count ++;

            ASSERT_TRUE(NULL != changeSet);

            switch(data.type)
            {
                case AwaResourceType_String:
                    ASSERT_EQ(data.expectedResult, AwaChangeSet_GetValueAsCStringPointer(changeSet, data.path, (const char **)&value));
                    break;
                case AwaResourceType_Integer:
                    ASSERT_EQ(data.expectedResult, AwaChangeSet_GetValueAsIntegerPointer(changeSet, data.path, (const AwaInteger **)&value));
                    break;
                case AwaResourceType_Float:
                    ASSERT_EQ(data.expectedResult, AwaChangeSet_GetValueAsFloatPointer(changeSet, data.path, (const AwaFloat **)&value));
                    break;
                case AwaResourceType_Boolean:
                    ASSERT_EQ(data.expectedResult, AwaChangeSet_GetValueAsBooleanPointer(changeSet, data.path, (const AwaBoolean **)&value));
                    break;
                case AwaResourceType_Opaque:
                    ASSERT_EQ(data.expectedResult, AwaChangeSet_GetValueAsOpaque(changeSet, data.path, &receivedOpaque));
                    ASSERT_EQ(data.expectedResult, AwaChangeSet_GetValueAsOpaquePointer(changeSet, data.path, (const AwaOpaque **)&value));
                    break;
                case AwaResourceType_Time:
                    ASSERT_EQ(data.expectedResult, AwaChangeSet_GetValueAsTimePointer(changeSet, data.path, (const AwaTime **)&value));
                    break;
                case AwaResourceType_ObjectLink:
                    ASSERT_EQ(data.expectedResult, AwaChangeSet_GetValueAsObjectLink(changeSet, data.path, &receivedObjectLink));
                    ASSERT_EQ(data.expectedResult, AwaChangeSet_GetValueAsObjectLinkPointer(changeSet, data.path, (const AwaObjectLink **)&value));
                    break;
                default:
                    ASSERT_TRUE(false);
                    break;
            }

            if (data.expectedResult == AwaError_Success)
            {
                ASSERT_TRUE(NULL != value);

                EXPECT_EQ(data.type, AwaChangeSet_GetResourceType(changeSet, data.path));

                switch(data.type)
                {
                    case AwaResourceType_String:
                    {
                        ASSERT_STREQ((char*) data.expectedValue, (char*) data.expectedValue);
                        break;
                    }
                    case AwaResourceType_Opaque:
                    {
                        AwaOpaque * expectedOpaque = (AwaOpaque *) data.expectedValue;
                        AwaOpaque * receivedOpaquePointer = (AwaOpaque *) value;
                        ASSERT_EQ(expectedOpaque->Size, receivedOpaquePointer->Size);
                        ASSERT_EQ(expectedOpaque->Size, receivedOpaque.Size);
                        ASSERT_EQ(0, memcmp(expectedOpaque->Data, receivedOpaquePointer->Data, expectedOpaque->Size));
                        ASSERT_EQ(0, memcmp(expectedOpaque->Data, receivedOpaque.Data, receivedOpaque.Size));
                        break;
                    }
                    case AwaResourceType_Integer:
                        ASSERT_EQ(*static_cast<const AwaInteger *>(data.expectedValue), *static_cast<AwaInteger *>(value));
                        break;
                    case AwaResourceType_Float:
                        ASSERT_EQ(*static_cast<const AwaFloat *>(data.expectedValue), *static_cast<AwaFloat *>(value));
                        break;
                    case AwaResourceType_Boolean:
                        ASSERT_EQ(*static_cast<const AwaBoolean *>(data.expectedValue), *static_cast<AwaBoolean *>(value));
                        break;
                    case AwaResourceType_Time:
                        ASSERT_EQ(*static_cast<const AwaTime *>(data.expectedValue), *static_cast<AwaTime *>(value));
                        break;
                    case AwaResourceType_ObjectLink:
                    {
                        const AwaObjectLink * expectedObjectLink = static_cast<const AwaObjectLink *>(data.expectedValue);
                        const AwaObjectLink * receivedObjectLinkPointer = static_cast<AwaObjectLink *>(value);
                        ASSERT_EQ(0, memcmp(expectedObjectLink, receivedObjectLinkPointer, sizeof(AwaObjectLink)));
                        ASSERT_EQ(0, memcmp(expectedObjectLink, &receivedObjectLink, sizeof(AwaObjectLink)));
                        break;
                    }
                    default:
                        ASSERT_TRUE(false);
                        break;
                }
            }
        }
        void TestBody() {}
    };

    ChangeCallbackHandler3 cbHandler(data);

    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    AwaClientChangeSubscription * subscription = AwaClientChangeSubscription_New(data.path, ChangeCallbackRunner, &cbHandler);
    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddChangeSubscription(operation, subscription));
    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Perform(operation, defaults::timeout));

    // set via client api to trigger notification.
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    switch(data.type)
    {
        case AwaResourceType_String:
            ASSERT_EQ(data.expectedResult, AwaClientSetOperation_AddValueAsCString(setOperation, data.path, (const char *)data.expectedValue));
            break;
        case AwaResourceType_Integer:
            ASSERT_EQ(data.expectedResult, AwaClientSetOperation_AddValueAsInteger(setOperation, data.path, *static_cast<const AwaInteger *>(data.expectedValue)));
            break;
        case AwaResourceType_Float:
            ASSERT_EQ(data.expectedResult, AwaClientSetOperation_AddValueAsFloat(setOperation, data.path, *static_cast<const AwaFloat *>(data.expectedValue)));
            break;
        case AwaResourceType_Boolean:
            ASSERT_EQ(data.expectedResult, AwaClientSetOperation_AddValueAsBoolean(setOperation, data.path, *static_cast<const AwaBoolean *>(data.expectedValue)));
            break;
        case AwaResourceType_Opaque:
            ASSERT_EQ(data.expectedResult, AwaClientSetOperation_AddValueAsOpaque(setOperation, data.path, *static_cast<const AwaOpaque *>(data.expectedValue)));
            break;
        case AwaResourceType_Time:
            ASSERT_EQ(data.expectedResult, AwaClientSetOperation_AddValueAsTime(setOperation, data.path, *static_cast<const AwaTime *>(data.expectedValue)));
            break;
        case AwaResourceType_ObjectLink:
            ASSERT_EQ(data.expectedResult, AwaClientSetOperation_AddValueAsObjectLink(setOperation, data.path, *static_cast<const AwaObjectLink *>(data.expectedValue)));
            break;
        default:
            ASSERT_TRUE(false);
            break;
    }

    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);


    ASSERT_EQ(AwaError_Success, AwaClientSession_Process(session_, defaults::timeout));

    cbHandler.count = 0;
    AwaClientSession_DispatchCallbacks(session_);
    ASSERT_EQ(1, cbHandler.count);

    ASSERT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&subscription));
    ASSERT_TRUE(NULL == subscription);

    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}

INSTANTIATE_TEST_CASE_P(
        TestChangeSetResource,
        TestAwaChangeSet,
        ::testing::Values(
          changeSetDetail::TestChangeSetResource {AwaError_Success,  (void *)changeSetDetail::expectedString1, -1,                              "/10000/0/1",   AwaResourceType_String},
          changeSetDetail::TestChangeSetResource {AwaError_Success,  &changeSetDetail::expectedInteger1,       sizeof(AwaInteger),          "/10000/0/2",   AwaResourceType_Integer},
          changeSetDetail::TestChangeSetResource {AwaError_Success,  &changeSetDetail::expectedFloat1,         sizeof(AwaFloat),            "/10000/0/3",   AwaResourceType_Float},
          changeSetDetail::TestChangeSetResource {AwaError_Success,  &changeSetDetail::expectedBoolean1,       sizeof(AwaBoolean),          "/10000/0/4",   AwaResourceType_Boolean},
          changeSetDetail::TestChangeSetResource {AwaError_Success,  &changeSetDetail::expectedOpaque1,        sizeof(changeSetDetail::expectedOpaque1), "/10000/0/5",   AwaResourceType_Opaque},
          changeSetDetail::TestChangeSetResource {AwaError_Success,  &changeSetDetail::expectedTime1,          sizeof(AwaTime),             "/10000/0/6",  AwaResourceType_Time},
          changeSetDetail::TestChangeSetResource {AwaError_Success,  &changeSetDetail::expectedObjectLink1,    sizeof(AwaObjectLink),       "/10000/0/7",  AwaResourceType_ObjectLink}
        ));


/***********************************************************************************************************
 * GetValueArray parameterised tests
 */

namespace changeSetDetail
{

struct TestAwaChangeSetResourceArray
{
    AwaError expectedResult;
    const char * path;
    bool UseResponse;
    AwaResourceType type;
};

::std::ostream& operator<<(::std::ostream& os, const TestAwaChangeSetResourceArray& item)
{
  return os << "Item: expectedResult " << item.expectedResult
            << ", path " << item.path
            << ", UseResponse " << item.UseResponse
            << ", type " << item.type;
}

}

class TestAwaChangeSetArray : public TestSubscribeToChangeWithConnectedSession, public ::testing::WithParamInterface< changeSetDetail::TestAwaChangeSetResourceArray>
{
protected:

    void SetUp() {
        TestSubscribeToChangeWithConnectedSession::SetUp();

        //Define and set our custom objects - TODO define in some common class for get, set etc?
        AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
        EXPECT_TRUE(defineOperation != NULL);

        AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(10000, "Test Object", 0, 1);
        ASSERT_TRUE(NULL != customObjectDefinition);

        expectedStringArray_ = AwaStringArray_New();
        expectedIntegerArray_ = AwaIntegerArray_New();
        expectedFloatArray_ = AwaFloatArray_New();
        expectedBooleanArray_ = AwaBooleanArray_New();
        expectedOpaqueArray_ = AwaOpaqueArray_New();
        expectedTimeArray_ = AwaTimeArray_New();
        expectedObjectLinkArray_ = AwaObjectLinkArray_New();

        for (int i = 0; i < 10; i++)
        {
            char * stringValue;
            msprintf(&stringValue, "%d", i*2);
            AwaStringArray_SetValueAsCString(expectedStringArray_, i, (const char *)stringValue);
            Awa_MemSafeFree(stringValue);

            AwaIntegerArray_SetValue(expectedIntegerArray_, i, i*2);
            AwaFloatArray_SetValue(expectedFloatArray_, i, i*2.5);
            AwaBooleanArray_SetValue(expectedBooleanArray_, i, i%2==0);

            static char dummyOpaqueData[] = {'a',0,'x','\0', (char)i};

            AwaOpaque opaqueValue = {(void*) dummyOpaqueData, sizeof(dummyOpaqueData)};

            AwaOpaqueArray_SetValue(expectedOpaqueArray_, i, opaqueValue);
            AwaTimeArray_SetValue(expectedTimeArray_, i, i*100);

            AwaObjectLink objectLinkValue = { 3, i };
            AwaObjectLinkArray_SetValue(expectedObjectLinkArray_, i, objectLinkValue);
        }

        initialStringArray_ = AwaStringArray_New();
        initialIntegerArray_ = AwaIntegerArray_New();
        initialFloatArray_ = AwaFloatArray_New();
        initialBooleanArray_ = AwaBooleanArray_New();
        initialOpaqueArray_ = AwaOpaqueArray_New();
        initialTimeArray_ = AwaTimeArray_New();
        initialObjectLinkArray_ = AwaObjectLinkArray_New();

        for (int i = 0; i < 10; i++)
        {
            char * stringValue;
            msprintf(&stringValue, "%d", i*3);
            AwaStringArray_SetValueAsCString(initialStringArray_, i, (const char *)stringValue);
            Awa_MemSafeFree(stringValue);

            AwaIntegerArray_SetValue(initialIntegerArray_, i, i*3);
            AwaFloatArray_SetValue(initialFloatArray_, i, i*1.3);
            AwaBooleanArray_SetValue(initialBooleanArray_, i, i%2!=0); // NB: Opposite of expected so that set will fire a notification

            static char dummyOpaqueData[] = {'x',0,'b','\0', (char)i};

            AwaOpaque opaqueValue = {(void*) dummyOpaqueData, sizeof(dummyOpaqueData)};

            AwaOpaqueArray_SetValue(initialOpaqueArray_, i, opaqueValue);
            AwaTimeArray_SetValue(initialTimeArray_, i, i*60);

            AwaObjectLink objectLinkValue = { 4, i };
            AwaObjectLinkArray_SetValue(initialObjectLinkArray_, i, objectLinkValue);
        }

        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsStringArray(customObjectDefinition, 1, "Test String Array Resource", 0, AWA_MAX_ID, AwaResourceOperations_ReadWrite, expectedStringArray_));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsIntegerArray(customObjectDefinition, 2, "Test Integer Array Resource", 0, AWA_MAX_ID, AwaResourceOperations_ReadWrite, expectedIntegerArray_));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloatArray(customObjectDefinition, 3, "Test Float Array Resource", 0, AWA_MAX_ID, AwaResourceOperations_ReadWrite, expectedFloatArray_));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBooleanArray(customObjectDefinition, 4, "Test Boolean Array Resource", 0, AWA_MAX_ID, AwaResourceOperations_ReadWrite, expectedBooleanArray_));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaqueArray(customObjectDefinition, 5, "Test Opaque Array Resource", 0, AWA_MAX_ID, AwaResourceOperations_ReadWrite, expectedOpaqueArray_));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTimeArray(customObjectDefinition, 6, "Test Time Array Resource", 0, AWA_MAX_ID, AwaResourceOperations_ReadWrite, expectedTimeArray_));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLinkArray(customObjectDefinition, 7, "Test ObjectLink Array Resource", 0, AWA_MAX_ID, AwaResourceOperations_ReadWrite, expectedObjectLinkArray_));


        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, customObjectDefinition));
        ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, defaults::timeout));

        AwaObjectDefinition_Free(&customObjectDefinition);
        AwaClientDefineOperation_Free(&defineOperation);

        //Create a basic set operation to create our custom objects - TODO remove setting values once we can set default values
        AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
        ASSERT_TRUE(NULL != setOperation);

        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(setOperation, "/10000/0"));

        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10000/0/1"));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10000/0/2"));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10000/0/3"));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10000/0/4"));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10000/0/5"));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10000/0/6"));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_CreateOptionalResource(setOperation, "/10000/0/7"));

        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsStringArray(setOperation, "/10000/0/1", initialStringArray_));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsIntegerArray(setOperation, "/10000/0/2", initialIntegerArray_));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsFloatArray(setOperation, "/10000/0/3", initialFloatArray_));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsBooleanArray(setOperation, "/10000/0/4", initialBooleanArray_));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsOpaqueArray(setOperation, "/10000/0/5", initialOpaqueArray_));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsTimeArray(setOperation, "/10000/0/6", initialTimeArray_));
        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsObjectLinkArray(setOperation, "/10000/0/7", initialObjectLinkArray_));

        ASSERT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
        AwaClientSetOperation_Free(&setOperation);
    }

    void TearDown()
    {
        TestSubscribeToChangeWithConnectedSession::TearDown();

        AwaStringArray_Free(&expectedStringArray_);
        AwaIntegerArray_Free(&expectedIntegerArray_);
        AwaFloatArray_Free(&expectedFloatArray_);
        AwaBooleanArray_Free(&expectedBooleanArray_);
        AwaOpaqueArray_Free(&expectedOpaqueArray_);
        AwaTimeArray_Free(&expectedTimeArray_);
        AwaObjectLinkArray_Free(&expectedObjectLinkArray_);

        AwaStringArray_Free(&initialStringArray_);
        AwaIntegerArray_Free(&initialIntegerArray_);
        AwaFloatArray_Free(&initialFloatArray_);
        AwaBooleanArray_Free(&initialBooleanArray_);
        AwaOpaqueArray_Free(&initialOpaqueArray_);
        AwaTimeArray_Free(&initialTimeArray_);
        AwaObjectLinkArray_Free(&initialObjectLinkArray_);
    }

    AwaStringArray * expectedStringArray_;
    AwaIntegerArray * expectedIntegerArray_;
    AwaFloatArray * expectedFloatArray_;
    AwaBooleanArray * expectedBooleanArray_;
    AwaOpaqueArray * expectedOpaqueArray_;
    AwaTimeArray * expectedTimeArray_;
    AwaObjectLinkArray * expectedObjectLinkArray_;

    AwaStringArray * initialStringArray_;
    AwaIntegerArray * initialIntegerArray_;
    AwaFloatArray * initialFloatArray_;
    AwaBooleanArray * initialBooleanArray_;
    AwaOpaqueArray * initialOpaqueArray_;
    AwaTimeArray * initialTimeArray_;
    AwaObjectLinkArray * initialObjectLinkArray_;
};

TEST_P(TestAwaChangeSetArray, TestAwaChangeSetArrayInstantiation)
{
    changeSetDetail::TestAwaChangeSetResourceArray data = GetParam();

    struct ChangeCallbackHandler4 : public TestSubscribeToChangeWithConnectedSession
    {
        int count;
        changeSetDetail::TestAwaChangeSetResourceArray data;
        AwaStringArray * expectedStringArray_;
        AwaIntegerArray * expectedIntegerArray_;
        AwaFloatArray * expectedFloatArray_;
        AwaBooleanArray * expectedBooleanArray_;
        AwaOpaqueArray * expectedOpaqueArray_;
        AwaTimeArray * expectedTimeArray_;
        AwaObjectLinkArray * expectedObjectLinkArray_;

        explicit ChangeCallbackHandler4(changeSetDetail::TestAwaChangeSetResourceArray data)
        : count(0),
          data(data),
          expectedStringArray_(nullptr),
          expectedIntegerArray_(nullptr),
          expectedFloatArray_(nullptr),
          expectedBooleanArray_(nullptr),
          expectedOpaqueArray_(nullptr),
          expectedTimeArray_(nullptr),
          expectedObjectLinkArray_(nullptr) {}

        void callbackHandler(const AwaChangeSet * changeSet)
        {
            AwaArray * expectedArray = NULL;
            AwaArray * array = NULL;

            count ++;

            ASSERT_TRUE(NULL != changeSet);

            switch(data.type)
            {
               case AwaResourceType_StringArray:
               {
                   expectedArray = (AwaArray *)this->expectedStringArray_;
                   ASSERT_EQ(data.expectedResult, AwaChangeSet_GetValuesAsStringArrayPointer(changeSet, data.path, (const AwaStringArray **)&array));
                   break;
               }
               case AwaResourceType_IntegerArray:
               {
                   expectedArray = (AwaArray *)this->expectedIntegerArray_;
                   ASSERT_EQ(data.expectedResult, AwaChangeSet_GetValuesAsIntegerArrayPointer(changeSet, data.path, (const AwaIntegerArray **)&array));
                   break;
               }
               case AwaResourceType_FloatArray:
               {
                   expectedArray = (AwaArray *)this->expectedFloatArray_;
                   ASSERT_EQ(data.expectedResult, AwaChangeSet_GetValuesAsFloatArrayPointer(changeSet, data.path, (const AwaFloatArray **)&array));
                   break;
               }
               case AwaResourceType_BooleanArray:
               {
                   expectedArray = (AwaArray *)this->expectedBooleanArray_;
                   ASSERT_EQ(data.expectedResult, AwaChangeSet_GetValuesAsBooleanArrayPointer(changeSet, data.path, (const AwaBooleanArray **)&array));
                   break;
               }
               case AwaResourceType_OpaqueArray:
               {
                   expectedArray = (AwaArray *)this->expectedOpaqueArray_;
                   ASSERT_EQ(data.expectedResult, AwaChangeSet_GetValuesAsOpaqueArrayPointer(changeSet, data.path, (const AwaOpaqueArray **)&array));
                   break;
               }
               case AwaResourceType_TimeArray:
               {
                   expectedArray = (AwaArray *)this->expectedTimeArray_;
                   ASSERT_EQ(data.expectedResult, AwaChangeSet_GetValuesAsTimeArrayPointer(changeSet, data.path, (const AwaTimeArray **)&array));
                   break;
               }
               case AwaResourceType_ObjectLinkArray:
               {
                   expectedArray = (AwaArray *)this->expectedObjectLinkArray_;
                   ASSERT_EQ(data.expectedResult, AwaChangeSet_GetValuesAsObjectLinkArrayPointer(changeSet, data.path, (const AwaObjectLinkArray **)&array));
                   break;
               }
               default:
                   ASSERT_TRUE(false);
                   break;
           }
           ASSERT_TRUE((data.expectedResult == AwaError_Success) == (array != NULL));
           if (array != NULL)
           {
               ASSERT_EQ(0, Array_Compare(expectedArray, array, data.type));
           }
        }
        void TestBody() {}
    };

    ChangeCallbackHandler4 cbHandler(data);
    cbHandler.expectedStringArray_ = this->expectedStringArray_;
    cbHandler.expectedIntegerArray_ = this->expectedIntegerArray_;
    cbHandler.expectedFloatArray_ = this->expectedFloatArray_;
    cbHandler.expectedBooleanArray_ = this->expectedBooleanArray_;
    cbHandler.expectedOpaqueArray_ = this->expectedOpaqueArray_;
    cbHandler.expectedTimeArray_ = this->expectedTimeArray_;
    cbHandler.expectedObjectLinkArray_ = this->expectedObjectLinkArray_;


    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    AwaClientChangeSubscription * subscription = AwaClientChangeSubscription_New(data.path, ChangeCallbackRunner, &cbHandler);
    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddChangeSubscription(operation, subscription));
    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Perform(operation, defaults::timeout));

    // set via client api to trigger notification.
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session_);
    ASSERT_TRUE(NULL != setOperation);

    switch(data.type)
    {
        case AwaResourceType_StringArray:
            ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsStringArray(setOperation, "/10000/0/1", expectedStringArray_));
            break;
        case AwaResourceType_IntegerArray:
             ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsIntegerArray(setOperation, "/10000/0/2", expectedIntegerArray_));
            break;
        case AwaResourceType_FloatArray:
            ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsFloatArray(setOperation, "/10000/0/3", expectedFloatArray_));
            break;
        case AwaResourceType_BooleanArray:
            ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsBooleanArray(setOperation, "/10000/0/4", expectedBooleanArray_));
            break;
        case AwaResourceType_OpaqueArray:
            ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsOpaqueArray(setOperation, "/10000/0/5", expectedOpaqueArray_));
            break;
        case AwaResourceType_TimeArray:
             ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsTimeArray(setOperation, "/10000/0/6", expectedTimeArray_));
            break;
        case AwaResourceType_ObjectLinkArray:
            ASSERT_EQ(AwaError_Success, AwaClientSetOperation_AddValueAsObjectLinkArray(setOperation, "/10000/0/7", expectedObjectLinkArray_));
            break;
        default:
            ASSERT_TRUE(false);
            break;
    }

    EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(setOperation, defaults::timeout));
    AwaClientSetOperation_Free(&setOperation);

    ASSERT_EQ(AwaError_Success, AwaClientSession_Process(session_, defaults::timeout));

    cbHandler.count = 0;
    AwaClientSession_DispatchCallbacks(session_);
    ASSERT_EQ(1, cbHandler.count);

    ASSERT_EQ(AwaError_Success, AwaClientChangeSubscription_Free(&subscription));
    ASSERT_TRUE(NULL == subscription);

    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}


INSTANTIATE_TEST_CASE_P(
        DISABLED_TestAwaChangeSetArraySuccessInstantiation,
        TestAwaChangeSetArray,
        ::testing::Values(
          changeSetDetail::TestAwaChangeSetResourceArray {AwaError_Success,           "/10000/0/1",   true,   AwaResourceType_StringArray},
          changeSetDetail::TestAwaChangeSetResourceArray {AwaError_Success,           "/10000/0/2",   true,   AwaResourceType_IntegerArray},
          changeSetDetail::TestAwaChangeSetResourceArray {AwaError_Success,           "/10000/0/3",   true,   AwaResourceType_FloatArray},
          changeSetDetail::TestAwaChangeSetResourceArray {AwaError_Success,           "/10000/0/4",   true,   AwaResourceType_BooleanArray},
          changeSetDetail::TestAwaChangeSetResourceArray {AwaError_Success,           "/10000/0/5",   true,   AwaResourceType_OpaqueArray},
          changeSetDetail::TestAwaChangeSetResourceArray {AwaError_Success,           "/10000/0/6",   true,   AwaResourceType_TimeArray},
          changeSetDetail::TestAwaChangeSetResourceArray {AwaError_Success,           "/10000/0/7",   true,   AwaResourceType_ObjectLinkArray}
        ));



/* ** Test Execute subscribers ** */

class TestSubscribeToExecuteWithConnectedSession : public TestServerAndClientWithConnectedSession
{
public:
    virtual void callbackHandler(const AwaExecuteArguments * arguments) {};
};


void (ExecuteCallbackRunner)(const AwaExecuteArguments * arguments, void * context)
{
    if (context)
    {
        auto * that = static_cast<TestSubscribeToExecuteWithConnectedSession*>(context);
        that->callbackHandler(arguments);
    }
}

TEST_F(TestSubscribeToExecuteWithConnectedSession, AwaExecuteSubscription_New_free_valid_inputs)
{
    int stuff_to_pass;

    AwaClientExecuteSubscription * subscription = AwaClientExecuteSubscription_New("/3/0/4", EmptyExecuteCallback, (void*)&stuff_to_pass);
    ASSERT_TRUE(NULL != subscription);
    ASSERT_EQ(AwaError_Success, AwaClientExecuteSubscription_Free(&subscription));
    ASSERT_TRUE(NULL == subscription);

    subscription = AwaClientExecuteSubscription_New("/3/0/4", EmptyExecuteCallback, NULL);
    ASSERT_TRUE(NULL != subscription);
    ASSERT_EQ(AwaError_Success, AwaClientExecuteSubscription_Free(&subscription));
    ASSERT_TRUE(NULL == subscription);
}

TEST_F(TestSubscribeToExecuteWithConnectedSession, AwaExecuteSubscription_New_free_invalid)
{
    int stuff_to_pass;

    AwaClientExecuteSubscription * subscription = AwaClientExecuteSubscription_New(NULL, EmptyExecuteCallback, (void*)&stuff_to_pass);
    ASSERT_TRUE(NULL == subscription);
    subscription = AwaClientExecuteSubscription_New("/3/0/4", NULL, (void*)&stuff_to_pass);
    ASSERT_TRUE(NULL == subscription);
    ASSERT_EQ(AwaError_SubscriptionInvalid, AwaClientExecuteSubscription_Free(NULL));
    AwaClientExecuteSubscription ** subscription2 = NULL;
    ASSERT_EQ(AwaError_SubscriptionInvalid, AwaClientExecuteSubscription_Free(subscription2));
}

TEST_F(TestSubscribeToExecuteWithConnectedSession, AwaClientExecuteSubscription_New_free_valid_inputs)
{
    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(client_session_);
    ASSERT_TRUE(NULL != operation);
    AwaClientExecuteSubscription * observation = AwaClientExecuteSubscription_New("/3/0/1", EmptyExecuteCallback, NULL);
    ASSERT_TRUE(NULL != observation);
    ASSERT_EQ(AwaError_Success, AwaClientExecuteSubscription_Free(&observation));
    ASSERT_TRUE(NULL == observation);
    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}

TEST_F(TestSubscribeToExecuteWithConnectedSession, AwaClientExecuteSubscription_New_free_invalid_inputs)
{
    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(client_session_);
    EXPECT_TRUE(NULL != operation);

    EXPECT_TRUE(NULL == AwaClientExecuteSubscription_New(NULL, EmptyExecuteCallback, NULL));
    EXPECT_TRUE(NULL == AwaClientExecuteSubscription_New("/3/0/1", NULL, NULL));

    AwaClientExecuteSubscription * observation = NULL;
    EXPECT_EQ(AwaError_SubscriptionInvalid, AwaClientExecuteSubscription_Free(&observation));
    EXPECT_EQ(AwaError_SubscriptionInvalid, AwaClientExecuteSubscription_Free(NULL));

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    EXPECT_TRUE(NULL == operation);
}

TEST_F(TestSubscribeToExecuteWithConnectedSession, AwaClientSubscribeOperation_AddExecuteSubscription_valid_inputs)
{
    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(client_session_);
    ASSERT_TRUE(NULL != operation);

    AwaClientExecuteSubscription * changeSubscription = AwaClientExecuteSubscription_New("/3/0/4", EmptyExecuteCallback, NULL);

    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddExecuteSubscription(operation, changeSubscription));

    ASSERT_EQ(AwaError_Success, AwaClientExecuteSubscription_Free(&changeSubscription));
    ASSERT_TRUE(NULL == changeSubscription);

    ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}

TEST_F(TestSubscribeToExecuteWithConnectedSession, AwaExecuteSubscription_GetPath_valid_invalid)
{
    ASSERT_TRUE(NULL == AwaClientExecuteSubscription_GetPath(NULL));
    AwaClientExecuteSubscription * subscription = AwaClientExecuteSubscription_New("/3/0/4", EmptyExecuteCallback, NULL);
    ASSERT_TRUE(NULL != subscription);
    ASSERT_STREQ("/3/0/4", AwaClientExecuteSubscription_GetPath(subscription));
    ASSERT_EQ(AwaError_Success, AwaClientExecuteSubscription_Free(&subscription));
    ASSERT_TRUE(NULL == subscription);
    ASSERT_TRUE(NULL == AwaClientExecuteSubscription_GetPath(subscription));
}

TEST_F(TestSubscribeToExecuteWithConnectedSession, AwaClientSubscribeOperation_AddCancelSubscription_cancel_existing_execute_subscription)
{
    // Do operation twice, first time creating the subscription, second time canceling the subscription.
    for (int i = 0; i < 2; i++)
    {
        AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(client_session_);
        ASSERT_TRUE(NULL != operation);
        AwaClientExecuteSubscription * executeSubscription = AwaClientExecuteSubscription_New("/3/0/4", EmptyExecuteCallback, NULL);

        if (i == 0)
            ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddExecuteSubscription(operation, executeSubscription));
        else
            ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddExecuteSubscription(operation, executeSubscription));

        EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Perform(operation, defaults::timeout));
        const AwaClientSubscribeResponse * response = AwaClientSubscribeOperation_GetResponse(operation);
        ASSERT_TRUE(NULL != response);
        const AwaPathResult * result = AwaClientSubscribeResponse_GetPathResult(response, "/3/0/4");
        EXPECT_EQ(AwaError_Success, AwaPathResult_GetError(result));
        ASSERT_EQ(AwaError_Success, AwaClientExecuteSubscription_Free(&executeSubscription));
        ASSERT_TRUE(NULL == executeSubscription);
        ASSERT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
        ASSERT_TRUE(NULL == operation);
    }
}

TEST_F(TestSubscribeToExecuteWithConnectedSession, AwaClientSubscribeOperation_Perform_handles_valid_execute_operation_with_callback_no_payload)
{
    struct ExecuteCallbackHandler1 : public TestSubscribeToExecuteWithConnectedSession
    {
        int count;

        ExecuteCallbackHandler1() : count(0) {}

        void callbackHandler(const AwaExecuteArguments * arguments)
        {
            count ++;

            ASSERT_TRUE(NULL != arguments);
            ASSERT_EQ(0u, arguments->Size);
            //ASSERT_TRUE(NULL == arguments->Data);
        }
        void TestBody() {}
    };
    ExecuteCallbackHandler1 cbHandler;

    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(client_session_);
    ASSERT_TRUE(NULL != operation);

    AwaClientExecuteSubscription * executeSubscription = AwaClientExecuteSubscription_New("/3/0/4", ExecuteCallbackRunner, &cbHandler);

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddExecuteSubscription(operation, executeSubscription));

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Perform(operation, defaults::timeout));

    //execute via server api to trigger notification
    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(server_session_);

    AwaExecuteArguments * arguments = NULL;
    AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/3/0/4", arguments);
    EXPECT_EQ(AwaError_Success, AwaServerExecuteOperation_Perform(executeOperation, defaults::timeout));
    AwaServerExecuteOperation_Free(&executeOperation);

    //wait for execute command to be sent to client
    sleep(1);

    EXPECT_EQ(AwaError_Success, AwaClientSession_Process(client_session_, defaults::timeout));

    cbHandler.count = 0;
    AwaClientSession_DispatchCallbacks(client_session_);
    EXPECT_EQ(1, cbHandler.count);

    EXPECT_EQ(AwaError_Success, AwaClientExecuteSubscription_Free(&executeSubscription));
    EXPECT_TRUE(NULL == executeSubscription);

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    EXPECT_TRUE(NULL == operation);
}

TEST_F(TestSubscribeToExecuteWithConnectedSession, AwaClientSubscribeOperation_Perform_handles_valid_execute_operation_with_callback)
{
    static const char dummyExecuteData[] = {'\0', 'h', 'e', 'l', 'l', 'o'};

    struct ExecuteCallbackHandler1 : public TestSubscribeToExecuteWithConnectedSession
    {
        int count;

        ExecuteCallbackHandler1() : count(0) {}

        void callbackHandler(const AwaExecuteArguments * arguments)
        {
            count ++;

            ASSERT_TRUE(NULL != arguments);
            EXPECT_EQ(6u, arguments->Size);
            EXPECT_EQ(0, memcmp(dummyExecuteData,arguments->Data, arguments->Size));
        }
        void TestBody() {}
    };
    ExecuteCallbackHandler1 cbHandler;

    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(client_session_);
    ASSERT_TRUE(NULL != operation);

    AwaClientExecuteSubscription * executeSubscription = AwaClientExecuteSubscription_New("/3/0/4", ExecuteCallbackRunner, &cbHandler);

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_AddExecuteSubscription(operation, executeSubscription));

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Perform(operation, defaults::timeout));

    //execute via server api to trigger notification.
    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(server_session_);

    AwaExecuteArguments arguments {(void *)dummyExecuteData, sizeof(dummyExecuteData)};
    AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/3/0/4", &arguments);
    EXPECT_EQ(AwaError_Success, AwaServerExecuteOperation_Perform(executeOperation, defaults::timeout));
    AwaServerExecuteOperation_Free(&executeOperation);

    //wait for execute command to be sent to client
    sleep(1);

    EXPECT_EQ(AwaError_Success, AwaClientSession_Process(client_session_, defaults::timeout));

    cbHandler.count = 0;
    AwaClientSession_DispatchCallbacks(client_session_);
    EXPECT_EQ(1, cbHandler.count);

    EXPECT_EQ(AwaError_Success, AwaClientExecuteSubscription_Free(&executeSubscription));
    EXPECT_TRUE(NULL == executeSubscription);

    EXPECT_EQ(AwaError_Success, AwaClientSubscribeOperation_Free(&operation));
    EXPECT_TRUE(NULL == operation);
}



} // namespace Awa


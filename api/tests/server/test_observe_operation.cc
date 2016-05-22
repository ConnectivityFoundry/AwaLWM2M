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

// TODO: Test observe to multiple objects in a single request, then cancel one of the observations without the others being broken

#include <gtest/gtest.h>

#include <lwm2m_tree_node.h>

#include "awa/server.h"
#include "awa/common.h"
#include "memalloc.h"
#include "log.h"
#include "arrays.h"
#include "support/support.h"
#include "observe_operation.h"
#include "server_session.h"
#include "utils.h"

namespace Awa {

class TestObserve : public TestServerBase {};

class TestObserveWithConnectedSession : public TestServerWithConnectedSession {};


void (EmptyObserveCallback)(const AwaChangeSet * changeSet, void * context)
{
}


struct ObserveWaitCondition : public WaitCondition
{

    AwaServerSession * session;
    int callbackCountMax;
    int callbackCount;


    ObserveWaitCondition(AwaServerSession * session, int callbackCountMax = 1) :
        session(session), callbackCountMax(callbackCountMax), callbackCount(0) {}

    virtual ~ObserveWaitCondition() {}

    virtual bool Check()
    {
        EXPECT_EQ(AwaError_Success, AwaServerSession_Process(session, defaults::timeout));
        EXPECT_EQ(AwaError_Success, AwaServerSession_DispatchCallbacks(session));
        return callbackCount >= callbackCountMax;
    }

    virtual void internalCallbackHandler(const AwaChangeSet * changeSet)
    {
        this->callbackCount++;
        this->callbackHandler(changeSet);
    };

    virtual void callbackHandler(const AwaChangeSet * changeSet) = 0;
};

void ObserveCallbackRunner(const AwaChangeSet * changeSet, void * context)
{
    if (context)
    {
        auto * that = static_cast<ObserveWaitCondition*>(context);
        that->internalCallbackHandler(changeSet);
    }
}

TEST_F(TestObserveWithConnectedSession, AwaServerObserveOperation_New_free_valid_inputs)
{
    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}

TEST_F(TestObserveWithConnectedSession, AwaServerObserveOperation_New_free_invalid_inputs)
{
    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(NULL);
    ASSERT_TRUE(NULL == operation);
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerObserveOperation_Free(&operation));
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerObserveOperation_Free(NULL));
}

TEST_F(TestObserveWithConnectedSession, AwaServerObservation_New_free_valid_inputs)
{
    AwaServerObservation * observation = AwaServerObservation_New("client1", "/3/0/1", EmptyObserveCallback, NULL);
    ASSERT_TRUE(NULL != observation);
    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&observation));
    ASSERT_TRUE(NULL == observation);
}

TEST_F(TestObserveWithConnectedSession, AwaServerObservation_New_free_invalid_inputs)
{
    EXPECT_TRUE(NULL == AwaServerObservation_New(NULL, "/3/0/1", EmptyObserveCallback, NULL));
    EXPECT_TRUE(NULL == AwaServerObservation_New("client1", NULL, EmptyObserveCallback, NULL));
    EXPECT_TRUE(NULL == AwaServerObservation_New("client1", "/3/0/1", NULL, NULL));

    AwaServerObservation * observation = NULL;
    EXPECT_EQ(AwaError_ObservationInvalid, AwaServerObservation_Free(&observation));
    EXPECT_EQ(AwaError_ObservationInvalid, AwaServerObservation_Free(NULL));
}

TEST_F(TestObserveWithConnectedSession, AwaServerObserveOperation_AddObservation_valid_inputs)
{
    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaServerObservation * observation = AwaServerObservation_New("client1", "/3/0/1", EmptyObserveCallback, NULL);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, observation));

    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&observation));
    ASSERT_TRUE(NULL == observation);
    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}

TEST_F(TestObserveWithConnectedSession, AwaServerObserveOperation_AddCancelObservation_null_inputs)
{
    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    ASSERT_EQ(AwaError_ObservationInvalid, AwaServerObserveOperation_AddObservation(operation, NULL));
    ASSERT_EQ(AwaError_ObservationInvalid, AwaServerObserveOperation_AddCancelObservation(operation, NULL));

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);

    ASSERT_EQ(AwaError_OperationInvalid, AwaServerObserveOperation_AddObservation(operation, NULL));
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerObserveOperation_AddCancelObservation(operation, NULL));

    AwaServerObservation * changeObservation = AwaServerObservation_New("client1", "/3/0/1", EmptyObserveCallback, NULL);

    ASSERT_EQ(AwaError_OperationInvalid, AwaServerObserveOperation_AddObservation(NULL, changeObservation));
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerObserveOperation_AddCancelObservation(NULL, changeObservation));

    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&changeObservation));
    ASSERT_TRUE(NULL == changeObservation);
}

TEST_F(TestObserveWithConnectedSession, AwaServerObserveOperation_AddObservation_duplicate_path)
{
    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaServerObservation * changeObservation = AwaServerObservation_New("client1", "/3/0/1", EmptyObserveCallback, NULL);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, changeObservation));
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerObserveOperation_AddObservation(operation, changeObservation));
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerObserveOperation_AddCancelObservation(operation, changeObservation));

    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&changeObservation));
    ASSERT_TRUE(NULL == changeObservation);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}

TEST_F(TestObserveWithConnectedSession, AwaServerObserveOperation_Perform_handles_invalid_inputs)
{
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerObserveOperation_Perform(NULL, 0));

    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerObserveOperation_Perform(NULL, -1));

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
    ASSERT_EQ(AwaError_OperationInvalid, AwaServerObserveOperation_Perform(operation, 0));
}

TEST_F(TestObserveWithConnectedSession, AwaServerObserveOperation_Perform_handles_valid_operation_with_callback)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, global::clientIpcPort, CURRENT_TEST_DESCRIPTION);
    sleep(1);

    struct CallbackHandler1 : public ObserveWaitCondition
    {
        int count;

        CallbackHandler1(AwaServerSession * session, int max)  : ObserveWaitCondition(session, max), count(0) {};

        void callbackHandler(const AwaChangeSet * changeSet)
        {
            count ++;

            ASSERT_TRUE(NULL != changeSet);
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3"));
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3/0"));
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3/0/15"));
            EXPECT_FALSE(AwaChangeSet_ContainsPath(changeSet, "/3000/0/0"));
            EXPECT_TRUE(AwaChangeSet_HasValue(changeSet, "/3/0/15"));
            EXPECT_FALSE(AwaChangeSet_HasValue(changeSet, "/3000/0/0"));

            // TODO: need to add support for the other cases
            EXPECT_EQ(AwaChangeType_ResourceModified, AwaChangeSet_GetChangeType(changeSet, "/3/0/15"));

            const char * value;
            AwaChangeSet_GetValueAsCStringPointer(changeSet, "/3/0/15", &value);
            EXPECT_STREQ(count == 1? "Pacific/Wellington" : "123414123", value);
        }
    };
    CallbackHandler1 cbHandler(session_, 2);

    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaServerObservation * observation = AwaServerObservation_New(global::clientEndpointName, "/3/0/15", ObserveCallbackRunner, &cbHandler);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, observation));

    EXPECT_EQ(AwaError_Success, AwaServerObserveOperation_Perform(operation, defaults::timeout));

    // set via server api to trigger notification.
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsCString(writeOperation, "/3/0/15", "123414123"));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    cbHandler.count = 0;
    ASSERT_TRUE(cbHandler.Wait());
    ASSERT_EQ(2, cbHandler.count);

    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&observation));
    ASSERT_TRUE(NULL == observation);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}

TEST_F(TestObserveWithConnectedSession, AwaServerObserveOperation_Perform_handles_wrong_session_type)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, global::clientIpcPort, CURRENT_TEST_DESCRIPTION);
    sleep(1);

    struct CallbackHandler1 : public ObserveWaitCondition
    {
        int count;

        CallbackHandler1(AwaServerSession * session, int max)  : ObserveWaitCondition(session, max), count(0) {};

        void callbackHandler(const AwaChangeSet * changeSet)
        {
            count ++;

            const AwaClientSession * clientSession = AwaChangeSet_GetClientSession(changeSet);
            EXPECT_TRUE(NULL == clientSession);
            const AwaServerSession * serverSession = AwaChangeSet_GetServerSession(changeSet);
            EXPECT_TRUE(NULL != serverSession);
        }
        void TestBody() {}
    };
    CallbackHandler1 cbHandler(session_, 2);

    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaServerObservation * observation = AwaServerObservation_New(global::clientEndpointName, "/3/0/15", ObserveCallbackRunner, &cbHandler);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, observation));

    EXPECT_EQ(AwaError_Success, AwaServerObserveOperation_Perform(operation, defaults::timeout));

    // set via server api to trigger notification.
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsCString(writeOperation, "/3/0/15", "123414123"));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    sleep(1); // otherwise we can miss the second notify

    ASSERT_EQ(AwaError_Success, AwaServerSession_Process(session_, defaults::timeout));

    cbHandler.count = 0;
    ASSERT_TRUE(cbHandler.Wait());
    ASSERT_EQ(2, cbHandler.count);

    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&observation));
    ASSERT_TRUE(NULL == observation);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}


TEST_F(TestObserveWithConnectedSession, AwaServerObserveOperation_Perform_handles_step_attribute)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, global::clientIpcPort, CURRENT_TEST_DESCRIPTION);
    sleep(1);

    struct CallbackHandler1 : public ObserveWaitCondition
    {
        int count;

        CallbackHandler1(AwaServerSession * session)  : ObserveWaitCondition(session), count(0) {};

        void callbackHandler(const AwaChangeSet * changeSet)
        {
            count ++;
            printf("Received notification %d\n", count);

            ASSERT_TRUE(NULL != changeSet);
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3"));
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3/0"));
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3/0/13"));
            EXPECT_FALSE(AwaChangeSet_ContainsPath(changeSet, "/3000/0/0"));
            EXPECT_TRUE(AwaChangeSet_HasValue(changeSet, "/3/0/13"));
            EXPECT_FALSE(AwaChangeSet_HasValue(changeSet, "/3000/0/0"));

            // TODO: need to add support for the other cases
            EXPECT_EQ(AwaChangeType_ResourceModified, AwaChangeSet_GetChangeType(changeSet, "/3/0/13"));

            const AwaTime * value;
            AwaChangeSet_GetValueAsTimePointer(changeSet, "/3/0/13", &value);
            switch(count)
            {
            case 1:
                EXPECT_EQ(2718619435, *value);
                break;
            case 2:
                EXPECT_EQ(0, *value);
                break;
            case 3:
                EXPECT_EQ(10, *value);
                break;
            default:
                EXPECT_TRUE(false);
            }


        }
    };
    CallbackHandler1 cbHandler(session_);

    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_TRUE(NULL != writeAttributesOperation);
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/13", "stp", 10));
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_Perform(writeAttributesOperation, defaults::timeout));
    AwaServerWriteAttributesOperation_Free(&writeAttributesOperation);


    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaServerObservation * observation = AwaServerObservation_New(global::clientEndpointName, "/3/0/13", ObserveCallbackRunner, &cbHandler);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, observation));

    EXPECT_EQ(AwaError_Success, AwaServerObserveOperation_Perform(operation, defaults::timeout));

    cbHandler.count = 0;
    ASSERT_TRUE(cbHandler.Wait());

    // set via server api to trigger notifications.
    // write 0, 5 and 10. 5 should be skipped as the step size is 10.
    for(int i = 0; i < 3; i++)
    {
        AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
        ASSERT_TRUE(NULL != writeOperation);
        ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsTime(writeOperation, "/3/0/13", i*5));
        EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
        AwaServerWriteOperation_Free(&writeOperation);

        cbHandler.callbackCount = 0;
        if(i != 1)
        {
            ASSERT_TRUE(cbHandler.Wait());
        }
        else
        {
            //This should timeout as 5 is written down - no notifications should be generated.
            ASSERT_FALSE(cbHandler.Wait());
        }
    }

    ASSERT_EQ(3, cbHandler.count);

    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&observation));
    ASSERT_TRUE(NULL == observation);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}

TEST_F(TestObserveWithConnectedSession, AwaServerObserveOperation_Perform_handles_pmin_attribute)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, global::clientIpcPort, CURRENT_TEST_DESCRIPTION);
    sleep(1);

    struct CallbackHandler1 : public ObserveWaitCondition
    {
        int count;

        CallbackHandler1(AwaServerSession * session)  : ObserveWaitCondition(session), count(0) {};

        void callbackHandler(const AwaChangeSet * changeSet)
        {
            count ++;
            printf("Received notification %d\n", count);

            ASSERT_TRUE(NULL != changeSet);
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3"));
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3/0"));
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3/0/13"));
            EXPECT_FALSE(AwaChangeSet_ContainsPath(changeSet, "/3000/0/0"));
            EXPECT_TRUE(AwaChangeSet_HasValue(changeSet, "/3/0/13"));
            EXPECT_FALSE(AwaChangeSet_HasValue(changeSet, "/3000/0/0"));

            // TODO: need to add support for the other cases
            EXPECT_EQ(AwaChangeType_ResourceModified, AwaChangeSet_GetChangeType(changeSet, "/3/0/13"));

            const AwaTime * value;
            AwaChangeSet_GetValueAsTimePointer(changeSet, "/3/0/13", &value);
            switch(count)
            {
            case 1:
                EXPECT_EQ(2718619435, *value);
                break;
            case 2:
                EXPECT_EQ(0, *value);
                break;
            case 3:
                EXPECT_EQ(5, *value);
                break;
            default:
                EXPECT_TRUE(false);
            }
        }
        void TestBody() {}
    };
    CallbackHandler1 cbHandler(session_);

    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_TRUE(NULL != writeAttributesOperation);
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/13", "pmin", 5));
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_Perform(writeAttributesOperation, defaults::timeout));
    AwaServerWriteAttributesOperation_Free(&writeAttributesOperation);


    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaServerObservation * observation = AwaServerObservation_New(global::clientEndpointName, "/3/0/13", ObserveCallbackRunner, &cbHandler);
    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, observation));
    EXPECT_EQ(AwaError_Success, AwaServerObserveOperation_Perform(operation, defaults::timeout));

    cbHandler.count = 0;
    cbHandler.callbackCount = 0;
    ASSERT_TRUE(cbHandler.Wait());
    ASSERT_EQ(1, cbHandler.count);


    printf("Doing first write - should receive immediate notification\n");
    // write first change via server api to trigger notification.
    {
        AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
        ASSERT_TRUE(NULL != writeOperation);
        ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsTime(writeOperation, "/3/0/13", 0));
        EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
        AwaServerWriteOperation_Free(&writeOperation);
    }

    cbHandler.callbackCount = 0;
    ASSERT_TRUE(cbHandler.Wait());
    ASSERT_EQ(2, cbHandler.count);

    printf("Doing second write - should receive notification after pmin...\n");
    BasicTimer pminTimer;
    // write second change via server api to trigger notification.
    // change notification should not come immediately due to pmin.
    {
        AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
        ASSERT_TRUE(NULL != writeOperation);
        ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsTime(writeOperation, "/3/0/13", 5));
        EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
        AwaServerWriteOperation_Free(&writeOperation);
        pminTimer.Start();
    }

    cbHandler.callbackCount = 0;
    ASSERT_TRUE(cbHandler.Wait());
    pminTimer.Stop();
    printf("Time elapsed %f ms\n", pminTimer.TimeElapsed_Milliseconds());
    ASSERT_LE(5, pminTimer.TimeElapsed_Seconds());
    ASSERT_EQ(3, cbHandler.count);

    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&observation));
    ASSERT_TRUE(NULL == observation);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}

TEST_F(TestObserveWithConnectedSession, AwaServerObserveOperation_Perform_handles_pmax_attribute)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, global::clientIpcPort, CURRENT_TEST_DESCRIPTION);
    sleep(1);

    struct CallbackHandler1 : public ObserveWaitCondition
    {
        int count;

        CallbackHandler1(AwaServerSession * session, int max)  : ObserveWaitCondition(session, max), count(0) {};

        void callbackHandler(const AwaChangeSet * changeSet)
        {
            count ++;
            printf("Received notification %d\n", count);

            ASSERT_TRUE(NULL != changeSet);
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3"));
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3/0"));
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3/0/13"));
            EXPECT_FALSE(AwaChangeSet_ContainsPath(changeSet, "/3000/0/0"));
            EXPECT_TRUE(AwaChangeSet_HasValue(changeSet, "/3/0/13"));
            EXPECT_FALSE(AwaChangeSet_HasValue(changeSet, "/3000/0/0"));

            // TODO: need to add support for the other cases
            EXPECT_EQ(AwaChangeType_ResourceModified, AwaChangeSet_GetChangeType(changeSet, "/3/0/13"));

            const AwaTime * value;
            AwaChangeSet_GetValueAsTimePointer(changeSet, "/3/0/13", &value);
            EXPECT_EQ(2718619435, *value);
        }
        void TestBody() {}
    };
    CallbackHandler1 cbHandler(session_, 3);

    AwaServerWriteAttributesOperation * writeAttributesOperation = AwaServerWriteAttributesOperation_New(session_);
    ASSERT_TRUE(NULL != writeAttributesOperation);
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_AddAttributeAsInteger(writeAttributesOperation, global::clientEndpointName, "/3/0/13", "pmax", 1));
    ASSERT_EQ(AwaError_Success, AwaServerWriteAttributesOperation_Perform(writeAttributesOperation, defaults::timeout));
    AwaServerWriteAttributesOperation_Free(&writeAttributesOperation);


    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaServerObservation * observation = AwaServerObservation_New(global::clientEndpointName, "/3/0/13", ObserveCallbackRunner, &cbHandler);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, observation));
    EXPECT_EQ(AwaError_Success, AwaServerObserveOperation_Perform(operation, defaults::timeout));

    //Should get multiple notifications due to pmax being a second
    cbHandler.count = 0;
    ASSERT_TRUE(cbHandler.Wait());
    ASSERT_TRUE(cbHandler.count = 3);

    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&observation));
    ASSERT_TRUE(NULL == observation);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}


TEST_F(TestObserveWithConnectedSession, AwaServerObserveOperation_Perform_handles_server_object_minimum_period)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, global::clientIpcPort, CURRENT_TEST_DESCRIPTION);
    sleep(1);

    struct CallbackHandler1 : public ObserveWaitCondition
    {
        int count;

        CallbackHandler1(AwaServerSession * session)  : ObserveWaitCondition(session), count(0) {};

        void callbackHandler(const AwaChangeSet * changeSet)
        {
            count ++;
            printf("Received notification %d\n", count);

            ASSERT_TRUE(NULL != changeSet);
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3"));
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3/0"));
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3/0/13"));
            EXPECT_FALSE(AwaChangeSet_ContainsPath(changeSet, "/3000/0/0"));
            EXPECT_TRUE(AwaChangeSet_HasValue(changeSet, "/3/0/13"));
            EXPECT_FALSE(AwaChangeSet_HasValue(changeSet, "/3000/0/0"));

            // TODO: need to add support for the other cases
            EXPECT_EQ(AwaChangeType_ResourceModified, AwaChangeSet_GetChangeType(changeSet, "/3/0/13"));

            const AwaTime * value;
            AwaChangeSet_GetValueAsTimePointer(changeSet, "/3/0/13", &value);
            switch(count)
            {
            case 1:
                EXPECT_EQ(2718619435, *value);
                break;
            case 2:
                EXPECT_EQ(0, *value);
                break;
            case 3:
                EXPECT_EQ(5, *value);
                break;
            default:
                EXPECT_TRUE(false);
            }
        }
        void TestBody() {}
    };
    CallbackHandler1 cbHandler(session_);

    //set pmin for the server object
    {
        AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
        ASSERT_TRUE(NULL != writeOperation);
        ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/1/0/2", 5));
        ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
        AwaServerWriteOperation_Free(&writeOperation);
    }


    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaServerObservation * observation = AwaServerObservation_New(global::clientEndpointName, "/3/0/13", ObserveCallbackRunner, &cbHandler);
    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, observation));
    EXPECT_EQ(AwaError_Success, AwaServerObserveOperation_Perform(operation, defaults::timeout));

    cbHandler.count = 0;

    cbHandler.callbackCount = 0;
    ASSERT_TRUE(cbHandler.Wait());

    ASSERT_EQ(1, cbHandler.count);
    printf("Doing first write - should receive immediate notification\n");
    // write first change via server api to trigger notification.
    {
        AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
        ASSERT_TRUE(NULL != writeOperation);
        ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsTime(writeOperation, "/3/0/13", 0));
        EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
        AwaServerWriteOperation_Free(&writeOperation);
    }

    cbHandler.callbackCount = 0;
    ASSERT_TRUE(cbHandler.Wait());
    ASSERT_EQ(2, cbHandler.count);

    printf("Doing second write - should receive notification after pmin...\n");
    BasicTimer pminTimer;
    // write second change via server api to trigger notification.
    // change notification should not come immediately due to pmin.
    {
        AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
        ASSERT_TRUE(NULL != writeOperation);
        ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsTime(writeOperation, "/3/0/13", 5));
        EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
        AwaServerWriteOperation_Free(&writeOperation);
        pminTimer.Start();
    }

    cbHandler.callbackCount = 0;
    ASSERT_TRUE(cbHandler.Wait());
    pminTimer.Stop();
    printf("Time elapsed %f ms\n", pminTimer.TimeElapsed_Milliseconds());
    ASSERT_LE(5, pminTimer.TimeElapsed_Seconds());
    ASSERT_EQ(3, cbHandler.count);

    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&observation));
    ASSERT_TRUE(NULL == observation);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}

TEST_F(TestObserveWithConnectedSession, AwaServerObserveOperation_Perform_handles_server_object_maximum_period)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, global::clientIpcPort, CURRENT_TEST_DESCRIPTION);
    sleep(1);

    struct CallbackHandler1 : public ObserveWaitCondition
    {
        int count;

        CallbackHandler1(AwaServerSession * session, int max)  : ObserveWaitCondition(session, max), count(0) {};


        void callbackHandler(const AwaChangeSet * changeSet)
        {
            count ++;
            printf("Received notification %d\n", count);

            ASSERT_TRUE(NULL != changeSet);
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3"));
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3/0"));
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3/0/13"));
            EXPECT_FALSE(AwaChangeSet_ContainsPath(changeSet, "/3000/0/0"));
            EXPECT_TRUE(AwaChangeSet_HasValue(changeSet, "/3/0/13"));
            EXPECT_FALSE(AwaChangeSet_HasValue(changeSet, "/3000/0/0"));

            // TODO: need to add support for the other cases
            EXPECT_EQ(AwaChangeType_ResourceModified, AwaChangeSet_GetChangeType(changeSet, "/3/0/13"));

            const AwaTime * value;
            AwaChangeSet_GetValueAsTimePointer(changeSet, "/3/0/13", &value);
            EXPECT_EQ(2718619435, *value);
        }
        void TestBody() {}
    };
    CallbackHandler1 cbHandler(session_, 3);

    //set pmax for the server object
    {
        AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
        ASSERT_TRUE(NULL != writeOperation);
        ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/1/0/3", 1));
        ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
        AwaServerWriteOperation_Free(&writeOperation);
    }


    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaServerObservation * observation = AwaServerObservation_New(global::clientEndpointName, "/3/0/13", ObserveCallbackRunner, &cbHandler);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, observation));
    EXPECT_EQ(AwaError_Success, AwaServerObserveOperation_Perform(operation, defaults::timeout));

    cbHandler.count = 0;
    ASSERT_TRUE(cbHandler.Wait());
    ASSERT_TRUE(cbHandler.count = 3);

    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&observation));
    ASSERT_TRUE(NULL == observation);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}

TEST_F(TestObserveWithConnectedSession, AwaServerObserveOperation_Perform_handles_operation_freed_after_observation)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, global::clientIpcPort, CURRENT_TEST_DESCRIPTION);
    sleep(1);

    struct CallbackHandler1 : public ObserveWaitCondition
    {
        int count;

        CallbackHandler1(AwaServerSession * session, int max)  : ObserveWaitCondition(session, max), count(0) {};

        void callbackHandler(const AwaChangeSet * changeSet)
        {
            count ++;
        }
    };
    CallbackHandler1 cbHandler(session_, 2);

    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    AwaServerObservation * observation = AwaServerObservation_New(global::clientEndpointName, "/3/0/15", ObserveCallbackRunner, &cbHandler);
    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, observation));
    EXPECT_EQ(AwaError_Success, AwaServerObserveOperation_Perform(operation, defaults::timeout));
    // set via server api to trigger notification.
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsCString(writeOperation, "/3/0/15", "123414123"));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);
    ASSERT_TRUE(cbHandler.Wait());
    ASSERT_EQ(2, cbHandler.count);
    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&observation));
    ASSERT_TRUE(NULL == observation);

    EXPECT_EQ(0u, Map_Length(ServerObserveOperation_GetObservers(operation)));

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}

TEST_F(TestObserveWithConnectedSession, AwaServerObserveOperation_Perform_handles_observation_freed_after_operation)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, global::clientIpcPort, CURRENT_TEST_DESCRIPTION);
    sleep(1);

    struct CallbackHandler1 : public ObserveWaitCondition
    {
        int count;

        CallbackHandler1(AwaServerSession * session, int max)  : ObserveWaitCondition(session, max), count(0) {};

        void callbackHandler(const AwaChangeSet * changeSet)
        {
            count ++;
        }
        void TestBody() {}
    };
    CallbackHandler1 cbHandler(session_, 2);

    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    AwaServerObservation * observation = AwaServerObservation_New(global::clientEndpointName, "/3/0/15", ObserveCallbackRunner, &cbHandler);
    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, observation));
    EXPECT_EQ(AwaError_Success, AwaServerObserveOperation_Perform(operation, defaults::timeout));
    // set via server api to trigger notification.
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsCString(writeOperation, "/3/0/15", "123414123"));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);
    cbHandler.count = 0;
    ASSERT_TRUE(cbHandler.Wait());
    ASSERT_EQ(2, cbHandler.count);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);

    EXPECT_EQ(0u, List_Length(ServerObservation_GetOperations(observation)));

    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&observation));
    ASSERT_TRUE(NULL == observation);
}

TEST_F(TestObserveWithConnectedSession, AwaServerObserveOperation_Perform_handles_observation_freed_before_perform)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, global::clientIpcPort, CURRENT_TEST_DESCRIPTION);
    sleep(1);

    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    AwaServerObservation * observation1 = AwaServerObservation_New(global::clientEndpointName, "/3/0/15", EmptyObserveCallback, NULL);
    AwaServerObservation * observation2 = AwaServerObservation_New(global::clientEndpointName, "/3/0/16", EmptyObserveCallback, NULL);
    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, observation1));
    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, observation2));

    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&observation2));
    ASSERT_TRUE(NULL == observation2);

    EXPECT_EQ(AwaError_Success, AwaServerObserveOperation_Perform(operation, defaults::timeout));

    const AwaServerObserveResponse * response = AwaServerObserveOperation_GetResponse(operation, global::clientEndpointName);
    ASSERT_TRUE(response != NULL);

    EXPECT_TRUE(AwaServerObserveResponse_GetPathResult(response, "/3/0/15") != NULL);
    EXPECT_TRUE(AwaServerObserveResponse_GetPathResult(response, "/3/0/16") == NULL);

    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&observation1));
    ASSERT_TRUE(NULL == observation1);
    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}


TEST_F(TestObserveWithConnectedSession, AwaServerObserveOperation_Perform_honours_timeout)
{
    // start a client
    AwaClientDaemonHorde * horde_ = new AwaClientDaemonHorde( { global::clientEndpointName }, global::clientIpcPort, CURRENT_TEST_DESCRIPTION);
    sleep(1);      // wait for the client to register with the server

    AwaClientSession * clientSession = AwaClientSession_New();
    ASSERT_TRUE(NULL != clientSession);
    EXPECT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(clientSession, "0.0.0.0", global::clientIpcPort));
    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(clientSession));

    AwaServerSession * serverSession = AwaServerSession_New();
    ASSERT_TRUE(NULL != serverSession);
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(serverSession, "0.0.0.0", global::serverIpcPort));
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(serverSession));

    AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(clientSession);
    EXPECT_TRUE(NULL != clientDefineOperation);
    AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(serverSession);
    EXPECT_TRUE(NULL != serverDefineOperation);
    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(10002, "Test Object2", 1, 1);
    EXPECT_TRUE(NULL != objectDefinition);

    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefinition, 0, "Test Mandatory Integer Resource", true, AwaResourceOperations_ReadWrite, 12345));
    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, objectDefinition));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, objectDefinition));
    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, defaults::timeout));
    ASSERT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, defaults::timeout));

    AwaObjectDefinition_Free(&objectDefinition);
    AwaClientDefineOperation_Free(&clientDefineOperation);
    AwaServerDefineOperation_Free(&serverDefineOperation);


    struct CallbackHandler1 : public ObserveWaitCondition
    {
        int count;

        CallbackHandler1(AwaServerSession * session)  : ObserveWaitCondition(session), count(0) {};

        void callbackHandler(const AwaChangeSet * changeSet)
        {
            count ++;
        }
        void TestBody() {}
    };
    CallbackHandler1 cbHandler(session_);

    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(serverSession);
    ASSERT_TRUE(NULL != operation);

    AwaServerObservation * observation = AwaServerObservation_New(global::clientEndpointName, "/10002", ObserveCallbackRunner, &cbHandler);
    ASSERT_TRUE(NULL != observation);
    EXPECT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, observation));

    // Tear down client process
    TestObserveWithConnectedSession::TearDown();

    BasicTimer timer;
    timer.Start();
    EXPECT_EQ(AwaError_Timeout, AwaServerObserveOperation_Perform(operation, defaults::timeout));
    timer.Stop();
    EXPECT_TRUE(ElapsedTimeWithinTolerance(timer.TimeElapsed_Milliseconds(), defaults::timeout, defaults::timeoutTolerance)) << "Time elapsed: " << timer.TimeElapsed_Milliseconds() << "ms";

    AwaServerObservation_Free(&observation);
    AwaServerObserveOperation_Free(&operation);

    AwaClientSession_Disconnect(clientSession);
    AwaClientSession_Free(&clientSession);

    AwaServerSession_Disconnect(serverSession);
    AwaServerSession_Free(&serverSession);

    AwaServerSession_Free(&serverSession);
    AwaClientSession_Free(&clientSession);

    delete horde_;
}



TEST_F(TestObserveWithConnectedSession, AwaServerObserveOperation_Perform_cancel_observation_stops_notifications)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, global::clientIpcPort, CURRENT_TEST_DESCRIPTION);
    sleep(1);

    struct CallbackHandler1 : public ObserveWaitCondition
    {
        int count;

        CallbackHandler1(AwaServerSession * session)  : ObserveWaitCondition(session), count(0) {};

        virtual void callbackHandler(const AwaChangeSet * changeSet)
        {
            ASSERT_TRUE(count == 0);
            count++;

            ASSERT_TRUE(NULL != changeSet);
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3"));
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3/0"));
            EXPECT_TRUE(AwaChangeSet_ContainsPath(changeSet, "/3/0/15"));
            EXPECT_FALSE(AwaChangeSet_ContainsPath(changeSet, "/3000/0/0"));
            EXPECT_TRUE(AwaChangeSet_HasValue(changeSet, "/3/0/15"));
            EXPECT_FALSE(AwaChangeSet_HasValue(changeSet, "/3000/0/0"));

            // TODO: need to add support for the other cases
            EXPECT_EQ(AwaChangeType_ResourceModified, AwaChangeSet_GetChangeType(changeSet, "/3/0/15"));

            const char * value;
            AwaChangeSet_GetValueAsCStringPointer(changeSet, "/3/0/15", &value);
            EXPECT_STREQ("Pacific/Wellington", value);
        }
    };
    CallbackHandler1 cbHandler(session_);

    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaServerObservation * observation = AwaServerObservation_New(global::clientEndpointName, "/3/0/15", ObserveCallbackRunner, &cbHandler);
    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, observation));
    EXPECT_EQ(AwaError_Success, AwaServerObserveOperation_Perform(operation, defaults::timeout));

    ASSERT_TRUE(cbHandler.Wait());

    AwaServerObserveOperation * cancelObserveOperation = AwaServerObserveOperation_New(session_);
    ASSERT_TRUE(NULL != cancelObserveOperation);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddCancelObservation(cancelObserveOperation, observation));
    EXPECT_EQ(AwaError_Success, AwaServerObserveOperation_Perform(cancelObserveOperation, defaults::timeout));


    // write via server api. Should NOT trigger a notification since we are no longer observing the resource.
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsCString(writeOperation, "/3/0/15", "123414123"));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    cbHandler.callbackCount = 0;
    ASSERT_FALSE(cbHandler.Wait());

    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&observation));
    ASSERT_TRUE(NULL == observation);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&cancelObserveOperation));
    ASSERT_TRUE(NULL == cancelObserveOperation);
}


TEST_F(TestObserveWithConnectedSession, AwaServerObserveOperation_Perform_handles_executable_resource)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, global::clientIpcPort, CURRENT_TEST_DESCRIPTION);
    sleep(1);

    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaServerObservation * observation = AwaServerObservation_New(global::clientEndpointName, "/3/0/4", EmptyObserveCallback, NULL);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, observation));

    EXPECT_EQ(AwaError_Response, AwaServerObserveOperation_Perform(operation, defaults::timeout));

    const AwaServerObserveResponse * response = AwaServerObserveOperation_GetResponse(operation, global::clientEndpointName);
    ASSERT_TRUE(NULL != response);
    const AwaPathResult * result = AwaServerObserveResponse_GetPathResult(response, "/3/0/4");
    ASSERT_TRUE(NULL != result);
    EXPECT_EQ(AwaError_LWM2MError, AwaPathResult_GetError(result));
    EXPECT_EQ(AwaLWM2MError_BadRequest, AwaPathResult_GetLWM2MError(result));


    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&observation));
    ASSERT_TRUE(NULL == observation);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}

//TEST_F(TestObserveWithConnectedSession, AwaServerObserveOperation_Perform_handles_valid_operation_multiple_paths)
//{
//    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
//    ASSERT_TRUE(NULL != operation);
//
//    AwaServerObservation * changeObservation = AwaServerObservation_New("/3/0/1", EmptyObserveCallback, NULL);
//    AwaServerObservation * changeObservation2 = AwaServerObservation_New("/3/0/0", EmptyObserveCallback, NULL);
//    AwaServerExecuteObservation * executeObservation = AwaServerExecuteObservation_New("/3/0/4", EmptyExecuteCallback, NULL);
//
//    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, changeObservation));
//    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, changeObservation2));
//    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddExecuteObservation(operation, executeObservation));
//
//    EXPECT_EQ(AwaError_Success, AwaServerObserveOperation_Perform(operation, defaults::timeout));
//
//    EXPECT_EQ(3, Map_Length(ServerSession_GetObservers(session_)));
//
//    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&changeObservation));
//    ASSERT_TRUE(NULL == changeObservation);
//
//    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&changeObservation2));
//    ASSERT_TRUE(NULL == changeObservation);
//
//    ASSERT_EQ(AwaError_Success, AwaServerExecuteObservation_Free(&executeObservation));
//    ASSERT_TRUE(NULL == executeObservation);
//
//    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
//    ASSERT_TRUE(NULL == operation);
//}
//
TEST_F(TestObserveWithConnectedSession, AwaServerObserveOperation_AddObservation_add_observation_to_non_existent_resource)
{
    // start a client
    AwaClientDaemonHorde horde( { global::clientEndpointName }, global::clientIpcPort, CURRENT_TEST_DESCRIPTION);
    sleep(1);

    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaServerObservation * observation = AwaServerObservation_New(global::clientEndpointName, "/3/0/9000", EmptyObserveCallback, NULL);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, observation));

    EXPECT_EQ(AwaError_Response, AwaServerObserveOperation_Perform(operation, defaults::timeout));

    const AwaServerObserveResponse * response = AwaServerObserveOperation_GetResponse(operation, global::clientEndpointName);
    ASSERT_TRUE(NULL != response);
    const AwaPathResult * result = AwaServerObserveResponse_GetPathResult(response, "/3/0/9000");
    ASSERT_TRUE(NULL != result);
    EXPECT_EQ(AwaError_LWM2MError, AwaPathResult_GetError(result));
    EXPECT_EQ(AwaLWM2MError_NotFound, AwaPathResult_GetLWM2MError(result));

    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&observation));
    ASSERT_TRUE(NULL == observation);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}

//TEST_F(TestObserveWithConnectedSession, AwaServerObserveOperation_AddObservation_replace_existing_observation)
//{
//    // Do operation twice, second should succeed (replace)
//    for (int i = 0; i < 2; i++)
//    {
//        AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
//        ASSERT_TRUE(NULL != operation);
//        AwaServerObservation * changeObservation = AwaServerObservation_New("/3/0/1", EmptyObserveCallback, NULL);
//        ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, changeObservation));
//        EXPECT_EQ(AwaError_Success, AwaServerObserveOperation_Perform(operation, defaults::timeout));
//        const AwaServerObserveResponse * response = AwaServerObserveOperation_GetResponse(operation);
//        ASSERT_TRUE(NULL != response);
//        const AwaPathResult * result;
//        AwaServerObserveResponse_GetPathResult(response, "/3/0/1", &result);
//        EXPECT_EQ(AwaError_Success, AwaPathResult_GetError(result));
//        ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&changeObservation));
//        ASSERT_TRUE(NULL == changeObservation);
//        ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
//        ASSERT_TRUE(NULL == operation);
//    }
//}
//
TEST_F(TestObserveWithConnectedSession, AwaServerObserveOperation_AddCancelObservation_cancel_non_existing_observation)
{
    // cancel should still work even if an observation doesn't exist?
    AwaClientDaemonHorde horde( { global::clientEndpointName }, global::clientIpcPort, CURRENT_TEST_DESCRIPTION);
    sleep(1);

    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaServerObservation * changeObservation = AwaServerObservation_New(global::clientEndpointName, "/3/0/1", EmptyObserveCallback, NULL);
    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddCancelObservation(operation, changeObservation));

    EXPECT_EQ(AwaError_Success, AwaServerObserveOperation_Perform(operation, defaults::timeout));
    const AwaServerObserveResponse * response = AwaServerObserveOperation_GetResponse(operation, global::clientEndpointName);
    ASSERT_TRUE(NULL != response);
    const AwaPathResult * result = AwaServerObserveResponse_GetPathResult(response, "/3/0/1");

    EXPECT_EQ(AwaError_Success, AwaPathResult_GetError(result));
    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&changeObservation));
    ASSERT_TRUE(NULL == changeObservation);
    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}

//
//
//TEST_F(TestObserveWithConnectedSession, AwaServerObserveResponse_NewPathIterator_handles_null_response)
//{
//    EXPECT_EQ(NULL, AwaServerObserveResponse_NewPathIterator(NULL));
//}
//
//TEST_F(TestObserveWithConnectedSession, AwaServerObserveResponse_NewPathIterator_handles_valid_response)
//{
//    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
//    ASSERT_TRUE(NULL != operation);
//
//    AwaServerObservation * changeObservation = AwaServerObservation_New("/3/0/0", EmptyObserveCallback, NULL);
//    AwaServerObservation * changeObservation2 = AwaServerObservation_New("/3/0/1", EmptyObserveCallback, NULL);
//    AwaServerExecuteObservation * executeObservation = AwaServerExecuteObservation_New("/3/0/4", EmptyExecuteCallback, NULL);
//
//    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, changeObservation));
//    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, changeObservation2));
//    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddExecuteObservation(operation, executeObservation));
//
//    EXPECT_EQ(AwaError_Success, AwaServerObserveOperation_Perform(operation, defaults::timeout));
//
//    const AwaServerObserveResponse * response = AwaServerObserveOperation_GetResponse(operation);
//    ASSERT_TRUE(NULL != response);
//
//    AwaPathIterator * iterator = AwaServerObserveResponse_NewPathIterator(response);
//    ASSERT_TRUE(NULL != iterator);
//
//    //ensure the observation paths are in our response
//    EXPECT_EQ(true, AwaPathIterator_Next(iterator));
//    EXPECT_STREQ("/3/0/0", AwaPathIterator_Get(iterator));
//
//    EXPECT_EQ(true, AwaPathIterator_Next(iterator));
//    EXPECT_STREQ("/3/0/1", AwaPathIterator_Get(iterator));
//
//    EXPECT_EQ(true, AwaPathIterator_Next(iterator));
//    EXPECT_STREQ("/3/0/4", AwaPathIterator_Get(iterator));
//
//    // not expecting any more paths
//    EXPECT_TRUE(false == AwaPathIterator_Next(iterator));
//    EXPECT_TRUE(NULL == AwaPathIterator_Get(iterator));
//
//    AwaPathIterator_Free(&iterator);
//    EXPECT_EQ(NULL, iterator);
//
//    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&changeObservation));
//    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&changeObservation2));
//    ASSERT_EQ(AwaError_Success, AwaServerExecuteObservation_Free(&executeObservation));
//    ASSERT_TRUE(NULL == changeObservation);
//    ASSERT_TRUE(NULL == changeObservation);
//    ASSERT_TRUE(NULL == executeObservation);
//
//    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
//    ASSERT_TRUE(NULL == operation);
//}
//
TEST_F(TestObserveWithConnectedSession, AwaObservation_New_free_valid_inputs)
{
    int stuff_to_pass;

    AwaServerObservation * observation = AwaServerObservation_New(global::clientEndpointName, "/3/0/1", EmptyObserveCallback, (void*)&stuff_to_pass);
    ASSERT_TRUE(NULL != observation);
    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&observation));
    ASSERT_TRUE(NULL == observation);

    observation = AwaServerObservation_New(global::clientEndpointName, "/3/0/1", EmptyObserveCallback, NULL);
    ASSERT_TRUE(NULL != observation);
    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&observation));
    ASSERT_TRUE(NULL == observation);
}

TEST_F(TestObserveWithConnectedSession, AwaObservation_New_free_invalid)
{
    int stuff_to_pass;

    AwaServerObservation * observation = AwaServerObservation_New(global::clientEndpointName, NULL, EmptyObserveCallback, (void*)&stuff_to_pass);
    ASSERT_TRUE(NULL == observation);
    observation = AwaServerObservation_New(global::clientEndpointName, "/3/0/1", NULL, (void*)&stuff_to_pass);
    ASSERT_TRUE(NULL == observation);
    observation = AwaServerObservation_New(NULL, "/3/0/1", EmptyObserveCallback, (void*)&stuff_to_pass);
    ASSERT_TRUE(NULL == observation);
    ASSERT_EQ(AwaError_ObservationInvalid, AwaServerObservation_Free(NULL));
    AwaServerObservation ** observation2 = NULL;
    ASSERT_EQ(AwaError_ObservationInvalid, AwaServerObservation_Free(observation2));
}



TEST_F(TestObserveWithConnectedSession, AwaServerSession_Process_nothing_pending)
{
    ASSERT_EQ(AwaError_Success, AwaServerSession_Process(session_, defaults::timeout));
}

TEST_F(TestObserveWithConnectedSession, AwaServerSession_DispatchCallbacks_nothing_in_queue)
{
    AwaServerSession_DispatchCallbacks(session_);
}

TEST_F(TestObserveWithConnectedSession, AwaServerSession_ContainsPath_invalid_input)
{
    EXPECT_FALSE(AwaChangeSet_ContainsPath(NULL, NULL));
    EXPECT_FALSE(AwaChangeSet_ContainsPath(NULL, "/3/2/1"));
}

TEST_F(TestObserveWithConnectedSession, AwaServerSession_HasValue_invalid_input)
{
    EXPECT_FALSE(AwaChangeSet_HasValue(NULL, NULL));
    EXPECT_FALSE(AwaChangeSet_HasValue(NULL, "/3/2/1"));
}

TEST_F(TestObserveWithConnectedSession, AwaServerSession_GetChangeType_invalid_input)
{
    EXPECT_EQ(AwaChangeType_Invalid, AwaChangeSet_GetChangeType(NULL, NULL));
    EXPECT_EQ(AwaChangeType_Invalid, AwaChangeSet_GetChangeType(NULL, "/3/2/1"));
}

TEST_F(TestObserveWithConnectedSession, AwaServerSession_AwaChangeSet_NewPathIterator_invalid_inputs)
{
    EXPECT_EQ(NULL, AwaChangeSet_NewPathIterator(NULL));
}

TEST_F(TestObserveWithConnectedSession, AwaServerSession_AwaChangeSet_NewPathIterator_valid_inputs)
{
    struct ChangeCallbackHandler2 : public ObserveWaitCondition
    {
        int count;

        ChangeCallbackHandler2(AwaServerSession * session, int max)  : ObserveWaitCondition(session, max), count(0) {};

        void callbackHandler(const AwaChangeSet * changeSet)
        {
            count ++;

            ASSERT_TRUE(NULL != changeSet);
            AwaPathIterator * iterator = AwaChangeSet_NewPathIterator(changeSet);
            ASSERT_TRUE(NULL != iterator);

            // FIXME: the changeset should only contain paths to resources/objects that have changed!
            // call Next once before getting values:
            //EXPECT_EQ(true, AwaPathIterator_Next(iterator));
            //EXPECT_STREQ("/3/0/15", AwaPathIterator_Get(iterator));

            // not expecting any more paths
            //EXPECT_TRUE(false == AwaPathIterator_Next(iterator));
            //EXPECT_TRUE(NULL == AwaPathIterator_Get(iterator));

            AwaPathIterator_Free(&iterator);

        }
        void TestBody() {}
    };
    AwaClientDaemonHorde horde( { global::clientEndpointName }, global::clientIpcPort, CURRENT_TEST_DESCRIPTION);
    sleep(1);

    ChangeCallbackHandler2 cbHandler(session_, 2);

    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
    ASSERT_TRUE(NULL != operation);

    AwaServerObservation * changeObservation = AwaServerObservation_New(global::clientEndpointName, "/3/0/15", ObserveCallbackRunner, &cbHandler);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, changeObservation));

    EXPECT_EQ(AwaError_Success, AwaServerObserveOperation_Perform(operation, defaults::timeout));

    // set via server api to trigger notification.
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);
    ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsCString(writeOperation, "/3/0/15", "123414123"));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    cbHandler.count = 0;
    ASSERT_TRUE(cbHandler.Wait());
    ASSERT_EQ(2, cbHandler.count);

    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&changeObservation));
    ASSERT_TRUE(NULL == changeObservation);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}



// Test ChangeSet functions
//------------------------------
namespace observeDetail
{

struct TestObserveResource
{
    AwaError expectedResult;
    const void * expectedInitialValue;
    const void * expectedValue;
    int expectedValueSize;
    const char * path;
    AwaResourceType type;
};

::std::ostream& operator<<(::std::ostream& os, const TestObserveResource& item)
{
  return os << "Item: expectedResult " << item.expectedResult
            << ", expectedValue " << item.expectedValue
            << ", expectedValueSize " << item.expectedValueSize
            << ", path " << item.path
            << ", type " << item.type;
}

// initial object values
const AwaInteger initialInteger1 = 100;
const char * initialString1 = "Lightweight M2M Server";
const AwaFloat initialFloat1 = 1.337;
const AwaTime initialTime1 = 0xA20AD72B;
const AwaBoolean initialBoolean1 = true;
const char dummyOpaqueData2[] = {'a',0,'x','\0', 123};
const AwaOpaque initialOpaque1 = {(void*) dummyOpaqueData2, sizeof(dummyOpaqueData2)};
const AwaObjectLink initialObjectLink1 = { 3, 0 };

// change to this to trigger notification.
const AwaInteger expectedInteger1 = 150;
const char * expectedString1 = "Heavyweight M2M Server";
const AwaFloat expectedFloat1 = 7.331;
const AwaTime expectedTime1 = 0xB67AD72B;
const AwaBoolean expectedBoolean1 = false;

const char dummyOpaqueData[] = {'c',0,'d','\0', 124};
const AwaOpaque expectedOpaque1 = {(void*) dummyOpaqueData, sizeof(dummyOpaqueData)};
const AwaObjectLink expectedObjectLink1 = { 4, 0 };

} // namespace detail

class TestAwaObserveChangeSet : public TestObserveWithConnectedSession, public ::testing::WithParamInterface< observeDetail::TestObserveResource>
{
protected:

    void SetUp() {
        TestObserveWithConnectedSession::SetUp();

        horde_ = global::spawnClientDaemon ? new AwaClientDaemonHorde( { global::clientEndpointName }, 61000, CURRENT_TEST_DESCRIPTION) : NULL;
        sleep(1);      // wait for the client to register with the server

        clientSession_ = AwaClientSession_New(); ASSERT_TRUE(NULL != clientSession_);
        ASSERT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(clientSession_, "127.0.0.1", global::spawnClientDaemon? 61000 : global::clientIpcPort));
        ASSERT_EQ(AwaError_Success, AwaClientSession_Connect(clientSession_));

        AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(clientSession_);
        AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(session_);
        EXPECT_TRUE(clientDefineOperation != NULL);
        EXPECT_TRUE(serverDefineOperation != NULL);

        AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(10000, "Test Object", 0, 1);
        ASSERT_TRUE(NULL != customObjectDefinition);

        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsNoType(customObjectDefinition, 0, "Test None Resource", false, AwaResourceOperations_Execute));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsString(customObjectDefinition, 1, "Test String Resource", true, AwaResourceOperations_ReadWrite, observeDetail::expectedString1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(customObjectDefinition, 2, "Test Integer Resource", true, AwaResourceOperations_ReadWrite, observeDetail::expectedInteger1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloat(customObjectDefinition, 3, "Test Float Resource", true, AwaResourceOperations_ReadWrite, observeDetail::expectedFloat1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBoolean(customObjectDefinition, 4, "Test Boolean Resource", true, AwaResourceOperations_ReadWrite, observeDetail::expectedBoolean1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque(customObjectDefinition, 5, "Test Opaque Resource", true, AwaResourceOperations_ReadWrite, observeDetail::expectedOpaque1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTime(customObjectDefinition, 6, "Test Time Resource", true, AwaResourceOperations_ReadWrite, observeDetail::expectedTime1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLink(customObjectDefinition, 7, "Test ObjectLink Resource", true, AwaResourceOperations_ReadWrite, observeDetail::expectedObjectLink1));

        // define another object that we can use for "path not in result" tests
        AwaObjectDefinition * customObjectDefinition2 = AwaObjectDefinition_New(10001, "Test Object 2", 0, 1);
        ASSERT_TRUE(NULL != customObjectDefinition2);

        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsNoType(customObjectDefinition2, 0, "Test None Resource", false, AwaResourceOperations_Execute));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsString(customObjectDefinition2, 1, "Test String Resource", true, AwaResourceOperations_ReadWrite, observeDetail::expectedString1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(customObjectDefinition2, 2, "Test Integer Resource", true, AwaResourceOperations_ReadWrite, observeDetail::expectedInteger1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloat(customObjectDefinition2, 3, "Test Float Resource", true, AwaResourceOperations_ReadWrite, observeDetail::expectedFloat1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBoolean(customObjectDefinition2, 4, "Test Boolean Resource", true, AwaResourceOperations_ReadWrite, observeDetail::expectedBoolean1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque(customObjectDefinition2, 5, "Test Opaque Resource", true, AwaResourceOperations_ReadWrite, observeDetail::expectedOpaque1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTime(customObjectDefinition2, 6, "Test Time Resource", true, AwaResourceOperations_ReadWrite, observeDetail::expectedTime1));
        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLink(customObjectDefinition2, 7, "Test ObjectLink Resource", true, AwaResourceOperations_ReadWrite, observeDetail::expectedObjectLink1));

        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition));
        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, customObjectDefinition2));
        ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, defaults::timeout));

        EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
        EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition2));
        ASSERT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, defaults::timeout));

        AwaObjectDefinition_Free(&customObjectDefinition);
        AwaObjectDefinition_Free(&customObjectDefinition2);
        AwaClientDefineOperation_Free(&clientDefineOperation);
        AwaServerDefineOperation_Free(&serverDefineOperation);

        //FIXME: FLOWDM-498: this creates the object instance for the server write below to operate correctly
        AwaClientSetOperation * clientSet = AwaClientSetOperation_New(clientSession_);
        EXPECT_TRUE(clientSet != NULL);
        EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(clientSet, "/10000/0"));
        EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(clientSet, defaults::timeout));
        AwaClientSetOperation_Free(&clientSet);

        //Create a basic set operation to create our custom objects - TODO remove once we can set default values
        AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
        ASSERT_TRUE(NULL != writeOperation);

        ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsCString(writeOperation, "/10000/0/1", observeDetail::initialString1));
        ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/10000/0/2", observeDetail::initialInteger1));
        ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsFloat(writeOperation, "/10000/0/3", observeDetail::initialFloat1));
        ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsBoolean(writeOperation, "/10000/0/4", observeDetail::initialBoolean1));
        ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsOpaque(writeOperation, "/10000/0/5", observeDetail::initialOpaque1));
        ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsTime(writeOperation, "/10000/0/6", observeDetail::initialTime1));
        ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsObjectLink(writeOperation, "/10000/0/7", observeDetail::initialObjectLink1));
        ASSERT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
        AwaServerWriteOperation_Free(&writeOperation);


    }

    void TearDown()
    {
        AwaClientSession_Disconnect(clientSession_);
        AwaClientSession_Free(&clientSession_);
        if (horde_ != NULL)
        {
            delete horde_;
        }
        TestObserveWithConnectedSession::TearDown();
    }
    AwaClientDaemonHorde * horde_;
    AwaClientSession * clientSession_;
};

TEST_P(TestAwaObserveChangeSet, TestAwaObserveChangeSetInstantiation)
{
    observeDetail::TestObserveResource data = GetParam();

    struct ChangeCallbackHandler3 : public ObserveWaitCondition
    {
        int count;
        observeDetail::TestObserveResource data;

        explicit ChangeCallbackHandler3(observeDetail::TestObserveResource data, AwaServerSession * session, int max) : ObserveWaitCondition(session, max), count(0), data(data) {}

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

                const void * expected = count == 1? data.expectedInitialValue : data.expectedValue;
                switch(data.type)
                {
                    case AwaResourceType_String:
                    {
                        ASSERT_STREQ((char*) expected, (char*) expected);
                        break;
                    }
                    case AwaResourceType_Opaque:
                    {
                        AwaOpaque * expectedOpaque = (AwaOpaque *) expected;
                        AwaOpaque * receivedOpaquePointer = (AwaOpaque *) value;
                        ASSERT_EQ(expectedOpaque->Size, receivedOpaquePointer->Size);
                        ASSERT_EQ(expectedOpaque->Size, receivedOpaque.Size);
                        ASSERT_EQ(0, memcmp(expectedOpaque->Data, receivedOpaquePointer->Data, expectedOpaque->Size));
                        ASSERT_EQ(0, memcmp(expectedOpaque->Data, receivedOpaque.Data, receivedOpaque.Size));
                        break;
                    }
                    case AwaResourceType_Integer:
                        ASSERT_EQ(*static_cast<const AwaInteger *>(expected), *static_cast<AwaInteger *>(value));
                        break;
                    case AwaResourceType_Float:
                        ASSERT_EQ(*static_cast<const AwaFloat *>(expected), *static_cast<AwaFloat *>(value));
                        break;
                    case AwaResourceType_Boolean:
                        ASSERT_EQ(*static_cast<const AwaBoolean *>(expected), *static_cast<AwaBoolean *>(value));
                        break;
                    case AwaResourceType_Time:
                        ASSERT_EQ(*static_cast<const AwaTime *>(expected), *static_cast<AwaTime *>(value));
                        break;
                    case AwaResourceType_ObjectLink:
                    {
                        const AwaObjectLink * expectedObjectLink = static_cast<const AwaObjectLink *>(expected);
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
    };

    ChangeCallbackHandler3 cbHandler(data, session_, 2);

    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
    ASSERT_TRUE(NULL != operation);
    AwaServerObservation * observation = AwaServerObservation_New(global::clientEndpointName, data.path, ObserveCallbackRunner, &cbHandler);
    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, observation));
    EXPECT_EQ(AwaError_Success, AwaServerObserveOperation_Perform(operation, defaults::timeout));

    // set via server api to trigger notification.
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    ASSERT_TRUE(NULL != writeOperation);

    switch(data.type)
    {
        case AwaResourceType_String:
            ASSERT_EQ(data.expectedResult, AwaServerWriteOperation_AddValueAsCString(writeOperation, data.path, (const char *)data.expectedValue));
            break;
        case AwaResourceType_Integer:
            ASSERT_EQ(data.expectedResult, AwaServerWriteOperation_AddValueAsInteger(writeOperation, data.path, *static_cast<const AwaInteger *>(data.expectedValue)));
            break;
        case AwaResourceType_Float:
            ASSERT_EQ(data.expectedResult, AwaServerWriteOperation_AddValueAsFloat(writeOperation, data.path, *static_cast<const AwaFloat *>(data.expectedValue)));
            break;
        case AwaResourceType_Boolean:
            ASSERT_EQ(data.expectedResult, AwaServerWriteOperation_AddValueAsBoolean(writeOperation, data.path, *static_cast<const AwaBoolean *>(data.expectedValue)));
            break;
        case AwaResourceType_Opaque:
            ASSERT_EQ(data.expectedResult, AwaServerWriteOperation_AddValueAsOpaque(writeOperation, data.path, *static_cast<const AwaOpaque *>(data.expectedValue)));
            break;
        case AwaResourceType_Time:
            ASSERT_EQ(data.expectedResult, AwaServerWriteOperation_AddValueAsTime(writeOperation, data.path, *static_cast<const AwaTime *>(data.expectedValue)));
            break;
        case AwaResourceType_ObjectLink:
            ASSERT_EQ(data.expectedResult, AwaServerWriteOperation_AddValueAsObjectLink(writeOperation, data.path, *static_cast<const AwaObjectLink *>(data.expectedValue)));
            break;
        default:
            ASSERT_TRUE(false);
            break;
    }

    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout));
    AwaServerWriteOperation_Free(&writeOperation);

    // ensure we receive both notifications
    sleep(1);

    ASSERT_EQ(AwaError_Success, AwaServerSession_Process(session_, defaults::timeout));

    cbHandler.count = 0;
    AwaServerSession_DispatchCallbacks(session_);
    ASSERT_EQ(2, cbHandler.count);

    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&observation));
    ASSERT_TRUE(NULL == observation);

    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
    ASSERT_TRUE(NULL == operation);
}

INSTANTIATE_TEST_CASE_P(
        TestObserveResource,
        TestAwaObserveChangeSet,
        ::testing::Values(
          observeDetail::TestObserveResource {AwaError_Success,  (void *)observeDetail::initialString1,    (void *)observeDetail::expectedString1, -1,                              "/10000/0/1",   AwaResourceType_String},
          observeDetail::TestObserveResource {AwaError_Success,  &observeDetail::initialInteger1,          &observeDetail::expectedInteger1,       sizeof(AwaInteger),          "/10000/0/2",   AwaResourceType_Integer},
          observeDetail::TestObserveResource {AwaError_Success,  &observeDetail::initialFloat1,            &observeDetail::expectedFloat1,         sizeof(AwaFloat),            "/10000/0/3",   AwaResourceType_Float},
          observeDetail::TestObserveResource {AwaError_Success,  &observeDetail::initialBoolean1,          &observeDetail::expectedBoolean1,       sizeof(AwaBoolean),          "/10000/0/4",   AwaResourceType_Boolean},
          observeDetail::TestObserveResource {AwaError_Success,  &observeDetail::initialOpaque1,           &observeDetail::expectedOpaque1,        sizeof(observeDetail::expectedOpaque1), "/10000/0/5",   AwaResourceType_Opaque},
          observeDetail::TestObserveResource {AwaError_Success,  &observeDetail::initialTime1,             &observeDetail::expectedTime1,          sizeof(AwaTime),             "/10000/0/6",  AwaResourceType_Time},
          observeDetail::TestObserveResource {AwaError_Success,  &observeDetail::initialObjectLink1,       &observeDetail::expectedObjectLink1,    sizeof(AwaObjectLink),       "/10000/0/7",  AwaResourceType_ObjectLink}
        ));


///***********************************************************************************************************
// * GetValueArray parameterised tests
// */
//
//namespace observeDetail
//{
//
//struct TestAwaChangeSetResourceArray
//{
//    AwaError expectedResult;
//    const char * path;
//    bool UseResponse;
//    AwaResourceType type;
//};
//
//::std::ostream& operator<<(::std::ostream& os, const TestAwaChangeSetResourceArray& item)
//{
//  return os << "Item: expectedResult " << item.expectedResult
//            << ", path " << item.path
//            << ", UseResponse " << item.UseResponse
//            << ", type " << item.type;
//}
//
//}
//
//class TestAwaChangeSetArray : public TestObserveWithConnectedSession, public ::testing::WithParamInterface< observeDetail::TestAwaChangeSetResourceArray>
//{
//protected:
//
//    void SetUp() {
//        TestObserveWithConnectedSession::SetUp();
//
//        //Define and set our custom objects - TODO define in some common class for get, set etc?
//        AwaServerDefineOperation * defineOperation = AwaServerDefineOperation_New(session_);
//        EXPECT_TRUE(defineOperation != NULL);
//
//        AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(10000, "Test Object", 0, 1);
//        ASSERT_TRUE(NULL != customObjectDefinition);
//
//        expectedStringArray_ = AwaStringArray_New();
//        expectedIntegerArray_ = AwaIntegerArray_New();
//        expectedFloatArray_ = AwaFloatArray_New();
//        expectedBooleanArray_ = AwaBooleanArray_New();
//        expectedOpaqueArray_ = AwaOpaqueArray_New();
//        expectedTimeArray_ = AwaTimeArray_New();
//        expectedObjectLinkArray_ = AwaObjectLinkArray_New();
//
//        for (int i = 0; i < 10; i++)
//        {
//            char * stringValue;
//            msprintf(&stringValue, "%d", i*2);
//            AwaStringArray_SetValueAsCString(expectedStringArray_, i, (const char *)stringValue);
//            Awa_MemSafeFree(stringValue);
//
//            AwaIntegerArray_SetValue(expectedIntegerArray_, i, i*2);
//            AwaFloatArray_SetValue(expectedFloatArray_, i, i*2.5);
//            AwaBooleanArray_SetValue(expectedBooleanArray_, i, i%2==0);
//
//            static char dummyOpaqueData[] = {'a',0,'x','\0', (char)i};
//
//            AwaOpaque opaqueValue = {(void*) dummyOpaqueData, sizeof(dummyOpaqueData)};
//
//            AwaOpaqueArray_SetValue(expectedOpaqueArray_, i, opaqueValue);
//            AwaTimeArray_SetValue(expectedTimeArray_, i, i*100);
//
//            AwaObjectLink objectLinkValue = { 3, i };
//            AwaObjectLinkArray_SetValue(expectedObjectLinkArray_, i, objectLinkValue);
//        }
//
//        initialStringArray_ = AwaStringArray_New();
//        initialIntegerArray_ = AwaIntegerArray_New();
//        initialFloatArray_ = AwaFloatArray_New();
//        initialBooleanArray_ = AwaBooleanArray_New();
//        initialOpaqueArray_ = AwaOpaqueArray_New();
//        initialTimeArray_ = AwaTimeArray_New();
//        initialObjectLinkArray_ = AwaObjectLinkArray_New();
//
//        for (int i = 0; i < 10; i++)
//        {
//            char * stringValue;
//            msprintf(&stringValue, "%d", i*3);
//            AwaStringArray_SetValueAsCString(initialStringArray_, i, (const char *)stringValue);
//            Awa_MemSafeFree(stringValue);
//
//            AwaIntegerArray_SetValue(initialIntegerArray_, i, i*3);
//            AwaFloatArray_SetValue(initialFloatArray_, i, i*1.3);
//            AwaBooleanArray_SetValue(initialBooleanArray_, i, i%2!=0); // NB: Opposite of expected so that set will fire a notification
//
//            static char dummyOpaqueData[] = {'x',0,'b','\0', (char)i};
//
//            AwaOpaque opaqueValue = {(void*) dummyOpaqueData, sizeof(dummyOpaqueData)};
//
//            AwaOpaqueArray_SetValue(initialOpaqueArray_, i, opaqueValue);
//            AwaTimeArray_SetValue(initialTimeArray_, i, i*60);
//
//            AwaObjectLink objectLinkValue = { 4, i };
//            AwaObjectLinkArray_SetValue(initialObjectLinkArray_, i, objectLinkValue);
//        }
//
//        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsStringArray(customObjectDefinition, 1, "Test String Array Resource", 0, AWA_MAX_ID, AwaResourceOperations_ReadWrite, expectedStringArray_));
//        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsIntegerArray(customObjectDefinition, 2, "Test Integer Array Resource", 0, AWA_MAX_ID, AwaResourceOperations_ReadWrite, expectedIntegerArray_));
//        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloatArray(customObjectDefinition, 3, "Test Float Array Resource", 0, AWA_MAX_ID, AwaResourceOperations_ReadWrite, expectedFloatArray_));
//        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBooleanArray(customObjectDefinition, 4, "Test Boolean Array Resource", 0, AWA_MAX_ID, AwaResourceOperations_ReadWrite, expectedBooleanArray_));
//        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaqueArray(customObjectDefinition, 5, "Test Opaque Array Resource", 0, AWA_MAX_ID, AwaResourceOperations_ReadWrite, expectedOpaqueArray_));
//        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTimeArray(customObjectDefinition, 6, "Test Time Array Resource", 0, AWA_MAX_ID, AwaResourceOperations_ReadWrite, expectedTimeArray_));
//        ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLinkArray(customObjectDefinition, 7, "Test ObjectLink Array Resource", 0, AWA_MAX_ID, AwaResourceOperations_ReadWrite, expectedObjectLinkArray_));
//
//
//        EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(defineOperation, customObjectDefinition));
//        ASSERT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(defineOperation, defaults::timeout));
//
//        AwaObjectDefinition_Free(&customObjectDefinition);
//        AwaServerDefineOperation_Free(&defineOperation);
//
//        //Create a basic set operation to create our custom objects - TODO remove setting values once we can set default values
//        AwaServerSetOperation * setOperation = AwaServerSetOperation_New(session_);
//        ASSERT_TRUE(NULL != setOperation);
//
//        ASSERT_EQ(AwaError_Success, AwaServerSetOperation_CreateObjectInstance(setOperation, "/10000/0"));
//
//        ASSERT_EQ(AwaError_Success, AwaServerSetOperation_CreateOptionalResource(setOperation, "/10000/0/1"));
//        ASSERT_EQ(AwaError_Success, AwaServerSetOperation_CreateOptionalResource(setOperation, "/10000/0/2"));
//        ASSERT_EQ(AwaError_Success, AwaServerSetOperation_CreateOptionalResource(setOperation, "/10000/0/3"));
//        ASSERT_EQ(AwaError_Success, AwaServerSetOperation_CreateOptionalResource(setOperation, "/10000/0/4"));
//        ASSERT_EQ(AwaError_Success, AwaServerSetOperation_CreateOptionalResource(setOperation, "/10000/0/5"));
//        ASSERT_EQ(AwaError_Success, AwaServerSetOperation_CreateOptionalResource(setOperation, "/10000/0/6"));
//        ASSERT_EQ(AwaError_Success, AwaServerSetOperation_CreateOptionalResource(setOperation, "/10000/0/7"));
//
//        ASSERT_EQ(AwaError_Success, AwaServerSetOperation_AddValueAsStringArray(setOperation, "/10000/0/1", initialStringArray_));
//        ASSERT_EQ(AwaError_Success, AwaServerSetOperation_AddValueAsIntegerArray(setOperation, "/10000/0/2", initialIntegerArray_));
//        ASSERT_EQ(AwaError_Success, AwaServerSetOperation_AddValueAsFloatArray(setOperation, "/10000/0/3", initialFloatArray_));
//        ASSERT_EQ(AwaError_Success, AwaServerSetOperation_AddValueAsBooleanArray(setOperation, "/10000/0/4", initialBooleanArray_));
//        ASSERT_EQ(AwaError_Success, AwaServerSetOperation_AddValueAsOpaqueArray(setOperation, "/10000/0/5", initialOpaqueArray_));
//        ASSERT_EQ(AwaError_Success, AwaServerSetOperation_AddValueAsTimeArray(setOperation, "/10000/0/6", initialTimeArray_));
//        ASSERT_EQ(AwaError_Success, AwaServerSetOperation_AddValueAsObjectLinkArray(setOperation, "/10000/0/7", initialObjectLinkArray_));
//
//        ASSERT_EQ(AwaError_Success, AwaServerSetOperation_Perform(setOperation, defaults::timeout));
//        AwaServerSetOperation_Free(&setOperation);
//    }
//
//    void TearDown()
//    {
//        TestObserveWithConnectedSession::TearDown();
//
//        AwaStringArray_Free(&expectedStringArray_);
//        AwaIntegerArray_Free(&expectedIntegerArray_);
//        AwaFloatArray_Free(&expectedFloatArray_);
//        AwaBooleanArray_Free(&expectedBooleanArray_);
//        AwaOpaqueArray_Free(&expectedOpaqueArray_);
//        AwaTimeArray_Free(&expectedTimeArray_);
//        AwaObjectLinkArray_Free(&expectedObjectLinkArray_);
//
//        AwaStringArray_Free(&initialStringArray_);
//        AwaIntegerArray_Free(&initialIntegerArray_);
//        AwaFloatArray_Free(&initialFloatArray_);
//        AwaBooleanArray_Free(&initialBooleanArray_);
//        AwaOpaqueArray_Free(&initialOpaqueArray_);
//        AwaTimeArray_Free(&initialTimeArray_);
//        AwaObjectLinkArray_Free(&initialObjectLinkArray_);
//    }
//
//    AwaStringArray * expectedStringArray_;
//    AwaIntegerArray * expectedIntegerArray_;
//    AwaFloatArray * expectedFloatArray_;
//    AwaBooleanArray * expectedBooleanArray_;
//    AwaOpaqueArray * expectedOpaqueArray_;
//    AwaTimeArray * expectedTimeArray_;
//    AwaObjectLinkArray * expectedObjectLinkArray_;
//
//    AwaStringArray * initialStringArray_;
//    AwaIntegerArray * initialIntegerArray_;
//    AwaFloatArray * initialFloatArray_;
//    AwaBooleanArray * initialBooleanArray_;
//    AwaOpaqueArray * initialOpaqueArray_;
//    AwaTimeArray * initialTimeArray_;
//    AwaObjectLinkArray * initialObjectLinkArray_;
//};
//
//TEST_P(TestAwaChangeSetArray, TestAwaChangeSetArrayInstantiation)
//{
//  observeDetail::TestAwaChangeSetResourceArray data = GetParam();
//
//    struct ChangeCallbackHandler4 : public TestObserveWithConnectedSession
//    {
//        int count;
//        observeDetail::TestAwaChangeSetResourceArray data;
//        AwaStringArray * expectedStringArray_;
//        AwaIntegerArray * expectedIntegerArray_;
//        AwaFloatArray * expectedFloatArray_;
//        AwaBooleanArray * expectedBooleanArray_;
//        AwaOpaqueArray * expectedOpaqueArray_;
//        AwaTimeArray * expectedTimeArray_;
//        AwaObjectLinkArray * expectedObjectLinkArray_;
//
//        ChangeCallbackHandler4(observeDetail::TestAwaChangeSetResourceArray data)
//            : count(0)
//        {
//            this->data = data;
//            this->expectedStringArray_ = NULL;
//            this->expectedIntegerArray_ = NULL;
//            this->expectedFloatArray_ = NULL;
//            this->expectedBooleanArray_ = NULL;
//            this->expectedOpaqueArray_ = NULL;
//            this->expectedTimeArray_ = NULL;
//            this->expectedObjectLinkArray_ = NULL;
//        }
//
//        void callbackHandler(const AwaChangeSet * changeSet)
//        {
//            AwaArray * expectedArray = NULL;
//            AwaArray * array = NULL;
//
//            count ++;
//
//            ASSERT_TRUE(NULL != changeSet);
//
//            switch(data.type)
//            {
//               case AwaResourceType_StringArray:
//               {
//                   expectedArray = (AwaArray *)this->expectedStringArray_;
//                   ASSERT_EQ(data.expectedResult, AwaChangeSet_GetValuesAsStringArrayPointer(changeSet, data.path, (const AwaStringArray **)&array));
//                   break;
//               }
//               case AwaResourceType_IntegerArray:
//               {
//                   expectedArray = (AwaArray *)this->expectedIntegerArray_;
//                   ASSERT_EQ(data.expectedResult, AwaChangeSet_GetValuesAsIntegerArrayPointer(changeSet, data.path, (const AwaIntegerArray **)&array));
//                   break;
//               }
//               case AwaResourceType_FloatArray:
//               {
//                   expectedArray = (AwaArray *)this->expectedFloatArray_;
//                   ASSERT_EQ(data.expectedResult, AwaChangeSet_GetValuesAsFloatArrayPointer(changeSet, data.path, (const AwaFloatArray **)&array));
//                   break;
//               }
//               case AwaResourceType_BooleanArray:
//               {
//                   expectedArray = (AwaArray *)this->expectedBooleanArray_;
//                   ASSERT_EQ(data.expectedResult, AwaChangeSet_GetValuesAsBooleanArrayPointer(changeSet, data.path, (const AwaBooleanArray **)&array));
//                   break;
//               }
//               case AwaResourceType_OpaqueArray:
//               {
//                   expectedArray = (AwaArray *)this->expectedOpaqueArray_;
//                   ASSERT_EQ(data.expectedResult, AwaChangeSet_GetValuesAsOpaqueArrayPointer(changeSet, data.path, (const AwaOpaqueArray **)&array));
//                   break;
//               }
//               case AwaResourceType_TimeArray:
//               {
//                   expectedArray = (AwaArray *)this->expectedTimeArray_;
//                   ASSERT_EQ(data.expectedResult, AwaChangeSet_GetValuesAsTimeArrayPointer(changeSet, data.path, (const AwaTimeArray **)&array));
//                   break;
//               }
//               case AwaResourceType_ObjectLinkArray:
//               {
//                   expectedArray = (AwaArray *)this->expectedObjectLinkArray_;
//                   ASSERT_EQ(data.expectedResult, AwaChangeSet_GetValuesAsObjectLinkArrayPointer(changeSet, data.path, (const AwaObjectLinkArray **)&array));
//                   break;
//               }
//               default:
//                   ASSERT_TRUE(false);
//                   break;
//           }
//           ASSERT_TRUE((data.expectedResult == AwaError_Success) == (array != NULL));
//           if (array != NULL)
//           {
//               ASSERT_EQ(0, Array_Compare(expectedArray, array));
//           }
//        }
//        void TestBody() {}
//    };
//
//    ChangeCallbackHandler4 cbHandler(data);
//    cbHandler.expectedStringArray_ = this->expectedStringArray_;
//    cbHandler.expectedIntegerArray_ = this->expectedIntegerArray_;
//    cbHandler.expectedFloatArray_ = this->expectedFloatArray_;
//    cbHandler.expectedBooleanArray_ = this->expectedBooleanArray_;
//    cbHandler.expectedOpaqueArray_ = this->expectedOpaqueArray_;
//    cbHandler.expectedTimeArray_ = this->expectedTimeArray_;
//    cbHandler.expectedObjectLinkArray_ = this->expectedObjectLinkArray_;
//
//
//    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session_);
//    ASSERT_TRUE(NULL != operation);
//    AwaServerObservation * observation = AwaServerObservation_New(data.path, ChangeCallbackRunner, &cbHandler);
//    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_AddObservation(operation, observation));
//    EXPECT_EQ(AwaError_Success, AwaServerObserveOperation_Perform(operation, defaults::timeout));
//
//    // set via server api to trigger notification.
//    AwaServerSetOperation * setOperation = AwaServerSetOperation_New(session_);
//    ASSERT_TRUE(NULL != setOperation);
//
//    switch(data.type)
//    {
//        case AwaResourceType_StringArray:
//            ASSERT_EQ(AwaError_Success, AwaServerSetOperation_AddValueAsStringArray(setOperation, "/10000/0/1", expectedStringArray_));
//            break;
//        case AwaResourceType_IntegerArray:
//             ASSERT_EQ(AwaError_Success, AwaServerSetOperation_AddValueAsIntegerArray(setOperation, "/10000/0/2", expectedIntegerArray_));
//            break;
//        case AwaResourceType_FloatArray:
//            ASSERT_EQ(AwaError_Success, AwaServerSetOperation_AddValueAsFloatArray(setOperation, "/10000/0/3", expectedFloatArray_));
//            break;
//        case AwaResourceType_BooleanArray:
//            ASSERT_EQ(AwaError_Success, AwaServerSetOperation_AddValueAsBooleanArray(setOperation, "/10000/0/4", expectedBooleanArray_));
//            break;
//        case AwaResourceType_OpaqueArray:
//            ASSERT_EQ(AwaError_Success, AwaServerSetOperation_AddValueAsOpaqueArray(setOperation, "/10000/0/5", expectedOpaqueArray_));
//            break;
//        case AwaResourceType_TimeArray:
//             ASSERT_EQ(AwaError_Success, AwaServerSetOperation_AddValueAsTimeArray(setOperation, "/10000/0/6", expectedTimeArray_));
//            break;
//        case AwaResourceType_ObjectLinkArray:
//            ASSERT_EQ(AwaError_Success, AwaServerSetOperation_AddValueAsObjectLinkArray(setOperation, "/10000/0/7", expectedObjectLinkArray_));
//            break;
//        default:
//            ASSERT_TRUE(false);
//            break;
//    }
//
//    EXPECT_EQ(AwaError_Success, AwaServerSetOperation_Perform(setOperation, defaults::timeout));
//    AwaServerSetOperation_Free(&setOperation);
//
//    ASSERT_EQ(AwaError_Success, AwaServerSession_Process(session_, defaults::timeout));
//
//    cbHandler.count = 0;
//    AwaServerSession_DispatchCallbacks(session_);
//    ASSERT_EQ(1, cbHandler.count);
//
//    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&observation));
//    ASSERT_TRUE(NULL == observation);
//
//    ASSERT_EQ(AwaError_Success, AwaServerObserveOperation_Free(&operation));
//    ASSERT_TRUE(NULL == operation);
//}
//
//
//INSTANTIATE_TEST_CASE_P(
//        TestAwaChangeSetArraySuccessInstantiation,
//        TestAwaChangeSetArray,
//        ::testing::Values(
//          observeDetail::TestAwaChangeSetResourceArray {AwaError_Success,           "/10000/0/1",   true,   AwaResourceType_StringArray},
//          observeDetail::TestAwaChangeSetResourceArray {AwaError_Success,           "/10000/0/2",   true,   AwaResourceType_IntegerArray},
//          observeDetail::TestAwaChangeSetResourceArray {AwaError_Success,           "/10000/0/3",   true,   AwaResourceType_FloatArray},
//          observeDetail::TestAwaChangeSetResourceArray {AwaError_Success,           "/10000/0/4",   true,   AwaResourceType_BooleanArray},
//          observeDetail::TestAwaChangeSetResourceArray {AwaError_Success,           "/10000/0/5",   true,   AwaResourceType_OpaqueArray},
//          observeDetail::TestAwaChangeSetResourceArray {AwaError_Success,           "/10000/0/6",   true,   AwaResourceType_TimeArray},
//          observeDetail::TestAwaChangeSetResourceArray {AwaError_Success,           "/10000/0/7",   true,   AwaResourceType_ObjectLinkArray}
//        ));


TEST_F(TestObserveWithConnectedSession, AwaObservation_GetPath_valid_invalid)
{
    ASSERT_TRUE(NULL == AwaServerObservation_GetPath(NULL));
    AwaServerObservation * observation = AwaServerObservation_New(global::clientEndpointName, "/3/0/4", EmptyObserveCallback, NULL);
    ASSERT_TRUE(NULL != observation);
    ASSERT_STREQ("/3/0/4", AwaServerObservation_GetPath(observation));
    ASSERT_EQ(AwaError_Success, AwaServerObservation_Free(&observation));
    ASSERT_TRUE(NULL == observation);
    ASSERT_TRUE(NULL == AwaServerObservation_GetPath(observation));
}


}


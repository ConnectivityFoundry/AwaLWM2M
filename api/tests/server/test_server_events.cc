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

#include "support/support.h"
#include "support/static_api_support.h"
#include "server_events.h"

namespace Awa {

class TestServerEvents : public TestAwaBase {};

TEST_F(TestServerEvents, ServerEventsCallbackInfo_New_and_Free)
{
    ServerEventsCallbackInfo * serverEvents = ServerEventsCallbackInfo_New();
    ASSERT_TRUE(NULL != serverEvents);
    ServerEventsCallbackInfo_Free(&serverEvents);
    ASSERT_EQ(NULL, serverEvents);
}

TEST_F(TestServerEvents, ServerEventsCallbackInfo_Free_handles_null)
{
    ServerEventsCallbackInfo_Free(NULL);
}

//class TestServerEventsWithConnectedSession : public TestStaticClientWithServer {};
class TestServerEventsWithConnectedSession : public TestServerWithConnectedSession {};

namespace detail {

struct CallbackRecord {
    CallbackRecord() :
        RegisterCallbackCounter(0),
        DeregisterCallbackCounter(0),
        UpdateCallbackCounter(0),
        LastEvent(nullptr) {};
    int RegisterCallbackCounter;
    int DeregisterCallbackCounter;
    int UpdateCallbackCounter;
    const void * LastEvent;
};

static void RegisterEventCallback(const AwaServerClientRegisterEvent * event, void * context) {
    std::cout << "RegisterEventCallback: event " << event << ", context " << context << std::endl;
    CallbackRecord * record = static_cast<CallbackRecord *>(context);
    record->RegisterCallbackCounter++;
    record->LastEvent = event;
}

} // namespace detail


// ClientRegisterEvent / AwaServerClientRegisterEvent tests:

class TestServerEventsClientRegisterEvent : public TestAwaBase {};

TEST_F(TestServerEventsClientRegisterEvent, ClientRegisterEvent_New_and_Free)
{
    ClientRegisterEvent * event = ClientRegisterEvent_New();
    EXPECT_TRUE(NULL != event);
    ClientRegisterEvent_Free(&event);
    EXPECT_EQ(NULL, event);
}



// ServerEventsCallbackInfo tests:

class TestServerEventsCallbackInfo : public TestAwaBase {};

TEST_F(TestServerEventsCallbackInfo, ServerEventsCallbackInfo_SetClientRegisterCallback_handles_null)
{
    EXPECT_EQ(-1, ServerEventsCallbackInfo_SetClientRegisterCallback(NULL, NULL, NULL));
    EXPECT_EQ(-1, ServerEventsCallbackInfo_SetClientRegisterCallback(NULL, detail::RegisterEventCallback, NULL));
}

TEST_F(TestServerEventsCallbackInfo, ServerEventsCallbackInfo_SetClientRegisterCallback_handles_valid)
{
    ServerEventsCallbackInfo * info = ServerEventsCallbackInfo_New();
    ASSERT_TRUE(NULL != info);
    EXPECT_EQ(0, ServerEventsCallbackInfo_SetClientRegisterCallback(info, NULL, NULL));  // disable a callback
    EXPECT_EQ(0, ServerEventsCallbackInfo_SetClientRegisterCallback(info, detail::RegisterEventCallback, NULL));
    int data = 0;
    EXPECT_EQ(0, ServerEventsCallbackInfo_SetClientRegisterCallback(info, detail::RegisterEventCallback, &data));
    ServerEventsCallbackInfo_Free(&info);
    EXPECT_EQ(NULL, info);
}

TEST_F(TestServerEventsCallbackInfo, ServerEventsCallbackInfo_InvokeClientUpdateCallback_handles_null)
{
    EXPECT_EQ(-1, ServerEventsCallbackInfo_InvokeClientRegisterCallback(NULL, NULL));

    ServerEventsCallbackInfo * info = ServerEventsCallbackInfo_New();
    EXPECT_EQ(-1, ServerEventsCallbackInfo_InvokeClientRegisterCallback(info, NULL));
    ServerEventsCallbackInfo_Free(&info);
}

TEST_F(TestServerEventsCallbackInfo, ServerEventsCallbackInfo_InvokeClientUpdateCallback)
{
    ServerEventsCallbackInfo * info = ServerEventsCallbackInfo_New();
    ASSERT_TRUE(NULL != info);
    detail::CallbackRecord record;
    EXPECT_EQ(0, ServerEventsCallbackInfo_SetClientRegisterCallback(info, detail::RegisterEventCallback, &record));
    ASSERT_EQ(0, record.RegisterCallbackCounter);

    ClientRegisterEvent * event = ClientRegisterEvent_New();
    EXPECT_EQ(0, ServerEventsCallbackInfo_InvokeClientRegisterCallback(info, event));
    EXPECT_EQ(1, record.RegisterCallbackCounter);
    EXPECT_EQ(0, record.DeregisterCallbackCounter);
    EXPECT_EQ(0, record.UpdateCallbackCounter);
    EXPECT_EQ(event, record.LastEvent);
    ClientRegisterEvent_Free(&event);

    ServerEventsCallbackInfo_Free(&info);
}







TEST_F(TestServerEventsCallbackInfo, ServerEventsCallbackInfo_SetClientDeregisterCallback_handles_null)
{
    EXPECT_EQ(-1, ServerEventsCallbackInfo_SetClientDeregisterCallback(NULL, NULL, NULL));
    EXPECT_EQ(-1, ServerEventsCallbackInfo_SetClientDeregisterCallback(NULL, detail::RegisterEventCallback, NULL));
}

TEST_F(TestServerEventsCallbackInfo, ServerEventsCallbackInfo_SetClientDeregisterCallback_handles_valid)
{
    ServerEventsCallbackInfo * info = ServerEventsCallbackInfo_New();
    ASSERT_TRUE(NULL != info);
    EXPECT_EQ(0, ServerEventsCallbackInfo_SetClientDeregisterCallback(info, NULL, NULL));  // disable a callback
    EXPECT_EQ(0, ServerEventsCallbackInfo_SetClientDeregisterCallback(info, detail::RegisterEventCallback, NULL));
    int data = 0;
    EXPECT_EQ(0, ServerEventsCallbackInfo_SetClientDeregisterCallback(info, detail::RegisterEventCallback, &data));
    ServerEventsCallbackInfo_Free(&info);
    EXPECT_EQ(NULL, info);
}

TEST_F(TestServerEventsCallbackInfo, ServerEventsCallbackInfo_SetClientUpdateCallback_handles_null)
{
    EXPECT_EQ(-1, ServerEventsCallbackInfo_SetClientUpdateCallback(NULL, NULL, NULL));
    EXPECT_EQ(-1, ServerEventsCallbackInfo_SetClientUpdateCallback(NULL, detail::RegisterEventCallback, NULL));
}

TEST_F(TestServerEventsCallbackInfo, ServerEventsCallbackInfo_SetClientUpdateCallback_handles_valid)
{
    ServerEventsCallbackInfo * info = ServerEventsCallbackInfo_New();
    ASSERT_TRUE(NULL != info);
    EXPECT_EQ(0, ServerEventsCallbackInfo_SetClientUpdateCallback(info, NULL, NULL));  // disable a callback
    EXPECT_EQ(0, ServerEventsCallbackInfo_SetClientUpdateCallback(info, detail::RegisterEventCallback, NULL));
    int data = 0;
    EXPECT_EQ(0, ServerEventsCallbackInfo_SetClientUpdateCallback(info, detail::RegisterEventCallback, &data));
    ServerEventsCallbackInfo_Free(&info);
    EXPECT_EQ(NULL, info);
}





// API tests

TEST_F(TestServerEventsWithConnectedSession, AwaServerSession_SetClientRegisterEventCallback_invalid_session)
{
    EXPECT_EQ(AwaError_SessionInvalid, AwaServerSession_SetClientRegisterEventCallback(NULL, NULL, NULL));
}

TEST_F(TestServerEventsWithConnectedSession, AwaServerSession_SetClientRegisterEventCallback_valid_null)
{
    detail::CallbackRecord record;
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetClientRegisterEventCallback(session_, NULL, NULL));
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetClientRegisterEventCallback(session_, NULL, &record));
    EXPECT_EQ(0, record.RegisterCallbackCounter);
    EXPECT_EQ(0, record.DeregisterCallbackCounter);
    EXPECT_EQ(0, record.UpdateCallbackCounter);
    EXPECT_EQ(nullptr, record.LastEvent);
}

TEST_F(TestServerEventsWithConnectedSession, AwaServerSession_SetRegisterEventCallback_valid)
{
    detail::CallbackRecord record;
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetClientRegisterEventCallback(session_, detail::RegisterEventCallback, &record));
    EXPECT_EQ(0, record.RegisterCallbackCounter);
    EXPECT_EQ(0, record.DeregisterCallbackCounter);
    EXPECT_EQ(0, record.UpdateCallbackCounter);
    EXPECT_EQ(nullptr, record.LastEvent);
}

// test register client



//struct EventPollCondition : public PollCondition
//{
//    AwaStaticClient * StaticClient;
//    AwaServerListClientsOperation * Operation;
//    std::string ClientEndpointName;
//
//    EventPollCondition(AwaStaticClient * StaticClient, AwaServerListClientsOperation * Operation, std::string ClientEndpointName, int maxCount) :
//        PollCondition(maxCount), StaticClient(StaticClient), Operation(Operation), ClientEndpointName(ClientEndpointName) {}
//    virtual ~SingleStaticClientPollCondition() {}
//
//    static void RegisterEventCallback(const AwaServerClientRegisterEvent * event, void * context) {
//        CallbackRecord * record = static_cast<CallbackRecord *>(context);
//
//        event->
//        record->callbackCounter++;
//    }
//
//    virtual bool Check()
//    {
//        bool found = false;
//
//        EXPECT_EQ(AwaError_Success, AwaServerListClientsOperation_Perform(Operation, defaults::timeout));
//        AwaClientIterator * iterator = AwaServerListClientsOperation_NewClientIterator(Operation);
//        EXPECT_TRUE(iterator != NULL);
//        if (AwaClientIterator_Next(iterator))
//        {
//            if (ClientEndpointName.compare(AwaClientIterator_GetClientID(iterator)) == 0)
//            {
//                found = true;
//            }
//        }
//        AwaClientIterator_Free(&iterator);
//        AwaStaticClient_Process(StaticClient);
//        return found;
//    }
//};
//
//TEST_F(TestServerEventsWithConnectedSession, ClientRegisterEvent)
//{
//    detail::CallbackRecord record;
//    EXPECT_EQ(AwaError_Success, AwaServerSession_SetClientRegisterEventCallback(session_, detail::RegisterEventCallback, &record));
//    EXPECT_EQ(0, record.callbackCounter);
//
//    SingleStaticClientPollCondition condition(client_, operation, global::clientEndpointName, 20);
//    ASSERT_TRUE(condition.Wait());
//}

// test overwrite callback

// test clear callback








} // namespace Awa

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

TEST_F(TestServerEvents, ServerEvents_New_and_Free)
{
    ServerEventsCallbackInfo * serverEvents = ServerEvents_New();
    ASSERT_TRUE(NULL != serverEvents);
    ServerEvents_Free(&serverEvents);
    ASSERT_EQ(NULL, serverEvents);
}

TEST_F(TestServerEvents, ServerEvents_Free_handles_null)
{
    ServerEvents_Free(NULL);
}


//class TestServerEventsWithConnectedSession : public TestStaticClientWithServer {};
class TestServerEventsWithConnectedSession : public TestServerWithConnectedSession {};

namespace detail {

struct CallbackRecord {
    CallbackRecord() : callbackCounter(0) {};
    int callbackCounter;
};

static void RegisterEventCallback(const AwaServerClientRegisterEvent * event, void * context) {
    CallbackRecord * record = static_cast<CallbackRecord *>(context);
    record->callbackCounter++;
}

} // namespace detail


TEST_F(TestServerEventsWithConnectedSession, AwaServerSession_SetClientRegisterEventCallback_invalid_session)
{
    EXPECT_EQ(AwaError_SessionInvalid, AwaServerSession_SetClientRegisterEventCallback(NULL, NULL, NULL));
}

TEST_F(TestServerEventsWithConnectedSession, AwaServerSession_SetClientRegisterEventCallback_valid_null)
{
    detail::CallbackRecord record;
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetClientRegisterEventCallback(session_, NULL, NULL));
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetClientRegisterEventCallback(session_, NULL, &record));
    EXPECT_EQ(0, record.callbackCounter);
}

TEST_F(TestServerEventsWithConnectedSession, AwaServerSession_SetRegisterEventCallback_valid)
{
    detail::CallbackRecord record;
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetClientRegisterEventCallback(session_, detail::RegisterEventCallback, &record));
    EXPECT_EQ(0, record.callbackCounter);
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

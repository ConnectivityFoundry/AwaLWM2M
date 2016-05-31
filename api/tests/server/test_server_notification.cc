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
#include "server_notification.h"
#include "server_events.h"
#include "ipc.h"
#include "xmltree.h"
#include "memalloc.h"

namespace Awa {

class TestServerNotification : public TestServerWithSession {};

TEST_F(TestServerNotification, ServerNotification_Process_handles_invalid_session)
{
    EXPECT_EQ(AwaError_SessionInvalid, ServerNotification_Process(NULL, NULL));

    IPCMessage * notification = IPCMessage_New();
    EXPECT_EQ(AwaError_SessionInvalid, ServerNotification_Process(NULL, notification));
    IPCMessage_Free(&notification);
}

TEST_F(TestServerNotification, ServerNotification_Process_handles_invalid_notification)
{
    EXPECT_EQ(AwaError_IPCError, ServerNotification_Process(session_, NULL));
}

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
    EXPECT_TRUE(NULL != context);
    CallbackRecord * record = static_cast<CallbackRecord *>(context);
    record->RegisterCallbackCounter++;
    record->LastEvent = event;

//    AwaClientIterator * clientIterator = AwaServerClientRegisterEvent_NewClientIterator(event);
//    EXPECT_TRUE(NULL != clientIterator);
//
//    while (AwaClientIterator_Next(clientIterator))
//    {
//        const char * clientID = AwaClientIterator_GetClientID(clientIterator);
//        std::cout << "Client ID: " << clientID << std::endl;
//
//        AwaRegisteredEntityIterator * entityIterator = AwaServerClientRegisterEvent_NewRegisteredEntityIterator(event, clientID);
//        while (AwaRegisteredEntityIterator_Next(entityIterator))
//        {
//            const char * path = AwaRegisteredEntityIterator_GetPath(entityIterator);
//            std::cout << "  " << path << std::endl;
//        }
//        AwaRegisteredEntityIterator_Free(&entityIterator);
//    }
//    AwaClientIterator_Free(&clientIterator);
}

static void UpdateEventCallback(const AwaServerClientUpdateEvent * event, void * context) {
    EXPECT_TRUE(NULL != context);
    CallbackRecord * record = static_cast<CallbackRecord *>(context);
    record->UpdateCallbackCounter++;
    record->LastEvent = event;
}

static void DeregisterEventCallback(const AwaServerClientDeregisterEvent * event, void * context) {
    EXPECT_TRUE(NULL != context);
    CallbackRecord * record = static_cast<CallbackRecord *>(context);
    record->DeregisterCallbackCounter++;
    record->LastEvent = event;
}

} // namespace detail

TEST_F(TestServerNotification, ServerNotification_Process_handles_client_register_notification)
{
    const char * xml =
            "<Clients>"
            "  <Client>"
            "    <ID>imagination1</ID>"
            "    <Objects>"
            "      <Object>"
            "        <ID>1</ID>"
            "        <ObjectInstance>"
            "          <ID>0</ID>"
            "        </ObjectInstance>"
            "      </Object>"
            "    </Objects>"
            "  </Client>"
            "</Clients>";

    TreeNode contentNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != contentNode);
    IPCMessage * notification = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_NOTIFICATION, IPC_MESSAGE_SUB_TYPE_CLIENT_REGISTER, 1234567);
    ASSERT_TRUE(NULL != notification);
    IPCMessage_AddContent(notification, contentNode);

    detail::CallbackRecord record;
    AwaServerSession_SetClientRegisterEventCallback(session_, detail::RegisterEventCallback, &record);
    EXPECT_EQ(AwaError_Success, ServerNotification_Process(session_, notification));

    // check correct callback fired
    EXPECT_EQ(1, record.RegisterCallbackCounter);
    EXPECT_EQ(0, record.DeregisterCallbackCounter);
    EXPECT_EQ(0, record.UpdateCallbackCounter);

    // check Event
    auto event = static_cast<const ClientRegisterEvent *>(record.LastEvent);
    ASSERT_TRUE(NULL != event);

    IPCMessage_Free(&notification);
    Tree_Delete(contentNode);
}

TEST_F(TestServerNotification, ServerNotification_Process_handles_client_update_notification)
{
    const char * xml =
            "<Clients>"
            "  <Client>"
            "    <ID>imagination1</ID>"
            "    <Objects>"
            "      <Object>"
            "        <ID>1</ID>"
            "        <ObjectInstance>"
            "          <ID>0</ID>"
            "        </ObjectInstance>"
            "      </Object>"
            "    </Objects>"
            "  </Client>"
            "</Clients>";

    TreeNode contentNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != contentNode);
    IPCMessage * notification = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_NOTIFICATION, IPC_MESSAGE_SUB_TYPE_CLIENT_UPDATE, 1234568);
    ASSERT_TRUE(NULL != notification);
    IPCMessage_AddContent(notification, contentNode);

    detail::CallbackRecord record;
    AwaServerSession_SetClientUpdateEventCallback(session_, detail::UpdateEventCallback, &record);
    EXPECT_EQ(AwaError_Success, ServerNotification_Process(session_, notification));

    // check correct callback fired
    EXPECT_EQ(0, record.RegisterCallbackCounter);
    EXPECT_EQ(0, record.DeregisterCallbackCounter);
    EXPECT_EQ(1, record.UpdateCallbackCounter);

    // check Event
    auto event = static_cast<const ClientUpdateEvent *>(record.LastEvent);
    ASSERT_TRUE(NULL != event);

    IPCMessage_Free(&notification);
    Tree_Delete(contentNode);
}

TEST_F(TestServerNotification, ServerNotification_Process_handles_client_deregister_notification)
{
    const char * xml =
            "<Clients>"
            "  <Client>"
            "    <ID>imagination1</ID>"
            "  </Client>"
            "</Clients>";

    TreeNode contentNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != contentNode);
    IPCMessage * notification = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_NOTIFICATION, IPC_MESSAGE_SUB_TYPE_CLIENT_DEREGISTER, 1234569);
    ASSERT_TRUE(NULL != notification);
    IPCMessage_AddContent(notification, contentNode);

    detail::CallbackRecord record;
    AwaServerSession_SetClientDeregisterEventCallback(session_, detail::DeregisterEventCallback, &record);
    EXPECT_EQ(AwaError_Success, ServerNotification_Process(session_, notification));

    // check correct callback fired
    EXPECT_EQ(0, record.RegisterCallbackCounter);
    EXPECT_EQ(1, record.DeregisterCallbackCounter);
    EXPECT_EQ(0, record.UpdateCallbackCounter);

    // check Event
    auto event = static_cast<const ClientDeregisterEvent *>(record.LastEvent);
    ASSERT_TRUE(NULL != event);

    IPCMessage_Free(&notification);
    Tree_Delete(contentNode);
}

} // namespace Awa

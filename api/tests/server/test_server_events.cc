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

#include "support/support.h"
#include "support/static_api_support.h"
#include "server_events.h"
#include "xmltree.h"
#include "memalloc.h"
#include "client_iterator.h"
#include "registered_entity_iterator.h"

namespace Awa {

class TestServerEvents : public TestAwaBase {};

class TestServerEventsWithSession : public TestServerWithSession {};

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

static void ClientRegisterEventCallback(const AwaServerClientRegisterEvent * event, void * context) {
    std::cout << "RegisterEventCallback: event " << event << ", context " << context << std::endl;
    CallbackRecord * record = static_cast<CallbackRecord *>(context);
    record->RegisterCallbackCounter++;
    record->LastEvent = event;
}

static void ClientDeregisterEventCallback(const AwaServerClientDeregisterEvent * event, void * context) {
    std::cout << "DeregisterEventCallback: event " << event << ", context " << context << std::endl;
    CallbackRecord * record = static_cast<CallbackRecord *>(context);
    record->DeregisterCallbackCounter++;
    record->LastEvent = event;
}

static void ClientUpdateEventCallback(const AwaServerClientUpdateEvent * event, void * context) {
    std::cout << "UpdateEventCallback: event " << event << ", context " << context << std::endl;
    CallbackRecord * record = static_cast<CallbackRecord *>(context);
    record->UpdateCallbackCounter++;
    record->LastEvent = event;
}

} // namespace detail


// ClientRegisterEvent / AwaServerClientRegisterEvent tests:

TEST_F(TestServerEventsWithSession, ClientRegisterEvent_New_and_Free)
{
    ClientRegisterEvent * event = ClientRegisterEvent_New();
    EXPECT_TRUE(NULL != event);
    ClientRegisterEvent_Free(&event);
    EXPECT_EQ(NULL, event);
}

TEST_F(TestServerEventsWithSession, ClientRegisterEvent_Free_handles_null_pointer)
{
    ClientRegisterEvent * event = NULL;
    ClientRegisterEvent_Free(&event);
    EXPECT_EQ(NULL, event);
}

TEST_F(TestServerEventsWithSession, ClientRegisterEvent_Free_handles_null)
{
    ClientRegisterEvent_Free(NULL);
}

TEST_F(TestServerEventsWithSession, ClientRegisterEvent_AddNotification_handles_null)
{
    EXPECT_EQ(-1, ClientRegisterEvent_AddNotification(NULL, NULL, NULL));

    IPCMessage * notification = IPCMessage_New();
    EXPECT_EQ(-1, ClientRegisterEvent_AddNotification(NULL, notification, NULL));

    ClientRegisterEvent * event = ClientRegisterEvent_New();
    EXPECT_EQ(-1, ClientRegisterEvent_AddNotification(event, NULL, NULL));
    EXPECT_EQ(-1, ClientRegisterEvent_AddNotification(event, notification, NULL));

    ClientRegisterEvent_Free(&event);
    IPCMessage_Free(&notification);
}

TEST_F(TestServerEventsWithSession, ClientRegisterEvent_AddNotification_handles_invalid_message)
{
    IPCMessage * notification = IPCMessage_New();
    ClientRegisterEvent * event = ClientRegisterEvent_New();
    EXPECT_EQ(-1, ClientRegisterEvent_AddNotification(event, notification, session_));
    ClientRegisterEvent_Free(&event);
    IPCMessage_Free(&notification);
}

TEST_F(TestServerEventsWithSession, ClientRegisterEvent_AddNotification_handles_non_notification)
{
    IPCMessage * notification = IPCMessage_New();
    IPCMessage_SetType(notification, IPC_MESSAGE_TYPE_RESPONSE, IPC_MESSAGE_SUB_TYPE_DEFINE);
    ClientRegisterEvent * event = ClientRegisterEvent_New();
    EXPECT_EQ(-1, ClientRegisterEvent_AddNotification(event, notification, session_));
    ClientRegisterEvent_Free(&event);
    IPCMessage_Free(&notification);
}

TEST_F(TestServerEventsWithSession, ClientRegisterEvent_AddNotification_handles_wrong_notification)
{
    IPCMessage * notification = IPCMessage_New();
    IPCMessage_SetType(notification, IPC_MESSAGE_TYPE_NOTIFICATION, IPC_MESSAGE_SUB_TYPE_CLIENT_UPDATE);
    ClientRegisterEvent * event = ClientRegisterEvent_New();
    EXPECT_EQ(-1, ClientRegisterEvent_AddNotification(event, notification, session_));
    ClientRegisterEvent_Free(&event);
    IPCMessage_Free(&notification);
}

TEST_F(TestServerEventsWithSession, ClientRegisterEvent_NewClientIterator_with_no_notification)
{
    ClientRegisterEvent * event = ClientRegisterEvent_New();
    EXPECT_EQ(NULL, ClientRegisterEvent_NewClientIterator(event));
    ClientRegisterEvent_Free(&event);
}

TEST_F(TestServerEventsWithSession, ClientRegisterEvent_NewRegisteredEntityIterator_handles_null)
{
    EXPECT_EQ(NULL, ClientRegisterEvent_NewRegisteredEntityIterator(NULL, NULL));

    ClientRegisterEvent * event = ClientRegisterEvent_New();
    EXPECT_EQ(NULL, ClientRegisterEvent_NewRegisteredEntityIterator(event, NULL));
    ClientRegisterEvent_Free(&event);
}

TEST_F(TestServerEventsWithSession, ClientRegisterEvent_handles_single_client_notification)
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

    std::vector<std::string> expectedClientIDs = { "imagination1" };

    TreeNode contentNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != contentNode);
    IPCMessage * notification = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_NOTIFICATION, IPC_MESSAGE_SUB_TYPE_CLIENT_REGISTER, 1234567);
    ASSERT_TRUE(NULL != notification);
    IPCMessage_AddContent(notification, contentNode);

    ClientRegisterEvent * event = ClientRegisterEvent_New();
    ASSERT_TRUE(NULL != event);
    EXPECT_EQ(0, ClientRegisterEvent_AddNotification(event, notification, session_));
    ClientIterator * clientIterator = ClientRegisterEvent_NewClientIterator(event);
    EXPECT_TRUE(NULL != clientIterator);

    std::vector<std::string> actualClientIDs;
    while (ClientIterator_Next(clientIterator))
    {
        const char * clientID = ClientIterator_GetClientID(clientIterator);
        actualClientIDs.push_back(clientID);
    }
    ClientIterator_Free(&clientIterator);

    EXPECT_EQ(expectedClientIDs.size(), actualClientIDs.size());
    if (expectedClientIDs.size() == actualClientIDs.size())
        EXPECT_TRUE(std::is_permutation(expectedClientIDs.begin(), expectedClientIDs.end(), actualClientIDs.begin()));

    ClientRegisterEvent_Free(&event);
    IPCMessage_Free(&notification);
    Tree_Delete(contentNode);
}

TEST_F(TestServerEventsWithSession, ClientRegisterEvent_handles_multiple_client_notification)
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
            "  <Client>"
            "    <ID>TestTwo</ID>"
            "    <Objects>"
            "      <Object>"
            "        <ID>1</ID>"
            "        <ObjectInstance>"
            "          <ID>0</ID>"
            "        </ObjectInstance>"
            "      </Object>"
            "    </Objects>"
            "  </Client>"
            "  <Client>"
            "    <ID>third_client3</ID>"
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

    std::vector<std::string> expectedClientIDs = { "imagination1", "TestTwo", "third_client3" };

    TreeNode contentNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != contentNode);
    IPCMessage * notification = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_NOTIFICATION, IPC_MESSAGE_SUB_TYPE_CLIENT_REGISTER, 1234567);
    ASSERT_TRUE(NULL != notification);
    IPCMessage_AddContent(notification, contentNode);

    ClientRegisterEvent * event = ClientRegisterEvent_New();
    ASSERT_TRUE(NULL != event);
    EXPECT_EQ(0, ClientRegisterEvent_AddNotification(event, notification, session_));
    ClientIterator * clientIterator = ClientRegisterEvent_NewClientIterator(event);
    EXPECT_TRUE(NULL != clientIterator);

    std::vector<std::string> actualClientIDs;
    while (ClientIterator_Next(clientIterator))
    {
        const char * clientID = ClientIterator_GetClientID(clientIterator);
        actualClientIDs.push_back(clientID);
    }
    ClientIterator_Free(&clientIterator);

    EXPECT_EQ(expectedClientIDs.size(), actualClientIDs.size());
    if (expectedClientIDs.size() == actualClientIDs.size())
        EXPECT_TRUE(std::is_permutation(expectedClientIDs.begin(), expectedClientIDs.end(), actualClientIDs.begin()));

    ClientRegisterEvent_Free(&event);
    IPCMessage_Free(&notification);
    Tree_Delete(contentNode);
}

TEST_F(TestServerEventsWithSession, ClientRegisterEvent_handles_registered_entities)
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
            "      <Object>"
            "        <ID>2</ID>"
            "        <ObjectInstance>"
            "          <ID>0</ID>"
            "        </ObjectInstance>"
            "        <ObjectInstance>"
            "          <ID>1</ID>"
            "        </ObjectInstance>"
            "        <ObjectInstance>"
            "          <ID>2</ID>"
            "        </ObjectInstance>"
            "        <ObjectInstance>"
            "          <ID>3</ID>"
            "        </ObjectInstance>"
            "      </Object>"
            "      <Object>"
            "        <ID>3</ID>"
            "        <ObjectInstance>"
            "          <ID>0</ID>"
            "        </ObjectInstance>"
            "      </Object>"
            "      <Object>"
            "        <ID>4</ID>"
            "        <ObjectInstance>"
            "          <ID>0</ID>"
            "        </ObjectInstance>"
            "      </Object>"
            "      <Object>"
            "        <ID>7</ID>"
            "      </Object>"
            "      <Object>"
            "        <ID>5</ID>"
            "        <ObjectInstance>"
            "          <ID>0</ID>"
            "        </ObjectInstance>"
            "      </Object>"
            "      <Object>"
            "        <ID>6</ID>"
            "        <ObjectInstance>"
            "          <ID>0</ID>"
            "        </ObjectInstance>"
            "      </Object>"
            "    </Objects>"
            "  </Client>"
            "</Clients>";


    std::vector<std::string> expectedClientIDs = { "imagination1" };
    std::vector<std::string> expectedPaths = { "/1/0", "/2/0", "/2/1", "/2/2", "/2/3", "/3/0", "/4/0", "/7", "/5/0", "/6/0" };

    TreeNode contentNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != contentNode);
    IPCMessage * notification = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_NOTIFICATION, IPC_MESSAGE_SUB_TYPE_CLIENT_REGISTER, 1234567);
    ASSERT_TRUE(NULL != notification);
    IPCMessage_AddContent(notification, contentNode);

    ClientRegisterEvent * event = ClientRegisterEvent_New();
    ASSERT_TRUE(NULL != event);
    EXPECT_EQ(0, ClientRegisterEvent_AddNotification(event, notification, session_));
    ClientIterator * clientIterator = ClientRegisterEvent_NewClientIterator(event);
    EXPECT_TRUE(NULL != clientIterator);

    std::vector<std::string> actualClientIDs;
    while (ClientIterator_Next(clientIterator))
    {
        const char * clientID = ClientIterator_GetClientID(clientIterator);
        actualClientIDs.push_back(clientID);
    }
    ClientIterator_Free(&clientIterator);

    EXPECT_EQ(expectedClientIDs.size(), actualClientIDs.size());
    if (expectedClientIDs.size() == actualClientIDs.size())
        EXPECT_TRUE(std::is_permutation(expectedClientIDs.begin(), expectedClientIDs.end(), actualClientIDs.begin()));

    // check for registered entities using first client ID
    const char * clientID = actualClientIDs[0].c_str();
    RegisteredEntityIterator * entityIterator = ClientRegisterEvent_NewRegisteredEntityIterator(event, clientID);
    EXPECT_TRUE(NULL != entityIterator);

    std::vector<std::string> actualPaths;
    while (RegisteredEntityIterator_Next(entityIterator))
    {
        const char * path = RegisteredEntityIterator_GetPath(entityIterator);
        actualPaths.push_back(path);
    }

//    std::cout << "Expected:" << std::endl;
//    for (auto i : expectedPaths)
//        std::cout << i << std::endl;
//
//    std::cout << "Actual:" << std::endl;
//    for (auto i : actualPaths)
//        std::cout << i << std::endl;

    EXPECT_EQ(expectedPaths.size(), actualPaths.size());
    if (expectedPaths.size() == actualPaths.size())
        EXPECT_TRUE(std::is_permutation(expectedPaths.begin(), expectedPaths.end(), actualPaths.begin()));

    RegisteredEntityIterator_Free(&entityIterator);
    ClientRegisterEvent_Free(&event);
    IPCMessage_Free(&notification);
    Tree_Delete(contentNode);
}

TEST_F(TestServerEventsWithSession, ClientRegisterEvent_NewRegisteredEntityIterator_without_ClientIterator)
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
            "      <Object>"
            "        <ID>2</ID>"
            "        <ObjectInstance>"
            "          <ID>0</ID>"
            "        </ObjectInstance>"
            "        <ObjectInstance>"
            "          <ID>1</ID>"
            "        </ObjectInstance>"
            "        <ObjectInstance>"
            "          <ID>2</ID>"
            "        </ObjectInstance>"
            "        <ObjectInstance>"
            "          <ID>3</ID>"
            "        </ObjectInstance>"
            "      </Object>"
            "      <Object>"
            "        <ID>7</ID>"
            "      </Object>"
            "    </Objects>"
            "  </Client>"
            "</Clients>";

    std::vector<std::string> expectedPaths = { "/1/0", "/2/0", "/2/1", "/2/2", "/2/3", "/7" };

    TreeNode contentNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != contentNode);
    IPCMessage * notification = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_NOTIFICATION, IPC_MESSAGE_SUB_TYPE_CLIENT_REGISTER, 1234567);
    ASSERT_TRUE(NULL != notification);
    IPCMessage_AddContent(notification, contentNode);

    ClientRegisterEvent * event = ClientRegisterEvent_New();
    ASSERT_TRUE(NULL != event);
    EXPECT_EQ(0, ClientRegisterEvent_AddNotification(event, notification, session_));

    // check for registered entities using pre-known client ID
    const char * clientID = "imagination1";
    RegisteredEntityIterator * entityIterator = ClientRegisterEvent_NewRegisteredEntityIterator(event, clientID);
    EXPECT_TRUE(NULL != entityIterator);

    std::vector<std::string> actualPaths;
    while (RegisteredEntityIterator_Next(entityIterator))
    {
        const char * path = RegisteredEntityIterator_GetPath(entityIterator);
        actualPaths.push_back(path);
    }

    EXPECT_EQ(expectedPaths.size(), actualPaths.size());
    if (expectedPaths.size() == actualPaths.size())
        EXPECT_TRUE(std::is_permutation(expectedPaths.begin(), expectedPaths.end(), actualPaths.begin()));

    RegisteredEntityIterator_Free(&entityIterator);
    ClientRegisterEvent_Free(&event);
    IPCMessage_Free(&notification);
    Tree_Delete(contentNode);
}

TEST_F(TestServerEventsWithSession, ClientRegisterEvent_NewRegisteredEntityIterator_with_invalid_clientID)
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

    ClientRegisterEvent * event = ClientRegisterEvent_New();
    ASSERT_TRUE(NULL != event);
    EXPECT_EQ(0, ClientRegisterEvent_AddNotification(event, notification, session_));

    // check for registered entities using invalid client ID
    const char * clientID = "imaginationBAD";
    RegisteredEntityIterator * entityIterator = ClientRegisterEvent_NewRegisteredEntityIterator(event, clientID);
    EXPECT_EQ(NULL, entityIterator);

    ClientRegisterEvent_Free(&event);
    IPCMessage_Free(&notification);
    Tree_Delete(contentNode);
}



// ClientDeregisterEvent / AwaServerClientDeregisterEvent tests:

TEST_F(TestServerEventsWithSession, ClientDeregisterEvent_New_and_Free)
{
    ClientDeregisterEvent * event = ClientDeregisterEvent_New();
    EXPECT_TRUE(NULL != event);
    ClientDeregisterEvent_Free(&event);
    EXPECT_EQ(NULL, event);
}

TEST_F(TestServerEventsWithSession, ClientDeregisterEvent_Free_handles_null_pointer)
{
    ClientDeregisterEvent * event = NULL;
    ClientDeregisterEvent_Free(&event);
    EXPECT_EQ(NULL, event);
}

TEST_F(TestServerEventsWithSession, ClientDeregisterEvent_Free_handles_null)
{
    ClientDeregisterEvent_Free(NULL);
}

TEST_F(TestServerEventsWithSession, ClientDeregisterEvent_AddNotification_handles_null)
{
    EXPECT_EQ(-1, ClientDeregisterEvent_AddNotification(NULL, NULL, NULL));

    IPCMessage * notification = IPCMessage_New();
    EXPECT_EQ(-1, ClientDeregisterEvent_AddNotification(NULL, notification, NULL));

    ClientDeregisterEvent * event = ClientDeregisterEvent_New();
    EXPECT_EQ(-1, ClientDeregisterEvent_AddNotification(event, NULL, NULL));
    EXPECT_EQ(-1, ClientDeregisterEvent_AddNotification(event, notification, NULL));

    ClientDeregisterEvent_Free(&event);
    IPCMessage_Free(&notification);
}

TEST_F(TestServerEventsWithSession, ClientDeregisterEvent_AddNotification_handles_invalid_message)
{
    IPCMessage * notification = IPCMessage_New();
    ClientDeregisterEvent * event = ClientDeregisterEvent_New();
    EXPECT_EQ(-1, ClientDeregisterEvent_AddNotification(event, notification, session_));
    ClientDeregisterEvent_Free(&event);
    IPCMessage_Free(&notification);
}

TEST_F(TestServerEventsWithSession, ClientDeregisterEvent_AddNotification_handles_non_notification)
{
    IPCMessage * notification = IPCMessage_New();
    IPCMessage_SetType(notification, IPC_MESSAGE_TYPE_RESPONSE, IPC_MESSAGE_SUB_TYPE_DEFINE);
    ClientDeregisterEvent * event = ClientDeregisterEvent_New();
    EXPECT_EQ(-1, ClientDeregisterEvent_AddNotification(event, notification, session_));
    ClientDeregisterEvent_Free(&event);
    IPCMessage_Free(&notification);
}

TEST_F(TestServerEventsWithSession, ClientDeregisterEvent_AddNotification_handles_wrong_notification)
{
    IPCMessage * notification = IPCMessage_New();
    IPCMessage_SetType(notification, IPC_MESSAGE_TYPE_NOTIFICATION, IPC_MESSAGE_SUB_TYPE_CLIENT_UPDATE);
    ClientDeregisterEvent * event = ClientDeregisterEvent_New();
    EXPECT_EQ(-1, ClientDeregisterEvent_AddNotification(event, notification, session_));
    ClientDeregisterEvent_Free(&event);
    IPCMessage_Free(&notification);
}

TEST_F(TestServerEventsWithSession, ClientDeregisterEvent_NewClientIterator_with_no_notification)
{
    ClientDeregisterEvent * event = ClientDeregisterEvent_New();
    EXPECT_EQ(NULL, ClientDeregisterEvent_NewClientIterator(event));
    ClientDeregisterEvent_Free(&event);
}

TEST_F(TestServerEventsWithSession, ClientDeregisterEvent_handles_single_client_notification)
{
    const char * xml =
            "<Clients>"
            "  <Client>"
            "    <ID>imagination1</ID>"
            "  </Client>"
            "</Clients>";

    std::vector<std::string> expectedClientIDs = { "imagination1" };

    TreeNode contentNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != contentNode);
    IPCMessage * notification = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_NOTIFICATION, IPC_MESSAGE_SUB_TYPE_CLIENT_DEREGISTER, 1234567);
    ASSERT_TRUE(NULL != notification);
    IPCMessage_AddContent(notification, contentNode);

    ClientDeregisterEvent * event = ClientDeregisterEvent_New();
    ASSERT_TRUE(NULL != event);
    EXPECT_EQ(0, ClientDeregisterEvent_AddNotification(event, notification, session_));
    ClientIterator * clientIterator = ClientDeregisterEvent_NewClientIterator(event);
    EXPECT_TRUE(NULL != clientIterator);

    std::vector<std::string> actualClientIDs;
    while (ClientIterator_Next(clientIterator))
    {
        const char * clientID = ClientIterator_GetClientID(clientIterator);
        actualClientIDs.push_back(clientID);
    }
    ClientIterator_Free(&clientIterator);

    EXPECT_EQ(expectedClientIDs.size(), actualClientIDs.size());
    if (expectedClientIDs.size() == actualClientIDs.size())
        EXPECT_TRUE(std::is_permutation(expectedClientIDs.begin(), expectedClientIDs.end(), actualClientIDs.begin()));

    ClientDeregisterEvent_Free(&event);
    IPCMessage_Free(&notification);
    Tree_Delete(contentNode);
}

TEST_F(TestServerEventsWithSession, ClientDeregisterEvent_handles_multiple_client_notification)
{
    const char * xml =
            "<Clients>"
            "  <Client>"
            "    <ID>imagination1</ID>"
            "  </Client>"
            "  <Client>"
            "    <ID>TestTwo</ID>"
            "  </Client>"
            "  <Client>"
            "    <ID>third_client3</ID>"
            "  </Client>"
            "</Clients>";

    std::vector<std::string> expectedClientIDs = { "imagination1", "TestTwo", "third_client3" };

    TreeNode contentNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != contentNode);
    IPCMessage * notification = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_NOTIFICATION, IPC_MESSAGE_SUB_TYPE_CLIENT_DEREGISTER, 1234567);
    ASSERT_TRUE(NULL != notification);
    IPCMessage_AddContent(notification, contentNode);

    ClientDeregisterEvent * event = ClientDeregisterEvent_New();
    ASSERT_TRUE(NULL != event);
    EXPECT_EQ(0, ClientDeregisterEvent_AddNotification(event, notification, session_));
    ClientIterator * clientIterator = ClientDeregisterEvent_NewClientIterator(event);
    EXPECT_TRUE(NULL != clientIterator);

    std::vector<std::string> actualClientIDs;
    while (ClientIterator_Next(clientIterator))
    {
        const char * clientID = ClientIterator_GetClientID(clientIterator);
        actualClientIDs.push_back(clientID);
    }
    ClientIterator_Free(&clientIterator);

    EXPECT_EQ(expectedClientIDs.size(), actualClientIDs.size());
    if (expectedClientIDs.size() == actualClientIDs.size())
        EXPECT_TRUE(std::is_permutation(expectedClientIDs.begin(), expectedClientIDs.end(), actualClientIDs.begin()));

    ClientDeregisterEvent_Free(&event);
    IPCMessage_Free(&notification);
    Tree_Delete(contentNode);
}


// ClientUpdateEvent / AwaServerClientUpdateEvent tests:

TEST_F(TestServerEventsWithSession, ClientUpdateEvent_New_and_Free)
{
    ClientUpdateEvent * event = ClientUpdateEvent_New();
    EXPECT_TRUE(NULL != event);
    ClientUpdateEvent_Free(&event);
    EXPECT_EQ(NULL, event);
}

TEST_F(TestServerEventsWithSession, ClientUpdateEvent_Free_handles_null_pointer)
{
    ClientUpdateEvent * event = NULL;
    ClientUpdateEvent_Free(&event);
    EXPECT_EQ(NULL, event);
}

TEST_F(TestServerEventsWithSession, ClientUpdateEvent_Free_handles_null)
{
    ClientUpdateEvent_Free(NULL);
}

TEST_F(TestServerEventsWithSession, ClientUpdateEvent_AddNotification_handles_null)
{
    EXPECT_EQ(-1, ClientUpdateEvent_AddNotification(NULL, NULL, NULL));

    IPCMessage * notification = IPCMessage_New();
    EXPECT_EQ(-1, ClientUpdateEvent_AddNotification(NULL, notification, NULL));

    ClientUpdateEvent * event = ClientUpdateEvent_New();
    EXPECT_EQ(-1, ClientUpdateEvent_AddNotification(event, NULL, NULL));
    EXPECT_EQ(-1, ClientUpdateEvent_AddNotification(event, notification, NULL));

    ClientUpdateEvent_Free(&event);
    IPCMessage_Free(&notification);
}

TEST_F(TestServerEventsWithSession, ClientUpdateEvent_AddNotification_handles_invalid_message)
{
    IPCMessage * notification = IPCMessage_New();
    ClientUpdateEvent * event = ClientUpdateEvent_New();
    EXPECT_EQ(-1, ClientUpdateEvent_AddNotification(event, notification, session_));
    ClientUpdateEvent_Free(&event);
    IPCMessage_Free(&notification);
}

TEST_F(TestServerEventsWithSession, ClientUpdateEvent_AddNotification_handles_non_notification)
{
    IPCMessage * notification = IPCMessage_New();
    IPCMessage_SetType(notification, IPC_MESSAGE_TYPE_RESPONSE, IPC_MESSAGE_SUB_TYPE_DEFINE);
    ClientUpdateEvent * event = ClientUpdateEvent_New();
    EXPECT_EQ(-1, ClientUpdateEvent_AddNotification(event, notification, session_));
    ClientUpdateEvent_Free(&event);
    IPCMessage_Free(&notification);
}

TEST_F(TestServerEventsWithSession, ClientUpdateEvent_AddNotification_handles_wrong_notification)
{
    IPCMessage * notification = IPCMessage_New();
    IPCMessage_SetType(notification, IPC_MESSAGE_TYPE_NOTIFICATION, IPC_MESSAGE_SUB_TYPE_CLIENT_REGISTER);
    ClientUpdateEvent * event = ClientUpdateEvent_New();
    EXPECT_EQ(-1, ClientUpdateEvent_AddNotification(event, notification, session_));
    ClientUpdateEvent_Free(&event);
    IPCMessage_Free(&notification);
}

TEST_F(TestServerEventsWithSession, ClientUpdateEvent_NewClientIterator_with_no_notification)
{
    ClientUpdateEvent * event = ClientUpdateEvent_New();
    EXPECT_EQ(NULL, ClientUpdateEvent_NewClientIterator(event));
    ClientUpdateEvent_Free(&event);
}

TEST_F(TestServerEventsWithSession, ClientUpdateEvent_NewRegisteredEntityIterator_handles_null)
{
    EXPECT_EQ(NULL, ClientUpdateEvent_NewRegisteredEntityIterator(NULL, NULL));

    ClientUpdateEvent * event = ClientUpdateEvent_New();
    EXPECT_EQ(NULL, ClientUpdateEvent_NewRegisteredEntityIterator(event, NULL));
    ClientUpdateEvent_Free(&event);
}

TEST_F(TestServerEventsWithSession, ClientUpdateEvent_handles_single_client_notification)
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

    std::vector<std::string> expectedClientIDs = { "imagination1" };

    TreeNode contentNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != contentNode);
    IPCMessage * notification = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_NOTIFICATION, IPC_MESSAGE_SUB_TYPE_CLIENT_UPDATE, 1234567);
    ASSERT_TRUE(NULL != notification);
    IPCMessage_AddContent(notification, contentNode);

    ClientUpdateEvent * event = ClientUpdateEvent_New();
    ASSERT_TRUE(NULL != event);
    EXPECT_EQ(0, ClientUpdateEvent_AddNotification(event, notification, session_));
    ClientIterator * clientIterator = ClientUpdateEvent_NewClientIterator(event);
    EXPECT_TRUE(NULL != clientIterator);

    std::vector<std::string> actualClientIDs;
    while (ClientIterator_Next(clientIterator))
    {
        const char * clientID = ClientIterator_GetClientID(clientIterator);
        actualClientIDs.push_back(clientID);
    }
    ClientIterator_Free(&clientIterator);

    EXPECT_EQ(expectedClientIDs.size(), actualClientIDs.size());
    if (expectedClientIDs.size() == actualClientIDs.size())
        EXPECT_TRUE(std::is_permutation(expectedClientIDs.begin(), expectedClientIDs.end(), actualClientIDs.begin()));

    ClientUpdateEvent_Free(&event);
    IPCMessage_Free(&notification);
    Tree_Delete(contentNode);
}

TEST_F(TestServerEventsWithSession, ClientUpdateEvent_handles_multiple_client_notification)
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
            "  <Client>"
            "    <ID>TestTwo</ID>"
            "    <Objects>"
            "      <Object>"
            "        <ID>1</ID>"
            "        <ObjectInstance>"
            "          <ID>0</ID>"
            "        </ObjectInstance>"
            "      </Object>"
            "    </Objects>"
            "  </Client>"
            "  <Client>"
            "    <ID>third_client3</ID>"
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

    std::vector<std::string> expectedClientIDs = { "imagination1", "TestTwo", "third_client3" };

    TreeNode contentNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != contentNode);
    IPCMessage * notification = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_NOTIFICATION, IPC_MESSAGE_SUB_TYPE_CLIENT_UPDATE, 1234567);
    ASSERT_TRUE(NULL != notification);
    IPCMessage_AddContent(notification, contentNode);

    ClientUpdateEvent * event = ClientUpdateEvent_New();
    ASSERT_TRUE(NULL != event);
    EXPECT_EQ(0, ClientUpdateEvent_AddNotification(event, notification, session_));
    ClientIterator * clientIterator = ClientUpdateEvent_NewClientIterator(event);
    EXPECT_TRUE(NULL != clientIterator);

    std::vector<std::string> actualClientIDs;
    while (ClientIterator_Next(clientIterator))
    {
        const char * clientID = ClientIterator_GetClientID(clientIterator);
        actualClientIDs.push_back(clientID);
    }
    ClientIterator_Free(&clientIterator);

    EXPECT_EQ(expectedClientIDs.size(), actualClientIDs.size());
    if (expectedClientIDs.size() == actualClientIDs.size())
        EXPECT_TRUE(std::is_permutation(expectedClientIDs.begin(), expectedClientIDs.end(), actualClientIDs.begin()));

    ClientUpdateEvent_Free(&event);
    IPCMessage_Free(&notification);
    Tree_Delete(contentNode);
}

TEST_F(TestServerEventsWithSession, ClientUpdateEvent_handles_registered_entities)
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
            "      <Object>"
            "        <ID>2</ID>"
            "        <ObjectInstance>"
            "          <ID>0</ID>"
            "        </ObjectInstance>"
            "        <ObjectInstance>"
            "          <ID>1</ID>"
            "        </ObjectInstance>"
            "        <ObjectInstance>"
            "          <ID>2</ID>"
            "        </ObjectInstance>"
            "        <ObjectInstance>"
            "          <ID>3</ID>"
            "        </ObjectInstance>"
            "      </Object>"
            "      <Object>"
            "        <ID>3</ID>"
            "        <ObjectInstance>"
            "          <ID>0</ID>"
            "        </ObjectInstance>"
            "      </Object>"
            "      <Object>"
            "        <ID>4</ID>"
            "        <ObjectInstance>"
            "          <ID>0</ID>"
            "        </ObjectInstance>"
            "      </Object>"
            "      <Object>"
            "        <ID>7</ID>"
            "      </Object>"
            "      <Object>"
            "        <ID>5</ID>"
            "        <ObjectInstance>"
            "          <ID>0</ID>"
            "        </ObjectInstance>"
            "      </Object>"
            "      <Object>"
            "        <ID>6</ID>"
            "        <ObjectInstance>"
            "          <ID>0</ID>"
            "        </ObjectInstance>"
            "      </Object>"
            "    </Objects>"
            "  </Client>"
            "</Clients>";


    std::vector<std::string> expectedClientIDs = { "imagination1" };
    std::vector<std::string> expectedPaths = { "/1/0", "/2/0", "/2/1", "/2/2", "/2/3", "/3/0", "/4/0", "/7", "/5/0", "/6/0" };

    TreeNode contentNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != contentNode);
    IPCMessage * notification = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_NOTIFICATION, IPC_MESSAGE_SUB_TYPE_CLIENT_UPDATE, 1234567);
    ASSERT_TRUE(NULL != notification);
    IPCMessage_AddContent(notification, contentNode);

    ClientUpdateEvent * event = ClientUpdateEvent_New();
    ASSERT_TRUE(NULL != event);
    EXPECT_EQ(0, ClientUpdateEvent_AddNotification(event, notification, session_));
    ClientIterator * clientIterator = ClientUpdateEvent_NewClientIterator(event);
    EXPECT_TRUE(NULL != clientIterator);

    std::vector<std::string> actualClientIDs;
    while (ClientIterator_Next(clientIterator))
    {
        const char * clientID = ClientIterator_GetClientID(clientIterator);
        actualClientIDs.push_back(clientID);
    }
    ClientIterator_Free(&clientIterator);

    EXPECT_EQ(expectedClientIDs.size(), actualClientIDs.size());
    if (expectedClientIDs.size() == actualClientIDs.size())
        EXPECT_TRUE(std::is_permutation(expectedClientIDs.begin(), expectedClientIDs.end(), actualClientIDs.begin()));

    // check for registered entities using first client ID
    const char * clientID = actualClientIDs[0].c_str();
    RegisteredEntityIterator * entityIterator = ClientUpdateEvent_NewRegisteredEntityIterator(event, clientID);
    EXPECT_TRUE(NULL != entityIterator);

    std::vector<std::string> actualPaths;
    while (RegisteredEntityIterator_Next(entityIterator))
    {
        const char * path = RegisteredEntityIterator_GetPath(entityIterator);
        actualPaths.push_back(path);
    }

    EXPECT_EQ(expectedPaths.size(), actualPaths.size());
    if (expectedPaths.size() == actualPaths.size())
        EXPECT_TRUE(std::is_permutation(expectedPaths.begin(), expectedPaths.end(), actualPaths.begin()));

    RegisteredEntityIterator_Free(&entityIterator);
    ClientUpdateEvent_Free(&event);
    IPCMessage_Free(&notification);
    Tree_Delete(contentNode);
}

TEST_F(TestServerEventsWithSession, ClientUpdateEvent_NewRegisteredEntityIterator_without_ClientIterator)
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
            "      <Object>"
            "        <ID>2</ID>"
            "        <ObjectInstance>"
            "          <ID>0</ID>"
            "        </ObjectInstance>"
            "        <ObjectInstance>"
            "          <ID>1</ID>"
            "        </ObjectInstance>"
            "        <ObjectInstance>"
            "          <ID>2</ID>"
            "        </ObjectInstance>"
            "        <ObjectInstance>"
            "          <ID>3</ID>"
            "        </ObjectInstance>"
            "      </Object>"
            "      <Object>"
            "        <ID>7</ID>"
            "      </Object>"
            "    </Objects>"
            "  </Client>"
            "</Clients>";

    std::vector<std::string> expectedPaths = { "/1/0", "/2/0", "/2/1", "/2/2", "/2/3", "/7" };

    TreeNode contentNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    ASSERT_TRUE(NULL != contentNode);
    IPCMessage * notification = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_NOTIFICATION, IPC_MESSAGE_SUB_TYPE_CLIENT_UPDATE, 1234567);
    ASSERT_TRUE(NULL != notification);
    IPCMessage_AddContent(notification, contentNode);

    ClientUpdateEvent * event = ClientUpdateEvent_New();
    ASSERT_TRUE(NULL != event);
    EXPECT_EQ(0, ClientUpdateEvent_AddNotification(event, notification, session_));

    // check for registered entities using pre-known client ID
    const char * clientID = "imagination1";
    RegisteredEntityIterator * entityIterator = ClientUpdateEvent_NewRegisteredEntityIterator(event, clientID);
    EXPECT_TRUE(NULL != entityIterator);

    std::vector<std::string> actualPaths;
    while (RegisteredEntityIterator_Next(entityIterator))
    {
        const char * path = RegisteredEntityIterator_GetPath(entityIterator);
        actualPaths.push_back(path);
    }

    EXPECT_EQ(expectedPaths.size(), actualPaths.size());
    if (expectedPaths.size() == actualPaths.size())
        EXPECT_TRUE(std::is_permutation(expectedPaths.begin(), expectedPaths.end(), actualPaths.begin()));

    RegisteredEntityIterator_Free(&entityIterator);
    ClientUpdateEvent_Free(&event);
    IPCMessage_Free(&notification);
    Tree_Delete(contentNode);
}

TEST_F(TestServerEventsWithSession, ClientUpdateEvent_NewRegisteredEntityIterator_with_invalid_clientID)
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
    IPCMessage * notification = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_NOTIFICATION, IPC_MESSAGE_SUB_TYPE_CLIENT_UPDATE, 1234567);
    ASSERT_TRUE(NULL != notification);
    IPCMessage_AddContent(notification, contentNode);

    ClientUpdateEvent * event = ClientUpdateEvent_New();
    ASSERT_TRUE(NULL != event);
    EXPECT_EQ(0, ClientUpdateEvent_AddNotification(event, notification, session_));

    // check for registered entities using invalid client ID
    const char * clientID = "imaginationBAD";
    RegisteredEntityIterator * entityIterator = ClientUpdateEvent_NewRegisteredEntityIterator(event, clientID);
    EXPECT_EQ(NULL, entityIterator);

    ClientUpdateEvent_Free(&event);
    IPCMessage_Free(&notification);
    Tree_Delete(contentNode);
}





// ServerEventsCallbackInfo tests:

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

TEST_F(TestServerEvents, ServerEventsCallbackInfo_SetClientRegisterCallback_handles_null)
{
    EXPECT_EQ(-1, ServerEventsCallbackInfo_SetClientRegisterCallback(NULL, NULL, NULL));
    EXPECT_EQ(-1, ServerEventsCallbackInfo_SetClientRegisterCallback(NULL, detail::ClientRegisterEventCallback, NULL));
}

TEST_F(TestServerEvents, ServerEventsCallbackInfo_SetClientRegisterCallback_handles_valid)
{
    ServerEventsCallbackInfo * info = ServerEventsCallbackInfo_New();
    ASSERT_TRUE(NULL != info);
    EXPECT_EQ(0, ServerEventsCallbackInfo_SetClientRegisterCallback(info, NULL, NULL));  // disable a callback
    EXPECT_EQ(0, ServerEventsCallbackInfo_SetClientRegisterCallback(info, detail::ClientRegisterEventCallback, NULL));
    int data = 0;
    EXPECT_EQ(0, ServerEventsCallbackInfo_SetClientRegisterCallback(info, detail::ClientRegisterEventCallback, &data));
    ServerEventsCallbackInfo_Free(&info);
    EXPECT_EQ(NULL, info);
}

TEST_F(TestServerEvents, ServerEventsCallbackInfo_InvokeClientUpdateCallback_handles_null)
{
    EXPECT_EQ(-1, ServerEventsCallbackInfo_InvokeClientRegisterCallback(NULL, NULL));

    ServerEventsCallbackInfo * info = ServerEventsCallbackInfo_New();
    EXPECT_EQ(-1, ServerEventsCallbackInfo_InvokeClientRegisterCallback(info, NULL));
    ServerEventsCallbackInfo_Free(&info);
}

TEST_F(TestServerEvents, ServerEventsCallbackInfo_InvokeClientRegisterCallback)
{
    ServerEventsCallbackInfo * info = ServerEventsCallbackInfo_New();
    ASSERT_TRUE(NULL != info);
    detail::CallbackRecord record;
    EXPECT_EQ(0, ServerEventsCallbackInfo_SetClientRegisterCallback(info, detail::ClientRegisterEventCallback, &record));
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


TEST_F(TestServerEvents, ServerEventsCallbackInfo_SetClientDeregisterCallback_handles_null)
{
    EXPECT_EQ(-1, ServerEventsCallbackInfo_SetClientDeregisterCallback(NULL, NULL, NULL));
    EXPECT_EQ(-1, ServerEventsCallbackInfo_SetClientDeregisterCallback(NULL, detail::ClientDeregisterEventCallback, NULL));
}

TEST_F(TestServerEvents, ServerEventsCallbackInfo_SetClientDeregisterCallback_handles_valid)
{
    ServerEventsCallbackInfo * info = ServerEventsCallbackInfo_New();
    ASSERT_TRUE(NULL != info);
    EXPECT_EQ(0, ServerEventsCallbackInfo_SetClientDeregisterCallback(info, NULL, NULL));  // disable a callback
    EXPECT_EQ(0, ServerEventsCallbackInfo_SetClientDeregisterCallback(info, detail::ClientDeregisterEventCallback, NULL));
    int data = 0;
    EXPECT_EQ(0, ServerEventsCallbackInfo_SetClientDeregisterCallback(info, detail::ClientDeregisterEventCallback, &data));
    ServerEventsCallbackInfo_Free(&info);
    EXPECT_EQ(NULL, info);
}

TEST_F(TestServerEvents, ServerEventsCallbackInfo_InvokeClientDeregisterCallback)
{
    ServerEventsCallbackInfo * info = ServerEventsCallbackInfo_New();
    ASSERT_TRUE(NULL != info);
    detail::CallbackRecord record;
    EXPECT_EQ(0, ServerEventsCallbackInfo_SetClientDeregisterCallback(info, detail::ClientDeregisterEventCallback, &record));
    ASSERT_EQ(0, record.DeregisterCallbackCounter);

    ClientDeregisterEvent * event = ClientDeregisterEvent_New();
    EXPECT_EQ(0, ServerEventsCallbackInfo_InvokeClientDeregisterCallback(info, event));
    EXPECT_EQ(0, record.RegisterCallbackCounter);
    EXPECT_EQ(1, record.DeregisterCallbackCounter);
    EXPECT_EQ(0, record.UpdateCallbackCounter);
    EXPECT_EQ(event, record.LastEvent);
    ClientDeregisterEvent_Free(&event);

    ServerEventsCallbackInfo_Free(&info);
}


TEST_F(TestServerEvents, ServerEventsCallbackInfo_SetClientUpdateCallback_handles_null)
{
    EXPECT_EQ(-1, ServerEventsCallbackInfo_SetClientUpdateCallback(NULL, NULL, NULL));
    EXPECT_EQ(-1, ServerEventsCallbackInfo_SetClientUpdateCallback(NULL, detail::ClientUpdateEventCallback, NULL));
}

TEST_F(TestServerEvents, ServerEventsCallbackInfo_SetClientUpdateCallback_handles_valid)
{
    ServerEventsCallbackInfo * info = ServerEventsCallbackInfo_New();
    ASSERT_TRUE(NULL != info);
    EXPECT_EQ(0, ServerEventsCallbackInfo_SetClientUpdateCallback(info, NULL, NULL));  // disable a callback
    EXPECT_EQ(0, ServerEventsCallbackInfo_SetClientUpdateCallback(info, detail::ClientUpdateEventCallback, NULL));
    int data = 0;
    EXPECT_EQ(0, ServerEventsCallbackInfo_SetClientUpdateCallback(info, detail::ClientUpdateEventCallback, &data));
    ServerEventsCallbackInfo_Free(&info);
    EXPECT_EQ(NULL, info);
}

TEST_F(TestServerEvents, ServerEventsCallbackInfo_InvokeClientUpdateCallback)
{
    ServerEventsCallbackInfo * info = ServerEventsCallbackInfo_New();
    ASSERT_TRUE(NULL != info);
    detail::CallbackRecord record;
    EXPECT_EQ(0, ServerEventsCallbackInfo_SetClientUpdateCallback(info, detail::ClientUpdateEventCallback, &record));
    ASSERT_EQ(0, record.UpdateCallbackCounter);

    ClientUpdateEvent * event = ClientUpdateEvent_New();
    EXPECT_EQ(0, ServerEventsCallbackInfo_InvokeClientUpdateCallback(info, event));
    EXPECT_EQ(0, record.RegisterCallbackCounter);
    EXPECT_EQ(0, record.DeregisterCallbackCounter);
    EXPECT_EQ(1, record.UpdateCallbackCounter);
    EXPECT_EQ(event, record.LastEvent);
    ClientUpdateEvent_Free(&event);

    ServerEventsCallbackInfo_Free(&info);
}



// API tests

class TestServerEventsWithConnectedSession : public TestStaticClientWithServer {};

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
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetClientRegisterEventCallback(session_, detail::ClientRegisterEventCallback, &record));
    EXPECT_EQ(0, record.RegisterCallbackCounter);
    EXPECT_EQ(0, record.DeregisterCallbackCounter);
    EXPECT_EQ(0, record.UpdateCallbackCounter);
    EXPECT_EQ(nullptr, record.LastEvent);
}

// test events

struct EventWaitCondition : public WaitCondition
{
    AwaServerSession * ServerSession;
    AwaStaticClient * StaticClient;
    std::string ClientEndpointName;
    int CallbackCountMax;
    int CallbackCount;

    EventWaitCondition(AwaServerSession * ServerSession, AwaStaticClient * StaticClient, std::string ClientEndpointName, int callbackCountMax) :
        WaitCondition(50e4),
        ServerSession(ServerSession),
        StaticClient(StaticClient),
        ClientEndpointName(ClientEndpointName),
        CallbackCountMax(callbackCountMax),
        CallbackCount(0) {}
    virtual ~EventWaitCondition() {}

    virtual bool Check()
    {
        std::cout << "Check" << std::endl;
        EXPECT_EQ(AwaError_Success, AwaServerSession_Process(ServerSession, defaults::timeout));
        EXPECT_EQ(AwaError_Success, AwaServerSession_DispatchCallbacks(ServerSession));
        if (StaticClient != NULL)
        {
            std::cout << "Check:StaticClient Process" << std::endl;
            AwaStaticClient_Process(StaticClient);
        }
        return CallbackCount >= CallbackCountMax;
    }

    virtual void internalCallbackHandler(const void * event)
    {
        this->CallbackCount++;
        this->callbackHandler(event);
    };

    virtual void callbackHandler(const void * event) = 0;

    virtual void Deregister()
    {
        std::cout << "Deregister" << std::endl;
        AwaStaticClient_Free(&StaticClient);
    }
};


void ClientEventCallback(const void * event, void * context)
{
    printf("internal callback handler %p %p\n", event, context);
    if (context)
    {
        auto * that = static_cast<EventWaitCondition*>(context);
        that->internalCallbackHandler(event);
    }
}

TEST_F(TestServerEventsWithConnectedSession, ClientRegisterEvent)
{

    struct CallbackHandler1 : public EventWaitCondition
    {
        int count;

        CallbackHandler1(AwaServerSession * ServerSession, AwaStaticClient * StaticClient, std::string ClientEndpointName, int callbackCountMax = 1) :
            EventWaitCondition(ServerSession, StaticClient, ClientEndpointName, callbackCountMax), count(0) {}

        void callbackHandler(const void * event)
        {
            count ++;
            AwaServerClientRegisterEvent * registerEvent = (AwaServerClientRegisterEvent *)event;
            EXPECT_TRUE(registerEvent != NULL);

            AwaClientIterator * clientIterator = AwaServerClientRegisterEvent_NewClientIterator(registerEvent);
            EXPECT_TRUE(clientIterator != NULL);

            EXPECT_TRUE(AwaClientIterator_Next(clientIterator));

            EXPECT_STREQ(global::clientEndpointName, AwaClientIterator_GetClientID(clientIterator));

            AwaRegisteredEntityIterator * entityIterator = AwaServerClientRegisterEvent_NewRegisteredEntityIterator(registerEvent, global::clientEndpointName);
            EXPECT_TRUE(entityIterator != NULL);

            // should be at least one registered entity
            EXPECT_TRUE(AwaRegisteredEntityIterator_Next(entityIterator));

            AwaRegisteredEntityIterator_Free(&entityIterator);

            AwaClientIterator_Free(&clientIterator);
        }
    };
    CallbackHandler1 cbHandler(session_, client_, global::clientEndpointName);

    EXPECT_EQ(AwaError_Success, AwaServerSession_SetClientRegisterEventCallback(session_, (void (*)(const _AwaServerClientRegisterEvent*, void*))ClientEventCallback, &cbHandler));

    EXPECT_TRUE(cbHandler.Wait());
    EXPECT_EQ(1, cbHandler.count);
}

TEST_F(TestServerEventsWithConnectedSession, ClientUpdateEvent)
{

    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_TRUE(NULL != operation);

    SingleStaticClientPollCondition condition(client_, operation, global::clientEndpointName, 20);
    ASSERT_TRUE(condition.Wait());

    AwaServerListClientsOperation_Free(&operation);

    struct CallbackHandler1 : public EventWaitCondition
    {
        int count;

        CallbackHandler1(AwaServerSession * ServerSession, AwaStaticClient * StaticClient, std::string ClientEndpointName, int callbackCountMax = 2) :
            EventWaitCondition(ServerSession, StaticClient, ClientEndpointName, callbackCountMax), count(0) {}

        void callbackHandler(const void * event)
        {
            count ++;
            AwaServerClientDeregisterEvent * deregisterEvent = (AwaServerClientDeregisterEvent *)event;
            EXPECT_TRUE(deregisterEvent != NULL);

            AwaClientIterator * clientIterator = AwaServerClientDeregisterEvent_NewClientIterator(deregisterEvent);
            EXPECT_TRUE(clientIterator != NULL);

            EXPECT_TRUE(AwaClientIterator_Next(clientIterator));

            EXPECT_STREQ(global::clientEndpointName, AwaClientIterator_GetClientID(clientIterator));

            AwaClientIterator_Free(&clientIterator);
        }
    };
    CallbackHandler1 cbHandler(session_, client_, global::clientEndpointName);

    EXPECT_EQ(AwaError_Success, AwaServerSession_SetClientUpdateEventCallback(session_, (void (*)(const _AwaServerClientUpdateEvent*, void*))ClientEventCallback, &cbHandler));

    AwaServerExecuteOperation * updateExecute = AwaServerExecuteOperation_New(session_);
    EXPECT_TRUE(updateExecute != NULL);
    EXPECT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(updateExecute, global::clientEndpointName, "/1/0/8", NULL));

    pthread_t executeThread;
    pthread_create(&executeThread, NULL, do_execute_operation, (void *)updateExecute);
    EXPECT_TRUE(cbHandler.Wait());
    //Expecting two registration updates
    EXPECT_EQ(2, cbHandler.count);
    pthread_join(executeThread, NULL);

    AwaServerExecuteOperation_Free(&updateExecute);
}

TEST_F(TestServerEventsWithConnectedSession, ClientDeregisterEvent)
{

    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_TRUE(NULL != operation);

    SingleStaticClientPollCondition condition(client_, operation, global::clientEndpointName, 20);
    ASSERT_TRUE(condition.Wait());

    AwaServerListClientsOperation_Free(&operation);

    struct CallbackHandler1 : public EventWaitCondition
    {
        int count;

        CallbackHandler1(AwaServerSession * ServerSession, AwaStaticClient * StaticClient, std::string ClientEndpointName, int callbackCountMax = 1) :
            EventWaitCondition(ServerSession, StaticClient, ClientEndpointName, callbackCountMax), count(0) {}

        void callbackHandler(const void * event)
        {
            count ++;
            AwaServerClientDeregisterEvent * deregisterEvent = (AwaServerClientDeregisterEvent *)event;
            EXPECT_TRUE(deregisterEvent != NULL);

            AwaClientIterator * clientIterator = AwaServerClientDeregisterEvent_NewClientIterator(deregisterEvent);
            EXPECT_TRUE(clientIterator != NULL);

            EXPECT_TRUE(AwaClientIterator_Next(clientIterator));

            EXPECT_STREQ(global::clientEndpointName, AwaClientIterator_GetClientID(clientIterator));

            AwaClientIterator_Free(&clientIterator);
        }
    };
    CallbackHandler1 cbHandler(session_, client_, global::clientEndpointName);

    EXPECT_EQ(AwaError_Success, AwaServerSession_SetClientDeregisterEventCallback(session_, (void (*)(const _AwaServerClientDeregisterEvent*, void*))ClientEventCallback, &cbHandler));

    // stop the static client, to cause a deregister
    cbHandler.Deregister();
    client_ = NULL;

    EXPECT_TRUE(cbHandler.Wait());
    EXPECT_EQ(1, cbHandler.count);
}

// test overwrite callback

// test clear callback

} // namespace Awa

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
#include "../src/objects_tree.h"

#include "awa/client.h"
#include "awa/common.h"
#include "memalloc.h"
#include "log.h"
#include "arrays.h"
#include "support/support.h"
#include "client_subscribe.h"
#include "client_session.h"
#include "subscribe_observe_common.h"
#include "utils.h"
#include "xmltree.h"

namespace Awa {

class TestSubscribeObserveCommon : public TestClientWithConnectedSession
{
public:
    unsigned int count_;
    std::vector<const char*> expectedPaths_;
    TestSubscribeObserveCommon() : count_(0) {}
};
static TestSubscribeObserveCommon * context;

void callbackHandler1(MapType * observers, const char * path, const AwaChangeSet * changeSet)
{
    ASSERT_TRUE(NULL != observers);
    ASSERT_TRUE(NULL != path);
    ASSERT_TRUE(NULL != changeSet);
    ASSERT_TRUE(context->count_ < context->expectedPaths_.size());
    EXPECT_STREQ(context->expectedPaths_[context->count_], path);
    context->count_++;
}

TEST_F(TestSubscribeObserveCommon, SubscribeObserveCommon_CallObservers_handles_single_object_instance)
{
    MapType * observers = Map_New();

    const char * xml =
               "<Objects>"
               "  <Object>"
               "    <ID>3</ID>"
               "    <ObjectInstance>"
               "      <ID>0</ID>"
               "    </ObjectInstance>"
               "  </Object>"
               "</Objects>";

    TreeNode objectsTree = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);

    AwaChangeSet * changeSet = ChangeSet_New(session_, SessionType_Client, objectsTree);

    expectedPaths_.push_back("/3/0");
    expectedPaths_.push_back("/3");

    context = this;
    SubscribeObserveCommon_CallObservers(observers, changeSet, callbackHandler1);

    Map_Free(&observers);
    EXPECT_TRUE(NULL == observers);

    ChangeSet_Free(&changeSet);
    EXPECT_TRUE(NULL == changeSet);

    Tree_Delete(objectsTree);
}

TEST_F(TestSubscribeObserveCommon, SubscribeObserveCommon_CallObservers_handles_multiple_resources)
{
    MapType * observers = Map_New();

    const char * xml =
               "<Objects>"
               "  <Object>"
               "    <ID>3</ID>"
               "    <ObjectInstance>"
               "      <ID>0</ID>"
               "      <Resource>"
               "        <ID>4</ID>"
               "      </Resource>"
               "      <Resource>"
               "        <ID>5</ID>"
               "      </Resource>"
               "    </ObjectInstance>"
               "  </Object>"
               "</Objects>";

    TreeNode objectsTree = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);

    AwaChangeSet * changeSet = ChangeSet_New(session_, SessionType_Client, objectsTree);

    expectedPaths_.push_back("/3/0/4");
    expectedPaths_.push_back("/3/0/5");
    expectedPaths_.push_back("/3/0");
    expectedPaths_.push_back("/3");

    context = this;
    SubscribeObserveCommon_CallObservers(observers, changeSet, callbackHandler1);

    Map_Free(&observers);
    EXPECT_TRUE(NULL == observers);

    ChangeSet_Free(&changeSet);
    EXPECT_TRUE(NULL == changeSet);

    Tree_Delete(objectsTree);
}

} // namespace Awa


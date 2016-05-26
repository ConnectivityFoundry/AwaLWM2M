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

#include <cstdio>
#include <fstream>

#include <unistd.h>

#include <gtest/gtest.h>

#include "support/support.h"
#include "support/daemon.h"

#include "awa/server.h"

namespace Awa {

struct SingleClientWaitCondition : public WaitCondition
{
    AwaServerListClientsOperation * Operation;
    std::string ClientEndpointName;

    SingleClientWaitCondition(AwaServerListClientsOperation * Operation, const std::string & ClientEndpointName) :
        Operation(Operation), ClientEndpointName(ClientEndpointName) {}
    virtual ~SingleClientWaitCondition() {}
    virtual bool Check()
    {
        AwaClientIterator * iterator = NULL;
        bool result =
                (AwaError_Success == AwaServerListClientsOperation_Perform(Operation, defaults::timeout)) &&
                ((iterator = AwaServerListClientsOperation_NewClientIterator(Operation)) != NULL) &&
                (AwaClientIterator_Next(iterator)) &&
                (ClientEndpointName.compare(AwaClientIterator_GetClientID(iterator)) == 0);
        AwaClientIterator_Free(&iterator);
        return result;
    }
};

class TestBootstrapServer : public TestAwaBase {};

TEST_F(TestBootstrapServer, bootstrap_with_single_client)
{
    std::string testDescription = std::string(CURRENT_TEST_CASE_NAME + std::string(".") + CURRENT_TEST_NAME);

    int bootstrapServerCoapPort = 23667;
    std::string bootstrapServerAddress = "127.0.0.1";
    std::string bootstrapURI = std::string("coap://") + bootstrapServerAddress + std::string(":") + std::to_string(bootstrapServerCoapPort);
    AwaBootstrapServerDaemon bootstrapServerDaemon;
    bootstrapServerDaemon.SetCoapPort(bootstrapServerCoapPort);

    std::string serverAddress = "127.0.0.1";
    int serverCoapPort = 44443;
    int serverIpcPort = 6301;
    AwaServerDaemon serverDaemon;
    serverDaemon.SetCoapPort(serverCoapPort);
    serverDaemon.SetIpcPort(serverIpcPort);

    int clientIpcPort = 6401;
    std::string clientEndpointName = "BootstrapTestClient";
    AwaClientDaemon clientDaemon;
    clientDaemon.SetIpcPort(clientIpcPort);
    clientDaemon.SetEndpointName(clientEndpointName);
    clientDaemon.SetBootstrapURI(bootstrapURI);

    BootstrapConfigFile bootstrapConfigFile(TempFilename().GetFilename(), serverAddress, serverCoapPort);
    bootstrapServerDaemon.SetConfigFile(bootstrapConfigFile.GetFilename());

    // start the bootstrap, server and client daemons
    ASSERT_TRUE(bootstrapServerDaemon.Start());
    ASSERT_TRUE(serverDaemon.Start());
    ASSERT_TRUE(clientDaemon.Start());

    // wait for the client to register with the server
    AwaServerSession * session = AwaServerSession_New();
    ASSERT_TRUE(NULL != session);
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session, serverAddress.c_str(), serverIpcPort));
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));

    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session);
    ASSERT_TRUE(NULL != operation);

    SingleClientWaitCondition condition(operation, clientEndpointName);
    EXPECT_TRUE(condition.Wait());

    AwaServerListClientsOperation_Free(&operation);
    AwaServerSession_Free(&session);

    clientDaemon.Stop();
    serverDaemon.Stop();
    bootstrapServerDaemon.Stop();
}

// TODO: test Network Interface
// TODO: test Address Family
// TODO: test alternative ports
// TODO: test alternative config files

// TODO: test multiple clients
// TODO: test multiple servers (multiple config files)

} // namespace Awa

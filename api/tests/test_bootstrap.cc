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

    SingleClientWaitCondition(AwaServerListClientsOperation * Operation, const std::string ClientEndpointName) :
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

    std::string bootstrapConfigFilename = tmpnam(NULL);
    BootstrapConfigFile bootstrapConfigFile (bootstrapConfigFilename, serverAddress, serverCoapPort);
    bootstrapServerDaemon.SetConfigFile(bootstrapConfigFile.GetFilename());

    // start the bootstrap, server and client daemons
    ASSERT_TRUE(bootstrapServerDaemon.Start(testDescription));
    ASSERT_TRUE(serverDaemon.Start(testDescription));
    ASSERT_TRUE(clientDaemon.Start(testDescription));

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

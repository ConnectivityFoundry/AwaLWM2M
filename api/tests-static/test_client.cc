#include <gtest/gtest.h>
#include "awa/static.h"
#include "awa/server.h"
#include "../tests/support/support.h"
#include "../tests/support/daemon.h"

namespace StaticClient {
int staticClientProcessBootstrapTimeout = 10;

}

namespace Awa {

class TestStaticClient : public testing::Test
{

};

TEST_F(TestStaticClient, AwaStaticClient_New_Free)
{
    AwaStaticClient * client = AwaStaticClient_New();
    ASSERT_TRUE(client != NULL);

    AwaStaticClient_Free(&client);
    ASSERT_TRUE(client == NULL);
}

TEST_F(TestStaticClient, AwaStaticClient_Init_NULL_Client)
{
    ASSERT_EQ(AwaError_StaticClientInvalid, AwaStaticClient_Init(NULL));
}

TEST_F(TestStaticClient, AwaStaticClient_New_Init_Unconfigured_Client)
{
    AwaStaticClient * client = AwaStaticClient_New();
    EXPECT_TRUE(client != NULL);

    ASSERT_EQ(AwaError_StaticClientNotConfigured, AwaStaticClient_Init(client));

    AwaStaticClient_Free(&client);
    EXPECT_TRUE(client == NULL);
}

TEST_F(TestStaticClient, AwaStaticClient_SetBootstrapServerURI_valid_input)
{
    AwaStaticClient * client = AwaStaticClient_New();
    EXPECT_TRUE(client != NULL);

    ASSERT_EQ(AwaError_Success, AwaStaticClient_SetBootstrapServerURI(client, "coap://127.0.0.1:15683/"));

    AwaStaticClient_Free(&client);
    EXPECT_TRUE(client == NULL);
}

TEST_F(TestStaticClient, AwaStaticClient_SetBootstrapServerURI_invalid_input)
{
    AwaStaticClient * client = AwaStaticClient_New();
    EXPECT_TRUE(client != NULL);

    ASSERT_EQ(AwaError_StaticClientInvalid, AwaStaticClient_SetBootstrapServerURI(NULL, "coap://127.0.0.1:15683/"));
    ASSERT_EQ(AwaError_StaticClientInvalid, AwaStaticClient_SetBootstrapServerURI(client, NULL));

    AwaStaticClient_Free(&client);
    EXPECT_TRUE(client == NULL);
}

TEST_F(TestStaticClient, AwaStaticClient_SetEndPointName_valid_input)
{
    AwaStaticClient * client = AwaStaticClient_New();
    EXPECT_TRUE(client != NULL);

    ASSERT_EQ(AwaError_Success, AwaStaticClient_SetEndPointName(client, "imagination1"));

    AwaStaticClient_Free(&client);
    EXPECT_TRUE(client == NULL);
}

TEST_F(TestStaticClient, AwaStaticClient_SetEndPointName_invalid_input)
{
    AwaStaticClient * client = AwaStaticClient_New();
    EXPECT_TRUE(client != NULL);

    ASSERT_EQ(AwaError_StaticClientInvalid, AwaStaticClient_SetEndPointName(NULL, "test"));
    ASSERT_EQ(AwaError_StaticClientInvalid, AwaStaticClient_SetEndPointName(client, NULL));

    AwaStaticClient_Free(&client);
    EXPECT_TRUE(client == NULL);
}

TEST_F(TestStaticClient, AwaStaticClient_SetEndPointName_long_name)
{
    AwaStaticClient * client = AwaStaticClient_New();
    EXPECT_TRUE(client != NULL);

    ASSERT_EQ(AwaError_OutOfMemory, AwaStaticClient_SetEndPointName(client, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"));

    AwaStaticClient_Free(&client);
    EXPECT_TRUE(client == NULL);
}

TEST_F(TestStaticClient, AwaStaticClient_SetCOAPListenAddressPort_valid_input)
{
    AwaStaticClient * client = AwaStaticClient_New();
    EXPECT_TRUE(client != NULL);

    ASSERT_EQ(AwaError_Success, AwaStaticClient_SetCOAPListenAddressPort(client, "::", 5683));

    AwaStaticClient_Free(&client);
    EXPECT_TRUE(client == NULL);
}

TEST_F(TestStaticClient, AwaStaticClient_SetCOAPListenAddressPort_invalid_input)
{
    AwaStaticClient * client = AwaStaticClient_New();
    EXPECT_TRUE(client != NULL);

    ASSERT_EQ(AwaError_StaticClientInvalid, AwaStaticClient_SetCOAPListenAddressPort(NULL, "test", 5683));
    ASSERT_EQ(AwaError_StaticClientInvalid, AwaStaticClient_SetCOAPListenAddressPort(client, NULL, 5683));

    AwaStaticClient_Free(&client);
    EXPECT_TRUE(client == NULL);
}

TEST_F(TestStaticClient, AwaStaticClient_SetCOAPListenAddressPort_long_name)
{
    AwaStaticClient * client = AwaStaticClient_New();
    EXPECT_TRUE(client != NULL);

    ASSERT_EQ(AwaError_OutOfMemory, AwaStaticClient_SetCOAPListenAddressPort(client, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 5683));

    AwaStaticClient_Free(&client);
    EXPECT_TRUE(client == NULL);
}


TEST_F(TestStaticClient, AwaStaticClient_Init_not_configured)
{
    AwaStaticClient * client = AwaStaticClient_New();
    EXPECT_TRUE(client != NULL);

    ASSERT_EQ(AwaError_StaticClientNotConfigured, AwaStaticClient_Init(client));

    AwaStaticClient_Free(&client);
    EXPECT_TRUE(client == NULL);
}

TEST_F(TestStaticClient, AwaStaticClient_Init_valid_inputs)
{
    AwaStaticClient * client = AwaStaticClient_New();
    EXPECT_TRUE(client != NULL);

    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetBootstrapServerURI(client, "coap://127.0.0.1:15683/"));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetEndPointName(client, "imagination1"));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetCOAPListenAddressPort(client, "0.0.0.0", 5683));

    ASSERT_EQ(AwaError_Success, AwaStaticClient_Init(client));

    AwaStaticClient_Free(&client);
    EXPECT_TRUE(client == NULL);
}

//This test is for valgrind
TEST_F(TestStaticClient, AwaStaticClient_Process)
{
    AwaStaticClient * client = AwaStaticClient_New();
    EXPECT_TRUE(client != NULL);

    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetBootstrapServerURI(client, "coap://127.0.0.1:15683/"));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetEndPointName(client, "imagination1"));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetCOAPListenAddressPort(client, "0.0.0.0", 5683));

    EXPECT_EQ(AwaError_Success, AwaStaticClient_Init(client));

    AwaStaticClient_Process(client);
    AwaStaticClient_Process(client);
    AwaStaticClient_Process(client);
    AwaStaticClient_Process(client);

    AwaStaticClient_Free(&client);
    EXPECT_TRUE(client == NULL);
}

TEST_F(TestStaticClient,  AwaStaticClient_Bootstrap_Test)
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

    std::string bootstrapConfigFilename = tmpnam(NULL);
    BootstrapConfigFile bootstrapConfigFile (bootstrapConfigFilename, serverAddress, serverCoapPort);
    bootstrapServerDaemon.SetConfigFile(bootstrapConfigFile.GetFilename());

    // start the bootstrap and server daemons
    EXPECT_TRUE(bootstrapServerDaemon.Start(testDescription));
    EXPECT_TRUE(serverDaemon.Start(testDescription));

    std::string clientEndpointName = "BootstrapTestClient";
    AwaStaticClient * client = AwaStaticClient_New();
    EXPECT_TRUE(client != NULL);

    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetBootstrapServerURI(client, bootstrapURI.c_str()));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetEndPointName(client, clientEndpointName.c_str()));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetCOAPListenAddressPort(client, "0.0.0.0", 5683));

    EXPECT_EQ(AwaError_Success, AwaStaticClient_Init(client));

    // wait for the client to register with the server
    AwaServerSession * session = AwaServerSession_New();
    EXPECT_TRUE(NULL != session);
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session, serverAddress.c_str(), serverIpcPort));
    EXPECT_EQ(AwaError_Success, AwaServerSession_Connect(session));


    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session);
    EXPECT_TRUE(NULL != operation);

    bool found = false;
    int counter = 0;

    while(counter < StaticClient::staticClientProcessBootstrapTimeout && !found)
    {
        EXPECT_EQ(AwaError_Success, AwaServerListClientsOperation_Perform(operation, defaults::timeout));
        AwaClientIterator * iterator = AwaServerListClientsOperation_NewClientIterator(operation);
        EXPECT_TRUE(iterator != NULL);
        if (AwaClientIterator_Next(iterator))
        {
            if (clientEndpointName.compare(AwaClientIterator_GetClientID(iterator)) == 0)
            {
                found = true;
                printf("Took %d iterations of AwaStaticClient_Process to bootstrap and register.\n", counter);
            }
        }
        counter++;
        AwaClientIterator_Free(&iterator);
        AwaStaticClient_Process(client);
    }

    //Check it hasn't timed out.
    ASSERT_NE(counter, StaticClient::staticClientProcessBootstrapTimeout);
    ASSERT_TRUE(found);

    AwaServerListClientsOperation_Free(&operation);
    AwaServerSession_Free(&session);

    AwaStaticClient_Free(&client);
    EXPECT_TRUE(client == NULL);

    serverDaemon.Stop();
    bootstrapServerDaemon.Stop();
}

extern "C" {

Lwm2mResult handler(void * context, LWM2MOperation operation, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID, void ** dataPointer, uint16_t * dataSize, bool * changed)
{
    Lwm2mResult result = Lwm2mResult_InternalError;
    printf("\n\nHandler called for operation %d\n\n", operation);

    switch(operation)
    {
        case LWM2MOperation_CreateResource:
            result = Lwm2mResult_SuccessCreated;
            break;
        case LWM2MOperation_CreateObjectInstance:
            result = Lwm2mResult_SuccessCreated;
            break;
        default:
            break;
    }

    return result;
}


};

TEST_F(TestStaticClient, AwaStaticClient_Register_Object)
{
    AwaStaticClient * client = AwaStaticClient_New();
    EXPECT_TRUE(client != NULL);

    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetBootstrapServerURI(client, "coap://127.0.0.1:15683/"));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetEndPointName(client, "imagination1"));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetCOAPListenAddressPort(client, "0.0.0.0", 5683));

    EXPECT_EQ(AwaError_Success, AwaStaticClient_Init(client));

    EXPECT_EQ(AwaError_Success,AwaStaticClient_RegisterObjectWithHandler(client, "TestObject", 9999, 0, 1, handler));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_RegisterResourceWithHandler(client, "test", 9999, 1, ResourceTypeEnum_TypeInteger, 1, 1, AwaAccess_Read, handler));

    AwaStaticClient_CreateObjectInstance(client, 9999, 0);

    AwaStaticClient_Process(client);
    AwaStaticClient_Process(client);
    AwaStaticClient_Process(client);
    AwaStaticClient_Process(client);

    AwaStaticClient_Free(&client);
    EXPECT_TRUE(client == NULL);
}

} // namespace Awa

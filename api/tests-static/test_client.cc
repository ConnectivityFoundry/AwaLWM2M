#include <pthread.h>
#include <gtest/gtest.h>
#include "awa/static.h"
#include "awa/server.h"
#include "../tests/support/support.h"
#include "../tests/support/daemon.h"

namespace StaticClient {
int staticClientProcessBootstrapTimeout = 10;

}

namespace Awa {


struct SignleStaticClientPollCondition : public PollCondition
{
    AwaStaticClient * StaticClient;
    AwaServerListClientsOperation * Operation;
    std::string ClientEndpointName;

    SignleStaticClientPollCondition(AwaStaticClient * StaticClient, AwaServerListClientsOperation * Operation, std::string ClientEndpointName, int maxCount) :
        PollCondition(maxCount), StaticClient(StaticClient), Operation(Operation), ClientEndpointName(ClientEndpointName) {}
    virtual ~SignleStaticClientPollCondition() {}

    virtual bool Check()
    {
        bool found = false;

        EXPECT_EQ(AwaError_Success, AwaServerListClientsOperation_Perform(Operation, defaults::timeout));
        AwaClientIterator * iterator = AwaServerListClientsOperation_NewClientIterator(Operation);
        EXPECT_TRUE(iterator != NULL);
        if (AwaClientIterator_Next(iterator))
        {
            if (ClientEndpointName.compare(AwaClientIterator_GetClientID(iterator)) == 0)
            {
                found = true;
            }
        }
        AwaClientIterator_Free(&iterator);
        AwaStaticClient_Process(StaticClient);
        return found;
    }
};

class TestStaticClient : public testing::Test {};

class TestStaticClientWithServer : public TestServerWithConnectedSession
{
protected:
    virtual void SetUp() {
        TestServerWithConnectedSession::SetUp();

        std::string serverURI = std::string("coap://127.0.0.1:") + std::to_string(global::serverCoapPort) + "/";
        std::string clientEndpointName = "TestClient";
        client_ = AwaStaticClient_New();
        EXPECT_TRUE(client_ != NULL);

        BootstrapInfo bootstrapinfo = { 0 };

        sprintf(bootstrapinfo.SecurityInfo.ServerURI, "%s", serverURI.c_str());
        bootstrapinfo.SecurityInfo.Bootstrap = false;
        bootstrapinfo.SecurityInfo.SecurityMode = 0;
        sprintf(bootstrapinfo.SecurityInfo.PublicKey, "[PublicKey]");
        sprintf(bootstrapinfo.SecurityInfo.SecretKey, "[SecretKey]");
        bootstrapinfo.SecurityInfo.ServerID = 1;
        bootstrapinfo.SecurityInfo.HoldOffTime = 30;

        bootstrapinfo.ServerInfo.ShortServerID = 1;
        bootstrapinfo.ServerInfo.LifeTime = 30;
        bootstrapinfo.ServerInfo.MinPeriod = 1;
        bootstrapinfo.ServerInfo.MaxPeriod = -1;
        bootstrapinfo.ServerInfo.DisableTimeout = 86400;
        bootstrapinfo.ServerInfo.Notification = false;
        sprintf(bootstrapinfo.ServerInfo.Binding, "U");

        EXPECT_EQ(AwaError_Success, AwaStaticClient_SetBootstrapServerURI(client_, ""));
        EXPECT_EQ(AwaError_Success, AwaStaticClient_SetEndPointName(client_, global::clientEndpointName));
        EXPECT_EQ(AwaError_Success, AwaStaticClient_SetCOAPListenAddressPort(client_, "0.0.0.0", global::clientLocalCoapPort));

        EXPECT_EQ(AwaError_Success, AwaStaticClient_Init(client_));

        EXPECT_EQ(AwaError_Success, AwaStaticClient_SetFactoryBootstrapInformation(client_, &bootstrapinfo));
    }

    virtual void TearDown() {
        AwaStaticClient_Free(&client_);
        EXPECT_TRUE(client_ == NULL);
        TestServerWithConnectedSession::TearDown();
    }

    AwaStaticClient * client_;
};


class StaticClientCallbackPollCondition : public PollCondition
{
    AwaStaticClient * StaticClient;


public:
    StaticClientCallbackPollCondition(AwaStaticClient * StaticClient, int maxCount) :
        PollCondition(maxCount), StaticClient(StaticClient), complete(false) {}
    bool complete;
    virtual bool Check()
    {
        printf("Check...\n");
        AwaStaticClient_Process(StaticClient);
        return complete;
    }

    virtual Lwm2mResult handler(void * context, LWM2MOperation operation, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID, void ** dataPointer, int * dataSize, bool * changed)    {
        return Lwm2mResult_InternalError;
    };
};

Lwm2mResult handler(void * context, LWM2MOperation operation, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID, void ** dataPointer, int * dataSize, bool * changed)
{
    AwaStaticClient * client = (AwaStaticClient *)context;
    Lwm2mResult result = Lwm2mResult_InternalError;

    std::cerr << "Handler for " <<std::to_string(operation) << std::endl;

    void * callback = AwaStaticClient_GetApplicationContext(client);

    if (callback)
    {
        auto * callbackClass = static_cast<StaticClientCallbackPollCondition*>(callback);
        result = callbackClass->handler(context, operation, objectID, objectInstanceID, resourceID, resourceInstanceID, dataPointer, dataSize, changed);
    }

    return result;
}



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

TEST_F(TestStaticClient, AwaStaticClient_SetApplicationContext_SetApplicationContext_invalid_inputs)
{
    ASSERT_TRUE(NULL == AwaStaticClient_GetApplicationContext(NULL));
    ASSERT_EQ(AwaError_StaticClientInvalid, AwaStaticClient_SetApplicationContext(NULL, NULL));
}

TEST_F(TestStaticClient, AwaStaticClient_SetApplicationContext_SetApplicationContext_valid_inputs)
{
    int dummycontext = 5;
    void * applicationContext = &dummycontext;
    AwaStaticClient * client = AwaStaticClient_New();
    EXPECT_TRUE(client != NULL);

    ASSERT_EQ(AwaError_Success, AwaStaticClient_SetApplicationContext(client, applicationContext));
    ASSERT_EQ(applicationContext, AwaStaticClient_GetApplicationContext(client));

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

    SignleStaticClientPollCondition condition(client, operation, clientEndpointName, 10);
    ASSERT_TRUE(condition.Wait());

    AwaServerListClientsOperation_Free(&operation);
    AwaServerSession_Free(&session);

    AwaStaticClient_Free(&client);
    EXPECT_TRUE(client == NULL);

    serverDaemon.Stop();
    bootstrapServerDaemon.Stop();
}

TEST_F(TestStaticClientWithServer,  AwaStaticClient_Factory_Bootstrap_Test)
{
    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_TRUE(NULL != operation);

    SignleStaticClientPollCondition condition(client_, operation, global::clientEndpointName, 20);
    ASSERT_TRUE(condition.Wait());

    AwaServerListClientsOperation_Free(&operation);
}

TEST_F(TestStaticClient, AwaStaticClient_Create_Operation_for_Object_and_Resource)
{
    AwaStaticClient * client = AwaStaticClient_New();
    EXPECT_TRUE(client != NULL);

    struct callback1 : public StaticClientCallbackPollCondition
    {
        callback1(AwaStaticClient * StaticClient, int maxCount) : StaticClientCallbackPollCondition(StaticClient, maxCount) {};

        Lwm2mResult handler(void * context, LWM2MOperation operation, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID, void ** dataPointer, int * dataSize, bool * changed)
        {
            Lwm2mResult result = Lwm2mResult_InternalError;
            EXPECT_TRUE((operation == LWM2MOperation_CreateResource) || (operation == LWM2MOperation_CreateObjectInstance));

            if (operation == LWM2MOperation_CreateObjectInstance)
            {
                EXPECT_EQ(9999, objectID);
                EXPECT_EQ(0, objectInstanceID);
                result = Lwm2mResult_SuccessCreated;
            }
            else if (operation == LWM2MOperation_CreateResource)
            {
                EXPECT_EQ(9999, objectID);
                EXPECT_EQ(0, objectInstanceID);
                EXPECT_EQ(1, resourceID);
                complete = true;
                result = Lwm2mResult_SuccessCreated;
            }

            return result;
        }
    };

    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetBootstrapServerURI(client, "coap://127.0.0.1:15683/"));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetEndPointName(client, "imagination1"));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetCOAPListenAddressPort(client, "0.0.0.0", 5683));

    callback1 cbHandler(client, 20);

    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetApplicationContext(client, &cbHandler));

    EXPECT_EQ(AwaError_Success, AwaStaticClient_Init(client));

    EXPECT_EQ(AwaError_Success,AwaStaticClient_RegisterObjectWithHandler(client, "TestObject", 9999, 0, 1, handler));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_RegisterResourceWithHandler(client, "test", 9999, 1, ResourceTypeEnum_TypeInteger, 1, 1, AwaAccess_Read, handler));

    ASSERT_EQ(AwaError_Success, AwaStaticClient_CreateObjectInstance(client, 9999, 0));

    ASSERT_TRUE(cbHandler.Wait());

    AwaStaticClient_Free(&client);
    EXPECT_TRUE(client == NULL);
}

void * do_write_operation(void * attr)
{
    AwaServerWriteOperation * writeOperation = (AwaServerWriteOperation *)attr;
    sleep(2);
    AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout);
    return 0;
}

TEST_F(TestStaticClientWithServer, AwaStaticClient_Create_and_Write_Operation_for_Object_and_Resource)
{
    struct callback1 : public StaticClientCallbackPollCondition
    {
        callback1(AwaStaticClient * StaticClient, int maxCount) : StaticClientCallbackPollCondition(StaticClient, maxCount) {};

        Lwm2mResult handler(void * context, LWM2MOperation operation, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID, void ** dataPointer, int * dataSize, bool * changed)
        {
            Lwm2mResult result = Lwm2mResult_InternalError;
            EXPECT_TRUE((operation == LWM2MOperation_CreateResource) || (operation == LWM2MOperation_CreateObjectInstance) || (operation == LWM2MOperation_Write));

            if (operation == LWM2MOperation_CreateObjectInstance)
            {
                EXPECT_EQ(9999, objectID);
                EXPECT_EQ(0, objectInstanceID);
                result = Lwm2mResult_SuccessCreated;
            }
            else if (operation == LWM2MOperation_CreateResource)
            {
                EXPECT_EQ(9999, objectID);
                EXPECT_EQ(0, objectInstanceID);
                EXPECT_EQ(1, resourceID);
                result = Lwm2mResult_SuccessCreated;
            }
            else if (operation == LWM2MOperation_Write)
            {
                AwaInteger * integer = (AwaInteger *)(*dataPointer);
                EXPECT_TRUE(dataPointer != NULL);
                EXPECT_TRUE(*dataPointer != NULL);
                EXPECT_TRUE(dataSize != NULL);
                EXPECT_EQ(static_cast<int>(sizeof(AwaInteger)), *dataSize);
                EXPECT_EQ(5, *integer);
                complete = true;
                result = Lwm2mResult_SuccessChanged;
            }

            return result;
        }
    };

    AwaServerDefineOperation * defineOpertaion = AwaServerDefineOperation_New(session_);
    EXPECT_TRUE(defineOpertaion != NULL);
    AwaObjectDefinition * objectDefintion = AwaObjectDefinition_New(9999, "TestObject", 0, 1);
    EXPECT_TRUE(objectDefintion != NULL);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefintion, 1, "TestResource", true, AwaResourceOperations_ReadWrite, 0));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(defineOpertaion, objectDefintion));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(defineOpertaion, defaults::timeout));
    AwaServerDefineOperation_Free(&defineOpertaion);
    AwaObjectDefinition_Free(&objectDefintion);

    callback1 cbHandler(client_, 10);

    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetApplicationContext(client_, &cbHandler));
    EXPECT_EQ(AwaError_Success,AwaStaticClient_RegisterObjectWithHandler(client_, "TestObject", 9999, 0, 1, handler));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_RegisterResourceWithHandler(client_, "TestResource", 9999, 1, ResourceTypeEnum_TypeInteger, 1, 1, AwaAccess_ReadWrite, handler));

    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    EXPECT_TRUE(writeOperation != NULL);
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/9999/0"));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/9999/0/1", 5));

    pthread_t t;
    pthread_create(&t, NULL, do_write_operation, (void *)writeOperation);

    ASSERT_TRUE(cbHandler.Wait());

    pthread_join(t, NULL);

    AwaServerWriteOperation_Free(&writeOperation);
}

void * do_read_operation(void * attr)
{
    AwaServerReadOperation * readOperation = (AwaServerReadOperation *)attr;
    sleep(2);
    AwaServerReadOperation_Perform(readOperation, defaults::timeout);
    return 0;
}

TEST_F(TestStaticClientWithServer, AwaStaticClient_Create_and_Read_Operation_for_Object_and_Resource)
{
    struct callback1 : public StaticClientCallbackPollCondition
    {
        AwaInteger integer = 5;
        int counter = 0;

        callback1(AwaStaticClient * StaticClient, int maxCount) : StaticClientCallbackPollCondition(StaticClient, maxCount) {};

        Lwm2mResult handler(void * context, LWM2MOperation operation, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID, void ** dataPointer, int * dataSize, bool * changed)
        {
            Lwm2mResult result = Lwm2mResult_InternalError;
            EXPECT_TRUE((operation == LWM2MOperation_CreateResource) || (operation == LWM2MOperation_CreateObjectInstance) || (operation == LWM2MOperation_Read));

            if (operation == LWM2MOperation_CreateObjectInstance)
            {
                EXPECT_EQ(9999, objectID);
                EXPECT_EQ(0, objectInstanceID);
                result = Lwm2mResult_SuccessCreated;
            }
            else if (operation == LWM2MOperation_CreateResource)
            {
                EXPECT_EQ(9999, objectID);
                EXPECT_EQ(0, objectInstanceID);
                EXPECT_EQ(1, resourceID);
                result = Lwm2mResult_SuccessCreated;
            }
            else if (operation == LWM2MOperation_Read)
            {
                EXPECT_TRUE(dataPointer != NULL);
                EXPECT_TRUE(dataSize != NULL);
                *dataPointer = &integer;
                *dataSize = sizeof(integer);
                complete = counter++ == 1 ? true : false;
                result = Lwm2mResult_SuccessContent;
            }

            return result;
        }
    };

    AwaServerDefineOperation * defineOpertaion = AwaServerDefineOperation_New(session_);
    EXPECT_TRUE(defineOpertaion != NULL);
    AwaObjectDefinition * objectDefintion = AwaObjectDefinition_New(9999, "TestObject", 0, 1);
    EXPECT_TRUE(objectDefintion != NULL);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefintion, 1, "TestResource", true, AwaResourceOperations_ReadWrite, 0));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(defineOpertaion, objectDefintion));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(defineOpertaion, defaults::timeout));
    AwaServerDefineOperation_Free(&defineOpertaion);
    AwaObjectDefinition_Free(&objectDefintion);

    callback1 cbHandler(client_, 20);

    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetApplicationContext(client_, &cbHandler));
    EXPECT_EQ(AwaError_Success,AwaStaticClient_RegisterObjectWithHandler(client_, "TestObject", 9999, 0, 1, handler));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_RegisterResourceWithHandler(client_, "TestResource", 9999, 1, ResourceTypeEnum_TypeInteger, 1, 1, AwaAccess_ReadWrite, handler));

    EXPECT_EQ(AwaError_Success, AwaStaticClient_CreateObjectInstance(client_, 9999, 0));


    AwaServerReadOperation * readOperation = AwaServerReadOperation_New(session_);
    EXPECT_TRUE(readOperation != NULL);
    EXPECT_EQ(AwaError_Success, AwaServerReadOperation_AddPath(readOperation, global::clientEndpointName, "/9999/0"));

    pthread_t t;
    pthread_create(&t, NULL, do_read_operation, (void *)readOperation);

    ASSERT_TRUE(cbHandler.Wait());

    pthread_join(t, NULL);

    AwaServerReadOperation_Free(&readOperation);
}


void * do_delete_operation(void * attr)
{
    AwaServerDeleteOperation * deleteOperation = (AwaServerDeleteOperation *)attr;
    sleep(2);
    AwaServerDeleteOperation_Perform(deleteOperation, defaults::timeout);
    return 0;
}

TEST_F(TestStaticClientWithServer, AwaStaticClient_Create_and_Delete_Operation_for_Object_and_Resource)
{
    struct callback1 : public StaticClientCallbackPollCondition
    {

        callback1(AwaStaticClient * StaticClient, int maxCount) : StaticClientCallbackPollCondition(StaticClient, maxCount) {};

        Lwm2mResult handler(void * context, LWM2MOperation operation, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID, void ** dataPointer, int * dataSize, bool * changed)
        {
            Lwm2mResult result = Lwm2mResult_InternalError;
            EXPECT_TRUE((operation == LWM2MOperation_CreateResource) || (operation == LWM2MOperation_CreateObjectInstance) || (operation == LWM2MOperation_DeleteObjectInstance) || (operation == LWM2MOperation_Read));

            if ((operation == LWM2MOperation_CreateObjectInstance) || (operation == LWM2MOperation_DeleteObjectInstance))
            {
                EXPECT_EQ(9999, objectID);
                EXPECT_EQ(0, objectInstanceID);
                result = operation == LWM2MOperation_CreateObjectInstance ? Lwm2mResult_SuccessCreated : Lwm2mResult_SuccessDeleted;

                if (operation == LWM2MOperation_DeleteObjectInstance)
                    complete = true;
            }
            else if ((operation == LWM2MOperation_CreateResource) )
            {
                EXPECT_EQ(9999, objectID);
                EXPECT_EQ(0, objectInstanceID);
                EXPECT_EQ(1, resourceID);
                result = Lwm2mResult_SuccessCreated;


            }

            return result;
        }
    };

    AwaServerDefineOperation * defineOpertaion = AwaServerDefineOperation_New(session_);
    EXPECT_TRUE(defineOpertaion != NULL);
    AwaObjectDefinition * objectDefintion = AwaObjectDefinition_New(9999, "TestObject", 0, 1);
    EXPECT_TRUE(objectDefintion != NULL);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefintion, 1, "TestResource", false, AwaResourceOperations_ReadWrite, 0));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(defineOpertaion, objectDefintion));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(defineOpertaion, defaults::timeout));
    AwaServerDefineOperation_Free(&defineOpertaion);
    AwaObjectDefinition_Free(&objectDefintion);

    callback1 cbHandler(client_, 20);

    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetApplicationContext(client_, &cbHandler));
    EXPECT_EQ(AwaError_Success,AwaStaticClient_RegisterObjectWithHandler(client_, "TestObject", 9999, 0, 1, handler));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_RegisterResourceWithHandler(client_, "TestResource", 9999, 1, ResourceTypeEnum_TypeInteger, 0, 1, AwaAccess_ReadWrite, handler));

    EXPECT_EQ(AwaError_Success, AwaStaticClient_CreateObjectInstance(client_, 9999, 0));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_CreateResource(client_, 9999, 0, 1));

    AwaServerDeleteOperation * deleteOperation = AwaServerDeleteOperation_New(session_);
    EXPECT_TRUE(deleteOperation != NULL);
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(deleteOperation, global::clientEndpointName, "/9999/0"));

    pthread_t t;
    pthread_create(&t, NULL, do_delete_operation, (void *)deleteOperation);

    ASSERT_TRUE(cbHandler.Wait());

    pthread_join(t, NULL);

    AwaServerDeleteOperation_Free(&deleteOperation);
}


} // namespace Awa

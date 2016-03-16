#include <pthread.h>
#include <gtest/gtest.h>
#include "awa/static.h"
#include "awa/server.h"
#include "../tests/support/support.h"
#include "../tests/support/daemon.h"
#include "../../core/src/common/lwm2m_debug.h"

namespace StaticClient {
int staticClientProcessBootstrapTimeout = 10;

}

namespace Awa {


struct SingleStaticClientPollCondition : public PollCondition
{
    AwaStaticClient * StaticClient;
    AwaServerListClientsOperation * Operation;
    std::string ClientEndpointName;

    SingleStaticClientPollCondition(AwaStaticClient * StaticClient, AwaServerListClientsOperation * Operation, std::string ClientEndpointName, int maxCount) :
        PollCondition(maxCount), StaticClient(StaticClient), Operation(Operation), ClientEndpointName(ClientEndpointName) {}
    virtual ~SingleStaticClientPollCondition() {}

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
        client_ = AwaStaticClient_New();
        EXPECT_TRUE(client_ != NULL);

        AwaFactoryBootstrapInfo bootstrapinfo = { 0 };

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

    virtual AwaLwm2mResult handler(AwaStaticClient * context, AwaOperation operation, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, void ** dataPointer, uint16_t * dataSize, bool * changed) {
        return AwaLwm2mResult_InternalError;
    };
};

AwaLwm2mResult handler(AwaStaticClient * context, AwaOperation operation, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, void ** dataPointer, uint16_t * dataSize, bool * changed)
{
    AwaStaticClient * client = (AwaStaticClient *)context;
    AwaLwm2mResult result = AwaLwm2mResult_InternalError;

    std::cerr << "Handler for " << std::to_string(operation) << std::endl;

    void * callback = AwaStaticClient_GetApplicationContext(client);

    if (callback)
    {
        auto * callbackClass = static_cast<StaticClientCallbackPollCondition*>(callback);
        result = callbackClass->handler(context, operation, objectID, objectInstanceID, resourceID, resourceInstanceID, dataPointer, dataSize, changed);
    }

    std::cerr << "Handler result " << std::to_string(result) << std::endl;

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


TEST_F(TestStaticClient, AwaStaticClient_SetLogLevel_valid_input)
{
    DebugLevel oldLogLevel = Lwm2m_GetLogLevel();
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetLogLevel(AwaLogLevel_None));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetLogLevel(AwaLogLevel_Debug));
    Lwm2m_SetLogLevel(oldLogLevel);
}

TEST_F(TestStaticClient, AwaStaticClient_SetLogLevel_invalid_input)
{
    DebugLevel oldLogLevel = Lwm2m_GetLogLevel();
    EXPECT_EQ(AwaError_LogLevelInvalid, AwaStaticClient_SetLogLevel(static_cast<AwaLogLevel>(AwaLogLevel_None - 1)));
    EXPECT_EQ(AwaError_LogLevelInvalid, AwaStaticClient_SetLogLevel(static_cast<AwaLogLevel>(AwaLogLevel_Debug + 1)));
    Lwm2m_SetLogLevel(oldLogLevel);
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

    SingleStaticClientPollCondition condition(client, operation, clientEndpointName, 10);
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

    SingleStaticClientPollCondition condition(client_, operation, global::clientEndpointName, 20);
    ASSERT_TRUE(condition.Wait());

    AwaServerListClientsOperation_Free(&operation);
}

void * do_write_operation(void * attr)
{
    AwaServerWriteOperation * writeOperation = (AwaServerWriteOperation *)attr;
    AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout);
    return 0;
}

TEST_F(TestStaticClientWithServer, AwaStaticClient_Create_and_Write_Operation_for_Object_and_Resource)
{
    struct callback1 : public StaticClientCallbackPollCondition
    {
        AwaInteger integer = 5;

        callback1(AwaStaticClient * StaticClient, int maxCount) : StaticClientCallbackPollCondition(StaticClient, maxCount) {};

        AwaLwm2mResult handler(AwaStaticClient * context, AwaOperation operation, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, void ** dataPointer, uint16_t * dataSize, bool * changed)
        {

            AwaLwm2mResult result = AwaLwm2mResult_InternalError;
            EXPECT_TRUE((operation == AwaOperation_CreateResource) || (operation == AwaOperation_CreateObjectInstance) || (operation == AwaOperation_Write) || (operation == AwaOperation_Read));

            switch (operation)
            {
                case AwaOperation_CreateObjectInstance:
                {
                    EXPECT_EQ(9999, objectID);
                    EXPECT_EQ(0, objectInstanceID);
                    result = AwaLwm2mResult_SuccessCreated;
                    break;
                }
                case AwaOperation_CreateResource:
                {
                    EXPECT_EQ(9999, objectID);
                    EXPECT_EQ(0, objectInstanceID);
                    EXPECT_EQ(1, resourceID);
                    result = AwaLwm2mResult_SuccessCreated;
                    break;
                }
                case AwaOperation_Write:
                {
                    EXPECT_TRUE(dataPointer != NULL);
                    AwaInteger * integerPtr = (AwaInteger *)(*dataPointer);
                    EXPECT_TRUE(dataSize != NULL);
                    EXPECT_EQ(static_cast<int>(sizeof(AwaInteger)), *dataSize);
                    EXPECT_EQ(5, *integerPtr);
                    complete = true;
                    result = AwaLwm2mResult_SuccessChanged;
                    break;
                }
                case AwaOperation_Read:
                {
                    EXPECT_TRUE(dataPointer != NULL);
                    EXPECT_TRUE(dataSize != NULL);
                    *dataPointer = &integer;
                    *dataSize = sizeof(integer);
                    result = AwaLwm2mResult_SuccessContent;
                    break;
                }
                default:
                    result = AwaLwm2mResult_InternalError;
                    break;
            }

            return result;
        }
    };

    callback1 cbHandler(client_, 20);
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetApplicationContext(client_, &cbHandler));
    EXPECT_EQ(AwaError_Success,AwaStaticClient_RegisterObjectWithHandler(client_, "TestObject", 9999, 0, 1, handler));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_RegisterResourceWithHandler(client_, "TestResource", 9999, 1, AwaStaticResourceType_Integer, 1, 1, AwaAccess_ReadWrite, handler));

    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_TRUE(NULL != operation);
    SingleStaticClientPollCondition condition(client_, operation, global::clientEndpointName, 10);
    EXPECT_TRUE(condition.Wait());
    AwaServerListClientsOperation_Free(&operation);

    AwaServerDefineOperation * defineOpertaion = AwaServerDefineOperation_New(session_);
    EXPECT_TRUE(defineOpertaion != NULL);
    AwaObjectDefinition * objectDefintion = AwaObjectDefinition_New(9999, "TestObject", 0, 1);
    EXPECT_TRUE(objectDefintion != NULL);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefintion, 1, "TestResource", true, AwaResourceOperations_ReadWrite, 0));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(defineOpertaion, objectDefintion));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(defineOpertaion, defaults::timeout));
    AwaServerDefineOperation_Free(&defineOpertaion);
    AwaObjectDefinition_Free(&objectDefintion);

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

        AwaLwm2mResult handler(AwaStaticClient * context, AwaOperation operation, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, void ** dataPointer, uint16_t * dataSize, bool * changed)
        {
            AwaLwm2mResult result = AwaLwm2mResult_InternalError;
            EXPECT_TRUE((operation == AwaOperation_CreateResource) || (operation == AwaOperation_CreateObjectInstance) || (operation == AwaOperation_Read));

            switch(operation)
            {
                case AwaOperation_CreateObjectInstance:
                {
                    EXPECT_EQ(9999, objectID);
                    EXPECT_EQ(0, objectInstanceID);
                    result = AwaLwm2mResult_SuccessCreated;
                    break;
                }
                case AwaOperation_CreateResource:
                {
                    EXPECT_EQ(9999, objectID);
                    EXPECT_EQ(0, objectInstanceID);
                    EXPECT_EQ(1, resourceID);
                    result = AwaLwm2mResult_SuccessCreated;
                    break;
                }
                case AwaOperation_Read:
                {
                    EXPECT_TRUE(dataPointer != NULL);
                    EXPECT_TRUE(dataSize != NULL);
                    *dataPointer = &integer;
                    *dataSize = sizeof(integer);
                    complete = counter++ == 1 ? true : false;
                    result = AwaLwm2mResult_SuccessContent;
                    break;
                }
                default:
                    result = AwaLwm2mResult_InternalError;
                    break;
            }

            return result;
        }
    };

    callback1 cbHandler(client_, 20);
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetApplicationContext(client_, &cbHandler));
    EXPECT_EQ(AwaError_Success,AwaStaticClient_RegisterObjectWithHandler(client_, "TestObject", 9999, 0, 1, handler));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_RegisterResourceWithHandler(client_, "TestResource", 9999, 1, AwaStaticResourceType_Integer, 1, 1, AwaAccess_ReadWrite, handler));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_CreateObjectInstance(client_, 9999, 0));

    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_TRUE(NULL != operation);
    SingleStaticClientPollCondition condition(client_, operation, global::clientEndpointName, 10);
    EXPECT_TRUE(condition.Wait());
    AwaServerListClientsOperation_Free(&operation);

    AwaServerDefineOperation * defineOpertaion = AwaServerDefineOperation_New(session_);
    EXPECT_TRUE(defineOpertaion != NULL);
    AwaObjectDefinition * objectDefintion = AwaObjectDefinition_New(9999, "TestObject", 0, 1);
    EXPECT_TRUE(objectDefintion != NULL);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefintion, 1, "TestResource", true, AwaResourceOperations_ReadWrite, 0));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(defineOpertaion, objectDefintion));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(defineOpertaion, defaults::timeout));
    AwaServerDefineOperation_Free(&defineOpertaion);
    AwaObjectDefinition_Free(&objectDefintion);

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
    AwaServerDeleteOperation_Perform(deleteOperation, defaults::timeout);
    return 0;
}

TEST_F(TestStaticClientWithServer, AwaStaticClient_WithPointer_Create_and_Write_Operation_for_Object_and_Resource)
{
    AwaInteger i = 10;

    EXPECT_EQ(AwaError_Success,AwaStaticClient_RegisterObject(client_, "TestObject", 7999, 0, 1));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_RegisterResourceWithPointer(client_, "TestResource", 7999, 1, AwaStaticResourceType_Integer, 1, 1, AwaAccess_ReadWrite,
                                                                            &i, sizeof(i), 0));

    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_TRUE(NULL != operation);
    SingleStaticClientPollCondition condition(client_, operation, global::clientEndpointName, 10);
    EXPECT_TRUE(condition.Wait());
    AwaServerListClientsOperation_Free(&operation);

    AwaServerDefineOperation * defineOpertaion = AwaServerDefineOperation_New(session_);
    EXPECT_TRUE(defineOpertaion != NULL);
    AwaObjectDefinition * objectDefintion = AwaObjectDefinition_New(7999, "TestObject", 0, 1);
    EXPECT_TRUE(objectDefintion != NULL);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefintion, 1, "TestResource", true, AwaResourceOperations_ReadWrite, 0));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(defineOpertaion, objectDefintion));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(defineOpertaion, defaults::timeout));
    AwaServerDefineOperation_Free(&defineOpertaion);
    AwaObjectDefinition_Free(&objectDefintion);

    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
    EXPECT_TRUE(writeOperation != NULL);
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_CreateObjectInstance(writeOperation, "/7999/0"));
    EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation, "/7999/0/1", 5));

    AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout);

    AwaServerWriteOperation_Free(&writeOperation);

    AwaStaticClient_Process(client_);

    ASSERT_EQ(5, i); 
}


TEST_F(TestStaticClientWithServer, AwaStaticClient_Create_and_Delete_Operation_for_Object_and_Resource)
{
    struct callback1 : public StaticClientCallbackPollCondition
    {
        AwaInteger integer = 5;
        callback1(AwaStaticClient * StaticClient, int maxCount) : StaticClientCallbackPollCondition(StaticClient, maxCount) {};

        AwaLwm2mResult handler(AwaStaticClient * context, AwaOperation operation, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, void ** dataPointer, uint16_t * dataSize, bool * changed)

        {
            AwaLwm2mResult result = AwaLwm2mResult_InternalError;
            EXPECT_TRUE((operation == AwaOperation_CreateResource) || (operation == AwaOperation_CreateObjectInstance) || (operation == AwaOperation_DeleteObjectInstance) || (operation == AwaOperation_Read));

            switch(operation)
            {

                case AwaOperation_CreateObjectInstance:
                case AwaOperation_DeleteObjectInstance:
                {
                    EXPECT_EQ(9999, objectID);
                    EXPECT_EQ(0, objectInstanceID);
                    result = operation == AwaOperation_CreateObjectInstance ? AwaLwm2mResult_SuccessCreated : AwaLwm2mResult_SuccessDeleted;

                    if (operation == AwaOperation_DeleteObjectInstance)
                        complete = true;

                    break;
                }
                case AwaOperation_CreateResource:
                {
                    EXPECT_EQ(9999, objectID);
                    EXPECT_EQ(0, objectInstanceID);
                    EXPECT_EQ(1, resourceID);
                    result = AwaLwm2mResult_SuccessCreated;
                    break;
                }
                case AwaOperation_Read:
                {
                    EXPECT_TRUE(dataPointer != NULL);
                    EXPECT_TRUE(dataSize != NULL);
                    *dataPointer = &integer;
                    *dataSize = sizeof(integer);
                    result = AwaLwm2mResult_SuccessContent;
                    break;
                }
                default:
                    result = AwaLwm2mResult_InternalError;
                    break;
            }

            return result;
        }
    };

    callback1 cbHandler(client_, 20);
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetApplicationContext(client_, &cbHandler));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_RegisterObjectWithHandler(client_, "TestObject", 9999, 0, 1, handler));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_RegisterResourceWithHandler(client_, "TestResource", 9999, 1, AwaStaticResourceType_Integer, 0, 1, AwaAccess_ReadWrite, handler));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_CreateObjectInstance(client_, 9999, 0));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_CreateResource(client_, 9999, 0, 1));

    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_TRUE(NULL != operation);
    SingleStaticClientPollCondition condition(client_, operation, global::clientEndpointName, 10);
    EXPECT_TRUE(condition.Wait());
    AwaServerListClientsOperation_Free(&operation);

    AwaServerDefineOperation * defineOpertaion = AwaServerDefineOperation_New(session_);
    EXPECT_TRUE(defineOpertaion != NULL);
    AwaObjectDefinition * objectDefintion = AwaObjectDefinition_New(9999, "TestObject", 0, 1);
    EXPECT_TRUE(objectDefintion != NULL);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefintion, 1, "TestResource", false, AwaResourceOperations_ReadWrite, 0));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(defineOpertaion, objectDefintion));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(defineOpertaion, defaults::timeout));
    AwaServerDefineOperation_Free(&defineOpertaion);
    AwaObjectDefinition_Free(&objectDefintion);

    AwaServerDeleteOperation * deleteOperation = AwaServerDeleteOperation_New(session_);
    EXPECT_TRUE(deleteOperation != NULL);
    EXPECT_EQ(AwaError_Success, AwaServerDeleteOperation_AddPath(deleteOperation, global::clientEndpointName, "/9999/0"));

    pthread_t t;
    pthread_create(&t, NULL, do_delete_operation, (void *)deleteOperation);
    ASSERT_TRUE(cbHandler.Wait());
    pthread_join(t, NULL);

    AwaServerDeleteOperation_Free(&deleteOperation);
}

void * do_execute_operation(void * attr)
{
    AwaServerExecuteOperation * executeOperation = (AwaServerExecuteOperation *)attr;
    AwaServerExecuteOperation_Perform(executeOperation, defaults::timeout);
    return 0;
}

TEST_F(TestStaticClientWithServer, AwaStaticClient_Create_and_Execute_Operation_for_Object_and_Resource)
{
    struct callback1 : public StaticClientCallbackPollCondition
    {
        callback1(AwaStaticClient * StaticClient, int maxCount) : StaticClientCallbackPollCondition(StaticClient, maxCount) {};

        AwaLwm2mResult handler(AwaStaticClient * context, AwaOperation operation, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, void ** dataPointer, uint16_t * dataSize, bool * changed)
        {
            AwaLwm2mResult result = AwaLwm2mResult_InternalError;

            EXPECT_TRUE((operation == AwaOperation_CreateResource) || (operation == AwaOperation_CreateObjectInstance) || (operation == AwaOperation_Execute));

            switch(operation)
            {
                case AwaOperation_CreateObjectInstance:
                case AwaOperation_DeleteObjectInstance:
                {
                    EXPECT_EQ(9999, objectID);
                    EXPECT_EQ(0, objectInstanceID);
                    result = operation == AwaOperation_CreateObjectInstance ? AwaLwm2mResult_SuccessCreated : AwaLwm2mResult_SuccessDeleted;
                    break;
                }
                case AwaOperation_CreateResource:
                {
                    EXPECT_EQ(9999, objectID);
                    EXPECT_EQ(0, objectInstanceID);
                    EXPECT_EQ(1, resourceID);
                    result = AwaLwm2mResult_SuccessCreated;
                    break;
                }
                case AwaOperation_Execute:
                {
                    EXPECT_EQ(9999, objectID);
                    EXPECT_EQ(0, objectInstanceID);
                    EXPECT_EQ(1, resourceID);
                    result = AwaLwm2mResult_Success;
                    complete = true;
                    break;
                }
                default:
                    result = AwaLwm2mResult_InternalError;
                    break;
            }

            return result;
        }
    };

    callback1 cbHandler(client_, 20);
    EXPECT_EQ(AwaError_Success, AwaStaticClient_SetApplicationContext(client_, &cbHandler));
    EXPECT_EQ(AwaError_Success,AwaStaticClient_RegisterObjectWithHandler(client_, "TestObject", 9999, 0, 1, handler));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_RegisterResourceWithHandler(client_, "TestResource", 9999, 1, AwaStaticResourceType_None, 1, 1, AwaAccess_Execute, handler));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_CreateObjectInstance(client_, 9999, 0));
    EXPECT_EQ(AwaError_Success, AwaStaticClient_CreateResource(client_, 9999, 0, 1));

    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
    EXPECT_TRUE(NULL != operation);
    SingleStaticClientPollCondition condition(client_, operation, global::clientEndpointName, 10);
    EXPECT_TRUE(condition.Wait());
    AwaServerListClientsOperation_Free(&operation);

    AwaServerDefineOperation * defineOpertaion = AwaServerDefineOperation_New(session_);
    EXPECT_TRUE(defineOpertaion != NULL);
    AwaObjectDefinition * objectDefintion = AwaObjectDefinition_New(9999, "TestObject", 0, 1);
    EXPECT_TRUE(objectDefintion != NULL);
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsNoType(objectDefintion, 1, "TestExecutableResource", true, AwaResourceOperations_Execute));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(defineOpertaion, objectDefintion));
    EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(defineOpertaion, defaults::timeout));
    AwaServerDefineOperation_Free(&defineOpertaion);
    AwaObjectDefinition_Free(&objectDefintion);

    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(session_);
    EXPECT_TRUE(executeOperation != NULL);
    EXPECT_EQ(AwaError_Success, AwaServerExecuteOperation_AddPath(executeOperation, global::clientEndpointName, "/9999/0/1", NULL));

    pthread_t t;
    pthread_create(&t, NULL, do_execute_operation, (void *)executeOperation);
    ASSERT_TRUE(cbHandler.Wait());
    pthread_join(t, NULL);

    AwaServerExecuteOperation_Free(&executeOperation);
}

namespace writeDetail
{

struct TestWriteResource
{
    AwaError expectedAddResult;
    AwaError expectedProcessResult;

    AwaObjectID objectID;
    AwaObjectInstanceID objectInstanceID;
    AwaResourceID resourceID;

    const void * value;
    const size_t valueCount;
    AwaStaticResourceType type;

    bool useOperation;
};

::std::ostream& operator<<(::std::ostream& os, const TestWriteResource& item)
{
  return os << "Item: expectedAddResult " << item.expectedAddResult
            << ", expectedProcessResult " << item.expectedProcessResult
            << ", objectID " << item.objectID
            << ", objectInstanceID " << item.objectInstanceID
            << ", resourceID " << item.resourceID
            << ", value " << item.value
            << ", valueCount " << item.valueCount
            << ", type " << item.type
            << ", useOperation " << item.useOperation;
}

const char * clientID = "TestClient1";

static AwaInteger dummyInteger1 = 123456;
static const char * dummyString1 = "Lightweight M2M Server";
static AwaFloat dummyFloat1 = 1.0;
static AwaTime dummyTime1 = 0xA20AD72B;
static AwaBoolean dummyBoolean1 = true;
static char dummyOpaqueData[] = {'a',0,'x','\0', 123};
//static int dummyObjLinkData[] = {-1,-1};

AwaObjectLink dummyObjectLink1 = { 3, 5 };

const char * dummyStringArray1[] = {"Lightweight M2M Server", "test1", ""};
const char * dummyStringArray2[] = {"Lightweight M2M Server", "test1", "", "", "", ""};
const AwaInteger dummyIntegerArray1[] = {55, 8732, 11};
const AwaInteger dummyIntegerArray2[] = {55, 8732, 11, 55, 8732, 11};
const AwaFloat dummyFloatArray1[] = {55.0, 0.0008732, 11e10};
const AwaFloat dummyFloatArray2[] = {55.0, 0.0008732, 11e10, 55.0, 0.0008732, 11e10};
const AwaBoolean dummyBooleanArray1[] = {true, false, true};
const AwaBoolean dummyBooleanArray2[] = {true, false, true, true, false, true};

const AwaInteger dummyTimeArray1[] = {16000, 8732222, 1111};
const AwaInteger dummyTimeArray2[] = {16000, 8732222, 1111, 16000, 8732222, 1111};

const AwaObjectID TEST_OBJECT_NON_ARRAY_TYPES = 10000;
const AwaResourceID TEST_RESOURCE_STRING = 1;
const AwaResourceID TEST_RESOURCE_INTEGER = 2;
const AwaResourceID TEST_RESOURCE_FLOAT = 3;
const AwaResourceID TEST_RESOURCE_BOOLEAN = 4;
const AwaResourceID TEST_RESOURCE_OPAQUE = 5;
const AwaResourceID TEST_RESOURCE_TIME = 6;
const AwaResourceID TEST_RESOURCE_OBJECTLINK = 7;

const AwaObjectID TEST_OBJECT_ARRAY_TYPES = 10001;
const AwaResourceID TEST_RESOURCE_STRINGARRAY = 1;
const AwaResourceID TEST_RESOURCE_INTEGERARRAY = 2;
const AwaResourceID TEST_RESOURCE_FLOATARRAY = 3;
const AwaResourceID TEST_RESOURCE_BOOLEANARRAY = 4;
const AwaResourceID TEST_RESOURCE_OPAQUEARRAY = 5;
const AwaResourceID TEST_RESOURCE_TIMEARRAY = 6;
const AwaResourceID TEST_RESOURCE_OBJECTLINKARRAY = 7;

}

typedef AwaLwm2mResult (*TestHandler)(void * context, AwaOperation operation, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, void ** dataPointer, uint16_t * dataSize, bool * changed);

struct TestWriteReadStaticResource
{
    TestHandler write_handler;
    TestHandler read_handler;

    AwaObjectID objectID;
    AwaObjectInstanceID objectInstanceID;
    AwaResourceID resourceID;

    const void * value;
    const int valueCount;
    const int valueSize;
    AwaStaticResourceType type;

    bool complete;
    bool testRead;
};

class TestWriteReadValueStaticClient : public TestStaticClientWithServer, public ::testing::WithParamInterface< TestWriteReadStaticResource >
{

protected:

    class callback1 : public StaticClientCallbackPollCondition
    {
    public:

        TestWriteReadStaticResource data;

        callback1(AwaStaticClient * StaticClient, int maxCount, TestWriteReadStaticResource data) : StaticClientCallbackPollCondition(StaticClient, maxCount), data(data) {};

        AwaLwm2mResult handler(AwaStaticClient * context, AwaOperation operation, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, void ** dataPointer, uint16_t * dataSize, bool * changed)
        {
            AwaLwm2mResult result = AwaLwm2mResult_InternalError;

            switch (operation)
            {
                    case AwaOperation_CreateObjectInstance:
                    {
                        EXPECT_EQ(data.objectID, objectID);
                        EXPECT_EQ(data.objectInstanceID, objectInstanceID);
                        result = AwaLwm2mResult_SuccessCreated;
                        break;
                    }
                    case AwaOperation_CreateResource:
                    {
                        EXPECT_EQ(data.objectID, objectID);
                        EXPECT_EQ(data.objectInstanceID, objectInstanceID);
                        EXPECT_EQ(data.resourceID, resourceID);
                        result = AwaLwm2mResult_SuccessCreated;
                        break;
                    }
                    case AwaOperation_Write:
                        result = data.write_handler(&data, operation, objectID, objectInstanceID, resourceID, resourceInstanceID, dataPointer, dataSize, changed);
                        break;
                    case AwaOperation_Read:
                        result = data.read_handler(&data, operation, objectID, objectInstanceID, resourceID, resourceInstanceID, dataPointer, dataSize, changed);
                        break;
                    default:
                        break;
            }


            complete = data.complete;
            return result;
        }
    };

    callback1 * cbHandler;


    void SetUp() {
        TestStaticClientWithServer::SetUp();
        TestWriteReadStaticResource data = GetParam();

        cbHandler = new callback1(client_, 20, data);

        EXPECT_EQ(AwaError_Success, AwaStaticClient_SetApplicationContext(client_, cbHandler));
        EXPECT_EQ(AwaError_Success,AwaStaticClient_RegisterObjectWithHandler(client_, "Test Object Single", writeDetail::TEST_OBJECT_NON_ARRAY_TYPES, 0, 1, handler));
        EXPECT_EQ(AwaError_Success, AwaStaticClient_RegisterResourceWithHandler(client_, "Test String Resource",      writeDetail::TEST_OBJECT_NON_ARRAY_TYPES, writeDetail::TEST_RESOURCE_STRING,     AwaStaticResourceType_String,     0, 1, AwaAccess_ReadWrite, handler));
        EXPECT_EQ(AwaError_Success, AwaStaticClient_RegisterResourceWithHandler(client_, "Test Integer Resource",     writeDetail::TEST_OBJECT_NON_ARRAY_TYPES, writeDetail::TEST_RESOURCE_INTEGER,    AwaStaticResourceType_Integer,    0, 1, AwaAccess_ReadWrite, handler));
        EXPECT_EQ(AwaError_Success, AwaStaticClient_RegisterResourceWithHandler(client_, "Test Float Resource",       writeDetail::TEST_OBJECT_NON_ARRAY_TYPES, writeDetail::TEST_RESOURCE_FLOAT,      AwaStaticResourceType_Float,      0, 1, AwaAccess_ReadWrite, handler));
        EXPECT_EQ(AwaError_Success, AwaStaticClient_RegisterResourceWithHandler(client_, "Test Boolean Resource",     writeDetail::TEST_OBJECT_NON_ARRAY_TYPES, writeDetail::TEST_RESOURCE_BOOLEAN,    AwaStaticResourceType_Boolean,    0, 1, AwaAccess_ReadWrite, handler));
        EXPECT_EQ(AwaError_Success, AwaStaticClient_RegisterResourceWithHandler(client_, "Test Opaque Resource",      writeDetail::TEST_OBJECT_NON_ARRAY_TYPES, writeDetail::TEST_RESOURCE_OPAQUE,     AwaStaticResourceType_Opaque,     0, 1, AwaAccess_ReadWrite, handler));
        EXPECT_EQ(AwaError_Success, AwaStaticClient_RegisterResourceWithHandler(client_, "Test Time Resource",        writeDetail::TEST_OBJECT_NON_ARRAY_TYPES, writeDetail::TEST_RESOURCE_TIME,       AwaStaticResourceType_Time,       0, 1, AwaAccess_ReadWrite, handler));
        EXPECT_EQ(AwaError_Success, AwaStaticClient_RegisterResourceWithHandler(client_, "Test Object Link Resource", writeDetail::TEST_OBJECT_NON_ARRAY_TYPES, writeDetail::TEST_RESOURCE_OBJECTLINK, AwaStaticResourceType_ObjectLink, 0, 1, AwaAccess_ReadWrite, handler));
        EXPECT_EQ(AwaError_Success, AwaStaticClient_CreateObjectInstance(client_, writeDetail::TEST_OBJECT_NON_ARRAY_TYPES, 0));

        AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session_);
        EXPECT_TRUE(NULL != operation);
        SingleStaticClientPollCondition condition(client_, operation, global::clientEndpointName, 10);
        EXPECT_TRUE(condition.Wait());
        AwaServerListClientsOperation_Free(&operation);

        AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(session_);
        EXPECT_TRUE(serverDefineOperation != NULL);

        AwaObjectDefinition * customObjectDefinition = AwaObjectDefinition_New(writeDetail::TEST_OBJECT_NON_ARRAY_TYPES, "Test Object Single", 0, 1);
        EXPECT_TRUE(NULL != customObjectDefinition);

        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsString     (customObjectDefinition, writeDetail::TEST_RESOURCE_STRING,     "Test String Resource",      false, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger    (customObjectDefinition, writeDetail::TEST_RESOURCE_INTEGER,    "Test Integer Resource",     false, AwaResourceOperations_ReadWrite, 0));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloat      (customObjectDefinition, writeDetail::TEST_RESOURCE_FLOAT,      "Test Float Resource",       false, AwaResourceOperations_ReadWrite, 0.0));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBoolean    (customObjectDefinition, writeDetail::TEST_RESOURCE_BOOLEAN,    "Test Boolean Resource",     false, AwaResourceOperations_ReadWrite, false));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque     (customObjectDefinition, writeDetail::TEST_RESOURCE_OPAQUE,     "Test Opaque Resource",      false, AwaResourceOperations_ReadWrite, AwaOpaque {0}));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTime       (customObjectDefinition, writeDetail::TEST_RESOURCE_TIME,       "Test Time Resource",        false, AwaResourceOperations_ReadWrite, 0));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLink (customObjectDefinition, writeDetail::TEST_RESOURCE_OBJECTLINK, "Test Object Link Resource", false, AwaResourceOperations_ReadWrite, AwaObjectLink {0}));

        EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
        AwaObjectDefinition_Free(&customObjectDefinition);

        customObjectDefinition = AwaObjectDefinition_New(writeDetail::TEST_OBJECT_ARRAY_TYPES, "Test Object Array", 0, 1);
        EXPECT_TRUE(NULL != customObjectDefinition);

        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsStringArray    (customObjectDefinition, writeDetail::TEST_RESOURCE_STRING,     "Test String Array Resource",      0,5, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsIntegerArray   (customObjectDefinition, writeDetail::TEST_RESOURCE_INTEGER,    "Test Integer Array Resource",     0,5, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloatArray     (customObjectDefinition, writeDetail::TEST_RESOURCE_FLOAT,      "Test Float Array Resource",       0,5, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBooleanArray   (customObjectDefinition, writeDetail::TEST_RESOURCE_BOOLEAN,    "Test Boolean Array Resource",     0,5, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaqueArray    (customObjectDefinition, writeDetail::TEST_RESOURCE_OPAQUE,     "Test Opaque Array Resource",      0,5, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTimeArray      (customObjectDefinition, writeDetail::TEST_RESOURCE_TIME,       "Test Time Array Resource",        0,5, AwaResourceOperations_ReadWrite, NULL));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLinkArray(customObjectDefinition, writeDetail::TEST_RESOURCE_OBJECTLINK, "Test Object Link Array Resource", 0,5, AwaResourceOperations_ReadWrite, NULL));

        EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, customObjectDefinition));
        EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, defaults::timeout));

        AwaObjectDefinition_Free(&customObjectDefinition);
        AwaServerDefineOperation_Free(&serverDefineOperation);

        writeOperation_ = AwaServerWriteOperation_New(session_, AwaWriteMode_Update);
        EXPECT_TRUE(NULL != writeOperation_);

        readOperation_ = AwaServerReadOperation_New(session_);
        EXPECT_TRUE(NULL != readOperation_);
    }

    void TearDown() {
        AwaServerWriteOperation_Free(&writeOperation_);
        AwaServerReadOperation_Free(&readOperation_);
        delete cbHandler;
        TestStaticClientWithServer::TearDown();
    }

    AwaClientSession * clientSession_;
    AwaServerSession * serverSession_;
    AwaClientDaemonHorde * horde_;
    AwaServerWriteOperation * writeOperation_;
    AwaServerReadOperation * readOperation_;
};

AwaLwm2mResult TestWriteValueStaticClient_WriteHandler(void * context, AwaOperation operation, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, void ** dataPointer, uint16_t * dataSize, bool * changed)
{
    TestWriteReadStaticResource * data = static_cast<TestWriteReadStaticResource *>(context);
    EXPECT_EQ(AwaOperation_Write, operation);
    EXPECT_EQ(data->objectID, objectID);
    EXPECT_EQ(data->objectInstanceID, objectInstanceID);
    EXPECT_EQ(data->resourceID, resourceID);
    EXPECT_EQ(0, resourceInstanceID);

    EXPECT_TRUE(dataSize != NULL);
    EXPECT_TRUE(dataPointer != NULL);

    EXPECT_EQ(data->valueSize, *dataSize);
    EXPECT_EQ(0, memcmp(data->value, *dataPointer, data->valueSize));

    data->complete = true;
    return AwaLwm2mResult_SuccessChanged;
}


AwaLwm2mResult TestWriteValueStaticClient_ReadHandler(void * context, AwaOperation operation, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, void ** dataPointer, uint16_t * dataSize, bool * changed)
{
    TestWriteReadStaticResource * data = static_cast<TestWriteReadStaticResource *>(context);

    EXPECT_EQ(AwaOperation_Read, operation);
    EXPECT_EQ(data->objectID, objectID);
    EXPECT_EQ(data->objectInstanceID, objectInstanceID);
    EXPECT_EQ(data->resourceID, resourceID);
    EXPECT_EQ(0, resourceInstanceID);

    EXPECT_TRUE(dataSize != NULL);
    EXPECT_TRUE(dataPointer != NULL);

    *dataPointer = (void*)data->value;
    *dataSize = data->valueSize;

    if(data->testRead == true)
    {
        data->complete = true;
    }

    return AwaLwm2mResult_SuccessContent;
}

TEST_P(TestWriteReadValueStaticClient, TestWriteReadValueSingle)
{
    TestWriteReadStaticResource data = GetParam();
    char path[128] = {0};

    EXPECT_EQ(AwaError_Success, AwaAPI_MakePath(path, sizeof(path), data.objectID, data.objectInstanceID, data.resourceID));

    switch (data.type)
    {
        case AwaStaticResourceType_String:
            EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsCString(writeOperation_, path, (const char *)data.value));
            break;
        case AwaStaticResourceType_Integer:
            EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsInteger(writeOperation_, path, *((AwaInteger*)data.value)));
            break;
        case AwaStaticResourceType_Float:
            EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsFloat(writeOperation_, path, *((AwaFloat*)data.value)));
            break;
        case AwaStaticResourceType_Boolean:
            EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsBoolean(writeOperation_, path, *((AwaBoolean*)data.value)));
            break;
        case AwaStaticResourceType_Opaque:
            {
                AwaOpaque opaque = { (void *)data.value, static_cast<size_t>(data.valueSize) };
                EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsOpaque(writeOperation_, path, opaque));
                break;
            }
        case AwaStaticResourceType_Time:
            EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsTime(writeOperation_, path, *((AwaTime*)data.value)));
            break;
        case AwaStaticResourceType_ObjectLink:
            {
                AwaObjectLink * objectlink = (AwaObjectLink *)data.value;
                EXPECT_EQ(AwaError_Success, AwaServerWriteOperation_AddValueAsObjectLink(writeOperation_, path, *objectlink));
                break;
            }
        default:
            EXPECT_TRUE(false);
            break;
    }

    pthread_t writeThread;
    pthread_create(&writeThread, NULL, do_write_operation, (void *)writeOperation_);
    ASSERT_TRUE(cbHandler->Wait());
    pthread_join(writeThread, NULL);

    cbHandler->data.complete = false;
    cbHandler->data.testRead = true;
    cbHandler->Reset();

    AwaServerReadOperation_AddPath(readOperation_, global::clientEndpointName, path);

    pthread_t readThread;
    pthread_create(&readThread, NULL, do_read_operation, (void *)readOperation_);
    ASSERT_TRUE(cbHandler->Wait());
    pthread_join(readThread, NULL);

    const AwaServerReadResponse * readResponse = AwaServerReadOperation_GetResponse(readOperation_, global::clientEndpointName );
    EXPECT_TRUE(readResponse != NULL);

    switch (data.type)
    {
        case AwaStaticResourceType_String:
            {
                const char * value;
                EXPECT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsCStringPointer(readResponse, path, (const char **)&value));
                ASSERT_STREQ((char*) data.value, (char*) value);
                break;
            }
        case AwaStaticResourceType_Integer:
            {
                AwaInteger * value;
                EXPECT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsIntegerPointer(readResponse, path, (const AwaInteger **)&value));
                ASSERT_EQ(*static_cast<const AwaInteger *>(data.value), *static_cast<AwaInteger *>(value));
                break;
            }
        case AwaStaticResourceType_Float:
            {
                AwaFloat * value;
                EXPECT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsFloatPointer(readResponse, path, (const AwaFloat **)&value));
                ASSERT_EQ(*static_cast<const AwaFloat *>(data.value), *static_cast<AwaFloat *>(value));
                break;
            }
        case AwaStaticResourceType_Boolean:
            {
                AwaBoolean * value;
                EXPECT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsBooleanPointer(readResponse, path, (const AwaBoolean **)&value));
                ASSERT_EQ(*static_cast<const AwaBoolean *>(data.value), *static_cast<AwaBoolean *>(value));
                break;
            }
        case AwaStaticResourceType_Opaque:
            {
                AwaOpaque * value;
                EXPECT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsOpaquePointer(readResponse, path, (const AwaOpaque **)&value));
                ASSERT_EQ(data.valueSize, static_cast<int>(value->Size));
                ASSERT_TRUE(memcmp(value->Data, data.value, data.valueSize) == 0);
                break;
            }
        case AwaStaticResourceType_Time:
            {
                AwaTime * value;
                EXPECT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsTimePointer(readResponse, path, (const AwaTime **)&value));
                ASSERT_EQ(*static_cast<const AwaTime *>(data.value), *static_cast<AwaTime *>(value));
                break;
            }
        case AwaStaticResourceType_ObjectLink:
            {
                AwaObjectLink * expectedObjectLink = (AwaObjectLink *)data.value;
                const AwaObjectLink * receivedObjectLinkPointer;
                EXPECT_EQ(AwaError_Success, AwaServerReadResponse_GetValueAsObjectLinkPointer(readResponse, path, (const AwaObjectLink **)&receivedObjectLinkPointer));
                EXPECT_EQ(expectedObjectLink->ObjectID, receivedObjectLinkPointer->ObjectID);
                EXPECT_EQ(expectedObjectLink->ObjectInstanceID, receivedObjectLinkPointer->ObjectInstanceID);
                break;
            }
    default:
        EXPECT_TRUE(false);
        break;
    }

}

INSTANTIATE_TEST_CASE_P(
        TestWriteReadValueStaticClient,
        TestWriteReadValueStaticClient,
        ::testing::Values(
        TestWriteReadStaticResource {TestWriteValueStaticClient_WriteHandler, TestWriteValueStaticClient_ReadHandler, writeDetail::TEST_OBJECT_NON_ARRAY_TYPES, 0, writeDetail::TEST_RESOURCE_STRING,     writeDetail::dummyString1,      1, static_cast<int>(strlen(writeDetail::dummyString1)),     AwaStaticResourceType_String,     true, false},
        TestWriteReadStaticResource {TestWriteValueStaticClient_WriteHandler, TestWriteValueStaticClient_ReadHandler, writeDetail::TEST_OBJECT_NON_ARRAY_TYPES, 0, writeDetail::TEST_RESOURCE_INTEGER,    &writeDetail::dummyInteger1,    1, static_cast<int>(sizeof(writeDetail::dummyInteger1)),    AwaStaticResourceType_Integer,    true, false},
        TestWriteReadStaticResource {TestWriteValueStaticClient_WriteHandler, TestWriteValueStaticClient_ReadHandler, writeDetail::TEST_OBJECT_NON_ARRAY_TYPES, 0, writeDetail::TEST_RESOURCE_FLOAT,      &writeDetail::dummyFloat1,      1, static_cast<int>(sizeof(writeDetail::dummyFloat1)),      AwaStaticResourceType_Float,      true, false},
        TestWriteReadStaticResource {TestWriteValueStaticClient_WriteHandler, TestWriteValueStaticClient_ReadHandler, writeDetail::TEST_OBJECT_NON_ARRAY_TYPES, 0, writeDetail::TEST_RESOURCE_BOOLEAN,    &writeDetail::dummyBoolean1,    1, static_cast<int>(sizeof(writeDetail::dummyBoolean1)),    AwaStaticResourceType_Boolean,    true, false},
        TestWriteReadStaticResource {TestWriteValueStaticClient_WriteHandler, TestWriteValueStaticClient_ReadHandler, writeDetail::TEST_OBJECT_NON_ARRAY_TYPES, 0, writeDetail::TEST_RESOURCE_OPAQUE,     writeDetail::dummyOpaqueData,   1, static_cast<int>(sizeof(writeDetail::dummyOpaqueData)),  AwaStaticResourceType_Opaque,     true, false},
        TestWriteReadStaticResource {TestWriteValueStaticClient_WriteHandler, TestWriteValueStaticClient_ReadHandler, writeDetail::TEST_OBJECT_NON_ARRAY_TYPES, 0, writeDetail::TEST_RESOURCE_TIME,       &writeDetail::dummyTime1,       1, static_cast<int>(sizeof(writeDetail::dummyTime1)),       AwaStaticResourceType_Time,       true, false},
        TestWriteReadStaticResource {TestWriteValueStaticClient_WriteHandler, TestWriteValueStaticClient_ReadHandler, writeDetail::TEST_OBJECT_NON_ARRAY_TYPES, 0, writeDetail::TEST_RESOURCE_OBJECTLINK, &writeDetail::dummyObjectLink1, 1, static_cast<int>(sizeof(writeDetail::dummyObjectLink1)), AwaStaticResourceType_ObjectLink, true, false}
        ));
} // namespace Awa

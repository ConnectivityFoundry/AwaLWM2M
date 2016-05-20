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

#ifndef STATIC_API_SUPPORT_H
#define STATIC_API_SUPPORT_H

/*************************************************************
 *** Awa Static API Client & Server Test Support Functions
 *************************************************************/

#include <gtest/gtest.h>
#include <unistd.h>

#include "awa/static.h"
#include "awa/server.h"

#include "../tests/support/support.h"
#include "../tests/support/daemon.h"

#include "../../core/src/common/lwm2m_debug.h"

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

struct SingleStaticClientObjectPollCondition : public PollCondition
{
    AwaStaticClient * StaticClient;
    AwaServerListClientsOperation * Operation;
    std::string ClientEndpointName;
    std::string ObjectPath;
    bool inverse;

    SingleStaticClientObjectPollCondition(AwaStaticClient * StaticClient, AwaServerListClientsOperation * Operation, std::string ClientEndpointName, std::string ObjectPath, int maxCount, bool inverse = false) :
        PollCondition(maxCount), StaticClient(StaticClient), Operation(Operation), ClientEndpointName(ClientEndpointName), ObjectPath(ObjectPath), inverse(inverse) {}
    virtual ~SingleStaticClientObjectPollCondition() {}

    virtual bool Check()
    {
        bool found = inverse;

        EXPECT_EQ(AwaError_Success, AwaServerListClientsOperation_Perform(Operation, defaults::timeout));
        const AwaServerListClientsResponse * clientListResponse = AwaServerListClientsOperation_GetResponse(Operation, ClientEndpointName.c_str());
        EXPECT_TRUE(clientListResponse != NULL);
        AwaRegisteredEntityIterator * objectIterator = AwaServerListClientsResponse_NewRegisteredEntityIterator(clientListResponse);
        EXPECT_TRUE(objectIterator != NULL);

        while (AwaRegisteredEntityIterator_Next(objectIterator))
        {
            if (ObjectPath.compare(AwaRegisteredEntityIterator_GetPath(objectIterator)) == 0)
            {
                found = !inverse;
            }
        }

        AwaRegisteredEntityIterator_Free(&objectIterator);
        AwaStaticClient_Process(StaticClient);
        return found;
    }
};

class TestStaticClientWithServer : public TestServerWithConnectedSession
{
protected:
    virtual void SetUp() {
        TestServerWithConnectedSession::SetUp();

        AwaStaticClient_SetLogLevel(static_cast<AwaLogLevel>(global::logLevel));

        std::string serverURI = std::string("coap://127.0.0.1:") + std::to_string(global::serverCoapPort) + "/";
        client_ = AwaStaticClient_New();
        ASSERT_TRUE(client_ != NULL);

        AwaFactoryBootstrapInfo bootstrapinfo = { 0 };

        sprintf(bootstrapinfo.SecurityInfo.ServerURI, "%s", serverURI.c_str());
        bootstrapinfo.SecurityInfo.SecurityMode = AwaSecurityMode_NoSec;
        sprintf(bootstrapinfo.SecurityInfo.PublicKeyOrIdentity, "[PublicKey]");
        sprintf(bootstrapinfo.SecurityInfo.SecretKey, "[SecretKey]");

        bootstrapinfo.ServerInfo.Lifetime = 60;
        bootstrapinfo.ServerInfo.DefaultMinPeriod = 1;
        bootstrapinfo.ServerInfo.DefaultMaxPeriod = -1;
        bootstrapinfo.ServerInfo.DisableTimeout = 86400;
        bootstrapinfo.ServerInfo.Notification = false;
        sprintf(bootstrapinfo.ServerInfo.Binding, "U");

        EXPECT_EQ(AwaError_Success, AwaStaticClient_SetBootstrapServerURI(client_, ""));
        EXPECT_EQ(AwaError_Success, AwaStaticClient_SetEndPointName(client_, global::clientEndpointName));
        EXPECT_EQ(AwaError_Success, AwaStaticClient_SetCoAPListenAddressPort(client_, "0.0.0.0", global::clientLocalCoapPort));

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
protected:
    AwaStaticClient * StaticClient;

public:
    StaticClientCallbackPollCondition(AwaStaticClient * StaticClient, int maxCount) :
        PollCondition(maxCount), StaticClient(StaticClient), complete(false) {}
    bool complete;
    virtual bool Check()
    {
        std::cout << "Check..." << std::endl;
        AwaStaticClient_Process(StaticClient);
        return complete;
    }

    virtual AwaResult handler(AwaStaticClient * context, AwaOperation operation, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, void ** dataPointer, size_t * dataSize, bool * changed) {
        return AwaResult_InternalError;
    };
};

AwaResult handler(AwaStaticClient * context, AwaOperation operation, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, void ** dataPointer, size_t * dataSize, bool * changed);
void * do_write_operation(void * attr);
void * do_read_operation(void * attr);
void * do_execute_operation(void * attr);

} // namespace Awa

#endif // STATIC_API_SUPPORT_H

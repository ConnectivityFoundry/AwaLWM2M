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

struct SingleStaticClientWaitCondition : public WaitCondition
{
    AwaStaticClient * StaticClient;
    AwaServerSession * ServerSession;
    std::string ClientEndpointName;
    bool found;

    SingleStaticClientWaitCondition(AwaStaticClient * StaticClient, AwaServerSession * ServerSession, std::string ClientEndpointName, int milliseconds) :
        WaitCondition(1e4, (milliseconds * 1e3)), StaticClient(StaticClient), ServerSession(ServerSession), ClientEndpointName(ClientEndpointName), found(false)
    {
        AwaServerSession_SetClientRegisterEventCallback(ServerSession, Client_Register, this);
    }
    virtual ~SingleStaticClientWaitCondition() {}

    virtual bool Check()
    {
        found = false;

        EXPECT_EQ(AwaError_Success, AwaServerSession_Process(ServerSession, (this->checkPeriod_ / 1e3)));
        EXPECT_EQ(AwaError_Success, AwaServerSession_DispatchCallbacks(ServerSession));
        AwaStaticClient_Process(StaticClient);
        return found;
    }

    static void Client_Register(const AwaServerClientRegisterEvent * event, void * context)
    {
        if (context)
        {
            auto * that = static_cast<SingleStaticClientWaitCondition*>(context);

            AwaClientIterator * iterator = AwaServerClientRegisterEvent_NewClientIterator(event);
            EXPECT_TRUE(iterator != NULL);
            if (AwaClientIterator_Next(iterator))
            {
                if (that->ClientEndpointName.compare(AwaClientIterator_GetClientID(iterator)) == 0)
                {
                    that->found = true;
                }
            }
            AwaClientIterator_Free(&iterator);
        }
    }
};

struct SingleStaticClientObjectWaitCondition : public WaitCondition
{
    AwaStaticClient * StaticClient;
    AwaServerSession * ServerSession;
    std::string ClientEndpointName;
    std::string ObjectPath;
    bool inverse;
    bool found;

    SingleStaticClientObjectWaitCondition(AwaStaticClient * StaticClient, AwaServerSession * ServerSession, std::string ClientEndpointName, std::string ObjectPath, int milliseconds, bool inverse = false) :
        WaitCondition(1e4, (milliseconds * 1e3)), StaticClient(StaticClient), ServerSession(ServerSession), ClientEndpointName(ClientEndpointName), ObjectPath(ObjectPath), inverse(inverse), found(inverse)
    {
        AwaServerSession_SetClientRegisterEventCallback(ServerSession, Client_Register, this);
        AwaServerSession_SetClientUpdateEventCallback(ServerSession, Client_Update, this);
    }
    virtual ~SingleStaticClientObjectWaitCondition() {}

    virtual bool Check()
    {
        found = inverse;

        EXPECT_EQ(AwaError_Success, AwaServerSession_Process(ServerSession, (this->checkPeriod_ / 1e3)));
        EXPECT_EQ(AwaError_Success, AwaServerSession_DispatchCallbacks(ServerSession));

        AwaStaticClient_Process(StaticClient);
        return found;
    }



    static void Client_Register(const AwaServerClientRegisterEvent * event, void * context)
    {
        if (context)
        {
            auto * that = static_cast<SingleStaticClientObjectWaitCondition*>(context);


            AwaRegisteredEntityIterator * objectIterator = AwaServerClientRegisterEvent_NewRegisteredEntityIterator(event, that->ClientEndpointName.c_str());
            if(objectIterator)
            {

                while (AwaRegisteredEntityIterator_Next(objectIterator))
                {
                    if (that->ObjectPath.compare(AwaRegisteredEntityIterator_GetPath(objectIterator)) == 0)
                    {
                        that->found = !(that->inverse);
                    }
                }

                AwaRegisteredEntityIterator_Free(&objectIterator);
            }
        }
    }

    static void Client_Update(const AwaServerClientUpdateEvent * event, void * context)
    {
        if (context)
        {
            auto * that = static_cast<SingleStaticClientObjectWaitCondition*>(context);


            AwaRegisteredEntityIterator * objectIterator = AwaServerClientUpdateEvent_NewRegisteredEntityIterator(event, that->ClientEndpointName.c_str());
            if(objectIterator)
            {

                while (AwaRegisteredEntityIterator_Next(objectIterator))
                {
                    if (that->ObjectPath.compare(AwaRegisteredEntityIterator_GetPath(objectIterator)) == 0)
                    {
                        that->found = !(that->inverse);
                    }
                }

                AwaRegisteredEntityIterator_Free(&objectIterator);
            }
        }
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

class StaticClientCallbackWaitCondition : public WaitCondition
{
protected:
    AwaStaticClient * StaticClient;

public:
    StaticClientCallbackWaitCondition(AwaStaticClient * StaticClient, int milliseconds) :
        WaitCondition(1e4, (milliseconds * 1e3)), StaticClient(StaticClient), complete(false) {}
    bool complete;
    virtual bool Check()
    {
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

typedef struct
{
    volatile bool Run;
    AwaStaticClient * StaticClient;
} StaticClientProccessInfo;

void * do_static_client_process(void * attr);

} // namespace Awa

#endif // STATIC_API_SUPPORT_H

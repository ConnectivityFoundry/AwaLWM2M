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

namespace Awa {

class TestServerEvent : public TestServerWithConnectedSession
{
public:
    void SetUp()
    {
        TestServerWithConnectedSession::SetUp();
        callback_counter = 0;
    }
    void Callback()
    {
        ++callback_counter;
    }

protected:
    int callback_counter;
};

static void DispatchTestServerRegisterEvent(const AwaServerRegisterEvent * event, void * context) { static_cast<TestServerEvent *>(context)->Callback(); }

TEST_F(TestServerEvent, AwaServerSession_SetRegisterEventCallback_invalid_session)
{
    EXPECT_EQ(AwaError_SessionInvalid, AwaServerSession_SetRegisterEventCallback(NULL, NULL, NULL));
}

TEST_F(TestServerEvent, AwaServerSession_SetRegisterEventCallback_valid_null)
{
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetRegisterEventCallback(session_, NULL, NULL));
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetRegisterEventCallback(session_, NULL, this));
    EXPECT_EQ(0, this->callback_counter);
}

TEST_F(TestServerEvent, AwaServerSession_SetRegisterEventCallback_valid)
{
    EXPECT_EQ(AwaError_Success, AwaServerSession_SetRegisterEventCallback(session_, DispatchTestServerRegisterEvent, this));
    EXPECT_EQ(0, this->callback_counter);
}

// test register client

// test overwrite callback

// test clear callback


} // namespace Awa

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

#include "operation_common.h"
#include "client_session.h"
#include "support/support.h"

namespace Awa {

class TestOperationCommon : public TestClientBase {};

// Missing tests!
TEST_F(TestOperationCommon, DISABLED_OperationCommon_AddPath_does_blah)
{
    // TODO
    ASSERT_FALSE(true);
}

TEST_F(TestOperationCommon, OperationCommon_New_handles_null_session)
{
    OperationCommon * operation = OperationCommon_New(NULL, SessionType_Client);
    ASSERT_EQ(NULL, OperationCommon_GetSession(operation, NULL));
    OperationCommon_Free(&operation);
}

TEST_F(TestOperationCommon, OperationCommon_New_and_Free_works)
{
    AwaClientSession * session = (AwaClientSession *)12345;
    OperationCommon * operation = OperationCommon_New(session, SessionType_Server);
    ASSERT_TRUE(NULL != operation);
    ASSERT_EQ(session, OperationCommon_GetSession(operation, NULL));
    OperationCommon_Free(&operation);
}

TEST_F(TestOperationCommon, OperationCommon_NewWithClientSession)
{
    AwaClientSession * session = (AwaClientSession *)12345;
    OperationCommon * operation = OperationCommon_NewWithClientSession(session);
    ASSERT_TRUE(NULL != operation);
    SessionType sessionType = SessionType_Invalid;
    ASSERT_EQ(session, OperationCommon_GetSession(operation, &sessionType));
    ASSERT_EQ(SessionType_Client, sessionType);
    OperationCommon_Free(&operation);
}

TEST_F(TestOperationCommon, OperationCommon_NewWithServerSession)
{
    AwaServerSession * session = (AwaServerSession *)12345;
    OperationCommon * operation = OperationCommon_NewWithServerSession(session);
    ASSERT_TRUE(NULL != operation);
    SessionType sessionType = SessionType_Invalid;
    ASSERT_EQ(session, OperationCommon_GetSession(operation, &sessionType));
    ASSERT_EQ(SessionType_Server, sessionType);
    OperationCommon_Free(&operation);
}

TEST_F(TestOperationCommon, OperationCommon_Free_nulls_pointer)
{
    AwaClientSession * session = (AwaClientSession *)12345;
    OperationCommon * operation = OperationCommon_New(session, SessionType_Client);
    ASSERT_TRUE(NULL != operation);
    OperationCommon_Free(&operation);
    ASSERT_EQ(NULL, operation);
}

TEST_F(TestOperationCommon, OperationCommon_Free_handles_null)
{
    OperationCommon_Free(NULL);
}

TEST_F(TestOperationCommon, OperationCommon_Free_handles_null_pointer)
{
    OperationCommon * operation = NULL;
    OperationCommon_Free(&operation);
}

TEST_F(TestOperationCommon, OperationCommon_GetObjectsTree_handles_null)
{
    ASSERT_EQ(NULL, OperationCommon_GetObjectsTree(NULL));
}

TEST_F(TestOperationCommon, OperationCommon_GetObjectsTree_handles_valid_operation)
{
    AwaServerSession * session = (AwaServerSession *)12345;
    OperationCommon * operation = OperationCommon_New(session, SessionType_Server);
    ASSERT_TRUE(NULL != OperationCommon_GetObjectsTree(operation));
    OperationCommon_Free(&operation);
}

TEST_F(TestOperationCommon, OperationCommon_GetSession_handles_null)
{
    ASSERT_EQ(NULL, OperationCommon_GetSession(NULL, NULL));
}

TEST_F(TestOperationCommon, OperationCommon_GetSession_handles_client_session)
{
    AwaClientSession * session = AwaClientSession_New();
    OperationCommon * operation = OperationCommon_New(session, SessionType_Client);
    SessionType sessionType = SessionType_Invalid;
    ASSERT_TRUE(NULL != OperationCommon_GetSession(operation, &sessionType));
    ASSERT_EQ(SessionType_Client, sessionType);
    OperationCommon_Free(&operation);
    AwaClientSession_Free(&session);
}

TEST_F(TestOperationCommon, OperationCommon_GetSession_handles_server_session)
{
    AwaServerSession * session = AwaServerSession_New();
    OperationCommon * operation = OperationCommon_New(session, SessionType_Server);
    SessionType sessionType = SessionType_Invalid;
    ASSERT_TRUE(NULL != OperationCommon_GetSession(operation, &sessionType));
    ASSERT_EQ(SessionType_Server, sessionType);
    OperationCommon_Free(&operation);
    AwaServerSession_Free(&session);
}

// OperationCommon_AddPath implicitly tested by Get/Delete operation tests

// OperationCommon_AddPathWithArrayRange implicitly tested by Get/Delete operation tests

} // namespace Awa

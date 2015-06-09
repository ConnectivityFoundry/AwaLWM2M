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

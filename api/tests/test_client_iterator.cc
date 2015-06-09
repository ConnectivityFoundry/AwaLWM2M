#include <gtest/gtest.h>

#include "client_iterator.h"
#include "support/support.h"

namespace Awa {

class TestClientIterator : public TestClientBase {};

TEST_F(TestClientIterator, ClientIterator_New_and_Free)
{
    ClientIterator * iterator = ClientIterator_New();
    ASSERT_TRUE(NULL != iterator);
    ClientIterator_Free(&iterator);
    EXPECT_EQ(NULL, iterator);
}

TEST_F(TestClientIterator, ClientIterator_Free_handles_null_pointer)
{
    ClientIterator * iterator = NULL;
    ClientIterator_Free(&iterator);
    EXPECT_EQ(NULL, iterator);
}

TEST_F(TestClientIterator, ClientIterator_Free_handles_null)
{
    ClientIterator_Free(NULL);
}

TEST_F(TestClientIterator, ClientIterator_Next_handles_empty_iterator)
{
    ClientIterator * iterator = ClientIterator_New();
    EXPECT_TRUE(false == ClientIterator_Next(iterator));
    ClientIterator_Free(&iterator);
}

TEST_F(TestClientIterator, ClientIterator_Next_handles_non_empty_iterator)
{
    ClientIterator * iterator = ClientIterator_New();
    ClientIterator_Add(iterator, "TestIMG1");
    ClientIterator_Add(iterator, "Imagination0");
    ClientIterator_Add(iterator, "ClientABC123____________abcdef");
    EXPECT_EQ(true, ClientIterator_Next(iterator));
    EXPECT_STREQ("TestIMG1", ClientIterator_GetClientID(iterator));
    EXPECT_EQ(true, ClientIterator_Next(iterator));
    EXPECT_STREQ("Imagination0", ClientIterator_GetClientID(iterator));
    EXPECT_EQ(true, ClientIterator_Next(iterator));
    EXPECT_STREQ("ClientABC123____________abcdef", ClientIterator_GetClientID(iterator));
    EXPECT_TRUE(false == ClientIterator_Next(iterator));
    EXPECT_TRUE(false == ClientIterator_Next(iterator));
    EXPECT_TRUE(false == ClientIterator_Next(iterator));
    EXPECT_EQ(NULL, ClientIterator_GetClientID(iterator));
    ClientIterator_Free(&iterator);
}

TEST_F(TestClientIterator, ClientIterator_Next_handles_null)
{
    EXPECT_TRUE(false == ClientIterator_Next(NULL));
}

TEST_F(TestClientIterator, AwaClientIterator_Next_handles_null)
{
    EXPECT_TRUE(false == AwaClientIterator_Next(NULL));
}

TEST_F(TestClientIterator, ClientIterator_handles_Get_before_Next)
{
    ClientIterator * iterator = ClientIterator_New();
    ClientIterator_Add(iterator, "TestIMG1");
    ClientIterator_Add(iterator, "Imagination0");
    ASSERT_TRUE(NULL == ClientIterator_GetClientID(iterator));
    ClientIterator_Free(&iterator);
}



} // namespace Awa

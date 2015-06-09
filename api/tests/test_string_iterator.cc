#include <gtest/gtest.h>

#include "string_iterator.h"
#include "support/support.h"

namespace Awa {

class TestStringIterator : public TestClientBase {};

TEST_F(TestStringIterator, StringIterator_New_and_Free)
{
    StringIterator * iterator = StringIterator_New();
    ASSERT_TRUE(NULL != iterator);
    StringIterator_Free(&iterator);
    EXPECT_EQ(NULL, iterator);
}

TEST_F(TestStringIterator, StringIterator_Free_handles_null_pointer)
{
    StringIterator * iterator = NULL;
    StringIterator_Free(&iterator);
    EXPECT_EQ(NULL, iterator);
}

TEST_F(TestStringIterator, StringIterator_Free_handles_null)
{
    StringIterator_Free(NULL);
}

TEST_F(TestStringIterator, StringIterator_Add_handles_null)
{
    StringIterator_Add(NULL, "/3/4/5");
}

TEST_F(TestStringIterator, StringIterator_Next_handles_empty_iterator)
{
    StringIterator * iterator = StringIterator_New();
    EXPECT_TRUE(false == StringIterator_Next(iterator));
    StringIterator_Free(&iterator);
}

TEST_F(TestStringIterator, StringIterator_Next_handles_non_empty_iterator)
{
    StringIterator * iterator = StringIterator_New();
    StringIterator_Add(iterator, "/3/4/5");
    StringIterator_Add(iterator, "/1/2");
    StringIterator_Add(iterator, "/0");
    EXPECT_EQ(true, StringIterator_Next(iterator));
    EXPECT_STREQ("/3/4/5", StringIterator_Get(iterator));
    EXPECT_EQ(true, StringIterator_Next(iterator));
    EXPECT_STREQ("/1/2", StringIterator_Get(iterator));
    EXPECT_EQ(true, StringIterator_Next(iterator));
    EXPECT_STREQ("/0", StringIterator_Get(iterator));
    EXPECT_TRUE(false == StringIterator_Next(iterator));
    EXPECT_TRUE(false == StringIterator_Next(iterator));
    EXPECT_TRUE(false == StringIterator_Next(iterator));
    EXPECT_EQ(NULL, StringIterator_Get(iterator));
    StringIterator_Free(&iterator);
}

TEST_F(TestStringIterator, StringIterator_Next_handles_null)
{
    EXPECT_TRUE(false == StringIterator_Next(NULL));
}

TEST_F(TestStringIterator, StringIterator_handles_Get_before_Next)
{
    StringIterator * iterator = StringIterator_New();
    StringIterator_Add(iterator, "/3/4/5");
    StringIterator_Add(iterator, "/1/2");
    StringIterator_Add(iterator, "/0");
    ASSERT_TRUE(NULL == StringIterator_Get(iterator));
    StringIterator_Free(&iterator);
}

TEST_F(TestStringIterator, StringIterator_Get_handles_null)
{
    StringIterator_Get(NULL);
}

} // namespace Awa

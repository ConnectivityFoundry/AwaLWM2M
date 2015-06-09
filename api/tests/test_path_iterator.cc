#include <gtest/gtest.h>

#include "path_iterator.h"
#include "support/support.h"

namespace Awa {

class TestPathIterator : public TestClientBase {};

// Can only test a subset of API wrapper functionality as there is no AwaPathIterator_New()
// defined, and it would break black-box testing to use the knowledge that it is implemented
// with a PathIterator.

TEST_F(TestPathIterator, AwaPathIterator_Free_handles_null_pointer)
{
    AwaPathIterator * iterator = NULL;
    AwaPathIterator_Free(&iterator);
    EXPECT_EQ(NULL, iterator);
}

TEST_F(TestPathIterator, AwaPathIterator_Free_handles_null)
{
    AwaPathIterator_Free(NULL);
}

TEST_F(TestPathIterator, AwaPathIterator_Next_handles_null)
{
    EXPECT_TRUE(false == AwaPathIterator_Next(NULL));
}

TEST_F(TestPathIterator, AwaPathIterator_Get_handles_null)
{
    AwaPathIterator_Get(NULL);
}

TEST_F(TestPathIterator, PathIterator_New_and_Free)
{
    PathIterator * iterator = PathIterator_New();
    ASSERT_TRUE(NULL != iterator);
    PathIterator_Free(&iterator);
    EXPECT_EQ(NULL, iterator);
}

TEST_F(TestPathIterator, PathIterator_Free_handles_null_pointer)
{
    PathIterator * iterator = NULL;
    PathIterator_Free(&iterator);
    EXPECT_EQ(NULL, iterator);
}

TEST_F(TestPathIterator, PathIterator_Free_handles_null)
{
    PathIterator_Free(NULL);
}

TEST_F(TestPathIterator, PathIterator_Add_handles_null)
{
    PathIterator_Add(NULL, "/3/4/5");
}

TEST_F(TestPathIterator, PathIterator_Next_handles_empty_iterator)
{
    PathIterator * iterator = PathIterator_New();
    EXPECT_TRUE(false == PathIterator_Next(iterator));
    PathIterator_Free(&iterator);
}

TEST_F(TestPathIterator, PathIterator_Next_handles_non_empty_iterator)
{
    PathIterator * iterator = PathIterator_New();
    PathIterator_Add(iterator, "/3/4/5");
    PathIterator_Add(iterator, "/1/2");
    PathIterator_Add(iterator, "/0");
    EXPECT_EQ(true, PathIterator_Next(iterator));
    EXPECT_STREQ("/3/4/5", PathIterator_Get(iterator));
    EXPECT_EQ(true, PathIterator_Next(iterator));
    EXPECT_STREQ("/1/2", PathIterator_Get(iterator));
    EXPECT_EQ(true, PathIterator_Next(iterator));
    EXPECT_STREQ("/0", PathIterator_Get(iterator));
    EXPECT_TRUE(false == PathIterator_Next(iterator));
    EXPECT_TRUE(false == PathIterator_Next(iterator));
    EXPECT_TRUE(false == PathIterator_Next(iterator));
    EXPECT_EQ(NULL, PathIterator_Get(iterator));
    PathIterator_Free(&iterator);
}

TEST_F(TestPathIterator, PathIterator_Next_handles_null)
{
    EXPECT_TRUE(false == PathIterator_Next(NULL));
}

TEST_F(TestPathIterator, PathIterator_handles_Get_before_Next)
{
    PathIterator * iterator = PathIterator_New();
    PathIterator_Add(iterator, "/3/4/5");
    PathIterator_Add(iterator, "/1/2");
    PathIterator_Add(iterator, "/0");
    ASSERT_TRUE(NULL == PathIterator_Get(iterator));
    PathIterator_Free(&iterator);
}

TEST_F(TestPathIterator, PathIterator_Get_handles_null)
{
    PathIterator_Get(NULL);
}

} // namespace Awa

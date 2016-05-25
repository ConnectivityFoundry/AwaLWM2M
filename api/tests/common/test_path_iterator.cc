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

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

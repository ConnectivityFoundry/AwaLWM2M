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
#include <algorithm>

#include "iterator.h"
#include "support/support.h"

namespace Awa {

class TestIterator : public TestAwaBase {};

TEST_F(TestIterator, Iterator_New_and_Free)
{
    Iterator * iterator = Iterator_New();
    ASSERT_TRUE(NULL != iterator);
    Iterator_Free(&iterator);
    EXPECT_EQ(NULL, iterator);
}

TEST_F(TestIterator, Iterator_Free_handles_null)
{
    Iterator_Free(NULL);
}

TEST_F(TestIterator, Iterator_Free_handles_null_pointer)
{
    Iterator * iterator = NULL;
    Iterator_Free(&iterator);
    EXPECT_EQ(NULL, iterator);
}

TEST_F(TestIterator, Iterator_FreeValues_handles_null)
{
    Iterator_FreeValues(NULL);
}

TEST_F(TestIterator, Iterator_Add_handles_null_iterator)
{
    Iterator_Add(NULL, NULL);
}

TEST_F(TestIterator, Iterator_Add_handles_valid_iterator)
{
    Iterator * iterator = Iterator_New();
    Iterator_Add(iterator, NULL);
    Iterator_Free(&iterator);
}

TEST_F(TestIterator, Iterator_Get_handles_null)
{
    Iterator_Get(NULL);
}

TEST_F(TestIterator, Iterator_Next_handles_null)
{
    Iterator_Next(NULL);
}

TEST_F(TestIterator, Iterator_Get_without_Next)
{
    Iterator * iterator = Iterator_New();
    EXPECT_EQ(NULL, Iterator_Get(iterator));
    int value = 7;
    Iterator_Add(iterator, &value);
    EXPECT_EQ(NULL, Iterator_Get(iterator));
    Iterator_Free(&iterator);
}

TEST_F(TestIterator, Iterator_single_item)
{
    Iterator * iterator = Iterator_New();
    int value = 7;
    Iterator_Add(iterator, &value);
    EXPECT_TRUE(Iterator_Next(iterator));
    EXPECT_EQ(7, *(int *)Iterator_Get(iterator));
    EXPECT_EQ(7, value);
    EXPECT_FALSE(Iterator_Next(iterator));
    EXPECT_FALSE(Iterator_Next(iterator));
    Iterator_Free(&iterator);
}

TEST_F(TestIterator, Iterator_multiple_items)
{
    std::vector<int> expectedValues { 7, 42, 99, 1234567 };
    Iterator * iterator = Iterator_New();

    for (auto it = expectedValues.begin(); it != expectedValues.end(); ++it)
    {
        Iterator_Add(iterator, &(*it));
    }

    std::vector<int> actualValues;
    while (Iterator_Next(iterator))
    {
        int value = *(int *)Iterator_Get(iterator);
        actualValues.push_back(value);
    }

    EXPECT_EQ(NULL, Iterator_Get(iterator));

    EXPECT_EQ(expectedValues.size(), actualValues.size());
    if (expectedValues.size() == actualValues.size())
        EXPECT_TRUE(std::is_permutation(expectedValues.begin(), expectedValues.end(), actualValues.begin()));

    Iterator_Free(&iterator);
}

TEST_F(TestIterator, Iterator_single_managed_item)
{
    Iterator * iterator = Iterator_New();
    int * value = (int *)malloc(sizeof(*value));
    *value = 9;
    Iterator_Add(iterator, value);
    EXPECT_TRUE(Iterator_Next(iterator));
    EXPECT_EQ(9, *(int *)Iterator_Get(iterator));
    EXPECT_EQ(9, *value);
    EXPECT_FALSE(Iterator_Next(iterator));
    EXPECT_FALSE(Iterator_Next(iterator));
    Iterator_FreeValues(iterator);
    Iterator_Free(&iterator);
}

TEST_F(TestIterator, Iterator_multiple_managed_items)
{
    std::vector<int> expectedValues { 7, 42, 99, 1234567 };
    Iterator * iterator = Iterator_New();

    for (auto it = expectedValues.begin(); it != expectedValues.end(); ++it)
    {
        int * value = (int *)malloc(sizeof(*value));
        *value = *it;
        Iterator_Add(iterator, value);
    }

    std::vector<int> actualValues;
    while (Iterator_Next(iterator))
    {
        int value = *(int *)Iterator_Get(iterator);
        actualValues.push_back(value);
    }

    EXPECT_EQ(NULL, Iterator_Get(iterator));

    EXPECT_EQ(expectedValues.size(), actualValues.size());
    if (expectedValues.size() == actualValues.size())
        EXPECT_TRUE(std::is_permutation(expectedValues.begin(), expectedValues.end(), actualValues.begin()));

    Iterator_FreeValues(iterator);
    Iterator_Free(&iterator);
}

} // namespace Awa

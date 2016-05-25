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

#include "memalloc.h"
#include "list.h"
#include "support/support.h"

namespace Awa {

class TestList : public TestClientBase {};

TEST_F(TestList, List_New_Free_valid_inputs)
{
    ListType * list = List_New();
    ASSERT_TRUE(NULL != list);

    List_Free(&list);
    ASSERT_TRUE(NULL == list);
}

TEST_F(TestList, List_Free_invalid_inputs)
{
    ListType ** list = NULL;

    List_Free(NULL);
    List_Free(list);
}

TEST_F(TestList, List_Add_Get_valid_inputs)
{
    ListType * list = List_New();
    ASSERT_TRUE(NULL != list);

    int item = 300;

    EXPECT_TRUE(List_Add(list, (void*)&item));

    int * returnValue;
    EXPECT_TRUE(List_Get(list, 0, (void**)&returnValue));
    EXPECT_EQ(300, *returnValue);

    List_Free(&list);
    ASSERT_TRUE(NULL == list);
}

TEST_F(TestList, List_Add_Remove)
{
    ListType * list = List_New();
    ASSERT_TRUE(NULL != list);

    int item = 300;

    EXPECT_TRUE(List_Add(list, (void*)&item));

    int * returnValue;
    EXPECT_TRUE(List_Get(list, 0, (void**)&returnValue));
    EXPECT_EQ(300, *returnValue);

    EXPECT_TRUE(List_Remove(list, &item));
    EXPECT_FALSE(List_Get(list, 0, (void**)&returnValue));
    EXPECT_FALSE(List_Remove(list, &item));

    List_Free(&list);
    ASSERT_TRUE(NULL == list);
}

TEST_F(TestList, List_Add_Remove_multiple)
{
    ListType * list = List_New();
    ASSERT_TRUE(NULL != list);

    int item1 = 300;
    int item2 = 301;

    EXPECT_TRUE(List_Add(list, (void*)&item1));
    EXPECT_TRUE(List_Add(list, (void*)&item2));

    EXPECT_TRUE(List_Contains(list, &item1));
    EXPECT_TRUE(List_Contains(list, &item2));

    EXPECT_TRUE(List_Remove(list, &item2));
    EXPECT_TRUE(List_Contains(list, &item1));
    EXPECT_FALSE(List_Contains(list, &item2));

    EXPECT_TRUE(List_Remove(list, &item1));
    EXPECT_FALSE(List_Contains(list, &item1));

    EXPECT_FALSE(List_Remove(list, &item1));
    EXPECT_FALSE(List_Remove(list, &item2));

    List_Free(&list);
    ASSERT_TRUE(NULL == list);
}


/*TEST_F(TestList, List_Contains_valid_invalid_inputs)
{
    ListType * list = List_New();
    ASSERT_TRUE(NULL != list);

    int item3 = 300;

    EXPECT_TRUE(List_Put(list, "300", (void*)&item3));

    EXPECT_TRUE(List_Contains(list, "300"));
    EXPECT_FALSE(List_Contains(list, "301"));
    EXPECT_FALSE(List_Contains(list, NULL));

    List_Free(&list);
    ASSERT_TRUE(NULL == list);
}

TEST_F(TestList, List_FreeValues_valid_invalid_inputs)
{
    ListType * list = List_New();
    ASSERT_TRUE(NULL != list);

    int * item3 = (int*)malloc(sizeof(int));
    *item3 = 300;

    EXPECT_TRUE(List_Put(list, "300", (void*)item3));
    List_FreeValues(list);
    EXPECT_EQ(0u, List_Length(list));

    List_Free(&list);
    ASSERT_TRUE(NULL == list);
}

TEST_F(TestList, List_Remove_valid_invalid_inputs)
{
    ListType * list = List_New();
    ASSERT_TRUE(NULL != list);

    int item3 = 300;

    EXPECT_TRUE(List_Put(list, "300", (void*)&item3));

    int * returnValue = (int *)List_Remove(list, "300");
    EXPECT_EQ(300, *returnValue);
    EXPECT_TRUE(NULL == List_Remove(list, "301"));
    EXPECT_TRUE(NULL == List_Remove(list, NULL));

    List_Free(&list);
    ASSERT_TRUE(NULL == list);
}

TEST_F(TestList, List_Put_Get_non_existent_input)
{
    ListType * list = List_New();
    ASSERT_TRUE(NULL != list);

    void * returnValue;
    EXPECT_FALSE(List_Get(list, "300", (void**)&returnValue));

    List_Free(&list);
    ASSERT_TRUE(NULL == list);
}

TEST_F(TestList, List_Put_overwrite_same_value)
{
    ListType * list = List_New();
    ASSERT_TRUE(NULL != list);

    int item3 = 300;

    EXPECT_TRUE(List_Put(list, "300", (void*)&item3));
    EXPECT_EQ(1u, List_Length(list));
    EXPECT_TRUE(List_Put(list, "300", (void*)&item3));
    EXPECT_EQ(1u, List_Length(list));

    List_Free(&list);
    ASSERT_TRUE(NULL == list);
}

TEST_F(TestList, List_Put_Get_invalid_inputs)
{
    ListType * list = List_New();

    int item = 0;
    int * item_out;

    EXPECT_EQ(0, List_Put(NULL, NULL, (void*)&item));
    EXPECT_EQ(0, List_Put(list, NULL, NULL));
    EXPECT_EQ(0, List_Put(NULL, "3", NULL));

    EXPECT_EQ(0, List_Get(NULL, "3", NULL));
    EXPECT_EQ(0, List_Get(NULL, NULL, (void**)&item_out));
    EXPECT_EQ(0, List_Get(list, NULL, NULL));

    List_Free(&list);
}*/

TEST_F(TestList, List_Flush_with_items)
{
    ListType * list = List_New();

    int item1 = 100;
    int item2 = 200;
    int item3 = 300;

    EXPECT_EQ(1, List_Add(list, (void*)&item1));
    EXPECT_EQ(1, List_Add(list, (void*)&item2));
    EXPECT_EQ(1, List_Add(list, (void*)&item3));

    EXPECT_EQ(3u, List_Length(list));

    // how can we actually validate this?
    List_Flush(list);

    EXPECT_EQ(0u, List_Length(list));

    List_Free(&list);
}


const char * expectedContext1 = "Hello";
class ListForEachTest : public testing::Test
{
public:
    virtual void callbackHandler(size_t index, void * value, void * context) {};
};

void listCallback1(size_t index, void * value, void * context)
{
    if (context)
    {
        auto * that = static_cast<ListForEachTest*>(context);
        that->callbackHandler(index, value, (void *)expectedContext1);
    }
}

TEST_F(TestList, List_ForEach)
{
    struct ListCallbackHandler1 : public ListForEachTest
    {
        int count;

        ListCallbackHandler1() : count(0) {}

        void callbackHandler(size_t index, void * value, void * context)
        {
            count ++;
            ASSERT_STREQ(expectedContext1, (const char *)context);
            ASSERT_EQ(static_cast<int>(index), *((int*)value));
        }
        void TestBody() {}
    };
    ListCallbackHandler1 cbHandler;

    ListType * list = List_New();

    int item1 = 0;
    int item2 = 1;
    int item3 = 2;

    EXPECT_EQ(1, List_Add(list, (void*)&item1));
    EXPECT_EQ(1, List_Add(list, (void*)&item2));
    EXPECT_EQ(1, List_Add(list, (void*)&item3));

    List_ForEach(list, listCallback1, &cbHandler);

    ASSERT_EQ(3, cbHandler.count);

    List_Free(&list);
}

/*TODO: size_t List_Find(ListType * list, ListFindFunction callback, void * context, void ** value);
size_t List_Remove(ListType * list, ListFindFunction findFunction, void * context);*/

} // namespace Awa

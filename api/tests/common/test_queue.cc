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
#include "queue.h"
#include "support/support.h"

namespace Awa {

class TestQueues : public TestClientBase {};

TEST_F(TestQueues, Queue_New_Free_valid_inputs)
{
    QueueType * queue = Queue_New();
    ASSERT_TRUE(NULL != queue);

    Queue_Free(&queue);
    ASSERT_TRUE(NULL == queue);
}

TEST_F(TestQueues, Queue_Free_invalid_inputs)
{
    QueueType ** queue = NULL;

    Queue_Free(NULL);
    Queue_Free(queue);
}

TEST_F(TestQueues, Queue_Push_Pop_maintains_order)
{
    QueueType * queue = Queue_New();
    ASSERT_TRUE(NULL != queue);

    int item1 = 100;
    int item2 = 200;
    int item3 = 300;

    EXPECT_EQ(1, Queue_Push(queue, (void*)&item1));
    EXPECT_EQ(1, Queue_Push(queue, (void*)&item2));
    EXPECT_EQ(1, Queue_Push(queue, (void*)&item3));

    int * item_out;

    EXPECT_EQ(1, Queue_Pop(queue, (void**)&item_out));
    EXPECT_EQ(*item_out, item1);

    EXPECT_EQ(1, Queue_Pop(queue, (void**)&item_out));
    EXPECT_EQ(*item_out, item2);

    EXPECT_EQ(1, Queue_Pop(queue, (void**)&item_out));
    EXPECT_EQ(*item_out, item3);

    EXPECT_EQ(0, Queue_Pop(queue, (void**)&item_out));

    Queue_Free(&queue);
    ASSERT_TRUE(NULL == queue);
}

TEST_F(TestQueues, Queue_Push_Pop_invalid_inputs)
{
    QueueType * queue = Queue_New();

    int item = 0;
    int * item_out;

    EXPECT_EQ(0, Queue_Push(NULL, (void*)&item));
    EXPECT_EQ(0, Queue_Push(queue, NULL));
    EXPECT_EQ(0, Queue_Push(NULL, NULL));

    EXPECT_EQ(0, Queue_Pop(NULL, NULL));
    EXPECT_EQ(0, Queue_Pop(NULL, (void**)&item_out));
    EXPECT_EQ(0, Queue_Pop(queue, NULL));

    Queue_Free(&queue);
}

TEST_F(TestQueues, Queue_Flush_with_items)
{
    QueueType * queue = Queue_New();

    int item1 = 100;
    int item2 = 200;
    int item3 = 300;

    EXPECT_EQ(1, Queue_Push(queue, (void*)&item1));
    EXPECT_EQ(1, Queue_Push(queue, (void*)&item2));
    EXPECT_EQ(1, Queue_Push(queue, (void*)&item3));

    // how can we actually validate this?
    Queue_Flush(queue);

    Queue_Free(&queue);
}

} // namespace Awa

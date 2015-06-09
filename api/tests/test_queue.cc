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

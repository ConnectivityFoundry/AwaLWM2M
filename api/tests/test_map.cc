#include <gtest/gtest.h>

#include <tuple>
#include <vector>
#include <string>
#include <algorithm>

#include "memalloc.h"
#include "map.h"
#include "support/support.h"

namespace Awa {

class TestMap : public TestClientBase {};

TEST_F(TestMap, Map_New_Free_valid_inputs)
{
    MapType * map = Map_New();
    ASSERT_TRUE(NULL != map);

    Map_Free(&map);
    ASSERT_TRUE(NULL == map);
}

TEST_F(TestMap, Map_Free_invalid_inputs)
{
    MapType ** map = NULL;

    Map_Free(NULL);
    Map_Free(map);
}

TEST_F(TestMap, Map_Put_Get_valid_inputs)
{
    MapType * map = Map_New();
    ASSERT_TRUE(NULL != map);

    int item3 = 300;

    EXPECT_TRUE(Map_Put(map, "300", (void*)&item3));

    int * returnValue;
    EXPECT_TRUE(Map_Get(map, "300", (void**)&returnValue));
    EXPECT_EQ(300, *returnValue);

    Map_Free(&map);
    ASSERT_TRUE(NULL == map);
}

TEST_F(TestMap, Map_Contains_valid_invalid_inputs)
{
    MapType * map = Map_New();
    ASSERT_TRUE(NULL != map);

    int item3 = 300;

    EXPECT_TRUE(Map_Put(map, "300", (void*)&item3));

    EXPECT_TRUE(Map_Contains(map, "300"));
    EXPECT_FALSE(Map_Contains(map, "301"));
    EXPECT_FALSE(Map_Contains(map, NULL));

    Map_Free(&map);
    ASSERT_TRUE(NULL == map);
}

TEST_F(TestMap, Map_FreeValues_valid_invalid_inputs)
{
    MapType * map = Map_New();
    ASSERT_TRUE(NULL != map);

    int * item3 = (int*)malloc(sizeof(int));
    *item3 = 300;

    EXPECT_TRUE(Map_Put(map, "300", (void*)item3));
    Map_FreeValues(map);
    EXPECT_EQ(0u, Map_Length(map));

    Map_Free(&map);
    ASSERT_TRUE(NULL == map);
}

TEST_F(TestMap, Map_Remove_valid_invalid_inputs)
{
    MapType * map = Map_New();
    ASSERT_TRUE(NULL != map);

    int item3 = 300;

    EXPECT_TRUE(Map_Put(map, "300", (void*)&item3));

    int * returnValue = (int *)Map_Remove(map, "300");
    EXPECT_EQ(300, *returnValue);
    EXPECT_TRUE(NULL == Map_Remove(map, "301"));
    EXPECT_TRUE(NULL == Map_Remove(map, NULL));

    Map_Free(&map);
    ASSERT_TRUE(NULL == map);
}

TEST_F(TestMap, Map_Put_Get_non_existent_input)
{
    MapType * map = Map_New();
    ASSERT_TRUE(NULL != map);

    void * returnValue;
    EXPECT_FALSE(Map_Get(map, "300", (void**)&returnValue));

    Map_Free(&map);
    ASSERT_TRUE(NULL == map);
}

TEST_F(TestMap, Map_Put_overwrite_same_value)
{
    MapType * map = Map_New();
    ASSERT_TRUE(NULL != map);

    int item3 = 300;

    EXPECT_TRUE(Map_Put(map, "300", (void*)&item3));
    EXPECT_EQ(1u, Map_Length(map));
    EXPECT_TRUE(Map_Put(map, "300", (void*)&item3));
    EXPECT_EQ(1u, Map_Length(map));

    Map_Free(&map);
    ASSERT_TRUE(NULL == map);
}

TEST_F(TestMap, Map_Put_Get_invalid_inputs)
{
    MapType * map = Map_New();

    int item = 0;
    int * item_out;

    EXPECT_EQ(0, Map_Put(NULL, NULL, (void*)&item));
    EXPECT_EQ(0, Map_Put(map, NULL, NULL));
    EXPECT_EQ(0, Map_Put(NULL, "3", NULL));

    EXPECT_EQ(0, Map_Get(NULL, "3", NULL));
    EXPECT_EQ(0, Map_Get(NULL, NULL, (void**)&item_out));
    EXPECT_EQ(0, Map_Get(map, NULL, NULL));

    Map_Free(&map);
}

TEST_F(TestMap, Map_Flush_with_items)
{
    MapType * map = Map_New();

    int item1 = 100;
    int item2 = 200;
    int item3 = 300;

    EXPECT_EQ(1, Map_Put(map, "100", (void*)&item1));
    EXPECT_EQ(1, Map_Put(map, "200", (void*)&item2));
    EXPECT_EQ(1, Map_Put(map, "300", (void*)&item3));

    EXPECT_EQ(3u, Map_Length(map));

    // how can we actually validate this?
    Map_Flush(map);

    EXPECT_EQ(0u, Map_Length(map));

    Map_Free(&map);
}

namespace detail {

typedef std::tuple<std::string, void *> TestItem;
std::vector<TestItem> gActualItems;

static void TestForEach(const char * key, void * value, void * context)
{
    gActualItems.push_back(TestItem { key, value });
}

} // namespace detail

TEST_F(TestMap, Map_ForEach_handles_null)
{
    MapType * map = Map_New();
    int item1 = 100;
    int item2 = 200;
    int item3 = 300;
    Map_Put(map, "100", (void*)&item1);
    Map_Put(map, "200", (void*)&item2);
    Map_Put(map, "300", (void*)&item3);
    std::vector<detail::TestItem> expectedItems = {
            detail::TestItem { "100", (void*)&item1 },
            detail::TestItem { "200", (void*)&item2 },
            detail::TestItem { "300", (void*)&item3 },
    };
    detail::gActualItems.clear();
    EXPECT_EQ(3u, Map_ForEach(map, detail::TestForEach, NULL));
    EXPECT_EQ(expectedItems.size(), detail::gActualItems.size());
    if (expectedItems.size() == detail::gActualItems.size())
        EXPECT_TRUE(std::is_permutation(expectedItems.begin(), expectedItems.end(), detail::gActualItems.begin()));

    Map_Free(&map);
}

} // namespace Awa

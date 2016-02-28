#include <gtest/gtest.h>
#include "awa/static.h"


namespace AwaStatic {

class TestStaticClient : public testing::Test
{

};

TEST_F(TestStaticClient, AwaStaticClient_test)
{
    ASSERT_TRUE(2 != 1);
}

} // namespace AwaStatic

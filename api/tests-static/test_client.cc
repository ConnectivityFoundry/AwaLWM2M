#include <gtest/gtest.h>
#include "awa/static.h"


namespace AwaStatic {

class TestStaticClient : public testing::Test
{

};

TEST_F(TestStaticClient, AwaStaticClient_New_Free)
{
    AwaStaticClient * client = AwaStaticClient_New();

    ASSERT_TRUE(client != NULL);

    AwaStaticClient_Free(&client);

    ASSERT_TRUE(client == NULL);
}

} // namespace AwaStatic

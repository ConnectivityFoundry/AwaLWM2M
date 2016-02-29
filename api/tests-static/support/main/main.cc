
#include <gtest/gtest.h>
#include <string>
#include <stdio.h>

GTEST_API_ int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);

    auto result = RUN_ALL_TESTS();
    return result;

    return result;
}

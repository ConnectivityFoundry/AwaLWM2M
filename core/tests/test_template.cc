
#include <gtest/gtest.h>
#include <string>
#include <stdio.h>

class TemplateTestSuite : public testing::Test
{
  void SetUp() { }
  void TearDown() { }
};

TEST_F(TemplateTestSuite, test_template1)
{
  ASSERT_EQ(0, 0);
}


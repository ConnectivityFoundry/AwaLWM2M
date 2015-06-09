#include <gtest/gtest.h>

#include "error.h"

#include "support/support.h"

namespace Awa {

class TestError : public TestClientBase {};

TEST_F(TestError, AwaError_Strings_are_consistent)
{
    ASSERT_EQ(static_cast<size_t>(AwaError_LAST), Error_GetNumberOfErrorStrings());
}

TEST_F(TestError, AwaError_ToString)
{
    EXPECT_STREQ("AwaError_Success", AwaError_ToString(AwaError_Success));
    EXPECT_STREQ("AwaError_ObservationInvalid", AwaError_ToString(AwaError_ObservationInvalid));
}

TEST_F(TestError, AwaError_FromString)
{
    EXPECT_EQ(AwaError_Success, AwaError_FromString("AwaError_Success"));
    EXPECT_EQ(AwaError_ObservationInvalid, AwaError_FromString("AwaError_ObservationInvalid"));
}



} // namespace Awa

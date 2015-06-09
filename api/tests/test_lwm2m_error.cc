#include <gtest/gtest.h>
#include "../src/lwm2m_error.h"

#include "support/support.h"

namespace Awa {

class TestLWM2MError : public TestClientBase {};

TEST_F(TestLWM2MError, test_AwaLWM2MError_Strings_are_consistent)
{
    ASSERT_EQ(static_cast<size_t>(AwaLWM2MError_LAST), LWM2MError_GetNumberOfServerErrorStrings());
}

TEST_F(TestLWM2MError, test_AwaLWM2MError_ToString)
{
    EXPECT_STREQ("AwaLWM2MError_Success", AwaLWM2MError_ToString(AwaLWM2MError_Success));
    EXPECT_STREQ("AwaLWM2MError_NotAcceptable", AwaLWM2MError_ToString(AwaLWM2MError_NotAcceptable));
}

TEST_F(TestLWM2MError, test_AwaLWM2MError_FromString)
{
    EXPECT_EQ(AwaLWM2MError_Success, AwaLWM2MError_FromString("AwaLWM2MError_Success"));
    EXPECT_EQ(AwaLWM2MError_NotAcceptable, AwaLWM2MError_FromString("AwaLWM2MError_NotAcceptable"));
}

} // namespace Awa

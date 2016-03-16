#include <gtest/gtest.h>

#include "memalloc.h"
#include "support/support.h"
#include "utils.h"

namespace Awa {

class TestUtils : public TestClientBase {};

TEST_F(TestUtils, msprintf_null_string_ptr)
{
    ASSERT_EQ(static_cast<size_t>(0), msprintf(NULL, "%s", "Foo"));
}

TEST_F(TestUtils, msprintf_null_format_ptr)
{
    char * string = NULL;
    ASSERT_EQ(static_cast<size_t>(0), msprintf(&string, NULL, "Foo"));
}

TEST_F(TestUtils, msprintf_correct)
{
    char * string = NULL;
    EXPECT_EQ(static_cast<size_t>(11), msprintf(&string, "%s/%d", "ABCDE", 12345));
    ASSERT_TRUE(NULL != string);
    EXPECT_STREQ("ABCDE/12345", string);
    Awa_MemSafeFree(string);
}

TEST_F(TestUtils, test_AwaUtils_Strings_are_consistent)
{
    ASSERT_EQ(static_cast<size_t>(AwaResourceType_LAST), Utils_GetNumberOfResourceTypeStrings());
}

TEST_F(TestUtils, test_AwaUtils_ToString)
{
    EXPECT_STREQ("Invalid", Utils_ResourceTypeToString(AwaResourceType_Invalid));
    EXPECT_STREQ("ObjectLink Array", Utils_ResourceTypeToString(AwaResourceType_ObjectLinkArray));
}

TEST_F(TestUtils, Utils_GetPrimativeResourceType)
{
    EXPECT_EQ(AwaResourceType_Invalid, Utils_GetPrimativeResourceType(AwaResourceType_Invalid));
    EXPECT_EQ(AwaResourceType_None, Utils_GetPrimativeResourceType(AwaResourceType_None));
    EXPECT_EQ(AwaResourceType_String, Utils_GetPrimativeResourceType(AwaResourceType_String));
    EXPECT_EQ(AwaResourceType_Integer, Utils_GetPrimativeResourceType(AwaResourceType_Integer));
    EXPECT_EQ(AwaResourceType_Boolean, Utils_GetPrimativeResourceType(AwaResourceType_Boolean));
    EXPECT_EQ(AwaResourceType_Opaque, Utils_GetPrimativeResourceType(AwaResourceType_Opaque));
    EXPECT_EQ(AwaResourceType_Time, Utils_GetPrimativeResourceType(AwaResourceType_Time));
    EXPECT_EQ(AwaResourceType_ObjectLink, Utils_GetPrimativeResourceType(AwaResourceType_ObjectLink));
    EXPECT_EQ(AwaResourceType_String, Utils_GetPrimativeResourceType(AwaResourceType_StringArray));
    EXPECT_EQ(AwaResourceType_Integer, Utils_GetPrimativeResourceType(AwaResourceType_IntegerArray));
    EXPECT_EQ(AwaResourceType_Boolean, Utils_GetPrimativeResourceType(AwaResourceType_BooleanArray));
    EXPECT_EQ(AwaResourceType_Opaque, Utils_GetPrimativeResourceType(AwaResourceType_OpaqueArray));
    EXPECT_EQ(AwaResourceType_Time, Utils_GetPrimativeResourceType(AwaResourceType_TimeArray));
    EXPECT_EQ(AwaResourceType_ObjectLink, Utils_GetPrimativeResourceType(AwaResourceType_ObjectLinkArray));
    EXPECT_EQ(AwaResourceType_Invalid, Utils_GetPrimativeResourceType(static_cast<AwaResourceType>(-1)));
    EXPECT_EQ(AwaResourceType_Invalid, Utils_GetPrimativeResourceType(static_cast<AwaResourceType>(100000)));
}

} // namespace Awa

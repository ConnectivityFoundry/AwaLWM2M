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

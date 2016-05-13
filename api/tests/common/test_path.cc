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

#include "path.h"
#include "support/support.h"

namespace Awa {

class TestPath : public TestClientBase {};

class TestPathValidWithParam : public TestPath, public ::testing::WithParamInterface<const char *> {};

TEST_P(TestPathValidWithParam, Path_IsValid_handles_valid_path)
{
    EXPECT_EQ(true, Path_IsValid(GetParam()));
}

INSTANTIATE_TEST_CASE_P(
        TestPathValidPathsInstantation,
        TestPathValidWithParam,
        ::testing::Values(
                "/0",
                "/0/0",
                "/0/0/0",
                "/65535/65535/65535"
        ));



class TestPathInvalidWithParam : public TestPath, public ::testing::WithParamInterface<const char *> {};

TEST_P(TestPathInvalidWithParam, Path_IsValid_handles_invalid_path)
{
    EXPECT_TRUE(false == Path_IsValid(GetParam()));
}

INSTANTIATE_TEST_CASE_P(
        TestPathInvalidPathsInstantation,
        TestPathInvalidWithParam,
        ::testing::Values(
                "",
                "/",
                "0",
                "0/",
                "/0/",
                "/0/1/",
                "0/1/2",
                "0/1/2/",
                "/0/1/2/",
                "//0/1/2",
                "/0/1/2/3",
                "/a/b/c",
                "/3/0/d",
                "a/n in/valid/ path",
                "/70000/0/0",
                "/0/70000/0",
                "/0/0/70000",
                "/0/0/-1",
                "/0/-1/0",
                "/-1/0/0",
                "root/0/1/2",
                "root/sub/0/1/2",
                "/Device/0/Manufacturer",
                "root/Device/0/Manufacturer"
        ));

TEST_F(TestPath, Path_IsValid_handles_null)
{
    EXPECT_TRUE(false == Path_IsValid(NULL));
}

TEST_F(TestPath, Path_IsValidForObject_handles_invalid_path)
{
    EXPECT_TRUE(false == Path_IsValidForObject(NULL));
    EXPECT_TRUE(false == Path_IsValidForObject(""));
    EXPECT_TRUE(false == Path_IsValidForObject("a/n in/valid/ path"));
    EXPECT_TRUE(false == Path_IsValidForObject("/0/0"));
    EXPECT_TRUE(false == Path_IsValidForObject("/0/0/0"));
}

TEST_F(TestPath, Path_IsValidForObject_handles_valid_path)
{
    EXPECT_EQ(true, Path_IsValidForObject("/0"));
    EXPECT_EQ(true, Path_IsValidForObject("/1"));
    EXPECT_EQ(true, Path_IsValidForObject("/3"));
    EXPECT_EQ(true, Path_IsValidForObject("/65535"));
}

TEST_F(TestPath, Path_IsValidForObjectInstance_handles_invalid_path)
{
    EXPECT_TRUE(false == Path_IsValidForObjectInstance(NULL));
    EXPECT_TRUE(false == Path_IsValidForObjectInstance(""));
    EXPECT_TRUE(false == Path_IsValidForObjectInstance("a/n in/valid/ path"));
    EXPECT_TRUE(false == Path_IsValidForObjectInstance("/0"));
    EXPECT_TRUE(false == Path_IsValidForObjectInstance("/0/0/0"));
}

TEST_F(TestPath, Path_IsValidForObjectInstance_handles_valid_path)
{
    EXPECT_EQ(true, Path_IsValidForObjectInstance("/0/0"));
    EXPECT_EQ(true, Path_IsValidForObjectInstance("/1/0"));
    EXPECT_EQ(true, Path_IsValidForObjectInstance("/1/1"));
    EXPECT_EQ(true, Path_IsValidForObjectInstance("/0/65535"));
    EXPECT_EQ(true, Path_IsValidForObjectInstance("/65535/0"));
    EXPECT_EQ(true, Path_IsValidForObjectInstance("/65535/65535"));
}

TEST_F(TestPath, Path_IsValidForResource_handles_invalid_path)
{
    EXPECT_TRUE(false == Path_IsValidForResource(NULL));
    EXPECT_TRUE(false == Path_IsValidForResource(""));
    EXPECT_TRUE(false == Path_IsValidForResource("a/n in/valid/ path"));
    EXPECT_TRUE(false == Path_IsValidForResource("/0"));
    EXPECT_TRUE(false == Path_IsValidForResource("/0/0"));
}

TEST_F(TestPath, Path_IsValidForResource_handles_valid_path)
{
    EXPECT_EQ(true, Path_IsValidForResource("/0/0/0"));
    EXPECT_EQ(true, Path_IsValidForResource("/0/0/65535"));
    EXPECT_EQ(true, Path_IsValidForResource("/65535/0/0"));
    EXPECT_EQ(true, Path_IsValidForResource("/65535/65535/0"));
    EXPECT_EQ(true, Path_IsValidForResource("/65535/65535/65535"));
    EXPECT_EQ(true, Path_IsValidForResource("/3/0/0"));
}

TEST_F(TestPath, Path_GetObjectID_handles_valid_path)
{
    EXPECT_EQ(0, Path_GetObjectID("/0/0/0"));
    EXPECT_EQ(7, Path_GetObjectID("/7/5/2"));
    EXPECT_EQ(65535, Path_GetObjectID("/65535/0/1"));
}

TEST_F(TestPath, Path_GetObjectID_handles_invalid_path)
{
    EXPECT_EQ(AWA_INVALID_ID, Path_GetObjectID(NULL));
    EXPECT_EQ(AWA_INVALID_ID, Path_GetObjectID(""));
    EXPECT_EQ(AWA_INVALID_ID, Path_GetObjectID("65535/0/1"));
    EXPECT_EQ(AWA_INVALID_ID, Path_GetObjectID("/0/0/1/"));
}

TEST_F(TestPath, Path_GetObjectInstanceID_handles_valid_path)
{
    EXPECT_EQ(0, Path_GetObjectInstanceID("/0/0/0"));
    EXPECT_EQ(5, Path_GetObjectInstanceID("/7/5/2"));
    EXPECT_EQ(0, Path_GetObjectInstanceID("/65535/0/1"));
    EXPECT_EQ(65535, Path_GetObjectInstanceID("/1/65535/2"));
    EXPECT_EQ(2, Path_GetObjectInstanceID("/1/2"));
    EXPECT_EQ(AWA_INVALID_ID, Path_GetResourceID("/1"));
}

TEST_F(TestPath, Path_GetObjectInstanceID_handles_invalid_path)
{
    EXPECT_EQ(AWA_INVALID_ID, Path_GetObjectInstanceID(NULL));
    EXPECT_EQ(AWA_INVALID_ID, Path_GetObjectInstanceID(""));
    EXPECT_EQ(AWA_INVALID_ID, Path_GetObjectInstanceID("65535/0/1"));
    EXPECT_EQ(AWA_INVALID_ID, Path_GetObjectInstanceID("/0/0/1/"));
}

TEST_F(TestPath, Path_GetResourceID_handles_valid_path)
{
    EXPECT_EQ(0, Path_GetResourceID("/0/0/0"));
    EXPECT_EQ(2, Path_GetResourceID("/7/5/2"));
    EXPECT_EQ(65535, Path_GetResourceID("/2/0/65535"));
    EXPECT_EQ(AWA_INVALID_ID, Path_GetResourceID("/1/2"));
    EXPECT_EQ(AWA_INVALID_ID, Path_GetResourceID("/1"));
}

TEST_F(TestPath, Path_GetResourceID_handles_invalid_path)
{
    EXPECT_EQ(AWA_INVALID_ID, Path_GetResourceID(NULL));
    EXPECT_EQ(AWA_INVALID_ID, Path_GetResourceID(""));
    EXPECT_EQ(AWA_INVALID_ID, Path_GetResourceID("65535/0/1"));
    EXPECT_EQ(AWA_INVALID_ID, Path_GetResourceID("/0/0/1/"));
}

namespace detail {

struct TestItem
{
    bool Valid;
    const char * Path;
    int Matches;
    AwaObjectID ObjectID;
    AwaObjectInstanceID ObjectInstanceID;
    AwaResourceID ResourceID;
};

::std::ostream& operator<<(::std::ostream& os, const TestItem& item)
{
  return os << "Item: Valid " << item.Valid
            << ", Path " << item.Path
            << ", Matches " << item.Matches
            << ", ObjectID " << item.ObjectID
            << ", ObjectInstanceID " << item.ObjectInstanceID
            << ", ResourceID " << item.ResourceID;
}

} // namespace detail

class TestPathWithParam2 : public TestPath, public ::testing::WithParamInterface<detail::TestItem> {};

TEST_P(TestPathWithParam2, Path_Parse_handles_paths)
{
    AwaObjectID objectID = AWA_INVALID_ID;
    AwaObjectInstanceID objectInstanceID = AWA_INVALID_ID;
    AwaResourceID resourceID = AWA_INVALID_ID;
    int matches = 0;

    detail::TestItem item = GetParam();

    EXPECT_EQ(item.Valid, Path_Parse(item.Path, &matches, &objectID, &objectInstanceID, &resourceID));
    EXPECT_EQ(item.Matches, matches);
    EXPECT_EQ(item.ObjectID, objectID);
    EXPECT_EQ(item.ObjectInstanceID, objectInstanceID);
    EXPECT_EQ(item.ResourceID, resourceID);
}

INSTANTIATE_TEST_CASE_P(
        TestPathWithParam2Instantiation,
        TestPathWithParam2,
        ::testing::Values(
            detail::TestItem { true, "/1/2/3",  3, 1, 2, 3 },
            detail::TestItem { true, "/1/2/3",  3, 1, 2, 3 },
            detail::TestItem { true, "/1/2",    2, 1, 2, AWA_INVALID_ID },
            detail::TestItem { true, "/1",      1, 1, AWA_INVALID_ID, AWA_INVALID_ID },
            detail::TestItem { false, NULL,     0, AWA_INVALID_ID, AWA_INVALID_ID, AWA_INVALID_ID },
            detail::TestItem { false, "",       0, AWA_INVALID_ID, AWA_INVALID_ID, AWA_INVALID_ID },
            detail::TestItem { false, "1",      0, AWA_INVALID_ID, AWA_INVALID_ID, AWA_INVALID_ID }
        ));

TEST_F(TestPath, AwaAPI_MakeObjectPath_handles_null_path)
{
    EXPECT_EQ(AwaError_PathInvalid, AwaAPI_MakeObjectPath(NULL, 0, 0));
}

TEST_F(TestPath, AwaAPI_MakeObjectPath_handles_zero_size_path)
{
    char path[100] = { 0 }; strcpy(path, "abcde");
    EXPECT_EQ(AwaError_Overrun, AwaAPI_MakeObjectPath(path, 0, 0));
    EXPECT_STREQ("", path);
}

TEST_F(TestPath, AwaAPI_MakeObjectPath_handles_null_terminator_overrun)
{
    // expecting "/0" with null terminator, but no space for terminator
    char path[2] = { 0 }; strcpy(path, "a");
    EXPECT_EQ(AwaError_Overrun, AwaAPI_MakeObjectPath(path, 2, 0));
    EXPECT_STREQ("", path);
}

TEST_F(TestPath, AwaAPI_MakeObjectPath_handles_valid_inputs)
{
    char path[100] = { 0 }; strcpy(path, "abcde");
    EXPECT_EQ(AwaError_Success, AwaAPI_MakeObjectPath(path, 100, 0));
    EXPECT_STREQ("/0", path);

    EXPECT_EQ(AwaError_Success, AwaAPI_MakeObjectPath(path, 100, 12345));
    EXPECT_STREQ("/12345", path);
}

TEST_F(TestPath, AwaAPI_MakeObjectPath_handles_invalid_inputs)
{
    char path[100] = { 0 }; strcpy(path, "abcde");
    EXPECT_EQ(AwaError_IDInvalid, AwaAPI_MakeObjectPath(path, 100, AWA_INVALID_ID));
}

TEST_F(TestPath, AwaAPI_MakeObjectInstancePath_handles_null_path)
{
    EXPECT_EQ(AwaError_PathInvalid, AwaAPI_MakeObjectInstancePath(NULL, 0, 0, 0));
}

TEST_F(TestPath, AwaAPI_MakeObjectInstancePath_handles_zero_size_path)
{
    char path[100] = { 0 }; strcpy(path, "abcde");
    EXPECT_EQ(AwaError_Overrun, AwaAPI_MakeObjectInstancePath(path, 0, 0, 0));
    EXPECT_STREQ("", path);
}

TEST_F(TestPath, AwaAPI_MakeObjectInstancePath_handles_null_terminator_overrun)
{
    // expecting "/0" with null terminator, but no space for terminator
    char path[2] = { 0 }; strcpy(path, "a");
    EXPECT_EQ(AwaError_Overrun, AwaAPI_MakeObjectInstancePath(path, 2, 0, 0));
    EXPECT_STREQ("", path);
}

TEST_F(TestPath, AwaAPI_MakeObjectInstancePath_handles_valid_inputs)
{
    char path[100] = { 0 }; strcpy(path, "abcde");
    EXPECT_EQ(AwaError_Success, AwaAPI_MakeObjectInstancePath(path, 100, 0, 0));
    EXPECT_STREQ("/0/0", path);

    EXPECT_EQ(AwaError_Success, AwaAPI_MakeObjectInstancePath(path, 100, 12345, 54321));
    EXPECT_STREQ("/12345/54321", path);
}

TEST_F(TestPath, AwaAPI_MakeObjectInstancePath_handles_invalid_inputs)
{
    char path[100] = { 0 }; strcpy(path, "abcde");
    EXPECT_EQ(AwaError_IDInvalid, AwaAPI_MakeObjectInstancePath(path, 100, AWA_INVALID_ID, 0));
    EXPECT_EQ(AwaError_IDInvalid, AwaAPI_MakeObjectInstancePath(path, 100, 0, AWA_INVALID_ID));
}

TEST_F(TestPath, AwaAPI_MakeResourcePath_handles_null_path)
{
    EXPECT_EQ(AwaError_PathInvalid, AwaAPI_MakeResourcePath(NULL, 0, 0, 0, 0));
}

TEST_F(TestPath, AwaAPI_MakeResourcePath_handles_zero_size_path)
{
    char path[100] = { 0 }; strcpy(path, "abcde");
    EXPECT_EQ(AwaError_Overrun, AwaAPI_MakeResourcePath(path, 0, 0, 0, 0));
    EXPECT_STREQ("", path);
}

TEST_F(TestPath, AwaAPI_MakeResourcePath_handles_null_terminator_overrun)
{
    // expecting "/0" with null terminator, but no space for terminator
    char path[2] = { 0 }; strcpy(path, "a");
    EXPECT_EQ(AwaError_Overrun, AwaAPI_MakeResourcePath(path, 2, 0, 0, 0));
    EXPECT_STREQ("", path);
}

TEST_F(TestPath, AwaAPI_MakeResourcePath_handles_valid_inputs)
{
    char path[100] = { 0 }; strcpy(path, "abcde");
    EXPECT_EQ(AwaError_Success, AwaAPI_MakeResourcePath(path, 100, 0, 0, 0));
    EXPECT_STREQ("/0/0/0", path);

    EXPECT_EQ(AwaError_Success, AwaAPI_MakeResourcePath(path, 100, 12345, 54321, 44444));
    EXPECT_STREQ("/12345/54321/44444", path);
}

TEST_F(TestPath, AwaAPI_MakeResourcePath_handles_invalid_inputs)
{
    char path[100] = { 0 }; strcpy(path, "abcde");
    EXPECT_EQ(AwaError_IDInvalid, AwaAPI_MakeResourcePath(path, 100, AWA_INVALID_ID, 0, 0));
    EXPECT_EQ(AwaError_IDInvalid, AwaAPI_MakeResourcePath(path, 100, 0, AWA_INVALID_ID, 0));
    EXPECT_EQ(AwaError_IDInvalid, AwaAPI_MakeResourcePath(path, 100, 0, 0, AWA_INVALID_ID));
}

TEST_F(TestPath, AwaAPI_MakePath_handles_null_path)
{
    EXPECT_EQ(AwaError_PathInvalid, AwaAPI_MakePath(NULL, 0, 0, 0, 0));
}

TEST_F(TestPath, AwaAPI_MakePath_handles_zero_size_path)
{
    char path[100] = { 0 }; strcpy(path, "abcde");
    EXPECT_EQ(AwaError_Overrun, AwaAPI_MakePath(path, 0, 0, 0, 0));
    EXPECT_STREQ("", path);
}

TEST_F(TestPath, AwaAPI_MakePath_handles_null_terminator_overrun)
{
    // expecting "/0" with null terminator, but no space for terminator
    char path[2] = { 0 }; strcpy(path, "a");
    EXPECT_EQ(AwaError_Overrun, AwaAPI_MakePath(path, 2, 0, 0, 0));
    EXPECT_STREQ("", path);
}

TEST_F(TestPath, AwaAPI_MakePath_handles_valid_inputs)
{
    // invalid IDs can be specified, as long as they start on the 'right'
    char path[100] = { 0 }; strcpy(path, "abcde");
    EXPECT_EQ(AwaError_Success, AwaAPI_MakePath(path, 100, 0, 0, 0));
    EXPECT_STREQ("/0/0/0", path);

    EXPECT_EQ(AwaError_Success, AwaAPI_MakePath(path, 100, 12345, 54321, 44444));
    EXPECT_STREQ("/12345/54321/44444", path);
    EXPECT_EQ(AwaError_Success, AwaAPI_MakePath(path, 100, 12345, 54321, AWA_INVALID_ID));
    EXPECT_STREQ("/12345/54321", path);
    EXPECT_EQ(AwaError_Success, AwaAPI_MakePath(path, 100, 12345, AWA_INVALID_ID, AWA_INVALID_ID));
    EXPECT_STREQ("/12345", path);
}

TEST_F(TestPath, AwaAPI_MakePath_handles_invalid_inputs)
{
    // invalid IDs can be specified, as long as they start on the 'right'
    char path[100] = { 0 };
    strcpy(path, "abcde");
    EXPECT_EQ(AwaError_IDInvalid, AwaAPI_MakePath(path, 100, AWA_INVALID_ID, 0, 0));
    EXPECT_STREQ("", path);
    strcpy(path, "abcde");
    EXPECT_EQ(AwaError_IDInvalid, AwaAPI_MakePath(path, 100, 0, AWA_INVALID_ID, 0));
    EXPECT_STREQ("", path);
    strcpy(path, "abcde");
    EXPECT_EQ(AwaError_IDInvalid, AwaAPI_MakePath(path, 100, AWA_INVALID_ID, AWA_INVALID_ID, AWA_INVALID_ID));
    EXPECT_STREQ("", path);
}

TEST_F(TestPath, AwaAPI_IsPathValid_handles_null)
{
    EXPECT_FALSE(AwaAPI_IsPathValid(NULL));
}

TEST_F(TestPath, AwaAPI_IsPathValid_handles_valid_path)
{
    EXPECT_TRUE(AwaAPI_IsPathValid("/0"));
    EXPECT_TRUE(AwaAPI_IsPathValid("/0/0"));
    EXPECT_TRUE(AwaAPI_IsPathValid("/0/0/0"));
}

TEST_F(TestPath, AwaAPI_IsPathValid_handles_invalid_path)
{
    EXPECT_FALSE(AwaAPI_IsPathValid(""));
    EXPECT_FALSE(AwaAPI_IsPathValid("/"));
    EXPECT_FALSE(AwaAPI_IsPathValid("0"));
    EXPECT_FALSE(AwaAPI_IsPathValid("0/"));
    EXPECT_FALSE(AwaAPI_IsPathValid("/0/"));
    EXPECT_FALSE(AwaAPI_IsPathValid("/0/1/"));
    EXPECT_FALSE(AwaAPI_IsPathValid("0/1/2"));
    EXPECT_FALSE(AwaAPI_IsPathValid("0/1/2/"));
    EXPECT_FALSE(AwaAPI_IsPathValid("/0/1/2/"));
    EXPECT_FALSE(AwaAPI_IsPathValid("//0/1/2"));
    EXPECT_FALSE(AwaAPI_IsPathValid("/0/1/2/3"));
    EXPECT_FALSE(AwaAPI_IsPathValid("/a/b/c"));
    EXPECT_FALSE(AwaAPI_IsPathValid("/3/0/d"));
    EXPECT_FALSE(AwaAPI_IsPathValid("a/n in/valid/ path"));
    EXPECT_FALSE(AwaAPI_IsPathValid("/70000/0/0"));
    EXPECT_FALSE(AwaAPI_IsPathValid("/0/70000/0"));
    EXPECT_FALSE(AwaAPI_IsPathValid("/0/0/70000"));
    EXPECT_FALSE(AwaAPI_IsPathValid("/0/0/-1"));
    EXPECT_FALSE(AwaAPI_IsPathValid("/0/-1/0"));
    EXPECT_FALSE(AwaAPI_IsPathValid("/-1/0/0"));
    EXPECT_FALSE(AwaAPI_IsPathValid("root/0/1/2"));
    EXPECT_FALSE(AwaAPI_IsPathValid("root/sub/0/1/2"));
    EXPECT_FALSE(AwaAPI_IsPathValid("/Device/0/Manufacturer"));
    EXPECT_FALSE(AwaAPI_IsPathValid("root/Device/0/Manufacturer"));
}



} // namespace Awa

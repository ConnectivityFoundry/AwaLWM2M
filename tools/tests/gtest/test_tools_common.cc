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
#include "support/support.h"

#include "awa/client.h"
#include "tools_common.h"

namespace Awa {

class TestToolsCommon : public ::testing::Test {};

TEST_F(TestToolsCommon, IsArrayType)
{
    EXPECT_FALSE(IsArrayType(AwaResourceType_Invalid));
    EXPECT_FALSE(IsArrayType(AwaResourceType_None));
    EXPECT_FALSE(IsArrayType(AwaResourceType_String));
    EXPECT_FALSE(IsArrayType(AwaResourceType_Integer));
    EXPECT_FALSE(IsArrayType(AwaResourceType_Float));
    EXPECT_FALSE(IsArrayType(AwaResourceType_Boolean));
    EXPECT_FALSE(IsArrayType(AwaResourceType_Opaque));
    EXPECT_FALSE(IsArrayType(AwaResourceType_Time));
    EXPECT_FALSE(IsArrayType(AwaResourceType_ObjectLink));

    EXPECT_TRUE(IsArrayType(AwaResourceType_StringArray));
    EXPECT_TRUE(IsArrayType(AwaResourceType_IntegerArray));
    EXPECT_TRUE(IsArrayType(AwaResourceType_FloatArray));
    EXPECT_TRUE(IsArrayType(AwaResourceType_BooleanArray));
    EXPECT_TRUE(IsArrayType(AwaResourceType_OpaqueArray));
    EXPECT_TRUE(IsArrayType(AwaResourceType_TimeArray));
    EXPECT_TRUE(IsArrayType(AwaResourceType_ObjectLinkArray));

    EXPECT_FALSE(IsArrayType(AwaResourceType_LAST));
}

namespace detail {

struct TestItem
{
    const char * Path;
    size_t PathLen;
    AwaObjectID ObjectID;
    AwaObjectInstanceID ObjectInstanceID;
    AwaResourceID ResourceID;
};

::std::ostream& operator<<(::std::ostream& os, const TestItem& item)
{
    return os << "Item: path " << item.Path
              << "pathLen " << item.PathLen
              << "objectID " << item.ObjectID
              << "objectInstanceID " << item.ObjectInstanceID
              << "resourceID " << item.ResourceID;
}

} // namespace detail
class TestToolsCommonOirToPath : public TestToolsCommon, public ::testing::WithParamInterface<detail::TestItem> {
protected:
    char path[100] = {  }; 
    size_t pathLen = 100;
};

TEST_P(TestToolsCommonOirToPath, TestToolsOriPathValidation)
{
    detail::TestItem item = GetParam();
    OirToPath(path, pathLen, item.ObjectID, item.ObjectInstanceID, item.ResourceID);

    EXPECT_STREQ(item.Path, path);
}

INSTANTIATE_TEST_CASE_P(
        TestToolsCommonOirToPathInstantiation,
        TestToolsCommonOirToPath,
        ::testing::Values(
            detail::TestItem { "/1/2/3", 3, 1,                  2,                  3 },
            detail::TestItem { "/1/2",   2, 1,                  2,                  AWA_INVALID_ID },
            detail::TestItem { "/1",     1, 1,                  AWA_INVALID_ID, AWA_INVALID_ID },
            detail::TestItem { "",       0, AWA_INVALID_ID, AWA_INVALID_ID, AWA_INVALID_ID },
            detail::TestItem { "",       1, -1,                 AWA_INVALID_ID, AWA_INVALID_ID }
        ));

class TestToolsCommon_IsTargetValid_valid : public TestToolsCommon, public ::testing::WithParamInterface<const char *> {};

TEST_P(TestToolsCommon_IsTargetValid_valid, with_valid_target)
{
    EXPECT_TRUE(IsTargetValid(GetParam()));
}

INSTANTIATE_TEST_CASE_P(
        TestToolsCommon_IsTargetValid_valid,
        TestToolsCommon_IsTargetValid_valid,
        ::testing::Values(
                "/0",
                "/12345",
                "/0/0",
                "/12345/12345",
                "/1/2/3",
                "/12345/54321/44444",
                "/1/2/3/4",
                "/1/2/3=",
                "/1/2/3=foo",
                "/1/2/3/4=bar",
                "/1/2/3/4=foo bar"
        ));

class TestToolsCommon_IsTargetValid_invalid : public TestToolsCommon, public ::testing::WithParamInterface<const char *> {};

TEST_P(TestToolsCommon_IsTargetValid_invalid, with_invalid_target)
{
    EXPECT_FALSE(IsTargetValid(GetParam()));
}

INSTANTIATE_TEST_CASE_P(
        TestToolsCommon_IsTargetValid_invalid,
        TestToolsCommon_IsTargetValid_invalid,
        ::testing::Values(
                "",
                "/",
                "an/invalid/path",
                "0",
                "0/",
                "/-1",
                "/0/",
                "/0/0/",
                "/0/-1",
                "/0=",
                "/0/0="
        ));




class TestToolsCommonWithConnectedSession: public TestClientWithConnectedSession {};

TEST_F(TestToolsCommonWithConnectedSession, IsMultiValuedResource_with_single_valued_resource_returns_false)
{
    EXPECT_FALSE(Client_IsMultiValuedResource(session_, "/3/0/0"));
    EXPECT_FALSE(Client_IsMultiValuedResource(session_, "/3/0/1"));
    EXPECT_FALSE(Client_IsMultiValuedResource(session_, "/3/0/4"));
    EXPECT_FALSE(Client_IsMultiValuedResource(session_, "/3/0/9"));
}

TEST_F(TestToolsCommonWithConnectedSession, IsMultiValuedResource_with_multiple_valued_resource_returns_true)
{
    EXPECT_TRUE(Client_IsMultiValuedResource(session_, "/3/0/6"));
    EXPECT_TRUE(Client_IsMultiValuedResource(session_, "/3/0/7"));
    EXPECT_TRUE(Client_IsMultiValuedResource(session_, "/3/0/8"));
    EXPECT_TRUE(Client_IsMultiValuedResource(session_, "/3/0/11"));
    EXPECT_TRUE(Client_IsMultiValuedResource(session_, "/2/0/2"));
}


TEST_F(TestToolsCommonWithConnectedSession, GetResourceOperations_returns_correct_enum)
{
    //AwaResourceOperations test cases      
    ASSERT_EQ(AwaResourceOperations_ReadOnly,  Client_GetResourceOperations(session_, "/3/0/6"));
    ASSERT_EQ(AwaResourceOperations_WriteOnly, Client_GetResourceOperations(session_, "/5/0/1"));
    ASSERT_EQ(AwaResourceOperations_ReadWrite, Client_GetResourceOperations(session_, "/3/0/13"));
    ASSERT_EQ(AwaResourceOperations_Execute,   Client_GetResourceOperations(session_, "/5/0/2"));
}

class TestToolsOir : public TestToolsCommonWithConnectedSession {
public:
    TestToolsOir() :
        TestToolsCommonWithConnectedSession(),
        objectTarget_          ({ const_cast<char *>("/3"),      AWA_INVALID_ID }),
        objectInstanceTarget_  ({ const_cast<char *>("/3/0"),    AWA_INVALID_ID }),
        resourceTarget_        ({ const_cast<char *>("/3/0/15"), AWA_INVALID_ID }),
        resourceInstanceTarget_({ const_cast<char *>("/3/0/11"), 2 })
    {}
protected:
    Target objectTarget_;
    Target objectInstanceTarget_;
    Target resourceTarget_;
    Target resourceInstanceTarget_;
};

TEST_F(TestToolsOir, IsObjectTarget)
{
    EXPECT_TRUE(Client_IsObjectTarget(session_,  &objectTarget_));
    EXPECT_FALSE(Client_IsObjectTarget(session_, &objectInstanceTarget_));
    EXPECT_FALSE(Client_IsObjectTarget(session_, &resourceTarget_));
    EXPECT_FALSE(Client_IsObjectTarget(session_, &resourceInstanceTarget_));
}

TEST_F(TestToolsOir, IsObjectInstanceTarget)
{
    EXPECT_FALSE(Client_IsObjectInstanceTarget(session_, &objectTarget_));
    EXPECT_TRUE(Client_IsObjectInstanceTarget(session_,  &objectInstanceTarget_));
    EXPECT_FALSE(Client_IsObjectInstanceTarget(session_, &resourceTarget_));
    EXPECT_FALSE(Client_IsObjectInstanceTarget(session_, &resourceInstanceTarget_));
}

TEST_F(TestToolsOir, IsResourceTarget)
{
    EXPECT_FALSE(Client_IsResourceTarget(session_, &objectTarget_));
    EXPECT_FALSE(Client_IsResourceTarget(session_, &objectInstanceTarget_));
    EXPECT_TRUE(Client_IsResourceTarget(session_,  &resourceTarget_));
    EXPECT_FALSE(Client_IsResourceTarget(session_, &resourceInstanceTarget_));
}

TEST_F(TestToolsOir, IsResourceInstanceTarget)
{
    EXPECT_FALSE(Client_IsResourceInstanceTarget(session_, &objectTarget_));
    EXPECT_FALSE(Client_IsResourceInstanceTarget(session_, &objectInstanceTarget_));
    EXPECT_FALSE(Client_IsResourceInstanceTarget(session_, &resourceTarget_));
    EXPECT_TRUE(Client_IsResourceInstanceTarget(session_,  &resourceInstanceTarget_));
}

class TestToolsOirInvalid : public TestToolsCommonWithConnectedSession, public ::testing::WithParamInterface<Target> {};

TEST_P(TestToolsOirInvalid, test_with_invalid_target)
{
    Target item = GetParam(); 
    EXPECT_FALSE(Client_IsObjectTarget(session_, &item));
    EXPECT_FALSE(Client_IsObjectInstanceTarget(session_, &item));
    EXPECT_FALSE(Client_IsResourceTarget(session_, &item));
    EXPECT_FALSE(Client_IsResourceInstanceTarget(session_, &item));
}

INSTANTIATE_TEST_CASE_P(
    TestToolsOirInvalid,
    TestToolsOirInvalid,
    ::testing::Values(
           Target { const_cast<char *>("/"),        0},
           Target { const_cast<char *>(""),         0},
           Target { const_cast<char *>("0"),        0},
           Target { const_cast<char *>("0/"),       0},
           Target { const_cast<char *>("/0/"),      0},
           Target { const_cast<char *>("/-1"),      0},
           Target { const_cast<char *>("/0/0"),     0},
           Target { const_cast<char *>("/0/-1"),    0},
           Target { const_cast<char *>("/0="),      0},
           Target { const_cast<char *>("/invalid"), 1},
           Target { const_cast<char *>("/3/0"),     4},
           Target { const_cast<char *>("/3"),   56345}
        ));



class TestToolsCommonWithSession: public TestClientWithSession {};

TEST_F(TestToolsCommonWithSession, Client_GetValue_handles_null_target)
{
    // test it doesn't seg-fault:
    Client_GetValue(session_, NULL, "");
}

TEST_F(TestToolsCommonWithSession, Client_GetValue_handles_null_arg)
{
    // test it doesn't seg-fault:
    Target t = { const_cast<char *>("/7/0/0"), 0 };
    Client_GetValue(session_, &t, NULL);
}

TEST_F(TestToolsCommonWithSession, Client_IsObjectTarget_handles_null_target)
{
    // test it doesn't seg-fault:
    Client_IsObjectTarget(session_, NULL);
}

TEST_F(TestToolsCommonWithSession, Client_IsObjectInstanceTarget_handles_null_target)
{
    // test it doesn't seg-fault:
    Client_IsObjectInstanceTarget(session_, NULL);
}

TEST_F(TestToolsCommonWithSession, Client_IsResourceTarget_handles_null_target)
{
    // test it doesn't seg-fault:
    Client_IsResourceTarget(session_, NULL);
}

TEST_F(TestToolsCommonWithSession, Client_IsResourceInstanceTarget_handles_null_target)
{
    // test it doesn't seg-fault:
    Client_IsResourceInstanceTarget(session_, NULL);
}


class TestToolsCommonWithServerSession: public TestServerWithSession {};

TEST_F(TestToolsCommonWithServerSession, Server_GetValue_handles_null_target)
{
    // test it doesn't seg-fault:
    Server_GetValue(session_, NULL, "");
}

TEST_F(TestToolsCommonWithServerSession, Server_GetValue_handles_null_arg)
{
    // test it doesn't seg-fault:
    Target t = { const_cast<char *>("/7/0/0"), 0 };
    Server_GetValue(session_, &t, NULL);
}

TEST_F(TestToolsCommonWithServerSession, Server_IsObjectTarget_handles_null_target)
{
    // test it doesn't seg-fault:
    Server_IsObjectTarget(session_, NULL);
}

TEST_F(TestToolsCommonWithServerSession, Server_IsObjectInstanceTarget_handles_null_target)
{
    // test it doesn't seg-fault:
    Server_IsObjectInstanceTarget(session_, NULL);
}

TEST_F(TestToolsCommonWithServerSession, Server_IsResourceTarget_handles_null_target)
{
    // test it doesn't seg-fault:
    Server_IsResourceTarget(session_, NULL);
}

TEST_F(TestToolsCommonWithServerSession, Server_IsResourceInstanceTarget_handles_null_target)
{
    // test it doesn't seg-fault:
    Server_IsResourceInstanceTarget(session_, NULL);
}


TEST_F(TestToolsCommon, GetNextTargetResourceInstanceIDFromPath_handles_null_targets)
{
    int index = 0;
    GetNextTargetResourceInstanceIDFromPath(NULL, 1, "/3/0/0", &index);
}

TEST_F(TestToolsCommon, GetNextTargetResourceInstanceIDFromPath_handles_null_path)
{
    Target t1 = { const_cast<char *>("/7/0/0"), 0 };
    Target * targets[] = { &t1 };
    int index = 0;
    GetNextTargetResourceInstanceIDFromPath(targets, 1, NULL, &index);
}

TEST_F(TestToolsCommon, GetNextTargetResourceInstanceIDFromPath_handles_null_index)
{
    Target t1 = { const_cast<char *>("/7/0/0"), 0 };
    Target * targets[] = { &t1 };
    GetNextTargetResourceInstanceIDFromPath(targets, 1, "/3/0/0", NULL);
}

TEST_F(TestToolsCommon, PrintAllObjectDefinitions_handles_null_iterator)
{
    PrintAllObjectDefinitions(NULL, OutputFormat_PlainTextVerbose);
}

TEST_F(TestToolsCommon,ResourceOperationToString_handles_valid_input)
{
    const char * output = NULL;
    std::string expectedOutput = "ReadOnly";

    output = ResourceOperationToString(AwaResourceOperations_ReadOnly);

    ASSERT_STREQ(expectedOutput.c_str(), output);
}

TEST_F(TestToolsCommon,ResourceOperationToString_handles_invalid_input)
{
    const char * output = NULL;
    std::string expectedOutput = "BAD OPERATION";


    output = ResourceOperationToString((AwaResourceOperations)-10);

    ASSERT_STREQ(expectedOutput.c_str(), output);

    output = ResourceOperationToString((AwaResourceOperations)(AwaResourceOperations_LAST+1));

    ASSERT_STREQ(expectedOutput.c_str(), output);
}

TEST_F(TestToolsCommon,ResourceTypeToString_handles_valid_input)
{
    const char * output = NULL;
    std::string expectedOutput = "Time";

    output = ResourceTypeToString(AwaResourceType_Time);

    ASSERT_STREQ(expectedOutput.c_str(), output);
}

TEST_F(TestToolsCommon,ResourceTypeToString_handles_invalid_input)
{
    const char * output = NULL;
    std::string expectedOutput = "BAD TYPE";


    output = ResourceTypeToString((AwaResourceType)-10);

    ASSERT_STREQ(expectedOutput.c_str(), output);

    output = ResourceTypeToString((AwaResourceType)(30));

    ASSERT_STREQ(expectedOutput.c_str(), output);
}


} // namespace Awa


// same namespace as Target
::std::ostream& operator<<(::std::ostream& os, const Target& target) {
  return os << "Target: path " << target.Path
            << "resourceInstanceID " << target.ResourceInstanceID;
}


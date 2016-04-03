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

#include "log.h"
#include "support/support.h"
#include "awa/types.h"

namespace Awa {

class TestLog : public TestClientBase {};

template <typename T>
class CaptureLog : public T
{
protected:
    virtual const char * Read() const {
        auto read = T::Read();

        // a prefix of '[file:lineno] ' is prepended, advance beyond this
        if ((read != nullptr) && (strlen(read) > 0))
        {
            if (strchr(read, '[') != nullptr)
            {
                auto msg = strchr(read, ']');
                if (msg != NULL && ((msg + 2) < read + strlen(read)))
                {
                    read = msg + 2;
                }
            }
        }
        return read;
    }
};

template <typename CaptureSource>
class TestLogCapture : public TestLog, public CaptureLog<CaptureSource>
{
protected:
    virtual void SetUp() {
        CaptureSource::SetUp();
        TestLog::SetUp();
    }
    virtual void TearDown() {
        TestLog::TearDown();
        CaptureSource::TearDown();
    }
};

typedef TestLogCapture<CaptureFile> TestLogCaptureFile;
typedef TestLogCapture<CaptureStdout> TestLogCaptureStdout;
typedef TestLogCapture<CaptureStderr> TestLogCaptureStderr;

TEST_F(TestLogCaptureFile, test_Log_level_None)
{
    const char * inputString = "Hello, World!";
    SetLogLevel(AwaLogLevel_None);
    Log(AwaLogLevel_Error, outFile_, "%s", inputString);
    EXPECT_STREQ("", Read());
    Log(AwaLogLevel_Warning, outFile_, "%s", inputString);
    EXPECT_STREQ("", Read());
    Log(AwaLogLevel_Verbose, outFile_, "%s", inputString);
    EXPECT_STREQ("", Read());
}

TEST_F(TestLogCaptureFile, test_Log_level_Error)
{
    const char * inputString = "Hello, World!";
    const char * expectedString = "Hello, World!\n";
    SetLogLevel(AwaLogLevel_Error);
    Log(0, outFile_, "%s", inputString);
    EXPECT_STREQ(expectedString, Read());
    Log(1, outFile_, "%s", inputString);
    EXPECT_STREQ("", Read());
    Log(2, outFile_, "%s", inputString);
    EXPECT_STREQ("", Read());
}

TEST_F(TestLogCaptureFile, test_Log_level_Warning)
{
    const char * inputString = "ABCDEFG\nHIJKL";
    const char * expectedString = "ABCDEFG\nHIJKL\n";
    SetLogLevel(1);
    Log(0, outFile_, "%s", inputString);
    EXPECT_STREQ(expectedString, Read());
    Log(1, outFile_, "%s", inputString);
    EXPECT_STREQ(expectedString, Read());
    Log(2, outFile_, "%s", inputString);
    EXPECT_STREQ("", Read());
}

TEST_F(TestLogCaptureFile, test_Log_level_Verbose)
{
    const char * inputString = "The quick brown fox blah blah blah";
    const char * expectedString = "The quick brown fox blah blah blah\n";
    SetLogLevel(2);
    Log(0, outFile_, "%s", inputString);
    EXPECT_STREQ(expectedString, Read());
    Log(1, outFile_, "%s", inputString);
    EXPECT_STREQ(expectedString, Read());
    Log(2, outFile_, "%s", inputString);
    EXPECT_STREQ(expectedString, Read());
}

TEST_F(TestLogCaptureFile, test_LogErrorWithEnum)
{
    AwaError error = LogErrorWithEnum(AwaError_ObservationInvalid); EXPECT_EQ(AwaError_ObservationInvalid, error);
    LogErrorWithEnum(AwaError_ObservationInvalid, "foobar");
    LogErrorWithEnum(AwaError_ObservationInvalid, "%d %s", 123, "hello");
}

TEST_F(TestLogCaptureStderr, test_LogError)
{
    // Errors are always logged
    SetLogLevel(0); LogError("12345"); EXPECT_STREQ("12345\n", Read());
    SetLogLevel(1); LogError("54321"); EXPECT_STREQ("54321\n", Read());
    SetLogLevel(2); LogError("67890"); EXPECT_STREQ("67890\n", Read());
}

TEST_F(TestLogCaptureStderr, test_LogWarning)
{
    // Warnings are logged on all levels except 0
    SetLogLevel(0); LogWarning("12345"); EXPECT_STREQ("", Read());
    SetLogLevel(1); LogWarning("54321"); EXPECT_STREQ("54321\n", Read());
    SetLogLevel(2); LogWarning("67890"); EXPECT_STREQ("67890\n", Read());
}

TEST_F(TestLogCaptureStdout, test_LogVerbose)
{
    // Verbose messages are logged on all levels except 0 and 1
    SetLogLevel(0); LogVerbose("12345"); EXPECT_STREQ("", Read());
    SetLogLevel(1); LogVerbose("54321"); EXPECT_STREQ("", Read());
    SetLogLevel(2); LogVerbose("67890"); EXPECT_STREQ("67890\n", Read());
}

TEST_F(TestLogCaptureStdout, test_LogDebug)
{
    // Debug messages are logged on all levels except 0, 1 and 2
    SetLogLevel(0); LogDebug("12345"); EXPECT_STREQ("", Read());
    SetLogLevel(1); LogDebug("54321"); EXPECT_STREQ("", Read());
    SetLogLevel(2); LogDebug("67890"); EXPECT_STREQ("", Read());
    SetLogLevel(3); LogDebug("09876"); EXPECT_STREQ("09876\n", Read());
}

TEST_F(TestLog, test_LogPError)
{
    errno = EADDRINUSE;
    LogPError("ABCDE")
    // expect "[filename:lineno] ABCDE: Address already in use"
}

TEST_F(TestLog, test_LogErrorWithEnum_one_argument)
{
    const char * param1 = "Bar";
    LogErrorWithEnum(AwaError_ObservationInvalid, "Foo %s", param1);
}

TEST_F(TestLog, test_LogErrorWithEnum_two_arguments)
{
    const char * param1 = "Bar";
    const char * param2 = "Baz";
    LogErrorWithEnum(AwaError_ObservationInvalid, "Foo %s %s", param1, param2);
}

AwaError ReturnLogErrorWithEnum()
{
    return LogErrorWithEnum(AwaError_Unsupported, "Expect this message!");
}

TEST_F(TestLogCaptureStderr, test_LogErrorWithEnum_with_return)
{
    SetLogLevel(0);
    ReturnLogErrorWithEnum();
    EXPECT_STREQ("AwaError_Unsupported: Expect this message!\n", Read());
}

TEST_F(TestLog, test_AwaLog_SetLevel)
{
    AwaLogLevel old = AwaLogLevel_Warning;
    AwaLog_SetLevel(AwaLogLevel_Verbose);
    old = AwaLog_SetLevel(AwaLogLevel_Error);
    EXPECT_EQ(old, AwaLogLevel_Verbose);
}

TEST_F(TestLogCaptureStderr, test_LogPError)
{
    AwaLog_SetLevel(AwaLogLevel_Error);
    errno =  EACCES;
    LogPError();
    EXPECT_STREQ("Permission denied: \n", Read());

    errno =  EACCES;
    LogPError("Test");
    EXPECT_STREQ("Permission denied: Test\n", Read());

    errno =  EAGAIN;
    int value0 = 7;
    LogPError("Message %d", value0);
    EXPECT_STREQ("Resource temporarily unavailable: Message 7\n", Read());
}

} // namespace Awa

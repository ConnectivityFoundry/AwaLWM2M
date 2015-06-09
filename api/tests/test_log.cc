#include <gtest/gtest.h>

#include "log.h"
#include "support/support.h"

namespace Awa {

class TestLog : public TestClientBase {};

class TestLogCaptureFile : public TestLog, public CaptureFile
{
protected:
    virtual void SetUp() {
        CaptureFile::SetUp();
        TestLog::SetUp();
    }
    virtual void TearDown() {
        TestLog::TearDown();
        CaptureFile::TearDown();
    }
};

class TestLogCaptureStdout : public TestLog, public CaptureStdout
{
protected:
    virtual void SetUp() {
        CaptureStdout::SetUp();
        TestLog::SetUp();
    }
    virtual void TearDown() {
        TestLog::TearDown();
        CaptureStdout::TearDown();
    }
};

class TestLogCaptureStderr : public TestLog, public CaptureStderr
{
protected:
    virtual void SetUp() {
        CaptureStderr::SetUp();
        TestLog::SetUp();
    }
    virtual void TearDown() {
        TestLog::TearDown();
        CaptureStderr::TearDown();
    }
};


TEST_F(TestLogCaptureFile, test_Log_level_0)
{
    const char * inputString = "Hello, World!";
    const char * expectedString = "Hello, World!\n";
    SetLogLevel(0);
    Log(0, outFile_, "%s", inputString);
    EXPECT_STREQ(expectedString, Read());
    Log(1, outFile_, "%s", inputString);
    EXPECT_STREQ("", Read());
    Log(2, outFile_, "%s", inputString);
    EXPECT_STREQ("", Read());
}

TEST_F(TestLogCaptureFile, test_Log_level_1)
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

TEST_F(TestLogCaptureFile, test_Log_level_2)
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

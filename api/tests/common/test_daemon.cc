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

#include <iostream>
#include <fstream>
#include <string>

#include <gtest/gtest.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "support/support.h"

namespace Awa {

class TestDaemon : public TestAwaBase {};

class TestDaemonVersion : public TestDaemon, public CaptureStdout
{
protected:
    static void SetUpTestCase() {
        expectedVersion = VERSION + std::string("\n");
    }

    virtual void SetUp() {
        CaptureStdout::SetUp();
        TestDaemon::SetUp();
    }
    virtual void TearDown() {
        TestDaemon::TearDown();
        CaptureStdout::TearDown();
    }

    void CheckOutput(const std::vector<const char *> & commandVector, const std::string & expectedOutput) {
        pid_t pid = SpawnProcess(commandVector, false, false);
        ASSERT_GT(pid, 0);

        int status = 0;
        ASSERT_EQ(pid, waitpid(pid, &status, 0));
        ASSERT_TRUE(WIFEXITED(status));

        auto output = Read();
        // because this test captures stdout, uncomment these lines to diagnose a failed test:
        //std::cerr << output << std::endl;
        //std::cerr << expectedOutput << std::endl;
        EXPECT_STREQ(expectedOutput.c_str(), output);
    }

    void CheckVersionOption(const char * daemonPath)
    {
        // suppress "deprecated conversion from string constant" warning
#pragma GCC diagnostic ignored "-Wwrite-strings"
        std::vector<const char *> commandVector { daemonPath, "--version" };
#pragma GCC diagnostic pop

        CheckOutput(commandVector, expectedVersion);
    }

    void CheckShortVersionOption(const char * daemonPath)
    {
        // suppress "deprecated conversion from string constant" warning
#pragma GCC diagnostic ignored "-Wwrite-strings"
        std::vector<const char *> commandVector { daemonPath, "-V" };
#pragma GCC diagnostic pop

        CheckOutput(commandVector, expectedVersion);
    }

    static std::string expectedVersion;
};

std::string TestDaemonVersion::expectedVersion = std::string();

TEST_F(TestDaemonVersion, bootstrap_prints_version)
{
    CheckVersionOption(global::bootstrapServerDaemonPath);
}

TEST_F(TestDaemonVersion, client_prints_version)
{
    CheckVersionOption(global::clientDaemonPath);
}

TEST_F(TestDaemonVersion, server_prints_version)
{
    CheckVersionOption(global::serverDaemonPath);
}

TEST_F(TestDaemonVersion, bootstrap_prints_version_short)
{
    CheckShortVersionOption(global::bootstrapServerDaemonPath);
}

TEST_F(TestDaemonVersion, client_prints_version_short)
{
    CheckShortVersionOption(global::clientDaemonPath);
}

TEST_F(TestDaemonVersion, server_prints_version_short)
{
    CheckShortVersionOption(global::serverDaemonPath);
}

// TODO: test that --logFile/-l option with --version/-V writes to log, not stdout.

} // namespace Awa




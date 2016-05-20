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

#ifndef SUPPORT_H
#define SUPPORT_H

/*************************************************************
 *** Awa Client & Server API Common Test Support Functions
 *************************************************************/

#include <gtest/gtest.h>
#include <unistd.h>

#include "awa/client.h"
#include "awa/server.h"

#include "daemon.h"
#include "process.h"

// Convert a preprocessor definition to a string
#define str(x) #x
#define stringify(x) str(x)

namespace Awa {

namespace defaults {
    const int logLevel = 1;
    const int timeout = 2500;          // milliseconds
    const int timeoutTolerance = 250;  // milliseconds

} // namespace defaults

namespace global {
    void SetGlobalDefaults(void);
    extern int logLevel;

} // namespace global

namespace detail {
    extern const char * NonRoutableIPv4Address;
    extern const char * NonRoutableIPv6Address;

} // namespace detail


#define CURRENT_TEST_CASE_NAME \
( ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name() )

#define CURRENT_TEST_NAME \
( ::testing::UnitTest::GetInstance()->current_test_info()->name() )

#define CURRENT_TEST_DESCRIPTION \
( std::string(CURRENT_TEST_CASE_NAME) + std::string(".") + std::string(CURRENT_TEST_NAME) )


/*********************************************************************
 *** Test Base Classes
 *********************************************************************/

// Base class for Awa tests
class TestAwaBase : public virtual testing::Test
{
protected:
    virtual void SetUp() {
        AwaLog_SetLevel(static_cast<AwaLogLevel>(global::logLevel));

        const ::testing::TestInfo* const test_info =
                ::testing::UnitTest::GetInstance()->current_test_info();
        testDescription_ = std::string(test_info->test_case_name()) + std::string(".") + std::string(test_info->name());
    }
    std::string testDescription_;
};


// Base class for Client tests
class TestClientBase : public TestAwaBase
{
};


// Base class for Client tests that require a client daemon to be spawned
class TestClientWithDaemonBase : public TestClientBase
{
public:
    TestClientWithDaemonBase() :
        TestClientBase(),
        daemon_(global::clientLocalCoapPort, global::clientIpcPort, global::clientLogFile, global::clientEndpointName) {}

protected:
  virtual void SetUp() {
      TestClientBase::SetUp();
      if (global::spawnClientDaemon)
      {
          int count = 0;
          while (IsUDPPortInUse(global::clientIpcPort) != false)
          {
              global::clientIpcPort = global::clientIpcPort < (defaults::clientIpcPort + defaults::clientIpcPortRange) ? global::clientIpcPort + 1 : defaults::clientIpcPort;
              if (++count > 2 * defaults::clientIpcPortRange)
              {
                  std::cerr << "Unable to find a usable port - exiting" << std::endl;
                  exit(-1);
              }
          }

          daemon_.SetIpcPort(global::clientIpcPort);
          ASSERT_TRUE(daemon_.Start(testDescription_));
      }
      else
      {
          daemon_.SkipStart(testDescription_);
      }
  }
  virtual void TearDown() {

      if (global::spawnClientDaemon)
      {
          // round-robin the IPC port to avoid port reuse issues during testing
          global::clientIpcPort = global::clientIpcPort < (defaults::clientIpcPort + defaults::clientIpcPortRange) ? global::clientIpcPort + 1 : defaults::clientIpcPort;
      }
      daemon_.Stop();
      TestClientBase::TearDown();
  }

private:
  AwaClientDaemon daemon_;
};


// For Client tests that require a valid session (session is not connected)
class TestClientWithSession : public TestClientWithDaemonBase
{
protected:
    virtual void SetUp() {
        TestClientWithDaemonBase::SetUp();

        // set up a valid, connected session:
        session_ = AwaClientSession_New(); ASSERT_TRUE(NULL != session_);
        ASSERT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(session_, "127.0.0.1", global::clientIpcPort));
    }

    virtual void Connect() {
        ASSERT_EQ(AwaError_Success, AwaClientSession_Connect(session_));
    }

    virtual void Disconnect() {
        AwaClientSession_Disconnect(session_);
    }

    virtual void TearDown() {
        AwaClientSession_Free(&session_);
        TestClientWithDaemonBase::TearDown();
    }

    AwaClientSession * session_;
};


// For Client tests that require a valid and connected session
class TestClientWithConnectedSession : public TestClientWithSession
{
protected:
    virtual void SetUp() {
        TestClientWithSession::SetUp();
        this->Connect();
    }

    virtual void TearDown() {
        this->Disconnect();
        TestClientWithSession::TearDown();
    }
};


// Base class for Server tests
class TestServerBase : public TestAwaBase
{
};


// Base class for Server tests that require a server daemon to be spawned
class TestServerWithDaemonBase : public TestServerBase
{
public:
    TestServerWithDaemonBase() :
        TestServerBase(),
        daemon_(global::serverCoapPort, global::serverIpcPort, global::serverLogFile) {}

protected:
  virtual void SetUp() {
      TestServerBase::SetUp();
      if (global::spawnServerDaemon)
      {
          int count = 0;
          while (IsUDPPortInUse(global::serverIpcPort) != false)
          {
              global::serverIpcPort = global::serverIpcPort < (defaults::serverIpcPort + defaults::serverIpcPortRange) ? global::serverIpcPort + 1 : defaults::serverIpcPort;
              if (++count > 2 * defaults::serverIpcPortRange)
              {
                  std::cerr << "Unable to find a usable port - exiting" << std::endl;
                  exit(-1);
              }
          }

          daemon_.SetIpcPort(global::serverIpcPort);
          ASSERT_TRUE(daemon_.Start(testDescription_));
      }
      else
      {
          daemon_.SkipStart(testDescription_);
      }
  }
  virtual void TearDown() {

      if (global::spawnServerDaemon)
      {
          // round-robin the IPC port to avoid port reuse issues during testing
          global::serverIpcPort = global::serverIpcPort < (defaults::serverIpcPort + defaults::serverIpcPortRange) ? global::serverIpcPort + 1 : defaults::serverIpcPort;
      }
      daemon_.Stop();
      TestServerBase::TearDown();
  }

private:
  AwaServerDaemon daemon_;
};


// For Server tests that require a valid session (session is not connected)
class TestServerWithSession : public TestServerWithDaemonBase
{
protected:
    virtual void SetUp() {
        TestServerWithDaemonBase::SetUp();

        // set up a valid, connected session:
        session_ = AwaServerSession_New(); ASSERT_TRUE(NULL != session_);
        ASSERT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(session_, "127.0.0.1", global::serverIpcPort));
    }

    virtual void Connect() {
        ASSERT_EQ(AwaError_Success, AwaServerSession_Connect(session_));
    }

    virtual void Disconnect() {
        AwaServerSession_Disconnect(session_);
    }

    virtual void TearDown() {
        AwaServerSession_Free(&session_);
        TestServerWithDaemonBase::TearDown();
    }

    AwaServerSession * session_;
};


// For Server tests that require a valid and connected session
class TestServerWithConnectedSession : public TestServerWithSession
{
protected:
    virtual void SetUp() {
        TestServerWithSession::SetUp();
        this->Connect();
    }

    virtual void TearDown() {
        this->Disconnect();
        TestServerWithSession::TearDown();
    }
};


// Base class for tests that require both a server and client daemon to be spawned
// Note: no sessions are created.
class TestServerAndClientWithDaemonBase : public TestServerWithDaemonBase, TestClientWithDaemonBase
{
protected:
  virtual void SetUp() {
      TestServerWithDaemonBase::SetUp();
      TestClientWithDaemonBase::SetUp();
  }
  virtual void TearDown() {
      TestClientWithDaemonBase::TearDown();
      TestServerWithDaemonBase::TearDown();
  }
};


// For tests that require both a server and client daemon to be spawned, with
// a valid session for each. The sessions are not connected.
class TestServerAndClientWithSession : public TestServerAndClientWithDaemonBase
{
protected:
    virtual void SetUp() {
        TestServerAndClientWithDaemonBase::SetUp();

        // set up valid, connected sessions:
        server_session_ = AwaServerSession_New(); ASSERT_TRUE(NULL != server_session_);
        ASSERT_EQ(AwaError_Success, AwaServerSession_SetIPCAsUDP(server_session_, "127.0.0.1", global::serverIpcPort));

        client_session_ = AwaClientSession_New(); ASSERT_TRUE(NULL != client_session_);
        ASSERT_EQ(AwaError_Success, AwaClientSession_SetIPCAsUDP(client_session_, "127.0.0.1", global::clientIpcPort));
    }

    virtual void Connect() {
        ASSERT_EQ(AwaError_Success, AwaServerSession_Connect(server_session_));
        ASSERT_EQ(AwaError_Success, AwaClientSession_Connect(client_session_));
    }

    virtual void Disconnect() {
        AwaClientSession_Disconnect(client_session_);
        AwaServerSession_Disconnect(server_session_);
    }

    virtual void TearDown() {
        AwaServerSession_Free(&server_session_);
        AwaClientSession_Free(&client_session_);
        TestServerAndClientWithDaemonBase::TearDown();
    }

    AwaServerSession * server_session_;
    AwaClientSession * client_session_;
};


// For tests that require both a server and client daemon to be spawned, with
// a valid session for each. The sessions are automatically connected.
class TestServerAndClientWithConnectedSession : public TestServerAndClientWithSession
{
protected:
    virtual void SetUp() {
        TestServerAndClientWithSession::SetUp();
        this->Connect();
    }

    virtual void TearDown() {
        this->Disconnect();
        TestServerAndClientWithSession::TearDown();
    }

    void WaitForClientDefinition(int objectID)
    {
        bool found = false;
        int maxOperations = 30;

        char objectPath[32];
        sprintf(objectPath, "/%d", objectID);

        printf("Waiting for %s\n", objectPath);

        while (!found && maxOperations-- > 0)
        {
            AwaServerListClientsOperation * listClientsOperation = AwaServerListClientsOperation_New(server_session_);
            AwaServerListClientsOperation_Perform(listClientsOperation, defaults::timeout);
            const AwaServerListClientsResponse * response = AwaServerListClientsOperation_GetResponse(listClientsOperation, global::clientEndpointName);

            AwaRegisteredEntityIterator * iterator = AwaServerListClientsResponse_NewRegisteredEntityIterator(response);

            while (AwaRegisteredEntityIterator_Next(iterator))
            {
                //Lwm2m_Debug("Waiting for server to know client knows about object 1000...");
                const char * path = AwaRegisteredEntityIterator_GetPath(iterator);

                if (strstr(path, objectPath) != NULL) {
                    // contains
                    printf("FOUND %s\n", path);
                    found = true;
                }
            }
            AwaRegisteredEntityIterator_Free(&iterator);
            AwaServerListClientsOperation_Free(&listClientsOperation);
            sleep(1);
        }
        ASSERT_TRUE(found);
    }
};


// For tests that require both a Server and Client daemon to be spawned, with
// a valid and connected session for each.
// A set of objects and resources that may be useful for testing are pre-defined.
class TestServerAndClientWithConnectedSessionWithDummyObjects  : public TestServerAndClientWithConnectedSession
{
protected:
    virtual void SetUp() {
        TestServerAndClientWithConnectedSession::SetUp();

        // create some custom objects
        AwaClientDefineOperation * clientDefineOperation = AwaClientDefineOperation_New(client_session_);
        EXPECT_TRUE(NULL != clientDefineOperation);
        AwaServerDefineOperation * serverDefineOperation = AwaServerDefineOperation_New(server_session_);
        EXPECT_TRUE(NULL != serverDefineOperation);

        AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(TEST_OBJECT_ID_, "Test Object", 1, 1);
        EXPECT_TRUE(NULL != objectDefinition);

        AwaObjectLink dummyObjectLink {3, 7};

        char dummyOpaqueData[] = {'h', 'e', 'l', 'l', 'o', 0, 'x', '\0', 123};
        AwaOpaque dummyOpaque {(void*) dummyOpaqueData, sizeof(dummyOpaqueData)};

        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsNoType(objectDefinition, 0, "Test None Resource", false, AwaResourceOperations_Execute));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, 1, "Test Mandatory String Resource", true, AwaResourceOperations_ReadWrite, "DefaultValue"));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefinition, 2, "Test Mandatory Integer Resource", true, AwaResourceOperations_ReadWrite, 12345));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloat(objectDefinition, 3, "Test Mandatory Float Resource", true, AwaResourceOperations_ReadWrite, 12345.54321));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBoolean(objectDefinition, 4, "Test Mandatory Boolean Resource", true, AwaResourceOperations_ReadWrite, true));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque(objectDefinition, 5, "Test Mandatory Opaque Resource", true, AwaResourceOperations_ReadWrite, dummyOpaque));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTime(objectDefinition, 6, "Test Mandatory Time Resource", true, AwaResourceOperations_ReadWrite, 123456789));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLink(objectDefinition, 7, "Test Mandatory ObjectLink Resource", true, AwaResourceOperations_ReadWrite, dummyObjectLink));

        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, 101, "Test Optional String Resource", false, AwaResourceOperations_ReadWrite, "DefaultValue"));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefinition, 102, "Test Optional Integer Resource", false, AwaResourceOperations_ReadWrite, 12345));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloat(objectDefinition, 103, "Test Optional Float Resource", false, AwaResourceOperations_ReadWrite, 12345.54321));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBoolean(objectDefinition, 104, "Test Optional Boolean Resource", false, AwaResourceOperations_ReadWrite, true));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque(objectDefinition, 105, "Test Optional Opaque Resource", false, AwaResourceOperations_ReadWrite, dummyOpaque));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTime(objectDefinition, 106, "Test Optional Time Resource", false, AwaResourceOperations_ReadWrite, 123456789));
        EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLink(objectDefinition, 107, "Test Optional ObjectLink Resource", false, AwaResourceOperations_ReadWrite, dummyObjectLink));

        EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(clientDefineOperation, objectDefinition));
        EXPECT_EQ(AwaError_Success, AwaServerDefineOperation_Add(serverDefineOperation, objectDefinition));

        ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(clientDefineOperation, defaults::timeout));
        ASSERT_EQ(AwaError_Success, AwaServerDefineOperation_Perform(serverDefineOperation, defaults::timeout));

        //FIXME: FLOWDM-498: this creates the object instance for the dummy object on the client as the server api does not support object/resource creation yet
        //comment this code to find tests that aren't LWM2M compliant
        AwaClientSetOperation * clientSet = AwaClientSetOperation_New(client_session_);
        EXPECT_TRUE(clientSet != NULL);
        EXPECT_EQ(AwaError_Success, AwaClientSetOperation_CreateObjectInstance(clientSet, "/10000/0"));
        EXPECT_EQ(AwaError_Success, AwaClientSetOperation_Perform(clientSet, defaults::timeout));
        AwaClientSetOperation_Free(&clientSet);

        AwaObjectDefinition_Free(&objectDefinition);

        AwaClientDefineOperation_Free(&clientDefineOperation);
        AwaServerDefineOperation_Free(&serverDefineOperation);
    }
    virtual void TearDown()
    {
        TestServerAndClientWithConnectedSession::TearDown();
    }

    const int TEST_OBJECT_ID_ = 10000;
};



namespace detail {

/**
 * @brief Set a file descriptor to blocking or non-blocking mode.
 * http://code.activestate.com/recipes/577384-setting-a-file-descriptor-to-blocking-or-non-block/
 *
 * @param fd The file descriptor
 * @param blocking 0:non-blocking mode, 1:blocking mode
 *
 * @return 0:success, -1:failure.
 **/
static int fd_set_blocking(int fd, int blocking) {
    /* Save the current flags */
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl");
        return -1;
    }

    if (blocking)
        flags &= ~O_NONBLOCK;
    else
        flags |= O_NONBLOCK;

    if (fcntl(fd, F_SETFL, flags) == -1)
    {
        perror("fcntl");
        return -1;
    }
    return 0;
}

const size_t BUF_SIZE = 4096;

static void null_terminate_buffer(char * buffer, int rc, size_t bufSize)
{
    // if rc is valid, null-terminate buffer after rc bytes read
    if (rc > 0)
    {
        if (rc < static_cast<int>(bufSize))
        {
            buffer[rc] = 0;
        }
        else
        {
            // unless bytes fill buffer entirely, then truncate
            buffer[bufSize - 1] = 0;
        }
    }
    else
    {
        buffer[0] = 0;
    }
}

} // namespace detail

// A test utility class for capturing output from a file
class CaptureFile
{
public:
  CaptureFile() : outFile_(nullptr), pipe_fd_(), buffer_(nullptr) {}
protected:
  virtual void SetUp() {
      // Create an unnamed pipe
      if (pipe(pipe_fd_) == -1)
      {
          perror("pipe");
          return;
      }
      // non-blocking read
      if (detail::fd_set_blocking(pipe_fd_[0], 0) != 0)
      {
          goto error;
      }
      // non-blocking write
      if (detail::fd_set_blocking(pipe_fd_[1], 0) != 0)
      {
          goto error;
      }

      outFile_ = fdopen(pipe_fd_[1], "w");
      if (outFile_ == NULL)
      {
          perror("fdopen");
          goto error;
      }
      buffer_ = new char[detail::BUF_SIZE]();
      return;

  error:
      close(pipe_fd_[0]);
      close(pipe_fd_[1]);
  }
  virtual void TearDown() {
      fclose(outFile_);
      close(pipe_fd_[0]);
      close(pipe_fd_[1]);
      delete [] buffer_;
      buffer_ = nullptr;
  }
  const char * Read() const {
      // non-blocking
      int rc = read(pipe_fd_[0], buffer_, detail::BUF_SIZE);
      detail::null_terminate_buffer(buffer_, rc, detail::BUF_SIZE);
      return buffer_;
  }

  FILE * outFile_;
private:
  int pipe_fd_[2];  // index 0 is read, 1 is write
  char * buffer_;
};

// A test utility class for capturing output from a C stream
class CaptureStream
{
protected:
  explicit CaptureStream(FILE * stream) : stream_(stream), pipe_fd_(), saved_stdout_fd_(), buffer_(nullptr) {}
  virtual void SetUp() {
      int streamFd = fileno(stream_);
      // create an unnamed pipe
      if (pipe(pipe_fd_) == -1)
      {
          perror("pipe");
          return;
      }

      // non-blocking read
      if (detail::fd_set_blocking(pipe_fd_[0], 0) != 0)
      {
          goto error;
      }

      // non-blocking write
      if (detail::fd_set_blocking(pipe_fd_[1], 0) != 0)
      {
          goto error;
      }

      // save stream and redirect to pipe
      fflush(stream_);
      saved_stdout_fd_ = dup(streamFd);
      if (saved_stdout_fd_ == -1)
      {
          perror("dup");
          goto error;
      }
      if (dup2(pipe_fd_[1], streamFd) == -1)
      {
          perror("dup2");
          goto error;
      }
      buffer_ = new char[detail::BUF_SIZE]();
      return;

  error:
      close(pipe_fd_[0]);
      close(pipe_fd_[1]);
  }
  virtual void TearDown() {
      // ignore errors, do as much cleanup as possible
      close(pipe_fd_[0]);
      close(pipe_fd_[1]);
      delete [] buffer_;
      buffer_ = nullptr;
      // restore stream
      dup2(saved_stdout_fd_, fileno(stream_));
      close(saved_stdout_fd_);
  }
  virtual const char * Read() const {
      // non-blocking
      int rc = read(pipe_fd_[0], buffer_, detail::BUF_SIZE);
      detail::null_terminate_buffer(buffer_, rc, detail::BUF_SIZE);
      return buffer_;
  }
private:
  FILE * stream_;
  int pipe_fd_[2];  // index 0 is read, 1 is write
  int saved_stdout_fd_;
  char * buffer_;
};

// A test utility class for capturing output from C stdout
class CaptureStdout : public CaptureStream
{
public:
    CaptureStdout() : CaptureStream(stdout) {};
};

// A test utility class for capturing output from C stderr
class CaptureStderr : public CaptureStream
{
public:
    CaptureStderr() : CaptureStream(stderr) {};
};

// A test utility class for basic time measurement
class BasicTimer
{
public:
    BasicTimer()
    {
        memset(&tval_start, 0, sizeof(struct timeval));
        memset(&tval_stop, 0, sizeof(struct timeval));
        running = false;
    }

    virtual ~BasicTimer() {}

    void Start()
    {
        if (!running)
        {
            gettimeofday(&tval_start, NULL);
            running = true;
        }
    }

    void Stop()
    {
        if (running)
        {
            gettimeofday(&tval_stop, NULL);
            running = false;
        }
    }

    double TimeElapsed_Seconds()
    {
        double result = 0;
        if (!running)
        {
            struct timeval tval_elapsed;
            timersub(&tval_stop, &tval_start, &tval_elapsed);
            return tval_elapsed.tv_sec + tval_elapsed.tv_usec * 1e-6;
        }
        return result;
    }

    double TimeElapsed_Milliseconds()
    {
        return TimeElapsed_Seconds() * 1000;
    }

protected:
    struct timeval tval_start;
    struct timeval tval_stop;
    bool running;
};

bool ElapsedTimeWithinTolerance(double time_ms, double time_target_ms, double tolerance_ms);


// Poll an overridden Check function until it returns true, or the timeout is reached.
class WaitCondition
{
public:
    WaitCondition(useconds_t checkPeriod=1e6,
                  useconds_t timeoutDuration=1e7) :
        checkPeriod_(checkPeriod), timeoutDuration_(timeoutDuration) {}
    virtual ~WaitCondition() {}
    virtual bool Wait()
    {
        const int maxCount = timeoutDuration_ / checkPeriod_;
        int count = 0;
        while (!Check())
        {
            if (++count > maxCount)
                break;
            //std::cout << "Wait " << count * checkPeriod_ / 1000 << "ms" << std::endl;
            usleep(checkPeriod_);
        }
        if (count > maxCount)
        {
            std::cerr << "Wait Condition timed out after " << timeoutDuration_ / 1000000.0 << " seconds." << std::endl;
        }
        return count <= maxCount;
    }
    virtual bool Check() = 0;
protected:
    useconds_t checkPeriod_;
    useconds_t timeoutDuration_;
};

class PollCondition
{
public:
    explicit PollCondition(int maxCount = 10) : pollCount(0), pollMaxCount(maxCount) {}

    virtual ~PollCondition() {}

    virtual void Reset()
    {
        pollCount = 0;
    }

    virtual bool Wait()
    {
        while(++pollCount < pollMaxCount)
        {
            if (Check())
            {
                break;
            }
        }

        return pollCount < pollMaxCount;
    }

    virtual bool Check() = 0;
protected:
    int pollCount;
    int pollMaxCount;
};

} // namespace Awa

#endif // SUPPORT_H

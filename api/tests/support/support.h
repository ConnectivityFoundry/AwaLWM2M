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

#include <iostream>
#include <vector>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <cstdio>
#include <memory>
#include <gtest/gtest.h>

#include "awa/client.h"
#include "awa/server.h"

// Convert a preprocessor definition to a string
#define str(x) #x
#define stringify(x) str(x)

namespace Awa {

namespace defaults {
    const int logLevel = 1;
    const int clientIpcPort = 57701;
    const int serverIpcPort = 58701;
    const int clientIpcPortRange = 50;  // round-robin client IPC port from clientIpcPort to clientIpcPort + clientIpcPortRange (exclusive)
    const int serverIpcPortRange = 50;  // round-robin server IPC port from serverIpcPort to serverIpcPort + serverIpcPortRange (exclusive)

    // FIXME: The tests should be smarter - rather than using a pre-assigned port range and hoping nothing is using a given port,
    // Test that the port is OK before creating daemons, etc. If the port cannot be bound, it's still in use and we should try
    // another one.

    const int clientLocalCoapPort = 6002;
    const int serverCoapPort = 6001;
    const int timeout = 2500;         // milliseconds
    const int timeoutTolerance = 100;  // milliseconds
    const char * const clientEndpointName = "TestIMG1";
    const char * const clientLogFile = "awa_clientd.log";
    const char * const serverLogFile = "awa_serverd.log";

    // assume binaries are run from directory 'build'
    const char * const coapClientPath      = "lib/libcoap/examples/coap-client";
    const char * const clientDaemonPath    = "core/src/client/awa_clientd";
    const char * const serverDaemonPath    = "core/src/server/awa_serverd";
    const char * const bootstrapDaemonPath = "core/src/bootstrap/awa_bootstrapd";
    const char * const bootstrapConfig     = "../api/tests/bootstrap-gtest.config";

} // namespace defaults

namespace global {

    void SetGlobalDefaults(void);

    extern int logLevel;
    extern int clientIpcPort;
    extern int serverIpcPort;
    extern int clientLocalCoapPort;
    extern int serverCoapPort;
    extern bool spawnClientDaemon;
    extern bool spawnServerDaemon;
    extern const char * coapClientPath;
    extern const char * clientDaemonPath;
    extern const char * serverDaemonPath;
    extern const char * bootstrapDaemonPath;
    extern const char * bootstrapConfig;
    extern const char * clientEndpointName;
    extern const char * clientLogFile;
    extern const char * serverLogFile;

} // namespace global

namespace detail {
    extern const char * NonRoutableIPv4Address;
    extern const char * NonRoutableIPv6Address;
} // namespace detail

// Spawn a new child process, arguments specified by a null-terminated commandVector.
// Note that the type of the vector must be char *, not const char *.
// If true, wait specifies that the parent should wait for the child to terminate.
// If true, silent specifies that the child's stdout will be redirected to /dev/null.
pid_t SpawnProcess(std::vector<char *> &commandVector, bool wait, bool silent);

// Kill an existing process, with SIGKILL.
void KillProcess(pid_t pid);

// Terminate an existing process, with SIGTERM.
void TerminateProcess(pid_t pid);

// Perform a CoAP operation on the specified resource, after a short delay.
pid_t CoAPOperation(int port, const char * method, const char * resource, int delay /*microseconds*/);

// Send a request to the specified IPC port, wait for response. Return 0 on success, -1 on error or timeout
int WaitForIpc(int ipcPort, int timeout /*seconds*/, const char * request, size_t requestLen);

// RAII CBuffer - free memory automatically when instance falls out of scope
class CBuffer
{
public:
    explicit CBuffer(size_t size) : ptr_(nullptr), size_(size) { ptr_ = malloc(size_); }
    ~CBuffer() { free(ptr_); ptr_ = nullptr; size_ = 0; }

    size_t len() const { return size_; }
    void * buf() const { return ptr_; }

    CBuffer(CBuffer const &) = delete;
    CBuffer & operator=(CBuffer const &x) = delete;

private:
    void * ptr_;
    size_t size_;
};

// Start a LWM2M Client process on the specified CoAP and IPC port. Redirect output to logFile. Return process ID, or 0 if failed.
pid_t StartLWM2MClient(int coapPort, int ipcPort, const char * logFile, const char * clientID);

// Start a LWM2M Server process on the specified CoAP and IPC port. Redirect output to logFile. Return process ID.
pid_t StartLWM2MServer(int coapPort, int ipcPort, const char * logFile);


/*********************************************************************
 *** Test Base Classes
 *********************************************************************/

class Daemon
{
public:
    explicit Daemon(const char * filename="daemon.log") : pid_(0), log_(filename, std::ios::out | std::ios::app ) {}
    virtual ~Daemon() {}
    virtual bool Start(const std::string & logMessage) = 0;
    virtual void Stop() = 0;

protected:
    pid_t pid_;
    std::ofstream log_;
};

class LWM2MClientDaemon : public Daemon
{
public:
  LWM2MClientDaemon() : Daemon(),
      coapPort_(0), ipcPort_(0), logFile_(), endpointName_("") {}
  virtual ~LWM2MClientDaemon() {}

  virtual bool Start(const std::string & logMessage)
  {
      // pick up the defaults, if not explicitly set
      ipcPort_ = ipcPort_ == 0 ? global::clientIpcPort : ipcPort_;
      coapPort_ = coapPort_ == 0 ? global::clientLocalCoapPort : coapPort_;
      logFile_ = logFile_.empty() ? global::clientLogFile : logFile_;
      endpointName_ = endpointName_.empty() ? global::clientEndpointName : endpointName_;

      if (global::spawnClientDaemon)
      {
          pid_ = StartLWM2MClient(coapPort_, ipcPort_, logFile_.c_str(), endpointName_.c_str());
          log_ << "Spawned LWM2M Client: "
                  << "pid " << pid_
                  << ", ID " << endpointName_
                  << ", Local CoAP port " << coapPort_
                  << ", IPC port " << ipcPort_
                  << ", Bootstrap config " << global::bootstrapConfig
                  << ", logging to " << logFile_
                  << ", " << logMessage << std::endl;
      }
      else
      {
          log_ << "Not spawning LWM2M Client: "
                  << "CoAP port " << coapPort_
                  << ", IPC port " << ipcPort_ << std::endl;
          pid_ = 0;
      }
      return pid_ >= 0;
  }
  virtual void Stop()
  {
      if (global::spawnClientDaemon)
      {
          // round-robin the IPC port to avoid port reuse issues during testing
          global::clientIpcPort = global::clientIpcPort < (defaults::clientIpcPort + defaults::clientIpcPortRange) ? global::clientIpcPort + 1 : defaults::clientIpcPort;
      }
      if (pid_ > 0)
      {
          // use SIGTERM so that valgrind can terminate correctly and write log
          log_ << "Terminating LWM2M Client: pid " << pid_ << std::endl;
          TerminateProcess(pid_);
          pid_ = 0;
      }
  }
  void SetCoapPort(int port)
  {
      coapPort_ = port;
  }
  void SetIpcPort(int port)
  {
      ipcPort_ = port;
  }
  void SetLogFile(const std::string & logFile)
  {
      logFile_ = logFile;
  }
  void SetEndpointName(const std::string & endpointName)
  {
      endpointName_ = endpointName;
  }

private:
  int coapPort_;
  int ipcPort_;
  std::string logFile_;
  std::string endpointName_;
};

class LWM2MServerDaemon : public Daemon
{
public:
  LWM2MServerDaemon() : Daemon(),
      coapPort_(0), ipcPort_(0), logFile_() {}
  virtual ~LWM2MServerDaemon() {}
  virtual bool Start(const std::string & logMessage)
  {
      // pick up the defaults, if not explicitly set
      ipcPort_ = ipcPort_ == 0 ? global::serverIpcPort : ipcPort_;
      coapPort_ = coapPort_ == 0 ? global::serverCoapPort : coapPort_;
      logFile_ = logFile_.empty() ? global::serverLogFile : logFile_;

      if (global::spawnServerDaemon)
      {
          pid_ = StartLWM2MServer(coapPort_, ipcPort_, logFile_.c_str());
          log_ << "Spawned LWM2M Server: "
                  << "pid " << pid_
                  << ", CoAP port " << coapPort_
                  << ", IPC port " << ipcPort_
                  << ", logging to " << logFile_
                  << ", " << logMessage << std::endl;
      }
      else
      {
          log_ << "Not spawning LWM2M Server: "
                  << "CoAP port " << coapPort_
                  << ", IPC port " << ipcPort_ << std::endl;
          pid_ = 0;
      }
      return pid_ >= 0;
  }
  virtual void Stop()
  {
      // round-robin the IPC port to avoid port reuse issues during testing
      if (global::spawnServerDaemon)
      {
          global::serverIpcPort = global::serverIpcPort < (defaults::serverIpcPort + defaults::serverIpcPortRange) ? global::serverIpcPort + 1 : defaults::serverIpcPort;
      }
      if (pid_ > 0)
      {
          // use SIGTERM so that valgrind can terminate correctly and write log
          log_ << "Terminating LWM2M Server: pid " << pid_ << std::endl;
          TerminateProcess(pid_);
          pid_ = 0;
      }
  }
  void SetCoapPort(int port)
  {
      coapPort_ = port;
  }
  void SetIpcPort(int port)
  {
      ipcPort_ = port;
  }
  void SetLogFile(const std::string & logFile)
  {
      logFile_ = logFile;
  }

private:
  int coapPort_;
  int ipcPort_;
  std::string logFile_;
};

class LWM2MClientDaemonHorde
{
public:
    LWM2MClientDaemonHorde(std::vector<std::string> clientIDs,
                           int startPort, std::string testDescription) :
      clients_(), clientIDs_(clientIDs), startPort_(startPort)
    {
        for (auto it = clientIDs_.begin(); it != clientIDs_.end(); ++it)
        {
            DaemonPtr p(new LWM2MClientDaemon);
            p->SetEndpointName(*it);
            p->SetIpcPort(startPort++);
            p->Start(std::string(*it) + std::string(" : ") + testDescription);
            clients_.push_back(std::move(p));
        }
    }
    ~LWM2MClientDaemonHorde()
    {
        for (auto it = clients_.begin(); it != clients_.end(); ++it)
        {
            (*it)->Stop();
        }
    }
    const std::vector<std::string> & GetClientIDs() const
    {
        return clientIDs_;
    }

private:
    typedef std::unique_ptr<LWM2MClientDaemon> DaemonPtr;
    std::vector<DaemonPtr> clients_;
    std::vector<std::string> clientIDs_;
    int startPort_;
};

#define CURRENT_TEST_CASE_NAME \
( ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name() )

#define CURRENT_TEST_NAME \
( ::testing::UnitTest::GetInstance()->current_test_info()->name() )

#define CURRENT_TEST_DESCRIPTION \
( std::string(CURRENT_TEST_CASE_NAME) + std::string(".") + std::string(CURRENT_TEST_NAME) )


// Base class for Awa tests
class TestAwaBase : public testing::Test
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
protected:
  virtual void SetUp() {
      TestClientBase::SetUp();
      ASSERT_TRUE(daemon_.Start(testDescription_));
  }
  virtual void TearDown() {
      daemon_.Stop();
      TestClientBase::TearDown();
  }

private:
  LWM2MClientDaemon daemon_;
};

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
protected:
  virtual void SetUp() {
      TestServerBase::SetUp();
      ASSERT_TRUE(daemon_.Start(testDescription_));
  }
  virtual void TearDown() {
      daemon_.Stop();
      TestServerBase::TearDown();
  }

private:
  LWM2MServerDaemon daemon_;
};

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

// Base class for tests that require a server and client daemon to be spawned
class TestServerAndClientWithDaemonBase : public TestAwaBase
{
protected:
  virtual void SetUp() {
      TestAwaBase::SetUp();
      ASSERT_TRUE(server_daemon_.Start(testDescription_));
      ASSERT_TRUE(client_daemon_.Start(testDescription_));
  }
  virtual void TearDown() {
      client_daemon_.Stop();
      server_daemon_.Stop();
      TestAwaBase::TearDown();
  }

private:
  LWM2MServerDaemon server_daemon_;
  LWM2MClientDaemon client_daemon_;
};

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
 * Set a file descriptor to blocking or non-blocking mode.
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

const size_t BUF_SIZE = 256;

} // namespace detail

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
      if (rc == 0 || rc == -1) {
          buffer_[0] = 0;
      }

      if (strlen(buffer_) > 0)
      {
          // a prefix of '[file:lineno] ' is prepended, advance beyond this
          char * result = strchr(buffer_, ']') + 2;
          return ((result != NULL) && (result <= (buffer_ + strlen(buffer_)))) ? result : buffer_;
      }
      else
      {
          return buffer_;
      }
  }

  FILE * outFile_;
private:
  int pipe_fd_[2];  // index 0 is read, 1 is write
  char * buffer_;
};

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
      if (rc == 0 || rc == -1) {
          buffer_[0] = 0;
      }

      if (strlen(buffer_) > 0)
      {
          // a prefix of '[file:lineno] ' is prepended, advance beyond this
          char * result = strchr(buffer_, ']') + 2;
          return ((result != NULL) && (result <= (buffer_ + strlen(buffer_)))) ? result : buffer_;
      }
      else
      {
          return buffer_;
      }
  }
private:
  FILE * stream_;
  int pipe_fd_[2];  // index 0 is read, 1 is write
  int saved_stdout_fd_;
  char * buffer_;
};

class CaptureStdout : public CaptureStream
{
public:
    CaptureStdout() : CaptureStream(stdout) {};
};

class CaptureStderr : public CaptureStream
{
public:
    CaptureStderr() : CaptureStream(stderr) {};
};

// Basic timing utility class
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

} // namespace Awa

#endif // SUPPORT_H

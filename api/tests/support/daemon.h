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

#ifndef DAEMON_H
#define DAEMON_H

#include <iostream>
#include <fstream>
#include <memory>
#include <string>

#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#include "process.h"
#include "log.h"
#include "file_resource.h"

namespace Awa {

namespace defaults {

const int clientIpcPort = 57701;
const int serverIpcPort = 58701;
const int clientIpcPortRange = 50;  // round-robin client IPC port from clientIpcPort to clientIpcPort + clientIpcPortRange (exclusive)
const int serverIpcPortRange = 50;  // round-robin server IPC port from serverIpcPort to serverIpcPort + serverIpcPortRange (exclusive)

// FIXME: The tests should be smarter - rather than using a pre-assigned port range and hoping nothing is using a given port,
// test that the port is OK before creating daemons.
// If the port cannot be bound, it's still in use and we should try another one.

const int clientLocalCoapPort = 6002;
const int serverCoapPort = 6001;
const int bootstrapServerCoapPort = 15685;
const char * const clientEndpointName = "TestIMG1";
const char * const clientLogFile = "awa_clientd.log";
const char * const serverLogFile = "awa_serverd.log";
const char * const bootstrapServerLogFile = "awa_bootstrapd.log";

// assume binaries are run from directory 'build'
const char * const coapClientPath      = "lib/libcoap/examples/coap-client";
const char * const clientDaemonPath    = "core/src/client/awa_clientd";
const char * const serverDaemonPath    = "core/src/server/awa_serverd";
const char * const bootstrapServerDaemonPath = "core/src/bootstrap/awa_bootstrapd";
const char * const bootstrapServerConfig     = "../api/tests/gtest.bsc";
const char * const objectDefinitionsFile     = "../api/tests/object-defs-gtest.xml";

} // namespace defaults

namespace global {

void SetDaemonGlobalDefaults(void);

extern int clientIpcPort;
extern int serverIpcPort;
extern int clientLocalCoapPort;
extern int serverCoapPort;
extern int bootstrapServerCoapPort;
extern bool spawnClientDaemon;
extern bool spawnServerDaemon;
extern bool spawnBootstrapServerDaemon;
extern const char * coapClientPath;
extern const char * clientDaemonPath;
extern const char * serverDaemonPath;
extern const char * bootstrapServerDaemonPath;
extern const char * bootstrapServerConfig;
extern const char * clientEndpointName;
extern const char * clientLogFile;
extern const char * serverLogFile;
extern const char * bootstrapServerLogFile;
extern const char * objectDefinitionsFile;
extern int timeout;

} // namespace global

class Daemon
{
public:
    explicit Daemon() : pid_(0) {}
    virtual ~Daemon()
    {
        if (pid_ > 0)
        {
            TerminateProcess(pid_);
            KillProcess(pid_);
            pid_ = 0;
        }
    }
    virtual bool Start() = 0;
    virtual void Stop() = 0;

    virtual void Pause()
    {
        PauseProcess(pid_);
    }
    virtual void Unpause()
    {
        UnpauseProcess(pid_);
    }

protected:

    pid_t pid_;
};

class AwaClientDaemon : public Daemon
{
public:
    AwaClientDaemon() :
        Daemon(),
        coapPort_(global::clientLocalCoapPort),
        ipcPort_(global::clientIpcPort),
        logFile_(global::clientLogFile),
        endpointName_(global::clientEndpointName),
        bootstrapConfig_(global::bootstrapServerConfig),
        bootstrapURI_(),
        objectDefinitionsFile_(global::objectDefinitionsFile),
        additionalOptions_() {}
    AwaClientDaemon(int coapPort, int ipcPort, std::string logFile, std::string endpointName) :
        Daemon(),
        coapPort_(coapPort),
        ipcPort_(ipcPort),
        logFile_(logFile),
        endpointName_(endpointName),
        bootstrapURI_(),
        objectDefinitionsFile_(global::objectDefinitionsFile),
        additionalOptions_() {}
    virtual ~AwaClientDaemon() {}

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
    void SetBootstrapConfig(const std::string & bootstrapConfig)
    {
        bootstrapConfig_ = bootstrapConfig;
    }
    void SetBootstrapURI(const std::string & bootstrapURI)
    {
        bootstrapURI_ = bootstrapURI;
    }
    void SetAdditionalOptions(const std::vector<std::string> & options)
    {
        additionalOptions_ = options;
    }
    virtual bool Start()
    {
        pid_ = StartAwaClient(global::clientDaemonPath, coapPort_, ipcPort_, logFile_.c_str(), endpointName_.c_str(), global::bootstrapServerConfig, bootstrapURI_.c_str(), objectDefinitionsFile_.c_str(), additionalOptions_);
        std::string bootstrapModeDescription = "Bootstrap " + std::string(bootstrapURI_.empty() ? ("config " + std::string(global::bootstrapServerConfig)) : ("URI " + std::string(bootstrapURI_)));
        global::testLog << "Spawned Awa Client: "
             << "pid " << pid_
             << ", ID " << endpointName_
             << ", Local CoAP port " << coapPort_
             << ", IPC port " << ipcPort_
             << ", " << bootstrapModeDescription
             << ", logging to " << logFile_;
        if (additionalOptions_.size() > 0)
        {
            global::testLog << ", [ ";
            for (auto x : additionalOptions_) {
                global::testLog << x << ", ";
            }
            global::testLog << "]";
        }
        global::testLog << std::endl;
        return pid_ >= 0;
    }
    virtual void SkipStart()
    {
        global::testLog << "Not spawning LWM2M Client: "
             << "CoAP port " << coapPort_
             << ", IPC port " << ipcPort_ << std::endl;
        pid_ = 0;
    }
    virtual void Stop()
    {
        if (pid_ > 0)
        {
            // use SIGTERM so that valgrind can terminate correctly and write log
            global::testLog << "Terminating Awa Client: pid " << pid_ << std::endl;
            TerminateProcess(pid_);
            pid_ = 0;
        }
    }

private:
    int coapPort_;
    int ipcPort_;
    std::string logFile_;
    std::string endpointName_;
    std::string bootstrapConfig_;
    std::string bootstrapURI_;
    std::string objectDefinitionsFile_;
    std::vector<std::string> additionalOptions_;
};

class AwaServerDaemon : public Daemon
{
public:
    AwaServerDaemon() :
        Daemon(),
        coapPort_(global::serverCoapPort),
        ipcPort_(global::serverIpcPort),
        logFile_(global::serverLogFile),
        additionalOptions_() {}
    AwaServerDaemon(int coapPort, int ipcPort, std::string logFile) :
        Daemon(),
        coapPort_(coapPort),
        ipcPort_(ipcPort),
        logFile_(logFile),
        additionalOptions_() {}
    virtual ~AwaServerDaemon() {}

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
    void SetAdditionalOptions(const std::vector<std::string> & options)
    {
        additionalOptions_ = options;
    }
    virtual bool Start()
    {
        pid_ = StartAwaServer(global::serverDaemonPath, coapPort_, ipcPort_, logFile_.c_str(), additionalOptions_);
        global::testLog << "Spawned Awa Server: "
             << "pid " << pid_
             << ", CoAP port " << coapPort_
             << ", IPC port " << ipcPort_
             << ", logging to " << logFile_;
        if (additionalOptions_.size() > 0)
        {
            global::testLog << ", [ ";
            for (auto x : additionalOptions_) {
                global::testLog << x << ", ";
            }
            global::testLog << "]";
        }
        global::testLog << std::endl;
        return pid_ >= 0;
    }
    virtual void SkipStart()
    {
        global::testLog << "Not spawning Awa Server: "
             << "CoAP port " << coapPort_
             << ", IPC port " << ipcPort_ << std::endl;
        pid_ = 0;
    }
    virtual void Stop()
    {
        if (pid_ > 0)
        {
            // use SIGTERM so that valgrind can terminate correctly and write log
            global::testLog << "Terminating Awa Server: pid " << pid_ << std::endl;
            TerminateProcess(pid_);
            pid_ = 0;
        }
    }

private:
    int coapPort_;
    int ipcPort_;
    std::string logFile_;
    std::vector<std::string> additionalOptions_;
};

class AwaBootstrapServerDaemon : public Daemon
{
public:
    AwaBootstrapServerDaemon() :
        Daemon(),
        coapPort_(global::bootstrapServerCoapPort),
        configFile_(global::bootstrapServerConfig),
        logFile_(global::bootstrapServerLogFile) {}
    AwaBootstrapServerDaemon(int coapPort, std::string configFile, std::string logFile) :
        Daemon(),
        coapPort_(coapPort),
        configFile_(configFile),
        logFile_(logFile) {}
    virtual ~AwaBootstrapServerDaemon() {}

    void SetCoapPort(int port)
    {
        coapPort_ = port;
    }
    void SetConfigFile(const std::string & configFile)
    {
        configFile_ = configFile;
    }
    void SetLogFile(const std::string & logFile)
    {
        logFile_ = logFile;
    }

    virtual bool Start()
    {
        pid_ = StartAwaBootstrapServer(global::bootstrapServerDaemonPath, coapPort_, configFile_.c_str(), logFile_.c_str());
        global::testLog << "Spawned Awa Bootstrap Server: "
             << "pid " << pid_
             << ", CoAP port " << coapPort_
             << ", config file " << configFile_
             << ", logging to " << logFile_ << std::endl;
        return pid_ >= 0;
    }
    virtual void Stop()
    {
        if (pid_ > 0)
        {
            // use SIGTERM so that valgrind can terminate correctly and write log
            global::testLog << "Terminating Awa Server: pid " << pid_ << std::endl;
            TerminateProcess(pid_);
            pid_ = 0;
        }
    }

private:
    int coapPort_;
    std::string configFile_;
    std::string logFile_;
};

class BootstrapConfigFile
{
public:
    BootstrapConfigFile(std::string filename, std::string serverAddress, int serverCoapPort) : filename_(filename)
    {
        std::ofstream file(filename, std::ios::out);

        file << "ServerURI=coap://" << serverAddress << ":" << serverCoapPort << std::endl;
        file << "SecurityMode=0" << std::endl;
        file << "PublicKey=[PublicKey]" << std::endl;
        file << "SecretKey=[SecretKey]" << std::endl;
        file << "ServerID=1" << std::endl;
        file << "HoldOffTime=30" << std::endl;
        file << "ShortServerID=1" << std::endl;
        file << "Binding=U" << std::endl;
        file << "LifeTime=30" << std::endl;
        file << "DefaultMinimumPeriod=1" << std::endl;
        file << "DefaultMaximumPeriod=-1" << std::endl;
        file << "DisableTimeout=86400" << std::endl;
        file << "NotificationStoringWhenDisabledOrOffline=true" << std::endl;

        file.close();
    }
    ~BootstrapConfigFile()
    {
        if (std::remove(filename_.c_str()) < 0)
        {
            std::perror("Error deleting file");
        }
    }
    const std::string & GetFilename() const
    {
        return filename_;
    }
private:
    std::string filename_;
};

class AwaClientDaemonHorde
{
public:
    AwaClientDaemonHorde(std::vector<std::string> clientIDs, int startIpcPort) :
        clients_(), clientIDs_(clientIDs), startPort_(startIpcPort)
    {
        global::testLog << "Client Horde:" << std::endl;
        for (auto it = clientIDs_.begin(); it != clientIDs_.end(); ++it)
        {
            DaemonPtr p(new AwaClientDaemon);
            p->SetIpcPort(startIpcPort++);
            p->SetCoapPort(startIpcPort++);
            p->SetEndpointName(*it);
            global::testLog << "  - spawn " << *it << std::endl;
            p->Start();
            clients_.push_back(std::move(p));
        }
    }
    ~AwaClientDaemonHorde()
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

    void Pause()
    {
        for (auto it = clients_.begin(); it != clients_.end(); ++it)
        {
            (*it)->Pause();
        }
    }
    void Unpause()
    {
        for (auto it = clients_.begin(); it != clients_.end(); ++it)
        {
            (*it)->Unpause();
        }
    }


private:
    typedef std::unique_ptr<AwaClientDaemon> DaemonPtr;
    std::vector<DaemonPtr> clients_;
    std::vector<std::string> clientIDs_;
    int startPort_;
};

} // namespace Awa

#endif // DAEMON_H

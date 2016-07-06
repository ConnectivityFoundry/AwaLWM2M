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
#include <string>
#include <cstring>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "process.h"

namespace Awa {

pid_t SpawnProcess(const std::vector<const char *> &commandVector, bool wait_, bool silent)
{
    pid_t c_pid, pid;

#if 0  // please leave - useful for debugging
    std::cerr << "SpawnProcess: " << commandVector.size() << std::endl;
    for (auto it = commandVector.begin(); it != commandVector.end(); ++it)
    {
      std::cerr << *it << std::endl;
    }
#endif // 0

    c_pid = fork();

    if (c_pid == 0)
    {
        // child
        //std::cout << "Child: executing " << commandVector[0] << std::endl;

        FILE * fd = NULL;
        if (silent)
        {
            // redirect stdout to /dev/null
            fd = fopen("/dev/null", "at");
            dup2(fileno(fd), STDOUT_FILENO);
        }

        // convert the const char * vector to a char * array, required by execvp
        std::vector<char *> argv(commandVector.size() + 1);    // one extra for the null
        for (std::size_t i = 0; i != commandVector.size(); ++i)
        {
            argv[i] = strdup(&commandVector[i][0]);
        }

        // pass the vector's internal array to execvp - standard guarantees vector elements are stored contiguously
        execvp(argv[0], argv.data());

        // only get here if exec failed
        perror("execve");
        std::cerr << argv[0] << std::endl;
        if (fd != NULL)
        {
            fclose(fd);
        }
        _exit(1);
    }
    else if (c_pid > 0)
    {
        // parent
        if (wait_)
        {
            //std::cout << "Parent: wait\n" << std::endl;
            int status = 0;
            if ((pid = wait(&status)) < 0)
            {
                perror("wait");
                c_pid = -1;
            }
            //std::cout << "Parent: finished" << std::endl;
        }
    }
    else
    {
        perror("fork");
        c_pid = -1;
    }

    return c_pid;
}

void KillProcess(pid_t pid)
{
    // kill LWM2M Client process
    //std::cout << "Kill pid " << pid_ << std::endl;
    if (pid > 0)
    {
        if (kill(pid, SIGKILL))
        {
            perror("kill");
        }
        pid = 0;
    }
}

void TerminateProcess(pid_t pid)
{
    //std::cout << "Terminate pid " << pid_ << std::endl;
    if (pid > 0)
    {
        if (kill(pid, SIGTERM))
        {
            perror("kill");
        }

        waitpid(pid, NULL, 0);
        pid = 0;
    }
}

void PauseProcess(pid_t pid)
{
    if (pid > 0)
    {
        if (kill(pid, SIGSTOP))
        {
            perror("kill");
        }
    }
}

void UnpauseProcess(pid_t pid)
{
    if (pid > 0)
    {
        if (kill(pid, SIGCONT))
        {
            perror("kill");
        }
    }

}

pid_t CoAPOperation(const char * coapClientPath, int port, const char * method, const char * resource, int delay /*microseconds*/)
{
    // unfortunately, execvp requires char * not const char * parameters
    std::string sMethod = method;
    char * cMethod = new char[sMethod.length() + 1];
    std::strcpy(cMethod, sMethod.c_str());

    // spawn concurrent process: run "coap-client -m post coap://127.0.0.1:6001/3/0/4"
    char url[256];
    sprintf(url, "coap://127.0.0.1:%d/%s", port, resource);

#pragma GCC diagnostic ignored "-Wwrite-strings"
    std::vector<const char *> commandVector { coapClientPath, "-N" /*NON-confirmable, avoid hang*/, "-m", cMethod, url };
#pragma GCC diagnostic pop

    // don't wait for process to finish
    pid_t pid = SpawnProcess(commandVector, false, true);
    usleep(delay);

    delete[] cMethod;
    return pid;
}

bool IsUDPPortInUse(int port)
{
    bool inUse = true;
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd != -1)
    {
        struct sockaddr_in si;
        memset((char *) &si, 0, sizeof(si));
        si.sin_family = AF_INET;
        si.sin_port = htons(port);
        si.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(sockfd, (const sockaddr *)&si, sizeof(si)) == -1)
        {
            //perror("bind");
            if (errno == EADDRINUSE)
            {
                std::cerr << "Port " << port << " is already in use - skipping" << std::endl;
                inUse = true;
            }
        }
        else
        {
            inUse = false;
        }
        close(sockfd);
    }
    return inUse;
}

int WaitForIpc(int ipcPort, int timeout /*seconds*/, const char * request, size_t requestLen)
{
    // repeatedly send a request until a response is received
    int sockfd = 0;
    struct sockaddr_in fromAddr;
    socklen_t fromAddrLen = sizeof(fromAddr);

    fromAddr.sin_family = AF_INET;
    fromAddr.sin_port = htons(ipcPort);

    if (inet_aton("127.0.0.1", &fromAddr.sin_addr) == 0)
    {
        printf("inet_aton failed.\n");
        return -1;
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror("Cannot create socket");
        return -1;
    }

    char d = 0;
    struct sockaddr_in theirAddr;
    socklen_t theirAddrLen = sizeof(theirAddr);
    bool response = false;

    // overall timeout (microseconds)
    int timeout_us = timeout * 1000000;  // 10 seconds

    // set recv timeout
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 10000;  // 10 milliseconds per attempt
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) !=  0)
    {
        perror("setsockopt failed");
        close(sockfd);
        return -1;
    }

    int count = 0;
    const int maxCount = timeout_us / (tv.tv_sec * 1000000 + tv.tv_usec);
    while (!response && count < maxCount)
    {
        int rc = sendto(sockfd, request, requestLen, 0, (const struct sockaddr *)&fromAddr, fromAddrLen);
        if (rc == -1)
        {
            perror("sendto failed");
            close(sockfd);
            return -1;
        }

        // Actual response is not important
        rc = recvfrom(sockfd, &d, sizeof(d), 0, (struct sockaddr *)&theirAddr, &theirAddrLen);
        response = rc == sizeof(d);
        ++count;
    }

    close(sockfd);
    return response ? 0 : -1;
}

// return 0 on success
static int WaitForLWM2MClientIpc(int ipcPort, int timeout /*seconds*/)
{
    //std::cout << "WaitForLwM2MClientIpc(ipcPort " << ipcPort << ", timeout " << timeout << ")" << std::endl;
    const char * request = "<Request><Type>Get</Type><Content><ObjectID>3</ObjectID><InstanceID>0</InstanceID><PropertyID>15</PropertyID></Content></Request>";
    return WaitForIpc(ipcPort, timeout, request, strlen(request));
}

pid_t StartAwaClient(const char * clientDaemonPath, int iCoapPort, int iIpcPort, const char * logFile, const char * endpointName, const char * bootstrapConfig, const char * bootstrapURI, const char * objectDefinitionsFile, const std::vector<std::string> & additionalOptions)
{
    // unfortunately, execvp requires char * not const char * parameters
    std::string sCoapPort = std::to_string(iCoapPort);
    std::string sIpcPort = std::to_string(iIpcPort);
    std::string sLogFile = logFile;
    std::string sEndpointName = endpointName;
    std::string sObjectDefinitionsFile = objectDefinitionsFile;

    char * cCoapPort = new char[sCoapPort.length() + 1];
    std::strcpy(cCoapPort, sCoapPort.c_str());

    char * cIpcPort = new char[sIpcPort.length() + 1];
    std::strcpy(cIpcPort, sIpcPort.c_str());

    char * cLogFile = new char[sLogFile.length() + 1];
    std::strcpy(cLogFile, sLogFile.c_str());

    char * cEndpointName = new char[sEndpointName.length() + 1];
    std::strcpy(cEndpointName, sEndpointName.c_str());

    char * cObjectDefinitionsFile = new char [sObjectDefinitionsFile.length() + 1];
    std::strcpy(cObjectDefinitionsFile, sObjectDefinitionsFile.c_str());

    // suppress "deprecated conversion from string constant" warning
#pragma GCC diagnostic ignored "-Wwrite-strings"
    std::vector<const char *> commandVector {
        clientDaemonPath,
        "--verbose",
        "--port", cCoapPort,
        "--ipcPort", cIpcPort,
        "--logFile", cLogFile,
        "--objDefs", cObjectDefinitionsFile,
        "--endPointName", cEndpointName };
#pragma GCC diagnostic pop

    // use bootstrap server if URI is provided
    if (strlen(bootstrapURI) == 0)
    {
        commandVector.push_back("--factoryBootstrap");
        commandVector.push_back(bootstrapConfig);
    }
    else
    {
        commandVector.push_back("--bootstrap");
        commandVector.push_back(bootstrapURI);
    }

    // append additional options
    for (auto & x : additionalOptions) {
        commandVector.push_back(x.c_str());
    }

    // don't wait for process to finish
    pid_t pid = SpawnProcess(commandVector, false, false);

    delete[] cCoapPort;
    delete[] cIpcPort;
    delete[] cLogFile;
    delete[] cEndpointName;
    delete[] cObjectDefinitionsFile;

    // wait for LWM2M Client to respond on IPC
    if (WaitForLWM2MClientIpc(iIpcPort, 10 /*seconds*/) != 0)
    {
        std::cout << "LWM2M Client IPC did not respond" << std::endl;
        pid = -1;  // error
    }

    return pid;
}

// return 0 on success
static int WaitForLwM2MServerIpc(int ipcPort, int timeout /*seconds*/)
{
    //std::cout << "WaitForLwM2MServerIpc(ipcPort " << ipcPort << ", timeout " << timeout << ")" << std::endl;
    const char * request = "<Request><Type>ListClients</Type></Request>";
    return WaitForIpc(ipcPort, timeout, request, strlen(request));
}

pid_t StartAwaServer(const char * serverDaemonPath, int iCoapPort, int iIpcPort, const char * logFile, const std::vector<std::string> & additionalOptions)
{
    // unfortunately, execvp requires char * not const char * parameters
    std::string sCoapPort = std::to_string(iCoapPort);
    std::string sIpcPort = std::to_string(iIpcPort);
    std::string sLogFile = logFile;

    char * cCoapPort = new char[sCoapPort.length() + 1];
    std::strcpy(cCoapPort, sCoapPort.c_str());

    char * cIpcPort = new char[sIpcPort.length() + 1];
    std::strcpy(cIpcPort, sIpcPort.c_str());

    char * cLogFile = new char[sLogFile.length() + 1];
    std::strcpy(cLogFile, sLogFile.c_str());

    // suppress "deprecated conversion from string constant" warning
#pragma GCC diagnostic ignored "-Wwrite-strings"
    std::vector<const char *> commandVector {
        serverDaemonPath,
        "--verbose",
        "--port", cCoapPort,
        "--ipcPort", cIpcPort,
        "--logFile", cLogFile };
#pragma GCC diagnostic pop

    // append additional options
    for (auto & x : additionalOptions) {
        commandVector.push_back(x.c_str());
    }

    // don't wait for process to finish
    int rc = SpawnProcess(commandVector, false, false);

    delete[] cCoapPort;
    delete[] cIpcPort;
    delete[] cLogFile;

    // wait for LWM2M Server to respond on IPC
    if (WaitForLwM2MServerIpc(iIpcPort, 10 /*seconds*/) != 0)
    {
        printf("LWM2M Server IPC did not respond\n");
        rc = -1;
    }

    return rc;
}

pid_t StartAwaBootstrapServer(const char * bootstrapServerDaemonPath, int iCoapPort, const char * configFile, const char * logFile)
{
    // unfortunately, execvp requires char * not const char * parameters
    std::string sCoapPort = std::to_string(iCoapPort);
    std::string sConfigFile = configFile;
    std::string sLogFile = logFile;

    char * cCoapPort = new char[sCoapPort.length() + 1];
    std::strcpy(cCoapPort, sCoapPort.c_str());

    char * cConfigFile = new char[sConfigFile.length() + 1];
    std::strcpy(cConfigFile, sConfigFile.c_str());

    char * cLogFile = new char[sLogFile.length() + 1];
    std::strcpy(cLogFile, sLogFile.c_str());

    // suppress "deprecated conversion from string constant" warning
#pragma GCC diagnostic ignored "-Wwrite-strings"
    std::vector<const char *> commandVector {
        bootstrapServerDaemonPath,
        "--verbose",
        "--port", cCoapPort,
        "--config", cConfigFile,
        "--logFile", cLogFile };
#pragma GCC diagnostic pop

    // don't wait for process to finish
    int rc = SpawnProcess(commandVector, false, false);

    delete[] cCoapPort;
    delete[] cConfigFile;
    delete[] cLogFile;

    // no way to be sure bootstrap server is ready, so wait a while
    sleep(0.1);

    return rc;
}

} // namespace Awa

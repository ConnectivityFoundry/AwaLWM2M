/********************************************************
 *** FlowDm Client SDK Common Test Support Functions
 *******************************************************/

#include <vector>
#include <unistd.h>
#include <cstdio>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>

#include "support.h"

namespace Awa {

namespace global {

    // initialization values are subject to Static Initialization Order Fiasco
    int logLevel = 0;
    int clientIpcPort = 0;
    int serverIpcPort = 0;
    int clientLocalCoapPort = 0;
    int serverCoapPort = 0;
    bool spawnClientDaemon = false;
    bool spawnServerDaemon = false;
    const char * coapClientPath = nullptr;
    const char * clientDaemonPath = nullptr;
    const char * serverDaemonPath = nullptr;
    const char * bootstrapDaemonPath = nullptr;
    const char * bootstrapConfig = nullptr;
    const char * clientEndpointName = nullptr;
    const char * clientLogFile = nullptr;
    const char * serverLogFile = nullptr;

    // initialise globals with a function to avoid Static Initialization Order Fiasco
    void SetGlobalDefaults(void)
    {
        global::logLevel = defaults::logLevel;
        global::clientIpcPort = defaults::clientIpcPort;
        global::serverIpcPort = defaults::serverIpcPort;
        global::clientLocalCoapPort = defaults::clientLocalCoapPort;
        global::serverCoapPort = defaults::serverCoapPort;
        global::spawnClientDaemon = true;
        global::spawnServerDaemon = true;
        global::coapClientPath = defaults::coapClientPath;
        global::clientDaemonPath = defaults::clientDaemonPath;
        global::serverDaemonPath = defaults::serverDaemonPath;
        global::bootstrapDaemonPath = defaults::bootstrapDaemonPath;
        global::bootstrapConfig = defaults::bootstrapConfig;
        global::clientEndpointName = defaults::clientEndpointName;
        global::clientLogFile = defaults::clientLogFile;
        global::serverLogFile = defaults::serverLogFile;
    }

} // namespace global

namespace detail {
    const char * NonRoutableIPv4Address = "192.0.2.0";
    const char * NonRoutableIPv6Address = "2001:db8::";
} // namespace detail

pid_t SpawnProcess(std::vector<const char *> &commandVector, bool wait_, bool silent)
{
    pid_t c_pid, pid;

//    std::cerr << "SpawnProcess: " << commandVector.size() << std::endl;
//    for (auto it = commandVector.begin(); it != commandVector.end(); ++it)
//    {
//      std::cerr << *it << std::endl;
//    }

    c_pid = fork();

    if (c_pid == 0)
    {
        // child
        //std::cout << "Child: executing " << commandVector[0] << std::endl;

        if (silent)
        {
            // redirect stdout to /dev/null
            FILE * fd = fopen("/dev/null", "at");
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
        perror("execve failed");
        std::cerr << argv[0] << std::endl;
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
        perror("fork failed");
        c_pid = -1;
    }

    return c_pid;
}

void KillProcess(pid_t pid)
{
    // kill LWM2M Client process
    //std::cout << "Kill pid " << pid_ << std::endl;
    if (pid > 0) {
        if (kill(pid, SIGKILL))
        {
            perror("kill failed");
        }
        pid = 0;
    }
}

void TerminateProcess(pid_t pid)
{
    //std::cout << "Terminate pid " << pid_ << std::endl;
    if (pid > 0) {
        if (kill(pid, SIGTERM))
        {
            perror("kill failed");
        }

        waitpid(pid, NULL, 0);
        pid = 0;
    }
}

pid_t CoAPOperation(int port, const char * method, const char * resource, int delay /*microseconds*/)
{
    // unfortunately, execvp requires char * not const char * parameters
    std::string sMethod = method;
    char * cMethod = new char[sMethod.length() + 1];
    std::strcpy(cMethod, sMethod.c_str());

    // spawn concurrent process: run "coap-client -m post coap://127.0.0.1:6001/3/0/4"
    char url[256];
    sprintf(url, "coap://127.0.0.1:%d/%s", port, resource);

#pragma GCC diagnostic ignored "-Wwrite-strings"
    std::vector<const char *> commandVector { global::coapClientPath, "-N" /*NON-confirmable, avoid hang*/, "-m", cMethod, url };
#pragma GCC diagnostic pop

    // don't wait for process to finish
    pid_t pid = SpawnProcess(commandVector, false, true);
    usleep(delay);

    delete[] cMethod;
    return pid;
}

int WaitForIpc(int ipcPort, int timeout /*seconds*/, const char * request, size_t requestLen)
{
    // repeatedly send a request until a response is received
    int sockfd = 0;
    struct sockaddr_in fromAddr;
    socklen_t fromAddrLen = sizeof(fromAddr);

    fromAddr.sin_family = AF_INET;
    fromAddr.sin_family = SOCK_DGRAM;
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
        return -1;
    }

    int count = 0;
    const int maxCount = timeout_us / (tv.tv_sec * 1000000 + tv.tv_usec);
    while (!response && count < maxCount)
    {
        int rc;

        //printf("count %d\n", count);
        //printf("Probing for active IPC\n");
        rc = sendto(sockfd, request, requestLen, 0, (const struct sockaddr *)&fromAddr, fromAddrLen);
        if (rc == -1)
        {
            perror("sendto failed");
            //printf("sockfd %d, request %p, requestLen %zu, fromAddr %p, fromAddrLen %d\n", sockfd, request, requestLen, &fromAddr, fromAddrLen);
            return -1;
        }

        // actual response is not important
        //printf("Waiting for response\n");
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

pid_t StartLWM2MClient(int iCoapPort, int iIpcPort, const char * logFile, const char * endpointName)
{
    // unfortunately, execvp requires char * not const char * parameters
    std::string sCoapPort = std::to_string(iCoapPort);
    std::string sIpcPort = std::to_string(iIpcPort);
    std::string sLogFile = logFile;
    std::string sEndpointName = endpointName;

    char * cCoapPort = new char[sCoapPort.length() + 1];
    std::strcpy(cCoapPort, sCoapPort.c_str());

    char * cIpcPort = new char[sIpcPort.length() + 1];
    std::strcpy(cIpcPort, sIpcPort.c_str());

    char * cLogFile = new char[sLogFile.length() + 1];
    std::strcpy(cLogFile, sLogFile.c_str());

    char * cEndpointName = new char[sEndpointName.length() + 1];
    std::strcpy(cEndpointName, sEndpointName.c_str());

    // gets rid of annoying "deprecated conversion from string constant blah blah" warning
#pragma GCC diagnostic ignored "-Wwrite-strings"
    std::vector<const char *> commandVector {
        global::clientDaemonPath,
        "--verbose",
        "--port", cCoapPort,
        "--ipcPort", cIpcPort,
        "--logFile", cLogFile,
        "--endPointName", cEndpointName,
        "--factoryBootstrap", global::bootstrapConfig };
#pragma GCC diagnostic pop

    // don't wait for process to finish
    pid_t pid = SpawnProcess(commandVector, false, false);

    delete[] cCoapPort;
    delete[] cIpcPort;
    delete[] cLogFile;
    delete[] cEndpointName;

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
    const char * request = "<Request><Type>ListClients</Type></Request>";
    return WaitForIpc(ipcPort, timeout, request, strlen(request));
}

pid_t StartLWM2MServer(int iCoapPort, int iIpcPort, const char * logFile)
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

    // gets rid of annoying "deprecated conversion from string constant blah blah" warning
#pragma GCC diagnostic ignored "-Wwrite-strings"
    std::vector<const char *> commandVector {
        global::serverDaemonPath,
        "--verbose",
        "--port", cCoapPort,
        "--ipcPort", cIpcPort,
        "--logFile", cLogFile };
#pragma GCC diagnostic pop

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

bool ElapsedTimeWithinTolerance(double time_ms, double time_target_ms, double tolerance_ms)
{
    return (time_ms >= time_target_ms - tolerance_ms) && (time_ms <= time_target_ms + tolerance_ms);
}

} // namespace Awa

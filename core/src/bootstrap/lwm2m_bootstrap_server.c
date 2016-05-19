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


#include <poll.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>

#include "lwm2m_object_store.h"
#include "coap_abstraction.h"
#include "lwm2m_core.h"
#include "lwm2m_object_defs.h"
#include "bootstrap/lwm2m_bootstrap.h"

#define DEFAULT_IP_ADDRESS          "0.0.0.0"
#define MAX_BOOTSTRAP_CONFIG_FILES  (2)

typedef struct
{
    int Port;
    bool Verbose;
    bool Daemonise;
    char * LogFile;
    const char * Config[MAX_BOOTSTRAP_CONFIG_FILES];
    int ConfigCount;
    char * IPAddress;
    char * InterfaceName;
    int AddressFamily;
    bool Version;
} Options;

static FILE * logFile;
static const char * version = VERSION; /* from Makefile */
static volatile int quit = 0;

static void PrintOptions(const Options * options);

static void CtrlCSignalHandler(int dummy)
{
    quit = 1;
}

// Fork off a daemon process, the parent will exit at this point
static void Daemonise(bool verbose)
{
    pid_t pid;

    // forkoff the parent process
    pid = fork();

    if (pid < 0)
    {
        printf("Failed to start daemon\n");
        exit(EXIT_FAILURE);
    }

    if (pid > 0)
    {
        if (verbose)
            printf("Daemon running as %d\n", pid);
        exit(EXIT_SUCCESS);
    }

    umask(0);

    // create sid for child
    if (setsid() < 0)
    {
        printf("Failed to set sid\n");
        exit(EXIT_FAILURE);
    }

    // close off standard file descriptors
    close (STDIN_FILENO);
    close (STDOUT_FILENO);
    close (STDERR_FILENO);
}

static int Bootstrap_Start(Options * options)
{
    int result = 0;

    if (options->Daemonise)
    {
        Daemonise(options->Verbose);
    }
    else
    {
        signal(SIGINT, CtrlCSignalHandler);
    }

    signal(SIGTERM, CtrlCSignalHandler);

    // open log files here
    if (options->LogFile)
    {
        errno = 0;
        logFile = fopen(options->LogFile, "at");
        if (logFile != NULL)
        {
            Lwm2m_SetOutput(logFile);

            // redirect stdout
            dup2(fileno(logFile), STDOUT_FILENO);
        }
        else
        {
            Lwm2m_Error("Failed to open log file %s: %s\n", options->LogFile, strerror(errno));
        }
    }

    if (options->Version)
    {
        Lwm2m_Printf(0, "%s\n", version);
        goto error_close_log;
    }

    Lwm2m_SetLogLevel((options->Verbose) ? DebugLevel_Debug : DebugLevel_Info);
    Lwm2m_PrintBanner();
    if (options->Verbose)
    {
        PrintOptions(options);
    }
    Lwm2m_Info("Awa LWM2M Bootstrap Server, version %s\n", version);
    Lwm2m_Info("  Process ID     : %d\n", getpid());
    Lwm2m_Info("  CoAP port      : %d\n", options->Port);

    if (options->InterfaceName != NULL)
    {
        Lwm2m_Info("  Interface      : %s [IPv%d]\n", options->InterfaceName, options->AddressFamily == AF_INET? 4 : 6);
    }
    else if (strcmp(DEFAULT_IP_ADDRESS, options->IPAddress) != 0)
    {
        Lwm2m_Info("  IP Address     : %s\n", options->IPAddress);
    }

    char ipAddress[NI_MAXHOST];
    if (options->InterfaceName != NULL)
    {
        if (Lwm2mCore_GetIPAddressFromInterface(options->InterfaceName, options->AddressFamily, ipAddress, sizeof(ipAddress)) != 0)
        {
            result = 1;
            goto error_close_log;
        }
        Lwm2m_Info("  Interface Addr : %s\n", ipAddress);
    }
    else
    {
        strncpy(ipAddress, options->IPAddress, NI_MAXHOST);
        ipAddress[NI_MAXHOST - 1] = '\0';  // Defensive
    }

    CoapInfo * coap = coap_Init(ipAddress, options->Port, (options->Verbose) ? DebugLevel_Debug : DebugLevel_Info);
    if (coap == NULL)
    {
        printf("Unable to map address to network interface\n");
        result = 1;
        goto error_close_log;
    }
    Lwm2mContextType * context = Lwm2mCore_Init(coap);

    // must happen after coap_Init()
    Lwm2m_RegisterObjectTypes(context);

    if (!Lwm2mBootstrap_BootStrapInit(context, options->Config, options->ConfigCount))
    {
        printf("Failed to initialise boostrap\n");
        result = 1;
        goto error_destroy;
    }

    // wait for messages on both the "IPC" and coap interfaces
    while (!quit)
    {
        int loop_result;
        struct pollfd fds[1];
        int nfds = 1;
        int timeout;

        fds[0].fd = coap->fd;
        fds[0].events = POLLIN;

        timeout = Lwm2mCore_Process(context);

        loop_result = poll(fds, nfds, timeout);

        if (loop_result < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            perror("poll:");
            break;
        }
        else if (loop_result > 0)
        {
            if (fds[0].revents == POLLIN)
            {
                coap_HandleMessage();
            }
        }
        coap_Process();
    }
    Lwm2m_Debug("Exit triggered\n");

error_destroy:
    Lwm2mBootstrap_Destroy();
    Lwm2mCore_Destroy(context);
    coap_Destroy();

error_close_log:
    Lwm2m_Info("Bootstrap Server exiting\n");
    if (logFile != NULL)
    {
        fclose(logFile);
    }

    return result;
}

static void PrintUsage(void)
{
    printf("Awa LWM2M Bootstrap Server, version %s\n", version);
    printf("Copyright (c) 2016 Imagination Technologies Limited and/or its affiliated group companies.\n\n");

    printf("Usage: awa_bootstrapd [options]\n\n");

    printf("Options:\n");
    printf("  --ip, -a ADDR           : Accept client bootstrap requests on IP address ADDR\n");
    printf("  --interface, -e IF      : Accept client bootstrap requests on network interface IF\n");
    printf("  --addressFamily, -f AF  : Address family for network interface. AF=4 for IPv4, AF=6 for IPv6\n");
    printf("  --port, -p PORT         : Use port number PORT for CoAP communications\n");
    printf("  --config, -c FILE       : Load server list configuration from FILE\n");
    printf("  --daemonize, -d         : Detach process from terminal and run in the background\n");
    printf("  --verbose, -v           : Generate verbose output\n");
    printf("  --logFile, -l FILE      : Log output to FILE\n");
    printf("  --version, -V           : Print version and exit\n");
    printf("  --help, -h              : Show usage\n\n");

    printf("Example:\n");
    printf("    awa_bootstrapd --port 15685 --config bootstrap.config\n\n");

}

static void PrintOptions(const Options * options)
{
    printf("Options provided:\n");
    printf("  IPAddress      (--ip)             : %s\n", options->IPAddress);
    printf("  InterfaceName  (--interface)      : %s\n", options->InterfaceName);
    printf("  AddressFamily  (--addressFamily)  : %d\n", options->AddressFamily == AF_INET? 4 : 6);
    printf("  Port           (--port)           : %d\n", options->Port);
    int i;
    for (i = 0; i < options->ConfigCount; ++i)
    {
        printf("  Config         (--config)         : %s\n", options->Config[i]);
    }
    printf("  Daemonise      (--daemonise)      : %d\n", options->Daemonise);
    printf("  Verbose        (--verbose)        : %d\n", options->Verbose);
    printf("  LogFile        (--logFile)        : %s\n", options->LogFile);
}

static int ParseOptions(int argc, char ** argv, Options * options)
{
    while (1)
    {
        int optionIndex = 0;

        static struct option longOptions[] =
        {
            {"ip",            required_argument,      0, 'a'},
            {"interface",     required_argument,      0, 'e'},
            {"addressFamily", required_argument,      0, 'f'},
            {"port",          required_argument,      0, 'p'},
            {"config",        required_argument,      0, 'c'},
            {"verbose",       no_argument,            0, 'v'},
            {"daemonise",     no_argument,            0, 'd'},
            {"logFile",       required_argument,      0, 'l'},
            {"version",       no_argument,            0, 'V'},
            {"help",          no_argument,            0, 'h'},
            {0,               0,                      0,  0 }
        };

        int c = getopt_long(argc, argv, "a:p:c:vdl:Vh", longOptions, &optionIndex);
        if (c == -1)
            break;

        switch (c)
        {
            case 'a':
                options->IPAddress = optarg;
                break;
            case 'e':
                options->InterfaceName = optarg;
                break;
            case 'f':
                options->AddressFamily = (atoi(optarg) == 4) ? AF_INET : AF_INET6;
                break;
            case 'p':
                options->Port = atoi(optarg);
                break;
            case 'c':
                options->Config[options->ConfigCount++] = optarg;
                break;
            case 'd':
                options->Daemonise = true;
                break;
            case 'v':
                options->Verbose = true;
                break;
            case 'l':
                options->LogFile = optarg;
                break;
            case 'V':
                options->Version = true;
                break;
            case 'h':
            default:
                PrintUsage();
                exit(EXIT_FAILURE);
        }
    }
    return 0;
}

int main(int argc, char ** argv)
{
    Options options =
    {
        .Port = 15685,
        .Verbose = false,
        .Daemonise = false,
        .LogFile = NULL,
        .Config = {0},
        .ConfigCount = 0,
        .IPAddress = DEFAULT_IP_ADDRESS,
        .InterfaceName = NULL,
        .AddressFamily = AF_INET,
        .Version = false,
    };

    if (ParseOptions(argc, argv, &options) == 0)
    {
        Bootstrap_Start(&options);
    }

    exit(EXIT_SUCCESS);
}


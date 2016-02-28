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

#include "lwm2m_core.h"  
#include "lwm2m_object_store.h"
#include "coap_abstraction.h"
#include "lwm2m_bootstrap.h"
#include "lwm2m_registration.h"
#include "lwm2m_connectivity_object.h"
#include "lwm2m_security_object.h"
#include "lwm2m_device_object.h"
#include "lwm2m_server_object.h"
#include "lwm2m_firmware_object.h"
#include "lwm2m_location_object.h"
#include "lwm2m_acl_object.h"
#include "lwm2m_client_xml_handlers.h"
#include "lwm2m_xml_interface.h"
#include "lwm2m_object_defs.h"


#define DEFAULT_COAP_PORT (6000)
#define DEFAULT_IPC_PORT (12345)


typedef struct
{
    int CoapPort;
    int IpcPort;
    bool Verbose;
    bool Daemonise;
    char * EndPointName;
    char * BootStrap;
    char * Logfile;
    int AddressFamily;
    const char * FactoryBootstrapFile;
} Options;


static FILE * logFile = NULL;
static const char * version = VERSION; // from Makefile
static volatile int quit = 0;


static void Lwm2m_CtrlCSignalHandler(int dummy)
{
    Lwm2m_Debug("Exit triggered\n");
    quit = 1;
}

static void RegisterObjects(Lwm2mContextType * context, Options * options)
{
    Lwm2m_Debug("Register built-in objects\n");

    Lwm2m_RegisterSecurityObject(context);
    if (options->BootStrap != NULL)
    {
        Lwm2m_PopulateSecurityObject(context, options->BootStrap);
    }
    Lwm2m_RegisterServerObject(context);
    Lwm2m_RegisterACLObject(context);
    Lwm2m_RegisterDeviceObject(context);
    Lwm2m_RegisterConnectivityObjects(context);
    Lwm2m_RegisterFirmwareObject(context);
    Lwm2m_RegisterLocationObject(context);
}

// Fork off a daemon process, the parent will exit at this point
static void Daemonise(bool verbose)
{
    pid_t pid;
    pid = fork();

    if (pid < 0)
    {
        printf("Failed to start daemon\n");
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0)
    {
        if (verbose)
        {
            printf("Daemon running as %d\n", pid);
        }
        exit(EXIT_SUCCESS);
    }

    umask(0);

    // Create a new session for child
    if (setsid() < 0)
    {
        perror("setsid");
        exit(EXIT_FAILURE);
    }

    // close off standard file descriptors
    close (STDIN_FILENO);
    close (STDOUT_FILENO);
    close (STDERR_FILENO);
}

static int Lwm2mClient_Start(Options * options)
{
    int result = 0;
    if (options->Daemonise)
    {
        Daemonise(options->Verbose);
    }
    else
    {
        signal(SIGINT, Lwm2m_CtrlCSignalHandler);
    }

    signal(SIGTERM, Lwm2m_CtrlCSignalHandler);

    if (options->Logfile)
    {
        errno = 0;
        logFile = fopen(options->Logfile, "at");
        if (logFile != NULL)
        {
            Lwm2m_SetOutput(logFile);
            // redirect stdout
            dup2(fileno(logFile), STDOUT_FILENO);
        }
        else
        {
            Lwm2m_Error("Failed to open log file %s: %s\n", options->Logfile, strerror(errno));
        }
    }

    Lwm2m_SetLogLevel((options->Verbose) ? DebugLevel_Debug : DebugLevel_Info);
    Lwm2m_PrintBanner();
    Lwm2m_Info("LWM2M client - version %s\n", version);
    Lwm2m_Info("LWM2M client - Local CoAP port %d\n", options->CoapPort);
    Lwm2m_Info("LWM2M client - Local IPC port %d\n", options->IpcPort);
    Lwm2m_Info("LWM2M client - Using IPv%d\n", options->AddressFamily == AF_INET ? 4 : 6);

    CoapInfo * coap = coap_Init((options->AddressFamily == AF_INET) ? "0.0.0.0" : "::", options->CoapPort, (options->Verbose) ? DebugLevel_Debug : DebugLevel_Info);
    if (coap == NULL)
    {
        Lwm2m_Error("Failed to initialise CoAP on port %d\n", options->CoapPort);
        result = 1;
        goto error;
    }

    // if required read the bootstrap information from a file
    const BootstrapInfo * factoryBootstrapInfo;
    if (options->FactoryBootstrapFile != NULL)
    {
        factoryBootstrapInfo = BootstrapInformation_ReadConfigFile(options->FactoryBootstrapFile);
    }
    else
    {
        factoryBootstrapInfo = NULL;
    }

    Lwm2mContextType * context = Lwm2mCore_Init(coap, options->EndPointName);

    // Must happen after coap_Init().
    RegisterObjects(context, options);

    Lwm2mCore_SetFactoryBootstrap(context, factoryBootstrapInfo);

    // bootstrap information has been loaded, no need to hang onto this anymore
    BootstrapInformation_DeleteBootstrapInfo(factoryBootstrapInfo);


    // Listen for UDP packets on IPC port
    int xmlFd = xmlif_init(context, options->IpcPort);
    if (xmlFd < 0)
    {
        if (logFile)
        {
            fclose(logFile);
        }

        Lwm2m_Error("Failed to initialise XML interface on port %d\n", options->IpcPort);
        result = 1;
        goto error;
    }

    xmlif_RegisterHandlers();

    // Wait for messages on both the "IPC" and CoAP interfaces
    while (!quit)
    {
        int result;
        struct pollfd fds[2];
        int nfds = 2;
        int timeout;

        fds[0].fd = coap->fd;
        fds[0].events = POLLIN;

        fds[1].fd = xmlFd;
        fds[1].events = POLLIN;

        timeout = Lwm2mCore_Process(context);

        result = poll(fds, nfds, timeout);
        if (result < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            perror("poll:");
            break;
        }
        else if (result > 0)
        {
            if (fds[0].revents == POLLIN)
            {
                coap_HandleMessage();
            }
            if (fds[1].revents == POLLIN)
            {
                xmlif_process(fds[1].fd);
            }
        }

        coap_Process();
    }

    Lwm2m_Info("Client exiting\n");

    Lwm2mCore_Destroy(context);
    coap_Destroy();
    xmlif_DestroyExecuteHandlers();
    xmlif_destroy(xmlFd);

    if (logFile)
    {
        fclose(logFile);
    }
error:
    return result;
}

static void PrintUsage(void)
{
    printf("Imagination Technologies LWM2M Client - %s\n\n", version);

    printf("Usage: awa_clientd [options] [--bootstrap [URI] | --factoryBootstrap [filename]]\n\n");

    printf("Options:\n");
    printf("  --port, -p          : Local port number for CoAP communications\n");
    printf("  --addressFamily     : Address family for network interface. 4 for IPv4, 6 for IPv6\n");
    printf("  --ipcPort, -i       : port number for IPC communications\n");
    printf("  --endPointName, -e  : client end point name\n");
    printf("  --bootstrap, -b     : bootstrap server URI\n");
    printf("  --factoryBootstrap, -f\n"
           "                      : factory bootstrap information file\n");
    printf("  --logFile, -l       : log filename\n");
    printf("  --daemonise, -d     : run as daemon\n");
    printf("  --verbose, -v       : enable verbose output\n");
    printf("  --help              : show usage\n\n");

    printf("Example:\n");
    printf("    awa_clientd --port 6000 --endPointName client1 --bootstrap coap://[::1]:2134\n\n");
}

static int ParseOptions(int argc, char ** argv, Options * options)
{
    while (1)
    {
        int optionIndex = 0;

        static struct option longOptions[] = 
        {
            {"port",             required_argument, 0, 'p'},
            {"addressFamily",    required_argument, 0, 'a'},
            {"ipcPort",          required_argument, 0, 'i'},
            {"bootstrap",        required_argument, 0, 'b'},
            {"endPointName",     required_argument, 0, 'e'},
            {"verbose",          no_argument,       0, 'v'},
            {"daemonise",        no_argument,       0, 'd'},
            {"help",             no_argument,       0, 'h'},
            {"logFile",          required_argument, 0, 'l'},
            {"factoryBootstrap", required_argument, 0, 'f'},
            {0,                  0,                 0,  0 }
        };

        int c = getopt_long(argc, argv, "p:a:i:b:e:vhl:df:", longOptions, &optionIndex);
        if (c == -1)
        {
            break;
        }

        switch (c)
        {
            case 'p':
                options->CoapPort = atoi(optarg);
                break;
            case 'a':
                options->AddressFamily = atoi(optarg) == 4 ? AF_INET : AF_INET6;
                break;
            case 'i':
                options->IpcPort = atoi(optarg);
                break;
            case 'b':
                options->BootStrap = optarg;
                break;
            case 'e':
                options->EndPointName = optarg;
                break;
            case 'l':
                options->Logfile = optarg;
                break;
            case 'd':
                options->Daemonise = true;
                break;
            case 'v':
                options->Verbose = true;
                break;
            case 'f':
                options->FactoryBootstrapFile = optarg;
                break;
            case 'h':
            default:
                PrintUsage();
                exit(EXIT_FAILURE);
        }
    }

    // Check to see if at least one bootstrap option is specified
    if ((options->BootStrap == NULL) && (options->FactoryBootstrapFile == NULL))
    {
        printf("Error: please specify a bootstrap option (--bootstrap or --factoryBootstrap)\n\n");
        PrintUsage();
        exit(EXIT_FAILURE);
    }

    return 0;
}

int main(int argc, char ** argv)
{
    Options options =
    {
        .CoapPort = DEFAULT_COAP_PORT,
        .IpcPort = DEFAULT_IPC_PORT,
        .Verbose = false,
        .Daemonise = false,
        .BootStrap = NULL,
        .EndPointName = "imagination1",
        .Logfile = NULL,
        .AddressFamily = AF_INET,
        .FactoryBootstrapFile = NULL,
    };

    if (ParseOptions(argc, argv, &options) == 0)
    {
        Lwm2mClient_Start(&options);
    }

    exit(EXIT_SUCCESS);
}

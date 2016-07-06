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

#include "awa_clientd_cmdline.h"
#include "lwm2m_core.h"
#include "lwm2m_object_store.h"
#include "coap_abstraction.h"
#include "dtls_abstraction.h"
#include "xmltree.h"
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
#include "lwm2m_client_cert.h"
#include "lwm2m_client_psk.h"

#define MAX_OBJDEFS_FILES (16)

typedef struct
{
    int CoapPort;
    int AddressFamily;
    int IpcPort;
    char * EndPointName;
    char * BootStrap;
    const char * FactoryBootstrapFile;
    const char * ObjDefsFiles[MAX_OBJDEFS_FILES];
    size_t NumObjDefsFiles;
    bool Daemonise;
    bool Verbose;
    char * LogFile;
    bool Version;
} Options;

static FILE * logFile = NULL;
static const char * version = VERSION; // from Makefile
static volatile int quit = 0;

static void PrintOptions(const Options * options);

static void Lwm2m_CtrlCSignalHandler(int dummy)
{
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

static int LoadObjectDefinitionsFromFile(Lwm2mContextType * context, const char * filename)
{
    DefinitionCount count = { 0 };
    int result = 0;

    Lwm2m_Info("Load definitions: from \'%s\'\n", filename);

    FILE *f = fopen(filename, "rb");
    if (f != NULL)
    {
        fseek(f, 0, SEEK_END);
        long pos = ftell(f);
        fseek(f, 0, SEEK_SET);
        uint8_t * doc = malloc(pos);
        if (doc != NULL)
        {
            size_t nmemb = fread(doc, pos, 1, f);
            if (nmemb == 1)
            {
                Lwm2m_Debug("Parsing %s, %ld bytes\n", filename, pos);
                TreeNode objectDefinitionsNode = TreeNode_ParseXML(doc, pos, true);
                count = xmlif_ParseObjDefDeviceServerXml(context, objectDefinitionsNode);
                result = 0;
                Tree_Delete(objectDefinitionsNode);
                free(doc);
            }
            else
            {
                perror("fread");
                result = -1;
                free(doc);
            }
        }
        else
        {
            Lwm2m_Error("Out of memory\n");
            result = -1;
        }
        fclose(f);
    }
    else
    {
        perror("fopen");
        result = -1;
    }

    if (result == 0)
    {
        // regard any failures as fatal

        if (count.NumObjectsFailed > 0) {
            Lwm2m_Error("%zu object definition%s failed\n", count.NumObjectsFailed, count.NumObjectsFailed != 1 ? "s" : "" );
            result = -1;
        }
        if (count.NumResourcesFailed > 0) {
            Lwm2m_Error("%zu resource definition%s failed\n", count.NumResourcesFailed, count.NumResourcesFailed != 1 ? "s" : "");
            result = -1;
        }
        Lwm2m_Info("Load definitions: %zu object%s and %zu resource%s defined\n", count.NumObjectsOK, count.NumObjectsOK != 1 ? "s" : "", count.NumResourcesOK, count.NumResourcesOK != 1 ? "s" : "");

        // also regard nothing defined as failure
        if (count.NumObjectsOK == 0 && count.NumResourcesOK == 0)
        {
            Lwm2m_Error("No objects or resources defined\n");
            result = -1;
        }
    }

    if (result < 0)
    {
        Lwm2m_Error("Load definitions: failed\n");
    }
    return result;
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
    Lwm2m_Info("Awa LWM2M Client, version %s\n", version);
    Lwm2m_Info("  Process ID     : %d\n", getpid());
    Lwm2m_Info("  Endpoint name  : \'%s\'\n", options->EndPointName);
    Lwm2m_Info("  DTLS library   : %s\n", DTLS_LibraryName);
    Lwm2m_Info("  CoAP library   : %s\n", coap_LibraryName);
    Lwm2m_Info("  CoAP port      : %d\n", options->CoapPort);
    Lwm2m_Info("  IPC port       : %d\n", options->IpcPort);
    Lwm2m_Info("  Address family : IPv%d\n", options->AddressFamily == AF_INET ? 4 : 6);

    CoapInfo * coap = coap_Init((options->AddressFamily == AF_INET) ? "0.0.0.0" : "::", options->CoapPort, false, (options->Verbose) ? DebugLevel_Debug : DebugLevel_Info);
    if (coap == NULL)
    {
        Lwm2m_Error("Failed to initialise CoAP on port %d\n", options->CoapPort);
        result = 1;
        goto error_close_log;
    }

    coap_SetCertificate(clientCert, sizeof(clientCert), CertificateFormat_PEM);
    coap_SetPSK(pskIdentity, pskKey, sizeof(pskKey));

    // if required read the bootstrap information from a file
    const BootstrapInfo * factoryBootstrapInfo;
    if (options->FactoryBootstrapFile != NULL)
    {
        factoryBootstrapInfo = BootstrapInformation_ReadConfigFile(options->FactoryBootstrapFile);
        if (factoryBootstrapInfo == NULL)
        {
            Lwm2m_Error("Factory Bootstrap configuration file load failed\n");
            result = 1;
            goto error_coap;
        }
        else
        {
            Lwm2m_Info("Factory Bootstrap:\n");
            Lwm2m_Info("Server Configuration\n");
            Lwm2m_Info("====================\n");
            BootstrapInformation_Dump(factoryBootstrapInfo);
        }
    }
    else
    {
        factoryBootstrapInfo = NULL;
    }

    Lwm2mContextType * context = Lwm2mCore_Init(coap, options->EndPointName);

    // Must happen after coap_Init().
    RegisterObjects(context, options);

    if (factoryBootstrapInfo != NULL)
    {
        Lwm2mCore_SetFactoryBootstrap(context, factoryBootstrapInfo);
    }

    // bootstrap information has been loaded, no need to hang onto this anymore
    BootstrapInformation_DeleteBootstrapInfo(factoryBootstrapInfo);

    int i;
    for (i = 0; i < options->NumObjDefsFiles; ++i)
    {
        if (LoadObjectDefinitionsFromFile(context, options->ObjDefsFiles[i]) != 0)
        {
            Lwm2m_Error("Failed to load object definitions from file \'%s\'\n", options->ObjDefsFiles[i]);
            goto error_core;
        }
    }

    // Listen for UDP packets on IPC port
    int xmlFd = xmlif_init(context, options->IpcPort);
    if (xmlFd < 0)
    {
        Lwm2m_Error("Failed to initialise XML interface on port %d\n", options->IpcPort);
        result = 1;
        goto error_core;
    }

    xmlif_RegisterHandlers();

    // Wait for messages on both the IPC and CoAP interfaces
    while (!quit)
    {
        int loop_result;
        struct pollfd fds[2];
        int nfds = 2;
        int timeout;

        fds[0].fd = coap->fd;
        fds[0].events = POLLIN;

        fds[1].fd = xmlFd;
        fds[1].events = POLLIN;

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
            if (fds[1].revents == POLLIN)
            {
                xmlif_process(fds[1].fd);
            }
        }
        coap_Process();
    }
    Lwm2m_Debug("Exit triggered\n");

    xmlif_DestroyExecuteHandlers();
    xmlif_destroy(xmlFd);
error_core:
    Lwm2mCore_Destroy(context);
error_coap:
    coap_Destroy();

error_close_log:
    Lwm2m_Info("Client exiting\n");
    if (logFile)
    {
        fclose(logFile);
    }

    return result;
}

static void PrintOptions(const Options * options)
{
    printf("Options specified or defaulted:\n");
    printf("  CoapPort             (--port)             : %d\n", options->CoapPort);
    printf("  AddressFamily        (--addressFamily)    : %d\n", options->AddressFamily == AF_INET? 4 : 6);
    printf("  IpcPort              (--ipcPort)          : %d\n", options->IpcPort);
    printf("  EndPointName         (--endPointName)     : %s\n", options->EndPointName ? options->EndPointName : "");
    printf("  Bootstrap            (--bootstrap)        : %s\n", options->BootStrap ? options->BootStrap : "");
    printf("  FactoryBootstrapFile (--factoryBootstrap) : %s\n", options->FactoryBootstrapFile ? options->FactoryBootstrapFile : "");
    int i;
    for (i = 0; i < options->NumObjDefsFiles; ++i)
    {
        printf("  ObjectDefinitions    (--objDefs)          : %s\n", options->ObjDefsFiles[i]);
    }
    printf("  Daemonize            (--daemonize)        : %d\n", options->Daemonise);
    printf("  Verbose              (--verbose)          : %d\n", options->Verbose);
    printf("  LogFile              (--logFile)          : %s\n", options->LogFile ? options->LogFile : "");
    printf("  Version              (--version)          : %d\n", options->Version);
}

static int ParseOptions(int argc, char ** argv, struct gengetopt_args_info * ai, Options * options)
{
    int result = EXIT_SUCCESS;
    if (cmdline_parser(argc, argv, ai) == 0)
    {
        options->CoapPort = ai->port_arg;
        options->AddressFamily = ai->addressFamily_arg == 4 ? AF_INET : AF_INET6;
        options->IpcPort = ai->ipcPort_arg;
        options->EndPointName = ai->endPointName_arg;
        options->BootStrap = ai->bootstrap_arg;
        options->FactoryBootstrapFile = ai->factoryBootstrap_arg;
        int i;
        for (i = 0; i < ai->objDefs_given; ++i)
        {
            options->ObjDefsFiles[i] = ai->objDefs_arg[i];
        }
        options->NumObjDefsFiles = ai->objDefs_given;
        options->Daemonise = ai->daemonize_flag;
        options->Verbose = ai->verbose_flag;
        options->LogFile = ai->logFile_arg;
        options->Version = ai->version_flag;

        // Check to see if at least one bootstrap option is specified
        if (!options->Version && (options->BootStrap == NULL) && (options->FactoryBootstrapFile == NULL))
        {
            printf("Error: specify a bootstrap option (--bootstrap or --factoryBootstrap) or --version\n\n");
            result = EXIT_FAILURE;
        }
    }
    else
    {
        result = EXIT_FAILURE;
    }
    return result;
}

int main(int argc, char ** argv)
{
    int result = EXIT_FAILURE;
    struct gengetopt_args_info ai;
    Options options =
    {
        .CoapPort = 0,
        .AddressFamily = AF_UNSPEC,
        .IpcPort = 0,
        .EndPointName = NULL,
        .BootStrap = NULL,
        .FactoryBootstrapFile = NULL,
        .ObjDefsFiles = {0},
        .NumObjDefsFiles = 0,
        .Daemonise = false,
        .Verbose = false,
        .LogFile = NULL,
        .Version = false,
    };

    if (ParseOptions(argc, argv, &ai, &options) == EXIT_SUCCESS)
    {
        result = Lwm2mClient_Start(&options);
    }
    cmdline_parser_free(&ai);
    exit(result);
}

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


// Retrieve list of LWM2M clients
//
//  Usage: awa-server-list-clients [OPTIONS]...
//
//    -h, --help                 Print help and exit
//    -V, --version              Print version and exit
//    -v, --verbose              Increase program verbosity  (default=off)
//    -d, --debug                Increase program verbosity  (default=off)
//    -p, --ipcPort=PORT         Connect to IPC port  (default=`12345')
//    -a, --ipcAddress           IP address of client (default='127.0.0.1')
//    -o                         show registered objects
//


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <inttypes.h>

#include "awa/server.h"
#include "awa-server-list-clients_cmdline.h"
#include "tools_common.h"


static void ListClientObjects(const AwaServerSession * session, const AwaServerListClientsOperation * operation, const char * clientID, bool quiet)
{
    const AwaServerListClientsResponse * clientListResponse = AwaServerListClientsOperation_GetResponse(operation, clientID);
    AwaRegisteredEntityIterator * objectIterator = AwaServerListClientsResponse_NewRegisteredEntityIterator(clientListResponse);
    while (AwaRegisteredEntityIterator_Next(objectIterator))
    {
        const char * path = AwaRegisteredEntityIterator_GetPath(objectIterator);
        if (quiet)
        {
            printf(" %s", path);
        }
        else
        {
            AwaObjectID objectID = AWA_INVALID_ID;
            AwaServerSession_PathToIDs(session, path, &objectID, NULL, NULL);
            const AwaObjectDefinition * objectDefinition = AwaServerSession_GetObjectDefinition(session, objectID);
            if (objectDefinition != NULL)
            {
                printf("  %-8s %s", path, AwaObjectDefinition_GetName(objectDefinition));
            }
            else
            {
                printf("  %s Unknown", path);
            }
            printf("\n");
        }
    }
    AwaRegisteredEntityIterator_Free(&objectIterator);
}

static void ListClients(const AwaServerSession * session, bool showObjectDetails, bool quiet)
{
    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session);
    if (operation != NULL)
    {
        if (AwaServerListClientsOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT) == AwaError_Success)
        {
            AwaClientIterator * clientIterator = AwaServerListClientsOperation_NewClientIterator(operation);
            if (clientIterator != NULL)
            {
                int numConnectedClients = 0;
                while (AwaClientIterator_Next(clientIterator))
                {
                    const char * clientID = AwaClientIterator_GetClientID(clientIterator);
                    if (quiet)
                    {
                        printf("%s", clientID);
                    }
                    else
                    {
                        printf("Client: %s\n", clientID);
                    }

                    if (showObjectDetails)
                    {
                        ListClientObjects(session, operation, clientID, quiet);
                    }
                    printf("\n");
                    ++numConnectedClients;
                }
                AwaClientIterator_Free(&clientIterator);
                if ((!quiet) && (numConnectedClients == 0))
                {
                    printf("No clients connected.\n");
                }
            }
            else
            {
                Error("AwaServerListClientsOperation_NewClientIterator failed\n");
            }
        }
        else
        {
            Error("AwaServerListClientsOperation_Perform failed\n");
        }

        if (AwaServerListClientsOperation_Free(&operation) != AwaError_Success)
        {
            Error("AwaServerListClientsOperation_Free failed\n");
        }
    }
    else
    {
        Error("AwaServerListClientsOperation_New failed\n");
    }
}

int main(int argc, char ** argv)
{
    int result = 0;
    struct gengetopt_args_info ai;
    AwaServerSession * session = NULL;

    if (cmdline_parser(argc, argv, &ai) != 0)
    {
        exit(1);
    }

    g_logLevel = ai.debug_given ? 2 : (ai.verbose_given ? 1 : 0);
    AwaLog_SetLevel(ai.debug_given ? AwaLogLevel_Debug : (ai.verbose_given ? AwaLogLevel_Verbose : AwaLogLevel_Warning));

    session = Server_EstablishSession(ai.ipcAddress_arg, ai.ipcPort_arg);
    if (session != NULL)
    {
        ListClients(session, ai.objects_given, ai.quiet_flag != 0);
    }
    
    if (session)
    {
        Server_ReleaseSession(&session);
    }
    cmdline_parser_free(&ai);
    return result;
}

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


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <inttypes.h>

#include "awa/common.h"
#include "awa/server.h"
#include "awa-server-create_cmdline.h"
#include "tools_common.h"


static void AddTarget(AwaServerCreateOperation * operation, const char * clientID, const Target * target)
{
    if (operation != NULL)
    {
        if (target != NULL)
        {
            Verbose("Create %s\n", target->Path);
            if (AwaServerCreateOperation_AddPath(operation, clientID, target->Path, ) != AwaError_Success)
            {
                Error("AwaServerCreateOperation_AddPath failed\n");
            }
        }
        else
        {
            Error("target is NULL\n");
        }
    }
    else
    {
        Error("operation is NULL\n");
    }
}

static void ProcessCreateOperation(const AwaServerSession * session, AwaClientCreateOperation * operation, const char * clientID, bool quiet)
{
    if (AwaServerCreateOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT) != AwaError_Success)
    {
        Error("AwaServerCreateOperation_Perform failed\n");
        return;
    }

    AwaServerCreateResponse * createResponse = AwaServerCreateOperation_GetResponse(operation, clientID);
    if (createResponse != NULL)
    {
        AwaPathIterator pathIterator = AwaServerCreateResponse_NewPathIterator(createResponse);
        while (AwaPathIterator_Next(pathIterator))
        {
            printf("Path %s created successfully\n", AwaPathIterator_Get(pathIterator));
        }
        AwaPathIterator_Free(&pathIterator);
    }
}

int main(int argc, char ** argv)
{
    AwaServerSession * session;
    AwaServerCreateOperation * operation = NULL;
    char address[128];
    unsigned int port;
    struct gengetopt_args_info ai;
    if (cmdline_parser(argc, argv, &ai) != 0)
    {
        exit(1);
    }

    g_logLevel = ai.debug_given ? 2 : (ai.verbose_given ? 1 : 0);
    AwaLog_SetLevel(ai.debug_given ? AwaLogLevel_Debug : (ai.verbose_given ? AwaLogLevel_Verbose : AwaLogLevel_Warning));

    if (ai.inputs_num == 0)
    {
        Error("Specify one or more resource paths.\n");
        exit(1);
    }

    port = ai.ipcPort_arg; 
    strncpy(address, ai.ipcAddress_arg,strlen(ai.ipcAddress_arg)+1);

    // Establish Awa Session with the daemon
    session = EstablishSessionWithServer(address, port);
    if (session == NULL)
    {
        Error("Failed to establish Awa Session\n");
        exit(1);
    }

    // Create Get operation
    operation = AwaServerCreateOperation_New(session);
    if (operation == NULL)
    {
        Error("AwaServerCreateOperation_New failed\n");
        ReleaseSessionWithServer(&session);
        exit(1);
    }
    
    // Add target paths from command line
    int i = 0;
    for (i = 0; i < ai.inputs_num; ++i)
    {
        Target * target = CreateTarget(ai.inputs[i]);
        if (target != NULL)
        {
            AddTarget(operation, ai.clientID_arg, target);
            FreeTarget(&target);
        }
    }
    
    ProcessCreateOperation(session, operation, ai.clientID_arg, ai.quiet_given);

    // Clean up
    AwaServerCreateOperation_Free(&operation);
    ReleaseSessionWithServer(&session);
    cmdline_parser_free(&ai);
    return 0;
}

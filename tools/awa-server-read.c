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


// Read one or more resources from an Object Instance
//
//  Usage: awa-server-read [OPTIONS]... [PATHS]...
//
//    -h, --help                 Print help and exit
//    -V, --version              Print version and exit
//    -v, --verbose              Increase program verbosity  (default=off)
//    -d, --debug                Increase program verbosity  (default=off)
//    -p, --ipcPort=PORT         Connect to Server IPC port  (default=`12345')
//    -a, --ipcAddress           IP address of server (default='127.0.0.1')
//    -q, --quiet                Print values only (quiet)  (default=off)
//    -c, --serverID             ID of Server
//
//  Specify one or more object, object instance and resource paths
//  in the format "/O/I/R", separated by spaces. For example:
//
//  /3/0/0 /4/1 /5 --serverID server1
//


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <inttypes.h>

#include "awa/common.h"
#include "awa/server.h"
#include "awa-server-read_cmdline.h"
#include "tools_common.h"


static void AddTarget(AwaServerReadOperation * operation, const Target * target, const char * clientID)
{
    if (operation != NULL)
    {
        if (target != NULL)
        {
            Verbose("Read %s %s\n", clientID, target->Path);
            if (AwaServerReadOperation_AddPath(operation, clientID, target->Path) != AwaError_Success)
            {
                Error("AwaServerReadOperation_AddPath failed\n");
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

static char * ResponseToCString(const AwaServerSession * session, const AwaServerReadResponse * response, Target ** targets, int numTargets, bool quiet)
{
    char * cstring = strdup("");
    AwaObjectID lastObjectID = AWA_INVALID_ID;
    AwaObjectInstanceID lastObjectInstanceID = AWA_INVALID_ID;
    AwaPathIterator * iterator = AwaServerReadResponse_NewPathIterator(response);
    while (AwaPathIterator_Next(iterator))
    {
        const char * path = AwaPathIterator_Get(iterator);

        const AwaPathResult * result = AwaServerReadResponse_GetPathResult(response, path);
        AwaError error = AwaPathResult_GetError(result);
        if (error == AwaError_Success)
        {
            AwaResourceInstanceID resourceInstanceID = AWA_INVALID_ID;
            int targetIndex = 0;
            do
            {
                resourceInstanceID = GetNextTargetResourceInstanceIDFromPath(targets, numTargets, path, &targetIndex);
                Server_AddPathToCString(&cstring, path, session, (void *)response, ResponseType_ReadResponse, quiet, &lastObjectID, &lastObjectInstanceID, resourceInstanceID);
            } while (resourceInstanceID != AWA_INVALID_ID);
        }
        else
        {
            if (error != AwaError_LWM2MError)
            {
                Error("Failed to read from path %s: %s\n", path, AwaError_ToString(error));
            }
            else
            {
                Error("Failed to read from path %s: %s\n", path, AwaLWM2MError_ToString(AwaPathResult_GetLWM2MError(result)));
            }
        }

    }
    AwaPathIterator_Free(&iterator);
    return cstring;
}

static int ProcessReadOperation(const AwaServerSession * session, AwaServerReadOperation * operation, Target ** targets, int numTargets, const char * clientID, bool quiet)
{
    int result = 0;
    const AwaServerReadResponse * response = NULL;
    char * output = NULL;
    if (AwaServerReadOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT) != AwaError_Success)
    {
        Error("AwaServerReadOperation_Perform failed\n");
        result = 1;
    }

    response = AwaServerReadOperation_GetResponse(operation, clientID);
    output = ResponseToCString(session, response, targets, numTargets, quiet);
    if (output != NULL)
    {
        printf("%s", output);
    }
    free(output);
    output = NULL;
    return result;
}

int main(int argc, char ** argv)
{
    int result = 0;
    struct gengetopt_args_info ai;
    AwaServerSession * session = NULL;
    AwaServerReadOperation * operation = NULL;
    Target ** targets = NULL;

    if (cmdline_parser(argc, argv, &ai) != 0)
    {
        exit(1);
    }

    g_logLevel = ai.debug_given ? 2 : (ai.verbose_given ? 1 : 0);
    AwaLog_SetLevel(ai.debug_given ? AwaLogLevel_Debug : (ai.verbose_given ? AwaLogLevel_Verbose : AwaLogLevel_Warning));

    if (ai.inputs_num == 0)
    {
        Error("Specify one or more resource paths.\n");
        result = 1;
        goto cleanup;
    }
    else if (!ai.clientID_given)
    {
        Error("Specify client ID.\n");
        result = 1;
        goto cleanup;
    }

    session = Server_EstablishSession(ai.ipcAddress_arg, ai.ipcPort_arg);
    if (session != NULL)
    {
        // Create Read operation
        operation = AwaServerReadOperation_New(session);
        if (operation == NULL)
        {
            Error("AwaServerReadOperation_New failed\n");
            result = 1;
            goto cleanup;
        }

        targets = malloc(ai.inputs_num * sizeof(Target *));
        // Add target paths from command line
        int i = 0;
        for (i = 0; i < ai.inputs_num; ++i)
        {
            targets[i] = CreateTarget(ai.inputs[i]);
            if (targets[i] != NULL)
            {
                AddTarget(operation, targets[i], ai.clientID_arg);
            }
        }

        result = ProcessReadOperation(session, operation, targets, ai.inputs_num, ai.clientID_arg, ai.quiet_given);
    }
    else
    {
        Error("Failed to establish Awa Session\n");
        result = 1;
    }

cleanup:
    if (operation)
    {
        AwaServerReadOperation_Free(&operation);
    }
    if (session)
    {
        Server_ReleaseSession(&session);
    }
    if (targets)
    {
        int i;
        for (i = 0; i < ai.inputs_num; ++i)
        {
            if (targets[i] != NULL)
            {
                FreeTarget(&targets[i]);
            }
        }
        free(targets);
    }
    cmdline_parser_free(&ai);
    return result;
}

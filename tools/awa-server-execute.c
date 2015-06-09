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


// Execute one or more LWM2M resources
//
//  Usage: fcdm-server-execute [OPTIONS]... [PATHS]...
//
//    -h, --help                 Print help and exit
//    -V, --version              Print version and exit
//    -v, --verbose              Increase program verbosity  (default=off)
//    -d, --debug                Increase program verbosity  (default=off)
//    -p, --ipcPort=PORT         Connect to IPC port  (default=`54321')
//    -p, --ipcPort=PORT         Connect to IPC port  (default=`12345')
//    -c, --clientID=ID          Client ID
//


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <inttypes.h>
#include <unistd.h>

#include "awa/server.h"
#include "awa-server-execute_cmdline.h"
#include "tools_common.h"


static int ProcessExecuteOperation(const AwaServerSession * session, AwaServerExecuteOperation * operation, const char * clientID)
{
    int result = 0;
    if (AwaServerExecuteOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT) != AwaError_Success)
    {
        Error("AwaServerExecuteOperation_Perform failed\n");
        result = 1;
    }

    const AwaServerExecuteResponse * response = AwaServerExecuteOperation_GetResponse(operation, clientID);
    if (response != NULL)
    {
        AwaPathIterator * pathIterator = AwaServerExecuteResponse_NewPathIterator(response);
        if (pathIterator != NULL)
        {
            while (AwaPathIterator_Next(pathIterator))
            {
                const char * path = AwaPathIterator_Get(pathIterator);
                const AwaPathResult * result = AwaServerExecuteResponse_GetPathResult(response, path);
                AwaError error = AwaPathResult_GetError(result);
                if (error == AwaError_Success)
                {
                    printf("Target %s executed successfully\n", path);
                }
                else
                {
                    if (error != AwaError_LWM2MError)
                    {
                        Error("Failed to execute target %s: %s\n", path, AwaError_ToString(error));
                    }
                    else
                    {
                        Error("Failed to execute target %s: %s\n", path, AwaLWM2MError_ToString(AwaPathResult_GetLWM2MError(result)));
                    }
                }
            }
            AwaPathIterator_Free(&pathIterator);
        }
        else
        {
            Error("AwaServerExecuteResponse_NewPathIterator returned NULL");
        }
    }
    else
    {
        Error("AwaServerExecuteOperation_GetResponse returned NULL");
    }
    return result;
}

static void AddTarget(AwaServerExecuteOperation * operation, const char * clientID, const Target * target, AwaExecuteArguments * args)
{
    if (operation != NULL)
    {
        if (target != NULL)
        {
            Verbose("Execute %s\n", target->Path);
            if (AwaServerExecuteOperation_AddPath(operation, clientID, target->Path, args) != AwaError_Success)
            {
                Error("AwaServerGetOperation_AddPath failed\n");
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

int main(int argc, char ** argv)
{
    int result = 0;
    AwaServerSession * session = NULL;
    AwaServerExecuteOperation * operation = NULL;
    struct gengetopt_args_info ai;
    if (cmdline_parser(argc, argv, &ai) != 0)
    {
        result = 1;
        goto cleanup;
    }

    g_logLevel = ai.debug_given ? 2 : (ai.verbose_given ? 1 : 0);
    AwaLog_SetLevel(ai.debug_given ? AwaLogLevel_Debug : (ai.verbose_given ? AwaLogLevel_Verbose : AwaLogLevel_Warning));

    if (ai.inputs_num == 0)
    {
        Error("Specify one or more resource paths.\n");
        result = 1;
        goto cleanup;
    }

    // Create Session
    session = Server_EstablishSession(ai.ipcAddress_arg, ai.ipcPort_arg);
    if (session == NULL)
    {
        Error("EstablishSession failed\n");
        result = 1;
        goto cleanup;
    }

    operation = AwaServerExecuteOperation_New(session);
    if (operation == NULL)
    {
        Error("AwaServerExecuteOperation_New failed\n");
        result = 1;
        goto cleanup;
    }

    AwaExecuteArguments arguments = { .Data = NULL, .Size = 0 };
    if (ai.stdin_given)
    {
        char buf[BUFSIZ];
        int numBytes = 0;
        while (fread(buf+numBytes, 1, 1, stdin) == 1)
        {
            numBytes++;
        }

        arguments.Data = buf;
        arguments.Size = numBytes;

        Verbose("Read payload from stdin: %s [%zu bytes]\n", buf, arguments.Size);
    }
    else
    {
        Verbose("No payload specified.\n");
    }

    int i = 0;
    for (i = 0; i < ai.inputs_num; ++i)
    {
        Target * target = CreateTarget(ai.inputs[i]);
        if (target != NULL)
        {
            AddTarget(operation, ai.clientID_arg, target, &arguments);
            FreeTarget(&target);
        }
    }

    result = ProcessExecuteOperation(session, operation, ai.clientID_arg);

cleanup:
    if (session)
    {
        Server_ReleaseSession(&session);
    }
    if (operation)
    {
        AwaServerExecuteOperation_Free(&operation);
    }
    cmdline_parser_free(&ai);
    return result;
}


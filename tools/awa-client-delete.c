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


// Delete one or more resources from an Object Instance
//
//  Usage: awa-client-delete [OPTIONS]... [PATHS]...
//
//    -h, --help                 Print help and exit
//    -V, --version              Print version and exit
//    -v, --verbose              Increase program verbosity  (default=off)
//    -d, --debug                Increase program verbosity  (default=off)
//    -p, --ipcPort=PORT         Connect to IPC port  (default=`12345')
//    -a, --ipcAddress              IP address of client (default='127.0.0.1')
//
//  Specify one or more object, object instance and resource paths
//  to delete in the format "/O/I/R/i", separated by spaces.
//  For example:
//
//      /3/0/0
//      /3/0/7/1
//      /4/0/3 /4/0/6
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <inttypes.h>

#include "awa-client-delete_cmdline.h"
#include "tools_common.h"


static int ProcessDeleteOperation(AwaClientDeleteOperation * operation)
{
    int result = 0;
    if (AwaClientDeleteOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT) != AwaError_Success)
    {
        Error("AwaClientDeleteOperation_Perform failed\n");
        result = 1;
    }

    const AwaClientDeleteResponse * response = AwaClientDeleteOperation_GetResponse(operation);
    if (response != NULL)
    {
        AwaPathIterator * pathIterator = AwaClientDeleteResponse_NewPathIterator(response);
        if (pathIterator != NULL)
        {
            while (AwaPathIterator_Next(pathIterator))
            {
                const char * path = AwaPathIterator_Get(pathIterator);
                const AwaPathResult * result = AwaClientDeleteResponse_GetPathResult(response, path);
                AwaError error = AwaPathResult_GetError(result);
                if (error == AwaError_Success)
                {
                    Verbose("Target %s deleted\n", path);
                }
                else
                {
                    Error("Failed to delete target %s: %s\n", path, AwaError_ToString(error));
                }
            }
            AwaPathIterator_Free(&pathIterator);
        }
        else
        {
            Error("AwaClientDeleteResponse_NewPathIterator returned NULL");
        }
    }
    else
    {
        Error("AwaClientDeleteOperation_GetResponse returned NULL");
    }

    return result;
}

static int AddTarget(AwaClientDeleteOperation * operation, const Target * target)
{
    int result = 0;
    Verbose("Delete %s\n", target->Path);

    if (AwaClientDeleteOperation_AddPath(operation, target->Path) != AwaError_Success )
    {
         Error("AwaClientDeleteOperation_AddPath failed");
         result = 1;
    }
    return result;
}

int main(int argc, char ** argv)
{
    int result = 0;
    struct gengetopt_args_info ai;
    AwaClientSession * session = NULL;
    AwaClientDeleteOperation * operation  = NULL;

    if (cmdline_parser(argc, argv, &ai) != 0)
    {
        exit(1);
    }

    g_logLevel = ai.debug_given ? 2 : (ai.verbose_given ? 1 : 0);
    AwaLog_SetLevel(ai.debug_given ? AwaLogLevel_Debug : (ai.verbose_given ? AwaLogLevel_Verbose : AwaLogLevel_Warning));

    if (ai.inputs_num == 0)
    {
        printf("Specify one or more resource paths.\n");
        result = 1;
        goto cleanup;
    }

    session = Client_EstablishSession(ai.ipcAddress_arg, ai.ipcPort_arg);
    if (session != NULL)
    {
        // Create delete operation
        operation = AwaClientDeleteOperation_New(session);
        if (operation == NULL)
        {
            Error("AwaClientDeleteOperation_New failed\n");
            result = 1;
            goto cleanup;
        }

        int i = 0;
        for (i = 0; i < ai.inputs_num; ++i)
        {
            Target * target = CreateTarget(ai.inputs[i]);

            if (target != NULL)
            {
                AddTarget(operation, target);
                FreeTarget(&target);
            }
        }
    }
    else
    {
        Error("Failed to establish Awa Session\n");
        result = 1;
        goto cleanup;
    }

    result = ProcessDeleteOperation(operation);

cleanup:
    if (session != NULL)
    {
        Client_ReleaseSession(&session);
    }
    cmdline_parser_free(&ai);
    return result;
}

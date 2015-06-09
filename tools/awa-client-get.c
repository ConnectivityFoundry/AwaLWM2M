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


// Get one or more resources from an Object Instance
//
//  Usage: awa-client-get [OPTIONS]... [PATHS]...
//
//    -h, --help                 Print help and exit
//    -V, --version              Print version and exit
//    -v, --verbose              Increase program verbosity  (default=off)
//    -d, --debug                Increase program verbosity  (default=off)
//    -p, --ipcPort=PORT         Connect to Client IPC port  (default=`12345')
//    -a, --ipcAddress              IP address of client (default='127.0.0.1')
//    -q, --quiet                Print values only (quiet)  (default=off)
//
//  Specify one or more object, object instance and resource paths
//  in the format "/O/I/R", separated by spaces. For example:
//
//  /3/0/0 /4/1 /5
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <inttypes.h>

#include "awa/common.h"
#include "awa/client.h"
#include "awa-client-get_cmdline.h"
#include "tools_common.h"


static void AddTarget(AwaClientGetOperation * operation, const Target * target)
{
    if (operation != NULL)
    {
        if (target != NULL)
        {
            if (IsIDValid(target->ResourceInstanceID))
            {
                Verbose("Get %s/%d\n", target->Path, target->ResourceInstanceID);

                // Use a combination of single-item ranges - may not be the most efficient way in some cases
                if (AwaClientGetOperation_AddPathWithArrayRange(operation, target->Path, target->ResourceInstanceID, 1) != AwaError_Success)
                {
                    Error("AwaClientGetOperation_AddPath failed\n");
                }
            }
            else
            {
                Verbose("Get %s\n", target->Path);
                if (AwaClientGetOperation_AddPath(operation, target->Path) != AwaError_Success)
                {
                    Error("AwaClientGetOperation_AddPath failed\n");
                }
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

static char * ResponseToCString(const AwaClientSession * session, const AwaClientGetResponse * response, bool quiet)
{
    char * cstring = strdup("");
    AwaObjectID lastObjectID = AWA_INVALID_ID;
    AwaObjectInstanceID lastObjectInstanceID = AWA_INVALID_ID;
    AwaPathIterator * iterator = AwaClientGetResponse_NewPathIterator(response);
    while (AwaPathIterator_Next(iterator))
    {
        const char * path = AwaPathIterator_Get(iterator);
        const AwaPathResult * result = AwaClientGetResponse_GetPathResult(response, path);
        AwaError error = AwaPathResult_GetError(result);
        if (error != AwaError_Success)
        {
            Error("Failed to retrieve %s: %s\n", path, AwaError_ToString(error));
        }
        else
        {
            Client_AddPathToCString(&cstring, path, session, (void *)response, ResponseType_GetResponse, quiet, &lastObjectID, &lastObjectInstanceID);
        }
    }
    AwaPathIterator_Free(&iterator);
    return cstring;
}


static int ProcessGetOperation(const AwaClientSession * session, AwaClientGetOperation * operation, bool quiet)
{
    int result = 0;
    const AwaClientGetResponse * response;

    if (AwaClientGetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT) != AwaError_Success)
    {

        Error("AwaClientGetOperation_Perform failed\n");
        result = 1;
    }
    else
    {
        Verbose("Get operation completed successfully.\n");
    }

    response = AwaClientGetOperation_GetResponse(operation);
    if (response != NULL)
    {
        char * output = ResponseToCString(session, response, quiet);
        if (output != NULL)
        {
            printf("%s", output);
        }
        free(output);
        output = NULL;
    }

    return result;
}

int main(int argc, char ** argv)
{
    int result = 0;
    struct gengetopt_args_info ai;
    AwaClientSession * session = NULL;
    AwaClientGetOperation * operation = NULL;

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

    session = Client_EstablishSession(ai.ipcAddress_arg, ai.ipcPort_arg);
    if (session != NULL)
    {
        // Create Get operation
        operation = AwaClientGetOperation_New(session);
        if (operation == NULL)
        {
            Error("AwaClientGetOperation_New failed\n");
            result = 1;
            goto cleanup;
        }

        // Add target paths from command line
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

        result = ProcessGetOperation(session, operation, ai.quiet_given);
    }
    else
    {
        Error("Failed to establish Awa Session\n");
        result = 1;
    }
    
cleanup:
    if (operation != NULL)
    {
        AwaClientGetOperation_Free(&operation);
    }
    if (session != NULL)
    {
        Client_ReleaseSession(&session);
    }
    cmdline_parser_free(&ai);
    return result;
}

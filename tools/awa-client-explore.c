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


// Get one or more resources from an Objects defined
//
//  Usage: awa-client-explore [OPTIONS]... [PATHS]...
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
// /3 /4 /4/0/7 /5/0/0 /5\n"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <inttypes.h>

#include "awa/common.h"
#include "awa/client.h"
#include "awa-client-explore_cmdline.h"
#include "tools_common.h"


static void Explore(const AwaClientSession * session, const Target * target, AwaObjectID * lastObjectIDPrinted, OutputFormat outputFormat)
{
    if (session != NULL)
    {
        AwaObjectID objectID = AWA_INVALID_ID;
        AwaResourceID resourceID = AWA_INVALID_ID;
        if (AwaClientSession_PathToIDs(session, target->Path, &objectID, NULL, &resourceID) == AwaError_Success)
        {
            if (IsIDValid(objectID))
            {
                PrintDefinitionTarget(AwaClientSession_GetObjectDefinition(session, objectID), outputFormat, objectID, resourceID, lastObjectIDPrinted);
            }
            else
            {
                Error("Path %s is not valid\n", target->Path);
            }
        }
        else
        {
            Error("PathToIDs failed\n");
        }
    }
    else
    {
        Error("Session is NULL\n");
    }
}

int main(int argc, char ** argv)
{
    int result = 0;
    struct gengetopt_args_info ai;
    AwaClientSession * session = NULL;
    OutputFormat outputFormat = OutputFormat_None;

    if (cmdline_parser(argc, argv, &ai) != 0)
    {
        exit(1);
    }

    g_logLevel = ai.debug_given ? 2 : (ai.verbose_given ? 1 : 0);
    AwaLog_SetLevel(ai.debug_given ? AwaLogLevel_Debug : (ai.verbose_given ? AwaLogLevel_Verbose : AwaLogLevel_Warning));

    if(ai.export_given)
    {
        outputFormat = OutputFormat_DeviceServerXML;
    }
    else if(ai.quiet_given)
    {
        outputFormat = OutputFormat_PlainTextQuiet;
    }
    else
    {
        outputFormat = OutputFormat_PlainTextVerbose;
    }

    // Establish Awa Session with the daemon
    session = Client_EstablishSession(ai.ipcAddress_arg, ai.ipcPort_arg);
    if (session != NULL)
    {
        if (ai.inputs_num == 0)
        {
            // Print all defined objects and resources
            AwaObjectDefinitionIterator * iterator = AwaClientSession_NewObjectDefinitionIterator(session);
            if (iterator != NULL)
            {
                PrintAllObjectDefinitions(iterator, ai.quiet_given);
            }
            else
            {
                Error("Could not create definition iterator\n");
                result = 1;
            }
        }
        else
        {
            // Add target paths from command line
            int i = 0;
            AwaObjectID lastObjectIDPrinted = AWA_INVALID_ID;
            for (i = 0; i < ai.inputs_num; ++i)
            {
                Target * target = CreateTarget(ai.inputs[i]);
                if (target != NULL)
                {
                    Explore(session, target, &lastObjectIDPrinted, outputFormat);
                    FreeTarget(&target);
                }
            }
        }
    }
    else
    {
        Error("Failed to establish Awa Session\n");
        result = 1;
    }

    // Clean up
    Client_ReleaseSession(&session);
    cmdline_parser_free(&ai);
    return result;
}

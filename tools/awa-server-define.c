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


// Register an Object Definition with the LWM2M Server daemon
//
//  Usage: awa-server-define [OPTIONS]...
//
//    -h, --help                    Print help and exit
//    -V, --version                 Print version and exit
//    -v, --verbose                 Increase program verbosity  (default=off)
//    -d, --debug                   Increase program verbosity  (default=off)
//    -p, --ipcPort=PORT            Connect to IPC port  (default=`54321')
//    -a, --ipcAddress=ADDRESS      IP address of server (default='127.0.0.1')
//
//   Mode: XML Definition
//    -x, --xmlFile=FILENAME        Load XML Definition file
//
//   Mode: Cmdline Definition
//    -o, --objectID=ID             Object ID
//    -j, --objectName=NAME         Object Name
//    -m, --objectMandatory         Object Mandatory  (default=off)
//    -y, --objectInstances=TYPE
//                                  Object Type Instances  (possible
//                                    values="single", "multiple"
//                                    default=`single')
//    -r, --resourceID=ID           Resource ID
//    -n, --resourceName=NAME       Resource Name
//    -t, --resourceType=TYPE       Resource Type  (possible values="opaque",
//                                    "integer", "float", "boolean",
//                                    "string", "time", "objectlink",
//                                    "none")
//    -u, --resourceInstances=VALUE Resource Instances  (possible
//                                    values="single", "multiple")
//    -q, --resourceRequired=VALUE  Resource Optional or Mandatory  (possible
//                                    values="optional", "mandatory")
//    -k, --resourceOperations=VALUE
//                                  Resource Operation  (possible values="r",
//                                    "w", "e", "rw", "rwe")
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

#include "awa/server.h"
#include "awa/common.h"

#include "tools_common.h"
#include "define_common.h"

int main(int argc, char ** argv)
{
    int result = 0;
    struct gengetopt_args_info ai;
    AwaServerSession * session = NULL;
    AwaServerDefineOperation * operation = NULL;

    if (cmdline_parser(argc, argv, &ai) != 0)
    {
        exit(1);
    }

    g_logLevel = ai.debug_given ? 2 : (ai.verbose_given ? 1 : 0);
    AwaLog_SetLevel(ai.debug_given ? AwaLogLevel_Debug : (ai.verbose_given ? AwaLogLevel_Verbose : AwaLogLevel_Warning));

    if (ai.XML_Definition_mode_counter > 0)
    {
        Error("XML file loading is not supported.\n");
        result = 1;
    }
    else if (ai.Cmdline_Definition_mode_counter > 0)
    {
        ObjectDescription * objectDescription = ObjectDescription_NewFromCmdline(&ai);
        if (objectDescription != NULL)
        {
            // Initialise Awa session and connect to IPC
            session = Server_EstablishSession(ai.ipcAddress_arg, ai.ipcPort_arg);
            if (session != NULL)
            {
                operation = AwaServerDefineOperation_New(session);
                if (operation != NULL)
                {
                    AwaObjectDefinition * objectDefinition = DefineObject(objectDescription);
                    if (objectDefinition != NULL)
                    {
                        if (AwaServerDefineOperation_Add(operation, objectDefinition) == AwaError_Success )
                        {
                            if (AwaServerDefineOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT) == AwaError_Success)
                            {
                                Verbose("ObjectID %d defined successfully\n", objectDescription->ID);
                            }
                            else
                            {
                                Error("Define operation failed\n");
                                result = 1;
                            }
                        }
                        else
                        {
                            Error("Could not add object definition to define operation\n");
                            result = 1;
                        }
                        AwaObjectDefinition_Free(&objectDefinition);
                    }
                    else
                    {
                        Error("Failed to create object definition\n");
                        result = 1;
                    }
                }
                else
                {
                    Error("Failed to create define operation\n");
                    result = 1;
                }
            }
            else
            {
                Error("Failed to establish Awa Session\n");
                result = 1;
            }
            ObjectDescription_Free(&objectDescription);
        }
        else
        {
            Error("Failed to create object description from command line\n");
            result = 1;
        }
    }
    else
    {
        // No mode specified
        cmdline_parser_print_help();
        result = 1;
    }

    if (operation)
    {
        AwaServerDefineOperation_Free(&operation);
    }
    if (session)
    {
        Server_ReleaseSession(&session);
    }
    cmdline_parser_free(&ai);
    return result;
}

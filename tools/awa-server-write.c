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


// Write to one or more resources on an Object Instance
//
//  Usage: awa-server-set [OPTIONS]... [PATHS]...
//
//    -h, --help                 Print help and exit
//    -V, --version              Print version and exit
//    -v, --verbose              Increase program verbosity  (default=off)
//    -d, --debug                Enable debug output (default=off)
//    -p, --ipcPort=PORT         Connect to IPC port  (default=`12345')
//    -a, --ipcAddress           IP address of client (default='127.0.0.1')
//
//  Specify one or more object, object instance and resource paths
//  and values in the format "/O/I/R/i=VALUE", separated by spaces.
//  For example:
//
//      /3/0/0=Imagination
//      /3/0/7/1=4200
//      /4/0/3=3 /4/0/6=7
//


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>

#include "awa/server.h"
#include "awa/common.h"
#include "awa-server-write_cmdline.h"
#include "tools_common.h"


#define MAX_RESOURCE_INSTANCEID_SIZE (100)
#define MAX_RESOURCE_VALUE_SIZE      (100)
#define MAX_OBJECT_ID_SIZE           (100)
#define MAX_INSTANCE_ID_SIZE         (100)


static bool SetObjectLinkFromInput(AwaObjectLink * objectLink, const char * value)
{
    int result = sscanf(value, "/%10d/%10d", &objectLink->ObjectID, &objectLink->ObjectInstanceID);
    return (result == 2);
}

static bool GetBooleanValue(const char* value)
{
    bool boolValue;
    // First check for "True" or "False"
    if ((strcasecmp(value, "True") == 0) || (strcasecmp(value, "False") == 0))
    {
        boolValue = !strcasecmp(value, "True");
    }
    else
    {
        // Don't assume the size of _Bool, use an int and convert it
        int tmp = 0;
        sscanf(value, "%10d", &tmp);
        boolValue = tmp == 0 ? false : true;
    }
    return boolValue;
}

static AwaOpaque MakeOpaqueValue(const char * value, int length)
{
    return (AwaOpaque) { .Data = (void *)value, length };
}

static int AddTargetWithValue(const AwaServerSession * session, AwaServerWriteOperation * operation, const Target * target, const char * value)
{
    int result = 0;

    if (operation != NULL)
    {
        if (target != NULL)
        {
            if (value != NULL)
            {
                if (Server_IsResourceDefined(session, target->Path))
                {
                    AwaResourceType resourceType = Server_GetResourceType(session, target->Path);
                    if (resourceType != AwaResourceType_Invalid )
                    {
                        if (IsIDValid(target->ResourceInstanceID))
                        {
                            Verbose("Set %s %s/%d <- %s\n", ResourceTypeToString(resourceType), target->Path, target->ResourceInstanceID, value);
                            if (!IsArrayType(resourceType))
                            {
                                Error("Error: resource %s is not an array; do not specify a resource instance ID\n", target->Path);
                                result = -1;
                            }
                        }
                        else
                        {
                            Verbose("Set %s %s <- %s\n", ResourceTypeToString(resourceType), target->Path, value);
                            if (IsArrayType(resourceType))
                            {
                                Error("Error: resource is an array; specify a resource instance ID: %s/x\n", target->Path);
                                result = -1;
                            }
                        }

                        if (result == 0)
                        {
                            switch (resourceType)
                            {
                                case AwaResourceType_None:
                                {
                                    Error("Resource %s is of type None and cannot be set\n", target->Path);
                                    result = -1;
                                    break;
                                }

                                case AwaResourceType_String:
                                {
                                    if (AwaServerWriteOperation_AddValueAsCString(operation, target->Path, value) != AwaError_Success)
                                    {
                                        Error("AwaServerWriteOperation_AddValueAsCString failed\n");
                                        result = -1;
                                    }
                                    break;
                                }

                                case AwaResourceType_Integer:
                                {
                                    AwaInteger tmp = strtoimax(value, NULL, 10);
                                    if (AwaServerWriteOperation_AddValueAsInteger(operation, target->Path, tmp) != AwaError_Success)
                                    {
                                        Error("AwaServerWriteOperation_AddValueAsInteger failed\n");
                                        result = -1;
                                    }
                                    break;
                                }

                                case AwaResourceType_Float:
                                {
                                    AwaFloat tmp = 0.0;
                                    sscanf(value, "%20lf", &tmp);
                                    if (AwaServerWriteOperation_AddValueAsFloat(operation, target->Path, tmp) != AwaError_Success)
                                    {
                                        Error("AwaServerWriteOperation_AddValueAsFloat failed\n");
                                        result = -1;
                                    }
                                    break;
                                }

                                case AwaResourceType_Boolean:
                                {
                                    AwaBoolean boolValue = GetBooleanValue(value);
                                    if (AwaServerWriteOperation_AddValueAsBoolean(operation, target->Path, boolValue) != AwaError_Success)
                                    {
                                        Error("AwaServerWriteOperation_AddValueAsBoolean failed\n");
                                        result = -1;
                                    }
                                    break;
                                }

                                case AwaResourceType_Opaque:
                                {
                                    AwaOpaque opaqueValue = MakeOpaqueValue(value, strlen(value));
                                    if (AwaServerWriteOperation_AddValueAsOpaque(operation, target->Path, opaqueValue) != AwaError_Success)
                                    {
                                        Error("AwaServerWriteOperation_AddValueAsOpaque failed\n");
                                        result = -1;
                                    }
                                    break;
                                }

                                case AwaResourceType_Time:
                                {
                                    AwaTime tmp = strtoimax(value, NULL, 10);
                                    if (AwaServerWriteOperation_AddValueAsTime(operation, target->Path, tmp) != AwaError_Success)
                                    {
                                        Error("AwaServerWriteOperation_AddValueAsTime failed\n");
                                        result = -1;
                                    }
                                    break;
                                }

                                case AwaResourceType_ObjectLink:
                                {
                                    AwaObjectLink objectLink;
                                    if (!SetObjectLinkFromInput(&objectLink, value))
                                    {
                                        Error("AwaServerWriteOperation_AddValueAsObjectLink failed\n");
                                        result = -1;
                                        break;
                                    }

                                    if (AwaServerWriteOperation_AddValueAsObjectLink(operation, target->Path, objectLink) != AwaError_Success)
                                    {
                                        Error("AwaServerWriteOperation_AddValueAsObjectLink failed\n");
                                        result = -1;
                                    }
                                    break;
                                }

                                case AwaResourceType_IntegerArray:
                                {
                                    AwaInteger tmp = strtoimax(value, NULL, 10);
                                    if (AwaServerWriteOperation_AddArrayValueAsInteger(operation, target->Path, target->ResourceInstanceID, tmp) != AwaError_Success)
                                    {
                                        Error("AwaServerWriteOperation_AddArrayValueAsInteger failed\n");
                                        result = -1;
                                    }
                                    break;
                                }

                                case AwaResourceType_StringArray:
                                {
                                    if (AwaServerWriteOperation_AddArrayValueAsCString(operation, target->Path, target->ResourceInstanceID, value) != AwaError_Success)
                                    {
                                        Error("AwaServerWriteOperation_AddArrayValueAsCString failed\n");
                                        result = -1;
                                    }
                                    break;
                                }

                                case AwaResourceType_FloatArray:
                                {
                                    AwaFloat tmp = 0.0;
                                    sscanf(value, "%20lf", &tmp);
                                    if (AwaServerWriteOperation_AddArrayValueAsFloat(operation, target->Path, target->ResourceInstanceID, tmp) != AwaError_Success)
                                    {
                                        Error("AwaServerWriteOperation_AddArrayValueAsFloat failed\n");
                                        result = -1;
                                    }
                                    break;
                                }

                                case AwaResourceType_BooleanArray:
                                {
                                    AwaBoolean boolValue = GetBooleanValue(value);
                                    if (AwaServerWriteOperation_AddArrayValueAsBoolean(operation, target->Path, target->ResourceInstanceID, boolValue) != AwaError_Success)
                                    {
                                        Error("AwaServerWriteOperation_AddArrayValueAsBoolean failed\n");
                                        result = -1;
                                    }
                                    break;
                                }

                                case AwaResourceType_OpaqueArray:
                                {
                                    AwaOpaque opaqueValue = MakeOpaqueValue(value, strlen(value));
                                    if (AwaServerWriteOperation_AddArrayValueAsOpaque(operation, target->Path, target->ResourceInstanceID, opaqueValue) != AwaError_Success)
                                    {
                                        Error("AwaServerWriteOperation_AddArrayValueAsOpaque failed\n");
                                        result = -1;
                                    }
                                    break;
                                }

                                case AwaResourceType_TimeArray:
                                {
                                    AwaTime tmp = strtoimax(value, NULL, 10);
                                    if (AwaServerWriteOperation_AddArrayValueAsTime(operation, target->Path, target->ResourceInstanceID, tmp) != AwaError_Success)
                                    {
                                        Error("AwaServerWriteOperation_AddArrayValueAsTime failed\n");
                                        result = -1;
                                    }
                                    break;
                                }

                                case AwaResourceType_ObjectLinkArray:
                                {
                                    AwaObjectLink objectLink;
                                    if (!SetObjectLinkFromInput(&objectLink, value))
                                    {
                                        Error("AwaServerWriteOperation_AddArrayValueAsObjectLink failed\n");
                                        result = -1;
                                        break;
                                    }

                                    if (AwaServerWriteOperation_AddArrayValueAsObjectLink(operation, target->Path, target->ResourceInstanceID, objectLink) != AwaError_Success)
                                    {
                                        Error("AwaServerWriteOperation_AddArrayValueAsObjectLink failed\n");
                                        result = -1;
                                    }
                                    break;
                                }

                                default:
                                    Error("Unhandled type\n");
                                    result = -1;
                            }
                        }
                    }
                }
                else
                {
                    Error("%s is not defined\n", target->Path);
                    result = -1;
                }
            }
            else
            {
                Error("value is NULL\n");
                result = -1;
            }
        }
        else
        {
            Error("target is NULL\n");
            result = -1;
        }
    }
    else
    {
        Error("operation is NULL\n");
        result = -1;
    }
    return result;
}

static int ProcessWriteOperation(AwaServerWriteOperation * operation, const char * clientID)
{
    int result = 0;
    if (AwaServerWriteOperation_Perform(operation, clientID, OPERATION_PERFORM_TIMEOUT) != AwaError_Success)
    {
        Error("AwaServerWriteOperation_Perform failed\n");
        result = 1;
    }
    else
    {
        Verbose("Write operation completed successfully.\n");
    }

    const AwaServerWriteResponse * response = AwaServerWriteOperation_GetResponse(operation, clientID);
    if (response != NULL)
    {
        AwaPathIterator * iterator = AwaServerWriteResponse_NewPathIterator(response);
        while (AwaPathIterator_Next(iterator))
        {
            const char * path = AwaPathIterator_Get(iterator);
            const AwaPathResult * result = AwaServerWriteResponse_GetPathResult(response, path);
            AwaError error = AwaPathResult_GetError(result);
            if (error != AwaError_Success)
            {
                if (error != AwaError_LWM2MError)
                {
                    Error("Failed to write to path %s: %s\n", path, AwaError_ToString(error));
                }
                else
                {
                    Error("Failed to write to path %s: %s\n", path, AwaLWM2MError_ToString(AwaPathResult_GetLWM2MError(result)));
                }
            }
        }
        AwaPathIterator_Free(&iterator);
    }
    return result;
}

static int CreateTargets(const AwaServerSession * session, AwaServerWriteOperation * operation, const char * clientID, char ** targets, unsigned int numCreates)
{
    int count = 0;
    int i = 0;

    // Handle object instance creates
    Debug("Filtering for Object Instance creates\n");
    for (i = 0; i < numCreates; ++i)
    {
        Target * target = CreateTarget(targets[i]);
        if (Server_IsObjectTarget(session, target) || Server_IsObjectInstanceTarget(session, target))
        {
            if (AwaServerWriteOperation_CreateObjectInstance(operation, target->Path) == AwaError_Success)
            {
                Verbose("Create %s\n", target->Path);
                ++count;
            }
        }
        else
        {
            // Skip it
            Debug("Skipping %s (%d)\n", targets[i], i);
        }
        FreeTarget(&target);
    }
    return count;
}

int main(int argc, char ** argv)
{
    int result = 1;
    struct gengetopt_args_info ai; 
    AwaServerSession * session = NULL;
    AwaServerWriteOperation * operation = NULL;
    char address[128];
    unsigned int port;
    if (cmdline_parser(argc, argv, &ai) != 0)
    {
        result = 1;
        goto cleanup;
    }

    g_logLevel = ai.debug_given ? 2 : (ai.verbose_given ? 1 : 0);
    AwaLog_SetLevel(ai.debug_given ? AwaLogLevel_Debug : (ai.verbose_given ? AwaLogLevel_Verbose : AwaLogLevel_Warning));

    if (ai.inputs_num == 0 && ai.create_given == 0)
    {
        Error("Specify one or more resource paths.\n");
        result = 1;
        goto cleanup;
    }

    port = ai.ipcPort_arg;
    strncpy(address, ai.ipcAddress_arg, strlen(ai.ipcAddress_arg)+1);

    // Establish Awa Session with the daemon
    session = Server_EstablishSession(address, port);
    if (session == NULL)
    {
        Error("Failed to establish Awa Session\n");
        result = 1;
        goto cleanup;
    }

    operation = AwaServerWriteOperation_New(session, AwaWriteMode_Update);
    if (operation == NULL)
    {
        Error("AwaServerWriteOperation_New failed\n");
        Server_ReleaseSession(&session);
        result = 1;
        goto cleanup;
    }

    // Add create directives first
    int count = 0;
    count = CreateTargets(session, operation, ai.clientID_arg, ai.create_arg, ai.create_given);

    // Add target paths and values from the command line
    int i = 0;
    for (i = 0; i < ai.inputs_num; ++i)
    {
        Target * target = CreateTarget(ai.inputs[i]);
        if (target != NULL)
        {
            char * value = Server_GetValue(session, target, ai.inputs[i]);
            if (value != NULL)
            {
                if (AddTargetWithValue(session, operation, target, value) == 0)
                {
                    ++count;
                }

                free(value);
                value = NULL;
            }
            FreeTarget(&target);
        }
    }
    if (count > 0)
    {
        result = ProcessWriteOperation(operation, ai.clientID_arg);
    }

cleanup:
    if (operation)
    {
        AwaServerWriteOperation_Free(&operation);
    }
    if (session)
    {
        Server_ReleaseSession(&session);
    }
    cmdline_parser_free(&ai);
    return result;
}

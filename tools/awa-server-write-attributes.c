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


// WriteAttributes Attributes to one or more resources on an Object Instance
//
//Usage: awa-server-write-attributes [OPTIONS]... [PATHS]...
//
//  -h, --help                Print help and exit
//  -V, --version             Print version and exit
//  -v, --verbose             Increase program verbosity  (default=off)
//  -d, --debug               Increase program verbosity  (default=off)
//  -a, --ipcAddress=ADDRESS  Connect to Server IPC Address
//                              (default=`127.0.0.1')
//  -p, --ipcPort=PORT        Connect to Server IPC port  (default=`54321')
//  -c, --clientID=ID         Client ID
//
//Specify one or more object, object instance and resource paths
//with query parameters in the format "/O/I/R?LINK=VALUE&LINK2=VALUE2",
//separated by spaces.
//For example:
//
//    /3/0/13?gt=5
//    /3/0/13?pmin=2&pmax=10


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>

#include "awa/server.h"
#include "awa/common.h"
#include "awa-server-write-attributes_cmdline.h"
#include "tools_common.h"


#define MAX_RESOURCE_INSTANCEID_SIZE (100)
#define MAX_RESOURCE_VALUE_SIZE      (100)
#define MAX_OBJECT_ID_SIZE           (100)
#define MAX_INSTANCE_ID_SIZE         (100)


typedef struct
{
    char * Key;
    char * Value;
} QueryPair;

typedef struct
{
    char * Path;
    QueryPair * QueryPairs;
    int NumPairs;
} WriteAttributesTarget;


static int CountQueryPairs(const char * query)
{
    int count = 0;
    if (query != NULL)
    {
        char * str = strdup(query);
        const char delim[] = "&?";
        char * token = strtok(str, delim);
        while (token != NULL)
        {
            count++;
            token = strtok(NULL, delim);
        }
        free(str);
    }
    return count;
}

QueryPair * SplitQuery(const char * query, int * numPairs)
{
    *numPairs = CountQueryPairs(query);
    QueryPair * pairs = NULL;
    if (*numPairs > 0)
    {
        pairs = malloc(*numPairs * sizeof(QueryPair));
        if (pairs != NULL)
        {
            int count = 0;
            char * str = strdup(query);
            const char delim[] = "&?";
            char * token = strtok(str, delim);
            while (token != NULL)
            {
                QueryPair * pair = &pairs[count];

                char * equals = strchr(token, '=');
                if (equals == NULL)
                {
                    pair->Key = strdup(token);
                    pair->Value = NULL;
                }
                else
                {
                    int equalsPos = (int)(equals - token);
                    pair->Key = (char *)malloc((equalsPos+1) * sizeof(char));
                    memcpy(pair->Key, token, equalsPos);
                    pair->Key[equalsPos] = '\0';
                    pair->Value = strdup(equals + 1);
                }

                token = strtok(NULL, delim);
                count++;
            }
            free(str);
        }
        else
        {
            Error("Failed to allocate memory for SplitUpQuery");
        }
    }
    return pairs;
}

void FreeQueryPairs(QueryPair * pairs, int numPairs)
{
    int i;
    if (pairs == NULL)
    {
        return;
    }
    for (i = 0; i < numPairs; i++)
    {
        QueryPair * pair = &pairs[i];
        free(pair->Key);
        free(pair->Value);
    }
    free(pairs);
}

static int AddWriteAttributesTarget(const AwaServerSession * session, AwaServerWriteAttributesOperation * operation, const char * clientID, const WriteAttributesTarget * target)
{
    int result = 0;

    if (operation != NULL)
    {
        if (target != NULL)
        {
            int i;
            for (i = 0; i < target->NumPairs; i++)
            {
                int integerValue = 0;
                float floatValue = 0;

                QueryPair * pair = &target->QueryPairs[i];

                if (strcasecmp(pair->Key, "cancel") == 0)
                {
                    // LWM2M spec 1.0 20141126-C:
                    //   Add "Cancel" attribute to cancel observation. No way to add as "None" type so add as Integer with value zero.
                    AwaServerWriteAttributesOperation_AddAttributeAsInteger(operation, clientID, target->Path, pair->Key, 0);
                }
                else if ((pair->Value != NULL) && (sscanf(pair->Value, "%24d", &integerValue) != 0))
                {
                    Verbose("Write Attribute to %s as Integer: %s = %d\n", target->Path, pair->Key, integerValue);
                    AwaServerWriteAttributesOperation_AddAttributeAsInteger(operation, clientID, target->Path, pair->Key, integerValue);
                }
                else if ((pair->Value != NULL) && (sscanf(pair->Value, "%24f", &floatValue) != 0))
                {
                    Verbose("Write Attribute to %s as Float: %s =%f\n", target->Path, pair->Key, floatValue);
                    AwaServerWriteAttributesOperation_AddAttributeAsFloat(operation, clientID, target->Path, pair->Key, floatValue);
                }
                else
                {
                    Error("Failed to parse value of query link: %s/x\n", target->Path);
                    result = -1;
                }
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

static int ProcessWriteAttributesOperation(AwaServerWriteAttributesOperation * operation, const char * clientID)
{
    int result = 0;
    if (AwaServerWriteAttributesOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT) != AwaError_Success)
    {
        Error("AwaServerWriteAttributesOperation_Perform failed\n");
        result = 1;
    }
    else
    {
        Verbose("Write Attributes operation completed successfully.\n");
    }

    const AwaServerWriteAttributesResponse * response = AwaServerWriteAttributesOperation_GetResponse(operation, clientID);
    if (response != NULL)
    {
        AwaPathIterator * iterator = AwaServerWriteAttributesResponse_NewPathIterator(response);
        while (AwaPathIterator_Next(iterator))
        {
            const char * path = AwaPathIterator_Get(iterator);
            const AwaPathResult * result = AwaServerWriteAttributesResponse_GetPathResult(response, path);
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

static int CountSlashes(const char * str)
{
    int i, count;
    for (i = 0, count = 0; str[i] != '\0'; ++i)
    {
        count += (str[i] == '/');
    }
    return count;
}

static WriteAttributesTarget * CreateWriteAttributesTarget(const char * arg)
{
    WriteAttributesTarget * target = NULL;
    int numPairs;

    const char * query = strchr(arg, '?');
    if (query == NULL)
    {
        Error("Target %s is not valid\n", arg);
        return NULL;
    }

    QueryPair * queryPairs = SplitQuery(query, &numPairs);
    if (queryPairs == NULL)
    {
        Error("Target %s is not valid\n", arg);
        return NULL;
    }

    if ((arg[0] != '/') || (CountSlashes(arg) > 3))
    {
        Error("Target %s is not valid\n", arg);
        free(queryPairs);
        return NULL;
    }

    AwaObjectID objectID = AWA_INVALID_ID;
    AwaObjectInstanceID objectInstanceID = AWA_INVALID_ID;
    AwaResourceID resourceID = AWA_INVALID_ID;

    sscanf(arg, "/%10d/%10d/%10d", &objectID, &objectInstanceID, &resourceID);

    char path[256] = { 0 };
    size_t pathSize = 256;
    if (AwaAPI_MakePath(path, pathSize, objectID, objectInstanceID, resourceID) == AwaError_Success)
    {
        if (AwaAPI_IsPathValid(path))
        {
            target = malloc(sizeof(*target));
            if (target != NULL)
            {
                memset(target, 0, sizeof(*target));
                target->Path = strdup(path);
                target->NumPairs = numPairs;
                target->QueryPairs = queryPairs;
            }
            else
            {
                Error("Out of memory\n");
                free(queryPairs);
            }
        }
        else
        {
            Error("Path %s is not valid\n", path);
            free(queryPairs);
        }
    }
    else
    {
        Error("WriteAttributesTarget %s is not valid\n", arg);
        free(queryPairs);
    }
    return target;
}

void FreeWriteAttributesTarget(WriteAttributesTarget ** targetPtr)
{
    if ((targetPtr != NULL) && (*targetPtr != NULL))
    {
        free((*targetPtr)->Path);
        FreeQueryPairs((*targetPtr)->QueryPairs, (*targetPtr)->NumPairs);
        (*targetPtr)->Path = NULL;
        free(*targetPtr);
        *targetPtr = NULL;
    }
}

int main(int argc, char ** argv)
{
    int result = 0;
    struct gengetopt_args_info ai; 
    AwaServerSession * session = NULL;
    AwaServerWriteAttributesOperation * operation = NULL;
    char address[128];
    unsigned int port;
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

    operation = AwaServerWriteAttributesOperation_New(session);
    if (operation == NULL)
    {
        Error("AwaServerWriteAttributesOperation_New failed\n");
        Server_ReleaseSession(&session);
        result = 1;
        goto cleanup;
    }

    // Add target paths and values from the command line
    int i = 0;
    for (i = 0; i < ai.inputs_num; ++i)
    {
        WriteAttributesTarget * target = CreateWriteAttributesTarget(ai.inputs[i]);
        if (target != NULL)
        {
            AddWriteAttributesTarget(session, operation, ai.clientID_arg, target);
            FreeWriteAttributesTarget(&target);
        }
    }

    result = ProcessWriteAttributesOperation(operation, ai.clientID_arg);

cleanup:
    if (operation)
    {
        AwaServerWriteAttributesOperation_Free(&operation);
    }
    if (session)
    {
        Server_ReleaseSession(&session);
    }
    cmdline_parser_free(&ai);
    return result;
}

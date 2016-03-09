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


// Observe to Change
//
//  Usage: awa-server-observe [OPTIONS]... [PATHS]...
//
//    -h, --help                 Print help and exit
//    -V, --version              Print version and exit
//    -v, --verbose              Increase program verbosity  (default=off)
//    -d, --debug                Increase program verbosity  (default=off)
//    -a, --ipcAddress           IP address of client (default='127.0.0.1')
//    -p, --ipcPort=PORT         Connect to Server IPC port  (default=`12345')
//    -c, --clientID=ID          Server ID
//    -t, --waitTime=SECONDS     Time to wait for notification  (default=`0')
//    -c, --waitCount=NUMBER     Number of notifications to wait for  (default=`0')
//


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <inttypes.h>
#include <signal.h>

#include "awa/common.h"
#include "awa/server.h"
#include "awa-server-observe_cmdline.h"
#include "tools_common.h"
#include "changeset_common.h"


struct _ObservationNode
{
    void * observation;
    struct _ObservationNode * next;
};

typedef struct _ObservationNode ObservationNode;

typedef struct
{
    bool quiet;
    Target ** targets;
    int numTargets;
} ObserveContext;

static int g_notifyCount = 0;
static ObservationNode * g_observationListHead = NULL;


bool AddObservation(void * observation)
{
    if (g_observationListHead == NULL)
    {
         g_observationListHead = (ObservationNode *)malloc(sizeof(ObservationNode));
         if(g_observationListHead != NULL)
         {
             g_observationListHead->observation = observation;
             g_observationListHead->next = NULL;
             return true;
         }
         else
         {
             return false;
         }
    }
    else
    {
         ObservationNode * currentObservation = g_observationListHead;

         while(currentObservation->next != NULL)
         {
             currentObservation = currentObservation->next;
         }

         currentObservation->next = (ObservationNode *)malloc(sizeof(ObservationNode));

         if(currentObservation->next != NULL)
         {
             currentObservation->next->observation = observation;
             currentObservation->next->next = NULL;
             return true;
         }
         else
         {
             return true;
         }
    }
}

void FreeObservation(ObservationNode **node)
{
    free(*node);
    *node = NULL;
}

void FreeObservationList()
{
    ObservationNode * currentObservation = g_observationListHead;
    ObservationNode * nextObservation;
    while (currentObservation != NULL)
    {
        nextObservation = currentObservation->next;
        FreeObservation(&currentObservation);
        currentObservation = nextObservation;
    }
}

static char * ChangeSetToCString(const AwaServerSession * session, const AwaChangeSet * changeSet, Target ** targets, int numTargets, bool quiet)
{
    char * cstring = strdup("");
    AwaObjectID lastObjectID = AWA_INVALID_ID;
    AwaObjectInstanceID lastObjectInstanceID = AWA_INVALID_ID;
    AwaPathIterator * iterator = AwaChangeSet_NewPathIterator(changeSet);
    while (AwaPathIterator_Next(iterator))
    {
        const char * path = AwaPathIterator_Get(iterator);
        AwaChangeType changeType = AwaChangeSet_GetChangeType(changeSet, path);
        PrintChanged(changeType, path);
        if ((changeType == AwaChangeType_ResourceCreated) || (changeType == AwaChangeType_ResourceModified))
        {
            AwaResourceInstanceID resourceInstanceID = AWA_INVALID_ID;
            int targetIndex = 0;
            do
            {
                resourceInstanceID = GetNextTargetResourceInstanceIDFromPath(targets, numTargets, path, &targetIndex);
                Server_AddPathToCString(&cstring, path, (const AwaServerSession * )session, (void *)changeSet, ResponseType_ChangeSet, quiet, &lastObjectID, &lastObjectInstanceID, resourceInstanceID);
            } while (resourceInstanceID != AWA_INVALID_ID);
        }
    }
    AwaPathIterator_Free(&iterator);
    return cstring;
}

void ObserveCallback(const AwaChangeSet * changeSet, void * context)
{
    Verbose("Notify %d from clientID %s:\n", g_notifyCount, AwaChangeSet_GetClientID(changeSet));
    const AwaServerSession * session = AwaChangeSet_GetServerSession(changeSet);
    ObserveContext * observeContext = (ObserveContext *)context;

    char * output = ChangeSetToCString(session, changeSet, observeContext->targets, observeContext->numTargets, observeContext->quiet);
    if (output != NULL)
    {
        printf("%s", output);
        // Provide output immediately
        fflush(stdout);
    }
    free(output);
    output = NULL;
    g_notifyCount++;
}

static bool ObserveTarget(AwaServerObserveOperation * operation, const char * clientID, const Target * target, void * context)
{
    bool result = false;

    if (operation != NULL)
    {
        if (target != NULL)
        {
            Verbose("Observe %s\n", target->Path);
            AwaServerObservation * observation = AwaServerObservation_New(clientID, target->Path, ObserveCallback, context);
            if (observation != NULL)
            {
                if (AwaServerObserveOperation_AddObservation(operation, observation) == AwaError_Success)
                {
                    AddObservation(observation);
                    result = true;
                }
                else
                {
                    Error("AwaServerObserveOperation_AddObservation failed\n");
                }
            }
            else
            {
                Error("AwaServerObservation_New failed\n");
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

    return result;
}

static void CancelObservationFromTargets(AwaServerSession * session, const char * clientID)
{
    AwaServerObserveOperation * operation = AwaServerObserveOperation_New(session);
    if (operation == NULL)
    {
        Error("Failed to create observe operation\n");
        exit(1);
    }

    // Add cancels for each observation
    ObservationNode * currentObservation = g_observationListHead;
    ObservationNode * nextObservation;
    while(currentObservation != NULL)
    {
        nextObservation = currentObservation->next;

        if (AwaServerObserveOperation_AddCancelObservation(operation, currentObservation->observation) != AwaError_Success)
        {
            Error("AwaServerObserveOperation_AddCancelChangeObservation() failed for path %s\n", AwaServerObservation_GetPath(currentObservation->observation));
        }
        currentObservation = nextObservation;
    }

    if (AwaServerObserveOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT) != AwaError_Success)
    {
        Error("Failed to perform observe operation\n");
    }

    // Check response for each observation
    const AwaServerObserveResponse * response = AwaServerObserveOperation_GetResponse(operation, clientID);
    currentObservation = g_observationListHead;
    while(currentObservation != NULL)
    {
        nextObservation = currentObservation->next;
        const AwaPathResult * result = NULL;

        AwaServerObservation * observation = (AwaServerObservation *)(currentObservation->observation);
        const char * path = AwaServerObservation_GetPath(observation);

        result = AwaServerObserveResponse_GetPathResult(response, path);
        if (AwaPathResult_GetError(result) != AwaError_Success)
        {
            Error("Failed to cancel observation to %s: %s\n", path, AwaError_ToString(AwaPathResult_GetError(result)));
        }

        currentObservation = nextObservation;
    }
    AwaServerObserveOperation_Free(&operation);

    // Finally free each observation
    currentObservation = g_observationListHead;
    while(currentObservation != NULL)
    {
        nextObservation = currentObservation->next;

        AwaServerObservation * observation = (AwaServerObservation *)(currentObservation->observation);
        if (AwaServerObservation_Free(&observation) != AwaError_Success)
        {
            Error("AwaServerObservation_Free() failed for path %s\n", AwaServerObservation_GetPath(observation));
        }
        currentObservation = nextObservation;
    }
}

static void Wait(AwaServerSession * session, int waitTime, int waitCount)
{
    int ticks = 0;

    Verbose("Waiting for");
    if (waitCount > 0)
    {
        Verbose(" %d", waitCount);
    }
    Verbose(" notifications");
    if (waitTime > 0)
    {
        Verbose(" over %d seconds", waitTime);
    }
    Verbose(":\n");

    // Implement timeout
    while(((waitCount > 0) ? (g_notifyCount <= waitCount) : true) && // Notify 0 is the initial value
          (g_signal == false) &&
          (waitTime == 0 || ticks < waitTime))
    {
        if (AwaServerSession_Process(session, 1000 /* 1 second */) != AwaError_Success)
        {
            printf("AwaServerSession_Process() failed\n");
            break;
        }
        AwaServerSession_DispatchCallbacks(session);
        ticks++;
    }
}

int main(int argc, char ** argv)
{
    int result = 0;
    AwaServerObserveOperation * operation;
    struct gengetopt_args_info ai;
    AwaServerSession * session = NULL;
    Target ** targets = NULL;

    // Catch CTRL-C to ensure clean-up
    signal(SIGINT, INThandler);

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

    session = Server_EstablishSession(ai.ipcAddress_arg, ai.ipcPort_arg);
    if (session != NULL)
    {
        operation = AwaServerObserveOperation_New(session);
        if (operation == NULL)
        {
            Error("Failed to create observe operation\n");
            exit(1);
        }

        targets = malloc(ai.inputs_num * sizeof(Target *));

        ObserveContext observeContext;
        observeContext.targets = targets;
        observeContext.numTargets = ai.inputs_num;
        observeContext.quiet = ai.quiet_given; // pass the quiet parameter as our context
        int count = 0;
        int i = 0;
        for (i = 0; i < ai.inputs_num; ++i)
        {
            targets[i] = CreateTarget(ai.inputs[i]);
            if (targets[i] != NULL)
            {
                count = ObserveTarget(operation, ai.clientID_arg, targets[i], &observeContext) ? count + 1 : count;
            }
        }

        if (AwaServerObserveOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT) != AwaError_Success)
        {
            Error("Failed to perform observe operation\n");
            goto cleanup;
        }

        int validCount = count;
        const AwaServerObserveResponse * response = AwaServerObserveOperation_GetResponse(operation, ai.clientID_arg);
        ObservationNode * currentObservation = g_observationListHead;
        ObservationNode * nextObservation;
        while (currentObservation != NULL)
        {
            nextObservation = currentObservation->next;
            const AwaPathResult * pathResult = NULL;

            AwaServerObservation * observation = (AwaServerObservation *)(currentObservation->observation);
            const char * path = AwaServerObservation_GetPath(observation);

            pathResult = AwaServerObserveResponse_GetPathResult(response, path);
            if (AwaPathResult_GetError(pathResult) != AwaError_Success)
            {
                Error("Failed to observe to %s: %s\n", path, AwaError_ToString(AwaPathResult_GetError(pathResult)));
                validCount--;
            }
            currentObservation = nextObservation;
        }

        AwaServerObserveOperation_Free(&operation);

        // Wait if there's something to wait for
        Debug("count %d\n", count);
        if (count > 0)
        {
            if (validCount > 0)
            {
                Wait(session, ai.waitTime_arg, ai.waitCount_arg);
            }
            CancelObservationFromTargets(session, ai.clientID_arg);
        }
    }
    else
    {
        Error("Failed to establish Awa Session\n");
        result = 1;
    }
cleanup:
    if (session)
    {
        FreeObservationList();
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

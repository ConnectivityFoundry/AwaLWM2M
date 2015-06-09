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


// Subscribe to Change or Execute
//
//  Usage: awa-client-subscribe [OPTIONS]... [PATHS]...
//
//    -h, --help                 Print help and exit
//    -V, --version              Print version and exit
//    -v, --verbose              Increase program verbosity  (default=off)
//    -d, --debug                Increase program verbosity  (default=off)
//    -p, --ipcPort=PORT         Connect to IPC port  (default=`12345')
//    -a, --ipcAddress              IP address of client (default='127.0.0.1')
//    -t, --waitTime=SECONDS     Time to wait for notification  (default=`0')
//    -c, --waitCount=NUMBER     Number of notifications to wait for  (default=`0')
//


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <inttypes.h>
#include <signal.h>
#include <stddef.h>

#include "awa-client-subscribe_cmdline.h"
#include "tools_common.h"
#include "changeset_common.h"


struct _SubscriptionNode
{
    AwaSubscribeType type;
    void * subscription;
    struct _SubscriptionNode * next;
};

typedef struct _SubscriptionNode SubscriptionNode;

static int g_notifyCount = 0;
static SubscriptionNode * g_subscriptionListHead = NULL;


bool AddSubscription(AwaSubscribeType subscriptionType, void * subscription)
{
    if (g_subscriptionListHead == NULL)
    { 
         g_subscriptionListHead = (SubscriptionNode *)malloc(sizeof(SubscriptionNode));
         if (g_subscriptionListHead != NULL)
         {
             g_subscriptionListHead->type = subscriptionType;
             g_subscriptionListHead->subscription = subscription;
             g_subscriptionListHead->next = NULL;
             return true;
         }
         else
         {
             return false;
         }
    }
    else
    {
         SubscriptionNode * currentSubscription = g_subscriptionListHead;

         while(currentSubscription->next != NULL)
         {
             currentSubscription = currentSubscription->next;
         }

         currentSubscription->next = (SubscriptionNode *)malloc(sizeof(SubscriptionNode));

         if (currentSubscription->next != NULL)
         {
             currentSubscription->next->type = subscriptionType,
             currentSubscription->next->subscription = subscription;
             currentSubscription->next->next = NULL;
             return true;
         }
         else
         {
             return true;
         }
    }
}

void FreeSubscription(SubscriptionNode **node)
{
    free(*node);
    *node = NULL;
}

void FreeSubscriptionList()
{
    SubscriptionNode * currentSubscription = g_subscriptionListHead;
    SubscriptionNode * nextSubscription;
    while( currentSubscription != NULL)
    {
        nextSubscription = currentSubscription->next;
        FreeSubscription(&currentSubscription);
        currentSubscription = nextSubscription;
    }
}

static char * ChangeSetToCString(const AwaClientSession * session, const AwaChangeSet * changeSet, bool quiet)
{
    char * cstring = strdup("");
    AwaObjectID lastObjectID = AWA_INVALID_ID;
    AwaObjectInstanceID lastObjectInstanceID = AWA_INVALID_ID;
    AwaPathIterator * iterator = AwaChangeSet_NewPathIterator(changeSet);
    while (AwaPathIterator_Next(iterator))
    {
        const char * path = AwaPathIterator_Get(iterator);
        AwaChangeType  changeType = AwaChangeSet_GetChangeType(changeSet, path);
        PrintChanged(changeType, path);
        if ((changeType == AwaChangeType_ResourceCreated) || (changeType == AwaChangeType_ResourceModified))
        {
            Client_AddPathToCString(&cstring, path, (const AwaClientSession *)session, (void *)changeSet, ResponseType_ChangeSet, quiet, &lastObjectID, &lastObjectInstanceID);
        }
    }
    AwaPathIterator_Free(&iterator);
    return cstring;
}

void ExecuteCallback(const AwaExecuteArguments * arguments, void * context)
{
    g_notifyCount++;
    Verbose("Execute %d:\n", g_notifyCount);

    if ((arguments != NULL) && (arguments->Data != NULL) && (arguments->Size > 0))
    {
        int i;
        printf("DATA: length %zu, payload: [", arguments->Size);
        for (i = 0; i < arguments->Size; i++)
        {
            printf("%02x ", ((uint8_t *)arguments->Data)[i]);
        } 
        printf("]\n");
    }
    else
    {
        printf("NO DATA\n");
    }
}

void ChangeCallback(const AwaChangeSet * changeSet, void * context)
{
    g_notifyCount++;
    Verbose("Notify %d:\n", g_notifyCount);

    const AwaClientSession * session = AwaChangeSet_GetClientSession(changeSet);
    bool quiet = *((bool*)context);

    char * output = ChangeSetToCString((void *)session, changeSet, quiet);
    if (output != NULL)
    {
        printf("%s", output);
    }
    free(output);
    output = NULL;
}

static bool SubscribeToTarget(AwaClientSession * session, AwaClientSubscribeOperation * operation, const Target * target, void * context)
{
    bool result = false;
    Verbose("Subscribe %s ", target->Path);
    AwaSubscribeType subscribeType = AwaSubscribeType_None;

    bool isResourceTarget = Client_IsResourceTarget(session, target);
    AwaResourceOperations operations = isResourceTarget? Client_GetResourceOperations(session, target->Path) : AwaResourceOperations_Invalid;

    if ((!isResourceTarget) || (operations != AwaResourceOperations_None && operations != AwaResourceOperations_Execute))
    {
        AwaClientChangeSubscription * changeSubscription = AwaClientChangeSubscription_New(target->Path, ChangeCallback, context);
        if (changeSubscription != NULL)
        {
            if (AwaClientSubscribeOperation_AddChangeSubscription(operation, changeSubscription) != AwaError_Success)
            {
                Error("Failed to add change subscription to operation: %s\n", target->Path);
            }
            else
            {
                subscribeType = AwaSubscribeType_Change;
                AddSubscription(subscribeType, changeSubscription);
                Verbose("Change");
                result = true;
            }
        }
        else
        {
            Error("Failed to create change subscription \n");
        }
    }
    else if (operations == AwaResourceOperations_Execute)
    {
        AwaClientExecuteSubscription * executeSubscription = AwaClientExecuteSubscription_New(target->Path, ExecuteCallback, context);
        if (executeSubscription != NULL)
        {
            if (AwaClientSubscribeOperation_AddExecuteSubscription(operation, executeSubscription) != AwaError_Success)
            {
                Error("Failed to add execute subscription to operation: %s\n", target->Path);
            }
            else
            {
                subscribeType = AwaSubscribeType_Execute;
                AddSubscription(subscribeType, executeSubscription);
                Verbose("Execute");
                result = true;
            }
        }
        else
        {
            Error("Failed to create execute subscription \n");
        }
    }

    Verbose("\n");
    return result;
}

static void UnsubscribeFromTargets(AwaClientSession * session)
{ 
    AwaClientSubscribeOperation * operation = AwaClientSubscribeOperation_New(session);
    if (operation == NULL)
    {
        Error("Failed to create subscribe operation\n");
        exit(1);
    }

    // Add cancels for each subscription
    SubscriptionNode * currentSubscription = g_subscriptionListHead;
    SubscriptionNode * nextSubscription;
    while(currentSubscription != NULL)
    {
        nextSubscription = currentSubscription->next;

        switch (currentSubscription->type)
        {
            case AwaSubscribeType_Change:
                if (AwaClientSubscribeOperation_AddCancelChangeSubscription(operation, currentSubscription->subscription) != AwaError_Success)
                {
                    Error("AwaClientSubscribeOperation_AddCancelChangeSubscription() failed for path %s\n", AwaClientChangeSubscription_GetPath(currentSubscription->subscription));
                }
                break;
            case AwaSubscribeType_Execute:
                if (AwaClientSubscribeOperation_AddCancelExecuteSubscription(operation, currentSubscription->subscription) != AwaError_Success)
                {
                    Error("AwaClientSubscribeOperation_AddCancelExecuteSubscription() failed for path %s\n", AwaClientExecuteSubscription_GetPath(currentSubscription->subscription));
                }
                break;
            default:
                Error("Unsupported subscribe type: %d\n", currentSubscription->type);
                break;
        }
        currentSubscription = nextSubscription;
    }

    if (AwaClientSubscribeOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT) != AwaError_Success)
    {
        Error("Failed to perform subscribe operation\n");
    }

    // Check response for each subscription
    const AwaClientSubscribeResponse * response = AwaClientSubscribeOperation_GetResponse(operation);
    currentSubscription = g_subscriptionListHead;
    while (currentSubscription != NULL)
    {
        nextSubscription = currentSubscription->next;

        const AwaPathResult * result = NULL;
        const char * path = NULL;

        switch (currentSubscription->type)
        {
            case AwaSubscribeType_Change:
            {
                AwaClientChangeSubscription * subscription = (AwaClientChangeSubscription *)(currentSubscription->subscription);
                path = AwaClientChangeSubscription_GetPath(subscription);
                break;
            }
            case AwaSubscribeType_Execute:
            {
                AwaClientExecuteSubscription * subscription = (AwaClientExecuteSubscription *)(currentSubscription->subscription);
                path = AwaClientExecuteSubscription_GetPath(subscription);
                break;
            }
            default:
                Error("Unsupported subscribe type: %d for path %s\n", currentSubscription->type, path);
                break;
        }

        result = AwaClientSubscribeResponse_GetPathResult(response, path);
        if (AwaPathResult_GetError(result) != AwaError_Success)
        {
            Error("Failed to cancel subscription to %s: %s\n", path, AwaError_ToString(AwaPathResult_GetError(result)));
        }

        currentSubscription = nextSubscription;
    }
    AwaClientSubscribeOperation_Free(&operation);

    // Finally free each subscription
    currentSubscription = g_subscriptionListHead;
    while(currentSubscription != NULL)
    {
        nextSubscription = currentSubscription->next;

        switch (currentSubscription->type)
        {
            case AwaSubscribeType_Change:
            {
                AwaClientChangeSubscription * subscription = (AwaClientChangeSubscription *)(currentSubscription->subscription);
                if (AwaClientChangeSubscription_Free(&subscription) != AwaError_Success)
                {
                    Error("AwaClientChangeSubscription_Free() failed for path %s\n", AwaClientChangeSubscription_GetPath(subscription));
                }
                break;
            }
            case AwaSubscribeType_Execute:
            {
                AwaClientExecuteSubscription * subscription = (AwaClientExecuteSubscription *)(currentSubscription->subscription);
                if (AwaClientExecuteSubscription_Free(&subscription) != AwaError_Success)
                {
                    Error("AwaClientExecuteSubscription_Free() failed\n for path %s\n", AwaClientExecuteSubscription_GetPath(subscription));
                }
                break;
            }
            default:
                Error("Unsupported subscribe type: %d\n", currentSubscription->type);
                break;
        }
        currentSubscription = nextSubscription;
    }
}

static void Wait(AwaClientSession * session, int waitTime, int waitCount)
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
    while((waitCount > 0 ? g_notifyCount < waitCount : true) &&
          (g_signal == false) &&
          (waitTime == 0 || ticks < waitTime))
    {
        if (AwaClientSession_Process(session, 1000 /* 1 second */) != AwaError_Success)
        {
            printf("AwaClientSession_Process() failed\n");
            break;
        }
        AwaClientSession_DispatchCallbacks(session);

        ticks++;
    }
}

int main(int argc, char ** argv)
{
    int result = 0;
    AwaClientSubscribeOperation * operation;
    struct gengetopt_args_info ai;
    AwaClientSession * session = NULL;

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

    session = Client_EstablishSession(ai.ipcAddress_arg, ai.ipcPort_arg);
    if (session != NULL)
    {
        operation = AwaClientSubscribeOperation_New(session);
        if (operation == NULL)
        {
            Error("Failed to create subscribe operation\n");
            exit(1);
        }

        void * context = &ai.quiet_given; // pass the quiet parameter as our context

        int count = 0;
        int i = 0;
        for (i = 0; i < ai.inputs_num; ++i)
        {
            Target * target = CreateTarget(ai.inputs[i]);
            if (target != NULL)
            {
                count = SubscribeToTarget(session, operation, target, context) ? count + 1 : count;
                FreeTarget(&target);
            }
        }

        if (AwaClientSubscribeOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT) != AwaError_Success)
        {
            Error("Failed to perform subscribe operation\n");
            goto cleanup;
        }

        int validCount = count;
        const AwaClientSubscribeResponse * response = AwaClientSubscribeOperation_GetResponse(operation);
        SubscriptionNode * currentSubscription = g_subscriptionListHead;
        SubscriptionNode * nextSubscription;
        while (currentSubscription != NULL)
        {
            nextSubscription = currentSubscription->next;

            const AwaPathResult * pathResult = NULL;
            const char * path = NULL;

            switch (currentSubscription->type)
            {
                case AwaSubscribeType_Change:
                {
                    AwaClientChangeSubscription * subscription = (AwaClientChangeSubscription *)(currentSubscription->subscription);
                    path = AwaClientChangeSubscription_GetPath(subscription);
                    break;
                }
                case AwaSubscribeType_Execute:
                {
                    AwaClientExecuteSubscription * subscription = (AwaClientExecuteSubscription *)(currentSubscription->subscription);
                    path = AwaClientExecuteSubscription_GetPath(subscription);
                    break;
                }
                default:
                    Error("Unsupported subscribe type: %d for path %s\n", currentSubscription->type, path);
                    break;
            }

            pathResult = AwaClientSubscribeResponse_GetPathResult(response, path);
            if (AwaPathResult_GetError(pathResult) != AwaError_Success)
            {
                Error("Failed to subscribe to %s: %s\n", path, AwaError_ToString(AwaPathResult_GetError(pathResult)));
                validCount--;
            }

            currentSubscription = nextSubscription;
        }

        AwaClientSubscribeOperation_Free(&operation);

        Debug("count %d\n", count);
        // Wait if there's something to wait for
        if (count > 0)
        {
            if (validCount > 0)
            {
                Wait(session, ai.waitTime_arg, ai.waitCount_arg);
            }
            UnsubscribeFromTargets(session);
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
        FreeSubscriptionList();
        Client_ReleaseSession(&session);
    }
    cmdline_parser_free(&ai);
    return result;
}

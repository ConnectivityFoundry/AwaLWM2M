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

/*
 * This example demonstrates how to respond to client events from the server.
 *
 * If a client registers, deregisters or updates its registration with the server,
 * an application callback is invoked.
 *
 * To run this example, run an Awa server and then run this example. Then
 * start a LWM2M client (such as the Awa client) and have it register with this
 * server. Hit CTRL-C to terminate.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include <awa/common.h>
#include <awa/server.h>

#define IPC_PORT (54321)
#define IPC_ADDRESS "127.0.0.2"
#define OPERATION_PERFORM_TIMEOUT 1000

/*
 * This function will be called by the API when a client registers
 * with the server and generates a ClientRegister event.
 * When invoked, event provides information about the client concerned.
 */
static void clientRegisterCallback(const AwaServerClientRegisterEvent * event, void * context)
{
    int * eventCount = (int*)context;
    (*eventCount)++;

    /* Display client ID and registered entities */
    AwaClientIterator * clientIterator = AwaServerClientRegisterEvent_NewClientIterator(event);

    while (AwaClientIterator_Next(clientIterator))
    {
        const char * clientID =  AwaClientIterator_GetClientID(clientIterator);
        printf("Event %d: Client %s registered:\n  ", *eventCount, clientID);
        AwaRegisteredEntityIterator * entityIterator = AwaServerClientRegisterEvent_NewRegisteredEntityIterator(event, clientID);
        while (AwaRegisteredEntityIterator_Next(entityIterator))
        {
            printf(" %s", AwaRegisteredEntityIterator_GetPath(entityIterator));
        }
        printf("\n");
        AwaRegisteredEntityIterator_Free(&entityIterator);
    }
    AwaClientIterator_Free(&clientIterator);
}

/*
 * This function will be called by the API when a client deregisters
 * with the server and generates a ClientDeregister event.
 * When invoked, event provides information about the client concerned.
 */
static void clientDeregisterCallback(const AwaServerClientDeregisterEvent * event, void * context)
{
    int * eventCount = (int*)context;
    (*eventCount)++;

    /* Display client ID */
    AwaClientIterator * clientIterator = AwaServerClientDeregisterEvent_NewClientIterator(event);

    while (AwaClientIterator_Next(clientIterator))
    {
        const char * clientID =  AwaClientIterator_GetClientID(clientIterator);
        printf("Event %d: Client %s deregistered.\n", *eventCount, clientID);
    }
    AwaClientIterator_Free(&clientIterator);
}

/*
 * This function will be called by the API when a client updates its
 * registration with the server and generates a ClientUpdate event.
 * When invoked, event provides information about the client concerned.
 */
static void clientUpdateCallback(const AwaServerClientUpdateEvent * event, void * context)
{
    int * eventCount = (int*)context;
    (*eventCount)++;

    /* Display client ID and registered entities */
    AwaClientIterator * clientIterator = AwaServerClientUpdateEvent_NewClientIterator(event);

    while (AwaClientIterator_Next(clientIterator))
    {
        const char * clientID =  AwaClientIterator_GetClientID(clientIterator);
        printf("Event %d: Client %s updated:\n  ", *eventCount, clientID);
        AwaRegisteredEntityIterator * entityIterator = AwaServerClientUpdateEvent_NewRegisteredEntityIterator(event, clientID);
        while (AwaRegisteredEntityIterator_Next(entityIterator))
        {
            printf(" %s", AwaRegisteredEntityIterator_GetPath(entityIterator));
        }
        printf("\n");
        AwaRegisteredEntityIterator_Free(&entityIterator);
    }
    AwaClientIterator_Free(&clientIterator);
}

static bool stopFlag = false;

/*
 * Trigger the program to exit on CTRL-C
 */
static void stop(int ignore)
{
    printf("Exiting...\n");
    stopFlag = true;
}

int main(void)
{
    signal(SIGINT, stop);

    /* Create and initialise server session */
    AwaServerSession * session = AwaServerSession_New();

    /* Use default IPC configuration */
    AwaServerSession_Connect(session);

    int eventCount = 0;

    /* Provide a callback function to the API for Client Register events */
    AwaServerSession_SetClientRegisterEventCallback(session, clientRegisterCallback, &eventCount);

    /* Provide a callback function to the API for Client Deregister events */
    AwaServerSession_SetClientDeregisterEventCallback(session, clientDeregisterCallback, &eventCount);

    /* Provide a callback function to the API for Client Update events */
    AwaServerSession_SetClientUpdateEventCallback(session, clientUpdateCallback, &eventCount);

    /* Wait for events */
    while (!stopFlag)
    {
        /* Receive events */
        AwaServerSession_Process(session, OPERATION_PERFORM_TIMEOUT);
        AwaServerSession_DispatchCallbacks(session);
    }

    AwaServerSession_Disconnect(session);
    AwaServerSession_Free(&session);
    return 0;
}

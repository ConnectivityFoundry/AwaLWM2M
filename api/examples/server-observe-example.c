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
 * This example demonstrates how to observe a client's resource via the server.
 * If the resource changes and a notification is generated, an application callback
 * is invoked.
 *
 * This example is a little contrived in that it creates an observation and then
 * triggers the notification itself with a WRITE. In many cases, the notification
 * will be triggered by an update to the resource value by another entity such as
 * the client itself.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <awa/common.h>
#include <awa/server.h>

#define IPC_PORT (54321)
#define IPC_ADDRESS "127.0.0.2"
#define OPERATION_PERFORM_TIMEOUT 1000
#define CLIENT_ID "TestClient1"

/*
 * This function will be called by the API when the server receives
 * a notification of change from the client. When invoked, changeSet
 * provides information about the value(s) that triggered the notification,
 * and context is the supplied pointer to any application data.
 *
 * This callback is called once when the observation is set up.
 */
static void observeCallback(const AwaChangeSet * changeSet, void * context)
{
    int * notificationCount = (int*)context;
    printf("Callback received, notification count = %d\n", *notificationCount);

    const char * value;
    AwaChangeSet_GetValueAsCStringPointer(changeSet, "/3/0/15", &value);
    if (*notificationCount == 0)
        printf("Initial value of resource /3/0/15 is: %s\n", value);
    else
        printf("Value of resource /3/0/15 changed to: %s\n", value);
    (*notificationCount)++;
}

int main(void)
{
    /* Create and initialise server session */
    AwaServerSession * session = AwaServerSession_New();

    /* Use default IPC configuration */
    AwaServerSession_Connect(session);

    int notificationCount = 0;

    /*
     * This example uses resource /3/0/15 which is the Timezone
     * resource in the standard Device object. It is a string.
     */

    /* Create a new observation to resource /3/0/15 */
    AwaServerObservation * observation = AwaServerObservation_New(CLIENT_ID, "/3/0/15", observeCallback, (void*)&notificationCount);

    /* Start listening to notifications */
    AwaServerObserveOperation * observeOperation = AwaServerObserveOperation_New(session);
    AwaServerObserveOperation_AddObservation(observeOperation, observation);
    AwaServerObserveOperation_Perform(observeOperation, OPERATION_PERFORM_TIMEOUT);
    AwaServerObserveOperation_Free(&observeOperation);

    /* Change the value of resource /3/0/15 so that a notification will be sent to our observation */
    AwaServerWriteOperation * writeOperation = AwaServerWriteOperation_New(session, AwaWriteMode_Update);
    AwaServerWriteOperation_AddValueAsCString(writeOperation, "/3/0/15", "Pacific/Auckland");
    AwaServerWriteOperation_Perform(writeOperation, CLIENT_ID, OPERATION_PERFORM_TIMEOUT);
    AwaServerWriteOperation_Free(&writeOperation);

    /* Sleep for a period to ensure we don't process before the notification is received */
    sleep(1);

    /* Receive notifications */
    AwaServerSession_Process(session, OPERATION_PERFORM_TIMEOUT);
    AwaServerSession_DispatchCallbacks(session);

    /* Cancel observation from resource /3/0/15 */
    AwaServerObserveOperation * cancelObserveOperation = AwaServerObserveOperation_New(session);
    AwaServerObserveOperation_AddCancelObservation(cancelObserveOperation, observation);
    AwaServerObserveOperation_Perform(cancelObserveOperation, OPERATION_PERFORM_TIMEOUT);
    AwaServerObserveOperation_Free(&cancelObserveOperation);

    /* Operations must be freed after use */
    AwaServerObservation_Free(&observation);

    AwaServerSession_Disconnect(session);
    AwaServerSession_Free(&session);
    return 0;
}

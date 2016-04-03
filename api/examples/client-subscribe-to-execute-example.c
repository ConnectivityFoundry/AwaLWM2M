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
 * This example demonstrates how to subscribe to the client for notifications of
 * execution of a resource, using a SUBSCRIBE to EXECUTE operation.
 *
 * This example is a little contrived in that it creates a subscription and then
 * triggers the notification itself. In most cases, the notification will be
 * triggered by an EXECUTE operation initiated by the server.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <awa/common.h>
#include <awa/client.h>
#include <awa/server.h>

#define CLIENT_IPC_PORT (12345)
#define CLIENT_IPC_ADDRESS "127.0.0.1"
#define SERVER_IPC_PORT (54321)
#define SERVER_IPC_ADDRESS "127.0.0.2"
#define CLIENT_ID "TestClient1"
#define OPERATION_PERFORM_TIMEOUT 1000

/*
 * This function will be called by the API when the client sends
 * a notification of execution. When invoked, arguments provides information
 * about the execute arguments supplied by the server, and context
 * is the supplied pointer to any application data.
 */
static void executeCallback(const AwaExecuteArguments * arguments, void * context)
{
    const char * userData = (const char *)context;
    printf("Callback received. Context = %s\n", userData);

    printf("Resource executed [%zu bytes payload]\n", arguments->Size);
}

int main(void)
{
    /* Create and initialise client session */
    AwaClientSession * clientSession = AwaClientSession_New();

    /* Use default IPC configuration */
    AwaClientSession_Connect(clientSession);

    /* Create and initialise server session (for EXECUTE) */
    AwaServerSession * serverSession = AwaServerSession_New();
    AwaServerSession_SetIPCAsUDP(serverSession, SERVER_IPC_ADDRESS, SERVER_IPC_PORT);
    AwaServerSession_Connect(serverSession);

    /* Application-specific data */
    const char * userData = "My custom data";

    /*
     * This example uses resource /3/0/4 which is the Reboot
     * resource in the standard Device object. It is an executable (None type) resource.
     */

    /*
     * Create a new execute subscription to resource /3/0/4.
     * Data can be provided to the callback function via the context pointer.
     */
    AwaClientExecuteSubscription * subscription = AwaClientExecuteSubscription_New("/3/0/4", executeCallback, (void*)userData);

    /* Start listening to notifications */
    AwaClientSubscribeOperation * subscribeOperation = AwaClientSubscribeOperation_New(clientSession);
    AwaClientSubscribeOperation_AddExecuteSubscription(subscribeOperation, subscription);
    AwaClientSubscribeOperation_Perform(subscribeOperation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSubscribeOperation_Free(&subscribeOperation);

    /* Use server API to execute resource /3/0/4 */
    AwaServerExecuteOperation * executeOperation = AwaServerExecuteOperation_New(serverSession);
    AwaServerExecuteOperation_AddPath(executeOperation, CLIENT_ID, "/3/0/4", NULL);
    AwaServerExecuteOperation_Perform(executeOperation, OPERATION_PERFORM_TIMEOUT);
    AwaServerExecuteOperation_Free(&executeOperation);

    /* Sleep for a period to ensure we don't process before the notification is received */
    sleep(1);

    /* Receive notifications */
    AwaClientSession_Process(clientSession, OPERATION_PERFORM_TIMEOUT);
    AwaClientSession_DispatchCallbacks(clientSession);

    /* Unsubscribe from resource /3/0/4 */
    AwaClientSubscribeOperation * cancelSubscribeOperation = AwaClientSubscribeOperation_New(clientSession);
    AwaClientSubscribeOperation_AddCancelExecuteSubscription(cancelSubscribeOperation, subscription);
    AwaClientSubscribeOperation_Perform(cancelSubscribeOperation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSubscribeOperation_Free(&cancelSubscribeOperation);

    /* Free the execute subscription */
    AwaClientExecuteSubscription_Free(&subscription);

    /* Free both sessions */
    AwaClientSession_Disconnect(clientSession);
    AwaClientSession_Free(&clientSession);
    AwaServerSession_Disconnect(serverSession);
    AwaServerSession_Free(&serverSession);
    return 0;
}

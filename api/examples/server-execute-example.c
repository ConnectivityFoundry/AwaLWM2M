/*
 * This example demonstrates how to initiate an EXECUTE operation via the server,
 * targeting an executable resource on the client.
 */

#include <stdlib.h>
#include <stdio.h>

#include <awa/common.h>
#include <awa/server.h>

#define IPC_PORT (54321)
#define IPC_ADDRESS "127.0.0.2"
#define OPERATION_PERFORM_TIMEOUT 1000
#define CLIENT_ID "TestClient1"

int main(void)
{
    /* Create and initialise server session */
    AwaServerSession * session = AwaServerSession_New();

    /* Use default IPC configuration */
    AwaServerSession_Connect(session);

    /* Create EXECUTE operation */
    AwaServerExecuteOperation * operation = AwaServerExecuteOperation_New(session);

    /*
     * This example uses resource /3/0/4 which is the Reboot
     * resource in the standard Device object. It is an executable (None type) resource.
     */

    /* Add resource path to the EXECUTE operation */
    AwaServerExecuteOperation_AddPath(operation, CLIENT_ID, "/3/0/4", NULL);
    AwaServerExecuteOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    /* Operations must be freed after use */
    AwaServerExecuteOperation_Free(&operation);

    AwaServerSession_Disconnect(session);
    AwaServerSession_Free(&session);
    return 0;
}

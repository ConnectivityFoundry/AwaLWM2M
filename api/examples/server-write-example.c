/*
 * This example demonstrates how to set the value of a resource on a registered
 * client with a WRITE operation.
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

    /* Create WRITE operation */
    AwaServerWriteOperation * operation = AwaServerWriteOperation_New(session, AwaWriteMode_Update);

    /*
     * This example uses resource /3/0/15 which is the Timezone
     * resource in the standard Device object. It is a string.
     */

    /* Add the resource path and value to the WRITE operation */
    AwaServerWriteOperation_AddValueAsCString(operation, "/3/0/15", "Pacific/Auckland");
    AwaServerWriteOperation_Perform(operation, CLIENT_ID, OPERATION_PERFORM_TIMEOUT);

    /* Operations must be freed after use */
    AwaServerWriteOperation_Free(&operation);

    AwaServerSession_Disconnect(session);
    AwaServerSession_Free(&session);
    return 0;
}

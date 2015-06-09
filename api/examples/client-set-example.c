/*
 * This example demonstrates how to set the value of a resource on the client
 * using the SET operation.
 */

#include <stdlib.h>
#include <stdio.h>

#include <awa/common.h>
#include <awa/client.h>

#define IPC_PORT (12345)
#define IPC_ADDRESS "127.0.0.1"
#define OPERATION_PERFORM_TIMEOUT 1000

int main(void)
{
    /* Create and initialise client session */
    AwaClientSession * session = AwaClientSession_New();

    /* Use default IPC configuration */
    AwaClientSession_Connect(session);

    /* Create SET operation */
    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);

    /*
     * This example uses resource /3/0/0 which is the Manufacturer
     * resource in the standard Device object. It is a string.
     */

    /* Provide a path and value for the resource */
    AwaClientSetOperation_AddValueAsCString(operation, "/3/0/0", "HotAir Systems Inc");

    /* Perform the SET operation */
    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    /* Operations must be freed after use */
    AwaClientSetOperation_Free(&operation);

    AwaClientSession_Disconnect(session);
    AwaClientSession_Free(&session);
    return 0;
}

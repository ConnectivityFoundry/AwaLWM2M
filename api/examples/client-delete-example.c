/*
 * This example demonstrates how to delete a resource from the client using a DELETE operation.
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

    /* Create DELETE operation */
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session);

    /*
     * This example uses resource /3/0/0 which is the Manufacturer
     * resource in the standard Device object. It is a string.
     */

    /* Add the resource path of the resource to delete to the DELETE operation */
    AwaClientDeleteOperation_AddPath(operation, "/3/0/0");
    AwaClientDeleteOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    /* Operations must be freed after use */
    AwaClientDeleteOperation_Free(&operation);

    AwaClientSession_Disconnect(session);
    AwaClientSession_Free(&session);
    return 0;
}

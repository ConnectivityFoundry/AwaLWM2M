/*
 * This example demonstrates how to create and set the value of an optional resource
 * on the client using the SET operation.
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
     * This example uses a custom object (ID 1000) and one of its resources (ID 101) which must
     * be created prior to using a set operation upon it. Refer to @example client-defineset-example.c
     */

    /* Before modifying resources on an object instance, it must be created */
    AwaClientSetOperation_CreateObjectInstance(operation, "/1000/0");

    /* Before modifying an optional resource, it must be created */
    AwaClientSetOperation_CreateOptionalResource(operation, "/1000/0/101");

    /* Provide a path and value for the new resource in the same operation */
    AwaClientSetOperation_AddValueAsCString(operation, "/1000/0/101", "HotAir Systems Inc");

    /* Perform the SET operation */
    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    /* Operations must be freed after use */
    AwaClientSetOperation_Free(&operation);

    AwaClientSession_Disconnect(session);
    AwaClientSession_Free(&session);
    return 0;
}

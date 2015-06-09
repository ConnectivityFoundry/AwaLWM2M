/*
 * This example demonstrates how to set the value of a resource instance of a
 * multi-instance resource on the client using the SET operation.
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
     * This example uses a custom object (ID 1000) and one of its resources (ID 105) which must
     * be created prior to using a set operation upon it. Refer to @example client-defineset-example.c
     * before using the resource we must create it and its object instance.
     */

    /* Create object instance 0 of object type 1000 */
    AwaClientSetOperation_CreateObjectInstance(operation, "/1000/0");

    /* Create an instance of optional resource 105 within this instance */
    AwaClientSetOperation_CreateOptionalResource(operation, "/1000/0/105");

    /* Set elements 0, 1, and 2 */
    AwaClientSetOperation_AddArrayValueAsInteger(operation, "/1000/0/105", 0, 30);
    AwaClientSetOperation_AddArrayValueAsInteger(operation, "/1000/0/105", 1, 35);
    AwaClientSetOperation_AddArrayValueAsInteger(operation, "/1000/0/105", 2, 40);

    /* Perform the SET operation */
    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    /* Operations must be freed after use */
    AwaClientSetOperation_Free(&operation);

    AwaClientSession_Disconnect(session);
    AwaClientSession_Free(&session);
    return 0;
}

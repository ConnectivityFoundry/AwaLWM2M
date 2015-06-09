/*
 * This example demonstrates how to delete an object instance from
 * the server using the DELETE operation.
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

    /* Create DELETE operation */
    AwaServerDeleteOperation * operation = AwaServerDeleteOperation_New(session);

    /*
     * This example uses /4/0 which is an instance of the LWM2M Connectivity Monitoring object
     */

    /* Add resource path to DELETE operation */
    AwaServerDeleteOperation_AddPath(operation, CLIENT_ID, "/4/0");
    AwaServerDeleteOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    /* Operations must be freed after use */
    AwaServerDeleteOperation_Free(&operation);

    AwaServerSession_Disconnect(session);
    AwaServerSession_Free(&session);
    return 0;
}

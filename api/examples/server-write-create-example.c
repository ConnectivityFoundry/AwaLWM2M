/*
 * This example demonstrates how to create a new object instance on a
 * registered client with a WRITE operation. In this case, the object
 * instance ID is specified by the application.
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
     * This example uses object /2 which is the Lwm2m ACL object.
     */

    /* Create a new object instance by specifying the object instance path */
    AwaServerWriteOperation_CreateObjectInstance(operation, "/2/10");
    AwaServerWriteOperation_Perform(operation, CLIENT_ID, OPERATION_PERFORM_TIMEOUT);

    /* Operations must be freed after use */
    AwaServerWriteOperation_Free(&operation);

    AwaServerSession_Disconnect(session);
    AwaServerSession_Free(&session);
    return 0;
}

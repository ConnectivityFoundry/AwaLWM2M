/*
 * This example demonstrates how to create a new object instance on a
 * registered client with a WRITE operation. In this case, the object
 * instance ID is not specified by the application, and chosen by the
 * client.
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
     *This example uses object /2 which is the Lwm2m ACL object.
     */

    /* Add the object path to the WRITE operation, without the object instance path */
    AwaServerWriteOperation_CreateObjectInstance(operation, "/2");
    AwaServerWriteOperation_Perform(operation, CLIENT_ID, OPERATION_PERFORM_TIMEOUT);

    /* Iterate through the response to find the newly created object instance */
    const AwaServerWriteResponse * response = AwaServerWriteOperation_GetResponse(operation, CLIENT_ID);
    AwaPathIterator * iterator = AwaServerWriteResponse_NewPathIterator(response);
    while (AwaPathIterator_Next(iterator))
    {
        const char * path = AwaPathIterator_Get(iterator);
        printf("Created new object instance: %s\n", path);
    }

    /* Operations must be freed after use */
    AwaServerWriteOperation_Free(&operation);

    AwaServerSession_Disconnect(session);
    AwaServerSession_Free(&session);
    return 0;
}

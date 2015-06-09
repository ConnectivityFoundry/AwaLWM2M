/*
 * This example demonstrates how to read a resource value from a registered
 * client with a READ operation.
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

    /* Create READ operation */
    AwaServerReadOperation * operation = AwaServerReadOperation_New(session);

    /*
     * This example uses resource /3/0/15 which is the Timezone
     * resource in the standard Device object. It is a string.
     * It assumes a client with endpoint name CLIENT_ID is running and connected to the server.
     */

    /* Add resource path to retrieve to the READ operation */
    AwaServerReadOperation_AddPath(operation, CLIENT_ID, "/3/0/15");
    AwaServerReadOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    /* A successful operation results in a READ response */
    const AwaServerReadResponse * response = AwaServerReadOperation_GetResponse(operation, CLIENT_ID);

    /* Extract the resource value from the response using the path */
    const char * value = NULL;
    AwaServerReadResponse_GetValueAsCStringPointer(response, "/3/0/15", &value);
    printf("%s\n", value);

    /* Operations must be freed after use */
    AwaServerReadOperation_Free(&operation);

    AwaServerSession_Disconnect(session);
    AwaServerSession_Free(&session);
    return 0;
}

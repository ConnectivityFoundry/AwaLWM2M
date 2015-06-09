/*
 * This example demonstrates how to retrieve the value of a resource from the client
 * using a GET operation.
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

    /* Create GET operation */
    AwaClientGetOperation * operation = AwaClientGetOperation_New(session);

    /*
     * This example uses resource /3/0/0 which is the Manufacturer
     * resource in the standard Device object. It is a string.
     */
    
    /* Add the resource path to retrieve to the GET operation */
    AwaClientGetOperation_AddPath(operation, "/3/0/0");
    AwaClientGetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    /* A successful operation results in a GET response */
    const AwaClientGetResponse * response = AwaClientGetOperation_GetResponse(operation);

    /* Extract the resource value from the response using the path */
    const char * value;
    AwaClientGetResponse_GetValueAsCStringPointer(response, "/3/0/0", &value);
    printf("%s\n", value);

    /* Operations must be freed after use */
    AwaClientGetOperation_Free(&operation);

    AwaClientSession_Disconnect(session);
    AwaClientSession_Free(&session);
    return 0;
}

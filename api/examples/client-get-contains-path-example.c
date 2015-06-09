/*
 * This example demonstrates how to test whether a GET operation response contains
 * a specific path. It also demonstrates retrieval of a value from a string resource.
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

    /* Add the path of the resource to retrieve to the GET operation */
    AwaClientGetOperation_AddPath(operation, "/3/0/0");
    AwaClientGetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    /* A successful operation results in a GET response */
    const AwaClientGetResponse * response = AwaClientGetOperation_GetResponse(operation);

    /* Test whether the response contains a result for the specified path */
    if (AwaClientGetResponse_ContainsPath(response, "/3/0/0"))
    {
        /* Test whether the response has a valid value for the specified path */
        if (AwaClientGetResponse_HasValue(response, "/3/0/0"))
        {
            /* Retrieve the value, as a C-style string */
            const char * value;
            AwaClientGetResponse_GetValueAsCStringPointer(response, "/3/0/0", &value);
            printf("%s\n", value);
        }
    }

    /* Operations must be freed after use */
    AwaClientGetOperation_Free(&operation);

    AwaClientSession_Disconnect(session);
    AwaClientSession_Free(&session);
    return 0;
}

/*
 * This example demonstrates how to retrieve the value of a resource instance
 * from a multi-instance resource (array) on the client using a GET operation.
 */

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

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
     * This example uses a custom object (ID 1000) and one of its resources (ID 105) which must
     * be created prior to using a set operation upon it. Refer to @example client-defineset-example.c
     * before using the resource we must create it and its object instance.
     */

    /* Add the resource path to retrieve to the GET operation */
    AwaClientGetOperation_AddPath(operation, "/1000/0/105");
    AwaClientGetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    /* A successful operation results in a GET response */
    const AwaClientGetResponse * response = AwaClientGetOperation_GetResponse(operation);

    /* Extract values from the response */
    const AwaIntegerArray * valueArray;
    AwaClientGetResponse_GetValuesAsIntegerArrayPointer(response, "/1000/0/105", &valueArray);

    AwaIntegerArrayIterator * iterator = AwaIntegerArray_NewIntegerArrayIterator(valueArray);
    while (AwaIntegerArrayIterator_Next(iterator))
    {
        printf("%zu : %" PRId64 "\n", AwaIntegerArrayIterator_GetIndex(iterator), AwaIntegerArrayIterator_GetValue(iterator));
    }

    /* Operations must be freed after use */
    AwaClientGetOperation_Free(&operation);

    AwaClientSession_Disconnect(session);
    AwaClientSession_Free(&session);
    return 0;
}

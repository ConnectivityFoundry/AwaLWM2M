/*
 * This example demonstrates how to delete a range of resource instances from
 * a multi-instance resource (array) on the client using a DELETE operation.
 */

#include <stdlib.h>
#include <stdio.h>

#include <awa/common.h>
#include <awa/client.h>

#define IPC_PORT (12345)
#define IPC_ADDRESS "127.0.0.1"
#define OPERATION_PERFORM_TIMEOUT 1000

// NB: AwaClientDeleteOperation_AddPathWithArrayRange is currently unsupported.

int main(void)
{
    /* Create and initialise client session */
    AwaClientSession * session = AwaClientSession_New();

    /* Use default IPC configuration */
    AwaClientSession_Connect(session);

    /*
     * This example uses a custom object (ID 1000) and one of its resources (ID 105) which must
     * be created prior to using a set operation upon it. Refer to @example defineset-example.c.
     * This example also assumes an object instance of object 1000 has been created, and resource
     * 105 has been populated. Refer to @example set-array-example.c
     */

    /* Create DELETE operation */
    AwaClientDeleteOperation * operation = AwaClientDeleteOperation_New(session);

    /* Delete the first two array elements by adding the path with range */
    AwaClientDeleteOperation_AddPathWithArrayRange(operation, "/1000/0/105", 0, 2);
    AwaClientDeleteOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    /* NB: the array range upper limit is exclusive - the instance at that index is not deleted!

    /* Operations must be freed after use */
    AwaClientDeleteOperation_Free(&operation);

    AwaClientSession_Disconnect(session);
    AwaClientSession_Free(&session);
    return 0;
}

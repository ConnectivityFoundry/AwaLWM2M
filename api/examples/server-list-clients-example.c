/*
 * This example demonstrates how to obtain a list of all registered clients from
 * the server using a LIST_CLIENTS operation. In addition, the list of registered
 * objects and object instances is extracted from the response.
 */

#include <stdlib.h>
#include <stdio.h>

#include <awa/common.h>
#include <awa/server.h>

#define IPC_PORT (54321)
#define IPC_ADDRESS "127.0.0.2"
#define OPERATION_PERFORM_TIMEOUT 1000

int main(void)
{
    /* Create and initialise server session */
    AwaServerSession * session = AwaServerSession_New();

    /* Use default IPC configuration */
    AwaServerSession_Connect(session);

    /* Create LIST_CLIENTS operation */
    AwaServerListClientsOperation * operation = AwaServerListClientsOperation_New(session);

    /* Perform the operation, resulting in a response for each registered client */
    AwaServerListClientsOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    /*
     * Iterate through each client in the response, extracting the client's
     * endpoint name and registered objects
     */
    AwaClientIterator * clientIterator = AwaServerListClientsOperation_NewClientIterator(operation);
    while (AwaClientIterator_Next(clientIterator))
    {
        const char * clientID = AwaClientIterator_GetClientID(clientIterator);
        printf("%s :", clientID);

        const AwaServerListClientsResponse * response = AwaServerListClientsOperation_GetResponse(operation, clientID);
        AwaRegisteredEntityIterator * entityIterator = AwaServerListClientsResponse_NewRegisteredEntityIterator(response);
        while (AwaRegisteredEntityIterator_Next(entityIterator))
        {
            printf(" %s", AwaRegisteredEntityIterator_GetPath(entityIterator));
        }
        printf("\n");

        /* Iterators must be freed after use */
        AwaRegisteredEntityIterator_Free(&entityIterator);
    }

    /* Iterators must be freed after use */
    AwaClientIterator_Free(&clientIterator);

    /* Operations must be freed after use */
    AwaServerListClientsOperation_Free(&operation);

    AwaServerSession_Disconnect(session);
    AwaServerSession_Free(&session);
    return 0;
}


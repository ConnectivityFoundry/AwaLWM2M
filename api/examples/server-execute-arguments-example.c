/*
 * This example demonstrates how to initiate an EXECUTE operation via the server,
 * passing application-specific data to the executable resource on the client.
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

    /* Create EXECUTE operation */
    AwaServerExecuteOperation * operation = AwaServerExecuteOperation_New(session);

    /*
     * This example uses resource /3/0/4 which is the Reboot
     * resource in the standard Device object. It is an executable (None type) resource.
     * Warning: The composition of these arguments will change in the future to conform
     * with the latest LWM2M Specification.
     */

    /* Encapsulate EXECUTE arguments within AwaExecuteArguments */
    AwaExecuteArguments arguments;
    char data[] = "hello";
    arguments.Data = data;
    arguments.Size = sizeof(data);

    /* Add resource path and arguments to the EXECUTE operation */
    AwaServerExecuteOperation_AddPath(operation, CLIENT_ID, "/3/0/4", &arguments);
    AwaServerExecuteOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    /* Operations must be freed after use */
    AwaServerExecuteOperation_Free(&operation);

    AwaServerSession_Disconnect(session);
    AwaServerSession_Free(&session);
    return 0;
}

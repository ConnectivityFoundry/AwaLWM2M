/*
 * This example demonstrates how to subscribe to the client for notifications of
 * changes to the value of a resource, using a SUBSCRIBE to CHANGE operation.
 *
 * This example is a little contrived in that it creates a subscription and then
 * triggers the notification itself. In most cases, the notification will be
 * triggered by a WRITE operation initiated by the server.
 */

#include <stdlib.h>
#include <stdio.h>

#include <awa/common.h>
#include <awa/client.h>

#define IPC_PORT (12345)
#define IPC_ADDRESS "127.0.0.1"
#define OPERATION_PERFORM_TIMEOUT 1000

/*
 * This function will be called by the API when the client sends
 * a notification of change. When invoked, changeSet provides information
 * about the value(s) that triggered the notification, and context
 * is the supplied pointer to any application data.
 */
static void changeCallback(const AwaChangeSet * changeSet, void * context)
{
    const char * userData = (const char *)context;
    printf("Callback received. Context = %s\n", userData);

    const char * value;
    AwaChangeSet_GetValueAsCStringPointer(changeSet, "/3/0/0", &value);
    printf("Value of resource /3/0/0 changed to: %s\n", value);
}

int main(void)
{
    /* Create and initialise client session */
    AwaClientSession * session = AwaClientSession_New();

    /* Use default IPC configuration */
    AwaClientSession_Connect(session);

    /* Application-specific data */
    const char * userData = "My custom data";

    /*
     * This example uses resource /3/0/0 which is the Manufacturer
     * resource in the standard Device object. It is a string.
     */

    /*
     * Create a new change subscription to resource /3/0/0.
     * Data can be provided to the callback function via the context pointer.
     */
    AwaClientChangeSubscription * subscription = AwaClientChangeSubscription_New("/3/0/0", changeCallback, (void*)userData);

    /* Start listening to notifications */
    AwaClientSubscribeOperation * subscribeOperation = AwaClientSubscribeOperation_New(session);
    AwaClientSubscribeOperation_AddChangeSubscription(subscribeOperation, subscription);
    AwaClientSubscribeOperation_Perform(subscribeOperation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSubscribeOperation_Free(&subscribeOperation);

    /* Change the value of resource /3/0/0 so that a notification will be sent to our subscription */
    AwaClientSetOperation * setOperation = AwaClientSetOperation_New(session);
    AwaClientSetOperation_AddValueAsCString(setOperation, "/3/0/0", "HotAir Systems Inc");
    AwaClientSetOperation_Perform(setOperation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSetOperation_Free(&setOperation);

    /* Receive notifications */
    AwaClientSession_Process(session, OPERATION_PERFORM_TIMEOUT);
    AwaClientSession_DispatchCallbacks(session);

    /* Unsubscribe from resource /3/0/0 */
    AwaClientSubscribeOperation * cancelSubscribeOperation = AwaClientSubscribeOperation_New(session);
    AwaClientSubscribeOperation_AddCancelChangeSubscription(cancelSubscribeOperation, subscription);
    AwaClientSubscribeOperation_Perform(cancelSubscribeOperation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSubscribeOperation_Free(&cancelSubscribeOperation);

    /* Free the change subscription */
    AwaClientChangeSubscription_Free(&subscription);

    AwaClientSession_Disconnect(session);
    AwaClientSession_Free(&session);
    return 0;
}

/*
 * This example demonstrates how to define a new object definition, consisting
 * of a number of resources, on the client using a DEFINE operation.
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

    /* Create object definition */
    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(1000, "Heater", 0, 1);

    /* Create resource definitions */
    AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, 101, "Manufacturer", false, AwaResourceOperations_ReadWrite, NULL);
    AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, 102, "ModelNumber" , false, AwaResourceOperations_ReadWrite, NULL);
    AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, 103, "SerialNumber", false, AwaResourceOperations_ReadWrite, NULL);
    AwaObjectDefinition_AddResourceDefinitionAsFloat(objectDefinition,  104, "Temperature",  false, AwaResourceOperations_ReadWrite, 0.0);

    /* Create a default array value */
    AwaIntegerArray * defaultPowerSourceVoltages = AwaIntegerArray_New();
    AwaObjectDefinition_AddResourceDefinitionAsIntegerArray(objectDefinition, 105, "PowerSourceVoltage", 0, AWA_MAX_ID, AwaResourceOperations_ReadWrite, defaultPowerSourceVoltages);
    AwaIntegerArray_Free(&defaultPowerSourceVoltages);

    /* Create DEFINE operation */
    AwaClientDefineOperation * operation = AwaClientDefineOperation_New(session);

    /* Add object definition to DEFINE operation */
    AwaClientDefineOperation_Add(operation, objectDefinition);

    /* Perform the DEFINE operation */
    AwaClientDefineOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    /* Operations must be freed after use */
    AwaClientDefineOperation_Free(&operation);

    AwaClientSession_Disconnect(session);
    AwaClientSession_Free(&session);
    return 0;
}

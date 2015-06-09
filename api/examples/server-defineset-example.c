/*
 * This example demonstrates how to define a new object definition, consisting
 * of a number of resources, on the server using a DEFINE operation.
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

    /* Create object definition */
    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(1000, "Heater", 0, 1);

    /* Create resource definitions */
    AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, 101, "Manufacturer", false, AwaResourceOperations_ReadWrite, NULL);
    AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, 102, "ModelNumber" , false, AwaResourceOperations_ReadWrite, NULL);
    AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, 103, "SerialNumber", false, AwaResourceOperations_ReadWrite, NULL);
    AwaObjectDefinition_AddResourceDefinitionAsFloat(objectDefinition,  104, "Temperature",  false, AwaResourceOperations_ReadWrite, 0.0);

    /* Create a default array value */
    AwaIntegerArray * defaultPowerSourceVoltages = AwaIntegerArray_New();
    AwaObjectDefinition_AddResourceDefinitionAsIntegerArray(objectDefinition,  105, "PowerSourceVoltage", 0, AWA_MAX_ID, AwaResourceOperations_ReadWrite, defaultPowerSourceVoltages);
    AwaIntegerArray_Free(&defaultPowerSourceVoltages);

    /* Create DEFINE operation */
    AwaServerDefineOperation * operation = AwaServerDefineOperation_New(session);

    /* Add object definition to DEFINE operation */
    AwaServerDefineOperation_Add(operation, objectDefinition);

    /* Perform the DEFINE operation */
    AwaServerDefineOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    /* Operations must be freed after use */
    AwaServerDefineOperation_Free(&operation);

    AwaServerSession_Disconnect(session);
    AwaServerSession_Free(&session);
    return 0;
}

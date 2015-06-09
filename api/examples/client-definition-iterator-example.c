/*
 * This example demonstrates how the AwaObjectDefinitionIterator and
 * AwaResourceDefinitionIterator can be used to iterate through all of
 * the objects and resources that have been previously defined in a client session.
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

    /* An object iterator can be used to iterate through all defined objects in the session */
    AwaObjectDefinitionIterator * objectIterator = AwaClientSession_NewObjectDefinitionIterator(session);

    while (AwaObjectDefinitionIterator_Next(objectIterator))
    {
        const AwaObjectDefinition * objectDefinition = AwaObjectDefinitionIterator_Get(objectIterator);

        printf("Object ID:%d name:%s minInstances:%d maxInstances:%d\n",
            AwaObjectDefinition_GetID(objectDefinition),
            AwaObjectDefinition_GetName(objectDefinition),
            AwaObjectDefinition_GetMinimumInstances(objectDefinition),
            AwaObjectDefinition_GetMaximumInstances(objectDefinition));

        /* A resource iterator can be used to iterate through all defined resources in the object */
        AwaResourceDefinitionIterator * resourceIterator = AwaObjectDefinition_NewResourceDefinitionIterator(objectDefinition);

        while (AwaResourceDefinitionIterator_Next(resourceIterator))
        {
            const AwaResourceDefinition * resourceDefinition = AwaResourceDefinitionIterator_Get(resourceIterator);
            printf("       Resource: ID:%d name:%s type:%d minInstances:%d maxInstances:%d operations:%d\n",
                AwaResourceDefinition_GetID(resourceDefinition),
                AwaResourceDefinition_GetName(resourceDefinition),
                AwaResourceDefinition_GetType(resourceDefinition),
                AwaResourceDefinition_GetMinimumInstances(resourceDefinition),
                AwaResourceDefinition_GetMaximumInstances(resourceDefinition),
                AwaResourceDefinition_GetSupportedOperations(resourceDefinition));
        }

        /* Iterators must be freed after use */
        AwaResourceDefinitionIterator_Free(&resourceIterator);
    }
    
    /* Iterators must be freed after use */
    AwaObjectDefinitionIterator_Free(&objectIterator);

    AwaClientSession_Disconnect(session);
    AwaClientSession_Free(&session);
    return 0;
}

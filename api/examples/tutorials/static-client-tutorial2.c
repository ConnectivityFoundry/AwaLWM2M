#include <string.h>
#include <stdio.h>
#include "awa/static.h"

#define HEATER_INSTANCES 1

typedef struct
{
    char Manufacturer[64];
    AwaFloat Temperature;

} HeaterObject;

static HeaterObject heater[HEATER_INSTANCES];

static void DefineHeaterObject(AwaStaticClient * awaClient)
{
    AwaStaticClient_DefineObject(awaClient, "Heater", 1000, 0, HEATER_INSTANCES);
    AwaStaticClient_DefineResourceWithPointer(awaClient, "Manufacturer", 1000, 101, AwaResourceType_String, 0, 1, AwaResourceOperations_ReadOnly,
                                                &heater[0].Manufacturer, sizeof(heater[0].Manufacturer), sizeof(heater[0]));
    AwaStaticClient_DefineResourceWithPointer(awaClient, "Temperature",  1000, 104, AwaResourceType_Float, 0, 1, AwaResourceOperations_ReadOnly,
                                                &heater[0].Temperature, sizeof(heater[0].Temperature), sizeof(heater[0]));
}

static void SetInitialValues(AwaStaticClient * awaClient)
{
    int instance = 0;

    AwaStaticClient_CreateObjectInstance(awaClient, 1000, instance);

    AwaStaticClient_CreateResource(awaClient, 1000, instance, 101);
    strcpy(heater[instance].Manufacturer, "HotAir Systems Inc");

    AwaStaticClient_CreateResource(awaClient, 1000, instance, 104);
    heater[instance].Temperature = 0.0;
}

int main(void)
{
    AwaStaticClient * awaClient = AwaStaticClient_New();

    AwaStaticClient_SetLogLevel(AwaLogLevel_Error);
    AwaStaticClient_SetEndPointName(awaClient, "AwaStaticClient1");
    AwaStaticClient_SetCoAPListenAddressPort(awaClient, "0.0.0.0", 6000);
    AwaStaticClient_SetBootstrapServerURI(awaClient, "coap://[127.0.0.1]:15685");

    AwaStaticClient_Init(awaClient);

    DefineHeaterObject(awaClient);
    SetInitialValues(awaClient);

    while (1)
    {
        AwaStaticClient_Process(awaClient);

        //heater[0].Temperature = value from hardware
        AwaStaticClient_ResourceChanged(awaClient, 1000, 0, 104);
    }

    AwaStaticClient_Free(&awaClient);

    return 0;
}


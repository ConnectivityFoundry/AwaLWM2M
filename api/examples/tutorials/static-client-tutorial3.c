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

AwaLwm2mResult handler(AwaStaticClient * client, AwaOperation operation, AwaObjectID objectID,
                       AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID,
                       void ** dataPointer, uint16_t * dataSize, bool * changed)
{
    AwaLwm2mResult result = AwaLwm2mResult_InternalError;

    if ((objectID == 1000) && (objectInstanceID >= 0) && (objectInstanceID < HEATER_INSTANCES))
    {
        switch (operation)
        {
            case AwaOperation_CreateObjectInstance:
            {
                result = AwaLwm2mResult_SuccessCreated;
                memset(&heater[objectInstanceID], 0, sizeof(heater[objectInstanceID]));
                break;
            }
            case AwaOperation_CreateResource:
            {
                switch (resourceID)
                {
                case 101:
                    strcpy(heater[objectInstanceID].Manufacturer, "HotAir Systems Inc");
                    break;
                case 104:
                    heater[objectInstanceID].Temperature = 0.0;
                    break;
                default:
                    break;
                }
                result = AwaLwm2mResult_SuccessCreated;
                break;
            }
            case AwaOperation_Write:
            {
                switch (resourceID)
                {
                    case 101:
                    {
                        int currentLength = strlen(heater[objectInstanceID].Manufacturer) + 1;
                        if ((*dataSize != currentLength) || (memcmp(*dataPointer, heater[objectInstanceID].Manufacturer, currentLength) != 0))
                        {
                            *changed = true;
                            strcpy(heater[objectInstanceID].Manufacturer, "HotAir Systems Inc");
                        }
                        break;
                    }
                    case 104:
                    {
                        AwaFloat newTemperature = **((AwaFloat **)dataPointer);
                        if (newTemperature != heater[objectInstanceID].Temperature)
                        {
                            *changed = true;
                            heater[objectInstanceID].Temperature = newTemperature;
                        }
                        break;
                    }
                    default:
                        break;
                }
                result = AwaLwm2mResult_SuccessChanged;
                break;
            }
            case AwaOperation_Read:
            {
                switch (resourceID)
                {
                    case 101:
                    {
                        *dataPointer = heater[objectInstanceID].Manufacturer;
                        *dataSize = strlen(heater[objectInstanceID].Manufacturer) + 1;
                        break;
                    }
                    case 104:
                    {
                        *dataPointer = &heater[objectInstanceID].Temperature;
                        *dataSize = sizeof(heater[objectInstanceID].Temperature);
                        break;
                    }
                    default:
                        break;
                }
                result = AwaLwm2mResult_SuccessContent;
                break;
            }
            default:
                result = AwaLwm2mResult_InternalError;
                break;
        }
    }
    return result;
}

static void DefineHeaterObject(AwaStaticClient * awaClient)
{
    AwaStaticClient_RegisterObjectWithHandler(awaClient, "Heater", 1000, 0, HEATER_INSTANCES, handler);
    AwaStaticClient_RegisterResourceWithHandler(awaClient, "Manufacturer", 1000, 101, AwaResourceType_String, 0, 1, AwaAccess_ReadWrite, handler);
    AwaStaticClient_RegisterResourceWithHandler(awaClient, "Temperature",  1000, 104, AwaResourceType_Float, 0, 1, AwaAccess_ReadWrite, handler);
}

static void CreateHeaterObject(AwaStaticClient * awaClient)
{
    int instance = 0;
    AwaStaticClient_CreateObjectInstance(awaClient, 1000, instance);
    AwaStaticClient_CreateResource(awaClient, 1000, instance, 101);
    AwaStaticClient_CreateResource(awaClient, 1000, instance, 104);
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
    CreateHeaterObject(awaClient);

    while (1)
    {
        AwaStaticClient_Process(awaClient);

        //heater[0].Temperature = value from hardware
        //AwaStaticClient_ResourceChanged(awaClient, 1000, 0, 104);
    }

    AwaStaticClient_Free(&awaClient);

    return 0;
}

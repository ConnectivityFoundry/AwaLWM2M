/************************************************************************************************************************
 Copyright (c) 2016, Imagination Technologies Limited and/or its affiliated group companies.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
 following conditions are met:
     1. Redistributions of source code must retain the above copyright notice, this list of conditions and the
        following disclaimer.
     2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
        following disclaimer in the documentation and/or other materials provided with the distribution.
     3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote
        products derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
 USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
************************************************************************************************************************/

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
    AwaStaticClient_DefineObject(awaClient, 1000, "Heater", 0, HEATER_INSTANCES);
    AwaStaticClient_DefineResource(awaClient, 1000, 101, "Manufacturer", AwaResourceType_String, 0, 1, AwaResourceOperations_ReadOnly);
    AwaStaticClient_SetResourceStorageWithPointer(awaClient, 1000, 101, &heater[0].Manufacturer, sizeof(heater[0].Manufacturer), sizeof(heater[0]));
    AwaStaticClient_DefineResource(awaClient, 1000, 104, "Temperature",  AwaResourceType_Float,  0, 1, AwaResourceOperations_ReadOnly);
    AwaStaticClient_SetResourceStorageWithPointer(awaClient, 1000, 104, &heater[0].Temperature, sizeof(heater[0].Temperature), sizeof(heater[0]));
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


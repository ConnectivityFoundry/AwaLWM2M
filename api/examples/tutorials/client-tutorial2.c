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

#include <stdlib.h>
#include <stdio.h>

#include <awa/common.h>
#include <awa/client.h>

#define OPERATION_PERFORM_TIMEOUT 1000

static void DefineHeaterObject(AwaClientSession * session)
{
    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(1000, "Heater", 0, 1);
    AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, 101, "Manufacturer", false, AwaResourceOperations_ReadWrite, NULL);
    AwaObjectDefinition_AddResourceDefinitionAsFloat(objectDefinition,  104, "Temperature", false, AwaResourceOperations_ReadWrite, 0.0);

    AwaClientDefineOperation * operation = AwaClientDefineOperation_New(session);
    AwaClientDefineOperation_Add(operation, objectDefinition);
    AwaClientDefineOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientDefineOperation_Free(&operation);
}

static void SetInitialValues(AwaClientSession * session)
{
    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);

    AwaClientSetOperation_CreateObjectInstance(operation, "/1000/0");
    AwaClientSetOperation_CreateOptionalResource(operation, "/1000/0/101");
    AwaClientSetOperation_CreateOptionalResource(operation, "/1000/0/104");
    AwaClientSetOperation_AddValueAsCString(operation, "/1000/0/101", "HotAir Systems Inc");

    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSetOperation_Free(&operation);
}

static void UpdateTemperature(AwaClientSession * session, float temperature)
{    
    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);
   
    AwaClientSetOperation_AddValueAsFloat(operation, "/1000/0/104", temperature);

    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSetOperation_Free(&operation);
}

int main(void)
{
    AwaClientSession * session = AwaClientSession_New();

    AwaClientSession_Connect(session);

    DefineHeaterObject(session);
    SetInitialValues(session);

    while (true)
    {
        float temperature; 
 
        printf("enter temperature or any other key to exit:");
        if (scanf("%8f", &temperature) == 0)
        {
            break;
        }
 
        UpdateTemperature(session, temperature);
        printf("set temperature /1000/0/104 to %f\n", temperature);
    }

    AwaClientSession_Disconnect(session);
    AwaClientSession_Free(&session);
    return 0;
}


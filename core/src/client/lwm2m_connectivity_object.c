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


#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "lwm2m_core.h"
#include "coap_abstraction.h"
#include "lwm2m_connectivity_object.h"
#include "lwm2m_objects.h"


// Default values - typically meaningless
#define DEFAULT_NETWORKBEARER             (0)
#define DEFAULT_AVAILABLENETWORKBEARER    (0)
#define DEFAULT_RADIOSIGNALSTRENTH        (92)
#define DEFAULT_LINKQUALITY               (2)
#define DEFAULT_IPADDRESS                 "192.168.0.100"
#define DEFAULT_PRENTIPADDRESS            "192.168.1.1"
#define DEFAULT_LINKUTILIZATION           (5)
#define DEFAULT_APN                       "internet"


static int Lwm2mConnectivityStatistics_CreateOptionalResourceHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    DefinitionRegistry * definitions = Lwm2mCore_GetDefinitions((Lwm2mContextType *)context);

    ResourceDefinition * definition = Definition_LookupResourceDefinition(definitions, objectID, resourceID);
    if (definition == NULL)
    {
        Lwm2m_Error("No definition for object ID %d Resource ID %d\n", objectID, resourceID);
        goto error;
    }

    if ((resourceID = Lwm2mCore_CreateOptionalResource(context, objectID, objectInstanceID, resourceID)) == -1)
    {
        goto error;
    }

    return 0;
error:
    return -1;
}

static int connectivityStatisticsStartOrReset(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, uint8_t * inValueBuffer, int inValueBufferLen)
{
    Lwm2m_Debug("Reset/Start Connectivity Statistics");

    // Not implemented

    return 0;
}

void Lwm2m_RegisterConnectivityObjects(Lwm2mContextType * context)
{
    {
        int objID = 4;
        Lwm2mCore_RegisterObjectType(context, "ConnectivityMonitoring" , objID, MultipleInstancesEnum_Single, MandatoryEnum_Optional, &defaultObjectOperationHandlers);
        Lwm2mCore_RegisterResourceType(context, "NetworkBearer",          objID, 0,  ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single,   MandatoryEnum_Mandatory, Operations_R, &defaultResourceOperationHandlers);
        Lwm2mCore_RegisterResourceType(context, "AvailableNetworkBearer", objID, 1,  ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Multiple, MandatoryEnum_Mandatory, Operations_R, &defaultResourceOperationHandlers);
        Lwm2mCore_RegisterResourceType(context, "RadioSignalStrength",    objID, 2,  ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single,   MandatoryEnum_Mandatory, Operations_R, &defaultResourceOperationHandlers);
        Lwm2mCore_RegisterResourceType(context, "LinkQuality",            objID, 3,  ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single,   MandatoryEnum_Optional,  Operations_R, &defaultResourceOperationHandlers);
        Lwm2mCore_RegisterResourceType(context, "IPAddresses",            objID, 4,  ResourceTypeEnum_TypeString,  MultipleInstancesEnum_Multiple, MandatoryEnum_Mandatory, Operations_R, &defaultResourceOperationHandlers);
        Lwm2mCore_RegisterResourceType(context, "RouterIPAddresse",       objID, 5,  ResourceTypeEnum_TypeString,  MultipleInstancesEnum_Multiple, MandatoryEnum_Optional,  Operations_R, &defaultResourceOperationHandlers);
        Lwm2mCore_RegisterResourceType(context, "LinkUtilization",        objID, 6,  ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single,   MandatoryEnum_Optional,  Operations_R, &defaultResourceOperationHandlers);
        Lwm2mCore_RegisterResourceType(context, "APN",                    objID, 7,  ResourceTypeEnum_TypeString,  MultipleInstancesEnum_Multiple, MandatoryEnum_Optional,  Operations_R, &defaultResourceOperationHandlers);
        Lwm2mCore_RegisterResourceType(context, "CellID",                 objID, 8,  ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single,   MandatoryEnum_Optional,  Operations_R, &defaultResourceOperationHandlers);
        Lwm2mCore_RegisterResourceType(context, "SMNC",                   objID, 9,  ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single,   MandatoryEnum_Optional,  Operations_R, &defaultResourceOperationHandlers);
        Lwm2mCore_RegisterResourceType(context, "SMCC",                   objID, 10, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single,   MandatoryEnum_Optional,  Operations_R, &defaultResourceOperationHandlers);

        Lwm2mCore_CreateObjectInstance(context, objID, 0);
    }

    ResourceOperationHandlers startOrResetExecuteHandler = {.Execute = connectivityStatisticsStartOrReset, .CreateOptionalResource = Lwm2mConnectivityStatistics_CreateOptionalResourceHandler};
    Lwm2mCore_RegisterObjectType(context, "Connectivity Statistics", LWM2M_CONNECTIVITY_STATISTICS_OBJECT, MultipleInstancesEnum_Single, MandatoryEnum_Optional, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "SMS Tx Counter",        LWM2M_CONNECTIVITY_STATISTICS_OBJECT, LWM2M_CONNECTIVITY_STATISTICS_OBJECT_SMS_TX_COUNTER,   ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional,  Operations_R, &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "SMS Rx Counter",        LWM2M_CONNECTIVITY_STATISTICS_OBJECT, LWM2M_CONNECTIVITY_STATISTICS_OBJECT_SMS_RX_COUNTER,   ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional,  Operations_R, &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "Tx Data",               LWM2M_CONNECTIVITY_STATISTICS_OBJECT, LWM2M_CONNECTIVITY_STATISTICS_OBJECT_TX_DATA,          ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional,  Operations_R, &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "Rx Data",               LWM2M_CONNECTIVITY_STATISTICS_OBJECT, LWM2M_CONNECTIVITY_STATISTICS_OBJECT_RX_DATA,          ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional,  Operations_R, &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "Max Message Size",      LWM2M_CONNECTIVITY_STATISTICS_OBJECT, LWM2M_CONNECTIVITY_STATISTICS_OBJECT_MAX_MESSAGE_SIZE, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional,  Operations_R, &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "Average Message Size",  LWM2M_CONNECTIVITY_STATISTICS_OBJECT, LWM2M_CONNECTIVITY_STATISTICS_OBJECT_AVG_MESSAGE_SIZE, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional,  Operations_R, &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "StartOrReset",          LWM2M_CONNECTIVITY_STATISTICS_OBJECT, LWM2M_CONNECTIVITY_STATISTICS_OBJECT_START_RESET,      ResourceTypeEnum_TypeNone,    MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_E, &startOrResetExecuteHandler);

    // Note: a device should really create its own optional resources
    Lwm2mCore_CreateOptionalResource(context, LWM2M_CONNECTIVITY_MONITORING_OBJECT, 0, 3);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_CONNECTIVITY_MONITORING_OBJECT, 0, 5);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_CONNECTIVITY_MONITORING_OBJECT, 0, 6);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_CONNECTIVITY_MONITORING_OBJECT, 0, 7);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_CONNECTIVITY_MONITORING_OBJECT, 0, 8);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_CONNECTIVITY_MONITORING_OBJECT, 0, 9);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_CONNECTIVITY_MONITORING_OBJECT, 0, 10);

    int networkBearer = DEFAULT_NETWORKBEARER;
    int availableNetworkBearer = DEFAULT_AVAILABLENETWORKBEARER;
    int radioSignalStrength = DEFAULT_RADIOSIGNALSTRENTH;
    int linkQuality = DEFAULT_LINKQUALITY;
    int linkUtilization = DEFAULT_LINKUTILIZATION;

    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_CONNECTIVITY_MONITORING_OBJECT,0,0,0,    &networkBearer,           sizeof(networkBearer));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_CONNECTIVITY_MONITORING_OBJECT,0,1,0,    &availableNetworkBearer,  sizeof(availableNetworkBearer));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_CONNECTIVITY_MONITORING_OBJECT,0,2,0,    &radioSignalStrength,     sizeof(radioSignalStrength));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_CONNECTIVITY_MONITORING_OBJECT,0,3,0,    &linkQuality,             sizeof(linkQuality));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_CONNECTIVITY_MONITORING_OBJECT,0,4,0,    DEFAULT_IPADDRESS,        strlen(DEFAULT_IPADDRESS));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_CONNECTIVITY_MONITORING_OBJECT,0,5,0,    DEFAULT_PRENTIPADDRESS,   strlen(DEFAULT_PRENTIPADDRESS));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_CONNECTIVITY_MONITORING_OBJECT,0,6,0,    &linkUtilization,         sizeof(linkUtilization));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_CONNECTIVITY_MONITORING_OBJECT,0,7,0,    DEFAULT_APN,              strlen(DEFAULT_APN));
}

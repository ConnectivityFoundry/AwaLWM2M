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
#include "lwm2m_device_object.h"
#include "lwm2m_objects.h"

#ifndef CONTIKI
#  include "lwm2m_client_xml_handlers.h"
#endif

#define DEVICE_MANUFACTURER          "Imagination Technologies"
#define DEVICE_MODEL_NUMBER          "Awa Client"
#define DEVICE_SERIAL_NUMBER         "SN12345678"
#define DEVICE_FIRMWARE_VERSION      "0.1a"
static int DEVICE_POWER_SOURCE_1  =  1;
static int DEVICE_POWER_SOURCE_2  =  5;
static int DEVICE_POWER_VOLTAGE_1 =  3800;
static int DEVICE_POWER_VOLTAGE_2 =  5000;
static int DEVICE_POWER_CURRENT_1 =  125;
static int DEVICE_POWER_CURRENT_2 =  900;
static int DEVICE_BATTERY_LEVEL   =  100;
static int DEVICE_MEMORY_FREE     =  15;
static int DEVICE_ERROR_CODE      =  0;
static int64_t DEVICE_TIME        =  0xA20AD72B;
#define DEVICE_TIMEOFFSET            "+12:00"
#define DEVICE_TIME_ZONE             "Pacific/Wellington"
#define DEVICE_BINDING_MODE          "U"
#define DEVICE_DEVICE_TYPE           "Awa Client"
#define DEVICE_HARDWARE_VERSION      "0.0.0.1"
#define DEVICE_SOFTWARE_VERSION      "0.0.0.11"
static int DEVICE_BATTERY_STATUS  =  2;
static int DEVICE_MEMORY_TOTAL    =  42;


// This function is called when an Execute is performed on the Reboot resource /3/X/4
static int executeReboot(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, uint8_t * inValueBuffer, size_t inValueBufferLen)
{
    Lwm2m_Debug("Reboot resource executed\n");
#ifndef CONTIKI
    // Fire IPC notifications to any subscribers.
    xmlif_ExecuteResourceHandler(context, objectID, objectInstanceID, resourceID, inValueBuffer, inValueBufferLen);
#endif
    return 0;
}

static int createRebootResource(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    // Not implemented
    return 0;
}

static int executeFactoryReset(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, uint8_t * inValueBuffer, size_t inValueBufferLen)
{
    Lwm2m_Debug("Factory Reset resource executed\n");
#ifndef CONTIKI
    // Fire IPC notifications to any subscribers.
    xmlif_ExecuteResourceHandler(context, objectID, objectInstanceID, resourceID, inValueBuffer, inValueBufferLen);
#endif
    return 0;
}

static int createFactoryResetResource(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    // Not implemented
    return 0;
}

void Lwm2m_RegisterDeviceObject(Lwm2mContextType * context)
{
    ResourceOperationHandlers rebootOperationHandlers       = { .Execute = executeReboot,       .CreateOptionalResource = createRebootResource };
    ResourceOperationHandlers factoryResetOperationHandlers = { .Execute = executeFactoryReset, .CreateOptionalResource = createFactoryResetResource };

    // Register Device object definition.
    Lwm2mCore_RegisterObjectType(context, "Device" , LWM2M_DEVICE_OBJECT, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, &defaultObjectOperationHandlers);

    Lwm2mCore_RegisterResourceType(context, "Manufacturer",             LWM2M_DEVICE_OBJECT, 0,  AwaResourceType_String,  MultipleInstancesEnum_Single,   MandatoryEnum_Optional,  AwaResourceOperations_ReadOnly,  &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "ModelNumber",              LWM2M_DEVICE_OBJECT, 1,  AwaResourceType_String,  MultipleInstancesEnum_Single,   MandatoryEnum_Optional,  AwaResourceOperations_ReadOnly,  &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "SerialNumber",             LWM2M_DEVICE_OBJECT, 2,  AwaResourceType_String,  MultipleInstancesEnum_Single,   MandatoryEnum_Optional,  AwaResourceOperations_ReadOnly,  &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "FirmwareVersion",          LWM2M_DEVICE_OBJECT, 3,  AwaResourceType_String,  MultipleInstancesEnum_Single,   MandatoryEnum_Optional,  AwaResourceOperations_ReadOnly,  &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "Reboot",                   LWM2M_DEVICE_OBJECT, 4,  AwaResourceType_None,    MultipleInstancesEnum_Single,   MandatoryEnum_Mandatory, AwaResourceOperations_Execute,   &rebootOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "FactoryReset",             LWM2M_DEVICE_OBJECT, 5,  AwaResourceType_None,    MultipleInstancesEnum_Single,   MandatoryEnum_Optional,  AwaResourceOperations_Execute,   &factoryResetOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "AvailablePowerSources",    LWM2M_DEVICE_OBJECT, 6,  AwaResourceType_Integer, MultipleInstancesEnum_Multiple, MandatoryEnum_Optional,  AwaResourceOperations_ReadOnly,  &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "PowerSourceVoltage",       LWM2M_DEVICE_OBJECT, 7,  AwaResourceType_Integer, MultipleInstancesEnum_Multiple, MandatoryEnum_Optional,  AwaResourceOperations_ReadOnly,  &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "PowerSourceCurrent",       LWM2M_DEVICE_OBJECT, 8,  AwaResourceType_Integer, MultipleInstancesEnum_Multiple, MandatoryEnum_Optional,  AwaResourceOperations_ReadOnly,  &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "BatteryLevel",             LWM2M_DEVICE_OBJECT, 9,  AwaResourceType_Integer, MultipleInstancesEnum_Single,   MandatoryEnum_Optional,  AwaResourceOperations_ReadOnly,  &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "MemoryFree",               LWM2M_DEVICE_OBJECT, 10, AwaResourceType_Integer, MultipleInstancesEnum_Single,   MandatoryEnum_Optional,  AwaResourceOperations_ReadOnly,  &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "ErrorCode",                LWM2M_DEVICE_OBJECT, 11, AwaResourceType_Integer, MultipleInstancesEnum_Multiple, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly,  &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "ResetErrorCode",           LWM2M_DEVICE_OBJECT, 12, AwaResourceType_None,    MultipleInstancesEnum_Single,   MandatoryEnum_Optional,  AwaResourceOperations_Execute,   &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "CurrentTime",              LWM2M_DEVICE_OBJECT, 13, AwaResourceType_Time,    MultipleInstancesEnum_Single,   MandatoryEnum_Optional,  AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "UTCOffset",                LWM2M_DEVICE_OBJECT, 14, AwaResourceType_String,  MultipleInstancesEnum_Single,   MandatoryEnum_Optional,  AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "Timezone",                 LWM2M_DEVICE_OBJECT, 15, AwaResourceType_String,  MultipleInstancesEnum_Single,   MandatoryEnum_Optional,  AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "SupportedBindingandModes", LWM2M_DEVICE_OBJECT, 16, AwaResourceType_String,  MultipleInstancesEnum_Single,   MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly,  &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "DeviceType",               LWM2M_DEVICE_OBJECT, 17, AwaResourceType_String,  MultipleInstancesEnum_Single,   MandatoryEnum_Optional,  AwaResourceOperations_ReadOnly,  &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "HardwareVersion",          LWM2M_DEVICE_OBJECT, 18, AwaResourceType_String,  MultipleInstancesEnum_Single,   MandatoryEnum_Optional,  AwaResourceOperations_ReadOnly,  &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "SoftwareVersion",          LWM2M_DEVICE_OBJECT, 19, AwaResourceType_String,  MultipleInstancesEnum_Single,   MandatoryEnum_Optional,  AwaResourceOperations_ReadOnly,  &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "BatteryStatus",            LWM2M_DEVICE_OBJECT, 20, AwaResourceType_Integer, MultipleInstancesEnum_Single,   MandatoryEnum_Optional,  AwaResourceOperations_ReadOnly,  &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "MemoryTotal",              LWM2M_DEVICE_OBJECT, 21, AwaResourceType_Integer, MultipleInstancesEnum_Single,   MandatoryEnum_Optional,  AwaResourceOperations_ReadOnly,  &defaultResourceOperationHandlers);

    // Create object instance.
    Lwm2mCore_CreateObjectInstance(context, LWM2M_DEVICE_OBJECT, 0);

    // Create optional device resources.
    Lwm2mCore_CreateOptionalResource(context, LWM2M_DEVICE_OBJECT, 0, 0);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_DEVICE_OBJECT, 0, 1);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_DEVICE_OBJECT, 0, 2);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_DEVICE_OBJECT, 0, 3);

    Lwm2mCore_CreateOptionalResource(context, LWM2M_DEVICE_OBJECT, 0, 5);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_DEVICE_OBJECT, 0, 6);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_DEVICE_OBJECT, 0, 7);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_DEVICE_OBJECT, 0, 8);

    Lwm2mCore_CreateOptionalResource(context, LWM2M_DEVICE_OBJECT, 0, 9);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_DEVICE_OBJECT, 0, 10);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_DEVICE_OBJECT, 0, 13);

    Lwm2mCore_CreateOptionalResource(context, LWM2M_DEVICE_OBJECT, 0, 14);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_DEVICE_OBJECT, 0, 15);

    Lwm2mCore_CreateOptionalResource(context, LWM2M_DEVICE_OBJECT, 0, 17);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_DEVICE_OBJECT, 0, 18);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_DEVICE_OBJECT, 0, 19);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_DEVICE_OBJECT, 0, 20);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_DEVICE_OBJECT, 0, 21);

    // Populate device object
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_DEVICE_OBJECT, 0, 0, 0,    DEVICE_MANUFACTURER,     strlen(DEVICE_MANUFACTURER));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_DEVICE_OBJECT, 0, 1, 0,    DEVICE_MODEL_NUMBER,     strlen(DEVICE_MODEL_NUMBER));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_DEVICE_OBJECT, 0, 2, 0,    DEVICE_SERIAL_NUMBER,    strlen(DEVICE_SERIAL_NUMBER));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_DEVICE_OBJECT, 0, 3, 0,    DEVICE_FIRMWARE_VERSION, strlen(DEVICE_FIRMWARE_VERSION));

    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_DEVICE_OBJECT, 0, 6, 0,    &DEVICE_POWER_SOURCE_1,  sizeof(DEVICE_POWER_SOURCE_1));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_DEVICE_OBJECT, 0, 7, 0,    &DEVICE_POWER_VOLTAGE_1, sizeof(DEVICE_POWER_VOLTAGE_1));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_DEVICE_OBJECT, 0, 8, 0,    &DEVICE_POWER_CURRENT_1, sizeof(DEVICE_POWER_CURRENT_1));

    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_DEVICE_OBJECT, 0, 6, 1,    &DEVICE_POWER_SOURCE_2,  sizeof(DEVICE_POWER_SOURCE_2));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_DEVICE_OBJECT, 0, 7, 1,    &DEVICE_POWER_VOLTAGE_2, sizeof(DEVICE_POWER_VOLTAGE_2));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_DEVICE_OBJECT, 0, 8, 1,    &DEVICE_POWER_CURRENT_2, sizeof(DEVICE_POWER_CURRENT_2));

    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_DEVICE_OBJECT, 0, 9, 0,    &DEVICE_BATTERY_LEVEL,   sizeof(DEVICE_BATTERY_LEVEL));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_DEVICE_OBJECT, 0, 10, 0,   &DEVICE_MEMORY_FREE,     sizeof(DEVICE_MEMORY_FREE));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_DEVICE_OBJECT, 0, 11, 0,   &DEVICE_ERROR_CODE,      sizeof(DEVICE_ERROR_CODE));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_DEVICE_OBJECT, 0, 13, 0,   &DEVICE_TIME,            sizeof(DEVICE_TIME));

    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_DEVICE_OBJECT, 0, 14, 0,   DEVICE_TIMEOFFSET,       strlen(DEVICE_TIMEOFFSET));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_DEVICE_OBJECT, 0, 15, 0,   DEVICE_TIME_ZONE,        strlen(DEVICE_TIME_ZONE));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_DEVICE_OBJECT, 0, 16, 0,   DEVICE_BINDING_MODE,     strlen(DEVICE_BINDING_MODE));

    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_DEVICE_OBJECT, 0, 17, 0,   DEVICE_DEVICE_TYPE,      strlen(DEVICE_DEVICE_TYPE));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_DEVICE_OBJECT, 0, 18, 0,   DEVICE_HARDWARE_VERSION, strlen(DEVICE_HARDWARE_VERSION));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_DEVICE_OBJECT, 0, 19, 0,   DEVICE_SOFTWARE_VERSION, strlen(DEVICE_SOFTWARE_VERSION));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_DEVICE_OBJECT, 0, 20, 0,   &DEVICE_BATTERY_STATUS,  sizeof(DEVICE_BATTERY_STATUS));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_DEVICE_OBJECT, 0, 21, 0,   &DEVICE_MEMORY_TOTAL,    sizeof(DEVICE_MEMORY_TOTAL));
}

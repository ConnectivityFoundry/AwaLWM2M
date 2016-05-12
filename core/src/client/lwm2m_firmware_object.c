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
#include <string.h>
#include <stdint.h>

#include "lwm2m_types.h"
#include "lwm2m_core.h"
#include "lwm2m_debug.h"
#include "lwm2m_objects.h"

#ifndef CONTIKI
#  include "lwm2m_client_xml_handlers.h"
#endif

#define FIRMWARE_PKG_NAME                "LWM2M-IMG-218430"
#define FIRMWARE_PKG_VERSION             "1.2"


static int executeUpdate(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, uint8_t * inValueBuffer, size_t inValueBufferLen)
{
    Lwm2m_Debug("Firmware Update resource executed\n");
#ifndef CONTIKI
    // Fire IPC notifications to any subscribers.
    xmlif_ExecuteResourceHandler(context, objectID, objectInstanceID, resourceID, inValueBuffer, inValueBufferLen);
#endif
    return 0;
}

void Lwm2m_RegisterFirmwareObject(Lwm2mContextType * context)
{
    ResourceOperationHandlers firmwareUpdateOperationHandlers = { .Execute = executeUpdate, .CreateOptionalResource = defaultResourceOperationHandlers.CreateOptionalResource };

    Lwm2mCore_RegisterObjectType(context, "FirmwareUpdate",           LWM2M_FIRMWARE_OBJECT, MultipleInstancesEnum_Single, MandatoryEnum_Optional, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "Package",                LWM2M_FIRMWARE_OBJECT, 0, AwaResourceType_Opaque,  MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_WriteOnly,  &defaultResourceOperationHandlers);// FIXME: &packageOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "PackageURI",             LWM2M_FIRMWARE_OBJECT, 1, AwaResourceType_String,  MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_WriteOnly,  &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "Update",                 LWM2M_FIRMWARE_OBJECT, 2, AwaResourceType_None,    MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_Execute,    &firmwareUpdateOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "State",                  LWM2M_FIRMWARE_OBJECT, 3, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly,   &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "UpdateSupportedObjects", LWM2M_FIRMWARE_OBJECT, 4, AwaResourceType_Boolean, MultipleInstancesEnum_Single, MandatoryEnum_Optional,  AwaResourceOperations_ReadWrite,  &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "UpdateResult",           LWM2M_FIRMWARE_OBJECT, 5, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly,   &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "PkgName",                LWM2M_FIRMWARE_OBJECT, 6, AwaResourceType_String,  MultipleInstancesEnum_Single, MandatoryEnum_Optional,  AwaResourceOperations_ReadOnly,   &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "PkgVersion",             LWM2M_FIRMWARE_OBJECT, 7, AwaResourceType_String,  MultipleInstancesEnum_Single, MandatoryEnum_Optional,  AwaResourceOperations_ReadOnly,   &defaultResourceOperationHandlers);

    // Create single optional object instance
    Lwm2mCore_CreateObjectInstance(context, LWM2M_FIRMWARE_OBJECT, 0);

    // Create optional device resources
    Lwm2mCore_CreateOptionalResource(context, LWM2M_FIRMWARE_OBJECT, 0, 4);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_FIRMWARE_OBJECT, 0, 6);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_FIRMWARE_OBJECT, 0, 7);

    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_FIRMWARE_OBJECT, 0, 6, 0, (void*)FIRMWARE_PKG_NAME,    sizeof(FIRMWARE_PKG_NAME));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_FIRMWARE_OBJECT, 0, 7, 0, (void*)FIRMWARE_PKG_VERSION, sizeof(FIRMWARE_PKG_VERSION));
}


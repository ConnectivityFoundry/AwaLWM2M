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
#include "lwm2m_location_object.h"


void Lwm2m_RegisterLocationObject(Lwm2mContextType * context)
{
    Lwm2mCore_RegisterObjectType(context, "Location" , LWM2M_LOCATION_OBJECT, MultipleInstancesEnum_Single, MandatoryEnum_Optional, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "Latitude",    LWM2M_LOCATION_OBJECT, 0, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_R, &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "Longitude",   LWM2M_LOCATION_OBJECT, 1, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_R, &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "Altitude",    LWM2M_LOCATION_OBJECT, 2, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Optional,  Operations_R, &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "Uncertainty", LWM2M_LOCATION_OBJECT, 3, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Optional,  Operations_R, &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "Velocity",    LWM2M_LOCATION_OBJECT, 4, AwaResourceType_Opaque, MultipleInstancesEnum_Single, MandatoryEnum_Optional,  Operations_R, &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "Timestamp",   LWM2M_LOCATION_OBJECT, 5, AwaResourceType_Time,   MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_R, &defaultResourceOperationHandlers);

    Lwm2mCore_CreateObjectInstance(context, LWM2M_LOCATION_OBJECT, 0);
}

void Lwm2mLocation_SetLocation(Lwm2mContextType * context, const char * latitude, const char * longitude, uint64_t timestamp)
{
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_LOCATION_OBJECT, 0, 0, 0, latitude,   strlen(latitude));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_LOCATION_OBJECT, 0, 1, 0, longitude,  strlen(longitude));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_LOCATION_OBJECT, 0, 5, 0, &timestamp, sizeof(timestamp));
}

void Lwm2mLocation_SetVelocity(Lwm2mContextType * context, uint8_t * velocity, unsigned char len)
{
    if (!Lwm2mCore_Exists(context, LWM2M_LOCATION_OBJECT, 0, 4))
    {
        Lwm2mCore_CreateOptionalResource(context, LWM2M_LOCATION_OBJECT, 0, 4);
    }
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_LOCATION_OBJECT, 0, 4, 0, velocity, len);
}

void Lwm2mLocation_SetAltitude(Lwm2mContextType * context, const char * altitude)
{
    if (!Lwm2mCore_Exists(context, LWM2M_LOCATION_OBJECT, 0, 2))
    {
        Lwm2mCore_CreateOptionalResource(context, LWM2M_LOCATION_OBJECT, 0, 2);
    }
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_LOCATION_OBJECT, 0, 2, 0, altitude, strlen(altitude));
}

void Lwm2mLocation_SetUncertainty(Lwm2mContextType * context, const char * uncertainty)
{
    if (!Lwm2mCore_Exists(context, LWM2M_LOCATION_OBJECT, 0, 3))
    {
        Lwm2mCore_CreateOptionalResource(context, LWM2M_LOCATION_OBJECT, 0, 3);
    }
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_LOCATION_OBJECT, 0, 3, 0, uncertainty, strlen(uncertainty));
}

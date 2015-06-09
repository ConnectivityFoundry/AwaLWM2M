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
#include "lwm2m_object_store.h"
#include "lwm2m_debug.h"
#include "lwm2m_objects.h"

void Lwm2m_RegisterObjectTypes(Lwm2mContextType * context)
{
    {
        int objID = LWM2M_ACL_OBJECT;
        Lwm2mCore_RegisterObjectType(context, "LWM2MAccessControl" , objID, MultipleInstancesEnum_Multiple, MandatoryEnum_Optional, NULL);
        Lwm2mCore_RegisterResourceType(context, "ObjectID", objID, 0, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "ObjectInstanceID", objID, 1, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "ACL", objID, 2, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Multiple, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "AccessControlOwner", objID, 3, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_R, NULL);
    }
    {
        int objID = LWM2M_CONNECTIVITY_MONITORING_OBJECT;
        Lwm2mCore_RegisterObjectType(context, "ConnectivityMonitoring" , objID, MultipleInstancesEnum_Single, MandatoryEnum_Optional, NULL);
        Lwm2mCore_RegisterResourceType(context, "NetworkBearer", objID, 0, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "AvailableNetworkBearer", objID, 1, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Multiple, MandatoryEnum_Mandatory, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "RadioSignalStrength", objID, 2, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "LinkQuality", objID, 3, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "IPAddresses", objID, 4, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Multiple, MandatoryEnum_Mandatory, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "RouterIPAddresse", objID, 5, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Multiple, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "LinkUtilization", objID, 6, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "APN", objID, 7, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Multiple, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "CellID", objID, 8, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "SMNC", objID, 9, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "SMCC", objID, 10, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
    }
    {
        int objID = LWM2M_CONNECTIVITY_STATISTICS_OBJECT;
        Lwm2mCore_RegisterObjectType(context, "ConnectivityStatistics" , objID, MultipleInstancesEnum_Single, MandatoryEnum_Optional, NULL);
        Lwm2mCore_RegisterResourceType(context, "SMSTxCounter", objID, 0, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "SMSRxCounter", objID, 1, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "TxData", objID, 2, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "RxData", objID, 3, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "MaxMessageSize", objID, 4, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "AverageMessageSize", objID, 5, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "StartOrReset", objID, 6, ResourceTypeEnum_TypeNone, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_E, NULL);
    }
    {
        int objID = LWM2M_DEVICE_OBJECT;
        Lwm2mCore_RegisterObjectType(context, "Device" , objID, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, NULL);
        Lwm2mCore_RegisterResourceType(context, "Manufacturer", objID, 0, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "ModelNumber", objID, 1, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "SerialNumber", objID, 2, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "FirmwareVersion", objID, 3, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "Reboot", objID, 4, ResourceTypeEnum_TypeNone, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_E, NULL);
        Lwm2mCore_RegisterResourceType(context, "FactoryReset", objID, 5, ResourceTypeEnum_TypeNone, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_E, NULL);
        Lwm2mCore_RegisterResourceType(context, "AvailablePowerSources", objID, 6, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Multiple, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "PowerSourceVoltage", objID, 7, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Multiple, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "PowerSourceCurrent", objID, 8, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Multiple, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "BatteryLevel", objID, 9, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "MemoryFree", objID, 10, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "ErrorCode", objID, 11, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Multiple, MandatoryEnum_Mandatory, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "ResetErrorCode", objID, 12, ResourceTypeEnum_TypeNone, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_E, NULL);
        Lwm2mCore_RegisterResourceType(context, "CurrentTime", objID, 13, ResourceTypeEnum_TypeTime, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "UTCOffset", objID, 14, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "Timezone", objID, 15, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "SupportedBindingandModes", objID, 16, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "DeviceType", objID, 17, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "HardwareVersion", objID, 18, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "SoftwareVersion", objID, 19, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "BatteryStatus", objID, 20, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "MemoryTotal", objID, 21, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
    }
    {
        int objID = LWM2M_FIRMWARE_OBJECT;
        Lwm2mCore_RegisterObjectType(context, "FirmwareUpdate" , objID, MultipleInstancesEnum_Single, MandatoryEnum_Optional, NULL);
        Lwm2mCore_RegisterResourceType(context, "Package", objID, 0, ResourceTypeEnum_TypeOpaque, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_W, NULL);
        Lwm2mCore_RegisterResourceType(context, "PackageURI", objID, 1, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_W, NULL);
        Lwm2mCore_RegisterResourceType(context, "Update", objID, 2, ResourceTypeEnum_TypeNone, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_E, NULL);
        Lwm2mCore_RegisterResourceType(context, "State", objID, 3, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "UpdateSupportedObjects", objID, 4, ResourceTypeEnum_TypeBoolean, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "UpdateResult", objID, 5, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "PkgName", objID, 6, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "PkgVersion", objID, 7, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
    }
    {
        int objID = LWM2M_LOCATION_OBJECT;
        Lwm2mCore_RegisterObjectType(context, "Location" , objID, MultipleInstancesEnum_Single, MandatoryEnum_Optional, NULL);
        Lwm2mCore_RegisterResourceType(context, "Latitude", objID, 0, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "Longitude", objID, 1, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "Altitude", objID, 2, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "Uncertainty", objID, 3, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "Velocity", objID, 4, ResourceTypeEnum_TypeOpaque, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "Timestamp", objID, 5, ResourceTypeEnum_TypeTime, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_R, NULL);
    }
    {
        int objID = LWM2M_SECURITY_OBJECT;
        Lwm2mCore_RegisterObjectType(context, "LWM2MSecurity" , objID, MultipleInstancesEnum_Multiple, MandatoryEnum_Mandatory, NULL);
        Lwm2mCore_RegisterResourceType(context, "LWM2MServerURI", objID, 0, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_None, NULL);
        Lwm2mCore_RegisterResourceType(context, "BootstrapServer", objID, 1, ResourceTypeEnum_TypeBoolean, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_None, NULL);
        Lwm2mCore_RegisterResourceType(context, "SecurityMode", objID, 2, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_None, NULL);
        Lwm2mCore_RegisterResourceType(context, "PublicKeyorIDentity", objID, 3, ResourceTypeEnum_TypeOpaque, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_None, NULL);
        Lwm2mCore_RegisterResourceType(context, "ServerPublicKeyorIDentity", objID, 4, ResourceTypeEnum_TypeOpaque, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_None, NULL);
        Lwm2mCore_RegisterResourceType(context, "SecretKey", objID, 5, ResourceTypeEnum_TypeOpaque, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_None, NULL);
        Lwm2mCore_RegisterResourceType(context, "SMSSecurityMode", objID, 6, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_None, NULL);
        Lwm2mCore_RegisterResourceType(context, "SMSBindingKeyParameters", objID, 7, ResourceTypeEnum_TypeOpaque, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_None, NULL);
        Lwm2mCore_RegisterResourceType(context, "SMSBindingSecretKeys", objID, 8, ResourceTypeEnum_TypeOpaque, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_None, NULL);
        Lwm2mCore_RegisterResourceType(context, "LWM2MServerSMSNumber", objID, 9, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_None, NULL);
        Lwm2mCore_RegisterResourceType(context, "ShortServerID", objID, 10, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_None, NULL);
        Lwm2mCore_RegisterResourceType(context, "ClientHoldOffTime", objID, 11, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_None, NULL);
    }
    {
        int objID = LWM2M_SERVER_OBJECT;
        Lwm2mCore_RegisterObjectType(context, "LWM2MServer" , objID, MultipleInstancesEnum_Multiple, MandatoryEnum_Mandatory, NULL);
        Lwm2mCore_RegisterResourceType(context, "ShortServerID", objID, 0, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "Lifetime", objID, 1, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "DefaultMinimumPeriod", objID, 2, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "DefaultMaximumPeriod", objID, 3, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "Disable", objID, 4, ResourceTypeEnum_TypeNone, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_E, NULL);
        Lwm2mCore_RegisterResourceType(context, "DisableTimeout", objID, 5, ResourceTypeEnum_TypeInteger, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "NotificationStoringWhenDisabledorOffline", objID, 6, ResourceTypeEnum_TypeBoolean, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "Binding", objID, 7, ResourceTypeEnum_TypeString, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_R, NULL);
        Lwm2mCore_RegisterResourceType(context, "RegistrationUpdateTrigger", objID, 8, ResourceTypeEnum_TypeNone, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, Operations_E, NULL);
    }
}

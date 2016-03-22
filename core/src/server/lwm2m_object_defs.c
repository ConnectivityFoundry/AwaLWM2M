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
        Lwm2mCore_RegisterResourceType(context, "ObjectID", objID, 0, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "ObjectInstanceID", objID, 1, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "ACL", objID, 2, AwaResourceType_Integer, MultipleInstancesEnum_Multiple, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "AccessControlOwner", objID, 3, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly, NULL);
    }
    {
        int objID = LWM2M_CONNECTIVITY_MONITORING_OBJECT;
        Lwm2mCore_RegisterObjectType(context, "ConnectivityMonitoring" , objID, MultipleInstancesEnum_Single, MandatoryEnum_Optional, NULL);
        Lwm2mCore_RegisterResourceType(context, "NetworkBearer", objID, 0, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "AvailableNetworkBearer", objID, 1, AwaResourceType_Integer, MultipleInstancesEnum_Multiple, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "RadioSignalStrength", objID, 2, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "LinkQuality", objID, 3, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "IPAddresses", objID, 4, AwaResourceType_String, MultipleInstancesEnum_Multiple, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "RouterIPAddresse", objID, 5, AwaResourceType_String, MultipleInstancesEnum_Multiple, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "LinkUtilization", objID, 6, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "APN", objID, 7, AwaResourceType_String, MultipleInstancesEnum_Multiple, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "CellID", objID, 8, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "SMNC", objID, 9, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "SMCC", objID, 10, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
    }
    {
        int objID = LWM2M_CONNECTIVITY_STATISTICS_OBJECT;
        Lwm2mCore_RegisterObjectType(context, "ConnectivityStatistics" , objID, MultipleInstancesEnum_Single, MandatoryEnum_Optional, NULL);
        Lwm2mCore_RegisterResourceType(context, "SMSTxCounter", objID, 0, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "SMSRxCounter", objID, 1, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "TxData", objID, 2, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "RxData", objID, 3, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "MaxMessageSize", objID, 4, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "AverageMessageSize", objID, 5, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "StartOrReset", objID, 6, AwaResourceType_None, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_Execute, NULL);
    }
    {
        int objID = LWM2M_DEVICE_OBJECT;
        Lwm2mCore_RegisterObjectType(context, "Device" , objID, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, NULL);
        Lwm2mCore_RegisterResourceType(context, "Manufacturer", objID, 0, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "ModelNumber", objID, 1, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "SerialNumber", objID, 2, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "FirmwareVersion", objID, 3, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "Reboot", objID, 4, AwaResourceType_None, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_Execute, NULL);
        Lwm2mCore_RegisterResourceType(context, "FactoryReset", objID, 5, AwaResourceType_None, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_Execute, NULL);
        Lwm2mCore_RegisterResourceType(context, "AvailablePowerSources", objID, 6, AwaResourceType_Integer, MultipleInstancesEnum_Multiple, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "PowerSourceVoltage", objID, 7, AwaResourceType_Integer, MultipleInstancesEnum_Multiple, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "PowerSourceCurrent", objID, 8, AwaResourceType_Integer, MultipleInstancesEnum_Multiple, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "BatteryLevel", objID, 9, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "MemoryFree", objID, 10, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "ErrorCode", objID, 11, AwaResourceType_Integer, MultipleInstancesEnum_Multiple, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "ResetErrorCode", objID, 12, AwaResourceType_None, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_Execute, NULL);
        Lwm2mCore_RegisterResourceType(context, "CurrentTime", objID, 13, AwaResourceType_Time, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "UTCOffset", objID, 14, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "Timezone", objID, 15, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "SupportedBindingandModes", objID, 16, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "DeviceType", objID, 17, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "HardwareVersion", objID, 18, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "SoftwareVersion", objID, 19, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "BatteryStatus", objID, 20, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "MemoryTotal", objID, 21, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
    }
    {
        int objID = LWM2M_FIRMWARE_OBJECT;
        Lwm2mCore_RegisterObjectType(context, "FirmwareUpdate" , objID, MultipleInstancesEnum_Single, MandatoryEnum_Optional, NULL);
        Lwm2mCore_RegisterResourceType(context, "Package", objID, 0, AwaResourceType_Opaque, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_WriteOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "PackageURI", objID, 1, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_WriteOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "Update", objID, 2, AwaResourceType_None, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_Execute, NULL);
        Lwm2mCore_RegisterResourceType(context, "State", objID, 3, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "UpdateSupportedObjects", objID, 4, AwaResourceType_Boolean, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "UpdateResult", objID, 5, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "PkgName", objID, 6, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "PkgVersion", objID, 7, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
    }
    {
        int objID = LWM2M_LOCATION_OBJECT;
        Lwm2mCore_RegisterObjectType(context, "Location" , objID, MultipleInstancesEnum_Single, MandatoryEnum_Optional, NULL);
        Lwm2mCore_RegisterResourceType(context, "Latitude", objID, 0, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "Longitude", objID, 1, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "Altitude", objID, 2, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "Uncertainty", objID, 3, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "Velocity", objID, 4, AwaResourceType_Opaque, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "Timestamp", objID, 5, AwaResourceType_Time, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly, NULL);
    }
    {
        int objID = LWM2M_SECURITY_OBJECT;
        Lwm2mCore_RegisterObjectType(context, "LWM2MSecurity" , objID, MultipleInstancesEnum_Multiple, MandatoryEnum_Mandatory, NULL);
        Lwm2mCore_RegisterResourceType(context, "LWM2MServerURI", objID, 0, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_None, NULL);
        Lwm2mCore_RegisterResourceType(context, "BootstrapServer", objID, 1, AwaResourceType_Boolean, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_None, NULL);
        Lwm2mCore_RegisterResourceType(context, "SecurityMode", objID, 2, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_None, NULL);
        Lwm2mCore_RegisterResourceType(context, "PublicKeyorIDentity", objID, 3, AwaResourceType_Opaque, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_None, NULL);
        Lwm2mCore_RegisterResourceType(context, "ServerPublicKeyorIDentity", objID, 4, AwaResourceType_Opaque, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_None, NULL);
        Lwm2mCore_RegisterResourceType(context, "SecretKey", objID, 5, AwaResourceType_Opaque, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_None, NULL);
        Lwm2mCore_RegisterResourceType(context, "SMSSecurityMode", objID, 6, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_None, NULL);
        Lwm2mCore_RegisterResourceType(context, "SMSBindingKeyParameters", objID, 7, AwaResourceType_Opaque, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_None, NULL);
        Lwm2mCore_RegisterResourceType(context, "SMSBindingSecretKeys", objID, 8, AwaResourceType_Opaque, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_None, NULL);
        Lwm2mCore_RegisterResourceType(context, "LWM2MServerSMSNumber", objID, 9, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_None, NULL);
        Lwm2mCore_RegisterResourceType(context, "ShortServerID", objID, 10, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_None, NULL);
        Lwm2mCore_RegisterResourceType(context, "ClientHoldOffTime", objID, 11, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_None, NULL);
    }
    {
        int objID = LWM2M_SERVER_OBJECT;
        Lwm2mCore_RegisterObjectType(context, "LWM2MServer" , objID, MultipleInstancesEnum_Multiple, MandatoryEnum_Mandatory, NULL);
        Lwm2mCore_RegisterResourceType(context, "ShortServerID", objID, 0, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "Lifetime", objID, 1, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "DefaultMinimumPeriod", objID, 2, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "DefaultMaximumPeriod", objID, 3, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "Disable", objID, 4, AwaResourceType_None, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_Execute, NULL);
        Lwm2mCore_RegisterResourceType(context, "DisableTimeout", objID, 5, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "NotificationStoringWhenDisabledorOffline", objID, 6, AwaResourceType_Boolean, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "Binding", objID, 7, AwaResourceType_String, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly, NULL);
        Lwm2mCore_RegisterResourceType(context, "RegistrationUpdateTrigger", objID, 8, AwaResourceType_None, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_Execute, NULL);
    }
}

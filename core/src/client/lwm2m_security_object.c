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
#include "lwm2m_objects.h"
#include "lwm2m_security_object.h"
#include "lwm2m_server_object.h"


#define SECURITY_STRING_LEN (64)
#define SERVER_URI_LEN (255)

static int SECURITY_BS =       1;
static int SECURITY_SECMODE =  LWM2MSecurityMode_PSK;
static int SECURITY_SERVERID = 0;
static int SECURITY_HOLDOFF =  0;


typedef struct
{
    struct ListHead list;             //< Prev/next pointers

    char ServerURI[SERVER_URI_LEN];   //< Uniquely identifies the LWM2M server or Bootstrap server. i.e coap://host:port
    AddressType address;              //< Stores the host IP address and port
    bool AddressResolved;
    bool IsBootstrapServer;
    LWM2MSecurityMode SecurityMode;
    char PublicKeyIdentity[SECURITY_STRING_LEN];         //< Stores the LWM2M client certificate, public key, or PSK identity
    int PublicKeyIdentityLength;
    char ServerPublicKey[SECURITY_STRING_LEN];           //< Stores the LWM2M server or bootstrap server certificate, public key or PSK identity.
    int ServerPublicKeyLength;
    char SecretKey[SECURITY_STRING_LEN];                 //< Stores the secret key, or private key of the security mode.
    int SecretKeyLength;
    int SmsSecurityMode;
    char SmsBindingKeyParams[SECURITY_STRING_LEN];
    int SmsBindingKeyParamsLength;
    char SmsBindingKeySecret[SECURITY_STRING_LEN];
    int SmsBindingKeySecretLength;
    int ServerSmsNumber;
    int ShortServerID;
    int ClientHoldOffTime;

    ObjectInstanceIDType objectInstanceID;
} LWM2MSecurityInfo;



static int Lwm2mSecurity_ResourceReadHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID,
                                             ResourceInstanceIDType resourceInstanceID, const void ** buffer, size_t * bufferLen);

static int Lwm2mSecurity_ResourceWriteHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID,
                                              ResourceInstanceIDType resourceInstanceID, uint8_t * srcBuffer, size_t srcBufferLen, bool * changed);

static int Lwm2mSecurity_ObjectCreateInstanceHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID);

static int Lwm2mSecurity_ObjectDeleteHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);

static int Lwm2mSecurity_CreateOptionalResourceHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);

static void DestroySecurityObjects(struct ListHead * securityObjectsList);


void Lwm2mSecurity_Create(struct ListHead * securityObjectList)
{
    ListInit(securityObjectList);
}

void Lwm2mSecurity_Destroy(struct ListHead * securityObjectList)
{
    DestroySecurityObjects(securityObjectList);
}

static void DestroySecurityObjects(struct ListHead * securityObjectList)
{
    struct ListHead * i, * n;
    ListForEachSafe(i, n, securityObjectList)
    {
        LWM2MSecurityInfo * securityInfo = ListEntry(i, LWM2MSecurityInfo, list);
        if (securityInfo != NULL)
        {
            free(securityInfo);
        }
    }
}

static LWM2MSecurityInfo * GetSecurityInfo(Lwm2mContextType * context, ObjectInstanceIDType objectInstanceID)
{
    struct ListHead * current;
    ListForEach(current, Lwm2mCore_GetSecurityObjectList(context))
    {
        LWM2MSecurityInfo * securityInfo = ListEntry(current, LWM2MSecurityInfo, list);
        if (securityInfo->objectInstanceID == objectInstanceID)
        {
            return securityInfo;
        }
    }
    return NULL;
}

static LWM2MSecurityInfo * GetSecurityInfoForShortServerID(Lwm2mContextType * context, int shortServerID)
{
    struct ListHead * current;
    ListForEach(current, Lwm2mCore_GetSecurityObjectList(context))
    {
        LWM2MSecurityInfo * securityInfo = ListEntry(current, LWM2MSecurityInfo, list);
        if (securityInfo->ShortServerID == shortServerID)
        {
            return securityInfo;
        }
    }
    return NULL;
}

static LWM2MSecurityInfo * AddSecurityInfo(Lwm2mContextType * context, ObjectInstanceIDType objectInstanceID)
{
    LWM2MSecurityInfo * new = NULL;
    if (GetSecurityInfo(context, objectInstanceID) == NULL)
    {
        new = malloc(sizeof(LWM2MSecurityInfo));
        if (new != NULL)
        {
            memset(new, 0, sizeof(LWM2MSecurityInfo));
            new->objectInstanceID = objectInstanceID;
            ListAdd(&new->list, Lwm2mCore_GetSecurityObjectList(context));
        }
        else
        {
            Lwm2m_Error("Failed to allocate memory for security object (instance %d)\n", objectInstanceID);
        }
    }
    return new;
}

bool RemoveSecurityInfo(Lwm2mContextType * context, ObjectInstanceIDType objectInstanceID)
{
    bool result = false;
    LWM2MSecurityInfo * info = GetSecurityInfo(context, objectInstanceID);
    if (info != NULL)
    {
        ListRemove(&info->list);
        free(info);
        result = true;
    }
    return result;
}


static ObjectOperationHandlers securityObjectOperationHandlers =
{
    .CreateInstance = Lwm2mSecurity_ObjectCreateInstanceHandler,
    .Delete = Lwm2mSecurity_ObjectDeleteHandler,
};

static ResourceOperationHandlers securityResourceOperationHandlers =
{
    .Read = Lwm2mSecurity_ResourceReadHandler,
    .Write = Lwm2mSecurity_ResourceWriteHandler,
    .Execute = NULL,
    .CreateOptionalResource = Lwm2mSecurity_CreateOptionalResourceHandler,
};


static int Lwm2mSecurity_ObjectCreateInstanceHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID)
{
    return (AddSecurityInfo(context, objectInstanceID) != NULL) ? objectInstanceID : -1;
}

static int Lwm2mSecurity_CreateOptionalResourceHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    // No action
    return 0;
}

static int Lwm2mSecurity_DeleteObjectInstance(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID)
{
    int result = -1;
    LWM2MSecurityInfo * info = GetSecurityInfo(context, objectInstanceID);
    if (info != NULL)
    {
        // Can only delete non bootstrap server entries
        if (!info->IsBootstrapServer)
        {
            if(RemoveSecurityInfo(context, objectInstanceID))
            {
                result = 0;
            }
        }
    }
    return result;
}

static int Lwm2mSecurity_ObjectDeleteHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    // 8.2.2 Delete operation MAY target to “/” URI to delete all the existing Object Instances except LWM2M
    // Bootstrap Server Account in the LWM2M Client.

    if (objectID != LWM2M_SECURITY_OBJECT)
    {
        Lwm2m_Error("Lwm2mSecurity_ObjectDeleteHandler Invalid OIR: %d/%d/%d\n", objectID, objectInstanceID, resourceID);
        return -1;
    }

    if (objectInstanceID != -1)
    {
        return Lwm2mSecurity_DeleteObjectInstance((Lwm2mContextType *)context, objectID, objectInstanceID);
    }
    else
    {

        ObjectIDType objectID = -1;
        while ((objectID = Lwm2mCore_GetNextObjectID(context, objectID)) != -1)
        {
            // Best effort attempt
            Lwm2mSecurity_DeleteObjectInstance((Lwm2mContextType *)context, objectID, objectInstanceID);
        }
        return 0;
    }
}

static int Lwm2mSecurity_ResourceReadHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID,
                                             ResourceInstanceIDType resourceInstanceID, const void ** buffer, size_t * bufferLen)
{
    LWM2MSecurityInfo * security = GetSecurityInfo(context, objectInstanceID);

    if (security != NULL)
    {
        switch (resourceID)
        {
            case LWM2M_SECURITY_OBJECT_SERVER_URI:
                *buffer = security->ServerURI;
                *bufferLen = strlen(security->ServerURI);
                break;

            case LWM2M_SECURITY_OBJECT_BOOTSTRAP_SERVER:
                *buffer = &security->IsBootstrapServer;
                *bufferLen = sizeof(security->IsBootstrapServer);
                break;

            case LWM2M_SECURITY_OBJECT_SECURITY_MODE:
                *buffer = &security->SecurityMode;
                *bufferLen = sizeof(security->SecurityMode);
                break;

            case LWM2M_SECURITY_OBJECT_PUBLIC_KEY:
                *buffer = &security->PublicKeyIdentity;
                *bufferLen = security->PublicKeyIdentityLength;
                break;

            case LWM2M_SECURITY_OBJECT_SERVER_PUBLIC_KEY:
                *buffer = &security->ServerPublicKey;
                *bufferLen = security->ServerPublicKeyLength;
                break;

            case LWM2M_SECURITY_OBJECT_SECRET_KEY:
                *buffer = &security->SecretKey;
                *bufferLen = security->SecretKeyLength;
                break;

            case LWM2M_SECURITY_OBJECT_SMS_MODE:
                *buffer = &security->SmsSecurityMode;
                *bufferLen = sizeof(security->SmsSecurityMode);
                break;

            case LWM2M_SECURITY_OBJECT_SMS_BINDING_PARAM:
                *buffer = &security->SmsBindingKeyParams;
                *bufferLen = security->SmsBindingKeyParamsLength;
                break;

            case LWM2M_SECURITY_OBJECT_SMS_BINDING_SECRET_KEY:
                *buffer = &security->SmsBindingKeySecret;
                *bufferLen = security->SmsBindingKeySecretLength;
                break;

            case LWM2M_SECURITY_OBJECT_SMS_NUMBER:
                *buffer = &security->ServerSmsNumber;
                *bufferLen = sizeof(security->ServerSmsNumber);
                break;

            case LWM2M_SECURITY_OBJECT_SHORT_SERVER_ID:
                *buffer = &security->ShortServerID;
                *bufferLen = sizeof(security->ShortServerID);
                break;

            case LWM2M_SECURITY_OBJECT_CLIENT_HOLD_OFF:
                *buffer = &security->ClientHoldOffTime;
                *bufferLen = sizeof(security->ClientHoldOffTime);
                break;

            default:
                *bufferLen = 0;
        }

    }
    else
    {
        *buffer = NULL;
        *bufferLen = 0;
    }

    return *bufferLen;
}

// Warn via log if the expected value size is larger than the supplied buffer size
static void WarnOfInsufficientData(size_t dest_size, size_t src_size)
{
    if (dest_size > src_size)
    {
        Lwm2m_Warning("Source size is insufficient in Write (expect %zu, got %zu)\n", dest_size, src_size);
    }
}

static int Lwm2mSecurity_ResourceWriteHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID,
                                              ResourceInstanceIDType resourceInstanceID, uint8_t * srcBuffer, size_t srcBufferLen, bool * changed)
{
    int result = -1;
    LWM2MSecurityInfo * security =  GetSecurityInfo(context, objectInstanceID);

    if (security != NULL)
    {
        switch (resourceID)
        {
            case LWM2M_SECURITY_OBJECT_SERVER_URI:
                if (srcBufferLen <= SERVER_URI_LEN)
                {
                    memset(security->ServerURI, 0, sizeof(security->ServerURI));
                    memcpy(security->ServerURI, srcBuffer, srcBufferLen);
                    // Recalculate socket address from URI
                    security->AddressResolved = coap_ResolveAddressByURI(security->ServerURI, &security->address);
                    result = srcBufferLen;
                }
                else
                {
                    Lwm2m_Error("Failed to write LWM2M_SECURITY_OBJECT_SERVER_URI - value is too big\n");
                    result = -1;
                }
                break;

            case LWM2M_SECURITY_OBJECT_BOOTSTRAP_SERVER:
                memcpy(&security->IsBootstrapServer, srcBuffer, sizeof(security->IsBootstrapServer));
                result = sizeof(security->IsBootstrapServer);
                WarnOfInsufficientData(result, srcBufferLen);
                break;

            case LWM2M_SECURITY_OBJECT_SECURITY_MODE:
                memcpy(&security->SecurityMode, srcBuffer, sizeof(security->SecurityMode));
                result = sizeof(security->SecurityMode);
                WarnOfInsufficientData(result, srcBufferLen);
                break;

            case LWM2M_SECURITY_OBJECT_PUBLIC_KEY:
                if (srcBufferLen <= SECURITY_STRING_LEN)
                {
                    security->PublicKeyIdentityLength = srcBufferLen;
                    memcpy(&security->PublicKeyIdentity, srcBuffer, security->PublicKeyIdentityLength);
                    result = security->PublicKeyIdentityLength;
                }
                else
                {
                    Lwm2m_Error("Failed to write LWM2M_SECURITY_OBJECT_PUBLIC_KEY - value is too big\n");
                    result = -1;
                }
                break;

            case LWM2M_SECURITY_OBJECT_SERVER_PUBLIC_KEY:
                if (srcBufferLen <= SECURITY_STRING_LEN)
                {
                    security->ServerPublicKeyLength = srcBufferLen;
                    memcpy(&security->ServerPublicKey, srcBuffer, security->ServerPublicKeyLength);
                    result = security->ServerPublicKeyLength;
                }
                else
                {
                    Lwm2m_Error("Failed to write LWM2M_SECURITY_OBJECT_SERVER_PUBLIC_KEY - value is too big\n");
                    result = -1;
                }
                break;

            case LWM2M_SECURITY_OBJECT_SECRET_KEY:
                if (srcBufferLen <= SECURITY_STRING_LEN)
                {
                    security->SecretKeyLength = srcBufferLen;
                    memcpy(&security->SecretKey, srcBuffer, security->SecretKeyLength);
                    result = security->SecretKeyLength;
                }
                else
                {
                    Lwm2m_Error("Failed to write LWM2M_SECURITY_OBJECT_SECRET_KEY - value is too big\n");
                    result = -1;
                }
                break;

            case LWM2M_SECURITY_OBJECT_SMS_MODE:
                memcpy(&security->SmsSecurityMode, srcBuffer, sizeof(security->SmsSecurityMode));
                result = sizeof(security->SmsSecurityMode);
                WarnOfInsufficientData(result, srcBufferLen);
                break;

            case LWM2M_SECURITY_OBJECT_SMS_BINDING_PARAM:
                if (srcBufferLen <= SECURITY_STRING_LEN)
                {
                    security->SmsBindingKeyParamsLength = srcBufferLen;
                    memcpy(&security->SmsBindingKeyParams, srcBuffer, security->SmsBindingKeyParamsLength);
                    result = security->SmsBindingKeyParamsLength;
                }
                else
                {
                    Lwm2m_Error("Failed to write LWM2M_SECURITY_OBJECT_SMS_BINDING_PARAM - value is too big\n");
                    result = -1;
                }
                break;

            case LWM2M_SECURITY_OBJECT_SMS_BINDING_SECRET_KEY:
                if (srcBufferLen <= SECURITY_STRING_LEN)
                {
                    security->SmsBindingKeySecretLength = srcBufferLen;
                    memcpy(&security->SmsBindingKeySecret, srcBuffer, security->SmsBindingKeySecretLength);
                    result = security->SmsBindingKeySecretLength;
                }
                else
                {
                    Lwm2m_Error("Failed to write LWM2M_SECURITY_OBJECT_SMS_BINDING_SECRET_KEY - value is too big\n");
                    result = -1;
                }
                break;

            case LWM2M_SECURITY_OBJECT_SMS_NUMBER:
                memcpy(&security->ServerSmsNumber, srcBuffer, sizeof(security->ServerSmsNumber));
                result = sizeof(security->ServerSmsNumber);
                WarnOfInsufficientData(result, srcBufferLen);
                break;

            case LWM2M_SECURITY_OBJECT_SHORT_SERVER_ID:
                memcpy(&security->ShortServerID, srcBuffer, sizeof(security->ShortServerID));
                result = sizeof(security->ShortServerID);
                WarnOfInsufficientData(result, srcBufferLen);
                break;

            case LWM2M_SECURITY_OBJECT_CLIENT_HOLD_OFF:
                memcpy(&security->ClientHoldOffTime, srcBuffer, sizeof(security->ClientHoldOffTime));
                result = sizeof(security->ClientHoldOffTime);
                WarnOfInsufficientData(result, srcBufferLen);
                break;

            default:
                Lwm2m_Error("Unhandled write to resource %d\n", resourceID);
                result = -1;
                break;
        }

        if (result > 0)
        {
            *changed = true;
        }
    }
    return result;
}

static LWM2MSecurityInfo * GetSecurityInfoForAddress(Lwm2mContextType * context, AddressType * address)
{
    LWM2MSecurityInfo * info = NULL;
    struct ListHead * current;
    ListForEach(current, Lwm2mCore_GetSecurityObjectList(context))
    {
        LWM2MSecurityInfo * securityInfo = ListEntry(current, LWM2MSecurityInfo, list);
        if (Lwm2mCore_CompareAddresses(address, &securityInfo->address) == 0)
        {
            info = securityInfo;
            break;
        }
    }
    return info;
}

// Check to see if the provided address matches a path in the security objects ServerURI resource
// and is so returns if this entry is marked as a bootstrap server or not.
bool Lwm2mCore_ServerIsBootstrap(Lwm2mContextType * context, AddressType * address)
{
    bool result = false;
    LWM2MSecurityInfo * security = GetSecurityInfoForAddress(context, address);
    if (security == NULL)
    {
        Lwm2m_Warning("No security object matches address: %s\n", Lwm2mCore_DebugPrintAddress(address));
    }
    else
    {
        result = security->IsBootstrapServer;
    }
    return result;
}

int Lwm2mSecurity_GetShortServerID(Lwm2mContextType * context, AddressType * address)
{
    LWM2MSecurityInfo * security = GetSecurityInfoForAddress(context, address);
    return (security != NULL) ? security->ShortServerID : -1;
}

// Read the ServerURI from the security object
int Lwm2m_GetServerURI(Lwm2mContextType * context, int shortServerID, char * buffer, int len)
{
    LWM2MSecurityInfo * security = GetSecurityInfoForShortServerID(context, shortServerID);
    if (security == NULL)
    {
        return -1;
    }
    strncpy(buffer, security->ServerURI, len);
    buffer[len - 1] = '\0';  // Defensive
    return strlen(buffer);
}

bool Lwm2mCore_IsNetworkAddressRevolved(Lwm2mContextType * context, int shortServerID)
{
    bool result = false;
    LWM2MSecurityInfo * security = GetSecurityInfoForShortServerID(context, shortServerID);
    if (security)
    {
        if (!security->AddressResolved)
        {
            security->AddressResolved = coap_ResolveAddressByURI(security->ServerURI, &security->address);
        }
        result = security->AddressResolved;
    }
    return result;
}

int Lwm2m_GetClientHoldOff(Lwm2mContextType * context, int shortServerID, int32_t * clientHoldOff)
{
    int result = -1;
    LWM2MSecurityInfo * security = GetSecurityInfoForShortServerID(context, shortServerID);
    if (security != NULL)
    {
        *clientHoldOff = security->ClientHoldOffTime;
        result = 0;
    }
    return result;
}

static void Lwm2m_PopulateSecurityObjectInstance(Lwm2mContextType * context, int instanceID, const char * serverURI, int bootstrap, LWM2MSecurityMode securityMode, LWM2MSecurityInfo * securityInfo, int serverID, int holdOffTime)
{
    int securityModeAsInt = (int)securityMode;
    Lwm2mCore_CreateObjectInstance(context, LWM2M_SECURITY_OBJECT, instanceID);

    // Create resource instance records and CoAP end points; this will call the write handler above.
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SECURITY_OBJECT, instanceID, 0,  0,  serverURI,                         strlen(serverURI));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SECURITY_OBJECT, instanceID, 1,  0,  &bootstrap,                        sizeof(bootstrap));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SECURITY_OBJECT, instanceID, 2,  0,  &securityModeAsInt,                sizeof(securityModeAsInt));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SECURITY_OBJECT, instanceID, 3,  0,  securityInfo->PublicKeyIdentity,   securityInfo->PublicKeyIdentityLength);
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SECURITY_OBJECT, instanceID, 4,  0,  securityInfo->ServerPublicKey,     securityInfo->ServerPublicKeyLength);
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SECURITY_OBJECT, instanceID, 5,  0,  securityInfo->SecretKey,           securityInfo->SecretKeyLength);
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SECURITY_OBJECT, instanceID, 7,  0,  securityInfo->SmsBindingKeyParams, securityInfo->SmsBindingKeyParamsLength);
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SECURITY_OBJECT, instanceID, 8,  0,  securityInfo->SmsBindingKeySecret, securityInfo->SmsBindingKeySecretLength);
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SECURITY_OBJECT, instanceID, 10, 0,  &serverID,                         sizeof(serverID));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SECURITY_OBJECT, instanceID, 11, 0,  &holdOffTime,                      sizeof(holdOffTime));
}

void Lwm2m_PopulateSecurityObject(Lwm2mContextType * context, const char * bootStrapServer)
{
    // Populate security object with defaults
    LWM2MSecurityInfo securityInfo;
    memset(&securityInfo, 0, sizeof(securityInfo));

    Lwm2m_PopulateSecurityObjectInstance(context, 0, bootStrapServer, SECURITY_BS, SECURITY_SECMODE, &securityInfo, SECURITY_SERVERID, SECURITY_HOLDOFF);
}

void Lwm2m_RegisterSecurityObject(Lwm2mContextType * context)
{
    Lwm2mCore_RegisterObjectType(context, "LWM2MSecurity" , LWM2M_SECURITY_OBJECT, MultipleInstancesEnum_Multiple, MandatoryEnum_Mandatory, &securityObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "LWM2MServerURI",            LWM2M_SECURITY_OBJECT, 0,  AwaResourceType_String,  MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_None, &securityResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "BootstrapServer",           LWM2M_SECURITY_OBJECT, 1,  AwaResourceType_Boolean, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_None, &securityResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "SecurityMode",              LWM2M_SECURITY_OBJECT, 2,  AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_None, &securityResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "PublicKeyorIDentity",       LWM2M_SECURITY_OBJECT, 3,  AwaResourceType_Opaque,  MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_None, &securityResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "ServerPublicKeyorIDentity", LWM2M_SECURITY_OBJECT, 4,  AwaResourceType_Opaque,  MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_None, &securityResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "SecretKey",                 LWM2M_SECURITY_OBJECT, 5,  AwaResourceType_Opaque,  MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_None, &securityResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "SMSSecurityMode",           LWM2M_SECURITY_OBJECT, 6,  AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_None, &securityResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "SMSBindingKeyParameters",   LWM2M_SECURITY_OBJECT, 7,  AwaResourceType_Opaque,  MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_None, &securityResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "SMSBindingSecretKeys",      LWM2M_SECURITY_OBJECT, 8,  AwaResourceType_Opaque,  MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_None, &securityResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "LWM2MServerSMSNumber",      LWM2M_SECURITY_OBJECT, 9,  AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_None, &securityResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "ShortServerID",             LWM2M_SECURITY_OBJECT, 10, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional,  AwaResourceOperations_None, &securityResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "ClientHoldOffTime",         LWM2M_SECURITY_OBJECT, 11, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, AwaResourceOperations_None, &securityResourceOperationHandlers);
}

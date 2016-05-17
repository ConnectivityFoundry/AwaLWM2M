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


#ifndef LWM2M_REGISTRATION_H
#define LWM2M_REGISTRATION_H

#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "lwm2m_core.h"
#include "coap_abstraction.h"
#include "../../api/src/ipc_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LIFETIME_DEFAULT (86400)

/* Transport Bindings
 * Behavior of the LWM2M Server and the LWM2M Client is differentiated by Current Transport Binding
 * and Mode. Current Transport Binding and Mode is decided by “Binding” Resource set by the LWM2M Server
 * and whether SMS and/or Queue Mode are supported by the LWM2M Client
 */
typedef enum
{
    BindingMode_Udp,                     // U
    BindingMode_UdpQueueMode,            // UQ
    BindingMode_Sms,                     // S
    BindingMode_SmsQueueMode,            // SQ
    BindingMode_UdpAndSms,               // US
    BindingMode_UdpWithQueueAndSms,      // UQS

} BindingMode;

typedef enum
{
    RegistrationEventType_Register,
    RegistrationEventType_Update,
    RegistrationEventType_Deregister,
} RegistrationEventType;

typedef struct
{
    struct ListHead list;
    ObjectIDType ObjectID;
    ObjectInstanceIDType InstanceID;

} ObjectListEntry;

// Information about Registered Clients
typedef struct
{
    struct ListHead list;
    char * EndPointName;               // Clients "unique" end point name
    AddressType Address;               // Clients address information
    int LifeTime;                      // Lifetime in seconds, 86400 is the default.
    BindingMode BindingMode;           // Binding mode, currently only "U" is supported.
    uint32_t LastUpdateTime;           // Time the client last sent an update or registration request to the server
    struct ListHead ObjectList;        // List of supported objects, object instances
    char * ResourceType;               // RFC6690 Resource Type parameter
    bool SupportsJson;                 // The Client supports JSON for all objects
    int Location;                      // /rd/location, this should probably be a string

} Lwm2mClientType;

int Lwm2m_RegistrationInit(Lwm2mContextType * context);
void Lwm2m_RegistrationDestroy(Lwm2mContextType * context);

/* Age the client registrations. The registration will be removed by the server if a registration or update
 * has not been received with the client lifetime.
 */
int32_t Lwm2m_AgeRegistrations(Lwm2mContextType * context);

Lwm2mClientType * Lwm2m_LookupClientByName(Lwm2mContextType * context, const char * endPointName);
Lwm2mClientType * Lwm2m_LookupClientByAddress(Lwm2mContextType * context, AddressType * address);

bool Lwm2m_ClientSupportsObject(Lwm2mClientType * client, ObjectIDType objectID, ObjectInstanceIDType instanceID);

// Functions to support Server Events

typedef void (*RegistrationEventCallback)(RegistrationEventType eventType, void * context, void * parameter);

void * Lwm2m_GetEventContext(Lwm2mContextType * lwm2mContext, IPCSessionID sessionID);
int Lwm2m_AddRegistrationEventCallback(Lwm2mContextType * lwm2mContext, IPCSessionID sessionID, RegistrationEventCallback callback, void * callbackContext);
int Lwm2m_DeleteRegistrationEventCallback(Lwm2mContextType * lwm2mContext, IPCSessionID sessionID);

#ifdef __cplusplus
}
#endif

#endif // LWM2M_REGISTRATION_H

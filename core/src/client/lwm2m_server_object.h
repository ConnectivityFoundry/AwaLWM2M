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


#ifndef LWM2M_SERVER_OBJECT_H
#define LWM2M_SERVER_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdint.h>

#include "lwm2m_core.h"
#include "lwm2m_registration.h"

#define LWM2M_SERVER_TYPE_LOCATION_SIZE (128)

typedef struct
{
    struct ListHead list;
    int ServerObjectInstanceID;
    char Location[LWM2M_SERVER_TYPE_LOCATION_SIZE];
    Lwm2mRegistrationState RegistrationState;
    uint32_t LastUpdate;
    int Attempts;
    bool UpdateRegistration;

    int LifeTime;
    int ShortServerID;
    int DefaultMinimumPeriod;
    int DefaultMaximumPeriod;
    int DisableTimeout;
    bool NotificationStoring;
    char Binding[4];  // maximum "UQS" + '\0'

} Lwm2mServerType;


void Lwm2m_RegisterServerObject(Lwm2mContextType * context);

 // Add a server entry to our list of servers or updates an existing one triggers a Registration request.
void Lwm2mCore_UpdateServer(Lwm2mContextType * context, int serverObjectInstanceID, Lwm2mRegistrationState state);

void Lwm2mCore_SetServerUpdateRegistration(Lwm2mContextType * context, int serverObjectInstanceID);

// Set all servers back to the Registration request state
void Lwm2mCore_UpdateAllServers(Lwm2mContextType * context, Lwm2mRegistrationState state);

void Lwm2mCore_DeregisterAllServers(Lwm2mContextType * context);

void Lwm2mCore_DestroyServerList(Lwm2mContextType * context);

int Lwm2mServerObject_GetDefaultMinimumPeriod(Lwm2mContextType * context, int shortServerID);

int Lwm2mServerObject_GetDefaultMaximumPeriod(Lwm2mContextType * context, int shortServerID);

int Lwm2mServerObject_GetTransportBinding(Lwm2mContextType * context, int shortServerID, char * buffer, int len);

int Lwm2mServerObject_GetLifeTime(Lwm2mContextType * context, int shortServerID);


#ifdef __cplusplus
}
#endif

#endif // LWM2M_SERVER_OBJECT_H

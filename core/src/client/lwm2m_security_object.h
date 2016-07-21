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


#ifndef LWM2M_SECURITY_H
#define LWM2M_SECURITY_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdbool.h>
#include <stdint.h>

#include "lwm2m_core.h"


typedef enum
{
    LWM2MSecurityMode_Invalid      = -1,
    LWM2MSecurityMode_PSK          = 0,
    LWM2MSecurityMode_RawPublicKey = 1,
    LWM2MSecurityMode_Certificate  = 2,
    LWM2MSecurityMode_NoSecurity   = 3
} LWM2MSecurityMode;


void Lwm2m_RegisterSecurityObject(Lwm2mContextType * context);
void Lwm2m_PopulateSecurityObject(Lwm2mContextType * context, const char * bootStrapServer);

int Lwm2m_GetServerURI(Lwm2mContextType * context, int shortServerID, char * buffer, int len);
int Lwm2m_GetClientHoldOff(Lwm2mContextType * context, int shortServerID, int32_t * clientHoldOff);

bool Lwm2mCore_IsNetworkAddressRevolved(Lwm2mContextType * context, int shortServerID);

bool Lwm2mCore_ServerIsBootstrap(Lwm2mContextType * context, AddressType * address);
int Lwm2mSecurity_GetShortServerID(Lwm2mContextType * context, AddressType * address);

void Lwm2mSecurity_Create(struct ListHead * securityObjectList);
void Lwm2mSecurity_Destroy(struct ListHead * securityObjectList);


#ifdef __cplusplus
}
#endif

#endif // LWM2M_SECURITY_H

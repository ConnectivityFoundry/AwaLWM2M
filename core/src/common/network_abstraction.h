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

#ifndef NETWORK_ABSTRACTION_H_
#define NETWORK_ABSTRACTION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "lwm2m_types.h"


typedef enum
{
    NetworkSocketError_NoError,
    NetworkSocketError_InvalidSocket,
    NetworkSocketError_InvalidArguments,
    NetworkSocketError_ReadError,
    NetworkSocketError_SendError,
    NetworkSocketError_ConnectionLost
}NetworkSocketError;

typedef enum
{
    NetworkSocketType_NotSet = 0,
    NetworkSocketType_UDP = 1,
    NetworkSocketType_TCP = 2,
    NetworkSocketType_Secure = 4
} NetworkSocketType;


typedef struct _NetworkAddress NetworkAddress;

typedef struct _NetworkSocket NetworkSocket;

NetworkAddress * NetworkAddress_New(const char * uri, int uriLength);

int NetworkAddress_Compare(NetworkAddress * addressX, NetworkAddress * addressY);

void NetworkAddress_SetAddressType(NetworkAddress * address, AddressType * addressType);

void NetworkAddress_Free(NetworkAddress ** address);

bool NetworkAddress_IsSecure(const NetworkAddress * address);

NetworkSocket * NetworkSocket_New(const char * ipAddress, NetworkSocketType socketType, uint16_t port);

NetworkSocketError NetworkSocket_GetError(NetworkSocket * networkSocket);

int NetworkSocket_GetFileDescriptor(NetworkSocket * networkSocket);

void NetworkSocket_SetCertificate(NetworkSocket * networkSocket, const uint8_t * cert, int certLength, AwaCertificateFormat format);

void NetworkSocket_SetPSK(NetworkSocket * networkSocket, const char * identity, const uint8_t * key, int keyLength);

bool NetworkSocket_StartListening(NetworkSocket * networkSocket);

//bool NetworkSocket_Connect(NetworkSocket networkSocket, NetworkAddress * destAddress);

bool NetworkSocket_Read(NetworkSocket * networkSocket, uint8_t * buffer, int bufferLength, NetworkAddress ** sourceAddress, int *readLength);

bool NetworkSocket_Send(NetworkSocket * networkSocket, NetworkAddress * destAddress, uint8_t * buffer, int bufferLength);

void NetworkSocket_Free(NetworkSocket ** networkSocket);

#ifdef __cplusplus
}
#endif

#endif /* NETWORK_ABSTRACTION_H_ */

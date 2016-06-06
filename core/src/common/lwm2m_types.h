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


#ifndef LWM2M_TYPES_H
#define LWM2M_TYPES_H

#include <awa/static.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef POSIX
  #include <sys/socket.h>
  #include <netinet/in.h>
#endif

#ifdef CONTIKI
  #include "net/ip/uip.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define LWM2M_MAX_ID (65535)

typedef enum
{
    MandatoryEnum_Optional = 0,
    MandatoryEnum_Mandatory = 1,
} MandatoryEnum;

typedef enum
{
    MultipleInstancesEnum_Single = 1,
    MultipleInstancesEnum_Multiple = LWM2M_MAX_ID,
} MultipleInstancesEnum;

typedef int ObjectIDType;
typedef int ObjectInstanceIDType;
typedef int ResourceIDType;
typedef int ResourceInstanceIDType;

#ifdef POSIX
typedef struct
{
    socklen_t Size;
    union
    {
        struct sockaddr     Sa;
        struct sockaddr_storage St;
        struct sockaddr_in  Sin;
        struct sockaddr_in6 Sin6;
    } Addr;

} AddressType;
#endif

#ifdef CONTIKI
typedef struct
{
    int Port;
    uip_ipaddr_t Addr;
} AddressType;
#endif

#ifdef MICROCHIP_PIC32
typedef struct
{
    uint16_t Port;      // TODO - FIXME
    uint32_t Addr;
} AddressType;
#endif

typedef enum
{
    ContentType_None = -1,
    ContentType_ApplicationPlainText      = 0,       // The new standard suggests to use PlainText
    ContentType_ApplicationLinkFormat     = 40,      // Object link format
    ContentType_ApplicationOctetStream    = 42,      // The new standard uses OctetStream, rather than omg.lwm2m+opaque
    ContentType_ApplicationJson           = 50,      // The new standard uses Json, rather than omg.lwm2m+json
    ContentType_ApplicationOmaLwm2mText   = 1541,    // application/vnd.oma.lwm2m+text (leshan uses 1541)
    ContentType_ApplicationOmaLwm2mTLV    = 1542,    // application/vnd.oma.lwm2m+tlv (TBD)??
    ContentType_ApplicationOmaLwm2mJson   = 1543,
    ContentType_ApplicationOmaLwm2mOpaque = 1544,

    ContentType_CustomPrettyPrint = 999,
} ContentType;

bool Operations_IsResourceTypeExecutable(AwaResourceOperations operation);
bool Operations_IsResourceTypeWritable(AwaResourceOperations operation);
bool Operations_IsResourceTypeReadable(AwaResourceOperations operation);
bool Operations_Contains(AwaResourceOperations parent, AwaResourceOperations child);

size_t Lwm2mCore_GetNumberOfResourceTypeStrings(void);
const char * Lwm2mCore_ResourceTypeToString(AwaResourceType resourceType);
AwaResourceType Lwm2mCore_ResourceTypeFromString(const char * resourceTypeString);

#ifdef __cplusplus
}
#endif

#endif // LWM2M_TYPES_H

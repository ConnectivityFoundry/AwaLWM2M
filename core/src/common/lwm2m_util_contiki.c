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


#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <inttypes.h>

#include "clock.h"

#include "lwm2m_util.h"
#include "lwm2m_list.h"
#include "lwm2m_debug.h"


uint64_t Lwm2mCore_GetTickCountMs(void)
{
    clock_time_t ticks = clock_time();

    return (ticks * 1000) / CLOCK_CONF_SECOND;
}

void Lwm2mCore_AddressTypeToPath(char * path, size_t pathSize, AddressType * addr)
{
    memcpy(path,"coap",4);
    path += 4;
    pathSize -= 4;

    if (addr->Secure)
    {
        *path = 's';
        path++;
        pathSize--;
    }
    snprintf(path, pathSize, "://[%04X:%04X:%04X:%04X:%04X:%04X:%04X:%04X]:%d", uip_htons(addr->Addr.u16[0]), uip_htons(addr->Addr.u16[1]), uip_htons(addr->Addr.u16[2]), uip_htons(addr->Addr.u16[3]), uip_htons(addr->Addr.u16[4]), uip_htons(addr->Addr.u16[5]), uip_htons(addr->Addr.u16[6]), uip_htons(addr->Addr.u16[7]), addr->Port);
}

const char * Lwm2mCore_DebugPrintAddress(AddressType * addr)
{
    static char ipv6addr[50] = {0};
    sprintf(ipv6addr, "[%04X:%04X:%04X:%04X:%04X:%04X:%04X:%04X]:%d", uip_htons(addr->Addr.u16[0]), uip_htons(addr->Addr.u16[1]), uip_htons(addr->Addr.u16[2]), uip_htons(addr->Addr.u16[3]), uip_htons(addr->Addr.u16[4]), uip_htons(addr->Addr.u16[5]), uip_htons(addr->Addr.u16[6]), uip_htons(addr->Addr.u16[7]), addr->Port);

    return ipv6addr;
}

bool Lwm2mCore_ResolveAddressByName(unsigned char * address, int addressLength, AddressType * addr)
{
    int result = false;
    /* DNS look up not supported */
    return result;
}

int Lwm2mCore_CompareAddresses(AddressType * addr1, AddressType * addr2)
{
    return memcmp(&addr1->Addr.u16, &addr2->Addr.u16, sizeof(addr2->Addr.u16));
}

int Lwm2mCore_ComparePorts(AddressType * addr1, AddressType * addr2)
{
    if(addr1->Port != addr2->Port)
    {
        return -1;
    }
    return 0;
}

int Lwm2mCore_GetIPAddressFromInterface(const char * interface, int addressFamily, char * destAddress, size_t destAddressLength)
{
    /* Note: only used by servers */
    int returnCode = 0;
    return returnCode;
}


/************************************************************************************************************************
 Copyright (c) 2017, Imagination Technologies Limited and/or its affiliated group companies.
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

#include "xtimer.h"
#include "lwm2m_util.h"
#include "net/sock/dns.h"
#include "net/sock/util.h"

#define POSIX_DNS_SERVER "53"

sock_udp_ep_t sock_dns_server;

// Get the system tick count in milliseconds
uint64_t Lwm2mCore_GetTickCountMs(void)
{
    return xtimer_now_usec64() / 1000;
}

bool Lwm2mCore_ResolveAddressByName(unsigned char * address, int addressLength, AddressType * addr)
{
    (void)addressLength;
    int res;
    uint8_t buffer[16];

    if (address == NULL || addr == NULL)
        return false;

    sock_udp_str2ep(&sock_dns_server, POSIX_DNS_SERVER);
    res = sock_dns_query((char*)address, buffer, AF_UNSPEC);
    if (res <= 0)
        return false;

    if (res == 4)
    {
        addr->Addr.Sa.sa_family = AF_INET;
        addr->Size = 4;
        memcpy(&addr->Addr.Sin, buffer, addr->Size);
    }
    else if (res == 6)
    {
        addr->Addr.Sa.sa_family = AF_INET6;
        addr->Size = 16;
        memcpy(&addr->Addr.Sin6, buffer, addr->Size);
    }
    else
        return false;

    return true;
}

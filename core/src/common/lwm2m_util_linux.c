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

#include <netdb.h>
#include <sys/time.h>
#include "lwm2m_util.h"
#include "lwm2m_debug.h"

uint64_t Lwm2mCore_GetTickCountMs(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return (tv.tv_sec * (uint64_t)1000 + (tv.tv_usec / 1000));
}

bool Lwm2mCore_ResolveAddressByName(unsigned char * address, int addressLength, AddressType * addr)
{
    bool result = false;
    struct addrinfo *res, *ainfo;
    struct addrinfo hints;
    static char addrstr[256];
    int error;

    memset(addrstr, 0, sizeof(addrstr));
    if (addressLength > 0)
    {
        memcpy(addrstr, address, addressLength);
    }
    else
    {
        memcpy(addrstr, "localhost", 9);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_UNSPEC;

    error = getaddrinfo(addrstr, NULL, &hints, &res);

    if (error != 0)
    {
        Lwm2m_Error("getaddrinfo: %s\n", gai_strerror(error));
    }
    else
    {
        for (ainfo = res; ainfo != NULL; ainfo = ainfo->ai_next)
        {
            if  (ainfo->ai_family == AF_INET6)
            {
                addr->Addr.Sa.sa_family = ainfo->ai_family;
                addr->Size = ainfo->ai_addrlen;
                memcpy(&addr->Addr.Sin6, ainfo->ai_addr, addr->Size);
                result = true;
                break;
            }
            else if  (ainfo->ai_family == AF_INET)
            {
                addr->Addr.Sa.sa_family = ainfo->ai_family;
                addr->Size = ainfo->ai_addrlen;
                memcpy(&addr->Addr.Sin, ainfo->ai_addr, addr->Size);
                result = true;
                break;
            }
        }
        freeaddrinfo(res);
    }
    return result;
}

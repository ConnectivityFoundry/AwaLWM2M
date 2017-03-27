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

#include <arpa/inet.h>
#include "xtimer.h"
#include "lwm2m_debug.h"
#include "lwm2m_util.h"

// Get the system tick count in milliseconds
uint64_t Lwm2mCore_GetTickCountMs(void)
{
    return xtimer_now_usec64() / 1000;
}


void Lwm2mCore_AddressTypeToPath(char * path, size_t pathSize, AddressType * addr)
{
    char buffer[255];
    const char* ip;
    int port;


    memcpy(path, "coap", 4);
    path += 4;
    pathSize -= 4;

    if (addr->Secure)
    {
        *path = 's';
        path++;
        pathSize--;
    }

    switch (addr->Addr.Sa.sa_family)
    {
        case AF_INET:
            ip = inet_ntop(AF_INET, &addr->Addr.Sin.sin_addr, buffer, sizeof(buffer));
            port = ntohs(addr->Addr.Sin.sin_port);
            snprintf(path, pathSize, "://%s:%d", ip, port);
            break;
        case AF_INET6:
            ip = inet_ntop(AF_INET6, &addr->Addr.Sin6.sin6_addr, buffer, sizeof(buffer));
            port =  ntohs(addr->Addr.Sin6.sin6_port);
            snprintf(path, pathSize, "://[%s]:%d", ip, port);
            break;
        default:
            Lwm2m_Error("Unsupported address family: %d\n", addr->Addr.Sa.sa_family);
            break;
    }
}

const char * Lwm2mCore_DebugPrintSockAddr(const struct sockaddr * sa)
{
    static char out[255];
    char buffer[64];
    const char* ip;
    int port;

    switch (sa->sa_family)
    {
        case AF_INET:
            ip = inet_ntop(AF_INET, &((struct sockaddr_in *)sa)->sin_addr,
                           buffer, sizeof(buffer));
            port = ntohs(((struct sockaddr_in *)sa)->sin_port);
            sprintf(out, "%s:%d", ip, port);
            break;
        case AF_INET6:
            ip = inet_ntop(AF_INET6, &((struct sockaddr_in6 *)sa)->sin6_addr,
                           buffer, sizeof(buffer));
            port =  ntohs(((struct sockaddr_in6 *)sa)->sin6_port);
            sprintf(out, "[%s]:%d", ip, port);
            break;
        default:
            Lwm2m_Error("Unsupported address family: %d\n", sa->sa_family);
            break;
    }
    return out;
}

const char * Lwm2mCore_DebugPrintAddress(AddressType * addr)
{
    return Lwm2mCore_DebugPrintSockAddr(&addr->Addr.Sa);
}

bool Lwm2mCore_ResolveAddressByName(unsigned char * address, int addressLength, AddressType * addr)
{
    (void)address;
    (void)addressLength;
    (void)addr;
    return false;
}
static int comparePorts(in_port_t x, in_port_t y)
{
    int result;
    if (x == y)
        result = 0;
    else if  (x > y)
        result = 1;
    else
        result = -1;
    return result;
}
int Lwm2mCore_CompareAddresses(AddressType * addr1, AddressType * addr2)
{
    int result = -1;
    if (addr1->Addr.Sa.sa_family == addr2->Addr.Sa.sa_family)
    {
        switch (addr1->Addr.Sa.sa_family)
        {
            case AF_INET:
                result = memcmp(&addr1->Addr.Sin.sin_addr.s_addr, &addr2->Addr.Sin.sin_addr, sizeof(addr2->Addr.Sin.sin_addr));
                if (result == 0)
                {
                    result = comparePorts(addr1->Addr.Sin.sin_port, addr2->Addr.Sin.sin_port);
                }
                break;
            case AF_INET6:
                result = memcmp(&addr1->Addr.Sin6.sin6_addr, &addr2->Addr.Sin6.sin6_addr, sizeof(addr2->Addr.Sin6.sin6_addr));
                if (result == 0)
                {
                    result = comparePorts(addr1->Addr.Sin6.sin6_port, addr2->Addr.Sin6.sin6_port);
                }
                break;
            default:
                Lwm2m_Error("Unsupported address family: %d\n", addr1->Addr.Sa.sa_family);
                break;
        }
    }

    return result;
}

int Lwm2mCore_ComparePorts(AddressType * addr1, AddressType * addr2)
{
    if(addr1->Addr.Sin6.sin6_port != addr2->Addr.Sin6.sin6_port)
    {
        return -1;
    }
    return 0;
}

int Lwm2mCore_GetIPAddressFromInterface(const char * interface, int addressFamily, char * destAddress, size_t destAddressLength)
{
    // See sc_netif.c
    (void)interface;
    (void)addressFamily;
    (void)destAddress;
    (void)destAddressLength;
    return -1;
}
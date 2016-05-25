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
#ifndef CONTIKI
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include <netdb.h>
 #include <sys/stat.h>
 #include <dirent.h>
 #include <signal.h>
 #include <sys/socket.h>
 #include <netdb.h>
 #include <ifaddrs.h>
 #include <unistd.h>
#endif
#include <errno.h>
#include <inttypes.h>
#ifndef CONTIKI
 #include <arpa/inet.h>
#else
 #include "clock.h"
#endif

#include "lwm2m_util.h"
#include "lwm2m_list.h"
#include "lwm2m_debug.h"

ObjectInstanceResourceKey UriToOir(const char * uri)
{
    ObjectInstanceResourceKey key = {-1,-1,-1};

    if (sscanf(uri, "%5d/%5d/%5d", &key.ObjectID, &key.InstanceID, &key.ResourceID) == 0)
    {
        sscanf(uri, "/%5d/%5d/%5d", &key.ObjectID, &key.InstanceID, &key.ResourceID);
    }

    return key;
}

const char * OirToUri(ObjectInstanceResourceKey key)
{
    static char buffer[64];

    memset(buffer, 0,  sizeof(buffer));

    if ((key.ResourceID != -1) && (key.InstanceID != -1))
    {
        sprintf(buffer, "%d/%d/%d", key.ObjectID, key.InstanceID, key.ResourceID);
    }
    else if (key.InstanceID != -1)
    {
        sprintf(buffer, "%d/%d", key.ObjectID, key.InstanceID);
    }
    else
    {
        sprintf(buffer, "%d", key.ObjectID);
    }

    return &buffer[0];
}

uint64_t Lwm2mCore_GetTickCountMs(void)
{
#ifdef CONTIKI
    clock_time_t ticks = clock_time();

    return (ticks * 1000) / CLOCK_CONF_SECOND;
#else
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return (tv.tv_sec * (uint64_t)1000 + (tv.tv_usec / 1000));
#endif
}

int8_t ptrToInt8(void * ptr)
{
    int8_t temp = 0;
    memcpy(&temp,ptr,sizeof(int8_t));
    return temp;
}

int16_t ptrToInt16(void * ptr)
{
    int16_t temp = 0;
    memcpy(&temp,ptr,sizeof(int16_t));
    return temp;
}

int32_t ptrToInt32(void * ptr)
{
    int32_t temp = 0;
    memcpy(&temp,ptr,sizeof(int32_t));
    return temp;
}

int64_t ptrToInt64(void * ptr)
{
    int64_t temp = 0;
    memcpy(&temp,ptr,sizeof(int64_t));
    return temp;
}

void Lwm2mCore_AddressTypeToPath(char * path, size_t pathSize, AddressType * addr)
{
#ifndef CONTIKI
    char buffer[255];
    const char* ip;
    int port;

    switch (addr->Addr.Sa.sa_family)
    {
        case AF_INET:
            ip = inet_ntop(AF_INET, &addr->Addr.Sin.sin_addr, buffer, sizeof(buffer));
            port = ntohs(addr->Addr.Sin.sin_port);
            snprintf(path, pathSize, "coap://%s:%d", ip, port);
            break;
        case AF_INET6:
            ip = inet_ntop(AF_INET6, &addr->Addr.Sin6.sin6_addr, buffer, sizeof(buffer));
            port =  ntohs(addr->Addr.Sin6.sin6_port);
            snprintf(path, pathSize, "coap://[%s]:%d", ip, port);
            break;
        default:
            Lwm2m_Error("Unsupported address family: %d\n", addr->Addr.Sa.sa_family);
            break;
    }
#else
    snprintf(path, pathSize, "coap://[%04X:%04X:%04X:%04X:%04X:%04X:%04X:%04X]:%d", uip_htons(addr->Addr.u16[0]), uip_htons(addr->Addr.u16[1]), uip_htons(addr->Addr.u16[2]), uip_htons(addr->Addr.u16[3]), uip_htons(addr->Addr.u16[4]), uip_htons(addr->Addr.u16[5]), uip_htons(addr->Addr.u16[6]), uip_htons(addr->Addr.u16[7]), addr->Port);
#endif
}

#ifndef CONTIKI
const char * Lwm2mCore_DebugPrintSockAddr(const struct sockaddr * sa)
{
    static char out[255];
    char buffer[64];
    const char* ip;
    int port;

    switch (sa->sa_family)
    {
        case AF_INET:
            ip = inet_ntop(AF_INET, &((struct sockaddr_in *)sa)->sin_addr, buffer, sizeof(buffer));
            port = ntohs(((struct sockaddr_in *)sa)->sin_port);
            sprintf(out, "%s:%d", ip, port);
            break;
        case AF_INET6:
            ip = inet_ntop(AF_INET6, &((struct sockaddr_in6 *)sa)->sin6_addr, buffer, sizeof(buffer));
            port =  ntohs(((struct sockaddr_in6 *)sa)->sin6_port);
            sprintf(out, "[%s]:%d", ip, port);
            break;
        default:
            Lwm2m_Error("Unsupported address family: %d\n", sa->sa_family);
            break;
    }
    return out;
}
#endif

const char * Lwm2mCore_DebugPrintAddress(AddressType * addr)
{
#ifndef CONTIKI
    return Lwm2mCore_DebugPrintSockAddr(&addr->Addr.Sa);
#else
    static char ipv6addr[50] = {0};
    sprintf(ipv6addr, "[%04X:%04X:%04X:%04X:%04X:%04X:%04X:%04X]:%d", uip_htons(addr->Addr.u16[0]), uip_htons(addr->Addr.u16[1]), uip_htons(addr->Addr.u16[2]), uip_htons(addr->Addr.u16[3]), uip_htons(addr->Addr.u16[4]), uip_htons(addr->Addr.u16[5]), uip_htons(addr->Addr.u16[6]), uip_htons(addr->Addr.u16[7]), addr->Port);

    return ipv6addr;
#endif
}

int Lwm2mCore_ResolveAddressByName(unsigned char * address, int addressLength, AddressType * addr)
{
    int len =-1;
#ifndef CONTIKI
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

    memset ((char *)&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_UNSPEC;

    error = getaddrinfo(addrstr, NULL, &hints, &res);

    if (error != 0)
    {
        Lwm2m_Error("getaddrinfo: %s\n", gai_strerror(error));
        return error;
    }

    for (ainfo = res; ainfo != NULL; ainfo = ainfo->ai_next)
    {
        switch (ainfo->ai_family)
        {
            case AF_INET6:
                addr->Addr.Sa.sa_family = ainfo->ai_family;
                len = ainfo->ai_addrlen;
                memcpy(&addr->Addr.Sin6, ainfo->ai_addr, len);
                addr->Size = len;
                goto finish;
            case AF_INET:
                addr->Addr.Sa.sa_family = ainfo->ai_family;
                len = ainfo->ai_addrlen;
                memcpy(&addr->Addr.Sin, ainfo->ai_addr, len);
                addr->Size = len;
                goto finish;
            default:
                break;
        }
    }

finish:
    freeaddrinfo(res);
#endif
    return len;
}

int Lwm2mCore_CompareAddresses(AddressType * addr1, AddressType * addr2)
{
#ifndef CONTIKI
    if (addr1->Addr.Sa.sa_family != addr2->Addr.Sa.sa_family)
    {
        return -1;
    }

    switch (addr1->Addr.Sa.sa_family)
    {
        case AF_INET:
            return memcmp(&addr1->Addr.Sin.sin_addr.s_addr, &addr2->Addr.Sin.sin_addr, sizeof(addr2->Addr.Sin.sin_addr));
        case AF_INET6:
            return memcmp(&addr1->Addr.Sin6.sin6_addr, &addr2->Addr.Sin6.sin6_addr, sizeof(addr2->Addr.Sin6.sin6_addr));
        default:
            Lwm2m_Error("Unsupported address family: %d\n", addr1->Addr.Sa.sa_family);
            break;
    }

    return -1;
#else
    return memcmp(&addr1->Addr.u16, &addr2->Addr.u16, sizeof(addr2->Addr.u16));
#endif
}

int Lwm2mCore_ComparePorts(AddressType * addr1, AddressType * addr2)
{
#ifndef CONTIKI
    if(addr1->Addr.Sin6.sin6_port != addr2->Addr.Sin6.sin6_port)
    {
        return -1;
    }
#else
    if(addr1->Port != addr2->Port)
    {
        return -1;
    }
#endif

    return 0;
}

int Lwm2mCore_GetIPAddressFromInterface(const char * interface, int addressFamily, char * destAddress, size_t destAddressLength)
{
    int returnCode = 0;
#ifndef CONTIKI
    if (addressFamily != AF_INET && addressFamily != AF_INET6)
    {
        Lwm2m_Error("Unsupported address family: %d. Only AF_INET and AF_INET6 are supported.\n", addressFamily);
        returnCode = 1;
        goto error;
    }
    struct ifaddrs *interfaceAddresses, *interfaceAddress;

    char host[NI_MAXHOST];

    if (getifaddrs(&interfaceAddresses) == -1)
    {
        perror("getifaddrs");
        returnCode = 1;
        goto error;
    }

    char linkLocalIpv6Address[NI_MAXHOST] = { 0 };
    char globalIpv6Address[NI_MAXHOST] = { 0 };
    bool found = false;
    int index = 0;
    for (interfaceAddress = interfaceAddresses; interfaceAddress != NULL; interfaceAddress = interfaceAddress->ifa_next)
    {
        if (interfaceAddress->ifa_addr == NULL)
        {
            continue;
        }

        if ((strcmp(interfaceAddress->ifa_name, interface)==0)&&(interfaceAddress->ifa_addr->sa_family==addressFamily))
        {
            int socketAddressLength = 0;
            switch(addressFamily)
            {
                case AF_INET:
                    socketAddressLength = sizeof(struct sockaddr_in);
                    break;
                default:
                    socketAddressLength = sizeof(struct sockaddr_in6);
                    break;
            }

            returnCode = getnameinfo(interfaceAddress->ifa_addr, socketAddressLength, host, sizeof(host), NULL, 0, NI_NUMERICHOST);

            if (returnCode != 0)
            {
                Lwm2m_Error("getnameinfo() failed: %s\n", gai_strerror(returnCode));
                goto error_free;
            }

            size_t addressLength = strlen(host);
            if (destAddressLength < addressLength)
            {
                Lwm2m_Error("Error: Address is longer than %zu characters\n", destAddressLength);
                goto error_free;
            }

            switch(addressFamily)
            {
                case AF_INET:
                    strcpy(destAddress, host);
                    found = true;
                    break;
                default:
                    if (strncmp(host, "fe80", 4) == 0)
                    {
                        Lwm2m_Debug("Address %d: %s (local)\n", index, host);
                        strcpy(linkLocalIpv6Address, host);
                    }
                    else
                    {
                        Lwm2m_Debug("Address %d: %s (global)\n", index, host);
                        strcpy(globalIpv6Address, host);
                    }
                    break;
            }
            index++;
        }
    }

    if (addressFamily == AF_INET6)
    {
        if (strlen(globalIpv6Address) > 0)
        {
            Lwm2m_Debug("Global IPv6 address found for interface %s: %s\n", interface, globalIpv6Address);
            strcpy(destAddress, globalIpv6Address);
            found = true;
        }
        else if (strlen(linkLocalIpv6Address) > 0)
        {
            Lwm2m_Warning("No global IPv6 address found for interface %s: using local: %s\n", interface, linkLocalIpv6Address);
            strcpy(destAddress, linkLocalIpv6Address);
            found = true;
        }
    }

    if (!found)
    {
        Lwm2m_Error("Could not find an %s IP address for interface %s\n", addressFamily == AF_INET? "IPv4" : "IPv6", interface);
        returnCode = 1;
    }

error_free:
    freeifaddrs(interfaceAddresses);
error:
#endif
    return returnCode;
}

static int Lwm2mCore_CountQueryPairs(const char * query)
{
    int count = 0;
    if (query != NULL)
    {
        char * str = strdup(query);

        const char delim[] = "&?";

        char * token = strtok(str, delim);

        while (token != NULL)
        {
            count++;
            token = strtok(NULL, delim);
        }
        free(str);
    }
    return count;
}

QueryPair * Lwm2mCore_SplitQuery(const char * query, int * numPairs)
{
    *numPairs = Lwm2mCore_CountQueryPairs(query);
    QueryPair * pairs = NULL;
    if (*numPairs > 0)
    {
        pairs = malloc(*numPairs * sizeof(QueryPair));
        if (pairs != NULL)
        {
            int count = 0;
            char * str = strdup(query);

            const char delim[] = "&?";

            char * token = strtok(str, delim);

            while (token != NULL)
            {
                QueryPair * pair = &pairs[count];

                char * equals = strchr(token, '=');
                if (equals == NULL)
                {
                    pair->Key = strdup(token);
                    pair->Value = NULL;
                }
                else
                {
                    int equalsPos = (int)(equals - token);

                    pair->Key = (char*) malloc((equalsPos+1) * sizeof(char));
                    memcpy(pair->Key, token, equalsPos);
                    pair->Key[equalsPos] = '\0';
                    pair->Value = strdup(equals + 1);
                    if (strlen(pair->Value) == 0)
                        pair->Value = NULL;
                }
                token = strtok(NULL, delim);
                count++;
            }
            free(str);
        }
        else
        {
            Lwm2m_Error("Failed to allocate memory for Lwm2mCore_SplitUpQuery");
        }
    }
    return pairs;
}

void Lwm2mCore_FreeQueryPairs(QueryPair * pairs, int numPairs)
{
    int i;
    if (pairs == NULL)
    {
        return;
    }
    for (i = 0; i < numPairs; i++)
    {
        QueryPair * pair = &pairs[i];
        free(pair->Key);
        free(pair->Value);
    }
    free(pairs);
}


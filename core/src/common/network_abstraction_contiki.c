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

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "contiki.h"
#include "contiki-net.h"
#include "resolv.h"

#ifndef CONTIKI
#define CONTIKI
#endif

#include "lwm2m_debug.h"
#include "lwm2m_util.h"
#include "network_abstraction.h"
#include "dtls_abstraction.h"


struct _NetworkAddress
{
    uip_ipaddr_t Address;
    uint16_t Port;
    bool Secure;
    int useCount;
};

struct _NetworkSocket
{
    struct uip_udp_conn * Socket;
    NetworkSocketType SocketType;
    uint16_t Port;
    NetworkSocketError LastError;
};

#define MAX_URI_LENGTH  (64)

typedef struct
{
    bool InUse;
    char Uri[MAX_URI_LENGTH];
    NetworkAddress Address;
} NetworkAddressCache;

typedef enum
{
    UriParseState_Scheme,
    UriParseState_Hostname,
    UriParseState_Port,
} UriParseState;



#ifndef MAX_NETWORK_ADDRESS_CACHE
    #define MAX_NETWORK_ADDRESS_CACHE  (2)
#endif

static NetworkSocket networkSocket;

static NetworkAddressCache networkAddressCache[MAX_NETWORK_ADDRESS_CACHE] = {{0}};

static NetworkAddress * addCachedAddress(const char * uri, int uriLength);
static NetworkAddress * getCachedAddress(NetworkAddress * matchAddress, const char * uri, int uriLength);
static NetworkAddress * getCachedAddressByUri(const char * uri, int uriLength);
static int getUriHostLength(const char * uri, int uriLength);

#ifndef ENCRYPT_BUFFER_LENGTH
#define ENCRYPT_BUFFER_LENGTH 1024
#endif

uint8_t encryptBuffer[ENCRYPT_BUFFER_LENGTH];

static NetworkTransmissionError SendDTLS(NetworkAddress * destAddress, const uint8_t * buffer, int bufferLength, void *context);


#define UIP_IP_BUF   ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])


uip_ipaddr_t * getHostByName(const char *hostName)
{
    uip_ipaddr_t * result = NULL;
    static uip_ipaddr_t ipaddr;
    if (uiplib_ipaddrconv(hostName, &ipaddr) == 0)
    {
        uip_ipaddr_t *resolved_addr = NULL;
        resolv_status_t status = resolv_lookup(hostName, &resolved_addr);
        if (status == RESOLV_STATUS_UNCACHED || status == RESOLV_STATUS_EXPIRED)
        {
            Lwm2m_Debug("Attempting to look up %s\n", hostName);
            resolv_query(hostName);
            status = RESOLV_STATUS_RESOLVING;
        }
        else if (status == RESOLV_STATUS_CACHED && resolved_addr != NULL )
        {
            Lwm2m_Debug("Lookup of \"%s\" succeded!\n", hostName);
        }
        else if (status == RESOLV_STATUS_RESOLVING)
        {
            resolved_addr = NULL;
            Lwm2m_Debug("Still looking up \"%s\"...\n", hostName);
        }
        else
        {
            resolved_addr = NULL;
            Lwm2m_Debug("Lookup of \"%s\" failed. status = %d\n", hostName, status);
        }
        if (resolved_addr)
        {
            uip_ipaddr_copy(&ipaddr, resolved_addr);
            result = &ipaddr;
        }
    }
    else
    {
        result = &ipaddr;
        Lwm2m_Debug("Cache hit on look up %s\n", hostName);
    }

    return result;
}


NetworkAddress * NetworkAddress_New(const char * uri, int uriLength)
{
    NetworkAddress * result = NULL;
    if (uri && uriLength > 0)
    {
        int uriHostLength = getUriHostLength(uri, uriLength);
        if (uriHostLength > 0)
            result = getCachedAddressByUri(uri, uriHostLength);
        if (!result)
        {
            bool secure = false;
            int index = 0;
            int startIndex = 0;
            int port = 5683;
            char hostname[MAX_URI_LENGTH];
            int hostnameLength = 0;
            UriParseState state = UriParseState_Scheme;
            while (index < uriLength)
            {
                if (state == UriParseState_Scheme)
                {
                    if ((uri[index] == ':') && ((index + 2) <  uriLength) && (uri[index+1] == '/') &&  (uri[index+2] == '/'))
                    {
                        int length = index - startIndex;
                        if ((length == 4) && (strncmp(&uri[startIndex],"coap", length) == 0))
                        {

                        }
                        else if ((length == 5) && (strncmp(&uri[startIndex],"coaps", length) == 0))
                        {
                            port = 5684;
                            secure = true;
                        }
                        else
                        {
                            break;
                        }
                        state = UriParseState_Hostname;
                        index += 2;
                        startIndex = index + 1;
                    }
                    index++;
                }
                else if (state == UriParseState_Hostname)
                {
                    if ((uri[index] == '[') )
                    {
                        index++;
                        while (index < uriLength)
                        {
                            if (uri[index] == ']')
                            {
                                break;
                            }
                            index++;
                        }
                    }
                    else if ((uri[index] == ':') || (uri[index] == '/') )
                    {
                        hostnameLength = index - startIndex;
                        memcpy(&hostname, &uri[startIndex], hostnameLength);
                        hostname[hostnameLength] = 0;
                        if  (uri[index] == '/')
                            break;
                        state = UriParseState_Port;
                        port = 0;
                        startIndex = index + 1;
                    }
                    index++;
                }
                else if (state == UriParseState_Port)
                {
                    if (uri[index] == '/')
                    {
                        break;
                    }
                    else if (isdigit(uri[index]))
                    {
                        port = (port * 10) + (uri[index] - '0');
                    }
                    index++;
                }
            }
            if (state == UriParseState_Hostname)
            {
                hostnameLength = uriLength - startIndex;
                memcpy(hostname, &uri[startIndex], hostnameLength);
                hostname[hostnameLength] = 0;
            }
            if (hostnameLength > 0 && port > 0)
            {
                uip_ipaddr_t * resolvedAddress = getHostByName(hostname);
                if (resolvedAddress)
                {
                    NetworkAddress * networkAddress = addCachedAddress(uri, uriHostLength);
                    if (networkAddress)
                    {
                        memcpy(&networkAddress->Address, resolvedAddress, sizeof(uip_ipaddr_t));
                        networkAddress->Secure = secure;
                        networkAddress->useCount = 1;
                        networkAddress->Port = UIP_HTONS(port);
                        result = networkAddress;
                    }
                }
            }
        }
    }
    return result;
}

int NetworkAddress_Compare(NetworkAddress * address1, NetworkAddress * address2)
{
    int result = -1;
    // Compare address and port (ignore uri)
    if (address1 && address2)
    {
        result = memcmp(&address1->Address, &address2->Address, sizeof(uip_ipaddr_t));
        if (result == 0)
        {
            result = address1->Port - address2->Port;
        }
    }
    return result;
}

void NetworkAddress_SetAddressType(NetworkAddress * address, AddressType * addressType)
{
    if (address && addressType)
    {
        memcpy(&addressType->Addr, &address->Address, sizeof(addressType->Addr));
        addressType->Port = address->Port;
        addressType->Secure = address->Secure;
    }
}

void NetworkAddress_Free(NetworkAddress ** address)
{
    // TODO - review when addresses are freed (e.g. after client bootstrap, or connection lost ?)
    if (address && *address)
    {
        (*address)->useCount--;
        if ((*address)->useCount == 0)
        {
            int index;
            for (index = 0; index < MAX_NETWORK_ADDRESS_CACHE; index++)
            {
                if (NetworkAddress_Compare(&networkAddressCache[index].Address, *address) == 0)
                {
                    if (networkAddressCache[index].Uri)
                    {
                        Lwm2m_Debug("Address free: %s\n", networkAddressCache[index].Uri);
                        networkAddressCache[index].Uri[0] = '\0';
                    }
                    else
                    {
                        Lwm2m_Debug("Address free\n");
                    }
                    networkAddressCache[index].InUse = false;
                    break;
                }
            }
        }
        *address = NULL;
    }
}

bool NetworkAddress_IsSecure(const NetworkAddress * address)
{
    bool result = false;
    if (address)
    {
        result = address->Secure;
    }
    return result;
}

static NetworkAddress * addCachedAddress(const char * uri, int uriLength)
{
    NetworkAddress * result = NULL;
    int index;
    for (index = 0; index < MAX_NETWORK_ADDRESS_CACHE; index++)
    {
        if (!networkAddressCache[index].InUse)
        {
            if (uri && uriLength > 0)
            {
                memcpy(networkAddressCache[index].Uri, uri, uriLength);
                networkAddressCache[index].Uri[uriLength] = 0;
                Lwm2m_Debug("Address add: %s\n", networkAddressCache[index].Uri);
            }
            networkAddressCache[index].InUse = true;
            result = &networkAddressCache[index].Address;
            break;
        }
    }
    return result;
}



static NetworkAddress * getCachedAddressByUri(const char * uri, int uriLength)
{
    NetworkAddress * result = NULL;
    int index;
    for (index = 0; index < MAX_NETWORK_ADDRESS_CACHE; index++)
    {
        if (networkAddressCache[index].Uri && (memcmp(networkAddressCache[index].Uri, uri, uriLength) == 0))
        {
            //Lwm2m_Debug("Address uri matched: %s\n", networkAddressCache[index].uri);
            result = &networkAddressCache[index].Address;
            break;
        }
    }
    return result;
}

static NetworkAddress * getCachedAddress(NetworkAddress * matchAddress, const char * uri, int uriLength)
{
    NetworkAddress * result = NULL;
    int index;
    for (index = 0; index < MAX_NETWORK_ADDRESS_CACHE; index++)
    {
        NetworkAddress * address = &networkAddressCache[index].Address;
        if (address)
        {
            if (NetworkAddress_Compare(matchAddress, address) == 0)
            {
                if (uri && uriLength > 0 && networkAddressCache[index].Uri == NULL)
                {
                    // Add info to cached address
                    address->Secure = matchAddress->Secure;
                    memcpy(networkAddressCache[index].Uri, uri, uriLength);
                    networkAddressCache[index].Uri[uriLength] = 0;
                    Lwm2m_Debug("Address add uri: %s\n", networkAddressCache[index].Uri);
                }
                result = address;
                break;
            }
        }
    }
    return result;
}

static int getUriHostLength(const char * uri, int uriLength)
{
    // Search for end of host + optional port
    int result = uriLength;
    char * pathStart = memchr(uri, '/', uriLength);
    if (pathStart && pathStart[1] == '/' )
    {
        pathStart += 2;
        int lengthRemaining = uriLength - (pathStart - uri);
        if (lengthRemaining > 0)
        {
            pathStart = memchr(pathStart, '/', lengthRemaining);
            if (pathStart)
            {
                result = pathStart - uri;
            }
        }
    }
    return result;
}


NetworkSocket * NetworkSocket_New(const char * ipAddress, NetworkSocketType socketType, uint16_t port)
{
    size_t size = sizeof(struct _NetworkSocket);
    NetworkSocket * result = &networkSocket;
    memset(result, 0, size);
    result->SocketType = socketType;
    result->Port = UIP_HTONS(port);
    DTLS_SetNetworkSendCallback(SendDTLS);
    return result;
}

NetworkSocketError NetworkSocket_GetError(NetworkSocket * networkSocket)
{
    NetworkSocketError result = NetworkSocketError_InvalidSocket;
    if (networkSocket)
    {
        result = networkSocket->LastError;
    }
    return result;
}

int NetworkSocket_GetFileDescriptor(NetworkSocket * networkSocket)
{
    int result = -1;
//    if (networkSocket)
//    {
//        result = (int)networkSocket->Socket;
//    }
    return result;
}

void NetworkSocket_SetCertificate(NetworkSocket * networkSocket, const uint8_t * cert, int certLength, AwaCertificateFormat format)
{
    DTLS_SetCertificate(cert, certLength, format);
}

void NetworkSocket_SetPSK(NetworkSocket * networkSocket, const char * identity, const uint8_t * key, int keyLength)
{

    DTLS_SetPSK(identity, key, keyLength);
}


bool NetworkSocket_StartListening(NetworkSocket * networkSocket)
{
    bool result = false;
    if (networkSocket)
    {

        if ((networkSocket->SocketType & NetworkSocketType_UDP) == NetworkSocketType_UDP)
        {
            networkSocket->Socket = udp_new(NULL, 0, NULL);
            if (networkSocket->Socket)
            {
                udp_bind(networkSocket->Socket, networkSocket->Port);
                result = true;
            }
        }
    }
    return result;
}

bool readUDP(NetworkSocket * networkSocket, uint8_t * buffer, int bufferLength, NetworkAddress ** sourceAddress, int *readLength)
{
    bool result = true;
    *readLength = 0;
    //if (uip_newdata() && (UIP_IP_BUF->destport == networkSocket->Port) )
    if (uip_newdata())
    {
        Lwm2m_Debug("Packet from: %d %d\n", UIP_IP_BUF->destport, networkSocket->Port);
        if (uip_datalen() > bufferLength)
            *readLength = bufferLength;
        else
            *readLength = uip_datalen();
    }
    if (*readLength > 0)
    {
        memcpy(buffer, uip_appdata, *readLength);
        NetworkAddress * networkAddress = NULL;
        NetworkAddress matchAddress;
        size_t size = sizeof(struct _NetworkAddress);
        memset(&matchAddress, 0, size);
        memcpy(&matchAddress.Address, &UIP_IP_BUF->srcipaddr, sizeof(uip_ipaddr_t));
        matchAddress.Port =  UIP_IP_BUF->srcport; //uip_ntohs(UIP_UDP_BUF->srcport);
        matchAddress.Secure = (networkSocket->SocketType & NetworkSocketType_Secure) == NetworkSocketType_Secure;
        networkAddress = getCachedAddress(&matchAddress, NULL, 0);

        if (networkAddress == NULL)
        {
            networkAddress = addCachedAddress(NULL, 0);
            if (networkAddress)
            {
                // Add new address to cache (note: uri and secure is unknown)
                memcpy(networkAddress, &matchAddress, size);
                networkAddress->useCount++;         // TODO - ensure addresses are freed? (after t/o or transaction or DTLS session closed)
            }
        }
        if (networkAddress)
        {
            *sourceAddress = networkAddress;
        }
    }
    return result;
}

bool NetworkSocket_Read(NetworkSocket * networkSocket, uint8_t * buffer, int bufferLength, NetworkAddress ** sourceAddress, int *readLength)
{
    bool result = false;
    if (networkSocket)
    {
        networkSocket->LastError = NetworkSocketError_NoError;
        if (buffer && bufferLength > 0 && readLength)
        {
            *readLength = 0;
            if ((networkSocket->SocketType & NetworkSocketType_UDP) == NetworkSocketType_UDP)
            {
                if (sourceAddress)
                {
                   if (networkSocket->Socket && readUDP(networkSocket, buffer, bufferLength, sourceAddress, readLength))
                   {
                       result = true;
                   }
                   if ((*readLength > 0) && *sourceAddress && (*sourceAddress)->Secure)
                   {
                       if (DTLS_Decrypt(*sourceAddress, buffer, *readLength, encryptBuffer, ENCRYPT_BUFFER_LENGTH, readLength, networkSocket))
                       {
                           if (*readLength > 0)
                           {
                               memcpy(buffer, encryptBuffer, *readLength);
                           }
                       }
                   }
                }
                else
                {
                    networkSocket->LastError = NetworkSocketError_InvalidArguments;
                }
            }
        }
        else
        {
            networkSocket->LastError = NetworkSocketError_InvalidArguments;
        }
    }
    return result;
}

bool sendUDP(NetworkSocket * networkSocket, NetworkAddress * destAddress, const uint8_t * buffer, int bufferLength)
{
    uip_udp_packet_sendto(networkSocket->Socket, buffer, bufferLength, &destAddress->Address, destAddress->Port);
    return true;
}

bool NetworkSocket_Send(NetworkSocket * networkSocket, NetworkAddress * destAddress, uint8_t * buffer, int bufferLength)
{
    bool result = false;
    if (networkSocket && networkSocket->Socket)
    {
        networkSocket->LastError = NetworkSocketError_NoError;
        if (buffer && bufferLength > 0)
        {
            if ((networkSocket->SocketType & NetworkSocketType_UDP) == NetworkSocketType_UDP)
            {
                if (destAddress)
                {
                    if (destAddress->Secure)
                    {
                        int encryptedBytes;
                        if (DTLS_Encrypt(destAddress, buffer, bufferLength, encryptBuffer, ENCRYPT_BUFFER_LENGTH, &encryptedBytes, networkSocket))
                        {
                            buffer = encryptBuffer;
                            bufferLength = encryptedBytes;
                        }
                        else
                        {
                            bufferLength = 0;
                        }
                    }
                    if (bufferLength > 0)
                    {
                        result = sendUDP(networkSocket, destAddress, buffer, bufferLength);
                    }
                }
                else
                {
                    networkSocket->LastError = NetworkSocketError_InvalidArguments;
                }
            }
        }
        else
        {
            networkSocket->LastError = NetworkSocketError_InvalidArguments;
        }
    }
    return result;
}

void NetworkSocket_Free(NetworkSocket ** networkSocket)
{
    if (networkSocket && *networkSocket)
    {
        if ((*networkSocket)->Socket)
            uip_udp_remove((*networkSocket)->Socket);
        free(*networkSocket);
        *networkSocket = NULL;
    }
}


static NetworkTransmissionError SendDTLS(NetworkAddress * destAddress, const uint8_t * buffer, int bufferLength, void *context)
{
    NetworkTransmissionError result = NetworkTransmissionError_None;
    NetworkSocket * networkSocket = (NetworkSocket *)context;
    if (networkSocket)
    {
        if (!sendUDP(networkSocket, destAddress, buffer, bufferLength))
        {
            result = NetworkTransmissionError_TransmitBufferFull;
        }
    }
    return result;
}

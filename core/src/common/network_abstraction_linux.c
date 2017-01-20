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
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
typedef int SOCKET;
#define SOCKET_ERROR            (-1)
#define closesocket close
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/tcp.h>
#include <linux/version.h>

#include "lwm2m_debug.h"
#include "lwm2m_util.h"
#include "network_abstraction.h"
#include "dtls_abstraction.h"


struct _NetworkAddress
{
    union
    {
        struct sockaddr     Sa;
        struct sockaddr_storage St;
        struct sockaddr_in  Sin;
        struct sockaddr_in6 Sin6;
    } Address;
    bool Secure;
    int useCount;
};

struct _NetworkSocket
{
    int Socket;
    int SocketIPv6;
    NetworkAddress * BindAddress;
    NetworkSocketType SocketType;
    uint16_t Port;
    NetworkSocketError LastError;
};

typedef struct
{
    char * uri;
    NetworkAddress * address;
} NetworkAddressCache;

typedef enum
{
    UriParseState_Scheme,
    UriParseState_Hostname,
    UriParseState_Port,
} UriParseState;

#define MAX_URI_LENGTH  (256)

#ifndef MAX_NETWORK_ADDRESS_CACHE
    #define MAX_NETWORK_ADDRESS_CACHE  (5)
#endif

static NetworkAddressCache networkAddressCache[MAX_NETWORK_ADDRESS_CACHE] = {{0}};

static void addCachedAddress(NetworkAddress * address, const char * uri, int uriLength);
static NetworkAddress * getCachedAddress(NetworkAddress * matchAddress, const char * uri, int uriLength);
static NetworkAddress * getCachedAddressByUri(const char * uri, int uriLength);
static int getUriHostLength(const char * uri, int uriLength);

#ifndef ENCRYPT_BUFFER_LENGTH
#define ENCRYPT_BUFFER_LENGTH 1024
#endif

uint8_t encryptBuffer[ENCRYPT_BUFFER_LENGTH];

static NetworkTransmissionError SendDTLS(NetworkAddress * destAddress, const uint8_t * buffer, int bufferLength, void *context);

NetworkAddress * NetworkAddress_FromIPAddress(const char * ipAddress, uint16_t port)
{
    NetworkAddress * result;
    size_t size = sizeof(struct _NetworkAddress);
    result = (NetworkAddress *)malloc(size);
    memset(result, 0, size);
    if (inet_pton(AF_INET, ipAddress, &result->Address.Sin.sin_addr) == 1)
    {
        result->Address.Sin.sin_family = AF_INET;
        result->Address.Sin.sin_port = htons(port);
    }
    else if (inet_pton(AF_INET6, ipAddress, &result->Address.Sin6.sin6_addr) == 1)
    {
        result->Address.Sin6.sin6_family = AF_INET6;
        result->Address.Sin6.sin6_port = htons(port);
    }
    else
    {
        free(result);
        result = NULL;
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
            bool ip6Address = false;
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
                        startIndex = index;
                        while (index < uriLength)
                        {
                            if (uri[index] == ']')
                            {
                                ip6Address = true;
                                break;
                            }
                            hostname[hostnameLength] = uri[index];
                            hostnameLength++;
                            index++;
                        }
                    }
                    else if ((uri[index] == ':') || (uri[index] == '/') )
                    {
                        hostname[hostnameLength] = 0;
                        if  (uri[index] == '/')
                            break;
                        state = UriParseState_Port;
                        port = 0;
                        startIndex = index + 1;
                    }
                    else
                    {
                        hostname[hostnameLength] = uri[index];
                        hostnameLength++;
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
                hostname[hostnameLength] = 0;
            }
            if (hostnameLength > 0 && port > 0)
            {
                NetworkAddress * networkAddress = NULL;
                if (ip6Address)
                {
                    networkAddress = NetworkAddress_FromIPAddress(hostname, port);
                }
                else
                {
                    struct hostent *resolvedAddress = gethostbyname(hostname);
                    if (resolvedAddress)
                    {
                        size_t size = sizeof(struct _NetworkAddress);
                        networkAddress = (NetworkAddress *) malloc(size);
                        if (networkAddress)
                        {
                            memset(networkAddress, 0, size);
                            if (resolvedAddress->h_addrtype == AF_INET)
                            {
                                networkAddress->Address.Sin.sin_family = AF_INET;
                                memcpy(&networkAddress->Address.Sin.sin_addr, *(resolvedAddress->h_addr_list), sizeof(struct in_addr));
                                networkAddress->Address.Sin.sin_port = htons(port);
                            }
                            else if (resolvedAddress->h_addrtype == AF_INET6)
                            {
                                networkAddress->Address.Sin6.sin6_family = AF_INET6;
                                memcpy(&networkAddress->Address.Sin6.sin6_addr, *(resolvedAddress->h_addr_list), sizeof(struct in6_addr));
                                networkAddress->Address.Sin6.sin6_port = htons(port);
                            }
                            else
                            {
                                free(networkAddress);
                                networkAddress = NULL;
                            }
                        }
                    }
                }
                if (networkAddress)
                {
                    networkAddress->Secure = secure;
                    result = getCachedAddress(networkAddress, uri, uriHostLength);
                    if (result)
                    {
                        // Matched existing address
                        free(networkAddress);
                    }
                    else
                    {
                        result = networkAddress;
                    }
                }

            }
        }

        if (result)
        {
            if (result->useCount == 0)
            {
                addCachedAddress(result, uri, uriHostLength);
            }
            result->useCount++;
        }
    }
    return result;
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

int NetworkAddress_Compare(NetworkAddress * address1, NetworkAddress * address2)
{
    int result = -1;

    // Compare address and port (ignore uri)
    if (address1 && address2 && address1->Address.Sa.sa_family == address2->Address.Sa.sa_family)
    {
        if (address1->Address.Sa.sa_family == AF_INET)
        {
            result = memcmp(&address1->Address.Sin.sin_addr.s_addr, &address2->Address.Sin.sin_addr.s_addr, sizeof(address2->Address.Sin.sin_addr.s_addr));
            if (result == 0)
            {
                result = comparePorts(address1->Address.Sin.sin_port, address2->Address.Sin.sin_port);
            }
        }
        else if (address1->Address.Sa.sa_family == AF_INET6)
        {
            result = memcmp(&address1->Address.Sin6.sin6_addr, &address2->Address.Sin6.sin6_addr, sizeof(address2->Address.Sin6.sin6_addr));
            if (result == 0)
            {
                result = comparePorts(address1->Address.Sin6.sin6_port, address2->Address.Sin6.sin6_port);
            }
        }
    }
    return result;
}

void NetworkAddress_SetAddressType(NetworkAddress * address, AddressType * addressType)
{
    if (address && addressType)
    {
        addressType->Size = sizeof(addressType->Addr);
        memcpy(&addressType->Addr, &address->Address, addressType->Size);
        addressType->Secure = address->Secure;
//        if (addressType->Addr.Sa.sa_family == AF_INET6)
//            addressType->Addr.Sin6.sin6_port = ntohs(address->Address.Sin6.sin6_port);
//        else
//            addressType->Addr.Sin.sin_port = ntohs(address->Address.Sin.sin_port);
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
                if (networkAddressCache[index].address == *address)
                {
                    if (networkAddressCache[index].uri)
                    {
                        Lwm2m_Debug("Address free: %s\n", networkAddressCache[index].uri);
                        free(networkAddressCache[index].uri);
                        networkAddressCache[index].uri = NULL;
                    }
                    else
                    {
                        Lwm2m_Debug("Address free\n");
                    }
                    networkAddressCache[index].address = NULL;
                    break;
                }
            }
            free(*address);
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

static void addCachedAddress(NetworkAddress * address, const char * uri, int uriLength)
{
    if (address)
    {
        int index;
        for (index = 0; index < MAX_NETWORK_ADDRESS_CACHE; index++)
        {
            if (networkAddressCache[index].address == NULL)
            {
                if (uri && uriLength > 0)
                {
                    networkAddressCache[index].uri = (char *)malloc(uriLength + 1);
                    if (networkAddressCache[index].uri)
                    {
                        memcpy(networkAddressCache[index].uri, uri, uriLength);
                        networkAddressCache[index].uri[uriLength] = 0;
                        networkAddressCache[index].address = address;
                        Lwm2m_Debug("Address add: %s\n", networkAddressCache[index].uri);
                    }
                }
                else
                {
                    networkAddressCache[index].address = address;
                    networkAddressCache[index].uri = NULL;
                    Lwm2m_Debug("Address add (received)\n");    // TODO - print remote address
                }
                break;
            }
        }
    }
}



static NetworkAddress * getCachedAddressByUri(const char * uri, int uriLength)
{
    NetworkAddress * result = NULL;
    int index;
    for (index = 0; index < MAX_NETWORK_ADDRESS_CACHE; index++)
    {
        if (networkAddressCache[index].uri && (memcmp(networkAddressCache[index].uri, uri, uriLength) == 0))
        {
            //Lwm2m_Debug("Address uri matched: %s\n", networkAddressCache[index].uri);
            result = networkAddressCache[index].address;
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
        NetworkAddress * address = networkAddressCache[index].address;
        if (address)
        {
            if (NetworkAddress_Compare(matchAddress, address) == 0)
            {
                if (uri && uriLength > 0 && networkAddressCache[index].uri == NULL)
                {
                    // Add info to cached address
                    address->Secure = matchAddress->Secure;
                    networkAddressCache[index].uri = (char *)malloc(uriLength + 1);
                    if (networkAddressCache[index].uri)
                    {
                        memcpy(networkAddressCache[index].uri, uri, uriLength);
                        networkAddressCache[index].uri[uriLength] = 0;
                        Lwm2m_Debug("Address add uri: %s\n", networkAddressCache[index].uri);
                    }
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
    NetworkSocket * result = (NetworkSocket *)malloc(size);
    if (result)
    {
        memset(result, 0, size);
        result->SocketType = socketType;
        result->Port = port;
        DTLS_SetNetworkSendCallback(SendDTLS);
        if (ipAddress && (*ipAddress != '\0'))
        {
            result->BindAddress = NetworkAddress_FromIPAddress(ipAddress, port);
            if (!result->BindAddress)
                NetworkSocket_Free(&result);
        }
    }
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
    if (networkSocket)
    {
        result = networkSocket->Socket;
        if (result == SOCKET_ERROR)
            result = networkSocket->SocketIPv6;
    }
    return result;
}

void NetworkSocket_SetCertificate(NetworkSocket * networkSocket, const uint8_t * cert, int certLength, AwaCertificateFormat format)
{
    (void)networkSocket;
    DTLS_SetCertificate(cert, certLength, format);
}

void NetworkSocket_SetPSK(NetworkSocket * networkSocket, const char * identity, const uint8_t * key, int keyLength)
{
    (void)networkSocket;
    DTLS_SetPSK(identity, key, keyLength);
}


bool NetworkSocket_StartListening(NetworkSocket * networkSocket)
{
    bool result = false;
    if (networkSocket)
    {
        int protocol = IPPROTO_UDP;
        int socketMode = SOCK_DGRAM;
        if ((networkSocket->SocketType & NetworkSocketType_TCP) == NetworkSocketType_TCP)
        {
            protocol = IPPROTO_TCP;
            socketMode = SOCK_STREAM;
        }
        struct sockaddr_in ip4AnyAddress;
        struct sockaddr_in6 ip6AnyAddress;

        struct sockaddr_in * ip4Address = NULL;
        struct sockaddr_in6 * ip6Address = NULL;
        if (networkSocket->BindAddress)
        {
            if (networkSocket->BindAddress->Address.Sa.sa_family == AF_INET6)
            {
                ip6Address = &networkSocket->BindAddress->Address.Sin6;
            }
            else if (networkSocket->BindAddress->Address.Sa.sa_family == AF_INET)
            {
                ip4Address = &networkSocket->BindAddress->Address.Sin;
            }
        }
        else
        {

            memset(&ip4AnyAddress, 0, sizeof(struct sockaddr_in));
            ip4AnyAddress.sin_family = AF_INET;
            ip4AnyAddress.sin_addr.s_addr = INADDR_ANY;
            ip4AnyAddress.sin_port = htons(networkSocket->Port);
            ip4Address = &ip4AnyAddress;

            memset(&ip6AnyAddress, 0, sizeof(struct sockaddr_in6));
            ip6AnyAddress.sin6_family = AF_INET6;
            ip6AnyAddress.sin6_port = htons(networkSocket->Port);
            ip6Address = &ip6AnyAddress;

        }
        if (ip4Address)
            networkSocket->Socket = socket(AF_INET, socketMode, protocol);
        else
            networkSocket->Socket = SOCKET_ERROR;
        if (networkSocket->Socket != SOCKET_ERROR)
        {
            struct sockaddr *address = NULL;
            socklen_t addressLength = 0;
            addressLength = sizeof(struct sockaddr_in);
            address = (struct sockaddr *)ip4Address;
            int flag = fcntl(networkSocket->Socket, F_GETFL);
            flag = flag | O_NONBLOCK;
            if (fcntl(networkSocket->Socket, F_SETFL, flag) < 0)
            {
                // ignore error
            }
            if (bind(networkSocket->Socket, address, addressLength) == SOCKET_ERROR)
            {
                Lwm2m_Debug("Failed to bind to ip4 socket\n");
            }
            else
            {
                result = true;

            }
        }
        if (ip6Address)
            networkSocket->SocketIPv6 = socket(AF_INET6, socketMode, protocol);
        else
            networkSocket->SocketIPv6 = SOCKET_ERROR;
        if (networkSocket->SocketIPv6 != SOCKET_ERROR)
        {

            int yes = 1;
            if (setsockopt(networkSocket->SocketIPv6, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(yes) != SOCKET_ERROR))
            {
                struct sockaddr *address = NULL;
                socklen_t addressLength = 0;
                addressLength = sizeof(struct sockaddr_in6);
                address = (struct sockaddr *)ip6Address;
                int flag = fcntl(networkSocket->SocketIPv6, F_GETFL);
                flag = flag | O_NONBLOCK;
                if (fcntl(networkSocket->SocketIPv6, F_SETFL, flag) < 0)
                {
                    // ignore error
                }
                if (bind(networkSocket->SocketIPv6, address, addressLength) == SOCKET_ERROR)
                {
                    Lwm2m_Debug("Failed to bind to ip6 socket\n");
                }
                else
                {
                    result = true;
                }
            }
        }
    }
    return result;
}

bool readUDP(NetworkSocket * networkSocket, int socketHandle, uint8_t * buffer, int bufferLength, NetworkAddress ** sourceAddress, int *readLength)
{
    bool result = false;
    struct sockaddr_storage sourceSocket;
    socklen_t sourceSocketLength = sizeof(struct sockaddr_storage);
    errno = 0;
    *readLength = recvfrom(socketHandle, buffer, bufferLength, MSG_DONTWAIT,
                           (struct sockaddr *)&sourceSocket,
                           &sourceSocketLength);
    int lastError = errno;
    if (*readLength == SOCKET_ERROR)
    {
        *readLength = 0;
        if ((lastError == EWOULDBLOCK) || (lastError == EAGAIN))
        {
            result = true;
        }
        else
        {
            networkSocket->LastError = NetworkSocketError_ReadError;
        }
    }
    else
    {
        NetworkAddress * networkAddress = NULL;
        NetworkAddress matchAddress;
        size_t size = sizeof(struct _NetworkAddress);
        memset(&matchAddress, 0, size);
        memcpy(&matchAddress.Address.Sa, &sourceSocket, sourceSocketLength);
        matchAddress.Secure = (networkSocket->SocketType & NetworkSocketType_Secure) == NetworkSocketType_Secure;
        networkAddress = getCachedAddress(&matchAddress, NULL, 0);

        if (networkAddress == NULL)
        {
            networkAddress = (NetworkAddress *)malloc(size);
            if (networkAddress)
            {
                // Add new address to cache (note: uri and secure is unknown)
                memcpy(networkAddress, &matchAddress, size);
                addCachedAddress(networkAddress, NULL, 0);
                networkAddress->useCount++;         // TODO - ensure addresses are freed? (after t/o or transaction or DTLS session closed)
            }
        }
        if (networkAddress)
        {
            *sourceAddress = networkAddress;
            result = true;
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
                   if ((networkSocket->Socket != SOCKET_ERROR) && readUDP(networkSocket, networkSocket->Socket, buffer, bufferLength, sourceAddress, readLength))
                   {
                       result = true;
                       if (*readLength == 0)
                       {
                           readUDP(networkSocket, networkSocket->SocketIPv6, buffer, bufferLength, sourceAddress, readLength);
                       }
                   }
                   else if ((networkSocket->SocketIPv6 != SOCKET_ERROR) && readUDP(networkSocket, networkSocket->SocketIPv6, buffer, bufferLength, sourceAddress, readLength))
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
            else
            {
                errno = 0;
                *readLength = recv(networkSocket->Socket, buffer, bufferLength, 0);
                int lastError = errno;
                if (*readLength == 0)
                {
                    networkSocket->LastError = NetworkSocketError_ConnectionLost;
                }
                else if (*readLength == SOCKET_ERROR)
                {
                    *readLength = 0;
                    if (lastError == EWOULDBLOCK)
                        *readLength = 0;
                    else if (lastError == ENOTCONN)
                        networkSocket->LastError = NetworkSocketError_ConnectionLost;
                    else if (lastError == EBADF)
                        networkSocket->LastError = NetworkSocketError_ConnectionLost;
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
    bool result = false;
    int socketHandle = networkSocket->Socket;
    if (destAddress->Address.Sa.sa_family == AF_INET6)
        socketHandle = networkSocket->SocketIPv6;
    size_t addressLength = sizeof(struct sockaddr_storage);
    while (bufferLength > 0)
    {
        errno = 0;
        int sentBytes = sendto(socketHandle, buffer, bufferLength, 0, &destAddress->Address.Sa, addressLength);
        int lastError = errno;
        if (sentBytes == SOCKET_ERROR)
        {
            if ((lastError == EWOULDBLOCK) || (lastError == EINTR))
            {
                sentBytes = 0;
            }
            else
            {
                networkSocket->LastError = NetworkSocketError_SendError;
                break;
            }
        }
        buffer += sentBytes;
        bufferLength -= sentBytes;
    }

    result = (bufferLength == 0);
    return result;
}

bool NetworkSocket_Send(NetworkSocket * networkSocket, NetworkAddress * destAddress, uint8_t * buffer, int bufferLength)
{
    bool result = false;
    if (networkSocket)
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
        if ((*networkSocket)->Socket != SOCKET_ERROR)
            close((*networkSocket)->Socket);
        if ((*networkSocket)->SocketIPv6 != SOCKET_ERROR)
            close((*networkSocket)->SocketIPv6);
        if ((*networkSocket)->BindAddress)
            NetworkAddress_Free(&(*networkSocket)->BindAddress);
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

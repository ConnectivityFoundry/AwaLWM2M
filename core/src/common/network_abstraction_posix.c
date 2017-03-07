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

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/socket.h>
#define SOCKET_ERROR            (-1)
#include <netinet/in.h>
#include <arpa/inet.h>

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

#ifndef ENCRYPT_BUFFER_LENGTH
#define ENCRYPT_BUFFER_LENGTH 1024
#endif

static uint8_t encryptBuffer[ENCRYPT_BUFFER_LENGTH];

static bool sendUDP(NetworkSocket * networkSocket, NetworkAddress * destAddress, const uint8_t * buffer, int bufferLength)
{
    bool result = false;
    int socketHandle = networkSocket->Socket;
    if (destAddress->Address.Sa.sa_family == AF_INET6)
        socketHandle = networkSocket->SocketIPv6;
    size_t addressLength = sizeof(struct sockaddr_storage);
    while (bufferLength > 0)
    {
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

bool NetworkAddress_IsSecure(const NetworkAddress * address)
{
    bool result = false;
    if (address)
    {
        result = address->Secure;
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

void NetworkAddress_SetAddressType(NetworkAddress * address, AddressType * addressType)
{
    if (address && addressType)
    {
        addressType->Size = sizeof(addressType->Addr);
        memcpy(&addressType->Addr, &address->Address, addressType->Size);
        addressType->Secure = address->Secure;
#ifdef RIOT
        if (addressType->Addr.Sa.sa_family == AF_INET6)
            addressType->Addr.Sin6.sin6_port = ntohs(address->Address.Sin6.sin6_port);
        else
            addressType->Addr.Sin.sin_port = ntohs(address->Address.Sin.sin_port);
#endif
    }
}
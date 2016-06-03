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
#include <linux/tcp.h>
#include <linux/version.h>

#include "network_abstraction.h"


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
};

struct _NetworkSocket
{
    int Socket;
    int SocketIPv4;
    NetworkSocketType SocketType;
    uint16_t Port;
    NetworkSocketError LastError;
};


typedef enum
{
    UriParseState_Scheme,
    UriParseState_Hostname,
    UriParseState_Port,
}UriParseState;


#define MAX_URI_LENGTH  (256)


NetworkAddress * NetworkAddress_New(const char * uri, int uriLength)
{
    NetworkAddress * result = NULL;
    if (uri && uriLength > 0)
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
                else if isdigit(uri[index])
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
            struct hostent *resolvedAddress = gethostbyname(hostname);
            if (resolvedAddress)
            {
                size_t size = sizeof(struct _NetworkAddress);
                result = (NetworkAddress *)malloc(size);
                if (result)
                {
                    memset(result, size, 0);
                    result->Secure = secure;
                    if (resolvedAddress->h_addrtype == AF_INET)
                    {
                        result->Address.Sin.sin_family = AF_INET;
                        memcpy(&result->Address.Sin.sin_addr,*(resolvedAddress->h_addr_list),sizeof(struct in_addr));
                        result->Address.Sin.sin_port = htons(port);
                    }
                    else if (resolvedAddress->h_addrtype == AF_INET6)
                    {
                        result->Address.Sin6.sin6_family = AF_INET6;
                        memcpy(&result->Address.Sin6.sin6_addr,*(resolvedAddress->h_addr_list),sizeof(struct in6_addr));
                        result->Address.Sin6.sin6_port = htons(port);
                    }
                    else
                    {
                        free(result);
                        result = NULL;
                    }
                }
            }
        }
    }
    return result;
}

//int NetworkAddress_Compare(NetworkAddress * addressX, NetworkAddress * addressY);

void NetworkAddress_SetAddressType(NetworkAddress * address, AddressType * addressType)
{
    if (address && addressType)
    {
        addressType->Size = sizeof(addressType->Addr);
        memcpy(&addressType->Addr, &address->Address, addressType->Size);
        if (addressType->Addr.Sa.sa_family == AF_INET6)
            addressType->Addr.Sin6.sin6_port = ntohs(address->Address.Sin6.sin6_port);
        else
            addressType->Addr.Sin.sin_port = ntohs(address->Address.Sin.sin_port);
    }
}

void NetworkAddress_Free(NetworkAddress ** address)
{
    if (address && *address)
    {
        free(*address);
        *address = NULL;
    }
}

NetworkSocket * NetworkSocket_New(NetworkSocketType socketType, uint16_t port)
{
    size_t size = sizeof(struct _NetworkSocket);
    NetworkSocket * result = (NetworkSocket *)malloc(size);
    if (result)
    {
        memset(result, size, 0);
        result->SocketType = socketType;
        result->Port = port;
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


void NetworkSocket_SetCertificate(NetworkSocket * networkSocket, uint8_t * cert, int certLength, CertificateFormat format)
{

}

void NetworkSocket_SetPSK(NetworkSocket * networkSocket, const char * identity, uint8_t * key, int keyLength)
{

}


bool NetworkSocket_StartListening(NetworkSocket * networkSocket)
{
    bool result = false;
    if (networkSocket)
    {
        int protocol = IPPROTO_UDP;
        int socketMode = SOCK_DGRAM;
        if (networkSocket->SocketType == NetworkSocketType_TCP)
        {
            protocol = IPPROTO_TCP;
            socketMode = SOCK_STREAM;
        }

        networkSocket->Socket = socket(AF_INET6, socketMode, protocol);
        if (networkSocket->Socket != SOCKET_ERROR)
        {
            int yes = 1;
            setsockopt(networkSocket->Socket, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(yes));
            struct sockaddr *address = NULL;
            socklen_t addressLength = 0;
            addressLength = sizeof(struct sockaddr_in6);
            struct sockaddr_in6 ipAddress;
            memset(&ipAddress, 0, addressLength);
            ipAddress.sin6_family = AF_INET6;
            //memset(&ipAddress->sin6_addr, 0, sizeof(ipAddress->sin6_addr));
            //ipAddress->sin6_addr.__in6_u = IN6ADDR_ANY_INIT; //IN6ADDR_ANY_INIT
            ipAddress.sin6_port = htons(networkSocket->Port);
            address = (struct sockaddr *)&ipAddress;
            int flag = fcntl(networkSocket->Socket, F_GETFL);
            flag = flag | O_NONBLOCK;
            if (fcntl(networkSocket->Socket, F_SETFL, flag) < 0)
            {

            }
            if (bind(networkSocket->Socket, address, addressLength) != SOCKET_ERROR)
            {
                result = true;
            }
        }
        networkSocket->SocketIPv4 = socket(AF_INET, socketMode, protocol);
        if (networkSocket->SocketIPv4 != SOCKET_ERROR)
        {

            struct sockaddr *address = NULL;
            socklen_t addressLength = 0;
            addressLength = sizeof(struct sockaddr_in);
            struct sockaddr_in ipAddress;
            memset(&ipAddress, 0, addressLength);
            ipAddress.sin_family = AF_INET;
            ipAddress.sin_addr.s_addr = INADDR_ANY;
            ipAddress.sin_port = htons(networkSocket->Port);
            address = (struct sockaddr *)&ipAddress;
            int flag = fcntl(networkSocket->SocketIPv4, F_GETFL);
            flag = flag | O_NONBLOCK;
            if (fcntl(networkSocket->SocketIPv4, F_SETFL, flag) < 0)
            {

            }
            if (bind(networkSocket->SocketIPv4, address, addressLength) != SOCKET_ERROR)
            {
                result = true;
            }
        }
    }
    return result;
}

//bool NetworkSocket_Connect(NetworkSocket networkSocket, NetworkAddress * destAddress);

bool readUDP(NetworkSocket * networkSocket, int socketHandle, uint8_t * buffer, int bufferLength, NetworkAddress ** sourceAddress, int *readLength)
{
    bool result = false;
    struct sockaddr sourceSocket;
    socklen_t sourceSocketLength = sizeof(struct sockaddr);
    errno = 0;
    *readLength = recvfrom(socketHandle, buffer, bufferLength, MSG_DONTWAIT, &sourceSocket, &sourceSocketLength);
    int lastError = errno;
    if (*readLength == SOCKET_ERROR)
    {
        *readLength = 0;
        if ((lastError == EWOULDBLOCK) || (lastError == EAGAIN) )
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
        if (*sourceAddress == NULL)
        {
            size_t size = sizeof(struct _NetworkAddress);
            *sourceAddress = (NetworkAddress *)malloc(size);
            if (*sourceAddress)
            {
                memset(*sourceAddress, size, 0);
            }
        }
        if (*sourceAddress)
        {
            memcpy(&(*sourceAddress)->Address, &sourceSocket, sourceSocketLength);
        }
        result = true;
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
            if (networkSocket->SocketType == NetworkSocketType_UDP)
            {
                if (sourceAddress)
                {
                   if (readUDP(networkSocket, networkSocket->Socket, buffer, bufferLength, sourceAddress, readLength))
                   {
                       result = true;
                       if (*readLength == 0)
                       {
                           readUDP(networkSocket, networkSocket->SocketIPv4, buffer, bufferLength, sourceAddress, readLength);
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

bool NetworkSocket_Send(NetworkSocket * networkSocket, NetworkAddress * destAddress, uint8_t * buffer, int bufferLength)
{
    bool result = false;
    if (networkSocket)
    {
        networkSocket->LastError = NetworkSocketError_NoError;
        if (buffer && bufferLength > 0)
        {
            if (networkSocket->SocketType == NetworkSocketType_UDP)
            {
                if (destAddress)
                {
                    int socketHandle = networkSocket->Socket;
                    if (destAddress->Address.Sa.sa_family == AF_INET)
                        socketHandle = networkSocket->SocketIPv4;
                    size_t addressLength = sizeof(struct sockaddr);
                    while (bufferLength > 0)
                    {
                        errno = 0;
                        int sentBytes = sendto(socketHandle, buffer, bufferLength, 0, &destAddress->Address.Sa, addressLength);
                        int lastError = errno;
                        if (sentBytes == SOCKET_ERROR)
                        {
                            if (lastError == EWOULDBLOCK)
                            {
                                sentBytes = 0;
                            }
                            else if (lastError == ENOTCONN)
                            {
                                networkSocket->LastError = NetworkSocketError_SendError;
                                break;
                            }
                            else if (lastError == ECONNRESET)
                            {
                                networkSocket->LastError = NetworkSocketError_SendError;
                                break;
                            }
                            else if (lastError == EBADF)
                            {
                                networkSocket->LastError = NetworkSocketError_SendError;
                                break;
                            }
                        }
                        buffer += sentBytes;
                        bufferLength -= sentBytes;
                    }
                    result = (bufferLength == 0);
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
        if ((*networkSocket)->SocketIPv4 != SOCKET_ERROR)
            close((*networkSocket)->SocketIPv4);
        free(*networkSocket);
        *networkSocket = NULL;
    }
}

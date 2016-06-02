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


#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/timeb.h>
#include <netdb.h>
#include <errno.h>
#include <poll.h>

#include "ipc.h"
#include "memalloc.h"
#include "log.h"
#include "xml.h"
#include "utils.h"

#define MAX_XML_BUFFER (65536)  // Should match core/src/common/lwm2m_xml_interface.c

struct _IPCInfo
{
    struct addrinfo * AddressInfo;
};

struct _IPCChannel
{
    int Socket;
    int NotifySocket;
    struct sockaddr_storage DestinationAddress;
    socklen_t DestinationAddressLength;
};

struct _IPCMessage
{
    TreeNode RootNode;
};

static struct addrinfo * GetAddressInfo(const char * address, unsigned short port)
{
    struct addrinfo hints;
    struct addrinfo *result;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_ADDRCONFIG; /* only return IPv4/6 if IPv4/6 address configured (respectively). */
    hints.ai_protocol = 0;          /* Any protocol */

    char service[32] = { 0 };
    sprintf(service, "%"PRIu16, port);

    if (getaddrinfo(address, service, &hints, &result) != 0)
    {
        result = NULL;
    }

    return result;
}

IPCInfo * IPCInfo_NewUDP(const char * address, unsigned short port)
{
    IPCInfo * ipcInfo = NULL;

    struct addrinfo * addressInfo = GetAddressInfo(address, port);

    if (addressInfo != NULL)
    {
        struct addrinfo * rp;

        // report all addrinfo records:
        for (rp = addressInfo; rp != NULL; rp = rp->ai_next)
        {
            LogDebug("addrinfo: family %d, socktype %d, protocol %d", rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        }

        ipcInfo = Awa_MemAlloc(sizeof(*ipcInfo));
        if (ipcInfo != NULL)
        {
            memset(ipcInfo, 0, sizeof(*ipcInfo));

            // only use the first result
            ipcInfo->AddressInfo = addressInfo;

            LogDebug("New UDP IPCInfo: address %s, port %d", address, port);
            LogNew("IPCInfo", ipcInfo);
        }
        else
        {
            freeaddrinfo(addressInfo);
            LogErrorWithEnum(AwaError_OutOfMemory);
        }
    }
    return ipcInfo;
}

void IPCInfo_Free(IPCInfo ** ipcInfo)
{
    if (ipcInfo != NULL && *ipcInfo != NULL)
    {
        if ((*ipcInfo)->AddressInfo != NULL)
        {
            freeaddrinfo((*ipcInfo)->AddressInfo);
        }
        LogFree("IPCInfo", ipcInfo);
        Awa_MemSafeFree(*ipcInfo);
        *ipcInfo = NULL;
    }
}

static InternalError CreateUDPSockets(IPCChannel * channel, const IPCInfo * ipcInfo)
{
    InternalError result = InternalError_Unspecified;

    if ((channel->Socket = socket(ipcInfo->AddressInfo->ai_family, ipcInfo->AddressInfo->ai_socktype, ipcInfo->AddressInfo->ai_protocol))  > 0)
    {
        if ((channel->NotifySocket = socket(ipcInfo->AddressInfo->ai_family, ipcInfo->AddressInfo->ai_socktype, ipcInfo->AddressInfo->ai_protocol)) > 0)
        {
            memcpy(&channel->DestinationAddress, ipcInfo->AddressInfo->ai_addr, ipcInfo->AddressInfo->ai_addrlen);
            channel->DestinationAddressLength = ipcInfo->AddressInfo->ai_addrlen;

            result = InternalError_Success;
            LogDebug("UDP sockets created");
        }
        else
        {
            LogPError("Could not create Notify UDP Socket");
            result = InternalError_IPCChannel;
        }
    }
    else
    {
        LogPError("Could not create UDP Socket");
        result = InternalError_IPCChannel;
    }

    return result;
}

IPCChannel * IPCChannel_New(const IPCInfo * ipcInfo)
{
    IPCChannel * channel = NULL;
    if (ipcInfo != NULL)
    {
        // initialise IPC channel so that it is fully operational
        channel = Awa_MemAlloc(sizeof(*channel));
        if (channel != NULL)
        {
            memset(channel, 0, sizeof(*channel));

            if (ipcInfo->AddressInfo != NULL)
            {
                // For UDP:
                if (CreateUDPSockets(channel, ipcInfo) == InternalError_Success)
                {
                    LogNew("IPCChannel", channel);
                }
                else
                {
                    Awa_MemSafeFree(channel);
                    channel = NULL;
                }
            }
        }
        else
        {
            LogErrorWithEnum(AwaError_OutOfMemory);
        }
    }
    return channel;
}

void IPCChannel_Free(IPCChannel ** channel)
{
    if ((channel != NULL) && (*channel != NULL))
    {
        if ((*channel)->Socket > 0)
        {
            close((*channel)->Socket);
            (*channel)->Socket = 0;
        }
        if ((*channel)->NotifySocket > 0)
        {
            close((*channel)->NotifySocket);
            (*channel)->NotifySocket = 0;
        }
        LogFree("IPCChannel", *channel);
        Awa_MemSafeFree(*channel);
        *channel = NULL;
    }
}

IPCMessage * IPCMessage_New(void)
{
    IPCMessage * message = Awa_MemAlloc(sizeof(*message));
    if (message != NULL)
    {
        memset(message, 0, sizeof(*message));
        message->RootNode = NULL;
        LogNew("IPCMessage", message);
    }
    else
    {
        LogErrorWithEnum(AwaError_OutOfMemory);
    }
    return message;
}

IPCMessage * IPCMessage_NewPlus(const char * type, const char * subType, IPCSessionID sessionID)
{
    IPCMessage * message = IPCMessage_New();
    if (message != NULL)
    {
        if ((IPCMessage_SetType(message, type, subType) != InternalError_Success) ||
            (IPCMessage_SetSessionID(message, sessionID) != InternalError_Success))
        {
            LogError("Failed to set message headers");
            IPCMessage_Free(&message);
        }
    }
    else
    {
        LogError("Failed to create message");
    }
    return message;
}

void IPCMessage_Free(IPCMessage ** message)
{
    if (message != NULL && *message != NULL)
    {
        if ((*message)->RootNode != NULL)
        {
            Tree_Delete((*message)->RootNode);
            (*message)->RootNode = NULL;
        }
        LogFree("IPCMessage", message);
        Awa_MemSafeFree(*message);
        *message = NULL;
    }
}

InternalError IPCMessage_SetType(IPCMessage * message, const char * type, const char * subType)
{
    InternalError result = InternalError_Success;
    if ((message != NULL) && (type != NULL) && (subType != NULL))
    {
        if (message->RootNode == NULL)
        {
            message->RootNode = Xml_CreateNode(type);
            if (message->RootNode != NULL)
            {
                result = InternalError_Success;
            }
            else
            {
                result = InternalError_OutOfMemory;
            }
        }
        else
        {
            // node already exists - rename the root node, add a Type node if it doesn't exist
            TreeNode_SetName(message->RootNode, type, strlen(type));
        }

        // determine length of path
        char * path = NULL;
        if (msprintf(&path, "%s/Type", type) > 0)
        {
            TreeNode subTypeNode = TreeNode_Navigate(message->RootNode, path);

            if (subTypeNode)
            {
                TreeNode_SetValue(subTypeNode, (const uint8_t *)subType, strlen(subType));
            }
            else
            {
                TreeNode subTypeNode = Xml_CreateNodeWithValue("Type", subType);
                if (subTypeNode != NULL)
                {
                    if (TreeNode_AddChild(message->RootNode, subTypeNode) != false)
                    {
                        result = InternalError_Success;
                    }
                    else
                    {
                        result = InternalError_Tree;
                    }
                }
                else
                {
                    result = InternalError_OutOfMemory;
                }

                TreeNode contentNode = Xml_CreateNode("Content");
                if (contentNode != NULL)
                {
                    if (TreeNode_AddChild(message->RootNode, contentNode) != false)
                    {
                        result = InternalError_Success;
                    }
                    else
                    {
                        result = InternalError_Tree;
                    }
                }
                else
                {
                    result = InternalError_OutOfMemory;
                }
            }

        }
        else
        {
            result = InternalError_OutOfMemory;
        }

        if (path != NULL)
        {
            Awa_MemSafeFree(path);
            path = NULL;
        }
    }
    else
    {
        result = InternalError_ParameterInvalid;
    }

    return result;
}

InternalError IPCMessage_GetType(const IPCMessage * message, const char ** type_, const char ** subType_)
{
    InternalError result = InternalError_Unspecified;
    const char * type = NULL;
    const char * subType = NULL;

    // message must be non-null, and either type or subType must be non-null, or both.
    if ((message != NULL) && !((type_ == NULL) && (subType_ == NULL)))
    {
        if (message->RootNode && (type = TreeNode_GetName(message->RootNode)) != NULL)
        {
            char * path = NULL;
            if (msprintf(&path, "%s/Type", type) > 0)
            {
                TreeNode subTypeNode = TreeNode_Navigate(message->RootNode, path);

                if (subTypeNode)
                {
                    if ((subType = (const char *)TreeNode_GetValue(subTypeNode)) != NULL)
                    {
                        result = InternalError_Success;
                    }
                    else
                    {
                        type = NULL;
                        result = InternalError_InvalidMessage;
                    }
                }
                else
                {
                    type = NULL;
                    result = InternalError_InvalidMessage;
                }
            }
            else
            {
                type = NULL;
                result = InternalError_InvalidMessage;
            }

            if (path != NULL)
            {
                Awa_MemSafeFree(path);
            }
        }
        else
        {
            result = InternalError_InvalidMessage;
        }
    }
    else
    {
        result = InternalError_ParameterInvalid;
    }

    if (type_)
    {
        *type_ = type;
    }

    if (subType_)
    {
        *subType_ = subType;
    }

    return result;
}

InternalError IPCMessage_SetSessionID(IPCMessage * message, IPCSessionID sessionID)
{
    InternalError result = InternalError_Success;

    // a session ID of -1 will clear any existing tag

    if (message != NULL)
    {
        if (message->RootNode != NULL)
        {
            const char * rootName = TreeNode_GetName(message->RootNode);
            if (rootName != NULL)
            {
                // determine length of path
                char * path = NULL;
                if (msprintf(&path, "%s/SessionID", TreeNode_GetName(message->RootNode)) > 0)
                {
                    TreeNode sessionIDNode = TreeNode_Navigate(message->RootNode, path);

                    if (sessionID == -1)
                    {
                        // clear existing
                        if (sessionIDNode)
                        {
                            Tree_DetachNode(sessionIDNode);
                            Tree_Delete(sessionIDNode);
                        }
                        // else do nothing
                    }
                    else
                    {
                        // set
                        char * value = NULL;
                        if (msprintf(&value, "%d", sessionID) > 0)
                        {
                            if (sessionIDNode)
                            {
                                TreeNode_SetValue(sessionIDNode, (const uint8_t *)value, strlen(value));
                                result = InternalError_Success;
                            }
                            else
                            {
                                TreeNode sessionIDNode = Xml_CreateNodeWithValue("SessionID", value);
                                if (sessionIDNode != NULL)
                                {
                                    if (TreeNode_AddChild(message->RootNode, sessionIDNode) != false)
                                    {
                                        result = InternalError_Success;
                                    }
                                    else
                                    {
                                        LogError("TreeNode_AddChild failed");
                                        result = InternalError_Tree;
                                    }
                                }
                                else
                                {
                                    LogError("Xml_CreateNodeWithValue failed");
                                    result = InternalError_OutOfMemory;
                                }
                            }
                        }
                        else
                        {
                            LogError("msprintf failed");
                            result = InternalError_OutOfMemory;
                        }

                        if (value != NULL)
                        {
                            Awa_MemSafeFree(value);
                            value = NULL;
                        }
                    }
                }
                else
                {
                    LogError("msprintf failed");
                    result = InternalError_OutOfMemory;
                }

                if (path != NULL)
                {
                    Awa_MemSafeFree(path);
                    path = NULL;
                }
            }
            else
            {
                LogError("message root name is NULL");
                result = InternalError_InvalidMessage;
            }
        }
        else
        {
            LogError("message root is NULL");
            result = InternalError_InvalidMessage;
        }
    }
    else
    {
        LogError("message is NULL");
        result = InternalError_InvalidMessage;
    }

    return result;
}

IPCSessionID IPCMessage_GetSessionID(const IPCMessage * message)
{
    IPCSessionID sessionID = -1;

    if (message != NULL)
    {
        const char * type = NULL;
        if (message->RootNode && (type = TreeNode_GetName(message->RootNode)) != NULL)
        {
            char * path = NULL;
            if (msprintf(&path, "%s/SessionID", type) > 0)
            {
               TreeNode sessionIDNode = TreeNode_Navigate(message->RootNode, path);
               const char * sessionIDStr = NULL;

               if ((sessionIDStr = (const char *)TreeNode_GetValue(sessionIDNode)) != NULL)
               {
                   sessionID = atoi(sessionIDStr);
               }
            }
            Awa_MemSafeFree(path);
        }
    }
    else
    {
        LogError("message is NULL");
    }
    return sessionID;
}

IPCResponseCode IPCMessage_GetResponseCode(const IPCMessage * message)
{
    IPCResponseCode code = IPCResponseCode_NotSet;
    if (message != NULL)
    {
        if (message->RootNode)
        {
            const char * type = TreeNode_GetName(message->RootNode);
            if (type != NULL)
            {
                char * path = NULL;
                if (msprintf(&path, "%s/Code", type) > 0)
                {
                   TreeNode codeNode = TreeNode_Navigate(message->RootNode, path);
                   const char * codeStr = (const char *)TreeNode_GetValue(codeNode);
                   if (codeStr != NULL)
                   {
                       code = atoi(codeStr);
                   }
                   else
                   {
                       LogError("codeStr is NULL");
                   }
                }
                else
                {
                    LogError("msprintf failed");
                }
                Awa_MemSafeFree(path);
            }
            else
            {
                LogError("message->RootNode name is NULL");
            }
        }
        else
        {
            LogError("message->RootNode is NULL");
        }
    }
    else
    {
        LogError("message is NULL");
    }
    return code;
}

TreeNode IPCMessage_GetContentNode(IPCMessage * message)
{
    TreeNode content = NULL;

    if (message != NULL)
    {
        const char * type = NULL;
        if (message->RootNode && (type = TreeNode_GetName(message->RootNode)) != NULL)
        {
            char * path = NULL;
            if (msprintf(&path, "%s/Content", type) > 0)
            {
               content = TreeNode_Navigate(message->RootNode, path);
            }

            Awa_MemSafeFree(path);
        }
    }
    else
    {
        LogError("message is NULL");
    }
    return content;
}

AwaError IPCMessage_AddContent(IPCMessage * message, TreeNode content)
{
    AwaError result = AwaError_IPCError;

    if (message != NULL)
    {
        const char * type = NULL;
        if (message->RootNode && (type = TreeNode_GetName(message->RootNode)) != NULL)
        {
            if (content != NULL)
            {
                char * path = NULL;
                if (msprintf(&path, "%s/Content", type) > 0)
                {
                    TreeNode contentNode = TreeNode_Navigate(message->RootNode, path);
                    if (contentNode)
                    {
                        TreeNode contentCopy = Tree_Copy(content);
                        if (contentCopy != NULL)
                        {
                            TreeNode_AddChild(contentNode, contentCopy);
                            result = AwaError_Success;
                        }
                        else
                        {
                            result = AwaError_OutOfMemory;
                        }
                    }
                    else
                    {
                        result = AwaError_IPCError;
                    }
                }
                else
                {
                    result = AwaError_OutOfMemory;
                }

                if (path != NULL)
                {
                    Awa_MemSafeFree(path);
                    path = NULL;
                }
            }
            else
            {
                result = AwaError_IPCError;
            }
        }
    }
    else
    {
        LogError("message is NULL");
    }
    return result;
}

static AwaError IPC_SendAndReceiveUsingSocket(int socket, struct sockaddr_storage * destinationAddress,  socklen_t destinationAddressLength, const IPCMessage * request, IPCMessage ** response, int32_t timeout)
{
    AwaError result = AwaError_Success;

    char * requestBuffer = IPC_SerialiseMessageToXML(request);

    if (response != NULL)
    {
        *response = NULL;
    }
    struct timeb start, end;
    ftime(&start);

    if ((requestBuffer != NULL) && (strlen(requestBuffer) > 0))
    {
        LogDebug("IPC send:\n%s", requestBuffer);
        if (sendto(socket, requestBuffer, strlen(requestBuffer), 0, (struct sockaddr *)destinationAddress, destinationAddressLength) > 0)
        {
            if (response != NULL)
            {
                struct pollfd fd = {
                        .fd = socket,
                        .events = POLLIN,
                };

                // an API timeout of zero means infinite wait
                timeout = timeout > 0 ? timeout : -1;
                int rc = poll(&fd, 1, timeout);

                if (rc < 0)
                {
                      LogPError("Could not receive response on IPC");
                      result = AwaError_IPCError;
                }
                else if (rc > 0)
                {
                    if (fd.revents == POLLIN)
                    {
                        char recvBuffer[MAX_XML_BUFFER] = {0};
                        int recvBufferLen = 0;
                        struct sockaddr_storage recvAddr = {0};
                        socklen_t recvAddrLen = 0;

                        if ((recvBufferLen = recvfrom(socket, recvBuffer, sizeof(recvBuffer), 0, (struct sockaddr *)&recvAddr, &recvAddrLen)) > 0)
                        {
                            LogDebug("IPC receive:\n%s", recvBuffer);
                            *response = IPC_DeserialiseMessageFromXML(recvBuffer, recvBufferLen);

                            if (*response != NULL)
                            {
                                //TODO: check response code
                                result = AwaError_Success;
                            }
                            else
                            {
                                result = LogErrorWithEnum(AwaError_IPCError, "Failed to deserialise XML");
                            }
                        }
                        else
                        {
                            LogPError("Could not receive response on IPC");
                            result = AwaError_IPCError;
                        }
                    }
                    else
                    {
                        result = AwaError_IPCError;
                    }
                }
                else
                {
                    ftime(&end);
                    int diff = (int) (1000.0 * (end.time - start.time) + (end.millitm - start.millitm));

                    LogError("Timed out receiving response on IPC (timeout %d ms, wait time %d ms)", timeout, diff);
                    result = AwaError_Timeout;
                }
            }
            else
            {
                // no response required
                result = AwaError_Success;
            }
        }
        else
        {
            LogPError("Could not send request on IPC");
            result = AwaError_IPCError;
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_IPCError, "Serialisation failed");
    }

    Awa_MemSafeFree(requestBuffer);


    return result;
}

AwaError IPC_SendAndReceive(IPCChannel * channel, const IPCMessage * request, IPCMessage ** response, int32_t timeout)
{
    AwaError result = AwaError_Success;
    if (channel != NULL)
    {
        result = IPC_SendAndReceiveUsingSocket(channel->Socket, &channel->DestinationAddress, channel->DestinationAddressLength, request, response, timeout);
    }
    else
    {
        result = LogErrorWithEnum(AwaError_IPCError, "Channel is NULL");
    }
    return result;
}

AwaError IPC_SendAndReceiveOnNotifySocket(IPCChannel * channel, const IPCMessage * request, IPCMessage ** response, int32_t timeout)
{
    AwaError result = AwaError_Success;
    if (channel != NULL)
    {
        result = IPC_SendAndReceiveUsingSocket(channel->NotifySocket, &channel->DestinationAddress, channel->DestinationAddressLength, request, response, timeout);
    }
    else
    {
        result = LogErrorWithEnum(AwaError_IPCError, "Channel is NULL");
    }
    return result;
}

AwaError IPC_WaitForNotification(IPCChannel * channel, int32_t timeout)
{
    AwaError result;

    if (channel != NULL)
    {
        struct pollfd fd = {
                .fd = channel->NotifySocket,
                .events = POLLIN,
        };

        int rc = poll(&fd, 1, timeout);

        if (rc < 0)
        {
            LogPError("Wait for notification failed");
            result = AwaError_IPCError;
        }
        else if (rc > 0)
        {
            if (fd.revents == POLLIN)
            {
                result = AwaError_Success;
            }
            else
            {
                result = AwaError_Timeout;
            }
        }
        else
        {
            result = AwaError_Timeout;
        }
    }
    else
    {
         result = LogErrorWithEnum(AwaError_IPCError, "Channel is NULL");
    }
    return result;
}

AwaError IPC_ReceiveNotification(IPCChannel * channel, IPCMessage ** notification)
{
    AwaError result = AwaError_Success;

    if (channel != NULL && notification != NULL)
    {
        char recvBuffer[MAX_XML_BUFFER] = {0};
        int recvBufferLen = 0;
        struct sockaddr_storage recvAddr = {0};
        socklen_t recvAddrLen = 0;

        if ((recvBufferLen = recvfrom(channel->NotifySocket, recvBuffer, sizeof(recvBuffer), 0, (struct sockaddr *)&recvAddr, &recvAddrLen)) > 0)
        {
            LogDebug("IPC notify:\n%s", recvBuffer);
            *notification = IPC_DeserialiseMessageFromXML(recvBuffer, recvBufferLen);

            const char * type = NULL;
            IPCMessage_GetType(*notification, &type, NULL);
            if (strcmp(IPC_MESSAGE_TYPE_NOTIFICATION, type) == 0)
            {
                if (*notification != NULL)
                {
                    // Notifications have no response code
                    result = AwaError_Success;
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_IPCError, "Failed to deserialise message.");
                }
            }
            else
            {
                result = LogErrorWithEnum(AwaError_IPCError, "Unexpected message on notification channel.");
            }
        }
        else
        {
            if (errno == EAGAIN)
            {
                result = LogErrorWithEnum(AwaError_Timeout, "Timed out receiving notification on IPC");
            }
            else
            {
                result = LogErrorWithEnum(AwaError_IPCError, "Could not receive notification on IPC");
            }
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_IPCError, "Parameter is NULL");
    }
    return result;
}


IPCMessage * IPC_DeserialiseMessageFromXML(char * messageBuffer, size_t messageBufferLen)
{
    IPCMessage * message = NULL;

    if (messageBuffer && messageBufferLen)
    {
        TreeNode rootNode = NULL;

        if ((rootNode = TreeNode_ParseXML((uint8_t *)messageBuffer, messageBufferLen, true)) != NULL)
        {
            message = IPCMessage_New();
            message->RootNode = rootNode;
        }
    }
    return message;
}

char * IPC_SerialiseMessageToXML(const IPCMessage * message)
{
    char * buffer = NULL;

    if (message != NULL)
    {
        buffer = Awa_MemAlloc(MAX_XML_BUFFER);
        if (buffer != NULL)
        {
            memset(buffer, 0, MAX_XML_BUFFER);
            Xml_TreeToString(message->RootNode, buffer, MAX_XML_BUFFER);
        }
        else
        {
            LogErrorWithEnum(AwaError_OutOfMemory);
        }
    }
    return buffer;
}


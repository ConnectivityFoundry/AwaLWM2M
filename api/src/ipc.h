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


#ifndef IPC_H
#define IPC_H

#include <stdint.h>
#include "awa/error.h"
#include "error.h"
#include "xmltree.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IPC_DEFAULT_ADDRESS "127.0.0.1"
#define IPC_DEFAULT_CLIENT_PORT (12345)
#define IPC_DEFAULT_SERVER_PORT (54321)

#define IPC_MSGTYPE_REQUEST  "Request"
#define IPC_MSGTYPE_RESPONSE "Response"

// Common request message types:
#define IPC_MSGTYPE_CONNECT              "Connect"
#define IPC_MSGTYPE_DISCONNECT           "Disconnect"
#define IPC_MSGTYPE_DELETE               "Delete"

// Client request message types:
#define IPC_MSGTYPE_GET                  "Get"
#define IPC_MSGTYPE_SET                  "Set"
#define IPC_MSGTYPE_SUBSCRIBE            "Subscribe"
#define IPC_MSGTYPE_CHANGE_NOTIFICATION  "ChangeNotification"
#define IPC_MSGTYPE_EXECUTE_NOTIFICATION "ExecuteNotification"

// Server request message types:
#define IPC_MSGTYPE_LIST_CLIENTS         "ListClients"
#define IPC_MSGTYPE_WRITE                "Write"
#define IPC_MSGTYPE_READ                 "Read"
#define IPC_MSGTYPE_OBSERVE              "Observe"
#define IPC_MSGTYPE_EXECUTE              "Execute"
#define IPC_MSGTYPE_WRITE_ATTRIBUTES     "WriteAttributes"

// IPC message tags:
#define IPC_MSG_CREATE                      "Create"
#define IPC_MSG_SUBSCRIBE_TO_CHANGE         "SubscribeToChange"
#define IPC_MSG_SUBSCRIBE_TO_EXECUTE        "SubscribeToExecute"
#define IPC_MSG_CANCEL_SUBSCRIBE_TO_CHANGE  "CancelSubscribeToChange"
#define IPC_MSG_CANCEL_SUBSCRIBE_TO_EXECUTE "CancelSubscribeToExecute"

#define IPC_MSG_OBSERVE "Observe"
#define IPC_MSG_CANCEL_OBSERVATION "CancelObserve"



typedef struct _IPCInfo IPCInfo;
typedef struct _IPCChannel IPCChannel;
typedef struct _IPCMessage IPCMessage;

#define IPC_UDP_TIMEOUT (5 * 1000)  /* 5 Secs Timeout */

// Response codes, as specified by daemon
typedef enum
{
    IPCResponseCode_NotSet                  = 0,

    IPCResponseCode_Success                 = 200,
    IPCResponseCode_SuccessCreated          = 201,
    IPCResponseCode_SuccessDeleted          = 202,
    IPCResponseCode_SuccessChanged          = 204,
    IPCResponseCode_SuccessContent          = 205,

    IPCResponseCode_FailureBadRequest       = 400,
    IPCResponseCode_FailureUnAuthorized     = 401,
    IPCResponseCode_FailureForbidden        = 403,
    IPCResponseCode_FailureNotFound         = 404,
    IPCResponseCode_FailureMethodNotAllowed = 405,

    IPCResponseCode_ErrorOutOfMemory        = 999,

    // Inclusive range for responses that are considered successful:
    IPCResponseCode_SuccessLower            = IPCResponseCode_Success,
    IPCResponseCode_SuccessUpper            = 299,

} IPCResponseCode;

/**
 * @brief Allocate a new IPC Info instance based on the UDP method.
 * @param[in] address IPC server address or hostname.
 * @param[in] port IPC server port.
 * @return IpcInfo pointer if address and port are valid.
 * @return NULL if address and/or port is invalid.
 */
IPCInfo * IPCInfo_NewUDP(const char * address, unsigned short port);

/**
 * @brief Free memory allocated to the specified IpcInfo instance.
 * @param[in/out] ipcInfo Address of IPC Info instance pointer to be freed. Will be set to NULL.
 */
void IPCInfo_Free(IPCInfo ** ipcInfo);

// IPC Channels
IPCChannel * IPCChannel_New(const IPCInfo * ipcInfo);
void IPCChannel_Free(IPCChannel ** channel);

// IPC Messages
IPCMessage * IPCMessage_New(void);
void IPCMessage_Free(IPCMessage ** message);

InternalError IPCMessage_SetType(IPCMessage * message, const char * type, const char * subType);
InternalError IPCMessage_GetType(IPCMessage * message, const char ** type, const char ** subType);

IPCResponseCode IPCMessage_GetResponseCode(IPCMessage * message);
TreeNode IPCMessage_GetContentNode(IPCMessage * message);
AwaError IPCMessage_AddContent(IPCMessage * message, TreeNode content);
AwaError IPCMessage_RemoveContentNode(IPCMessage * message, TreeNode contentNode);

AwaError IPC_SendAndReceive(IPCChannel * channel, const IPCMessage * request, IPCMessage ** response, int32_t timeout);
AwaError IPC_SendAndReceiveOnNotifySocket(IPCChannel * channel, const IPCMessage * request, IPCMessage ** response, int32_t timeout);

AwaError IPC_WaitForNotification(IPCChannel * channel, int32_t timeout);
AwaError IPC_ReceiveNotification(IPCChannel * channel, IPCMessage ** notification);

IPCMessage * IPC_DeserialiseMessageFromXML(char * messageBuffer, size_t messageBufferLen);
char * IPC_SerialiseMessageToXML(const IPCMessage * message);

#ifdef __cplusplus
}
#endif

#endif // IPC_H


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

#ifndef IPC_DEFS_H
#define IPC_DEFS_H

#define IPC_DEFAULT_ADDRESS "127.0.0.1"
#define IPC_DEFAULT_CLIENT_PORT (12345)
#define IPC_DEFAULT_SERVER_PORT (54321)

#define IPC_MSGTYPE_REQUEST      "Request"
#define IPC_MSGTYPE_RESPONSE     "Response"
#define IPC_MSGTYPE_NOTIFICATION "Notification"

// Common request message types:
#define IPC_MSGTYPE_CONNECT              "Connect"
#define IPC_MSGTYPE_CONNECT_NOTIFY       "ConnectNotify"
#define IPC_MSGTYPE_DISCONNECT           "Disconnect"
#define IPC_MSGTYPE_DISCONNECT_NOTIFY    "DisconnectNotify"
#define IPC_MSGTYPE_DELETE               "Delete"
#define IPC_MSGTYPE_DEFINE               "Define"

// Client request message types:
#define IPC_MSGTYPE_GET                  "Get"
#define IPC_MSGTYPE_SET                  "Set"
#define IPC_MSGTYPE_SUBSCRIBE            "Subscribe"
#define IPC_MSGTYPE_CHANGE_NOTIFICATION  "ServerChange"
#define IPC_MSGTYPE_EXECUTE_NOTIFICATION "ServerExecute"

// Server request message types:
#define IPC_MSGTYPE_LIST_CLIENTS         "ListClients"
#define IPC_MSGTYPE_WRITE                "Write"
#define IPC_MSGTYPE_READ                 "Read"
#define IPC_MSGTYPE_OBSERVE              "Observe"
#define IPC_MSGTYPE_EXECUTE              "Execute"
#define IPC_MSGTYPE_WRITE_ATTRIBUTES     "WriteAttributes"
#define IPC_MSGTYPE_DISCOVER             "Discover"

// IPC message tags:
#define IPC_MSG_CREATE                      "Create"
#define IPC_MSG_SUBSCRIBE_TO_CHANGE         "SubscribeToChange"
#define IPC_MSG_SUBSCRIBE_TO_EXECUTE        "SubscribeToExecute"
#define IPC_MSG_CANCEL_SUBSCRIBE_TO_CHANGE  "CancelSubscribeToChange"
#define IPC_MSG_CANCEL_SUBSCRIBE_TO_EXECUTE "CancelSubscribeToExecute"

#define IPC_MSG_OBSERVE "Observe"
#define IPC_MSG_CANCEL_OBSERVATION "CancelObserve"

#endif // IPC_DEFS_H

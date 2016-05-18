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

// Contains data related to an IPC Session.

#ifndef LWM2M_SESSION_H
#define LWM2M_SESSION_H

#include <sys/types.h>
#include <sys/socket.h>

#include "../../api/src/ipc_defs.h"
#include "lwm2m_context.h"
#include "xmltree.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _IPCSession IPCSession;

void IPCSession_Init(void);

void IPCSession_Shutdown(void);

// Return 0 on success, -1 on error
int IPCSession_New(IPCSessionID sessionID);

// Return 0 on success, -1 on error
int IPCSession_AddRequestChannel(IPCSessionID sessionID, int sockfd, const struct sockaddr * fromAddr, int addrLen);
int IPCSession_GetRequestChannel(IPCSessionID sessionID, int * sockfd, const struct sockaddr ** fromAddr, int * addrLen);

// Return 0 on success, -1 on error
int IPCSession_AddNotifyChannel(IPCSessionID sessionID, int sockfd, const struct sockaddr * fromAddr, int addrLen);
int IPCSession_GetNotifyChannel(IPCSessionID sessionID, int * sockfd, const struct sockaddr ** fromAddr, int * addrLen);

IPCSessionID IPCSession_AssignSessionID(void);

bool IPCSession_IsValid(IPCSessionID sessionID);

void IPCSession_Dump(void);

#ifdef __cplusplus
}
#endif

#endif // LWM2M_SESSION_H

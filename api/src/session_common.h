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


#ifndef SESSION_COMMON_H
#define SESSION_COMMON_H

#include <stdbool.h>

#include "ipc.h"
#include "lwm2m_definition.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _SessionCommon SessionCommon;

typedef enum
{
    SessionType_Invalid = -1,
    SessionType_Client = 0,
    SessionType_Server,
} SessionType;

SessionCommon * SessionCommon_New(SessionType sessionType);

AwaError SessionCommon_Free(SessionCommon ** operation);

AwaError SessionCommon_SetIPCAsUDP(SessionCommon * session, const char * address, unsigned short port);

bool SessionCommon_HasIPCInfo(const SessionCommon * session);

AwaError SessionCommon_ConnectSession(SessionCommon * session);

AwaError SessionCommon_DisconnectSession(SessionCommon * session);

const AwaObjectDefinition * SessionCommon_GetObjectDefinition(const SessionCommon * session, AwaObjectID objectID);

AwaObjectDefinitionIterator * SessionCommon_NewObjectDefinitionIterator(const SessionCommon * session);

bool SessionCommon_IsConnected(const SessionCommon * session);

IPCChannel * SessionCommon_GetChannel(const SessionCommon * session);

DefinitionRegistry * SessionCommon_GetDefinitionRegistry(const SessionCommon * session);

AwaError SessionCommon_PathToIDs(const SessionCommon * session, const char * path, AwaObjectID * objectID, AwaObjectInstanceID * objectInstanceID, AwaResourceID * resourceID);

AwaError SessionCommon_CheckResourceTypeFromPath(const SessionCommon * session, const char * path, AwaResourceType expected);

const AwaResourceDefinition * SessionCommon_GetResourceDefinitionFromPath(const SessionCommon * session, const char * path);

bool SessionCommon_IsObjectDefined(const SessionCommon * session, AwaObjectID objectID);

AwaError SessionCommon_SendDefineMessage(const SessionCommon * session, const TreeNode objectDefinitionsNode, int32_t timeout);

SessionType SessionCommon_GetSessionType(const SessionCommon * session);

IPCSessionID SessionCommon_GetSessionID(const SessionCommon * session);

#ifdef __cplusplus
}
#endif

#endif // SESSION_COMMON_H


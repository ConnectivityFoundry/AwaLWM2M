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


#ifndef SERVER_OPERATION_H
#define SERVER_OPERATION_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "lwm2m_tree_node.h"
#include "lwm2m_definition.h"
#include "xml.h"
#include "error.h"
#include "server_session.h"
#include "operation_common.h"
#include "ipc_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _ServerOperation ServerOperation;

ServerOperation * ServerOperation_New(const AwaServerSession * session);
void ServerOperation_Free(ServerOperation ** operation);

const AwaServerSession * ServerOperation_GetSession(const ServerOperation * operation);

TreeNode ServerOperation_GetClientsTree(const ServerOperation * operation);
TreeNode ServerOperation_GetObjectsTree(const ServerOperation * operation, const char * clientID);
OperationCommon * ServerOperation_CreateOperationCommon(const ServerOperation * operation, const char * clientID);

AwaError ServerOperation_AddPath(ServerOperation * operation, const char * clientID, const char * path, TreeNode * resultNode);

OperationCommon * ServerOperation_GetOperationCommon(const ServerOperation * operation, const char * clientID);
OperationCommon * ServerOperation_GetDefaultClientOperation(const ServerOperation * operation);

const char * ServerOperation_GetClientIDFromClientNode(const TreeNode clientNode);

IPCSessionID ServerOperation_GetSessionID(const ServerOperation * operation);

#ifdef __cplusplus
}
#endif

#endif // SERVER_OPERATION_H


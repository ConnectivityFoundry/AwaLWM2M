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


#ifndef OPERATION_COMMON_H
#define OPERATION_COMMON_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "lwm2m_tree_node.h"
#include "lwm2m_definition.h"
#include "xml.h"
#include "error.h"
#include "client_session.h"
#include "server_session.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _OperationCommon OperationCommon;

// A generic Operation can hold a pointer to either a Client or a Server session.
// It is up to the caller to determine which it is before dereferencing it.
typedef void Session;

OperationCommon * OperationCommon_NewWithClientSession(const AwaClientSession * session);
OperationCommon * OperationCommon_NewWithServerSession(const AwaServerSession * session);
OperationCommon * OperationCommon_New(const Session * session, SessionType sessionType);
OperationCommon * OperationCommon_NewWithExistingObjectsTree(const Session * session, SessionType sessionType, TreeNode objectsTree);
void OperationCommon_Free(OperationCommon ** operation);

/**
 * @brief Return the session pointer. This must be correctly cast before dereferencing,
 *        so an optional pointer to a SessionType variable may be supplied, which will be
 *        populated with the session type, identifying the pointer's concrete type.
 * @param[in] operation Pointer to operation.
 * @param[out] sessionType Pointer to SessionType variable, used to identify concrete type of returned pointer.
 * @return Type-eliminated pointer to a Session.
 */
const Session * OperationCommon_GetSession(const OperationCommon * operation, SessionType * sessionType);

/**
 * @brief Return the Common session pointer, which is independent of Client or Server.
 * @param[in] operation Pointer to operation.
 * @return SessionCommon pointer.
 */
const SessionCommon * OperationCommon_GetSessionCommon(const OperationCommon * operation);

TreeNode OperationCommon_GetObjectsTree(const OperationCommon * operation);

AwaError OperationCommon_AddPathToObjectsTree(TreeNode objectsTree, const char * path, TreeNode * resultNode);
AwaError OperationCommon_AddPathV2(OperationCommon * operation, const char * path, TreeNode * resultNode);
AwaError OperationCommon_AddPathWithArrayRange(OperationCommon * operation, const char * path, AwaArrayIndex startIndex, AwaArrayLength indexCount);

IPCSessionID OperationCommon_GetSessionID(const OperationCommon * operation);

#ifdef __cplusplus
}
#endif

#endif // OPERATION_COMMON_H


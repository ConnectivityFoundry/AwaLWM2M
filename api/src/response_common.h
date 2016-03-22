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


#ifndef RESPONSE_COMMON_H
#define RESPONSE_COMMON_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "lwm2m_tree_node.h"
#include "lwm2m_definition.h"
#include "xml.h"
#include "error.h"
#include "arrays.h"
#include "operation_common.h"
#include "path_result.h"
#include "path_iterator.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _ResponseCommon ResponseCommon;

ResponseCommon * ResponseCommon_New(const OperationCommon * operation, TreeNode objectsNode);

// FIXME Currently The client is using the old xml spec and therefore can't be used with the new objects tree.
// This is a temporary workaround to prevent error messages when the response does not contain an 'Objects' node
ResponseCommon * ResponseCommon_NewClient(const OperationCommon * operation, TreeNode objectsNode);

AwaError ResponseCommon_Free(ResponseCommon ** response);

const OperationCommon * ResponseCommon_GetOperation(const ResponseCommon * response);

TreeNode ResponseCommon_GetObjectsNode(const ResponseCommon * response);

AwaError ResponseCommon_CheckForErrors(const ResponseCommon * response);

AwaError ResponseCommon_BuildValues(ResponseCommon * response);

AwaError ResponseCommon_BuildPathResults(ResponseCommon * response);

AwaError ResponseCommon_GetPathResult(const ResponseCommon * response, const char * path, const PathResult ** result);

PathIterator * ResponseCommon_NewPathIterator(const ResponseCommon * response);

AwaError ResponseCommon_GetValuePointer(const ResponseCommon * response, const char * path, const void ** value, size_t * valueSize, AwaResourceType resourceType, int resourceSize);
AwaError ResponseCommon_GetValuePointerWithNull(const ResponseCommon * response, const char * path, const void ** value, size_t * valueSize, AwaResourceType resourceType, int resourceSize);
AwaError ResponseCommon_GetValueAsObjectLink(const ResponseCommon * response, const char * path, AwaObjectLink * value);
AwaError ResponseCommon_GetValueAsOpaque(const ResponseCommon * response, const char * path, AwaOpaque * value);

bool ResponseCommon_HasValue(const ResponseCommon * response, const char * path);
bool ResponseCommon_ContainsPath(const ResponseCommon * response, const char * path);

#ifdef __cplusplus
}
#endif

#endif // RESPONSE_COMMON_H



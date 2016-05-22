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


#ifndef SERVER_RESPONSE_H
#define SERVER_RESPONSE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "lwm2m_tree_node.h"
#include "lwm2m_definition.h"
#include "xml.h"
#include "error.h"
#include "response_common.h"
#include "server_operation.h"
#include "client_iterator.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _ServerResponse ServerResponse;

/**
 * @brief Construct a new ServerResponse based on the given operation and supplied Clients node.
 * @param[in] clientsNode Clients node. Does not take ownership.
 */
ServerResponse * ServerResponse_New(const ServerOperation * operation, const TreeNode clientsNode);

ServerResponse * ServerResponse_NewFromServerOperation(const ServerOperation * serverOperation, const TreeNode clientsNode);

AwaError ServerResponse_Free(ServerResponse ** response);

const char * ServerResponse_GetNextClientID(const ServerResponse * response, const char * previousClientID);

const ResponseCommon * ServerResponse_GetClientResponse(const ServerResponse * response, const char * clientID);

AwaError ServerResponse_CheckForErrors(const ServerResponse * response);

ClientIterator * ServerResponse_NewClientIterator(const ServerResponse * response);


#ifdef __cplusplus
}
#endif

#endif // SERVER_RESPONSE_H


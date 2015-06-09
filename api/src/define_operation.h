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


#ifndef DEFINE_OPERATION_H
#define DEFINE_OPERATION_H

#include <stdbool.h>

#include "awa/error.h"
#include "awa/common.h"
#include "session_common.h"
#include "operation_common.h"
#include "xmltree.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _DefineOperation DefineOperation;

DefineOperation * DefineOperation_NewWithClientSession(const AwaClientSession * session);

DefineOperation * DefineOperation_NewWithServerSession(const AwaServerSession * session);

AwaError DefineOperation_Free(DefineOperation ** operation);

AwaError DefineOperation_Add(DefineOperation * operation, const AwaObjectDefinition * objectDefinition);

AwaError DefineOperation_SendDefineMessage(const SessionCommon * session, const TreeNode objectDefinitionsNode, int32_t timeout);

AwaError DefineOperation_Perform(DefineOperation * operation, AwaTimeout timeout);


#ifdef __cplusplus
}
#endif

#endif // DEFINE_OPERATION_H


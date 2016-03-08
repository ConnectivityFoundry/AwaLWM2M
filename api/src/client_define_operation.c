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


#include <stdbool.h>

#include "awa/client.h"
#include "client_session.h"
#include "memalloc.h"
#include "log.h"
#include "operation_common.h"
#include "define_operation.h"

struct _AwaClientDefineOperation
{
    DefineOperation * DefineOperation;
};

AwaClientDefineOperation * AwaClientDefineOperation_New(const AwaClientSession * session)
{
    AwaClientDefineOperation * operation = NULL;
    if (session != NULL)
    {
        operation = (AwaClientDefineOperation *)Awa_MemAlloc(sizeof(*operation));
        if (operation != NULL)
        {
            memset(operation, 0, sizeof(*operation));
            operation->DefineOperation = DefineOperation_NewWithClientSession(session);
            if (operation->DefineOperation != NULL)
            {
                LogNew("AwaClientDefineOperation", session);
            }
            else
            {
                LogErrorWithEnum(AwaError_OutOfMemory, "Could not create define operation");
                Awa_MemSafeFree(operation);
                operation = NULL;
            }
        }
        else
        {
            LogErrorWithEnum(AwaError_OutOfMemory);
        }
    }
    return operation;
}

AwaError AwaClientDefineOperation_Free(AwaClientDefineOperation ** operation)
{
    AwaError result = AwaError_Unspecified;

    if (operation != NULL && *operation != NULL)
    {
        DefineOperation_Free(&(*operation)->DefineOperation);

        LogFree("AwaClientDefineOperation", *operation);
        Awa_MemSafeFree(*operation);
        *operation = NULL;

        result = AwaError_Success;
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "Operation is NULL");
    }
    return result;
}

AwaError AwaClientDefineOperation_Add(AwaClientDefineOperation * operation, const AwaObjectDefinition * objectDefinition)
{
    AwaError result = AwaError_Unspecified;
    if (operation != NULL)
    {
        result = DefineOperation_Add(operation->DefineOperation, objectDefinition);
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid);
    }
    return result;
}

AwaError AwaClientDefineOperation_Perform(AwaClientDefineOperation * operation, AwaTimeout timeout)
{
    AwaError result = AwaError_Unspecified;
    if (operation != NULL)
    {
        result = DefineOperation_Perform(operation->DefineOperation, timeout);
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid);
    }
    return result;
}

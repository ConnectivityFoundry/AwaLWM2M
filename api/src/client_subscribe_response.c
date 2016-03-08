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


#include "lwm2m_tree_node.h"
#include "get_response.h"
#include "get_operation.h"
#include "response_common.h"
#include "client_session.h"
#include "path_iterator.h"
#include "utils.h"
#include "arrays.h"
#include "map.h"
#include "path_result.h"
#include "client_subscribe.h"
#include "objects_tree.h"

typedef struct _AwaArray AwaArray;

struct _AwaClientSubscribeResponse
{
    ResponseCommon * Common;
};

AwaClientSubscribeResponse * SubscribeResponse_New(AwaClientSubscribeOperation * operation, TreeNode objectsNode)
{
    AwaClientSubscribeResponse * response = NULL;

    response = Awa_MemAlloc(sizeof(*response));
    if (response != NULL)
    {
        memset(response, 0, sizeof(*response));

        response->Common = ResponseCommon_New(ClientSubscribeOperation_GetOperationCommon(operation), objectsNode);
        if (response->Common != NULL)
        {
            LogNew("AwaClientSubscribeResponse", response);
        }
        else
        {
            LogErrorWithEnum(AwaError_Internal, "Unable to initialise response");
            Awa_MemSafeFree(response);
            response = NULL;
        }
    }
    else
    {
        LogErrorWithEnum(AwaError_OutOfMemory);
    }
    return response;
}

AwaError SubscribeResponse_Free(AwaClientSubscribeResponse ** response)
{
    AwaError result = AwaError_OperationInvalid;
    if ((response != NULL) && (*response != NULL))
    {
        ResponseCommon_Free(&(*response)->Common);
        LogFree("AwaClientSubscribeResponse", *response);
        Awa_MemSafeFree(*response);
        *response = NULL;
        result = AwaError_Success;
    }
    return result;
}

AwaPathIterator * AwaClientSubscribeResponse_NewPathIterator(const AwaClientSubscribeResponse * response)
{
    PathIterator * iterator = NULL;
    if (response != NULL)
    {
        iterator = ResponseCommon_NewPathIterator(response->Common);
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "response is NULL");
    }
    // AwaPathIterator is an alias for PathIterator
    return (AwaPathIterator *)iterator;
}

const AwaPathResult * AwaClientSubscribeResponse_GetPathResult(const AwaClientSubscribeResponse * response, const char * path)
{
    const PathResult * pathResult = NULL;
    if (response != NULL)
    {
        ResponseCommon_GetPathResult(response->Common, path, &pathResult);
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "response is NULL");
    }
    // AwaPathResult is an alias for PathResult
    return (AwaPathResult *)pathResult;
}


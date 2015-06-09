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


#include "path_result.h"
#include "memalloc.h"
#include "log.h"
#include "lwm2m_xml_serdes.h"

// This struct is used for API type safety and is never instantiated.
// DO NOT USE THIS STRUCTURE!
struct _AwaPathResult {};

struct _PathResult
{
    AwaError Error;
    AwaLWM2MError ServerError;
};

AwaError AwaPathResult_GetError(const AwaPathResult * result)
{
    return PathResult_GetError((PathResult *)result);
}

AwaLWM2MError AwaPathResult_GetLWM2MError(const AwaPathResult * result)
{
    return PathResult_GetLWM2MError((PathResult *)result);
}

PathResult * PathResult_New(TreeNode resultNode)
{
    PathResult * result = Awa_MemAlloc(sizeof(*result));
    if (result != NULL)
    {
        memset(result, 0, sizeof(*result));
        if (resultNode != NULL)
        {
            const char * errorValue = (const char *)xmlif_GetOpaque(resultNode, "Result/Error");
            if (errorValue != NULL)
            {
                result->Error = AwaError_FromString(errorValue);
            }
            else
            {
                LogErrorWithEnum(AwaError_Internal, "No <Error> in result node");
                result->Error = AwaError_Unspecified;
            }

            if (result->Error == AwaError_LWM2MError)
            {
                const char * serverErrorValue = (const char *)xmlif_GetOpaque(resultNode, "Result/LWM2MError");
                if (serverErrorValue != NULL)
                {
                    result->ServerError = AwaLWM2MError_FromString(serverErrorValue);
                }
                else
                {
                    LogErrorWithEnum(AwaError_Internal, "No <LWM2MError> in result node");
                    result->ServerError = AwaLWM2MError_Unspecified;
                }
            }
            else
            {
                result->ServerError = AwaLWM2MError_Unspecified;
            }
        }
        else
        {
            // if there's no resultNode available, then the error is unspecified
            result->Error = AwaError_Unspecified;
        }
        LogNew("PathResult", result);
    }
    else
    {
        LogErrorWithEnum(AwaError_OutOfMemory);
    }
    return result;
}

void PathResult_Free(PathResult ** result)
{
    if ((result != NULL) && (*result != NULL))
    {
        LogFree("PathResult", *result);
        Awa_MemSafeFree(*result);
        *result = NULL;
    }
}

AwaError PathResult_GetError(const PathResult * result)
{
    AwaError error = AwaError_Unspecified;
    if (result != NULL)
    {
        error = result->Error;
    }
    else
    {
        error = LogErrorWithEnum(AwaError_Unspecified, "result is NULL");
    }
    return error;
}

AwaLWM2MError PathResult_GetLWM2MError(const PathResult * result)
{
    AwaLWM2MError error = AwaLWM2MError_Unspecified;
    if (result != NULL)
    {
        error = result->ServerError;
    }
    else
    {
        error = LogErrorWithEnum(AwaLWM2MError_Unspecified, "result is NULL");
    }
    return error;
}


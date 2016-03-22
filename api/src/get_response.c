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

#include "operation_common.h"
#include "response_common.h"
#include "get_response.h"
#include "get_operation.h"
#include "client_session.h"
#include "path_iterator.h"
#include "utils.h"
#include "arrays.h"

bool AwaClientGetResponse_HasValue(const AwaClientGetResponse * response, const char * path)
{
    bool hasValue = false;

    if (response != NULL)
    {
        // AwaClientGetResponse is an alias for ResponseCommon
        hasValue = ResponseCommon_HasValue((const ResponseCommon *)response, path);
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "Get Response is NULL");
    }
    return hasValue;
}

bool AwaClientGetResponse_ContainsPath(const AwaClientGetResponse * response, const char * path)
{
    bool containsPath = false;

    if (response != NULL)
    {
        // AwaClientGetResponse is an alias for ResponseCommon
        containsPath = ResponseCommon_ContainsPath((const ResponseCommon *)response, path);
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "Get Response is NULL");
    }
    return containsPath;
}

const AwaPathResult * AwaClientGetResponse_GetPathResult(const AwaClientGetResponse * response, const char * path)
{
    // AwaServerGetResponse is an alias for ResponseCommon
    const PathResult * pathResult = NULL;
    ResponseCommon_GetPathResult((const ResponseCommon *)response, path, &pathResult);
    // AwaPathResult is an alias for PathResult
    return (AwaPathResult *)pathResult;
}

AwaError AwaClientGetResponse_GetValueAsIntegerPointer(const AwaClientGetResponse * response, const char * path, const AwaInteger ** value)
{
    // AwaClientGetResponse is an alias for ResponseCommon
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)value, NULL, AwaResourceType_Integer, sizeof(AwaInteger));
}

AwaError AwaClientGetResponse_GetValueAsCStringPointer(const AwaClientGetResponse * response, const char * path, const char ** value)
{
    // AwaClientGetResponse is an alias for ResponseCommon
    return ResponseCommon_GetValuePointerWithNull((const ResponseCommon *)response, path, (const void **)value, NULL, AwaResourceType_String, -1);
}

AwaError AwaClientGetResponse_GetValueAsFloatPointer(const AwaClientGetResponse * response, const char * path, const AwaFloat ** value)
{
    // AwaClientGetResponse is an alias for ResponseCommon
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)value, NULL, AwaResourceType_Float, sizeof(AwaFloat));
}

AwaError AwaClientGetResponse_GetValueAsTimePointer(const AwaClientGetResponse * response, const char * path, const AwaTime ** value)
{
    // AwaClientGetResponse is an alias for ResponseCommon
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)value, NULL, AwaResourceType_Time, sizeof(AwaTime));
}

AwaError AwaClientGetResponse_GetValueAsBooleanPointer(const AwaClientGetResponse * response, const char * path, const AwaBoolean ** value)
{
    // AwaClientGetResponse is an alias for ResponseCommon
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)value, NULL, AwaResourceType_Boolean, sizeof(AwaBoolean));
}

AwaError AwaClientGetResponse_GetValueAsOpaque(const AwaClientGetResponse * response, const char * path, AwaOpaque * value)
{
    // AwaClientGetResponse is an alias for ResponseCommon
    return ResponseCommon_GetValueAsOpaque((const ResponseCommon *)response, path, value);
}

AwaError AwaClientGetResponse_GetValueAsObjectLink(const AwaClientGetResponse * response, const char * path, AwaObjectLink * value)
{
    // AwaClientGetResponse is an alias for ResponseCommon
    return ResponseCommon_GetValueAsObjectLink((const ResponseCommon *)response, path, value);
}

AwaError AwaClientGetResponse_GetValueAsObjectLinkPointer(const AwaClientGetResponse * response, const char * path, const AwaObjectLink ** value)
{
    // AwaClientGetResponse is an alias for ResponseCommon
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)value, NULL, AwaResourceType_ObjectLink, sizeof(AwaObjectLink));
}

AwaError AwaClientGetResponse_GetValueAsOpaquePointer    (const AwaClientGetResponse * response, const char * path, const AwaOpaque ** value)
{
    // AwaClientGetResponse is an alias for ResponseCommon
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)value, NULL, AwaResourceType_Opaque, sizeof(AwaOpaque));
}

AwaError AwaClientGetResponse_GetValuesAsStringArrayPointer(const AwaClientGetResponse * response, const char * path, const AwaStringArray ** valueArray)
{
    // AwaClientGetResponse is an alias for ResponseCommon
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)valueArray, NULL, AwaResourceType_StringArray, sizeof(AwaStringArray *));
}

AwaError AwaClientGetResponse_GetValuesAsIntegerArrayPointer(const AwaClientGetResponse * response, const char * path, const AwaIntegerArray ** valueArray)
{
    // AwaClientGetResponse is an alias for ResponseCommon
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)valueArray, NULL, AwaResourceType_IntegerArray, sizeof(AwaIntegerArray *));
}

AwaError AwaClientGetResponse_GetValuesAsFloatArrayPointer(const AwaClientGetResponse * response, const char * path, const AwaFloatArray ** valueArray)
{
    // AwaClientGetResponse is an alias for ResponseCommon
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)valueArray, NULL, AwaResourceType_FloatArray, sizeof(AwaFloatArray *));
}

AwaError AwaClientGetResponse_GetValuesAsBooleanArrayPointer(const AwaClientGetResponse * response, const char * path, const AwaBooleanArray ** valueArray)
{
    // AwaClientGetResponse is an alias for ResponseCommon
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)valueArray, NULL, AwaResourceType_BooleanArray, sizeof(AwaBooleanArray *));
}

AwaError AwaClientGetResponse_GetValuesAsOpaqueArrayPointer(const AwaClientGetResponse * response, const char * path, const AwaOpaqueArray ** valueArray)
{
    // AwaClientGetResponse is an alias for ResponseCommon
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)valueArray, NULL, AwaResourceType_OpaqueArray, sizeof(AwaOpaqueArray *));
}

AwaError AwaClientGetResponse_GetValuesAsTimeArrayPointer(const AwaClientGetResponse * response, const char * path, const AwaTimeArray ** valueArray)
{
    // AwaClientGetResponse is an alias for ResponseCommon
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)valueArray, NULL, AwaResourceType_TimeArray, sizeof(AwaTimeArray *));
}

AwaError AwaClientGetResponse_GetValuesAsObjectLinkArrayPointer(const AwaClientGetResponse * response, const char * path, const AwaObjectLinkArray ** valueArray)
{
    // AwaClientGetResponse is an alias for ResponseCommon
    return ResponseCommon_GetValuePointer((const ResponseCommon *)response, path, (const void **)valueArray, NULL, AwaResourceType_ObjectLinkArray, sizeof(AwaObjectLinkArray *));
}

AwaPathIterator * AwaClientGetResponse_NewPathIterator(const AwaClientGetResponse * response)
{
    PathIterator * iterator = NULL;
    if (response != NULL)
    {
        // AwaClientGetResponse is an alias for ResponseCommon
        iterator = ResponseCommon_NewPathIterator((const ResponseCommon *)response);
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "Get Response is NULL");
    }
    // AwaPathIterator is an alias for PathIterator
    return (AwaPathIterator *)iterator;
}

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


#include "awa/client.h"
#include "awa/server.h"
#include "log.h"

AwaError AwaClientSession_Refresh(AwaClientSession * session)
{
  AwaError result = LogErrorWithEnum(AwaError_Unsupported);
  return result;
}

AwaError AwaServerSession_Refresh(AwaServerSession * session)
{
    return LogErrorWithEnum(AwaError_Unsupported);
}

const AwaServerDefineResponse * AwaServerDefineOperation_GetResponse(const AwaServerDefineOperation * operation)
{
    LogErrorWithEnum(AwaError_Unsupported);
    return NULL;
}

AwaPathIterator * AwaServerDefineResponse_NewPathIterator(const AwaServerDefineResponse * response)
{
    LogErrorWithEnum(AwaError_Unsupported);
    return NULL;
}

const AwaPathResult * AwaServerDefineResponse_GetPathResult(const AwaServerDefineResponse * response, const char * path)
{
    LogErrorWithEnum(AwaError_Unsupported);
    return NULL;
}

AwaError AwaServerWriteOperation_SetObjectInstanceWriteMode(AwaServerWriteOperation * operation, const char * path, AwaWriteMode mode)
{
    return LogErrorWithEnum(AwaError_Unsupported);
}

AwaError AwaServerWriteOperation_SetResourceWriteMode(AwaServerWriteOperation * operation, const char * path, AwaWriteMode mode)
{
    return LogErrorWithEnum(AwaError_Unsupported);
}

AwaError AwaServerWriteOperation_AddArrayValueAsCString(AwaServerWriteOperation * operation, const char * path, int resourceInstanceID, const char * value)
{
    return LogErrorWithEnum(AwaError_Unsupported);
}

AwaError AwaServerWriteOperation_AddArrayValueAsInteger(AwaServerWriteOperation * operation, const char * path, int resourceInstanceID, AwaInteger value)
{
    return LogErrorWithEnum(AwaError_Unsupported);
}

AwaError AwaServerWriteOperation_AddArrayValueAsFloat(AwaServerWriteOperation * operation, const char * path, int resourceInstanceID, AwaFloat value)
{
    return LogErrorWithEnum(AwaError_Unsupported);
}

AwaError AwaServerWriteOperation_AddArrayValueAsBoolean(AwaServerWriteOperation * operation, const char * path, int resourceInstanceID, AwaBoolean value)
{
    return LogErrorWithEnum(AwaError_Unsupported);
}

AwaError AwaServerWriteOperation_AddArrayValueAsTime(AwaServerWriteOperation * operation, const char * path, int resourceInstanceID, AwaTime value)
{
    return LogErrorWithEnum(AwaError_Unsupported);
}

AwaError AwaServerWriteOperation_AddArrayValueAsOpaque(AwaServerWriteOperation * operation, const char * path, int resourceInstanceID, AwaOpaque value)
{
    return LogErrorWithEnum(AwaError_Unsupported);
}

AwaError AwaServerWriteOperation_AddArrayValueAsObjectLink(AwaServerWriteOperation * operation, const char * path, int resourceInstanceID, AwaObjectLink value)
{
    return LogErrorWithEnum(AwaError_Unsupported);
}

AwaServerDiscoverOperation * AwaServerDiscoverOperation_New(const AwaServerSession * session)
{
    LogErrorWithEnum(AwaError_Unsupported);
    return NULL;
}

AwaError AwaServerDiscoverOperation_AddPath(AwaServerDiscoverOperation * operation, const char * clientID, const char * path)
{
    return LogErrorWithEnum(AwaError_Unsupported);
}

AwaError AwaServerDiscoverOperation_Perform(AwaServerDiscoverOperation * operation, AwaTimeout timeout)
{
    return LogErrorWithEnum(AwaError_Unsupported);
}

AwaError AwaServerDiscoverOperation_Free(AwaServerDiscoverOperation ** operation)
{
    return LogErrorWithEnum(AwaError_Unsupported);
}

AwaClientIterator * AwaServerDiscoverOperation_NewClientIterator(const AwaServerDiscoverOperation * operation)
{
    LogErrorWithEnum(AwaError_Unsupported);
    return NULL;
}

const AwaServerDiscoverResponse * AwaServerDiscoverOperation_GetResponse(const AwaServerDiscoverOperation * operation, const char * clientID)
{
    LogErrorWithEnum(AwaError_Unsupported);
    return NULL;
}

AwaPathIterator * AwaServerDiscoverResponse_NewPathIterator(const AwaServerDiscoverResponse * response)
{
    LogErrorWithEnum(AwaError_Unsupported);
    return NULL;
}

const AwaPathResult * AwaServerDiscoverResponse_GetPathResult(const AwaServerDiscoverResponse * response, const char * path)
{
    LogErrorWithEnum(AwaError_Unsupported);
    return NULL;
}

bool AwaServerDiscoverResponse_ContainsPath(const AwaServerDiscoverResponse * response, const char * path)
{
    LogErrorWithEnum(AwaError_Unsupported);
    return false;
}

AwaAttributeIterator * AwaServerDiscoverResponse_NewAttributeIterator(const AwaServerDiscoverResponse * response, const char * path)
{
    LogErrorWithEnum(AwaError_Unsupported);
    return NULL;
}

bool AwaAttributeIterator_Next(AwaAttributeIterator * iterator)
{
    LogErrorWithEnum(AwaError_Unsupported);
    return false;
}

const char * AwaAttributeIterator_GetLink(const AwaAttributeIterator * iterator)
{
    LogErrorWithEnum(AwaError_Unsupported);
    return NULL;
}

void AwaAttributeIterator_Free(AwaAttributeIterator ** iterator)
{
    LogErrorWithEnum(AwaError_Unsupported);
}

AwaError AwaServerDiscoverResponse_GetAttributeValueAsIntegerPointer(const AwaServerDiscoverResponse * response, const char * path, const char * link, const AwaInteger ** value)
{
    return LogErrorWithEnum(AwaError_Unsupported);
}

AwaError AwaServerDiscoverResponse_GetAttributeValueAsFloatPointer  (const AwaServerDiscoverResponse * response, const char * path, const char * link, const AwaFloat ** value)
{
    return LogErrorWithEnum(AwaError_Unsupported);
}

AwaClientIterator * AwaServerObserveOperation_NewClientIterator(const AwaServerObserveOperation * operation)
{
    LogErrorWithEnum(AwaError_Unsupported);
    return NULL;
}

bool AwaServerDefineResponse_ContainsPath(const AwaServerDefineResponse * response, const char * path)
{
    LogErrorWithEnum(AwaError_Unsupported);
    return NULL;
}

bool AwaServerWriteResponse_ContainsPath(const AwaServerWriteResponse * response, const char * path)
{
    LogErrorWithEnum(AwaError_Unsupported);
    return NULL;
}

bool AwaServerDeleteResponse_ContainsPath(const AwaServerDeleteResponse * response, const char * path)
{
    LogErrorWithEnum(AwaError_Unsupported);
    return NULL;
}

bool AwaServerObserveResponse_ContainsPath(const AwaServerObserveResponse * response, const char * path)
{
    LogErrorWithEnum(AwaError_Unsupported);
    return NULL;
}


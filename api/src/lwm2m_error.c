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


#include "lwm2m_error.h"

#include <stddef.h>

#include "awa/common.h"
#include <string.h>

// This table must align with awa/common.h:AwaLWM2MError
static const char * ServerErrorStrings[] =
{
    "AwaLWM2MError_Success",
    "AwaLWM2MError_Unspecified",
    "AwaLWM2MError_BadRequest",
    "AwaLWM2MError_Unauthorized",
    "AwaLWM2MError_NotFound",
    "AwaLWM2MError_MethodNotAllowed",
    "AwaLWM2MError_NotAcceptable",
    "AwaLWM2MError_Timeout",
};

const char * AwaLWM2MError_ToString(AwaLWM2MError error)
{
    return LWM2MError_ToString(error);
}

AwaLWM2MError AwaLWM2MError_FromString(const char * errorString)
{
    return LWM2MError_FromString(errorString);
}

const char * LWM2MError_ToString(AwaLWM2MError error)
{
    static const char * result = "Unknown ServerError";
    size_t numEntries = LWM2MError_GetNumberOfServerErrorStrings();

    if (error >= 0 && error < numEntries)
    {
        result = ServerErrorStrings[error];
    }
    return result;
}

AwaLWM2MError LWM2MError_FromString(const char * errorString)
{
    int i;
    for (i = 0; i < AwaLWM2MError_LAST; i++)
    {
        if (strcmp(ServerErrorStrings[i], errorString) == 0)
        {
            return i;
        }
    }
    return AwaLWM2MError_LAST;
}

AwaLWM2MError LWM2MError_FromCoapResponseCode(int responseCode)
{
    AwaLWM2MError error = AwaLWM2MError_Unspecified;
    switch(responseCode)
    {
    case 400:
        error = AwaLWM2MError_BadRequest;
        break;
    case 401:
        error = AwaLWM2MError_Unauthorized;
        break;
    case 404:
        error = AwaLWM2MError_NotFound;
        break;
    case 405:
        error = AwaLWM2MError_MethodNotAllowed;
        break;
    case 504:
        error = AwaLWM2MError_Timeout;
        break;
    default:
        break;
    }
    return error;
}

size_t LWM2MError_GetNumberOfServerErrorStrings(void)
{
    size_t numEntries = sizeof(ServerErrorStrings) / sizeof(ServerErrorStrings[0]);
    return numEntries;
}


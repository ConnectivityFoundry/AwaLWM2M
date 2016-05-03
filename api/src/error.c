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


#include <stddef.h>
#include <string.h>

#include "awa/common.h"
#include "error.h"

// This table must align with awa/common.h:AwaError
static const char * ErrorStrings[] =
{
    "AwaError_Success",
    "AwaError_Unspecified",
    "AwaError_Unsupported",
    "AwaError_Internal",
    "AwaError_OutOfMemory",

    // 5
    "AwaError_SessionInvalid",
    "AwaError_SessionNotConnected",
    "AwaError_NotDefined",
    "AwaError_AlreadyDefined",
    "AwaError_OperationInvalid",

    // 10
    "AwaError_PathInvalid",
    "AwaError_PathNotFound",
    "AwaError_TypeMismatch",
    "AwaError_Timeout",
    "AwaError_Overrun",

    // 15
    "AwaError_IDInvalid",
    "AwaError_AddInvalid",
    "AwaError_CannotCreate",
    "AwaError_CannotDelete",
    "AwaError_DefinitionInvalid",

    // 20
    "AwaError_AlreadySubscribed",
    "AwaError_SubscriptionInvalid",
    "AwaError_ObservationInvalid",
    "AwaError_IPCError",
    "AwaError_ResponseInvalid",

    // 25
    "AwaError_ClientIDInvalid",
    "AwaError_ClientNotFound",
    "AwaError_LWM2MError",
    "AwaError_IteratorInvalid",
    "AwaError_Response",

    // 30
    "AwaError_RangeInvalid",

    // 31
    "AwaError_StaticClientInvalid",
    "AwaError_StaticClientNotConfigured",
    "AwaError_StaticClientNotInitialized",
    "AwaError_LogLevelInvalid",
};

const char * AwaError_ToString(AwaError error)
{
    return Error_ToString(error);
}

AwaError AwaError_FromString(const char * errorString)
{
    return Error_FromString(errorString);
}

const char * Error_ToString(AwaError error)
{
    static const char * result = "Unknown Error";
    size_t numEntries = Error_GetNumberOfErrorStrings();

    if (error >= 0 && error < numEntries)
    {
        result = ErrorStrings[error];
    }
    return result;
}

AwaError Error_FromString(const char * errorString)
{
    AwaError i, error = AwaError_LAST;
    for (i = (AwaError)0; i < AwaError_LAST; ++i)
    {
        if (strcmp(ErrorStrings[i], errorString) == 0)
        {
            error = i;
            break;
        }
    }
    return error;
}

size_t Error_GetNumberOfErrorStrings(void)
{
    return sizeof(ErrorStrings) / sizeof(ErrorStrings[0]);
}


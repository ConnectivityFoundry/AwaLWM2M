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


#include "lwm2m_result.h"
#include "lwm2m_debug.h"
#include <awa/error.h>

static AwaLwm2mResult lastResult = AwaLwm2mResult_Success;

AwaLwm2mResult AwaLwm2mResult_GetLastResult(void)
{
    return lastResult;
}

void AwaLwm2mResult_SetResult(AwaLwm2mResult result)
{
    lastResult = result;
}

bool AwaLwm2mResult_IsSuccess(AwaLwm2mResult result)
{
    return result >= 200 && result < 300;
}

int AwaLwm2mResult_ToAwaError(AwaLwm2mResult result, int defaultError)
{
    switch (result)
    {
    case AwaLwm2mResult_Success: 
    case AwaLwm2mResult_SuccessChanged:
    case AwaLwm2mResult_SuccessContent: 
    case AwaLwm2mResult_SuccessCreated:
    case AwaLwm2mResult_SuccessDeleted:
        return AwaError_Success;
    case AwaLwm2mResult_NotFound:
        return AwaError_PathNotFound;
    case AwaLwm2mResult_MethodNotAllowed:
        return AwaError_PathInvalid;
    default:
        return defaultError;
    }
}

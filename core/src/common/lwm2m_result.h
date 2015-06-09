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


#ifndef LWM2M_RESULT_H
#define LWM2M_RESULT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "lwm2m_types.h"
#include "lwm2m_list.h"
#include "lwm2m_observers.h"

// LWM2M Core result codes
typedef enum
{
    Lwm2mResult_Success = 200,
    Lwm2mResult_SuccessCreated = 201,
    Lwm2mResult_SuccessDeleted = 202,
    Lwm2mResult_SuccessChanged = 204,
    Lwm2mResult_SuccessContent = 205,

    Lwm2mResult_BadRequest = 400,
    Lwm2mResult_Unauthorized = 401,
    Lwm2mResult_Forbidden = 403,
    Lwm2mResult_NotFound = 404,
    Lwm2mResult_MethodNotAllowed = 405,

    Lwm2mResult_InternalError = 500,

    Lwm2mResult_OutOfMemory = 999,
    Lwm2mResult_AlreadyRegistered,
    Lwm2mResult_MismatchedRegistration,
    Lwm2mResult_AlreadyCreated,
    Lwm2mResult_Unsupported,
    Lwm2mResult_Unspecified = -1,

} Lwm2mResult;

// Returns the last result code of an lwm2m related function
Lwm2mResult Lwm2mResult_GetLastResult(void);

// Retrieves the last result code of an lwm2m related function
void Lwm2mResult_SetResult(Lwm2mResult result);

bool Lwm2mResult_IsSuccess(Lwm2mResult result);

// Required for returning errors to the API through the IPC
int Lwm2mResult_ToAwaError(Lwm2mResult result, int defaultError);

#ifdef __cplusplus
}
#endif

#endif

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


#ifndef LWM2M_ENDPOINTS_H
#define LWM2M_ENDPOINTS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "lwm2m_list.h"
#include "lwm2m_debug.h"
#include "lwm2m_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*EndpointHandlerFunction)(int type, void * ctxt, AddressType * addr, const char * path, const char * query, const char * token,
                                       int tokenLength, AwaContentType contentType, const char * requestContent, size_t requestContentLen,
                                       AwaContentType * responseContentType, char * responseContent, size_t * responseContentLen, int * responseCode);
typedef struct
{
    struct ListHead     list;  // Next/Prev pointers
    char * Root;               // "/" by default, or "/lwm2m"
    char * Path;
    EndpointHandlerFunction Handler;

} ResourceEndPoint;

typedef struct
{
    struct ListHead EndPoint;

} ResourceEndPointList;

int Lwm2mEndPoint_InitEndPointList(ResourceEndPointList * endPointList);
int Lwm2mEndPoint_DestroyEndPointList(ResourceEndPointList * endPointList);

ResourceEndPoint * Lwm2mEndPoint_FindResourceEndPoint(ResourceEndPointList * endPointList, const char * path);
ResourceEndPoint * Lwm2mEndPoint_FindResourceEndPointAncestors(ResourceEndPointList * endPointList, const char * path);

int Lwm2mEndPoint_AddResourceEndPoint(ResourceEndPointList * endPointList, const char * path, EndpointHandlerFunction handler);
int Lwm2mEndPoint_RemoveResourceEndPoint(ResourceEndPointList * endPointList, const char * path);

#ifdef __cplusplus
}
#endif

#endif // LWM2M_ENDPOINTS_H

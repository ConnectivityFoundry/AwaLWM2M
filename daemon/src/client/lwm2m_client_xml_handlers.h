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


#ifndef LWM2M_CLIENT_XML_HANDLER_H
#define LWM2M_CLIENT_XML_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <xmltree.h>

#include "lwm2m_object_store.h"
#include "lwm2m_util.h"
#include "lwm2m_xml_interface.h"
#include "../../api/src/ipc_defs.h"

typedef struct
{
    struct ListHead List;
    ObjectIDType ObjectID;
    ObjectInstanceIDType ObjectInstanceID;
    ResourceIDType ResourceID;
    RequestInfoType ExecuteTarget;
    IPCSessionID SessionID;
} ExecuteHandlerType;


int xmlif_ExecuteResourceHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID,
                                 uint8_t * inValueBuffer, size_t inValueBufferLen);

int xmlif_CreateOptionalResourceHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);

int xmlif_AddExecuteHandler(RequestInfoType * request, ObjectInstanceResourceKey * key);

ExecuteHandlerType * xmlif_GetExecuteHandler(ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);

void xmlif_RegisterHandlers(void);

void xmlif_DestroyExecuteHandlers(void);

DefinitionCount xmlif_ParseObjDefDeviceServerXml(Lwm2mContextType * context, TreeNode content);

#ifdef __cplusplus
}
#endif

#endif // LWM2M_CLIENT_XML_HANDLER_H

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


#ifndef LWM2M_CORE_H
#define LWM2M_CORE_H

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "lwm2m_context.h"
#include "lwm2m_list.h"
#include "coap_abstraction.h"
#include "lwm2m_types.h"
#include "lwm2m_debug.h"
#include "lwm2m_util.h"
#include "lwm2m_object_store.h"
#include "lwm2m_attributes.h"
#include "lwm2m_definition.h"
#include "lwm2m_endpoints.h"
#include "lwm2m_request_origin.h"
#include "lwm2m_observers.h"

#ifdef __cplusplus
extern "C" {
#endif

Lwm2mContextType * Lwm2mCore_Init(CoapInfo * coap, AwaContentType contentType);

// Update the LWM2M state machine, process any message timeouts, registration attempts etc.
int Lwm2mCore_Process(Lwm2mContextType * context);

int Lwm2mCore_GetEndPointClientName(Lwm2mContextType * context, char * buffer, int len);

void Lwm2mCore_GetObjectList(Lwm2mContextType * context, char * altPath, char * buffer, int len, bool updated);

// Called by the server on shutdown, clean up any resources
void Lwm2mCore_Destroy(Lwm2mContextType * context);

// The following functions are called by other parts of the system to "Operate" on a resource
int Lwm2mCore_GetResourceInstanceValue(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID,
                                       ResourceInstanceIDType resourceInstanceID, const void ** Value, size_t * ValueBufferSize);

int Lwm2mCore_GetResourceInstanceLength(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID);
int Lwm2mCore_GetResourceInstanceCount(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);

int Lwm2mCore_SetResourceInstanceValue(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID,
                                       ResourceInstanceIDType resourceInstanceID, const void * Value, size_t ValueSize);

int Lwm2mCore_CreateObjectInstance(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID);
int Lwm2mCore_CreateOptionalResource(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);

int Lwm2mCore_RegisterObjectType(Lwm2mContextType * context, const char * objName, ObjectIDType objectID, uint16_t MaximumInstances,
                                 uint16_t MinimumInstances, ObjectOperationHandlers * Handlers);


int Lwm2mCore_RegisterResourceTypeWithDefaultValue(Lwm2mContextType * context, const char * resName, ObjectIDType objectID, ResourceIDType resourceID, AwaResourceType resourceType,
                                                   uint16_t MaximumInstances, uint16_t MinimumInstances, AwaResourceOperations operations,
                                                   ResourceOperationHandlers * Handlers, Lwm2mTreeNode * defaultValueNode);
int Lwm2mCore_RegisterResourceType(Lwm2mContextType * context, const char * resName, ObjectIDType objectID, ResourceIDType resourceID, AwaResourceType resourceType,
                                   uint16_t MaximumInstances, uint16_t MinimumInstances, AwaResourceOperations operations,
                                   ResourceOperationHandlers * Handlers);

ObjectInstanceIDType Lwm2mCore_GetNextObjectInstanceID(Lwm2mContextType * context, ObjectIDType  objectID, ObjectInstanceIDType objectInstanceID);
ResourceIDType Lwm2mCore_GetNextResourceID(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);
ResourceInstanceIDType Lwm2mCore_GetNextResourceInstanceID(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID);


int Lwm2mCore_AddResourceEndPoint(Lwm2mContextType * context, const char * path, EndpointHandlerFunction handler);
int Lwm2mCore_RemoveResourceEndPoint(Lwm2mContextType * context, const char * path);

DefinitionRegistry * Lwm2mCore_GetDefinitions(Lwm2mContextType * context);

struct ListHead * Lwm2mCore_GetClientList(Lwm2mContextType * context);
AwaContentType Lwm2mCore_GetContentType(Lwm2mContextType * context);
int Lwm2mCore_GetLastLocation(Lwm2mContextType * context);
struct ListHead * Lwm2mCore_GetEventRecordList(Lwm2mContextType * context);
void Lwm2mCore_SetLastLocation(Lwm2mContextType * context, int location);

#ifdef __cplusplus
}
#endif

#endif // LWM2M_CORE_H

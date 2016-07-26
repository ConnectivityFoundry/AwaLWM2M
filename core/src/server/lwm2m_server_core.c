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


#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "lwm2m_list.h"
#include "lwm2m_endpoints.h"
#include "coap_abstraction.h"
#include "lwm2m_types.h"
#include "lwm2m_debug.h"
#include "lwm2m_util.h"
#include "lwm2m_object_store.h"
#include "lwm2m_core.h"
#include "lwm2m_result.h"
#include "server/lwm2m_registration.h"

struct _Lwm2mContextType
{
    ObjectStore * Store;                      // Object store associated with this context
    DefinitionRegistry * Definitions;
    ResourceEndPointList EndPointList;        // CoAP endpoints
    CoapInfo * Coap;                          // CoAP library context information
    struct ListHead ClientList;               // List of registered clients
    int LastLocation;                         // Used for registration, creates /rd/0, /rd/1 etc
    AwaContentType ContentType;                  // Used to set CoAP content type
    struct ListHead EventRecordList;          // Used to dispatch event callbacks
};

static Lwm2mContextType Lwm2mContext;

//Dummy function to keep the linker happy.
int Lwm2mCore_CreateObjectInstance(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID)
{
    return 0;
}

int Lwm2mCore_RegisterObjectType(Lwm2mContextType * context, const char * objName, ObjectIDType objectID,
                                 uint16_t maximumInstances, uint16_t minimumInstances, ObjectOperationHandlers * handlers)
{
    return Definition_RegisterObjectType(context->Definitions, objName, objectID, maximumInstances, minimumInstances, handlers);
}

int Lwm2mCore_RegisterResourceTypeWithDefaultValue(Lwm2mContextType * context, const char * resName, ObjectIDType objectID, ResourceIDType resourceID,
                                                   AwaResourceType resourceType, uint16_t maximumInstances, uint16_t minimumInstances,
                                                   AwaResourceOperations operations, ResourceOperationHandlers * handlers, Lwm2mTreeNode * defaultValueNode)
{
    return Definition_RegisterResourceType(context->Definitions, resName, objectID, resourceID, resourceType,
                                           maximumInstances, minimumInstances, operations, handlers, defaultValueNode);
}

int Lwm2mCore_RegisterResourceType(Lwm2mContextType * context, const char * resName, ObjectIDType objectID,
                                   ResourceIDType resourceID, AwaResourceType resourceType, uint16_t maximumInstances,
                                   uint16_t minimumInstances, AwaResourceOperations operations, ResourceOperationHandlers * handlers)
{
    return Definition_RegisterResourceType(context->Definitions, resName, objectID, resourceID,
                                           resourceType, maximumInstances, minimumInstances, operations, handlers, NULL);
}

ObjectInstanceIDType Lwm2mCore_GetNextObjectInstanceID(Lwm2mContextType * context, ObjectIDType  objectID, ObjectInstanceIDType objectInstanceID)
{
    return ObjectStore_GetNextObjectInstanceID(context->Store, objectID, objectInstanceID);
}

ResourceIDType Lwm2mCore_GetNextResourceID(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
     return ObjectStore_GetNextResourceID(context->Store, objectID, objectInstanceID, resourceID);
}

ResourceInstanceIDType Lwm2mCore_GetNextResourceInstanceID(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID,
                                                           ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID)
{
    return ObjectStore_GetNextResourceInstanceID(context->Store, objectID, objectInstanceID, resourceID, resourceInstanceID);
}

// This function is called by the CoAP library to handle any requests
static int Lwm2mCore_HandleRequest(CoapRequest * request, CoapResponse * response)
{
    int result = 0;
    Lwm2mContextType * context = (Lwm2mContextType *)request->ctxt;
    ResourceEndPoint * endPoint = Lwm2mEndPoint_FindResourceEndPoint(&context->EndPointList, request->path);
    if (endPoint != NULL)
    {
        result = endPoint->Handler(request->type, request->ctxt, &request->addr, request->path, request->query,
                 request->token, request->tokenLength, request->contentType, request->requestContent,
                 request->requestContentLen, &response->responseContentType, response->responseContent,
                 &response->responseContentLen, &response->responseCode);
    }
    else
    {
        response->responseContentType = AwaContentType_None;
        response->responseContentLen = 0;
        response->responseCode = AwaResult_NotFound;
    }

    return result;
}

int Lwm2mCore_AddResourceEndPoint(Lwm2mContextType * context, const char * path, EndpointHandlerFunction handler)
{
    return Lwm2mEndPoint_AddResourceEndPoint(&context->EndPointList, path, handler);
}

DefinitionRegistry * Lwm2mCore_GetDefinitions(Lwm2mContextType * context)
{
    return context->Definitions;
}

int Lwm2mCore_RemoveResourceEndPoint(Lwm2mContextType * context, const char * path)
{
    return Lwm2mEndPoint_RemoveResourceEndPoint(&context->EndPointList, path);
}

struct ListHead * Lwm2mCore_GetClientList(Lwm2mContextType * context)
{
    return &context->ClientList;
}

AwaContentType Lwm2mCore_GetContentType(Lwm2mContextType * context)
{
    return context->ContentType;
}

int Lwm2mCore_GetLastLocation(Lwm2mContextType * context)
{
    return context->LastLocation;
}

struct ListHead * Lwm2mCore_GetEventRecordList(Lwm2mContextType * context)
{
    return &context->EventRecordList;
}

void Lwm2mCore_SetLastLocation(Lwm2mContextType * context, int location)
{
    context->LastLocation = location;
}

Lwm2mContextType * Lwm2mCore_Init(CoapInfo * coap, AwaContentType contentType)
{
    Lwm2m_Debug("Create object store\n");

    Lwm2mContextType * context = &Lwm2mContext;
    context->Coap = coap;
    context->Store = ObjectStore_Create();
    context->Definitions = DefinitionRegistry_Create();
    context->ContentType = contentType;

    Lwm2mEndPoint_InitEndPointList(&context->EndPointList);

    coap_SetContext(context);
    coap_SetRequestHandler(Lwm2mCore_HandleRequest);

    // Initialise registration data, i.e tables for storing client information
    Lwm2m_RegistrationInit(context);
    return context;
}

void Lwm2mCore_Destroy(Lwm2mContextType * context)
{
    Lwm2mEndPoint_DestroyEndPointList(&context->EndPointList);
    ObjectStore_Destroy(context->Store);
    Lwm2m_RegistrationDestroy(context);
    DefinitionRegistry_Destroy(context->Definitions);
}

bool qClientRegisterEvent = false;

int Lwm2mCore_Process(Lwm2mContextType * context)
{
    int nextTick = 1000;
    Lwm2m_AgeRegistrations(context);
    return nextTick;
}

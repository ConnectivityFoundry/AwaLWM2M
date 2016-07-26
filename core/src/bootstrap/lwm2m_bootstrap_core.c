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

#include "bootstrap/lwm2m_bootstrap.h"
#include "lwm2m_list.h"
#include "lwm2m_endpoints.h"
#include "coap_abstraction.h"
#include "lwm2m_types.h"
#include "lwm2m_debug.h"
#include "lwm2m_util.h"
#include "lwm2m_core.h"
#include "lwm2m_result.h"

struct _Lwm2mContextType
{
    ObjectStore * Store;                      // Object store associated with this context
    DefinitionRegistry * Definitions;
    ResourceEndPointList EndPointList;        // CoAP endpoints
    CoapInfo * Coap;                          // CoAP library context information
};

static Lwm2mContextType Lwm2mContext;


int Lwm2mCore_RegisterObjectType(Lwm2mContextType * context, const char * objName, ObjectIDType objectID, uint16_t maximumInstances,
                                 uint16_t minimumInstances, ObjectOperationHandlers * handlers)
{
    return Definition_RegisterObjectType(context->Definitions, objName, objectID, maximumInstances, minimumInstances, handlers);
}

int Lwm2mCore_RegisterResourceType(Lwm2mContextType * context, const char * resName, ObjectIDType objectID, ResourceIDType resourceID, AwaResourceType resourceType,
                                   uint16_t maximumInstances, uint16_t minimumInstances, AwaResourceOperations operations,
                                   ResourceOperationHandlers * Handlers)
{
    return Definition_RegisterResourceType(context->Definitions, resName, objectID, resourceID, resourceType, maximumInstances, minimumInstances, operations, Handlers, NULL);
}

int Lwm2mCore_RegisterResourceTypeWithDefaultValue(Lwm2mContextType * context, const char * resName, ObjectIDType objectID, ResourceIDType resourceID, AwaResourceType resourceType,
                                   uint16_t maximumInstances, uint16_t minimumInstances, AwaResourceOperations operations,
                                   ResourceOperationHandlers * handlers, Lwm2mTreeNode * defaultValueNode)
{
    return Definition_RegisterResourceType(context->Definitions, resName, objectID, resourceID, resourceType, maximumInstances, minimumInstances, operations, handlers, defaultValueNode);
}

int Lwm2mCore_SetResourceInstanceValue(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID,
                                        ResourceInstanceIDType resourceInstanceID, const void * value, size_t valueSize)
{
    int nullTerminator = 0;
    bool changed;

    if (Definition_GetResourceType(((Lwm2mContextType *)(context))->Definitions, objectID, resourceID) == AwaResourceType_String)
    {
        nullTerminator = 1;
    }

    return ObjectStore_SetResourceInstanceValue(((Lwm2mContextType *)(context))->Store, objectID, objectInstanceID, resourceID, resourceInstanceID,
                                                 valueSize + nullTerminator, value, 0, valueSize, &changed);
}

int Lwm2mCore_CreateObjectInstance(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID)
{
    ObjectDefinition * definition = Definition_LookupObjectDefinition(context->Definitions, objectID);
    int result = -1;

    if (definition != NULL)
    {
        result = ObjectStore_CreateObjectInstance(context->Store, objectID, objectInstanceID, definition->MaximumInstances);
    }
    else
    {
        Lwm2m_Error("No definition for object ID %d\n", objectID);
        AwaResult_SetResult(AwaResult_NotFound);
    }
    return result;
}

int Lwm2mCore_CreateOptionalResource(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    return (ObjectStore_CreateResource(context->Store, objectID, objectInstanceID, resourceID) == -1) ? -1 : 0;
}

int Lwm2mCore_GetResourceInstanceValue(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, 
                                       ResourceInstanceIDType resourceInstanceID, const void ** buffer, size_t * bufferLen)
{
    return ObjectStore_GetResourceInstanceValue(((Lwm2mContextType *)(context))->Store, objectID, objectInstanceID, resourceID, resourceInstanceID, buffer, bufferLen);
}

ObjectInstanceIDType Lwm2mCore_GetNextObjectInstanceID(Lwm2mContextType * context, ObjectIDType  objectID, ObjectInstanceIDType objectInstanceID)
{
    return ObjectStore_GetNextObjectInstanceID(context->Store, objectID, objectInstanceID);
}

ResourceIDType Lwm2mCore_GetNextResourceID(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
     return ObjectStore_GetNextResourceID(context->Store, objectID, objectInstanceID, resourceID);
}

ResourceInstanceIDType Lwm2mCore_GetNextResourceInstanceID(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID)
{
    return ObjectStore_GetNextResourceInstanceID(context->Store, objectID, objectInstanceID, resourceID, resourceInstanceID);
}

int Lwm2mCore_AddResourceEndPoint(Lwm2mContextType * context, const char * path, EndpointHandlerFunction handler)
{
    return Lwm2mEndPoint_AddResourceEndPoint(&context->EndPointList, path, handler);
}

DefinitionRegistry * Lwm2mCore_GetDefinitions(Lwm2mContextType * context)
{
    return context->Definitions;
}

bool Lwm2mCore_Exists(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    bool result = false;

    if (context != NULL)
    {
        result = ObjectStore_Exists(context->Store, objectID, objectInstanceID, resourceID);
    }

    return result;
}

// This function is called by the CoAP library to handle any requests
static int Lwm2mCore_HandleRequest(CoapRequest * request, CoapResponse * response)
{
    Lwm2mContextType * context = (Lwm2mContextType*)request->ctxt;
    ResourceEndPoint * endPoint = Lwm2mEndPoint_FindResourceEndPoint(&context->EndPointList, request->path);
    if (endPoint == NULL)
    {
        response->responseContentType = AwaContentType_None;
        response->responseContentLen = 0;
        response->responseCode = AwaResult_NotFound;
        return 0;
    }

    return endPoint->Handler(request->type, request->ctxt, &request->addr, request->path, request->query,
                             request->token, request->tokenLength, request->contentType, request->requestContent,
                             request->requestContentLen, &response->responseContentType, response->responseContent,
                             &response->responseContentLen, &response->responseCode);
}

// Initialise the LWM2M core, setup any callbacks, initialise CoAP etc
Lwm2mContextType * Lwm2mCore_Init(CoapInfo * coap)
{
    Lwm2mContextType * context = &Lwm2mContext;
    context->Coap = coap;
    context->Store = ObjectStore_Create();
    context->Definitions = DefinitionRegistry_Create();

    coap_SetContext(context);
    coap_SetRequestHandler(Lwm2mCore_HandleRequest);

    Lwm2mEndPoint_InitEndPointList(&context->EndPointList);

    return context;
}

/**
 * @brief Update the LWM2M state machine
 *
 * @param[in] content
 * @returns time until next service (TODO)
 */
int Lwm2mCore_Process(Lwm2mContextType * context)
{
    int nextTick = 10;

    Lwm2mBootstrap_BootStrapUpdate(context);

    return nextTick;
}

void Lwm2mCore_Destroy(Lwm2mContextType * context)
{
    Lwm2mEndPoint_DestroyEndPointList(&context->EndPointList);
    ObjectStore_Destroy(context->Store);
    DefinitionRegistry_Destroy(context->Definitions);
}


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
#ifdef LWM2M_CLIENT
#include "lwm2m_object_tree.h"
#include "lwm2m_result.h"
#include "lwm2m_bootstrap_config.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LWM2M_CLIENT
#define MAX_ENDPOINT_NAME_LENGTH 128

typedef enum
{
    Lwm2mBootStrapState_NotBootStrapped,
    Lwm2mBootStrapState_BootStrapPending,
    Lwm2mBootStrapState_CheckExisting,
    Lwm2mBootStrapState_ClientHoldOff,
    Lwm2mBootStrapState_BootStrapFinishPending,  // Waiting for the server to send a bootstrap finished.
    Lwm2mBootStrapState_BootStrapped,
    Lwm2mBootStrapState_BootStrapFailed,

} Lwm2mBootStrapState;

typedef enum
{
    Lwm2mRegistrationState_NotRegistered,
    Lwm2mRegistrationState_Register,
    Lwm2mRegistrationState_Registering,
    Lwm2mRegistrationState_Registered,
    Lwm2mRegistrationState_Deregister,
    Lwm2mRegistrationState_Deregistering,
    Lwm2mRegistrationState_RegisterFailed,
    Lwm2mRegistrationState_RegisterFailedRetry,
    Lwm2mRegistrationState_UpdatingRegistration,

} Lwm2mRegistrationState;

typedef struct
{
    struct ListHead list;
    int ServerObjectInstanceID;
    char Location[128];
    Lwm2mRegistrationState RegistrationState;
    uint32_t LastUpdate;
    int Attempts;
    bool UpdateRegistration;

    int LifeTime;
    int ShortServerID;
    int DefaultMinimumPeriod;
    int DefaultMaximumPeriod;
    int DisableTimeout;
    bool NotificationStoring;
    char Binding[4];  // maximum "UQS" + '\0'

} Lwm2mServerType;

typedef struct
{
    Lwm2mBootStrapState BootStrapState;       // Current bootstrap state
    uint32_t LastBootStrapUpdate;             // Time that the last bootstrap state-machine update was performed
    struct ListHead ServerList;               // Linked list of "Lwm2mServerType" for the registration process
    struct ListHead SecurityObjectList;       // Linked list of "LWM2MSecurityInfo"
    Lwm2mObjectTree ObjectTree;
    ObjectStore * Store;                      // Object store associated with this context
    AttributeStore * AttributeStore;          // Notification Attributes store associated with this context
    DefinitionRegistry * Definitions;         // Storage for object/resource definitions.
    ResourceEndPointList EndPointList;        // CoaP endpoints.
    CoapInfo * Coap;                          // CoAP library context information
    char EndPointName[MAX_ENDPOINT_NAME_LENGTH];  // Client EndPoint name
    bool UseFactoryBootstrap;                 // Factory bootstrap information has been loaded from file.
    struct ListHead observerList;
} Lwm2mContextType;

// Default handlers for objects and resources. these write directly to the object store
extern ResourceOperationHandlers defaultResourceOperationHandlers;
extern ObjectOperationHandlers defaultObjectOperationHandlers;
#else

typedef struct
{
    ObjectStore * Store;                      // Object store associated with this context
    DefinitionRegistry * Definitions;
    ResourceEndPointList EndPointList;        // CoAP endpoints
    CoapInfo * Coap;                          // CoAP library context information
    struct ListHead ClientList;               // List of registered clients
    int LastLocation;                         // Used for registration, creates /rd/0, /rd/1 etc
    ContentType ContentType;                  // Used to set CoAP content type
} Lwm2mContextType;

#endif

// Initialise the LWM2M core, setup any callbacks, initialise CoAP etc
#ifdef LWM2M_CLIENT
Lwm2mContextType * Lwm2mCore_Init(CoapInfo * coap, char * endPointName);

void Lwm2mCore_SetFactoryBootstrap(Lwm2mContextType * context, const BootstrapInfo * factoryBootstrapInformation);

#elif defined(LWM2M_SERVER)
Lwm2mContextType * Lwm2mCore_Init(CoapInfo * coap, ContentType contentType);
#else
Lwm2mContextType * Lwm2mCore_Init(CoapInfo * coap);
#endif

// Update the LWM2M state machine, process any message timeouts, registration attempts etc.
int Lwm2mCore_Process(Lwm2mContextType * context);

int Lwm2mCore_GetEndPointClientName(Lwm2mContextType * context, char * buffer, int len);

void Lwm2mCore_GetObjectList(Lwm2mContextType * context, char * altPath, char * buffer, int len, bool updated);

// Called by the server on shutdown, clean up any resources
void Lwm2mCore_Destroy(Lwm2mContextType * context);

// The following functions are called by other parts of the system to "Operate" on a resource
int Lwm2mCore_GetResourceInstanceValue(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID,
                                       ResourceInstanceIDType resourceInstanceID, void * Value, int ValueBufferSize);

int Lwm2mCore_GetResourceInstanceLength(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID);
int Lwm2mCore_GetResourceInstanceCount(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);

int Lwm2mCore_CreateObjectInstance(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID);
int Lwm2mCore_SetResourceInstanceValue(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID,
                                       ResourceInstanceIDType resourceInstanceID, const void * Value, int ValueSize);

int Lwm2mCore_CreateObjectInstance(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID);
int Lwm2mCore_CreateOptionalResource(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);

int Lwm2mCore_RegisterObjectType(Lwm2mContextType * context, const char * objName, ObjectIDType objectID, uint16_t MaximumInstances,
                                 uint16_t MinimumInstances, ObjectOperationHandlers * Handlers);


int Lwm2mCore_RegisterResourceTypeWithDefaultValue(Lwm2mContextType * context, const char * resName, ObjectIDType objectID, ResourceIDType resourceID, ResourceTypeType resourceType,
                                                   uint16_t MaximumInstances, uint16_t MinimumInstances, Operations operations,
                                                   ResourceOperationHandlers * Handlers, Lwm2mTreeNode * defaultValueNode);
int Lwm2mCore_RegisterResourceType(Lwm2mContextType * context, const char * resName, ObjectIDType objectID, ResourceIDType resourceID, ResourceTypeType resourceType,
                                   uint16_t MaximumInstances, uint16_t MinimumInstances, Operations operations,
                                   ResourceOperationHandlers * Handlers);

ObjectInstanceIDType Lwm2mCore_GetNextObjectInstanceID(Lwm2mContextType * context, ObjectIDType  objectID, ObjectInstanceIDType objectInstanceID);
ResourceIDType Lwm2mCore_GetNextResourceID(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);
ResourceInstanceIDType Lwm2mCore_GetNextResourceInstanceID(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID);


#ifdef LWM2M_CLIENT
Lwm2mResult Lwm2mCore_Delete(Lwm2mContextType * context, Lwm2mRequestOrigin requestOrigin, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);

int Lwm2mCore_Observe(Lwm2mContextType * context, AddressType * addr, const char * token, int tokenLength, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID,
                      ResourceIDType resourceID, ContentType contentType, Lwm2mNotificationCallback callback, void * ContextData);

int Lwm2mCore_CancelObserve(Lwm2mContextType * context, AddressType * addr, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);

int Lwm2mCore_Exists(Lwm2mContextType * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);

Lwm2mResult Lwm2mCore_CheckWritePermissionsForResourceNode(Lwm2mContextType * context, Lwm2mRequestOrigin origin, Lwm2mTreeNode * resourceNode,
                                                           ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, bool createObjectInstance);

Lwm2mResult Lwm2mCore_CheckWritePermissionsForObjectInstanceNode(Lwm2mContextType * context, Lwm2mRequestOrigin origin,
                                                                 Lwm2mTreeNode * objectInstanceNode, int objectID, bool createObjectInstance);

Lwm2mResult Lwm2mCore_CheckWritePermissionsForObjectNode(Lwm2mContextType * context, Lwm2mRequestOrigin origin, Lwm2mTreeNode * objectNode, bool createObjectInstance);

Lwm2mResult Lwm2mCore_ParseObjectNodeAndWriteToStore(Lwm2mContextType * context, Lwm2mTreeNode * objectNode, bool createObjectInstance,
                                                     bool createOptionalResources, bool replace, int * newObjectInstanceID);

ObjectIDType Lwm2mCore_GetNextObjectID(Lwm2mContextType * context, ObjectIDType  objectID);

int Lwm2mCore_GetObjectNumInstances(Lwm2mContextType * context, ObjectIDType objectID);


#endif

#ifdef __cplusplus
}
#endif

#endif // LWM2M_CORE_H

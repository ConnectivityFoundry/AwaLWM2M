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


#include "lwm2m_core.h"
#include "lwm2m_server_object.h"
#include "lwm2m_objects.h"

static int Lwm2mServer_ResourceReadHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID,
                                           ResourceInstanceIDType resourceInstanceID, const void ** buffer, size_t * bufferLen);

static int Lwm2mServer_ResourceWriteHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID,
                                            ResourceInstanceIDType resourceInstanceID, uint8_t * srcBuffer, size_t srcBufferLen, bool * changed);

static int Lwm2mServer_ObjectCreateInstanceHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID);

static int Lwm2mServer_ObjectDeleteHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);

static int Lwm2mServer_ObjectDeleteHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);

static int Lwm2mServer_CreateOptionalResourceHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);


static ObjectOperationHandlers serverObjectOperationHandlers =
{
    .CreateInstance = Lwm2mServer_ObjectCreateInstanceHandler,
    .Delete = Lwm2mServer_ObjectDeleteHandler,
};

static ResourceOperationHandlers serverResourceOperationHandlers =
{
    .Read = Lwm2mServer_ResourceReadHandler,
    .Write = Lwm2mServer_ResourceWriteHandler,
    .Execute = NULL,
    .CreateOptionalResource = Lwm2mServer_CreateOptionalResourceHandler,
};


static Lwm2mServerType * GetServerObjectByObjectInstanceID(Lwm2mContextType * context, int objectInstanceID)
{
    Lwm2mServerType * serverObject = NULL;
    struct ListHead * i;
    ListForEach(i, Lwm2mCore_GetServerList(context))
    {
        Lwm2mServerType * server = ListEntry(i, Lwm2mServerType, list);
        if (server->ServerObjectInstanceID == objectInstanceID)
        {
            serverObject = server;
        }
    }
    return serverObject;
}

static Lwm2mServerType * GetServerObjectByShortServerID(Lwm2mContextType * context, int shortServerID)
{
    Lwm2mServerType * serverObject = NULL;
    struct ListHead * i;
    ListForEach(i, Lwm2mCore_GetServerList(context))
    {
        Lwm2mServerType * server = ListEntry(i, Lwm2mServerType, list);
        if (server->ShortServerID == shortServerID)
        {
            serverObject = server;
        }
    }
    return serverObject;
}

static Lwm2mServerType * AddServerObject(Lwm2mContextType * context, int serverObjectInstanceID)
{
    Lwm2mServerType * serverType = NULL;

    if (GetServerObjectByObjectInstanceID(context, serverObjectInstanceID) == NULL)
    {
        serverType = malloc(sizeof(Lwm2mServerType));
        if (serverType != NULL)
        {
            memset(serverType, 0, sizeof(Lwm2mServerType));
            serverType->ServerObjectInstanceID = serverObjectInstanceID;
            serverType->NotificationStoring = true;
            serverType->DisableTimeout = 86400;
            serverType->DefaultMinimumPeriod = 0;
            serverType->DefaultMaximumPeriod = -1;
            strcpy(serverType->Binding, "U");
            ListAdd(&serverType->list, Lwm2mCore_GetServerList(context));
        }
        else
        {
            Lwm2m_Error("Failed to allocate memory for server object (instance %d)\n", serverObjectInstanceID);
        }
    }
    return serverType;
}

static int Lwm2mServer_ObjectCreateInstanceHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID)
{
    Lwm2m_Debug("Creating server object instance\n");
    return (AddServerObject(context, objectInstanceID) != NULL) ? objectInstanceID : -1;
}

static int Lwm2mServer_CreateOptionalResourceHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    // No action
    return 0;
}

static int RemoveServerObject(Lwm2mContextType * context, int objectInstanceID)
{
    int result = -1;
    Lwm2mServerType * server = GetServerObjectByObjectInstanceID(context, objectInstanceID);
    if (server != NULL)
    {
        ListRemove(&server->list);
        free(server);
        result = 0;
    }
    return result;
}

static int Lwm2mServer_ObjectDeleteHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    return RemoveServerObject(context, objectInstanceID);
}

static int Lwm2mServer_ResourceReadHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID,
                                           ResourceInstanceIDType resourceInstanceID, const void ** buffer, size_t * bufferLen)
{
    Lwm2mServerType * server = GetServerObjectByObjectInstanceID(context, objectInstanceID);
    if (server != NULL)
    {
        switch(resourceID)
        {
            case LWM2M_SERVER_OBJECT_SHORT_SERVER_ID:
                *buffer = &server->ShortServerID;
                *bufferLen = sizeof(server->ShortServerID);
                break;

            case LWM2M_SERVER_OBJECT_LIFETIME:
                *buffer = &server->LifeTime;
                *bufferLen = sizeof(server->LifeTime);
                break;

            case LWM2M_SERVER_OBJECT_MINIMUM_PERIOD:
                *buffer = &server->DefaultMinimumPeriod;
                *bufferLen = sizeof(server->DefaultMinimumPeriod);
                break;

            case LWM2M_SERVER_OBJECT_MAXIMUM_PERIOD:
                *buffer = &server->DefaultMaximumPeriod;
                *bufferLen = sizeof(server->DefaultMaximumPeriod);
                break;

            case LWM2M_SERVER_OBJECT_DISABLE_TIMEOUT:
                *buffer = &server->DisableTimeout;
                *bufferLen = sizeof(server->DisableTimeout);
                break;

            case LWM2M_SERVER_OBJECT_NOTIFICATION_STORING:
                *buffer = &server->NotificationStoring;
                *bufferLen = sizeof(server->NotificationStoring);
                break;

            case LWM2M_SERVER_OBJECT_BINDING:
                *buffer = server->Binding;
                *bufferLen = strlen(server->Binding);
                break;

            default:
                *bufferLen = -1;
        }
    }
    else
    {
        *bufferLen = -1;
    }
    return *bufferLen;
}

// Warn via log if the expected value size is larger than the supplied buffer size
static void WarnOfInsufficientData(size_t dest_size, size_t src_size)
{
    if (dest_size > src_size)
    {
        Lwm2m_Warning("Source size is insufficient in Write (expect %zu, got %zu)\n", dest_size, src_size);
    }
}

static int Lwm2mServer_ResourceWriteHandler(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID,
                                            ResourceInstanceIDType resourceInstanceID, uint8_t * srcBuffer, size_t srcBufferLen, bool * changed)
{
    int result = -1;

    // FIXME: this server entry needs to be created when the server objects are written.

    // Add entry to server list.
    Lwm2mCore_UpdateServer(context, objectInstanceID, Lwm2mRegistrationState_Register);

    Lwm2mServerType * server = GetServerObjectByObjectInstanceID(context, objectInstanceID);
    if (server != NULL)
    {
        switch (resourceID)
        {
            case LWM2M_SERVER_OBJECT_SHORT_SERVER_ID:
                memcpy(&server->ShortServerID, srcBuffer, sizeof(server->ShortServerID));
                result = sizeof(server->ShortServerID);
                WarnOfInsufficientData(result, srcBufferLen);
                *changed = true;
                break;

            case LWM2M_SERVER_OBJECT_LIFETIME:
                memcpy(&server->LifeTime, srcBuffer, sizeof(server->LifeTime));
                result = sizeof(server->LifeTime);
                WarnOfInsufficientData(result, srcBufferLen);
                *changed = true;
                break;

            case LWM2M_SERVER_OBJECT_MINIMUM_PERIOD:
            {
                int temp = 0;
                memcpy(&temp, srcBuffer, sizeof(server->DefaultMinimumPeriod));
                if ((server->DefaultMaximumPeriod != -1) && (temp > server->DefaultMaximumPeriod))
                {
                    Lwm2m_Error("Set server object %d default minimum period greater than default maximum period\n", server->ShortServerID);
                }
                else
                {
                    server->DefaultMinimumPeriod = temp;
                    result = sizeof(server->DefaultMinimumPeriod);
                    WarnOfInsufficientData(result, srcBufferLen);
                    *changed = true;
                }
                break;
            }

            case LWM2M_SERVER_OBJECT_MAXIMUM_PERIOD:
            {
                int temp = 0;
                memcpy(&temp, srcBuffer, sizeof(server->DefaultMaximumPeriod));
                if ((temp != -1) && (temp < server->DefaultMinimumPeriod))
                {
                    Lwm2m_Error("Set server object %d default maximum period smaller than default minimum period\n", server->ShortServerID);
                }
                else
                {
                    server->DefaultMaximumPeriod = temp;
                    result = sizeof(server->DefaultMaximumPeriod);
                    WarnOfInsufficientData(result, srcBufferLen);
                    *changed = true;
                }
                break;
            }

            case LWM2M_SERVER_OBJECT_DISABLE_TIMEOUT:
                memcpy(&server->DisableTimeout, srcBuffer, sizeof(server->DisableTimeout));
                result = sizeof(server->DisableTimeout);
                WarnOfInsufficientData(result, srcBufferLen);
                *changed = true;
                break;

            case LWM2M_SERVER_OBJECT_NOTIFICATION_STORING:
                memcpy(&server->NotificationStoring, srcBuffer, sizeof(server->NotificationStoring));
                result = sizeof(server->NotificationStoring);
                WarnOfInsufficientData(result, srcBufferLen);
                *changed = true;
                break;

            case LWM2M_SERVER_OBJECT_BINDING:
                memset(server->Binding, 0, sizeof(server->Binding));
                memcpy(server->Binding, srcBuffer, srcBufferLen);
                result = strlen(server->Binding);
                WarnOfInsufficientData(result, srcBufferLen);
                *changed = true;
                break;

            default:
                result = -1;
                break;
        }
    }
    return result;
}

static int executeRegistrationUpdateTrigger(void * context, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, uint8_t * inValueBuffer, size_t inValueBufferLen)
{
    Lwm2m_Debug("Registration Update triggered for server %d\n", objectInstanceID);
    Lwm2mCore_SetServerUpdateRegistration(context, objectInstanceID);
    return 0;
}

void Lwm2m_RegisterServerObject(Lwm2mContextType * context)
{
    ResourceOperationHandlers registrationUpdateTriggerOperationHandler = { .Execute = executeRegistrationUpdateTrigger, .CreateOptionalResource = Lwm2mServer_CreateOptionalResourceHandler };

    Lwm2mCore_RegisterObjectType(context, "LWM2MServer" , LWM2M_SERVER_OBJECT, MultipleInstancesEnum_Multiple, MandatoryEnum_Mandatory, &serverObjectOperationHandlers);

    Lwm2mCore_RegisterResourceType(context, "ShortServerID",                            LWM2M_SERVER_OBJECT, 0, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly,  &serverResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "Lifetime",                                 LWM2M_SERVER_OBJECT, 1, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadWrite, &serverResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "DefaultMinimumPeriod",                     LWM2M_SERVER_OBJECT, 2, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional,  AwaResourceOperations_ReadWrite, &serverResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "DefaultMaximumPeriod",                     LWM2M_SERVER_OBJECT, 3, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional,  AwaResourceOperations_ReadWrite, &serverResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "Disable",                                  LWM2M_SERVER_OBJECT, 4, AwaResourceType_None,    MultipleInstancesEnum_Single, MandatoryEnum_Optional,  AwaResourceOperations_Execute,  &serverResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "DisableTimeout",                           LWM2M_SERVER_OBJECT, 5, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional,  AwaResourceOperations_ReadWrite, &serverResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "NotificationStoringWhenDisabledorOffline", LWM2M_SERVER_OBJECT, 6, AwaResourceType_Boolean, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadWrite, &serverResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "Binding",                                  LWM2M_SERVER_OBJECT, 7, AwaResourceType_String,  MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadWrite, &serverResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "RegistrationUpdateTrigger",                LWM2M_SERVER_OBJECT, 8, AwaResourceType_None,    MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_Execute,  &registrationUpdateTriggerOperationHandler);
}

// Add a server entry to our list of servers or updates an existing one triggers a Registration request.
void Lwm2mCore_UpdateServer(Lwm2mContextType * context, int serverObjectInstanceID, Lwm2mRegistrationState state)
{
    Lwm2mServerType * server = GetServerObjectByObjectInstanceID(context, serverObjectInstanceID);
    if (server != NULL)
    {
        server->RegistrationState = state;
        server->Attempts = 0;
    }
}

void Lwm2mCore_SetServerUpdateRegistration(Lwm2mContextType * context, int serverObjectInstanceID)
{
    Lwm2mServerType * server = GetServerObjectByObjectInstanceID(context, serverObjectInstanceID);
    if (server != NULL)
    {
        server->UpdateRegistration = true;
    }
}

int Lwm2mServerObject_GetDefaultMinimumPeriod(Lwm2mContextType * context, int shortServerID)
{
    int64_t defaultMinimumPeriod = 0;
    Lwm2mServerType * server = GetServerObjectByShortServerID(context, shortServerID);
    if (server != NULL)
    {
        defaultMinimumPeriod = server->DefaultMinimumPeriod;
    }
    return defaultMinimumPeriod;
}
int Lwm2mServerObject_GetDefaultMaximumPeriod(Lwm2mContextType * context, int shortServerID)
{
    int64_t defaultMaximumPeriod = -1;
    Lwm2mServerType * server = GetServerObjectByShortServerID(context, shortServerID);
    if (server != NULL)
    {
       defaultMaximumPeriod = server->DefaultMaximumPeriod;
    }
    return defaultMaximumPeriod;
}

int Lwm2mServerObject_GetTransportBinding(Lwm2mContextType * context, int shortServerID, char * buffer, int len)
{
    Lwm2mServerType * server = GetServerObjectByShortServerID(context, shortServerID);
    if (server == NULL)
    {
        return -1;
    }
    strncpy(buffer, server->Binding, len);
    return strlen(buffer);
}

int Lwm2mServerObject_GetLifeTime(Lwm2mContextType * context, int shortServerID)
{
    Lwm2mServerType * server = GetServerObjectByShortServerID(context, shortServerID);
    if (server == NULL)
    {
        return -1;
    }
    return server->LifeTime;
}

// Set all servers back to the Registration request state
void Lwm2mCore_UpdateAllServers(Lwm2mContextType * context, Lwm2mRegistrationState state)
{
    struct ListHead * i;
    ListForEach(i, Lwm2mCore_GetServerList(context))
    {
        Lwm2mServerType * server = ListEntry(i, Lwm2mServerType, list);
        if (server != NULL)
        {
            server->RegistrationState = state;
            server->Attempts = 0;
        }
    }
}

void Lwm2mCore_DeregisterAllServers(Lwm2mContextType * context)
{
    struct ListHead * i;
    ListForEach(i, Lwm2mCore_GetServerList(context))
    {
        Lwm2mServerType * server = ListEntry(i, Lwm2mServerType, list);
        if (server != NULL)
        {
            if (server->RegistrationState != Lwm2mRegistrationState_NotRegistered)
            {
                server->RegistrationState = Lwm2mRegistrationState_Deregister;
            }
        }
    }
}

void Lwm2mCore_DestroyServerList(Lwm2mContextType * context)
{
    if (context != NULL)
    {
        struct ListHead * i, * n;
        ListForEachSafe(i, n, Lwm2mCore_GetServerList(context))
        {
            Lwm2mServerType * server = ListEntry(i, Lwm2mServerType, list);
            if (server != NULL)
            {
                free(server);
            }
        }
    }
}

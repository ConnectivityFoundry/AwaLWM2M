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

#if !defined (CONTIKI)
  #include <signal.h>
  #include <errno.h>
  #include <poll.h>
#else
  #include "contiki.h"
  #include "contiki-net.h"
#endif

#include <stdint.h>
#include <stdbool.h>

#include "awa/static.h"
#include "lwm2m_security_object.h"
#include "lwm2m_server_object.h"
#include "lwm2m_acl_object.h"
#include "lwm2m_debug.h"

#define MAX_ADDRESS_LENGTH           (50)
#define DEFAULT_CLIENT_HOLD_OFF_TIME (30)

struct _AwaStaticClient
{
    Lwm2mContextType * Context;
    CoapInfo * CoAPInfo;
    bool BootstrapConfigured;
    bool EndpointNameConfigured;
    bool CoAPConfigured;
    bool Running;
    bool Initialised;
    const char * BootstrapServerURI;
    char CoAPListenAddress[MAX_ADDRESS_LENGTH];
    int CoAPListenPort;
    void * ApplicationContext;
};

AwaStaticClient * AwaStaticClient_New()
{
    AwaStaticClient * client = (AwaStaticClient *)malloc(sizeof(*client));

    if (client != NULL)
    {
        memset(client, 0, sizeof(*client));
        client->Context = Lwm2mCore_New();

        if (client->Context != NULL)
        {
            client->BootstrapConfigured = false;
            client->EndpointNameConfigured = false;
            client->CoAPConfigured = false;
            client->Running = false;
            client->Initialised = false;
        }
        else
        {
            free(client);
            client = NULL;
        }
    }

    return client;
}

void AwaStaticClient_Free(AwaStaticClient ** client)
{
    if ((client != NULL) && (*client != NULL))
    {
        Lwm2mCore_Destroy((*client)->Context);

        if ((*client)->CoAPInfo != NULL)
        {
            coap_Destroy();
        }

        free(*client);
        *client = NULL;
    }
}

AwaError AwaStaticClient_Init(AwaStaticClient * client)
{
    AwaError result = AwaError_Unspecified;

    if (client != NULL)
    {
        if (client->CoAPConfigured && client->BootstrapConfigured && client->EndpointNameConfigured)
        {
            client->CoAPInfo = coap_Init(client->CoAPListenAddress, client->CoAPListenPort, Lwm2m_GetLogLevel());

            if (client->CoAPInfo != NULL)
            {
                Lwm2mCore_SetCoapInfo(client->Context, client->CoAPInfo);
                Lwm2m_RegisterACLObject(client->Context);
                Lwm2m_RegisterServerObject(client->Context);
                Lwm2m_RegisterSecurityObject(client->Context);
                Lwm2m_PopulateSecurityObject(client->Context, client->BootstrapServerURI);
                Lwm2mCore_SetApplicationContext(client->Context, client);
                client->Initialised = true;
                result = AwaError_Success;
            }
            else
            {
                result = AwaError_Internal;
            }
        }
        else
        {
            result = AwaError_StaticClientNotConfigured;
        }
    }
    else
    {
        result = AwaError_StaticClientInvalid;
    }

    return result;
}

AwaError AwaStaticClient_SetLogLevel(AwaLogLevel level)
{
    AwaError result = AwaError_Unspecified;
    if ((level >= AwaLogLevel_None) && (level <= AwaLogLevel_Debug))
    {
        Lwm2m_SetAwaLogLevel(level);
        coap_SetLogLevel(Lwm2m_GetLogLevel());

        result = AwaError_Success;
    }
    else
    {
        result = AwaError_LogLevelInvalid;
    }

    return result;
}

AwaError AwaStaticClient_SetBootstrapServerURI(AwaStaticClient * client, const char * bootstrapServerURI)
{
    AwaError result = AwaError_Unspecified;

    if ((client != NULL) && (bootstrapServerURI != NULL))
    {
        if (!client->Running && !client->Initialised)
        {
            client->BootstrapServerURI = bootstrapServerURI;
            client->BootstrapConfigured = true;
            result = AwaError_Success;
        }
        else
        {
            result = AwaError_OperationInvalid;
        }
    }
    else
    {
        result = AwaError_StaticClientInvalid;
    }

    return result;
}

AwaError AwaStaticClient_SetFactoryBootstrapInformation(AwaStaticClient * client, const AwaFactoryBootstrapInfo * factoryBootstrapInformation)
{
    AwaError result = AwaError_Unspecified;

    if ((client != NULL) && (factoryBootstrapInformation != NULL))
    {
        if (!client->Running && client->Initialised)
        {

            if(client->CoAPConfigured)
            {

                // Mandatory resources only
#if (__GNUC__ >= 5)
                BootstrapInfo info = { 0 };
#else
                // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53119
                BootstrapInfo info;
                memset(&info, 0, sizeof(info));
#endif

                // Assign an arbitrary server ID
                static int serverID = 1000;
                ++serverID;

                info.SecurityInfo.ServerID = serverID;
                strncpy(info.SecurityInfo.ServerURI, factoryBootstrapInformation->SecurityInfo.ServerURI, BOOTSTRAP_CONFIG_SERVER_URI_SIZE);
                info.SecurityInfo.ServerURI[BOOTSTRAP_CONFIG_SERVER_URI_SIZE - 1] = '\0'; // Defensive
                info.SecurityInfo.Bootstrap = false;
                info.SecurityInfo.SecurityMode = factoryBootstrapInformation->SecurityInfo.SecurityMode;
                memcpy(info.SecurityInfo.PublicKey, factoryBootstrapInformation->SecurityInfo.PublicKeyOrIdentity, BOOTSTRAP_CONFIG_PUBLIC_KEY_SIZE);
                memcpy(info.SecurityInfo.SecretKey, factoryBootstrapInformation->SecurityInfo.SecretKey, BOOTSTRAP_CONFIG_SECRET_KEY_SIZE);
                info.SecurityInfo.HoldOffTime = DEFAULT_CLIENT_HOLD_OFF_TIME;

                info.ServerInfo.ShortServerID = serverID;
                info.ServerInfo.LifeTime = factoryBootstrapInformation->ServerInfo.Lifetime;
                info.ServerInfo.MinPeriod = factoryBootstrapInformation->ServerInfo.DefaultMinPeriod;
                info.ServerInfo.MaxPeriod = factoryBootstrapInformation->ServerInfo.DefaultMaxPeriod;
                info.ServerInfo.DisableTimeout = factoryBootstrapInformation->ServerInfo.DisableTimeout;
                info.ServerInfo.Notification = factoryBootstrapInformation->ServerInfo.Notification;
                strncpy(info.ServerInfo.Binding, factoryBootstrapInformation->ServerInfo.Binding, BOOTSTRAP_CONFIG_BINDING_SIZE);
                info.ServerInfo.Binding[BOOTSTRAP_CONFIG_BINDING_SIZE - 1] = '\0'; // Defensive

                Lwm2mCore_SetFactoryBootstrap(client->Context, &info);
                result = AwaError_Success;
            }
            else
            {
                result = AwaError_StaticClientInvalid;
            }
        }
        else
        {
            result = AwaError_OperationInvalid;
        }
    }
    else
    {
        result = AwaError_StaticClientInvalid;
    }

    return result;
}

AwaError AwaStaticClient_SetEndPointName(AwaStaticClient * client, const char * EndPointName)
{
    AwaError result = AwaError_Unspecified;

    if ((client != NULL) && (EndPointName != NULL))
    {
        if (!client->Running && !client->Initialised)
        {
            if (Lwm2mCore_SetEndPointClientName(client->Context, EndPointName) > 0)
            {
                Lwm2m_Debug("Client endpoint name: %s\n", EndPointName);
                client->EndpointNameConfigured = true;
                result = AwaError_Success;
            }
            else
            {
                result = AwaError_OutOfMemory;
            }
        }
        else
        {
            result = AwaError_OperationInvalid;
        }
    }
    else
    {
        result = AwaError_StaticClientInvalid;
    }

    return result;
}

AwaError AwaStaticClient_SetCoAPListenAddressPort(AwaStaticClient * client, const char * address, int port)
{
    AwaError result = AwaError_Unspecified;

    if ((client != NULL) && (address != NULL))
    {
        if (!client->Running && !client->Initialised)
        {
            if (strlen(address) < MAX_ADDRESS_LENGTH)
            {
                strcpy(client->CoAPListenAddress, address);
                client->CoAPListenPort = port;
                client->CoAPConfigured = true;
                result = AwaError_Success;
            }
            else
            {
                result = AwaError_OutOfMemory;
            }
        }
        else
        {
            result = AwaError_OperationInvalid;
        }
    }
    else
    {
        result = AwaError_StaticClientInvalid;
    }

    return result;
}

AwaError AwaStaticClient_SetApplicationContext(AwaStaticClient * client, void * context)
{
    AwaError result = AwaError_Unspecified;

    if (client != NULL)
    {
        client->ApplicationContext = context;
        result = AwaError_Success;
    }
    else
    {
        result = AwaError_StaticClientInvalid;
    }

    return result;
}

void * AwaStaticClient_GetApplicationContext(AwaStaticClient * client)
{
    void *  result = NULL;

    if (client != NULL)
    {
        result = client->ApplicationContext;
    }

    return result;
}

int AwaStaticClient_Process(AwaStaticClient * client)
{
    int result = -1;

    if (client != NULL)
    {
        if (!client->Running)
        {
            client->Running = true;
        }
#if !defined (CONTIKI)
        struct pollfd fds[1];
        int nfds = 1;
        int timeout;

        fds[0].fd = client->CoAPInfo->fd;
        fds[0].events = POLLIN;

        timeout = Lwm2mCore_Process(client->Context);

        result = poll(fds, nfds, timeout);
        if (result < 0)
        {
            if (errno == EINTR)
            {
                result = timeout;
            }
            else
            {
                perror("poll:");
            }
        }
        else if (result > 0)
        {
            if (fds[0].revents == POLLIN)
            {
                coap_HandleMessage();
            }

            result = timeout;
        }

        if (result == timeout)
        {
            coap_Process();
        }
#else
        result = Lwm2mCore_Process(client->Context);
#endif

    }

    return result;
}

static AwaResult DefaultHandler(AwaStaticClient * client, AwaOperation operation,
                                AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID,
                                void ** dataPointer, size_t * dataSize, bool * changed)
{
    AwaResult result;

    ObjectDefinition * objectDefinition = Definition_LookupObjectDefinition(Lwm2mCore_GetDefinitions(client->Context), objectID);
    if (objectDefinition != NULL)
    {
        uint8_t * offset;
        ResourceDefinition * resourceDefinition;

        // Check instance range
        if ((objectInstanceID >= 0) && (objectInstanceID < objectDefinition->MaximumInstances))
        {
            switch (operation)
            {
            case AwaOperation_CreateObjectInstance:
                result = AwaResult_SuccessCreated;
                break;

            case AwaOperation_DeleteObjectInstance:
                result = AwaResult_SuccessDeleted;
                break;

            case AwaOperation_CreateResource:
                result = AwaResult_SuccessCreated;
                break;

            case AwaOperation_DeleteResource:
                result = AwaResult_SuccessDeleted;
                break;

            case AwaOperation_Write:

                resourceDefinition = Definition_LookupResourceDefinitionFromObjectDefinition(objectDefinition, resourceID);
                if (resourceDefinition != NULL)
                {
                    if (resourceDefinition->IsPointerArray)
                    {
                        offset = resourceDefinition->DataPointers + (objectInstanceID * sizeof(void*));
                    }
                    else
                    {
                        offset = resourceDefinition->DataPointers + (resourceDefinition->DataStepSize * objectInstanceID);
                    }

                    if (*dataSize <= resourceDefinition->DataElementSize)
                    {
                        memset(offset, 0, resourceDefinition->DataElementSize);
                        // cppcheck-suppress redundantCopy
                        memcpy(offset, *dataPointer, *dataSize);
                        result = AwaResult_SuccessChanged;
                    }
                    else
                    {
                        result = AwaResult_BadRequest;
                    }

                }
                else
                {
                    result = AwaResult_BadRequest;
                }
                break;

            case AwaOperation_Read:

                resourceDefinition = Definition_LookupResourceDefinitionFromObjectDefinition(objectDefinition, resourceID);
                if (resourceDefinition != NULL)
                {
                    if (resourceDefinition->IsPointerArray)
                    {
                        offset = resourceDefinition->DataPointers + (objectInstanceID * sizeof(void*));
                    }
                    else
                    {
                        offset = resourceDefinition->DataPointers + (resourceDefinition->DataStepSize * objectInstanceID);
                    }

                    *dataPointer = offset;

                    if (resourceDefinition->Type == AwaResourceType_String)
                    {
                        *dataSize = strlen(offset);
                    }
                    else
                    {
                        *dataSize = resourceDefinition->DataElementSize;
                    }
                    result = AwaResult_SuccessContent;
                }
                else
                {
                    result = AwaResult_BadRequest;
                }
                break;

            case AwaOperation_Execute:
            default:
                result = AwaResult_BadRequest;
                break;
            }
        }
        else
        {
            result = AwaResult_BadRequest;
        }
    }
    else
    {
        result = AwaResult_BadRequest;
    }
    return result;
}

AwaError AwaStaticClient_DefineObject(AwaStaticClient * client, AwaObjectID objectID, const char * objectName,
                                      uint16_t minimumInstances, uint16_t maximumInstances)
{
    AwaError result = AwaError_Unspecified;

    if (client != NULL)
    {
        if (client->Initialised)
        {
            if ((objectName != NULL) && (minimumInstances <= maximumInstances))
            {
                ObjectDefinition * definition = Definition_NewObjectTypeWithHandler(objectName, objectID, minimumInstances, maximumInstances, (LWM2MHandler)DefaultHandler);

                if (definition != NULL)
                {
                    if (Definition_AddObjectType(Lwm2mCore_GetDefinitions(client->Context), definition) == 0)
                    {
                        Lwm2mCore_ObjectCreated(client->Context, objectID);
                        result = AwaError_Success;
                    }
                    else
                    {
                        result = AwaError_Internal;
                    }
                }
                else
                {
                    result = AwaError_OutOfMemory;
                }
            }
            else
            {
                result = AwaError_DefinitionInvalid;
            }
        }
        else
        {
            result = AwaError_StaticClientNotInitialized;
        }
    }
    else
    {
        result = AwaError_StaticClientInvalid;
    }

    return result;
}

AwaError AwaStaticClient_SetObjectOperationHandler(AwaStaticClient * client, AwaObjectID objectID, AwaStaticClientHandler handler)
{
    AwaError result = AwaError_Unspecified;

    if (client != NULL)
    {
        if (handler != NULL)
        {
            DefinitionRegistry * registry = Lwm2mCore_GetDefinitions(client->Context);
            if (registry != NULL)
            {
                ObjectDefinition * definition = Definition_LookupObjectDefinition(registry, objectID);
                if (definition != NULL)
                {
                    if (Definition_SetObjectHandler(definition, (LWM2MHandler)handler) == 0)
                    {
                        result = AwaError_Success;
                    }
                    else
                    {
                        result = AwaError_Internal;
                    }
                }
                else
                {
                    result = AwaError_NotDefined;
                }
            }
            else
            {
                result = AwaError_Internal;
            }
        }
        else
        {
            result = AwaError_DefinitionInvalid;
        }
    }
    else
    {
        result = AwaError_StaticClientInvalid;
    }

    return result;
}

const void * AwaStaticClient_GetResourceInstancePointer(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, size_t * resourceSize)
{
    const void * result = NULL;

    if (client != NULL)
    {
        size_t resourceSizeValue = 0;

        Lwm2mCore_GetResourceInstanceValue(client->Context, objectID, objectInstanceID, resourceID, resourceInstanceID, &result, &resourceSizeValue);

        if (resourceSize != NULL)
        {
            *resourceSize = resourceSizeValue;
        }
    }

    return result;
}

AwaError AwaStaticClient_CreateResource(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID)
{
    AwaError result = AwaError_Unspecified;

    if (client != NULL)
    {
        if (Lwm2mCore_CreateOptionalResource(client->Context, objectID, objectInstanceID, resourceID) == 0)
        {
            result = AwaError_Success;
        }
        else
        {
            result = AwaError_CannotCreate;
        }
    }
    else
    {
        result = AwaError_StaticClientInvalid;
    }

    return result;
}

AwaError AwaStaticClient_DeleteResource(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID)
{
    AwaError result = AwaError_Unspecified;

    if (client != NULL)
    {
        if (Lwm2mCore_Delete(client->Context, Lwm2mRequestOrigin_Client, objectID, objectInstanceID, resourceID, false) == AwaResult_SuccessDeleted)
		{
			result = AwaError_Success;
		}
        else
        {
            result = AwaError_CannotDelete;
        }
    }
    else
    {
        result = AwaError_StaticClientInvalid;
    }

    return result;
}

AwaError AwaStaticClient_CreateObjectInstance(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID)
{
    AwaError result;

    if (client != NULL)
    {
        if (Lwm2mCore_CreateObjectInstance(client->Context, objectID, objectInstanceID) != -1)
        {
            result = AwaError_Success;
        }
        else
        {
            result = AwaError_CannotCreate;
        }
    }
    else
    {
        result = AwaError_StaticClientInvalid;
    }

    return result;
}

AwaError AwaStaticClient_DeleteObjectInstance(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID)
{
    AwaError result;

    if (client != NULL)
    {
        if (Lwm2mCore_Delete(client->Context, Lwm2mRequestOrigin_Client, objectID, objectInstanceID, AWA_INVALID_ID, false) == AwaResult_SuccessDeleted)
        {
            result = AwaError_Success;
        }
        else
        {
            result = AwaError_CannotDelete;
        }
    }
    else
    {
        result = AwaError_StaticClientInvalid;
    }

    return result;
}


static AwaError DefineResource(AwaStaticClient * client, const char * resourceName,
                               AwaObjectID objectID, AwaResourceID resourceID, AwaResourceType resourceType,
                               uint16_t minimumInstances, uint16_t maximumInstances, AwaResourceOperations operations,
                               AwaStaticClientHandler handler,  void * dataPointers, bool isPointerArray,
                               size_t dataElementSize, size_t dataStepSize)
{
    AwaError result = AwaError_Unspecified;

    if (client != NULL)
    {
        if (client->Initialised)
        {
            if ((resourceName != NULL) && (minimumInstances <= maximumInstances))
            {
                ObjectDefinition * objFormat = Definition_LookupObjectDefinition(Lwm2mCore_GetDefinitions(client->Context), objectID);
                if (objFormat != NULL)
                {
                    ResourceDefinition * resourceDefinition = Definition_NewResourceTypeWithHandler(objFormat, resourceName, resourceID, resourceType, minimumInstances, maximumInstances, operations, (LWM2MHandler)handler);
                    if (resourceDefinition != NULL)
                    {
                        resourceDefinition->Handler = (LWM2MHandler)handler;
                        resourceDefinition->DataPointers = dataPointers;
                        resourceDefinition->IsPointerArray = isPointerArray;
                        resourceDefinition->DataElementSize = dataElementSize;
                        resourceDefinition->DataStepSize = dataStepSize;
                        result = AwaError_Success;
                    }
                    else
                    {
                        Lwm2m_Warning("resourceDefinition is NULL\n");
                        result = AwaError_DefinitionInvalid;
                    }
                }
                else
                {
                    Lwm2m_Warning("objFormat is NULL\n");
                    result = AwaError_DefinitionInvalid;
                }
            }
            else
            {
                Lwm2m_Warning("One or more Define parameters are invalid\n");
                result = AwaError_DefinitionInvalid;
            }
        }
        else
        {
            result = AwaError_StaticClientNotInitialized;
        }
    }
    else
    {
        result = AwaError_StaticClientInvalid;
    }

    return result;
}

static AwaError SetResourceStorage(AwaStaticClient * client, AwaObjectID objectID, AwaResourceID resourceID,
                                   AwaStaticClientHandler handler,  void * dataPointers, bool isPointerArray,
                                   size_t dataElementSize, size_t dataStepSize)
{
    AwaError result = AwaError_Unspecified;

    if (client != NULL)
    {
        ObjectDefinition * objFormat = Definition_LookupObjectDefinition(Lwm2mCore_GetDefinitions(client->Context), objectID);
        if (objFormat != NULL)
        {
            ResourceDefinition * resourceDefinition = Definition_LookupResourceDefinitionFromObjectDefinition(objFormat, resourceID);
            if (resourceDefinition != NULL)
            {
                resourceDefinition->Handler = (LWM2MHandler)handler;
                resourceDefinition->DataPointers = dataPointers;
                resourceDefinition->IsPointerArray = isPointerArray;
                resourceDefinition->DataElementSize = dataElementSize;
                resourceDefinition->DataStepSize = dataStepSize;
                result = AwaError_Success;
            }
            else
            {
                Lwm2m_Warning("resourceDefinition is NULL\n");
                result = AwaError_DefinitionInvalid;
            }
        }
        else
        {
            Lwm2m_Warning("objFormat is NULL\n");
            result = AwaError_DefinitionInvalid;
        }
    }
    else
    {
        Lwm2m_Warning("client is NULL\n");
        result = AwaError_StaticClientInvalid;
    }

    return result;
}

AwaError AwaStaticClient_DefineResource(AwaStaticClient * client, AwaObjectID objectID, AwaResourceID resourceID,
                                        const char * resourceName, AwaResourceType resourceType,
                                        uint16_t minimumInstances, uint16_t maximumInstances, AwaResourceOperations operations)
{
    return DefineResource(client, resourceName,
                          objectID, resourceID, resourceType,
                          minimumInstances, maximumInstances, operations,
                          NULL, NULL, false, 0, 0);
}

AwaError AwaStaticClient_SetResourceOperationHandler(AwaStaticClient * client, AwaObjectID objectID, AwaResourceID resourceID, AwaStaticClientHandler handler)
{
    AwaError result = AwaError_Unspecified;
    if (client == NULL)
    {
        result = AwaError_StaticClientInvalid;
    }
    else if (handler == NULL)
    {
        result = AwaError_DefinitionInvalid;
    }
    else
    {
        result = SetResourceStorage(client, objectID, resourceID, handler, NULL, false, 0, 0);
    }
    return result;
}

AwaError AwaStaticClient_SetResourceStorageWithPointer(AwaStaticClient * client, AwaObjectID objectID, AwaResourceID resourceID, void * dataPointer, size_t dataElementSize, size_t dataStepSize)
{
    AwaError result = AwaError_Unspecified;
    if (client == NULL)
    {
        result = AwaError_StaticClientInvalid;
    }
    else if ((dataPointer == NULL) || (dataElementSize == 0))
    {
        result = AwaError_DefinitionInvalid;
    }
    else
    {
        result = SetResourceStorage(client, objectID, resourceID, DefaultHandler, dataPointer, false, dataElementSize, dataStepSize);
    }
    return result;
}

AwaError AwaStaticClient_SetResourceStorageWithPointerArray(AwaStaticClient * client, AwaObjectID objectID, AwaResourceID resourceID, void * dataPointers[], size_t dataElementSize)
{
    AwaError result = AwaError_Unspecified;
    if (client == NULL)
    {
        result = AwaError_StaticClientInvalid;
    }
    else if ((dataPointers == NULL) || (dataElementSize == 0))
    {
        result = AwaError_DefinitionInvalid;
    }
    else
    {
        result = SetResourceStorage(client, objectID, resourceID, DefaultHandler, dataPointers, true, dataElementSize, 0);
    }
    return result;
}

AwaError AwaStaticClient_ResourceChanged(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID)
{
    AwaError result = AwaError_Unspecified;

    if (client != NULL)
    {
        ResourceDefinition * definition = Definition_LookupResourceDefinition(Lwm2mCore_GetDefinitions(client->Context), objectID, resourceID);
        if (definition != NULL)
        {
            if (definition->Handler != NULL)
            {
                void * valueBuffer = NULL;
                size_t valueBufferSize = 0;
                int resourceInstanceID = 0;  // Note: This will only work for single-instance resources.
                AwaResult lwm2mResult = AwaResult_Unspecified;
                if ((lwm2mResult = definition->Handler(client, AwaOperation_Read, objectID, objectInstanceID, resourceID, resourceInstanceID, (void **)&valueBuffer, &valueBufferSize, NULL)) == AwaResult_SuccessContent)
                {
                    Lwm2m_MarkObserversChanged(client->Context, objectID, objectInstanceID, resourceID, valueBuffer, valueBufferSize);
                    result = AwaError_Success;
                }
                else
                {
                    Lwm2m_Debug("Read handler for /%d/%d/%d returned %d\n", objectID, objectInstanceID, resourceID, lwm2mResult);
                    result = AwaError_Internal;
                }
            }
            else
            {
                result = AwaError_DefinitionInvalid;
            }
        }
        else
        {
            result = AwaError_DefinitionInvalid;
        }
    }
    else
    {
        result = AwaError_StaticClientInvalid;
    }
    return result;
}

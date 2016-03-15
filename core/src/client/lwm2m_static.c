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

#include "lwm2m_static.h"
#include "lwm2m_security_object.h"
#include "lwm2m_server_object.h"
#include "lwm2m_acl_object.h"
#include "lwm2m_debug.h"

#define MAX_ADDRESS_LENGTH      50

struct _AwaStaticClient
{
    Lwm2mContextType * Context;
    CoapInfo * COAP;
    bool BootstrapConfigured;
    bool EndpointNameConfigured;
    bool COAPConfigured;
    bool Running;
    const char * BootstrapServerURI;
    char COAPListenAddress[MAX_ADDRESS_LENGTH];
    int COAPListenPort;
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
            client->COAPConfigured = false;
            client->Running = false;
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

        if ((*client)->COAP != NULL)
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
        if (client->COAPConfigured && client->BootstrapConfigured && client->EndpointNameConfigured)
        {
            client->COAP = coap_Init(client->COAPListenAddress, client->COAPListenPort, DebugLevel_Debug);

            if (client->COAP != NULL)
            {
                Lwm2mCore_SetCoapInfo(client->Context, client->COAP);
                Lwm2m_RegisterACLObject(client->Context);
                Lwm2m_RegisterServerObject(client->Context);
                Lwm2m_RegisterSecurityObject(client->Context);
                Lwm2m_PopulateSecurityObject(client->Context, client->BootstrapServerURI);
                Lwm2mCore_SetApplicationContext(client->Context, client);
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

AwaError AwaStaticClient_SetBootstrapServerURI(AwaStaticClient * client, const char * bootstrapServerURI)
{
    AwaError result = AwaError_Unspecified;

    if ((client != NULL) && (bootstrapServerURI != NULL))
    {
        if (!client->Running)
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
        if (!client->Running)
        {
            Lwm2mCore_SetFactoryBootstrap(client->Context, (BootstrapInfo*)factoryBootstrapInformation);
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

AwaError AwaStaticClient_SetEndPointName(AwaStaticClient * client, const char * EndPointName)
{
    AwaError result = AwaError_Unspecified;

    if ((client != NULL) && (EndPointName != NULL))
    {
        if (!client->Running)
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

AwaError AwaStaticClient_SetCOAPListenAddressPort(AwaStaticClient * client, const char * address, int port)
{
    AwaError result = AwaError_Unspecified;

    if ((client != NULL) && (address != NULL))
    {
        if (!client->Running)
        {
            if (strlen(address) < MAX_ADDRESS_LENGTH)
            {
                strcpy(client->COAPListenAddress, address);
                client->COAPListenPort = port;
                client->COAPConfigured = true;
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
    int result;
#if !defined (CONTIKI)
    struct pollfd fds[1];
    int nfds = 1;
    int timeout;

    fds[0].fd = client->COAP->fd;
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
        coap_Process();

#else
    result = Lwm2mCore_Process(client->Context);
#endif
    return result;
}

static AwaLwm2mResult AwaStaticClientDefaultHandler(AwaStaticClient * client, AwaOperation operation, AwaObjectID objectID, 
                                                    AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, 
                                                    void ** dataPointer, uint16_t * dataSize, bool * changed)
{
    AwaLwm2mResult result;

    ObjectDefinition * objectDefinition = Definition_LookupObjectDefinition(Lwm2mCore_GetDefinitions(client->Context), objectID);
    if (objectDefinition != NULL)
    {
        uint8_t * offset;
        ResourceDefinition * resourceDefinition;

        // check instance range
        if ((objectInstanceID >= 0) && (objectInstanceID < objectDefinition->MaximumInstances))
        {
            switch (operation)
            {
            case AwaOperation_CreateObjectInstance:
                result = AwaLwm2mResult_SuccessCreated;
                break;

            case AwaOperation_DeleteObjectInstance:
                result = AwaLwm2mResult_SuccessDeleted;
                break;

            case AwaOperation_CreateResource:
                result = AwaLwm2mResult_SuccessCreated;
                break;

            case AwaOperation_DeleteResource:
                break;

            case AwaOperation_Write:
            
                resourceDefinition = Definition_LookupResourceDefinitionFromObjectDefinition(objectDefinition, resourceID);
                if (resourceDefinition != NULL)
                {
                // TODO: check dataSize vs definition->DataElementSize
                // TODO: what do we do about storing the length.. for opaque etc...
                    offset = resourceDefinition->DataPointer + (resourceDefinition->DataStepSize * objectInstanceID);
                    memcpy(offset, *dataPointer, *dataSize);
                    result = AwaLwm2mResult_SuccessChanged;
                }
                else
                {
                    result = AwaLwm2mResult_BadRequest;
                }
                break;

            case AwaOperation_Read:

                resourceDefinition = Definition_LookupResourceDefinitionFromObjectDefinition(objectDefinition, resourceID);
                if (resourceDefinition != NULL)
                {
                    offset = resourceDefinition->DataPointer + (resourceDefinition->DataStepSize * objectInstanceID);
                    *dataPointer = offset;
                    *dataSize = resourceDefinition->DataElementSize;
                    result = AwaLwm2mResult_SuccessContent;
                }
                else
                {
                    result = AwaLwm2mResult_BadRequest;
                }
                break;

            case AwaOperation_Execute:
                result = AwaLwm2mResult_BadRequest;
                break;
            }
        }
        else
        {
            result = AwaLwm2mResult_BadRequest;
        }
    }
    else
    {
        result = AwaLwm2mResult_BadRequest;
    }
    return result;
}

AwaError AwaStaticClient_RegisterObject(AwaStaticClient * client, const char * objectName, AwaObjectID objectID,
                                        uint16_t minimumInstances, uint16_t maximumInstances)
{
    return AwaStaticClient_RegisterObjectWithHandler(client, objectName, objectID, minimumInstances, maximumInstances, AwaStaticClientDefaultHandler);
}

AwaError AwaStaticClient_RegisterObjectWithHandler(AwaStaticClient * client, const char * objectName, AwaObjectID objectID,
                                                     uint16_t minimumInstances, uint16_t maximumInstances,
                                                     AwaStaticClientHandler handler)
{
    AwaError result = AwaError_Unspecified;

    if ((client != NULL) && (objectName != NULL))
    {
        ObjectDefinition * defintion = Definition_NewObjectTypeWithHandler(objectName, objectID, minimumInstances, maximumInstances, (LWM2MHandler)handler);

        if (defintion != NULL)
        {
            if (Definition_AddObjectType(Lwm2mCore_GetDefinitions(client->Context), defintion) == 0)
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
        result = AwaError_StaticClientInvalid;
    }

    return result;
}

AwaError AwaStaticClient_CreateResource(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID)
{
    AwaError result = AwaError_Unspecified;

    if (Lwm2mCore_CreateOptionalResource(client->Context, objectID, objectInstanceID, resourceID) == 0)
    {
        result = AwaError_Success;
    }
    else
    {
        result = AwaError_CannotCreate;
    }

    return result;
}

AwaError AwaStaticClient_CreateObjectInstance(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID)
{
    AwaError result = AwaError_Unspecified;

    if (objectInstanceID == Lwm2mCore_CreateObjectInstance(client->Context, objectID, objectInstanceID))
    {
        result = AwaError_Success;
    }
    else
    {
        result = AwaError_CannotCreate;
    }

    return result;
}

static AwaError AwaStaticClient_RegisterResource(AwaStaticClient * client, const char * resourceName,
                                                 AwaObjectID objectID, AwaResourceID resourceID, AwaStaticResourceType resourceType,
                                                 uint16_t minimumInstances, uint16_t maximumInstances, AwaAccess operations,
                                                 AwaStaticClientHandler handler,  void * dataPointer, size_t dataElementSize, size_t dataStepSize)
{
    AwaError result = AwaError_Unspecified;

    if ((client != NULL) && (resourceName != NULL) && (handler != NULL))
    {
        ObjectDefinition * objFormat = Definition_LookupObjectDefinition(Lwm2mCore_GetDefinitions(client->Context), objectID);
        if (objFormat != NULL)
        {
            ResourceDefinition * resourceDefinition = Definition_NewResourceTypeWithHandler(objFormat, resourceName, resourceID, resourceType, minimumInstances, maximumInstances, operations, (LWM2MHandler)handler);
            if (resourceDefinition != NULL)
            {
                // TODO: perhaps a better option would be to bind some opaque data
                //       to the resource definition, rather than using individual variables.
                resourceDefinition->DataPointer = dataPointer;
                resourceDefinition->DataElementSize = dataElementSize;
                resourceDefinition->DataStepSize = dataStepSize;
                result = AwaError_Success;
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

AwaError AwaStaticClient_RegisterResourceWithPointer(AwaStaticClient * client, const char * resourceName,
                                                     AwaObjectID objectID, AwaResourceID resourceID, AwaStaticResourceType resourceType,
                                                     uint16_t minimumInstances, uint16_t maximumInstances, AwaAccess access,
                                                     void * dataPointer, size_t dataElementSize, size_t dataStepSize)
{
    return AwaStaticClient_RegisterResource(client, resourceName,
                                            objectID, resourceID, resourceType,
                                            minimumInstances, maximumInstances, access, AwaStaticClientDefaultHandler,
                                            dataPointer, dataElementSize, dataStepSize);

}

AwaError AwaStaticClient_RegisterResourceWithHandler(AwaStaticClient * client, const char * resourceName,
                                                     AwaObjectID objectID, AwaResourceID resourceID, AwaStaticResourceType resourceType,
                                                     uint16_t minimumInstances, uint16_t maximumInstances, AwaAccess access,
                                                     AwaStaticClientHandler handler)
{
    return AwaStaticClient_RegisterResource(client, resourceName,
                                            objectID, resourceID, resourceType,
                                            minimumInstances, maximumInstances, access, handler,
                                            NULL, 0 , 0);
}



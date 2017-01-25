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


#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include "lwm2m_endpoints.h"
#include "coap_abstraction.h"
#include "lwm2m_bootstrap.h"
#include "lwm2m_serdes.h"
#include "lwm2m_bootstrap_config.h"
#include "lwm2m_tree_builder.h"
#include "lwm2m_result.h"
#include "lwm2m_objects.h"
#include "lwm2m_util.h"

#define MAX_CLIENTS              (100)
#define MAX_PAYLOAD_LENGTH      (1024)
#define MAX_URI_LENGTH           (512)

#define QUERY_EP_NAME "ep="

typedef struct
{
    bool Used;
    AddressType Addr;
    ObjectIDType ObjectID;
    ObjectInstanceIDType ObjectInstanceID;
    Lwm2mContextType * Context;
    char * EndPointName;
} Lwm2mBootstrapClient;

static Lwm2mBootstrapClient bootStrapQueue[MAX_CLIENTS];

static char * GetEndPointNameFromQuery(const char * query)
{
    char * endPointName = NULL;
    char * str = strdup(query);
    const char delim[] = "&?";
    char * token = strtok(str, delim);
    while (token != NULL)
    {
        if (endPointName != NULL)
        {
            free(endPointName);
            endPointName = NULL;
        }

        // Find end point name
        if (strncmp(token, QUERY_EP_NAME, strlen(QUERY_EP_NAME)) == 0)
        {
            endPointName = strdup(token + strlen(QUERY_EP_NAME));
        }
        token = strtok(NULL, delim);
    }
    free(str);
    return endPointName;
}

static void Lwm2mBootstrap_AddClientToQueue(Lwm2mBootstrapClient * client)
{
    int i;
    for (i = 0 ; i < MAX_CLIENTS; i++)
    {
        if (!bootStrapQueue[i].Used)
        {
            memcpy(&bootStrapQueue[i], client, sizeof(*client));
            bootStrapQueue[i].Used = true;
            break;
        }
    }
}

static bool Lwm2mBootstrap_AddServerValues(Lwm2mContextType * context, const char * config)
{
    bool result = false;
    if (config != NULL)
    {
        const BootstrapInfo * bootstrapInfo = BootstrapInformation_ReadConfigFile(config);
        if ((bootstrapInfo == NULL) || (BootstrapInformation_Apply(context, bootstrapInfo) != 0))
        {
            Lwm2m_Error("Configuration file load failed\n");
        }
        else
        {
            Lwm2m_Info("Server Configuration\n");
            Lwm2m_Info("====================\n");
            BootstrapInformation_Dump(bootstrapInfo);
            result = true;
        }
        BootstrapInformation_DeleteBootstrapInfo(bootstrapInfo);
    }
    return result;
}

static int EndpointHandler(int type, void * ctxt, AddressType * addr, const char * path, const char * query,
                           const char * token, int tokenLength, AwaContentType contentType, const char * requestContent,
                           size_t requestContentLen, AwaContentType * responseContentType, char * responseContent,
                           size_t * responseContentLen, int * responseCode)
{
    *responseContentType = AwaContentType_None;
    *responseContentLen = 0;

    switch (type)
    {
        case COAP_PUT_REQUEST:
        case COAP_POST_REQUEST:
        {
            Lwm2m_Debug("%s to /bs, query %s\n", (type == COAP_POST_REQUEST) ? "POST" : "PUT", query);

            Lwm2mBootstrapClient client;

            memcpy(&client.Addr, addr, sizeof(client.Addr));
            client.ObjectID = LWM2M_SECURITY_OBJECT;
            client.ObjectInstanceID = -1;
            client.Context = ctxt;
            client.EndPointName = GetEndPointNameFromQuery(query);

            Lwm2mBootstrap_AddClientToQueue(&client);

            *responseCode = AwaResult_SuccessChanged;
            break;
        }

        default:
            *responseCode = AwaResult_MethodNotAllowed;
            break;
    }
    return 0;
}

static void BootstrapTransactionCallback(void * context, AddressType * addr, const char * responsePath, int responseCode, AwaContentType contentType, char * payload, size_t payloadLen)
{
    if ((responseCode >= AwaResult_Success) && (responseCode <= AwaResult_SuccessContent))
    {
        Lwm2mBootstrapClient * client = (Lwm2mBootstrapClient *)context;
        char uri[MAX_URI_LENGTH];
        char server[MAX_URI_LENGTH];

        Lwm2mCore_AddressTypeToPath(server, MAX_URI_LENGTH, addr);

        // Get the next security/server object to send
        if ((client->ObjectID == LWM2M_SECURITY_OBJECT) || (client->ObjectID == 1))
        {
            client->ObjectInstanceID = Lwm2mCore_GetNextObjectInstanceID(client->Context, client->ObjectID, client->ObjectInstanceID);

            // If iterated through all security instances then progress to server instances
            if ((client->ObjectInstanceID == -1) && (client->ObjectID == LWM2M_SECURITY_OBJECT))
            {
                client->ObjectID = LWM2M_SERVER_OBJECT;
                client->ObjectInstanceID = Lwm2mCore_GetNextObjectInstanceID(client->Context, client->ObjectID, client->ObjectInstanceID);
            }

            // If object instance is -1 by this stage there are no more objects to iterate through
            if (client->ObjectInstanceID == -1)
            {
                client->ObjectID = -1;
            }
        }

        if ((client->ObjectID != -1) && (client->ObjectInstanceID != -1))
        {
            char payload[MAX_PAYLOAD_LENGTH];
            int payloadLen;
            Lwm2mTreeNode * objectInstance;

            // Write object, with callback to write next object on success
            sprintf(uri, "%s/%d", server, client->ObjectID); // Since we are creating a new object instance, we must post to the object level
            TreeBuilder_CreateTreeFromObjectInstance(&objectInstance, client->Context, Lwm2mRequestOrigin_BootstrapServer, client->ObjectID, client->ObjectInstanceID);

            // Wrap the object instance in an object node
            Lwm2mTreeNode * object = Lwm2mTreeNode_Create();
            Lwm2mTreeNode_SetType(object, Lwm2mTreeNodeType_Object);
            Lwm2mTreeNode_SetID(object, client->ObjectID);
            Lwm2mTreeNode_AddChild(object, objectInstance);

            payloadLen = SerialiseObject(AwaContentType_ApplicationOmaLwm2mTLV, object, client->ObjectID, payload, sizeof(payload));
            Lwm2mTreeNode_DeleteRecursive(object);

            Lwm2m_Debug("Put to %s\n", uri);
            coap_PutRequest(context, uri, AwaContentType_ApplicationOmaLwm2mTLV, payload, payloadLen, BootstrapTransactionCallback);
        }
        else
        {
            // No more objects so write to /bs to indicate bootstrap complete
            sprintf(uri, "%s/%s", server, "/bs");
            Lwm2m_Debug("Post to %s\n", uri);
            Lwm2m_Info("Client bootstrapped: %s, \'%s\'\n", server, client->EndPointName);
            coap_PostRequest(context, uri, AwaContentType_None, NULL, 0, NULL);

            // Delete the client record
            free(client->EndPointName);
            memset(client, 0, sizeof(*client));
        }
    }
}

void Lwm2mBootstrap_BootStrapUpdate(Lwm2mContextType * context)
{
    // Loop through bootstrap request queue and send responses for each client
    int i;
    for (i = 0 ; i < MAX_CLIENTS; i++)
    {
        // Kick start sending bootstrap with callback
        if (bootStrapQueue[i].Used && (bootStrapQueue[i].ObjectID == 0) && (bootStrapQueue[i].ObjectInstanceID == -1))
        {
            BootstrapTransactionCallback(&bootStrapQueue[i], &bootStrapQueue[i].Addr, NULL, AwaResult_Success, 0, NULL, 0);
        }
    }
}

// Initialise the boot strap mechanism, create the /bs endpoint
bool Lwm2mBootstrap_BootStrapInit(Lwm2mContextType * context, const char ** config, int configCount)
{
    bool success = true;
    int i = 0;

    memset(bootStrapQueue, 0, sizeof(bootStrapQueue));
    Lwm2mCore_AddResourceEndPoint(context, "/bs", EndpointHandler);

    for (i = 0; i < configCount; i++)
    {
        if ((success = Lwm2mBootstrap_AddServerValues(context, config[i])) == false)
        {
            Lwm2m_Error("Failed to initialise bootstrap config for server %d\n", i);
            break;
        }
    }
    return success;
}

void Lwm2mBootstrap_Destroy(void)
{
    int i;
    for (i = 0 ; i < MAX_CLIENTS; i++)
    {
        if (bootStrapQueue[i].Used)
        {
            free(bootStrapQueue[i].EndPointName);
            memset(&bootStrapQueue[i], 0, sizeof(bootStrapQueue[i]));
        }
    }
}

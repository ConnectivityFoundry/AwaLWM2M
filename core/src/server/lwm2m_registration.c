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

#include "lwm2m_core.h"
#include "lwm2m_result.h"
#include "lwm2m_endpoints.h"
#include "server/lwm2m_registration.h"
#include "lwm2m_events.h"
#include "../../api/src/ipc_defs.h"

#define QUERY_EP_NAME  "ep="
#define QUERY_LIFETIME "lt="
#define QUERY_BINDING  "b="

typedef struct
{
    const char * EndPointName;
    int LifeTime;
    BindingMode BindingModeValue;

} RegistrationQueryString;

typedef struct
{
    struct ListHead list;
    IPCSessionID SessionID;
    RegistrationEventCallback Callback;
    void * Context;

} EventRecord;

static void DestroyObjectList(struct ListHead * objectList);

static int RegistrationEndpointHandler(int type, void * ctxt, AddressType * addr, const char * path, const char * query, const char * token,
                                       int tokenLength, AwaContentType contentType, const char * requestContent, size_t requestContentLen,
                                       AwaContentType * responseContentType, char * responseContent, size_t * responseContentLen, int * responseCode);

static int UpdateEndpointHandler(int type, void * ctxt, AddressType * addr, const char * path, const char * query, const char * token,
                                 int tokenLength, AwaContentType contentType, const char * requestContent, size_t requestContentLen,
                                 AwaContentType * responseContentType, char * responseContent, size_t * responseContentLen, int * responseCode);

static void Lwm2m_SplitUpQuery(const char * query, RegistrationQueryString * result)
{
    memset(result, 0, sizeof(RegistrationQueryString));

    char * str = strdup(query);

    const char delim[] = "&?";

    char * token = strtok(str, delim);

    while (token != NULL)
    {
        // find end point name
        if (strncmp(token, QUERY_EP_NAME, strlen(QUERY_EP_NAME)) == 0)
        {
            result->EndPointName = strdup(token + strlen(QUERY_EP_NAME));
        }
        else if (strncmp(token, QUERY_LIFETIME, strlen(QUERY_LIFETIME)) == 0)
        {
            sscanf(token, QUERY_LIFETIME"%10d", &result->LifeTime);
        }
        else if (strncmp(token, QUERY_BINDING, strlen(QUERY_BINDING)) == 0)
        {
            char bindingStr[10] = {0};

            sscanf(token, QUERY_BINDING"%5s", bindingStr);

            if (strcmp(bindingStr, "U") == 0)
            {
                result->BindingModeValue = BindingMode_Udp;
            }
            else
            {
                Lwm2m_Error("Unsupported binding mode %s, using \"U\" instead\n", bindingStr);
                result->BindingModeValue = BindingMode_Udp;
            }
        }
        token = strtok(NULL, delim);
    }

    free(str);
}

bool Lwm2m_ClientSupportsObject(Lwm2mClientType * client, ObjectIDType objectID, ObjectInstanceIDType instanceID)
{
    bool clientSupportsObject = false;
    struct ListHead * i;
    ListForEach(i, &client->ObjectList)
    {
        ObjectListEntry * entry = ListEntry(i, ObjectListEntry, list);
        if ((entry->ObjectID == objectID) &&
            ((entry->InstanceID == instanceID) || (instanceID == -1)))
        {
            clientSupportsObject = true;
            break;
        }
    }
    return clientSupportsObject;
}

// parse object list in "CoRE" format
static void Lwm2m_ParseObjectList(Lwm2mClientType * client, const char * objectList, int objectListLength)
{
    // clear all entries out of object list
    struct ListHead * i, * n;
    ListForEachSafe(i, n, &client->ObjectList)
    {
        ObjectListEntry * entry = ListEntry(i, ObjectListEntry, list);
        ListRemove(i);
        free(entry);
    }

    if ((objectListLength > 0) && (objectList != NULL))
    {
        char altPath[128];
        strcpy(altPath, "/"); // Assume root path is "/" until proven otherwise

        char * str = strndup(objectList, objectListLength);
        const char delim[] = ", ";
        char * savePointer;

        char * token = strtok_r(str, delim, &savePointer);
        while (token != NULL)
        {
            char * savePointer2;

            // Parse token into O/I
            int object, instance = -1;

            char * objectStr = strtok_r(token, ";", &savePointer2);
            if (objectStr != NULL)
            {
                char * attribute = strtok_r(NULL, ";", &savePointer2);
                while (attribute != NULL)
                {
                    int contentType;
                    char resourceType[128];

                    if (sscanf(attribute, "ct=%10d", &contentType))
                    {
                        // If the LWM2M Client supports the JSON data format for all the objects it should inform the LWM2M server
                        // by including the content type in the root path link using the ct= link attribute.
                        if ((contentType == AwaContentType_ApplicationOmaLwm2mJson) && (!strcmp(objectStr, "</>")))
                        {
                            client->SupportsJson = true;
                            Lwm2m_Info("Supports JSON\n");
                        }
                    }
                    else if (sscanf(attribute, "rt=%127s", resourceType))
                    {
                        if (!strcmp(resourceType, "oma.lwm2m"))
                        {
                            sscanf(objectStr, "<%127s>", altPath);
                        }
                        else
                        {
                            Lwm2m_Info("unknown resource type %s, skipping %s\n", resourceType, objectStr);
                            goto skip;
                        }
                    }
                    attribute = strtok_r(NULL, ";", &savePointer2);
                }

                int pos = sscanf(objectStr, "</%5d/%5d>", &object, &instance);
                if (pos <= 0)
                {
                    token = strtok_r(NULL, delim, &savePointer);
                    continue;
                }

                // Create new entry, perhaps check for duplicates?
                ObjectListEntry * entry = malloc(sizeof(ObjectListEntry));
                if (entry == NULL)
                {
                    break;
                }

                entry->ObjectID = object;
                entry->InstanceID = instance;

                ListAdd(&entry->list, &client->ObjectList);
            }

        skip:
            token = strtok_r(NULL, delim, &savePointer);
        }

        free(str);

        // Debug, printout list
        ListForEach(i, &client->ObjectList)
        {
            ObjectListEntry * entry = ListEntry(i, ObjectListEntry, list);
            if (entry->InstanceID != -1)
            {
                Lwm2m_Info("Path %s Object %d, Instance %d\n", altPath, entry->ObjectID, entry->InstanceID);
            }
            else
            {
                Lwm2m_Info("Path %s Object %d\n", altPath, entry->ObjectID);
            }
        }
    }
}

static void Lwm2m_ReleaseQueryString(RegistrationQueryString * queryString)
{
    if (queryString != NULL)
    {
        free((char*)queryString->EndPointName);
    }
}

Lwm2mClientType * Lwm2m_LookupClientByName(Lwm2mContextType * context, const char * endPointName)
{
    Lwm2mClientType * client = NULL;
    struct ListHead * i;
    ListForEach(i, Lwm2mCore_GetClientList(context))
    {
        Lwm2mClientType * c = ListEntry(i, Lwm2mClientType, list);
        if (strcmp(c->EndPointName, endPointName) == 0)
        {
            client = c;
            break;
        }
    }
    return client;
}

static Lwm2mClientType * Lwm2m_LookupClientByLocation(Lwm2mContextType * context, int location)
{
    Lwm2mClientType * client = NULL;
    struct ListHead * i;
    ListForEach(i, Lwm2mCore_GetClientList(context))
    {
        Lwm2mClientType * c = ListEntry(i, Lwm2mClientType, list);
        if (c->Location == location)
        {
            client = c;
            break;
        }
    }
    return client;
}

Lwm2mClientType * Lwm2m_LookupClientByAddress(Lwm2mContextType * context, AddressType * address)
{
    Lwm2mClientType * client = NULL;
    struct ListHead * i;
    ListForEach(i, Lwm2mCore_GetClientList(context))
    {
        Lwm2mClientType * c = ListEntry(i, Lwm2mClientType, list);

        if (memcmp(c->Address.Addr.Sa.sa_data, address->Addr.Sa.sa_data, sizeof(c->Address.Addr.Sa.sa_data)) == 0)
        {
            client = c;
            break;
        }
    }
    return client;
}

static void DispatchRegistrationEventCallbacks(Lwm2mContextType * lwm2mContext, RegistrationEventType eventType, void * parameter)
{
    struct ListHead * eventRecordList = Lwm2mCore_GetEventRecordList(lwm2mContext);
    if (eventRecordList != NULL)
    {
        struct ListHead * i;
        ListForEach(i, eventRecordList)
        {
            EventRecord * eventRecord = ListEntry(i, EventRecord, list);
            if (eventRecord != NULL)
            {
                if (eventRecord->Callback != NULL)
                {
                    Lwm2m_Debug("Calling registration event callback with type %d, context %p\n", eventType, eventRecord->Context)
                    eventRecord->Callback(eventType, eventRecord->Context, parameter);
                }
            }
        }
    }
}

static int Lwm2m_UpdateClient(Lwm2mContextType * context, int location, int lifeTime, BindingMode bindingMode,
                              AddressType * addr, AwaContentType contentType, const char * objectList, int objectListLength,
                              RegistrationEventType registrationEventType)
{
    int result = -1;
    Lwm2mClientType * client = Lwm2m_LookupClientByLocation(context, location);
    if (client)
    {
        uint32_t now = Lwm2mCore_GetTickCountMs();

        if (lifeTime > 0)
        {
            client->LifeTime = lifeTime;
        }
        else
        {
            client->LifeTime = LIFETIME_DEFAULT;
        }

        memcpy(&client->Address, addr, sizeof(AddressType));

        if (contentType == AwaContentType_ApplicationLinkFormat)
        {
            Lwm2m_ParseObjectList(client, objectList, objectListLength);
        }

        client->LastUpdateTime = now;

        DispatchRegistrationEventCallbacks(context, registrationEventType, client);

        result = 0;
    }
    else
    {
        Lwm2m_Error("Client not registered at location /rd/%d\n", location);
    }
    return result;
}

static int Lwm2m_RegisterClient(Lwm2mContextType * context, const char * endPointName, int lifeTime, BindingMode bindingMode,
                                AddressType * addr, AwaContentType contentType, const char * objectList, int objectListLength)
{
    int result = -1;
    Lwm2mClientType * client = Lwm2m_LookupClientByName(context, endPointName);
    if (client == NULL)
    {
        client = malloc(sizeof(Lwm2mClientType));
        if (client != NULL)
        {
            char RegisterLocation[128] = {0};

            client->EndPointName = strdup(endPointName);
            client->BindingMode = bindingMode;
            client->SupportsJson = false;

            client->Location = Lwm2mCore_GetLastLocation(context) + 1;
            Lwm2mCore_SetLastLocation(context, client->Location);

            ListInit(&client->ObjectList);

            ListAdd(&client->list, Lwm2mCore_GetClientList(context));

            sprintf(RegisterLocation, "/rd/%d", client->Location);
            Lwm2mCore_AddResourceEndPoint(context, RegisterLocation, UpdateEndpointHandler);

            result = Lwm2m_UpdateClient(context, client->Location, lifeTime, bindingMode, addr, contentType, objectList, objectListLength, RegistrationEventType_Register);

            Lwm2m_Info("Client registered: \'%s\'\n", endPointName);
        }
        else
        {
            Lwm2m_Error("Failed to allocate memory for Client entry\n");
        }
    }
    else
    {
        Lwm2m_Error("Client already registered\n");
    }
    return result;
}

static void Lwm2m_DeregisterClient(Lwm2mContextType * context, Lwm2mClientType * client)
{
    char RegisterLocation[128] = {0};

    ListRemove(&client->list);
    DestroyObjectList(&client->ObjectList);

    sprintf(RegisterLocation, "/rd/%d", client->Location);
    Lwm2mCore_RemoveResourceEndPoint(context, RegisterLocation);

    Lwm2m_Info("Client deregistered: \'%s\'\n", client->EndPointName);

    DispatchRegistrationEventCallbacks(context, RegistrationEventType_Deregister, client);

    free(client->EndPointName);
    free(client);
}

// handler called when a client posts to /rd
static int Lwm2m_RegisterPost(void * ctxt, AddressType * addr, const char * path,
                              const char * query, AwaContentType contentType, 
                              const char * requestContent, size_t requestContentLen,
                              char * responseContent, size_t * responseContentLen,
                              int * responseCode)
{
    Lwm2mContextType * context = (Lwm2mContextType*)ctxt;
    RegistrationQueryString q;
    Lwm2mClientType * client;

    Lwm2m_Debug("Lwm2m_RegisterPost %s %s\n", path, query);

    Lwm2m_SplitUpQuery(query, &q);

    if (q.EndPointName == NULL)
    {
        *responseContentLen = 0;
        *responseCode = AwaResult_BadRequest;
        goto done;
    }

    // Check object list is supplied, or at least an empty list with the type application-link
    if (contentType != AwaContentType_ApplicationLinkFormat)
    {
        *responseContentLen = 0;
        *responseCode = AwaResult_BadRequest;
        goto done;
    }

    /* If the LWM2M Client sends a “Register” operation to the LWM2M Server even though the LWM2M Server has registration
     * information of the LWM2M Client, the LWM2M Server removes the existing registration information and performs the
     * new “Register” operation. This situation happens when the LWM2M Client forgets the state of the LWM2M Server (e.g., factory reset).
     */
    if ((client = Lwm2m_LookupClientByName(context, q.EndPointName)) != NULL)
    {
        // Check to see if this is a re-register from the same address, otherwise treat as a duplicate.
        if ((addr->Size == client->Address.Size) && (memcmp(addr, &client->Address, addr->Size) == 0))
        {
            Lwm2m_Info("Client \'%s\' already registered, deleting\n", q.EndPointName);
            Lwm2m_DeregisterClient(context, client);
        }
    }

    if (Lwm2m_RegisterClient(context, q.EndPointName, q.LifeTime, q.BindingModeValue, addr, contentType, requestContent, requestContentLen) == 0)
    {
        Lwm2mClientType * client = Lwm2m_LookupClientByName(context, q.EndPointName);

        sprintf(responseContent, "rd/%d", client->Location);
    
        *responseContentLen = strlen(responseContent);  // no content
        *responseCode = AwaResult_SuccessCreated;
    }
    else
    {
        *responseContentLen = 0;
        *responseCode = AwaResult_Forbidden;  // Duplicate
    }

    Lwm2m_ReleaseQueryString(&q);

done:
    return 0;
}

// handler called when a client puts to /rd/<location>
static int RegisterPut(void * ctxt, AddressType * addr, const char * path,
                       const char * query, AwaContentType contentType,
                       const char * requestContent, size_t requestContentLen,
                       char * responseContent, size_t * responseContentLen,
                       int * responseCode)
{
    Lwm2mContextType * context = (Lwm2mContextType*)ctxt;
    RegistrationQueryString q;

    int32_t location;

    Lwm2m_Debug("Lwm2m_RegisterPut %s %s\n", path, query);

    *responseContentLen = 0;

    if (sscanf(path, "/rd/%10d\n", &location) != 1)
    {
        *responseCode = AwaResult_BadRequest;
        goto done;
    }

    Lwm2m_SplitUpQuery(query, &q);

    if (Lwm2m_UpdateClient(context, location, q.LifeTime, q.BindingModeValue, addr, contentType, requestContent, requestContentLen, RegistrationEventType_Update) == 0)
    {
        *responseCode = AwaResult_SuccessChanged;
    }
    else
    {
        *responseCode = AwaResult_NotFound;
    }

    Lwm2m_ReleaseQueryString(&q);
done:
    return 0;
}

// handler called when a client sends a deregister by sending a DELETE to /rd/X
static int RegisterDelete(void * ctxt, AddressType * addr, const char * path, const char * query, AwaContentType contentType,
                          const char * requestContent, size_t requestContentLen, char * responseContent, size_t * responseContentLen, int * responseCode)
{
    Lwm2mContextType * context = (Lwm2mContextType*)ctxt;

    int32_t location;

    Lwm2m_Debug("Lwm2m_RegisterDelete %s %s\n", path, query);

    *responseContentLen = 0;

    if (sscanf(path, "/rd/%10d\n", &location) != 1)
    {
        *responseCode = AwaResult_BadRequest;
        goto done;
    }

    Lwm2mClientType * client = Lwm2m_LookupClientByLocation(context, location);
    if (client == NULL)
    {
        *responseCode = AwaResult_NotFound;
    }
    else
    {
        Lwm2m_DeregisterClient(context, client);
        *responseCode = AwaResult_SuccessDeleted;
    }
done:
    return 0;
}


/* This function is called when a CoAP request is made to any endpoints created during the registration phase.
 * i.e when a Registration message is handled, a new endpoint is created for the client /rd/<location>. these
 * end points are handled here.
 */
static int UpdateEndpointHandler(int type, void * ctxt, AddressType * addr, const char * path, const char * query, const char * token,
                                 int tokenLength, AwaContentType contentType, const char * requestContent, size_t requestContentLen,
                                 AwaContentType * responseContentType, char * responseContent, size_t * responseContentLen, int * responseCode)
{
    switch(type)
    {
        // The old standard used to use PUT for update requests, so we must continue to support this
        case COAP_PUT_REQUEST:
            return RegisterPut(ctxt, addr, path, query, contentType, requestContent, requestContentLen, responseContent, responseContentLen, responseCode);

        case COAP_POST_REQUEST:
            return RegisterPut(ctxt, addr, path, query, contentType, requestContent, requestContentLen, responseContent, responseContentLen, responseCode);

        case COAP_DELETE_REQUEST:
            return RegisterDelete(ctxt, addr, path, query, contentType, requestContent, requestContentLen, responseContent, responseContentLen, responseCode);

        default:
            break;
    }

    *responseContentType = AwaContentType_None;
    *responseContentLen = 0;
    *responseCode = AwaResult_MethodNotAllowed;
    return 0;
}

// This function is called when a CoAP request is made to /rd
static int RegistrationEndpointHandler(int type, void * ctxt, AddressType * addr, const char * path, const char * query, const char * token,
                                       int tokenLength, AwaContentType contentType, const char * requestContent, size_t requestContentLen,
                                       AwaContentType * responseContentType, char * responseContent, size_t * responseContentLen, int * responseCode)
{
    switch (type)
    {
        case COAP_POST_REQUEST:
            return Lwm2m_RegisterPost(ctxt, addr, path, query, contentType, requestContent, requestContentLen, responseContent, responseContentLen, responseCode);
        default:
            break;
    }

    *responseContentType = AwaContentType_None;
    *responseContentLen = 0;
    *responseCode = AwaResult_MethodNotAllowed;
    return 0;
}

int32_t Lwm2m_AgeRegistrations(Lwm2mContextType * context)
{
    uint32_t now = Lwm2mCore_GetTickCountMs();
    struct ListHead * i, *n;

    ListForEachSafe(i, n, Lwm2mCore_GetClientList(context))
    {
        Lwm2mClientType * client = ListEntry(i, Lwm2mClientType, list);

        if ((now - client->LastUpdateTime) > (client->LifeTime * 1000))
        {
            Lwm2m_Error("Client \'%s\' Lifetime Expired\n", client->EndPointName);

            Lwm2m_DeregisterClient(context, client);
        }
    }
    return 0;
}

int Lwm2m_RegistrationInit(Lwm2mContextType * context)
{
    // Initialise client list
    ListInit(Lwm2mCore_GetClientList(context));
    Lwm2mCore_SetLastLocation(context, 0);

    Lwm2mCore_AddResourceEndPoint(context, "/rd", RegistrationEndpointHandler);

    ListInit(Lwm2mCore_GetEventRecordList(context));

    return 0;
}

static void DestroyObjectList(struct ListHead * objectList)
{
    if (objectList != NULL)
    {
        struct ListHead * i, * n;
        ListForEachSafe(i, n, objectList)
        {
            ObjectListEntry * object = ListEntry(i, ObjectListEntry, list);
            if (object != NULL)
            {
                free(object);
            }
        }
    }
}

static void DestroyClientList(struct ListHead * clientList)
{
    if (clientList != NULL)
    {
        struct ListHead * i, * n;
        ListForEachSafe(i, n, clientList)
        {
            Lwm2mClientType * client = ListEntry(i, Lwm2mClientType, list);
            if (client != NULL)
            {
                DestroyObjectList(&client->ObjectList);
                free(client->EndPointName);
                free(client);
            }
        }
    }
}

// Note: this will free all stored callback contexts
static void DestroyEventList(struct ListHead * eventRecordList)
{
    if (eventRecordList != NULL)
    {
        struct ListHead * i, * n;
        ListForEachSafe(i, n, eventRecordList)
        {
            EventRecord * eventRecord = ListEntry(i, EventRecord, list);
            if (eventRecord != NULL)
            {
                free(eventRecord->Context);
                eventRecord->Context = NULL;
                free(eventRecord);
            }
        }
    }
}

void Lwm2m_RegistrationDestroy(Lwm2mContextType * context)
{
    DestroyClientList(Lwm2mCore_GetClientList(context));
    DestroyEventList(Lwm2mCore_GetEventRecordList(context));
}

void * Lwm2m_GetEventContext(Lwm2mContextType * lwm2mContext, IPCSessionID sessionID)
{
    void * result = NULL;
    if (lwm2mContext != NULL)
    {
        struct ListHead * i;
        struct ListHead * eventRecordList = Lwm2mCore_GetEventRecordList(lwm2mContext);
        ListForEach(i, eventRecordList)
        {
            EventRecord * eventRecord = ListEntry(i, EventRecord, list);
            if (eventRecord != NULL)
            {
                if (eventRecord->SessionID == sessionID)
                {
                    result = eventRecord->Context;
                    break;
                }
            }
        }
    }
    return result;
}

// Note: callbackContext *MUST* be heap-allocated, as it will be freed later.
int Lwm2m_AddRegistrationEventCallback(Lwm2mContextType * lwm2mContext, IPCSessionID sessionID, RegistrationEventCallback callback, void * callbackContext)
{
    int result = -1;
    EventRecord * eventRecord = malloc(sizeof(*eventRecord));
    if (eventRecord != NULL)
    {
        memset(eventRecord, 0, sizeof(*eventRecord));
        eventRecord->SessionID = sessionID;
        eventRecord->Callback = callback;
        eventRecord->Context = callbackContext;
        ListAdd(&eventRecord->list, Lwm2mCore_GetEventRecordList(lwm2mContext));
        Lwm2m_Debug("Added EventRecord %d: %p\n", sessionID, eventRecord);
        result = 0;
    }
    else
    {
        Lwm2m_Error("Out of memory\n");
        result = -1;
    }
    return result;
}

// Note: this will free the stored callback context
int Lwm2m_DeleteRegistrationEventCallback(Lwm2mContextType * lwm2mContext, IPCSessionID sessionID)
{
    int result = -1;
    if (lwm2mContext != NULL)
    {
        struct ListHead * i, * n;
        struct ListHead * eventRecordList = Lwm2mCore_GetEventRecordList(lwm2mContext);
        ListForEachSafe(i, n, eventRecordList)
        {
            EventRecord * eventRecord = ListEntry(i, EventRecord, list);
            if (eventRecord != NULL)
            {
                if (eventRecord->SessionID == sessionID)
                {
                    ListRemove(i);
                    free(eventRecord->Context);
                    eventRecord->Context = NULL;
                    Lwm2m_Debug("Removed EventRecord %d: %p\n", sessionID, eventRecord);
                    free(eventRecord);
                }
            }
        }
        result = 0;
    }
    else
    {
        Lwm2m_Error("lwm2mContext is NULL\n");
        result = -1;
    }
    return result;
}


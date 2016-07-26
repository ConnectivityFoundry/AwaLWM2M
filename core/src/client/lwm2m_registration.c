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



// The registration interface is used by a LWM2M Client to register with a LWM2M Server, identified by
// the LWM2M Server URI. Registration is performed by sending a CoAP POST to the LWM2M Server URI, with
// registration parameters passed as query string parameters as per Table 19 and Object and Object Instances
// included in the payload as specified in Section 5.2.1. The response includes Location-Path Options,
// which indicate the path to use for updating or deleting the registration. The server MUST return
// a location under the /rd path segment.
//
// Registration update is performed by sending a CoAP PUT to the Location path returned to the LWM2M
// Client as a result of a successful registration.
// De-registration is performed by sending a CoAP DELETE to the Location path returned to the LWM2M
// Client as a result of a successful registration.
//
//
//   LWM2M Client                               LWM2M Server
//     Registration ----POST/rd?ep=example------>
//                  <1/1>,<1/2>,<2/0>,<2/1>,<3/0>
//                  <---2.01 Created /rd/5a3f----
//
//     Update       ----PUT/rd/5a3f?lt=60000---->
//                  <-------2.04 Changed---------
//
//     De-register  -----DELETE/rd/5a3f--------->
//                  <-------2.02 Deleted---------


#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "coap_abstraction.h"
#include "lwm2m_object_store.h"
#include "lwm2m_registration.h"
#include "lwm2m_objects.h"
#include "lwm2m_security_object.h"
#include "lwm2m_core.h"
#include "lwm2m_server_object.h"
#include "lwm2m_request_origin.h"


#define REGISTRATION_RETRY_ATTEMPTS (10)
#define REGISTRATION_TIMEOUT        (30000)


static void HandleRegisterUpdateResponse(void * ctxt, AddressType* address, const char * responsePath, int responseCode, AwaContentType contentType, char * payload, size_t payloadLen);
static void HandleRegisterResponse(void * ctxt, AddressType* address, const char * responsePath, int responseCode, AwaContentType contentType, char * payload, size_t payloadLen);
static void HandleDeregisterResponse(void * ctxt, AddressType* address, const char * responsePath, int responseCode, AwaContentType contentType, char * payload, size_t payloadLen);


static int GetTransportBinding(Lwm2mContextType * context, int shortServerID, char * buffer, size_t len)
{
    // Get binding from Server object in object store as a string
    enum { BINDING_MAX_SIZE = 16 };
    char binding[BINDING_MAX_SIZE];
   
    int res = Lwm2mServerObject_GetTransportBinding(context, shortServerID, binding, sizeof(binding));
    if (res < 0)
    {
        Lwm2m_Error("binding mode not specified\n");
        return res;
    }

    return snprintf(buffer, len, "&b=%s", binding);
}

static void GetQueryString(Lwm2mContextType * context, int shortServerID, char * buffer, size_t len)
{
    int pos = 0;
    char temp[128];
    int32_t lifetime;

    // First get endpoint
    Lwm2mCore_GetEndPointClientName(context, temp, sizeof(temp));
    pos += sprintf(buffer, "?ep=%s", temp);

    // Add life time
    lifetime = Lwm2mServerObject_GetLifeTime(context, shortServerID);
    if (lifetime > 0)
    {
        pos += snprintf(buffer + pos, len - pos, "&lt=%d", lifetime);
    }
    // Add transport binding
    pos += GetTransportBinding(context, shortServerID, buffer + pos, len - pos);
}


// Registration is performed when a LWM2M Client sends a “Register” operation to the LWM2M Server.
// After the LWM2M Device is turned on and the bootstrap procedure has been completed, the LWM2M Client
// MUST perform a “Register” operation to each LWM2M Server that the LWM2M Client has a Server Object Instance.
static void SendRegisterRequest(Lwm2mContextType * context, Lwm2mServerType * server)
{
    // POST, Uri-Path: "rd", Uri-Query: "ep={ClientName}&lt={LifeTime}&sms={msisdn}&lwm2m={version}&b={binding}"
    //       PayLoad: </0/0>,</0/1>....
    // success 2.01 Created
    // failure, 4.00 Bad request, 4.09 Conflict
    char uriQuery[1024];
    char payload[1024];
    char * uriPath = "/rd";
    char serverUri[255];
    char uri[1024];

    if (!Lwm2mCore_IsNetworkAddressRevolved(context, server->ShortServerID))
    {
        Lwm2m_Debug("Registration: network address not yet revolved\n");
        return;
    }

    if (Lwm2m_GetServerURI(context, server->ShortServerID, serverUri, sizeof(serverUri)) < 0)
    {
        Lwm2m_Error("Registration: Server URI is not specified\n");
        server->RegistrationState = Lwm2mRegistrationState_RegisterFailed;
        return;
    }


    GetQueryString(context, server->ShortServerID, uriQuery, sizeof(uriQuery));
    Lwm2mCore_GetObjectList(context, NULL, payload, sizeof(payload), true);

    Lwm2m_Debug("Registration: POST %s %s %s %s\n", serverUri, uriPath, uriQuery, payload);

    Lwm2m_Info("Register with %s\n", serverUri);
    sprintf(uri, "%s%s%s", serverUri, uriPath, uriQuery);
    Lwm2m_Debug("Register: POST %s\n", uri);

    coap_Reset(uri);

    coap_PostRequest(server, uri, AwaContentType_ApplicationLinkFormat, payload, strlen(payload), HandleRegisterResponse);
    server->RegistrationState = Lwm2mRegistrationState_Registering;
}

static void HandleRegisterResponse(void * ctxt, AddressType * address, const char * responsePath, int responseCode, AwaContentType contentType, char * payload, size_t payloadLen)
{
    Lwm2mServerType * server = ctxt;

    if (responseCode == 201)
    {
        Lwm2m_Debug("Registration Response %s %d\n", responsePath, responseCode);
        Lwm2m_Info("Registered with %s\n", Lwm2mCore_DebugPrintAddress(address));
        strncpy(server->Location, responsePath, LWM2M_SERVER_TYPE_LOCATION_SIZE);
        server->Location[LWM2M_SERVER_TYPE_LOCATION_SIZE - 1] = '\0'; // Defensive
        server->RegistrationState = Lwm2mRegistrationState_Registered;
    }
    else
    {
        Lwm2m_Error("Registration Failed! %d\n", responseCode);
        server->RegistrationState = Lwm2mRegistrationState_RegisterFailedRetry;
    }
}


// Periodically or based on certain events within the LWM2M Client or initiated by the LWM2M Server,
// the LWM2M Client updates its registration information with a LWM2M Server by sending an “Update”
// operation to the LWM2M Server. This “Update” operation MUST contain only the parameters listed in
// Table 5 which have changed compared to the last registration parameters sent to the LWM2M Server.
static void SendRegistrationUpdate(Lwm2mContextType * context, Lwm2mServerType * server)
{
    // POST /{location}?lt={Lifetime}&sms={msisdn}&b={binding}
    // success 2.04 Changed
    // failure 4.00 Bad request, 4.04 Not found
    char uriQuery[1024];
    char payload[1024];
    char serverUri[255];
    char uri[1024];
    int32_t lifetime = 0;
    int pos = 0;

    Lwm2m_GetServerURI(context, server->ShortServerID, serverUri, sizeof(serverUri));

    lifetime = Lwm2mServerObject_GetLifeTime(context, server->ShortServerID);
    if (lifetime <= 0)
    {
        // Use a sensible default
        lifetime = 60;
    }
    pos += snprintf(uriQuery, sizeof(uriQuery), "?lt=%d", lifetime);
    pos += GetTransportBinding(context, server->ShortServerID, uriQuery + pos, sizeof(uriQuery) - pos);

    Lwm2m_Debug("Server %d will%s send full update.\n", server->ShortServerID, server->UpdateRegistration ? "" : " not");
    Lwm2mCore_GetObjectList(context, NULL, payload, sizeof(payload), server->UpdateRegistration);
    sprintf(uri, "%s%s%s", serverUri, server->Location, uriQuery);

    Lwm2m_Debug("Registration Update: PUT %s %s\n", uri, payload);
#ifdef LWM2M_V1_0
    coap_PutRequest(server, uri, strlen(payload) ? AwaContentType_ApplicationLinkFormat: AwaContentType_None, payload, strlen(payload), HandleRegisterUpdateResponse);
#else
    coap_PostRequest(server, uri, strlen(payload) ? AwaContentType_ApplicationLinkFormat: AwaContentType_None, payload, strlen(payload), HandleRegisterUpdateResponse);
#endif
}

static void HandleRegisterUpdateResponse(void * ctxt, AddressType * address, const char * responsePath, int responseCode, AwaContentType contentType, char * payload, size_t payloadLen)
{
    Lwm2mServerType * server = ctxt;
    Lwm2m_Debug("Registration Update Response %s %d\n", responsePath, responseCode);

    if (responseCode != 204)
    {
        Lwm2m_Error("Registration Update Failed!\n");
        server->RegistrationState = Lwm2mRegistrationState_RegisterFailedRetry;
    }
    else
    {
        server->RegistrationState = Lwm2mRegistrationState_Registered;
    }
}

// When a LWM2M Client determines that it no longer requires to be available to a LWM2M Server
// (e.g., LWM2M Device factory reset), the LWM2M Client SHOULD send a “De-register” operation to
// the LWM2M Server. Upon receiving this message, the LWM2M Server removes the registration information
// from the LWM2M Server.
static void Deregister(Lwm2mContextType * context, Lwm2mServerType * server)
{
    // DELETE /{location}
    // success 2.02 Deleted
    char serverUri[255];
    char uri[1024];

    if (strlen(server->Location) == 0)
    {
        server->RegistrationState = Lwm2mRegistrationState_NotRegistered;
        return;
    }

    Lwm2m_GetServerURI(context, server->ShortServerID, serverUri, sizeof(serverUri));
    Lwm2m_Info("Deregister from %s\n", serverUri);
    sprintf(uri, "%s%s", serverUri, server->Location);
    Lwm2m_Debug("Deregister: DELETE %s\n", uri);
    coap_DeleteRequest(server, uri, HandleDeregisterResponse);
    server->RegistrationState = Lwm2mRegistrationState_Deregistering;
}

static void HandleDeregisterResponse(void * ctxt, AddressType* address, const char * responsePath, int responseCode, AwaContentType contentType, char * payload, size_t payloadLen)
{
    Lwm2mServerType * server = ctxt;
    if (responseCode == 202)
    {
        server->RegistrationState = Lwm2mRegistrationState_NotRegistered;
        Lwm2m_Info("Deregistered\n");
    }
}

// Set all servers to send a Registration Update.
void Lwm2m_SetUpdateRegistration(Lwm2mContextType * context)
{
    struct ListHead * i;
    ListForEach(i, Lwm2mCore_GetServerList(context))
    {
        Lwm2mServerType * server = ListEntry(i, Lwm2mServerType, list);

        if (server != NULL)
        {
            server->UpdateRegistration = true;
            Lwm2m_Debug("Registration update for ServerID %d scheduled\n", server->ShortServerID);
        }
    }
}

// Update the registration state of all servers in the serverList.
// If a server is in the Register state, then send a registration request to that server.
// If a server is Registered and it's last update request is greater than the server
//    lifetime / 2 then an update request will be sent.
// If the server state has been set to "Deregister" then a deregister request will be
//    sent to the server.
// Return time till next update is required.
int32_t Lwm2m_UpdateRegistrationState(Lwm2mContextType * context)
{
    uint32_t now = Lwm2mCore_GetTickCountMs();
    uint32_t lifeTime;
    int failedCount = 0;
    int serverCount = 0;
    struct ListHead * i;
    ListForEach(i, Lwm2mCore_GetServerList(context))
    {
        Lwm2mServerType * server = ListEntry(i, Lwm2mServerType, list);
        serverCount++;

        switch (server->RegistrationState)
        {
            case Lwm2mRegistrationState_NotRegistered:
                // Not registered, with no intent on registering.
                break;

            case Lwm2mRegistrationState_Register:
                // This server is in the Not registered, but a registration is expected
                SendRegisterRequest(context, server);
                server->LastUpdate = now;
                server->Attempts++;
                break;

            case Lwm2mRegistrationState_Registering:
                // Wait here until the registration completes or times out
                if (now - server->LastUpdate >= REGISTRATION_TIMEOUT)
                {
                    Lwm2m_Error("Registration attempt timed out.\n");
                    server->RegistrationState = Lwm2mRegistrationState_RegisterFailedRetry;
                }
                break;

            case Lwm2mRegistrationState_Registered:

                if (Lwm2mCore_GetBootstrapState(context) == Lwm2mBootStrapState_CheckExisting)
                {
                    Lwm2mCore_SetBootstrapState(context, Lwm2mBootStrapState_BootStrapped);
                }

                // Registered, update the LifeTime and send an update request if required
                lifeTime = Lwm2mServerObject_GetLifeTime(context, server->ShortServerID);

                // Use 1/2 lifetime to prevent timing out.
                if ((server->UpdateRegistration) || (now - server->LastUpdate >= ((lifeTime * 1000) / 2)))
                {
                    SendRegistrationUpdate(context, server);
                    server->LastUpdate = now;
                    server->RegistrationState = Lwm2mRegistrationState_UpdatingRegistration;
                    server->UpdateRegistration = false;
                }
                break;

            case Lwm2mRegistrationState_UpdatingRegistration:

                if (now - server->LastUpdate >= REGISTRATION_TIMEOUT)
                {
                    Lwm2m_Error("Update attempt timed out.\n");
                    server->RegistrationState = Lwm2mRegistrationState_RegisterFailedRetry;
                }
                break;

            case Lwm2mRegistrationState_Deregister:
                Deregister(context, server);
                server->LastUpdate = now;
                break;

            case Lwm2mRegistrationState_Deregistering:
                // No action
                break;

            case Lwm2mRegistrationState_RegisterFailedRetry:

                if (server->Attempts >= REGISTRATION_RETRY_ATTEMPTS)
                {
                    server->RegistrationState = Lwm2mRegistrationState_RegisterFailed;
                    break;
                }
                lifeTime = Lwm2mServerObject_GetLifeTime(context, server->ShortServerID);

                // Registration failed, so lets wait for the lifetime before re-registering
                if (now - server->LastUpdate >= (lifeTime * 1000))
                {
                    server->RegistrationState = Lwm2mRegistrationState_Register;
                }
                break;

            case Lwm2mRegistrationState_RegisterFailed:
                // Exceeded retry attempt count and unable to register
                failedCount++;
                break;
        }
    }

    // If all server registrations have failed, then pass control back to the bootstrap
    // process. if we are just checking the existing server entries then continue with
    // the bootstrap process from the client hold off step (4) otherwise, start the
    // bootstrap all over again.
    if (failedCount == serverCount)
    {
        if (Lwm2mCore_GetBootstrapState(context) == Lwm2mBootStrapState_CheckExisting)
        {
            if (serverCount == 0)
            {
                Lwm2m_Info("No servers are defined\n");
            }
            else
            {
                Lwm2m_Warning("Bootstrap from existing server entry failed\n");
            }
            Lwm2mCore_SetBootstrapState(context, Lwm2mBootStrapState_ClientHoldOff);
        }
        else
        {
            Lwm2mCore_SetBootstrapState(context, Lwm2mBootStrapState_NotBootStrapped);
        }
    }
    return 0;
}

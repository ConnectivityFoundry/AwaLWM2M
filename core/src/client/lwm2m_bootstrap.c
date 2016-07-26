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
#include <stdint.h>
#include <stdbool.h>

#include "coap_abstraction.h"
#include "lwm2m_endpoints.h"
#include "lwm2m_object_store.h"
#include "lwm2m_bootstrap.h"
#include "lwm2m_objects.h"
#include "lwm2m_security_object.h"
#include "lwm2m_server_object.h"
#include "lwm2m_bootstrap_config.h"
#include "lwm2m_result.h"

#define BOOTSTRAP_TIMEOUT          (10000)
#define BOOTSTRAP_FINISHED_TIMEOUT (15000)


static void HandleBootstrapResponse(void * ctxt, AddressType* address, const char * responsePath, int responseCode, AwaContentType contentType, char * payload, size_t payloadLen);


/*
 * The bootstrap interface is used to optionally configure a LWM2M Client so that it can successfully register with a LWM2M Server.
 * The client bootstrap operation is performed by sending a CoAP POST request to the LWM2M Bootstrap Server at the /bs path including
 * the Endpoint Client Name as a query string parameter.
 *
 * In client-initiated bootstrap, when the Bootstrap Server receives Request Bootstrap operation, the Bootstrap Server performs Write
 * and/or Delete operation. In server initiated bootstrap, the Bootstrap Server performs Write operation.
 * The Write or Delete operation targets to an Object Instance or a Resource. The Write and Delete operation can be sent multiple times.
 * Only in Bootstrap Interface, Delete operation MAY target to “/” URI to delete all the existing Object Instances except
 * LWM2M Bootstrap Server Account in the LWM2M Client for initializing before LWM2M Bootstrap Server sends Write operation(s)
 * to the LWM2M Client. Different from „Write“ operation in Device Management and Service Enablement interface, the LWM2M Client MUST write
 * the value included in the payload regardless of an existence of the targeting Object Instance or Resource.
 */
static void SendBootStrapRequest(Lwm2mContextType * context, int shortServerID)
{
    char * uriPath = "/bs";
    char uriQuery[128];
    char buffer[128];
    char serverPath[128];
    char uri[1024];

    Lwm2mCore_GetEndPointClientName(context, buffer, sizeof(buffer));
    sprintf(uriQuery, "?ep=%s", buffer);

    Lwm2m_GetServerURI(context, shortServerID, serverPath, sizeof(serverPath));

    sprintf(uri, "%s%s%s", serverPath, uriPath, uriQuery);
    Lwm2m_Info("Bootstrap with %s\n", uri);

    coap_Reset(uri);

    coap_PostRequest(context, uri, AwaContentType_None, NULL, 0, HandleBootstrapResponse);
}

static void HandleBootstrapResponse(void * ctxt, AddressType* address, const char * responsePath, int responseCode, AwaContentType contentType, char * payload, size_t payloadLen)
{
    Lwm2mContextType * context = ctxt;

    if (responseCode == AwaResult_SuccessChanged)
    {
        Lwm2m_Info("Waiting for bootstrap to finish\n");
        Lwm2mCore_SetBootstrapState(context, Lwm2mBootStrapState_BootStrapFinishPending);
        Lwm2mCore_SetLastBootStrapUpdate(context, Lwm2mCore_GetTickCountMs());
    }
    else
    {
        Lwm2mCore_SetBootstrapState(context, Lwm2mBootStrapState_BootStrapFailed);
    }
}

static bool BootStrapFromSmartCard(Lwm2mContextType * context)
{
    // not implemented

    Lwm2m_Debug("Lwm2m_BootstrapFromSmartCard\n");
    return false;
}

static bool BootStrapFromFactory(Lwm2mContextType * context)
{
    Lwm2m_Debug("Lwm2m_BootstrapFromFactory: %s\n", Lwm2mCore_GetUseFactoryBootstrap(context) ? "True" : "False");

    return Lwm2mCore_GetUseFactoryBootstrap(context);
}

// Handler called when the server posts a "finished" message to /bs
static int BootStrapPost(void * ctxt, AddressType * addr, const char * path, const char * query, AwaContentType contentType,
                         const char * requestContent, size_t requestContentLen, char * responseContent, size_t * responseContentLen, int * responseCode)
{
    Lwm2mContextType * context = (Lwm2mContextType *)ctxt;
    Lwm2mBootStrapState state = Lwm2mCore_GetBootstrapState(context);

    *responseContentLen = 0;  // no content

    Lwm2m_Debug("POST to /bs\n");

    if (state == Lwm2mBootStrapState_BootStrapFinishPending)
    {
        Lwm2m_Info("Bootstrap finished\n");
        Lwm2mCore_SetBootstrapState(context, Lwm2mBootStrapState_BootStrapped);
        Lwm2mCore_UpdateAllServers(context, Lwm2mRegistrationState_Register);
        *responseCode = AwaResult_SuccessChanged;
    }
    else if ((state == Lwm2mBootStrapState_BootStrapPending) ||
             (state == Lwm2mBootStrapState_ClientHoldOff))
    {
        Lwm2m_Info("Server initiated bootstrap\n");
        Lwm2mCore_SetBootstrapState(context, Lwm2mBootStrapState_BootStrapped);
        Lwm2mCore_UpdateAllServers(context, Lwm2mRegistrationState_Register);
        *responseCode = AwaResult_SuccessChanged;
    }
    else
    {
        *responseCode = AwaResult_BadRequest;
    }
    return 0;
}

// Handler for /bs
static int BootstrapEndpointHandler(int type, void * ctxt, AddressType * addr,
                                              const char * path, const char * query, const char * token, int tokenLength,
                                              AwaContentType contentType, const char * requestContent, size_t requestContentLen,
                                              AwaContentType * responseContentType, char * responseContent, size_t * responseContentLen, int * responseCode)
{
    switch (type)
    {
        case COAP_PUT_REQUEST:  // no break
        case COAP_POST_REQUEST:
            return BootStrapPost(ctxt, addr, path, query, contentType, requestContent, requestContentLen, responseContent, responseContentLen, responseCode);
        default:
            *responseContentType = AwaContentType_None;
            *responseContentLen = 0;
            *responseCode = AwaResult_MethodNotAllowed;
            break;
    }
    return -1;
}

// Initialise the boot strap mechanism, create the /bs endpoint
void Lwm2m_BootStrapInit(Lwm2mContextType * context)
{
    Lwm2mCore_AddResourceEndPoint(context, "/bs", BootstrapEndpointHandler);
}

/* The LWM2M Client MUST follow the procedure specified as below when attempting to bootstrap a LWM2M Device:
 * 1. If the LWM2M Device has Smartcard, the LWM2M Client tries to obtain Bootstrap Information
 *    from the Smartcard using the Bootstrap from Smartcard mode.
 * 2. If the LWM2M Client is not configured using the Bootstrap from Smartcard mode, the LWM2M
 *    Client tries to obtain the Bootstrap Information by using Factory Bootstrap mode.
 * 3. If the LWM2M Client has any LWM2M Server Object Instances from the previous steps, the LWM2M
 *    Client tries to register to the LWM2M Server(s) configured in the LWM2M Server Object Instance(s).
 * 4. If LWM2M Client fails to register to all the LWM2M Servers or the Client doesn’t have any
 *    LWM2M Server Object Instances, and the LWM2M Client hasn’t received a Server Initiated Bootstrap
 *    within the ClientHoldOffTime, the LWM2M Client performs the Client Initiated Bootstrap.
 */
void Lwm2m_UpdateBootStrapState(Lwm2mContextType * context)
{
    uint32_t now = Lwm2mCore_GetTickCountMs();
    uint32_t clientHoldOff;
    enum { SERVER_BOOTSTRAP = 0 };

    switch (Lwm2mCore_GetBootstrapState(context))
    {
        case Lwm2mBootStrapState_NotBootStrapped:

            // First attempt smart card bootstrap.
            if (BootStrapFromSmartCard(context))
            {
                Lwm2mCore_SetBootstrapState(context, Lwm2mBootStrapState_BootStrapped);
            }
            // If that fails try and use the factory information.
            else if (BootStrapFromFactory(context))
            {
                Lwm2mCore_SetBootstrapState(context, Lwm2mBootStrapState_BootStrapped);
            }
            // If that fails wait for the client hold off time, for a server initiated bootstrap.
            else
            {
                Lwm2m_Info("Try existing server entries\n");
                Lwm2mCore_SetBootstrapState(context, Lwm2mBootStrapState_CheckExisting);
            }
            Lwm2mCore_UpdateAllServers(context, Lwm2mRegistrationState_Register);
            Lwm2mCore_SetLastBootStrapUpdate(context, now);
            break;

        case Lwm2mBootStrapState_CheckExisting:
            // Pass control to Registration process, if this fails then we will
            // drop into the ClientHoldOff State
            break;

        case Lwm2mBootStrapState_ClientHoldOff:
            // Wait for server initiated bootstrap.
            // Only one bootstrap server is supported.

            // If the hold off timer has expired, then request a boot strap.
            Lwm2m_GetClientHoldOff(context, SERVER_BOOTSTRAP, &clientHoldOff);

            if (now - Lwm2mCore_GetLastBootStrapUpdate(context) >= (clientHoldOff * 1000))
            {
                Lwm2m_Info("Hold Off expired - attempt client-initiated bootstrap\n");
                Lwm2mCore_SetBootstrapState(context, Lwm2mBootStrapState_BootStrapPending);
                SendBootStrapRequest(context, SERVER_BOOTSTRAP);
                Lwm2mCore_SetLastBootStrapUpdate(context, now);
            }
            break;

        case Lwm2mBootStrapState_BootStrapPending:

            if (now - Lwm2mCore_GetLastBootStrapUpdate(context) >= BOOTSTRAP_TIMEOUT)
            {
                Lwm2m_Error("No response to client initiated bootstrap\n");
                Lwm2mCore_SetBootstrapState(context, Lwm2mBootStrapState_BootStrapFailed);
                Lwm2mCore_SetLastBootStrapUpdate(context, now);
            }

            break;

        case Lwm2mBootStrapState_BootStrapFinishPending:
            // The 2015/07/07 LWM2M draft requires that the server sends a bootstrap finished to the clients /bs endpoint,
            // however for now lets just wait up to 15 seconds and then move to BootStrapped state.
            if (now - Lwm2mCore_GetLastBootStrapUpdate(context) >= BOOTSTRAP_FINISHED_TIMEOUT)
            {
                Lwm2m_Warning("No bootstrap finished after 15 seconds, retrying...\n");
                Lwm2mCore_SetBootstrapState(context, Lwm2mBootStrapState_BootStrapFailed);
            }
            break;

        case Lwm2mBootStrapState_BootStrapped:
            break;

        case Lwm2mBootStrapState_BootStrapFailed:

            // If the hold off timer has expired, then request a boot strap.
            Lwm2m_GetClientHoldOff(context, SERVER_BOOTSTRAP, &clientHoldOff);

            if (now - Lwm2mCore_GetLastBootStrapUpdate(context) >= (clientHoldOff * 1000))
            {
                Lwm2m_Warning("HoldOff Expired - Re-attempt bootstrap\n");
                Lwm2mCore_SetBootstrapState(context, Lwm2mBootStrapState_NotBootStrapped);
            }
            break;

        default:
            Lwm2m_Error("Unhandled bootstrap state %d\n", Lwm2mCore_GetBootstrapState(context));
    }
}

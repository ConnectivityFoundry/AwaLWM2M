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

/**
 * @file server.h
 * @brief This interface enables an application to interact with the LWM2M interface provided by LWM2M Server Core.
 *
 * The Core may be running as a separate process (daemon), or it may be linked with the application directly.
 *
 * The Core provides LWM2M operations targeted at registered LWM2M clients. LWM2M operations provide access to
 * client resources within a data model based on Objects, Object Instances and Resources.
 * Please consult the LWM2M specification for details of this model.
 *
 * A Management Application may interact with clients via the Awa Server API, accessing the client's resources. Therefore
 * it is essential that both the Client Application and Management Application are aware of the same data
 * model.
 *
 * It is recommended that registered IDs are used for objects that conform to registered LWM2M objects such
 * as IPSO objects.
 */

#ifndef AWA_SERVER_H
#define AWA_SERVER_H

// @cond
#include <stdbool.h>
// @endcond

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
 * Example Usage
 *************************************************************************************************/

/**
* @example server-list-clients-example.c
*        Create, process and free a basic List Clients request.
* @example server-read-example.c
*        Create, process and free a basic Read request on a single resource.
* @example server-write-example.c
*        Create, process and free a basic Write request on a single resource.
* @example server-write-create-example.c
*        Create, process and free a basic Write request to create an object instance.
* @example server-write-create-unspecified-instance-example.c
*        Create, process and free a basic Write request to create an object instance
*        without specifying the object instance ID.
* @example server-delete-object-instance-example.c
*        Create, process and free a basic Delete request on an object instance.
* @example server-observe-example.c
*        Create, process and free a basic Observe request on a single resource and
*        receive notifications containing the initial and changed value.
* @example server-defineset-example.c
*        Create, process and free a define request to define a custom object with resources.
* @example server-execute-example.c
*        Create, process and free a basic request to execute a single resource.
* @example server-execute-arguments-example.c
*        Create, process and free a basic request to execute a single resource.
* @example server-event-example.c
*        Respond to events such as Client Register, Client Deregister and Client Update.
*/


/**************************************************************************************************
 * Public Types and Enumerations
 *************************************************************************************************/

/**
 * Supported write modes
 */
typedef enum
{
    AwaWriteMode_Replace = 0,  /**< indicates a write using CoAP "PUT" */
    AwaWriteMode_Update,       /**< indicates a write using CoAP "POST" */
    AwaWriteMode_LAST,         /**< Reserved value */
} AwaWriteMode;

/// @cond ignore
typedef struct _AwaServerSession AwaServerSession;
typedef struct _AwaServerListClientsOperation AwaServerListClientsOperation;
typedef struct _AwaServerListClientsResponse AwaServerListClientsResponse;
typedef struct _AwaClientIterator AwaClientIterator;
typedef struct _AwaRegisteredEntityIterator AwaRegisteredEntityIterator;
typedef struct _AwaServerDefineOperation AwaServerDefineOperation;
typedef struct _AwaServerDefineResponse AwaServerDefineResponse;
typedef struct _AwaServerReadOperation AwaServerReadOperation;
typedef struct _AwaServerReadResponse AwaServerReadResponse;
typedef struct _AwaServerWriteOperation AwaServerWriteOperation;
typedef struct _AwaWriteResponse AwaServerWriteResponse;
typedef struct _AwaServerDeleteResponse AwaServerDeleteResponse;
typedef struct _AwaServerDeleteOperation AwaServerDeleteOperation;
typedef struct _AwaServerExecuteOperation AwaServerExecuteOperation;
typedef struct _AwaServerExecuteResponse AwaServerExecuteResponse;
typedef struct _AwaServerWriteAttributesOperation AwaServerWriteAttributesOperation;
typedef struct _AwaServerWriteAttributesResponse AwaServerWriteAttributesResponse;
typedef struct _AwaServerDiscoverOperation AwaServerDiscoverOperation;
typedef struct _AwaServerDiscoverResponse AwaServerDiscoverResponse;
typedef struct _AwaAttributeIterator AwaAttributeIterator;
typedef struct _AwaServerObserveOperation AwaServerObserveOperation;
typedef struct _AwaServerObservation AwaServerObservation;
typedef struct _AwaServerObserveResponse AwaServerObserveResponse;
typedef struct _AwaServerClientRegisterEvent AwaServerClientRegisterEvent;
typedef struct _AwaServerClientUpdateEvent AwaServerClientUpdateEvent;
typedef struct _AwaServerClientDeregisterEvent AwaServerClientDeregisterEvent;
/// @endcond


/**************************************************************************************************
 * Callbacks
 *************************************************************************************************/

/**
 * @brief A user-specified callback handler for an Observation which will be fired on
 *        AwaServerSession_DispatchCallbacks if the subscribed entity has changed since
 *        the observation's session callbacks were last dispatched.
 *        Warning: Do NOT process any operations while inside an observe callback!
 * @param[in] changeSet A pointer to a valid ChangeSet.
 * @param[in] context A pointer to user-specified data passed to ::AwaServerObservation_New
 */
typedef void (*AwaServerObservationCallback)(const AwaChangeSet * changeSet, void * context);

/**
 * @brief A user-specified callback handler for Server Notifications which will be fired on client registration.
 *        Warning: Do NOT process any operations while inside this callback!
 * @param[in] context A pointer to user-specified data passed to ::AwaServerSession_SetClientRegisterEventCallback.
 */
typedef void (*AwaServerClientRegisterEventCallback)(const AwaServerClientRegisterEvent * event, void * context);

/**
 * @brief A user-specified callback handler for Server Notifications which will be fired on client update.
 *        Warning: Do NOT process any operations while inside this callback!
 * @param[in] context A pointer to user-specified data passed to ::AwaServerSession_SetClientUpdateEventCallback.
 */
typedef void (*AwaServerClientUpdateEventCallback)(const AwaServerClientUpdateEvent * event, void * context);

/**
 * @brief A user-specified callback handler for Server Notifications which will be fired on client deregister.
 *        Warning: Do NOT process any operations while inside this callback!
 * @param[in] context A pointer to user-specified data passed to ::AwaServerSession_SetClientDeregisterEventCallback.
 */
typedef void (*AwaServerClientDeregisterEventCallback)(const AwaServerClientDeregisterEvent * event, void * context);


/**************************************************************************************************
 * Server Session Management
 *************************************************************************************************/

/**
 * @brief Initialise a new session of the Awa subsystem.
 *        A Session is required for interaction with the Awa Core.
 *        Operations to interact with Core resources are created in the context of a
 *        session, however the session must be connected before an operation can be processed.
 *        The session is owned by the caller and should eventually be freed with AwaClientSession_Free.
 * @return Pointer to new client session.
 * @return NULL on error.
 */
AwaServerSession * AwaServerSession_New(void);

/**
 * @brief Configure the IPC mechanism used by the API to communicate with the Core.
 *        This function configures the mechanism to use UDP with the Core located at
 *        the specified address and port.
 * @param[in] session Pointer to the session that is to be configured.
 * @param[in] address Specifies the IP address or hostname to use to connect with the Core.
 * @param[in] port Specifies the Port number to use to connect with the Core.
 * @return AwaError_Success on success.
 * @return AwaError_IPCError if an IPC error occurs.
 * @return AwaError_SessionInvalid if the specified session is invalid.
 */
AwaError AwaServerSession_SetIPCAsUDP(AwaServerSession * session, const char * address, unsigned short port);

// Not yet implemented:
//AwaError AwaServerSession_SetIPCAsLocal(AwaServerSession * session);
//AwaError AwaServerSession_SetIPCAsMQTT(AwaServerSession * session /* ... */);
//AwaError AwaServerSession_SetIPCAsUbus(AwaServerSession * session /* ... */);
//AwaError AwaServerSession_SetIPCAsNanoMsg(AwaServerSession * session /* ... */);
//AwaError AwaServerSession_SetIPCAsREST(AwaServerSession * session /* ... */);

/**
 * @brief Set IPC connect timeout.
 *        Used internally by ::AwaServerSession_Connect and ::AwaServerSession_Disconnect.
 * @param[in] session Pointer to a valid server session.
 * @param[in] timeout The time within which an IPC session Connect must complete to avoid timeout.
 *                    It must be greater than 0.
 * @return AwaError_Success on success.
 * @return Various errors on failure.
 */
AwaError AwaServerSession_SetDefaultTimeout(AwaServerSession * session, AwaTimeout timeout);

/**
 * @brief Connect a session (configured with an IPC mechanism) to the Core.
 *        A session must be connected before operations can be processed.
 * @param[in] session Pointer to an IPC-configured session.
 * @return AwaError_Success on success.
 * @return Various errors on failure.
 */
AwaError AwaServerSession_Connect(AwaServerSession * session);

/**
 * @brief Object and Resource Definitions are assembled by the application and sent to
 *        the server daemon for storage (a process called "Define"). After successful
 *        definition, the application can shut down and the daemon will retain the definitions.
 *        Later, an application may wish to interact with these objects and resources. In
 *        order to do this in a type-safe manner, the Session must be aware of the definitions.
 *        Definitions will be held by the Session if DefineObject/DefineResource functions were
 *        used to successfully define objects and resources. If not, then the definitions must
 *        be retrieved with AwaServerSession_Refresh.
 * @param[in] session A pointer to a valid session instance.
 * @return AwaError_Success on success.
 * @return AwaError_SessionInvalid if the specified session is invalid.
 */
AwaError AwaServerSession_Refresh(AwaServerSession * session);

/**
 * @brief Determines whether the specified session holds a valid definition for the given object.
 *        A session will hold a valid definition if the Core has a valid definition at the time
 *        the session was connected, or if a successful Define operation has been performed for this
 *        object.
 * @param[in] session Pointer to a connected session.
 * @param[in] objectID Identifies the object for which the query is targeted.
 * @return True if object is defined.
 * @return False if object is not defined, or if the session is invalid.
 */
bool AwaServerSession_IsObjectDefined(const AwaServerSession * session, AwaObjectID objectID);

/**
 * @brief Retrieves an object definition instance corresponding to the object identified by the given object ID.
 *        The object definition is owned by the session and should not be freed by the caller.
 * @param[in] session Pointer to a connected session.
 * @param[in] objectID Identifies the object for which the query is targeted.
 * @return A const pointer to an object definition instance, if it exists.
 * @return NULL if a definition for the object specified does not exist.
 */
const AwaObjectDefinition * AwaServerSession_GetObjectDefinition(const AwaServerSession * session, AwaObjectID objectID);

/**
 * @brief Retrieves a new object definition iterator that can be used to iterate through the set
 *        of defined objects within the context of the given session.
 *        The iterator is owned by the caller and should eventually be freed with AwaObjectDefinitionIterator_Free.
 * @param[in] session Pointer to a connected session.
 * @return A pointer to an object definition iterator, if the session is valid.
 * @return NULL if the session is not valid.
 */
AwaObjectDefinitionIterator * AwaServerSession_NewObjectDefinitionIterator(const AwaServerSession * session);

/**
 * @brief Process any incoming requests from a LWM2M Client. Callbacks are scheduled on the session but are not invoked.
 * @param[in] session Pointer to a connected session.
 * @param[in] timeout The function will wait at least as long as this value for a response.
 * @return AwaError_Success on success.
 * @return AwaError_Timeout if no response is received after the timeout duration expires.
 * @return Various errors on failure.
 */
AwaError AwaServerSession_Process(AwaServerSession * session, AwaTimeout timeout);

/**
 * @brief Invoke any callbacks scheduled since this function was last called.
 * @param[in] session Pointer to a connected session.
 * @return AwaError_Success on success.
 * @return Various errors on failure.
 */
AwaError AwaServerSession_DispatchCallbacks(AwaServerSession * session);

/**
 * @brief When a session is no longer required, or if the application intends to sleep for some time, the session can be
 *        disconnected from the Core. This maintains object and resource definition information, but prevents the
 *        processing of operations, or the reception of subscription notifications.
 *        The session can be reconnected to the Core with AwaServerSession_Connect.
 *        If a connected session is freed, it is automatically disconnected.
 * @param[in] session Pointer to a connected session.
 * @return AwaError_Success on success.
 * @return AwaError_IPCError if the session is not configured.
 * @return AwaError_SessionNotConnected if the session is not connected.
 * @return AwaError_SessionInvalid if the session is invalid.
 */
AwaError AwaServerSession_Disconnect(AwaServerSession * session);

/**
 * @brief Shut down an existing session, freeing any allocated memory. This function
 *        should eventually be called on every session to avoid a memory leak.
 *        If the session is connected, it will automatically be disconnected first.
 * @param[in,out] session A pointer to a session pointer that will be set to NULL.
 * @return AwaError_Success on success.
 * @return AwaError_SessionInvalid if the session is not valid.
 */
AwaError AwaServerSession_Free(AwaServerSession ** session);

/**
 * @brief Paths are used to identify objects, object instances and resources within the data model.
 *        This function converts a path to equivalent object, object instance and resource IDs.
 * @param[in] session Pointer to a valid session.
 * @param[in] path Resource path to convert.
 * @param[out] objectID Pointer to Object ID for result, or NULL to ignore.
 * @param[out] objectInstanceID Pointer to Object Instance ID for result, or NULL to ignore.
 * @param[out] resourceID Pointer to Resource ID for result, or NULL to ignore.
 * @return AwaError_Success on success
 * @return AwaError_PathInvalid if path is not in the correct form.
 * @return AwaError_PathNotFound if path is correctly formed but does not correspond with a defined entity.
 */
AwaError AwaServerSession_PathToIDs(const AwaServerSession * session, const char * path, AwaObjectID * objectID, AwaObjectInstanceID * objectInstanceID, AwaResourceID * resourceID);


/**************************************************************************************************
 * List Clients Operation
 *************************************************************************************************/

/**
 * @brief Allocate and return a pointer to a new List Clients operation,
 *        that can be used to retrieve a list of clients and their registered entities currently
 *        connected to an LWM2M Server Daemon. When processed, the response is made available via
 *        AwaServerListClientsOperation_GetResponse which may be iterated through to retrieve
 *        the endpoint name of each client and their registered objects and object instances.
 *        The List Clients operation is owned by the caller and should eventually be freed with
 *        AwaServerListClientsOperation_Free.
 * @param[in] session A pointer to a valid session.
 * @return Pointer to a newly allocated List Clients operation instance.
 * @return NULL on failure.
 */
AwaServerListClientsOperation * AwaServerListClientsOperation_New(const AwaServerSession * session);

/**
 * @brief Process the List Clients operation by sending it to the Core.
 *        If successful, the response can be obtained with AwaServerListClientsOperation_GetResponse
 *        and iterated through to retrieve the registered entities for each connected client.
 * @param[in] operation The List Clients operation to process.
 * @param[in] timeout The function will wait at least as long as this value for a response.
 * @return AwaError_Success on success.
 * @return AwaError_Timeout if no response is received after the timeout duration expires.
 * @return Various errors on failure.
 */
AwaError AwaServerListClientsOperation_Perform(AwaServerListClientsOperation * operation, AwaTimeout timeout);

/**
 * @brief Obtain a List Clients Response instance from a processed List Clients operation. This may be
 *        iterated through using a AwaRegisteredEntityIterator in order to retrieve the
 *        registered objects and object instances for the specified client.
 *        The List Clients Response is owned by the List Clients operation and should not be freed by the caller.
 * @param[in] operation The processed List Clients operation to obtain the List Clients response from.
 * @param[in] clientID The endpoint name of the connected client to search.
 * @return Pointer to AwaClientList ClientsResponse if operation is valid and has a valid response.
 * @return NULL if the operation is invalid, or has no valid response (may not have been processed).
 */
const AwaServerListClientsResponse * AwaServerListClientsOperation_GetResponse(const AwaServerListClientsOperation * operation, const char * clientID);

/**
 * @brief Create a new Registered Entity iterator for a List Clients Response, used to iterate through the list of
 *        objects and object instances that exist within the client the response relates to.
 *        The resulting iterator is owned by the caller and should eventually be freed with AwaRegisteredEntityIterator_Free.
 *        This function can only be successful after a List Clients operation has been successfully processed.
 * @param[in] response A pointer to the List Clients response to search.
 * @return A pointer to a new RegisteredEntityIterator instance on success.
 * @return NULL on failure.
 */
AwaRegisteredEntityIterator * AwaServerListClientsResponse_NewRegisteredEntityIterator(const AwaServerListClientsResponse * response);

/**
 * @brief Clean up a List Clients operation, freeing all allocated resources.
 *        Once freed, the operation is no longer valid.
 *        Note: freeing a List Clients operation will invalidate all derived List Clients Response instances.
 * @param[in,out] operation A pointer to a List Clients operation pointer that will be set to NULL.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is not valid.
 */
AwaError AwaServerListClientsOperation_Free(AwaServerListClientsOperation ** operation);

/**
 * @brief Determine whether the Client Iterator has not reached the end of a set of clients.
 *        If the iterator has not reached the end, advance its position to the next client.
 * @param[in] iterator A pointer to a valid Client Iterator.
 * @return True if the Client Iterator is valid and the iterator has not yet reached the end of its elements.
 * @return False if the Client Iterator is invalid, or has reached the end of its elements.
 */
bool AwaClientIterator_Next(AwaClientIterator * iterator);

/**
 * @brief Retrieve the ID from the current client in the Client Iterator.
 * @param[in] iterator A pointer to a valid Client Iterator.
 * @return The client ID if the Client Iterator is valid.
 * @return NULL if the Client Iterator is invalid.
 */
const char * AwaClientIterator_GetClientID(const AwaClientIterator * iterator);

/**
 * @brief Clean up a Client Iterator, freeing all allocated resources.
 *        Once freed, the iterator is no longer valid.
 * @param[in,out] iterator A pointer to a Client Iterator pointer that will be set to NULL.
 */
void AwaClientIterator_Free(AwaClientIterator ** iterator);

/**
 * @brief Create a new Client Iterator for a List Clients Response, used to iterate through the list of client
 *        endpoint names retrieved by the corresponding List Clients operation.
 *        The resulting iterator is owned by the caller and should eventually be freed with AwaClientIterator_Free.
 *        This function can only be successful after a List Clients operation has been successfully processed.
 * @param[in] operation A pointer to a performed List Clients operation to iterate through
 * @return A pointer to a new ClientIterator instance on success.
 * @return NULL on failure.
 */
AwaClientIterator * AwaServerListClientsOperation_NewClientIterator(const AwaServerListClientsOperation * operation);

/**
 * @brief Determine whether the Registered Entity Iterator has not reached the end of a list of registered entities.
 *        If the iterator has not reached the end, advance its position to the next registered entity.
 * @param[in] iterator A pointer to a valid Registered Entity Iterator.
 * @return True if the Registered Entity Iterator is valid and the iterator has not yet reached the end of its elements.
 * @return False if the Registered Entity Iterator is invalid, or has reached the end of its elements.
 */
bool AwaRegisteredEntityIterator_Next(AwaRegisteredEntityIterator * iterator);

/**
 * @brief Retrieve the path to the current registered entity in the Registered Entity Iterator.
 * @param[in] iterator A pointer to a valid Registered Entity Iterator.
 * @return The path if the Registered Entity Iterator is valid.
 * @return NULL if the Registered Entity Iterator is invalid.
 */
const char * AwaRegisteredEntityIterator_GetPath(const AwaRegisteredEntityIterator * iterator);

/**
 * @brief Clean up a Registered Entity Iterator, freeing all allocated resources.
 *        Once freed, the iterator is no longer valid.
 * @param[in,out] iterator A pointer to a Registered Entity Iterator pointer that will be set to NULL.
 */
void AwaRegisteredEntityIterator_Free(AwaRegisteredEntityIterator ** iterator);


/**************************************************************************************************
 * Object & Resource Definition Operation
 *************************************************************************************************/

/**
 * @brief Allocate and return a pointer to a new Define operation,
 *        that can be used to define Objects and Resources.
 *        The operation is owned by the caller and should eventually be freed with AwaServerDefineOperation_Free.
 *
 *        Object and Resource Definitions are assembled by the application and sent to
 *        the client daemon for storage (a process called "Define"). After successful
 *        definition, the application can shut down and the daemon will retain the definitions.
 *
 *        Later, an application may wish to interact with these objects and resources. In
 *        order to do this in a type-safe manner, the Session must be aware of the definition.
 *        Definitions will be held by the Session if DefineObject/DefineResource functions were
 *        used to successfully define objects and resources. If not, then the definition must
 *        be retrieved with SessionRefresh.
 *
 * @param[in] session A pointer to a valid session.
 * @return Pointer to a newly allocated Define operation instance.
 * @return NULL on failure.
 */
AwaServerDefineOperation * AwaServerDefineOperation_New(const AwaServerSession * session);

/**
 * @brief Add an Object Definition to an existing Define operation, so that
 *        the client will create a suitable definition for this new object type.
 *        Multiple object definitions can be added to a single Define operation provided they
 *        correspond to different objects.
 *        An existing object definition cannot be redefined via this API.
 *        Note: A copy of the Object Definition is made, therefore all resources in the object
 *        definition must be defined before calling this function, if all are to be included.
 * @param[in] operation Pointer to a valid Define operation.
 * @param[in] objectDefinition Pointer to a valid Object Definition.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid or NULL.
 * @return AwaError_DefinitionInvalid if the object definition is invalid or NULL.
 * @return AwaError_AlreadyDefined if a definition already exists for the new object definition.
 */
AwaError AwaServerDefineOperation_Add(AwaServerDefineOperation * operation, const AwaObjectDefinition * objectDefinition);

// Future: support per-client definitions:
//AwaError AwaServerDefineOperation_AddWithClient(AwaServerDefineOperation * operation, const char * clientID, const AwaObjectDefinition * objectDefinition);

/*
 * @brief Process the Define operation by sending it to the Core.
 *        If successful, the object definition is also added to the session.
 * @param[in] operation The Define operation to process.
 * @param[in] timeout The function will wait at least as long as this value for a response.
 * @return AwaError_Success on success.
 * @return AwaError_Timeout if no response is received after the timeout duration expires.
 * @return Various errors on failure.
 */
AwaError AwaServerDefineOperation_Perform(AwaServerDefineOperation * operation, AwaTimeout timeout);

/**
 * @brief Clean up a Define operation, freeing all allocated resources.
 *        Once freed, the operation is no longer valid.
 * @param[in,out] operation A pointer to a Define operation pointer that will be set to NULL.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is not valid.
 */
AwaError AwaServerDefineOperation_Free(AwaServerDefineOperation ** operation);

/**
 * @brief Obtain a Server Define Response instance from a processed Server Define operation. This may be
 *        iterated through to determine whether the define operation succeeded for the requested paths.
 *        The Server Define Response is owned by the Server Define operation and should not be freed by the caller.
 * @param[in] operation The processed Server Define operation to obtain the Server Define response from.
 * @return Pointer to AwaClientServer DefineResponse if operation is valid and has a valid response.
 * @return NULL if the operation is invalid, or has no valid response (may not have been processed).
 */
const AwaServerDefineResponse * AwaServerDefineOperation_GetResponse(const AwaServerDefineOperation * operation);

/**
 * @brief Create a new Path Iterator for a Define Response, used to iterate through the list of paths
 *        returned in the response of the corresponding Define operation.
 *        The resulting iterator is owned by the caller and should eventually be freed with AwaPathIterator_Free.
 *        This function can only be successful after a Define operation has been successfully processed.
 * @param[in] response A pointer to the Define Response to search.
 * @return A pointer to a new PathIterator instance on success.
 * @return NULL on failure.
 */
AwaPathIterator * AwaServerDefineResponse_NewPathIterator(const AwaServerDefineResponse * response);

/**
 * @brief Retrieve a path result from a Define Response relating to a definition on that path
 *        retrieved by the corresponding Define operation. Querying the path result will allow
 *        the caller to determine the status of a request to define objects and resources.
 * @param[in] response A pointer to the Define Response to search.
 * @param[in] path A path to an entity that was requested to be defined.
 * @return NULL if no definition exists in the Define response for the specified path,
 *         or if the response, path or result is NULL.
 * @return Pointer to PathResult instance. Do not free.
 */
const AwaPathResult * AwaServerDefineResponse_GetPathResult(const AwaServerDefineResponse * response, const char * path);

/**
 * @brief Test if the Define Response has the specified path to a resource or object present.
 *        If the Define Response contains paths beyond the specified path, the path will be considered present.
 * @param[in] response A pointer to a valid Define Response.
 * @param[in] path The path with which to query the Define Response.
 * @return True if the Define Response contains the specified resource path.
 * @return False if the Define Response does not contain the specified resource path, or if the path is invalid.
 */
bool AwaServerDefineResponse_ContainsPath(const AwaServerDefineResponse * response, const char * path);


/**************************************************************************************************
 * Object & Resource Retrieval Operation
 *************************************************************************************************/

/**
 * @brief Allocate and return a pointer to a new Read operation,
 *        that can be used to retrieve resource values from the Core.
 *        Read operations can be loaded with one or more paths of interest, corresponding to
 *        resources or trees of resources that the application wishes to retrieve. When
 *        processed, the response is made available via AwaServerReadOperation_GetResponse
 *        and will contain the resources returned by the Core in response to the query.
 *        The Read operation is owned by the caller and should eventually be freed with AwaServerReadOperation_Free.
 * @param[in] session A pointer to a valid session.
 * @return Pointer to a newly allocated Read operation instance.
 * @return NULL on failure.
 */
AwaServerReadOperation * AwaServerReadOperation_New(const AwaServerSession * session);

/**
 * @brief Adds a path of interest to a Read operation, as a request to retrieve all resources at
 *        or covered by this path from the Core.
 *        Adding an object path will retrieve all resources held by all object instances of this object.
 *        Adding an object instance path will retrieve all resources held by this object instance.
 *        Adding a resource path will retrieve that specific resource.
 *        Multiple-instance resources are retrieved in their entirety (the entire array is retrieved)
 *        Adding a path that does not correspond to any resources in the Core will result in the
 *        subsequent ReadResponse lacking resources for that path.
 * @param[in] operation The Read operation to add the path of interest to.
 * @param[in] clientID The name of the client to query
 * @param[in] path The path of the resource, object instance or object requested for retrieval.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_PathInvalid if the path specified is not a valid data model path.
 */
AwaError AwaServerReadOperation_AddPath(AwaServerReadOperation * operation, const char * clientID, const char * path);

/**
 * @brief Process the Read operation by sending it to the Core.
 *        If successful, the response can be obtained with AwaServerReadOperation_GetResponse
 *        and queried for retrieved resource values.
 * @param[in] operation The Read operation to process.
 * @param[in] timeout The function will wait at least as long as this value for a response.
 * @return AwaError_Success on success.
 * @return AwaError_Timeout if no response is received after the timeout duration expires.
 * @return Various errors on failure.
 */
AwaError AwaServerReadOperation_Perform(AwaServerReadOperation * operation, AwaTimeout timeout);

/**
 * @brief Clean up a Read operation, freeing all allocated resources.
 *        Once freed, the operation is no longer valid.
 *        Note: freeing a Read operation will invalidate all derived Read Response instances,
 *        and any Opaque or CString pointers retrieved from an associated Read Response.
 * @param[in,out] operation A pointer to a Read operation pointer that will be set to NULL.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is not valid.
 */
AwaError AwaServerReadOperation_Free(AwaServerReadOperation ** operation);

/**
 * @brief Create a new Client Iterator for a performed Read Operation, used to iterate through the list of client
 *        endpoint names retrieved by the corresponding Read operation.
 *        The resulting iterator is owned by the caller and should eventually be freed with AwaClientIterator_Free.
 *        This function can only be successful after a Read operation has been successfully processed.
 * @param[in] operation A pointer to a performed Read operation to iterate through
 * @return A pointer to a new ClientIterator instance on success.
 * @return NULL on failure.
 */
AwaClientIterator * AwaServerReadOperation_NewClientIterator(const AwaServerReadOperation * operation);

/**
 * @brief Obtain a Read Response instance from a processed Read operation. This may be
 *        queried directly by path to obtain the resource values returned by the Core.
 *        A Path iterator can be used to obtain a list of all resource paths provided by
 *        the Read Response.
 *        The Read Response is owned by the Read operation and should not be freed by the caller.
 * @param[in] operation The processed Read operation to obtain the Read response from.
 * @param[in] clientID The endpoint name of the client to retrieve read values from.
 * @return Pointer to AwaServerReadResponse if operation is valid and has a valid response.
 * @return NULL if the operation is invalid, or has no valid response (may not have been processed).
 */
const AwaServerReadResponse * AwaServerReadOperation_GetResponse(const AwaServerReadOperation * operation, const char * clientID);

/**
 * @brief Create a new Path Iterator for a Read Response, used to iterate through the list of resource paths
 *        retrieved by the corresponding Read operation.
 *        The resulting iterator is owned by the caller and should eventually be freed with AwaPathIterator_Free.
 *        This function can only be successful after a Read operation has been successfully processed.
 * @param[in] response A pointer to the Read Response to search.
 * @return A pointer to a new PathIterator instance on success.
 * @return NULL on failure.
 */
AwaPathIterator * AwaServerReadResponse_NewPathIterator(const AwaServerReadResponse * response);

/**
 * @brief Retrieve a path result from a Read Response relating to a subscription to that path
 *        retrieved by the corresponding Read operation. Querying the path result will allow
 *        the caller to determine the status of a request to read an object, object instance or resource.
 * @param[in] response A pointer to the Read Response to search.
 * @param[in] path A path to an entity that was requested to be read from.
 * @return NULL if no definition exists in the Read response for the specified path,
 *         or if the response, path or result is NULL.
 * @return Pointer to PathResult instance. Do not free.
 */
const AwaPathResult * AwaServerReadResponse_GetPathResult(const AwaServerReadResponse * response, const char * path);

/**
 * @brief Test if the Read Response has the specified path (resource, object instance or object) present.
 *        If the Read Response contains paths beyond the specified path, the path will be considered present.
 *        This function can only be successful after a Read operation has been successfully processed.
 * @param[in] response A pointer to a valid Read Response.
 * @param[in] path The path with which to query the Read Response.
 * @return True if the Read Response contains the specified resource path.
 * @return False if the Read Response does not contain the specified resource path, or if the path is invalid.
 */
bool AwaServerReadResponse_ContainsPath(const AwaServerReadResponse * response, const char * path);

/**
 * @brief Test if the Read Response has a value for the specified resource path.
 *        If the Read Response contains a value for the specified path, and the Resource Type is known,
 *        it can be retrieved with the appropriate AwaServerReadResponse_ReadValueAs_ function.
 *        This function can only be successful after a Read operation has been successfully processed.
 * @param[in] response A pointer to a valid Read Response.
 * @param[in] path The path with which to query the Read Response.
 * @return True if the Read Response contains a value for the specified resource path.
 * @return False if the Read Response does not contain a value for the specified resource path, or if the path is invalid.
 */
bool AwaServerReadResponse_HasValue(const AwaServerReadResponse * response, const char * path);

/**
 * @defgroup ReadResponse_GetValueAs AwaServerReadResponse_GetValueAs___Pointer
 * @addtogroup ReadResponse_GetValueAs
 * @brief Retrieve a temporary pointer to a resource's value from a Read Response. The resource is identified by the path.
 *        This function can only be successful after a Read operation has been successfully processed.
 *        The pointer is only valid while the operation is valid. If the operation is freed, the
 *        pointer returned is immediately invalid and should not be used or dereferenced.
 * @param[in] response The current Read Response to retrieve the value from.
 * @param[in] path The path of the resource requested for retrieval.
 * @param[in,out] value A pointer to a const pointer that will be modified to point to the requested value. Set to null on error.
 * @return AwaError_Success on success.
 * @return AwaError_TypeMismatch if the resource type corresponding to the specified path is not of the correct type.
 * @return AwaError_PathNotFound if the specified resource path is not covered by the Read Response.
 * @return AwaError_OperationInvalid if the specified operation is invalid or NULL.
 * @return AwaError_PathInvalid if the specified path is invalid or does not correspond to a resource path.
 * @{
 */
AwaError AwaServerReadResponse_GetValueAsCStringPointer    (const AwaServerReadResponse * response, const char * path, const char ** value);
AwaError AwaServerReadResponse_GetValueAsIntegerPointer    (const AwaServerReadResponse * response, const char * path, const AwaInteger ** value);
AwaError AwaServerReadResponse_GetValueAsFloatPointer      (const AwaServerReadResponse * response, const char * path, const AwaFloat ** value);
AwaError AwaServerReadResponse_GetValueAsBooleanPointer    (const AwaServerReadResponse * response, const char * path, const AwaBoolean ** value);
AwaError AwaServerReadResponse_GetValueAsTimePointer       (const AwaServerReadResponse * response, const char * path, const AwaTime ** value);
AwaError AwaServerReadResponse_GetValueAsObjectLinkPointer (const AwaServerReadResponse * response, const char * path, const AwaObjectLink ** value);
AwaError AwaServerReadResponse_GetValueAsOpaquePointer     (const AwaServerReadResponse * response, const char * path, const AwaOpaque ** value);
/** @} */

/**
 * @brief Retrieve an opaque resource's value from a Read Response. The resource is identified by the path.
 *        This function can only be successful after a Read operation has been successfully processed.
 *        A pointer to a AwaOpaque struct is passed in and populated by the function.
 *        The data pointer within the AwaOpaque struct is only valid as long as the Read operation remains valid.
 *        If the operation is freed, the data pointer is immediately invalid and should not be used or dereferenced.
 * @param[in] response The current Read Response to retrieve the value from.
 * @param[in] path The path of the opaque resource requested for retrieval.
 * @param[in,out] value A pointer to a AwaOpaque struct that will be modified to refer to the requested opaque value.
 * @return AwaError_Success on success.
 * @return AwaError_TypeMismatch if the resource type corresponding to the specified path is not of type AwaResourceType_Opaque.
 * @return AwaError_PathNotFound if the specified resource path is not covered by the Read Response.
 * @return AwaError_OperationInvalid if the specified operation is invalid or NULL.
 * @return AwaError_PathInvalid if the specified path is invalid or does not correspond to a resource path.
 */
AwaError AwaServerReadResponse_GetValueAsOpaque(const AwaServerReadResponse * response, const char * path, AwaOpaque * value);

/**
 * @brief Retrieve an object link resource's value from a Read Response. The resource is identified by the path.
 *        This function can only be successful after a Read operation has been successfully processed.
 *        A pointer to a AwaObjectLink struct is passed in and populated by the function.
 * @param[in] response The current Read Response to retrieve the value from.
 * @param[in] path The path of the object link resource requested for retrieval.
 * @param[in,out] value A pointer to a AwaObjectLink struct that will be modified to hold the requested object link value.
 * @return AwaError_Success on success.
 * @return AwaError_TypeMismatch if the resource type corresponding to the specified path is not of type AwaResourceType_ObjectLink.
 * @return AwaError_PathNotFound if the specified resource path is not covered by the Read Response.
 * @return AwaError_OperationInvalid if the specified operation is invalid or NULL.
 * @return AwaError_PathInvalid if the specified path is invalid or does not correspond to a resource path.
 */
AwaError AwaServerReadResponse_GetValueAsObjectLink(const AwaServerReadResponse * response, const char * path, AwaObjectLink * value);

/**
 * @defgroup ReadResponse_GetValuesAsArray AwaServerReadResponse_GetValuesAs___ArrayPointer
 * @addtogroup ReadResponse_GetValuesAsArray
 * @brief Retrieve a temporary pointer to a multiple-instance resource's array value from a Read Response. The resource is identified by the path.
 *        This function can only be successful after a Read operation has been successfully processed.
 *        The pointer is only valid while the operation is valid. If the operation is freed, the
 *        pointer returned is immediately invalid and should not be used or dereferenced.
 * @param[in] response The current Read Response to retrieve the value from.
 * @param[in] path The path of the multiple-instance resource requested for retrieval.
 * @param[in,out] valueArray A pointer to a const pointer that will be modified to point to the requested value. Set to null on error.
 * @return AwaError_Success on success.
 * @return AwaError_TypeMismatch if the resource type corresponding to the specified path is not of the correct type.
 * @return AwaError_PathNotFound if the specified resource path is not covered by the Read Response.
 * @return AwaError_OperationInvalid if the specified operation is invalid or NULL.
 * @return AwaError_PathInvalid if the specified path is invalid or does not correspond to a resource path.
 * @{
 */
AwaError AwaServerReadResponse_GetValuesAsStringArrayPointer    (const AwaServerReadResponse * response, const char * path, const AwaStringArray ** valueArray);
AwaError AwaServerReadResponse_GetValuesAsIntegerArrayPointer   (const AwaServerReadResponse * response, const char * path, const AwaIntegerArray ** valueArray);
AwaError AwaServerReadResponse_GetValuesAsFloatArrayPointer     (const AwaServerReadResponse * response, const char * path, const AwaFloatArray ** valueArray);
AwaError AwaServerReadResponse_GetValuesAsBooleanArrayPointer   (const AwaServerReadResponse * response, const char * path, const AwaBooleanArray ** valueArray);
AwaError AwaServerReadResponse_GetValuesAsTimeArrayPointer      (const AwaServerReadResponse * response, const char * path, const AwaTimeArray ** valueArray);
AwaError AwaServerReadResponse_GetValuesAsOpaqueArrayPointer    (const AwaServerReadResponse * response, const char * path, const AwaOpaqueArray ** valueArray);
AwaError AwaServerReadResponse_GetValuesAsObjectLinkArrayPointer(const AwaServerReadResponse * response, const char * path, const AwaObjectLinkArray ** valueArray);
/** @} */


/**************************************************************************************************
 * Object & Resource Value Setting Operation
 *************************************************************************************************/

/**
 * @brief Get a descriptive name for the specified Write Mode
 * @param[in] writeMode Write Mode
 * @return Pointer to an immutable c string
 */
const char * AwaWriteMode_ToString(AwaWriteMode writeMode);

/**
 * @brief Get a Write Mode from a descriptive name of a Write Mode
 * @param[in] writeModeString A Write Mode in string form
 * @return A Awa Write Mode code
 */
AwaWriteMode AwaWriteMode_FromString(const char * writeModeString);

/**
 * @brief Allocate and return a pointer to a new Write Operation, that can be used to set resource
 *        values.
 *        Optional resources can be created if AwaWriteMode_Update is specified as the default write mode.
 *        New object instances can be created, automatically creating all covered mandatory resources.
 *        Write Operations can be loaded with one or more resource paths with corresponding value.
 *        The Write Operation is owned by the caller and should eventually be freed with AwaServerWriteOperation_Free.
 * @param[in] session A pointer to a valid session.
 * @param[in] defaultMode The default write mode to use; either AwaWriteMode_Replace, or AwaWriteMode_Update.
 * @return A pointer to a newly allocated Write Operation instance.
 * @return NULL on failure.
 */
AwaServerWriteOperation * AwaServerWriteOperation_New(const AwaServerSession * session, AwaWriteMode defaultMode);

/**
 * @brief Adds an object instance path to a Write Operation, as a request to create an Object Instance.
 *        The target object must support creation of a new object instance, such as supporting multiple instances,
 *        or an optional instance that does not exist.
 *        When the operation is processed, if the object instance ID is specified in the path, the object instance
 *        requested will be created (or an error returned if it already exists).
 *        If the object instance ID is not specified in the path, the ID will be automatically assigned by the Core
 *        if the instance can be created.
 * @param[in] operation The Write Operation to add the path of interest to.
 * @param[in] path The path of the object or object instance requested for object instance creation.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_PathInvalid if the specified path is invalid.
 */
AwaError AwaServerWriteOperation_CreateObjectInstance(AwaServerWriteOperation * operation, const char * path);

/**
 * @brief Changes the write mode for an object instance path.
 *        Write Operations on object instances use defaultMode set in AwaServerWriteOperation_New by default,
 *        but can be set to another mode to change the write behavior for specific object instance paths.
 *        Using AwaWriteMode_Replace will only succeed if the object instance already exists.
 * @param[in] operation The Write Operation to add the path of interest to.
 * @param[in] path The path of object instance to modify for a corresponding write.
 * @param[in] mode The write mode to use; either AwaWriteMode_Replace, or AwaWriteMode_Update.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid, or the specified write mode is invalid.
 * @return AwaError_PathInvalid if the specified path is invalid.
 */
AwaError AwaServerWriteOperation_SetObjectInstanceWriteMode(AwaServerWriteOperation * operation, const char * path, AwaWriteMode mode);

/**
 * @brief Changes the write mode for a resource path.
 *        Write Operations on resource use defaultMode set in AwaServerWriteOperation_New by default,
 *        but can be set to another mode to change the write behavior for specific resource paths.
 *        Using AwaWriteMode_Replace will only succeed if the resource already exists.
 *        This function is intended to be used for multiple-instance resources, where AwaWriteMode_Update
 *        will amend an array resource, whereas AwaWriteMode_Replace will entirely replace it with the values
 *        passed in the write operation.
 * @param[in] operation The Write Operation to add the path of interest to.
 * @param[in] path The path of object instance to modify for a corresponding write.
 * @param[in] mode The write mode to use; either AwaWriteMode_Replace, or AwaWriteMode_Update.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid, or the specified write mode is invalid.
 * @return AwaError_PathInvalid if the specified path is invalid.
 */
AwaError AwaServerWriteOperation_SetResourceWriteMode(AwaServerWriteOperation * operation, const char * path, AwaWriteMode mode);

/**
 * @defgroup WriteOperation_AddValueAs AwaServerWriteOperation_AddValueAs___
 * @addtogroup WriteOperation_AddValueAs
 * @brief Adds a path and value to a Write Operation, as a request to change the resource to the specified value.
 * @param[in] operation The Write Operation to add the path and value to.
 * @param[in] path The path of the resource requested for change.
 * @param[in] value The new value of the resource.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_PathInvalid if the specified path is invalid.
 * @return AwaError_TypeMismatch if the specified path refers to a resource with a non-corresponding type.
 * @return AwaError_NotDefined if the specified path refers to an object or resource that is not defined.
 * @{
 */
AwaError AwaServerWriteOperation_AddValueAsCString   (AwaServerWriteOperation * operation, const char * path, const char * value);
AwaError AwaServerWriteOperation_AddValueAsInteger   (AwaServerWriteOperation * operation, const char * path, AwaInteger value);
AwaError AwaServerWriteOperation_AddValueAsFloat     (AwaServerWriteOperation * operation, const char * path, AwaFloat value);
AwaError AwaServerWriteOperation_AddValueAsBoolean   (AwaServerWriteOperation * operation, const char * path, AwaBoolean value);
AwaError AwaServerWriteOperation_AddValueAsTime      (AwaServerWriteOperation * operation, const char * path, AwaTime value);
AwaError AwaServerWriteOperation_AddValueAsOpaque    (AwaServerWriteOperation * operation, const char * path, AwaOpaque value);
AwaError AwaServerWriteOperation_AddValueAsObjectLink(AwaServerWriteOperation * operation, const char * path, AwaObjectLink value);
/** @} */

/**
 * @defgroup WriteOperation_AddValueAsArray AwaServerWriteOperation_AddValueAs___Array
 * @addtogroup WriteOperation_AddValueAsArray
 * @brief Adds a path and array of values to a Write Operation, as a request to change a multiple-instance resource to the specified value.
 *        The array can be sparse and only contain entries for modification. Existing resource instances that are not replaced remain unmodified.
 * @param[in] operation The Set operation to add the path and value to.
 * @param[in] path The path of the resource requested for change.
 * @param[in] array The new value of the resource, given as a sparse array of resource instances.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_PathInvalid if the specified path is invalid.
 * @return AwaError_TypeMismatch if the specified path refers to a resource with a non-corresponding type.
 * @return AwaError_NotDefined if the specified path refers to an object or resource that is not defined.
 * @{
 */
AwaError AwaServerWriteOperation_AddValueAsStringArray    (AwaServerWriteOperation * operation, const char * path, const AwaStringArray * array);
AwaError AwaServerWriteOperation_AddValueAsIntegerArray   (AwaServerWriteOperation * operation, const char * path, const AwaIntegerArray * array);
AwaError AwaServerWriteOperation_AddValueAsFloatArray     (AwaServerWriteOperation * operation, const char * path, const AwaFloatArray * array);
AwaError AwaServerWriteOperation_AddValueAsBooleanArray   (AwaServerWriteOperation * operation, const char * path, const AwaBooleanArray * array);
AwaError AwaServerWriteOperation_AddValueAsTimeArray      (AwaServerWriteOperation * operation, const char * path, const AwaTimeArray * array);
AwaError AwaServerWriteOperation_AddValueAsOpaqueArray    (AwaServerWriteOperation * operation, const char * path, const AwaOpaqueArray * array);
AwaError AwaServerWriteOperation_AddValueAsObjectLinkArray(AwaServerWriteOperation * operation, const char * path, const AwaObjectLinkArray * array);
/** @} */

/**
 * @defgroup WriteOperation_AddArrayValueAs AwaServerWriteOperation_AddArrayValueAs___
 * @addtogroup WriteOperation_AddArrayValueAs
 * @brief Adds a path and value to a Write Operation, as a request to change a multiple-instance resource's instance to the specified value.
 *        This allows a Write Operation to modify a single instance of a multiple-instance resource, leaving all other instances unmodified.
 *        Only values with the same type as the array resource specified by the path can be added.
 * @param[in] operation The Write Operation to add the path and value to.
 * @param[in] path The path of the resource requested for change.
 * @param[in] resourceInstanceID The resource instance ID to change.
 * @param[in] value The new value of the resource instance.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_PathInvalid if the specified path is invalid.
 * @return AwaError_TypeMismatch if the specified path refers to a resource with a non-corresponding type.
 * @return AwaError_NotDefined if the specified path refers to an object or resource that is not defined.
 * @return AwaError_AddInvalid if the specified resource instance ID is outside of the valid range.
 * @{
 */
AwaError AwaServerWriteOperation_AddArrayValueAsCString   (AwaServerWriteOperation * operation, const char * path, int resourceInstanceID, const char * value);
AwaError AwaServerWriteOperation_AddArrayValueAsInteger   (AwaServerWriteOperation * operation, const char * path, int resourceInstanceID, AwaInteger value);
AwaError AwaServerWriteOperation_AddArrayValueAsFloat     (AwaServerWriteOperation * operation, const char * path, int resourceInstanceID, AwaFloat value);
AwaError AwaServerWriteOperation_AddArrayValueAsBoolean   (AwaServerWriteOperation * operation, const char * path, int resourceInstanceID, AwaBoolean value);
AwaError AwaServerWriteOperation_AddArrayValueAsTime      (AwaServerWriteOperation * operation, const char * path, int resourceInstanceID, AwaTime value);
AwaError AwaServerWriteOperation_AddArrayValueAsOpaque    (AwaServerWriteOperation * operation, const char * path, int resourceInstanceID, AwaOpaque value);
AwaError AwaServerWriteOperation_AddArrayValueAsObjectLink(AwaServerWriteOperation * operation, const char * path, int resourceInstanceID, AwaObjectLink value);
/** @} */

/**
 * @brief Process the Write Operation by sending it to the Core. This function is intended to be
 *        called once per connected client in order to set resources more efficiently than being
 *        required to specify multiple client IDs for each path.
 * @param[in] operation The Write Operation to process.
 * @param[in] clientID The name of the client to perform the Write Operation
 * @param[in] timeout The function will wait at least as long as this value for a response.
 * @return AwaError_Success on success.
 * @return AwaError_Timeout if no response is received after the timeout duration expires.
 * @return Various errors on failure.
 */
AwaError AwaServerWriteOperation_Perform(AwaServerWriteOperation * operation, const char * clientID, AwaTimeout timeout);

/**
 * @brief Obtain a Write Response instance from a processed Write Operation. This may be
 *        iterated through to determine whether the write operation succeeded for the requested paths.
 *        The Write Response is owned by the Write Operation and should not be freed by the caller.
 * @param[in] operation The processed Write Operation to obtain the Write response from.
 * @param[in] clientID The endpoint name of the connected client to search.
 * @return Pointer to AwaServerWriteResponse if operation is valid and has a valid response.
 * @return NULL if the operation is invalid, or has no valid response (may not have been processed).
 */
const AwaServerWriteResponse * AwaServerWriteOperation_GetResponse(const AwaServerWriteOperation * operation, const char * clientID);

/**
 * @brief Clean up a Write Operation, freeing all allocated resources.
 *        Once freed, the operation is no longer valid.
 * @param[in,out] operation A pointer to a Write Operation pointer that will be set to NULL.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is not valid.
 */
AwaError AwaServerWriteOperation_Free(AwaServerWriteOperation ** operation);

/**
 * @brief Create a new Client Iterator for a Write Response, used to iterate through the list of client
 *        endpoint names retrieved by the corresponding Write Operation.
 *        The resulting iterator is owned by the caller and should eventually be freed with AwaClientIterator_Free.
 *        This function can only be successful after a Write Operation has been successfully processed.
 * @param[in] operation A pointer to a performed Write Operation to iterate through
 * @return A pointer to a new ClientIterator instance on success.
 * @return NULL on failure.
 */
AwaClientIterator * AwaServerWriteOperation_NewClientIterator(const AwaServerWriteOperation * operation);

/**
 * @brief Create a new Path Iterator for a Write Response, used to iterate through the list of paths
 *        returned in the response of the corresponding Write Operation.
 *        The resulting iterator is owned by the caller and should eventually be freed with AwaPathIterator_Free.
 *        This function can only be successful after a Write Operation has been successfully processed.
 * @param[in] response A pointer to the Write Response to search.
 * @return A pointer to a new PathIterator instance on success.
 * @return NULL on failure.
 */
AwaPathIterator * AwaServerWriteResponse_NewPathIterator(const AwaServerWriteResponse * response);

/**
 * @brief Retrieve a path result from a Write Response relating to a write to that path
 *        retrieved by the corresponding Write Operation. Querying the path result will allow
 *        the caller to determine the status of a request to define objects and resources.
 * @param[in] response A pointer to the Write Response to search.
 * @param[in] path A path to an entity that was written to.
 * @return NULL if no definition exists in the Write response for the specified path,
 *         or if the response, path or result is NULL.
 * @return Pointer to PathResult instance. Do not free.
 */
const AwaPathResult * AwaServerWriteResponse_GetPathResult(const AwaServerWriteResponse * response, const char * path);

/**
 * @brief Test if the Write Response has the specified path to a resource or object present.
 *        If the Write Response contains paths beyond the specified path, the path will be considered present.
 * @param[in] response A pointer to a valid Write Response.
 * @param[in] path The path with which to query the Write Response.
 * @return True if the Write Response contains the specified resource path.
 * @return False if the Write Response does not contain the specified resource path, or if the path is invalid.
 */
bool AwaServerWriteResponse_ContainsPath(const AwaServerWriteResponse * response, const char * path);


/**************************************************************************************************
 * Object Instance & Resource Deletion Operation
 *************************************************************************************************/

/**
 * @brief Allocate and return a pointer to a new Delete operation,
 *        that can be used to delete optional resources, resource instances, and object instances.
 * @param[in] session A pointer to a valid session instance.
 * @return A pointer to a newly allocated Delete operation instance, or NULL on failure.
 */
AwaServerDeleteOperation * AwaServerDeleteOperation_New(const AwaServerSession * session);

/**
 * @brief Adds a path to a Delete operation, as a request to delete the specified object or object instance.
 *        Individual resources cannot be deleted.
 * @param[in] operation A pointer to a valid delete operation.
 * @param[in] clientID The endpoint name of the connected client to request resources to be deleted.
 * @param[in] path The path of the resource, resource instance or object instance requested to delete.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_PathInvalid if the specified path is invalid.
 * @return AwaError_NotDefined if the specified path refers to an object or resource that is not defined.
 */
AwaError AwaServerDeleteOperation_AddPath(AwaServerDeleteOperation * operation, const char * clientID, const char * path);

/**
 * @brief Process the Delete operation by sending it to the Core.
 * @param[in] operation The Delete operation to process.
 * @param[in] timeout The function will wait at least as long as this value for a response.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_Timeout if no response is received after the timeout duration expires.
 * @return Various errors on failure.
 */
AwaError AwaServerDeleteOperation_Perform(AwaServerDeleteOperation * operation, AwaTimeout timeout);

/**
 * @brief Obtain a Delete Response instance from a processed Delete operation. This may be
 *        iterated through to determine whether the delete operation succeeded for the requested paths.
 *        The Delete Response is owned by the Delete operation and should not be freed by the caller.
 * @param[in] operation The processed Delete operation to obtain the Delete response from.
 * @param[in] clientID The endpoint name of the connected client to search.
 * @return Pointer to AwaServerDeleteResponse if operation is valid and has a valid response.
 * @return NULL if the operation is invalid, or has no valid response (may not have been processed).
 */
const AwaServerDeleteResponse * AwaServerDeleteOperation_GetResponse(const AwaServerDeleteOperation * operation, const char * clientID);

/**
 * @brief Clean up a Delete operation, freeing all allocated resources.
 *        Once freed, the operation is no longer valid.
 * @param[in,out] operation A pointer to a Delete operation pointer that will be set to NULL.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is not valid.
 */
AwaError AwaServerDeleteOperation_Free(AwaServerDeleteOperation ** operation);

/**
 * @brief Create a new Client Iterator for a performed Delete Operation, used to iterate through the list of client
 *        endpoint names retrieved by the corresponding Delete operation.
 *        The resulting iterator is owned by the caller and should eventually be freed with AwaClientIterator_Free.
 *        This function can only be successful after a Delete operation has been successfully processed.
 * @param[in] operation A pointer to a performed Delete Operation to iterate through.
 * @return A pointer to a new ClientIterator instance on success.
 * @return NULL on failure.
 */
AwaClientIterator * AwaServerDeleteOperation_NewClientIterator(const AwaServerDeleteOperation * operation);

/**
 * @brief Create a new Path Iterator for a Delete Response, used to iterate through the list of resource paths
 *        retrieved by the corresponding Delete operation.
 *        The resulting iterator is owned by the caller and should eventually be freed with AwaPathIterator_Free.
 *        This function can only be successful after a Delete operation has been successfully processed.
 * @param[in] response A pointer to the Delete Response to search.
 * @return A pointer to a new PathIterator instance on success.
 * @return NULL on failure.
 */
AwaPathIterator * AwaServerDeleteResponse_NewPathIterator(const AwaServerDeleteResponse * response);

/**
 * @brief Retrieve a path result from a Delete Response relating to a subscription to that path
 *        retrieved by the corresponding Delete operation. Querying the path result will allow
 *        the caller to determine the status of a request to delete an object or object instance.
 * @param[in] response A pointer to the Delete Response to search.
 * @param[in] path A path to an entity that was requested to be deleted.
 * @return NULL if no definition exists in the Delete response for the specified path,
 *         or if the response, path or result is NULL.
 * @return Pointer to PathResult instance. Do not free.
 */
const AwaPathResult * AwaServerDeleteResponse_GetPathResult(const AwaServerDeleteResponse * response, const char * path);

/**
 * @brief Test if the Delete Response has the specified path (object instance or object) present.
 *        If the Delete Response contains paths beyond the specified path, the path will be considered present.
 *        This function can only be successful after a Delete operation has been successfully processed.
 * @param[in] response A pointer to a valid Delete Response.
 * @param[in] path The path with which to query the Delete Response.
 * @return True if the Delete Response contains the specified resource path.
 * @return False if the Delete Response does not contain the specified resource path, or if the path is invalid.
 */
bool AwaServerDeleteResponse_ContainsPath(const AwaServerDeleteResponse * response, const char * path);


/**************************************************************************************************
 * Resource Execute Operation
 *************************************************************************************************/

/**
 * @brief Allocate and return a pointer to a new Execute operation, that can be used to trigger
 *        executable resources. An execute arguments instance can be optionally passed in order to provide a way
 *        to control how a resource is executed. The Execute operation is owned by the caller and
 *        should eventually be freed with AwaServerExecuteOperation_Free.
 * @param[in] session A pointer to a valid session.
 * @return A pointer to a newly allocated Execute operation instance.
 * @return NULL on failure.
 */
AwaServerExecuteOperation * AwaServerExecuteOperation_New(const AwaServerSession * session);

/**
 * @brief Adds a path to an Execute operation, as a request to execute the specified resource.
 * @param[in] operation A pointer to a valid Execute operation.
 * @param[in] clientID The endpoint name of the connected client to execute upon.
 * @param[in] path The path of the resource, resource instance or object instance requested to execute.
 * @param[in] arguments Execute arguments, or NULL to execute the resource without arguments.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_PathInvalid if the specified path is invalid.
 * @return AwaError_NotDefined if the specified path refers to an object or resource that is not defined.
 */
AwaError AwaServerExecuteOperation_AddPath(AwaServerExecuteOperation * operation, const char * clientID, const char * path, const AwaExecuteArguments * arguments);

/**
 * @brief Process the Execute operation by sending it to the Core.
 *        In the current version perform only supports executing resources on a single client,
 *        but in the future multiple clients will be supported.
 * @param[in] operation The Execute operation to process.
 * @param[in] timeout The function will wait at least as long as this value for a response.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_Timeout if no response is received after the timeout duration expires.
 * @return Various errors on failure.
 */
AwaError AwaServerExecuteOperation_Perform(AwaServerExecuteOperation * operation, AwaTimeout timeout);

/**
 * @brief Obtain an Execute Response instance from a processed Execute operation. This may be
 *        iterated through to determine whether the execute operation succeeded for the requested resource paths.
 *        The Execute Response is owned by the Execute operation and should not be freed by the caller.
 * @param[in] operation The processed Execute operation to obtain the Execute response from.
 * @param[in] clientID The endpoint name of the connected client to search.
 * @return Pointer to AwaServerExecuteResponse if operation is valid and has a valid response.
 * @return NULL if the operation is invalid, or has no valid response (may not have been processed).
 */
const AwaServerExecuteResponse * AwaServerExecuteOperation_GetResponse(const AwaServerExecuteOperation * operation, const char * clientID);

/**
 * @brief Clean up an Execute operation, freeing all allocated resources.
 *        Once freed, the operation is no longer valid.
 * @param[in,out] operation A pointer to an Execute operation pointer that will be set to NULL.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is not valid.
 */
AwaError AwaServerExecuteOperation_Free(AwaServerExecuteOperation ** operation);

/**
 * @brief Create a new Client Iterator for a performed Execute Operation, used to iterate through the list of client
 *        endpoint names retrieved by the corresponding Execute operation.
 *        The resulting iterator is owned by the caller and should eventually be freed with AwaClientIterator_Free.
 *        This function can only be successful after an Execute operation has been successfully processed.
 * @param[in] operation A pointer to a performed Execute operation to iterate through
 * @return A pointer to a new ClientIterator instance on success.
 * @return NULL on failure.
 */
AwaClientIterator * AwaServerExecuteOperation_NewClientIterator(const AwaServerExecuteOperation * operation);

/**
 * @brief Create a new Path Iterator for an Execute Response, used to iterate through the list of resource paths
 *        retrieved by the corresponding Execute operation.
 *        The resulting iterator is owned by the caller and should eventually be freed with AwaPathIterator_Free.
 *        This function can only be successful after an Execute operation has been successfully processed.
 * @param[in] response A pointer to the Execute Response to search.
 * @return A pointer to a new PathIterator instance on success.
 * @return NULL on failure.
 */
AwaPathIterator * AwaServerExecuteResponse_NewPathIterator(const AwaServerExecuteResponse * response);

/**
 * @brief Retrieve a path result from an Execute Response relating to an execution to that path
 *        retrieved by the corresponding Execute operation. Querying the path result will allow
 *        the caller to determine the status of a request to execute a resource.
 * @param[in] response A pointer to the Execute Response to search.
 * @param[in] path A path to a resource that was requested to be executed.
 * @return NULL if no definition exists in the Execute response for the specified path,
 *         or if the response, path or result is NULL.
 * @return Pointer to PathResult instance. Do not free.
 */
const AwaPathResult * AwaServerExecuteResponse_GetPathResult(const AwaServerExecuteResponse * response, const char * path);

/**
 * @brief Test if the Execute Response has the specified resource path present.
 *        If the Execute Response contains paths beyond the specified path, the path will be considered present.
 *        This function can only be successful after an Execute operation has been successfully processed.
 * @param[in] response A pointer to a valid Execute Response.
 * @param[in] path The path with which to query the Execute Response.
 * @return True if the Execute Response contains the specified resource path.
 * @return False if the Execute Response does not contain the specified resource path, or if the path is invalid.
 */
bool AwaServerExecuteResponse_ContainsPath(const AwaServerExecuteResponse * response, const char * path);


/**************************************************************************************************
 * Write-Attributes Operation
 *************************************************************************************************/

/**
 * @brief Allocate and return a pointer to a new Write Attributes operation, that can be used to modify
 *        when notifications for changes to objects, object instances and resources will be sent from a
 *        connected client to the observing server daemon.
 *        The inverse of the Write Attributes operation is Discover, which allows the discovery of existing
 *        write attributes attached to objects, object instances and resources.
 *        In LWM2M 1.0, only the following attributes are supported:
 *        * Name:Minimum Period, Link:pmin, Attachment:Resource, Type:Integer
 *        * Name:Maximum Period, Link:pmax, Attachment:Resource, Type:Integer
 *        * Name:Greater Than,   Link:gt,   Attachment:Resource, Type:Float
 *        * Name:Less Than,      Link:lt,   Attachment:Resource, Type:Float
 *        * Name:Step,           Link:stp,  Attachment:Resource, Type:Float
 *        The Write Attributes operation is owned by the caller and should eventually be freed with
 *        AwaServerWrite AttributesOperation_Free.
 * @param[in] session A pointer to a valid session.
 * @return A pointer to a newly allocated Write Attributes operation instance.
 * @return NULL on failure.
 */
AwaServerWriteAttributesOperation * AwaServerWriteAttributesOperation_New(const AwaServerSession * session);

/**
 * @defgroup WriteAttributesOperation_AddAttributeAs AwaServerWriteAttributesOperation_AddAttributeAs___
 * @addtogroup WriteAttributesOperation_AddAttributeAs
 * @brief Adds a path and value to a Write Attributes operation, as a request to change the value of a
 *        object, object instance or resource level attribute on the specified path.
 * @param[in] operation The Write Attributes operation to add the path and attribute value to.
 * @param[in] clientID The endpoint name of the client to update the changed attributes.
 * @param[in] path The path of the resource requested for change.
 * @param[in] link The link identifying which attribute to be set (currently limited to "pmin", "pmax", "gt", "lt", and "stp").
 * @param[in] value The new value of the resource.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_PathInvalid if the specified path is invalid.
 * @return AwaError_TypeMismatch if the specified path refers to a resource with a non-corresponding type.
 * @return AwaError_NotDefined if the specified path refers to an object or resource that is not defined.
 * @{
 */
AwaError AwaServerWriteAttributesOperation_AddAttributeAsInteger   (AwaServerWriteAttributesOperation * operation, const char * clientID, const char * path, const char * link, AwaInteger value);
AwaError AwaServerWriteAttributesOperation_AddAttributeAsFloat     (AwaServerWriteAttributesOperation * operation, const char * clientID, const char * path, const char * link, AwaFloat value);
// not yet implemented:
//AwaError AwaServerWriteAttributesOperation_AddAttributeAsCString   (AwaServerWriteAttributesOperation * operation, const char * path, const char * link, const char * value);
//AwaError AwaServerWriteAttributesOperation_AddAttributeAsBoolean   (AwaServerWriteAttributesOperation * operation, const char * path, const char * link, AwaBoolean value);
//AwaError AwaServerWriteAttributesOperation_AddAttributeAsTime      (AwaServerWriteAttributesOperation * operation, const char * path, const char * link, AwaTime value);
//AwaError AwaServerWriteAttributesOperation_AddAttributeAsOpaque    (AwaServerWriteAttributesOperation * operation, const char * path, const char * link, AwaOpaque value);
//AwaError AwaServerWriteAttributesOperation_AddAttributeAsObjectLink(AwaServerWriteAttributesOperation * operation, const char * path, const char * link, AwaObjectLink value);
/** @} */

/**
 * @brief Process the Write Attributes operation by sending it to the Core.
 *        All attribute values must be verified before being written for the result to be successful.
 *        Any error will result in no attributes being written.
 * @param[in] operation The Write Attributes operation to process.
 * @param[in] timeout The function will wait at least as long as this value for a response.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_Timeout if no response is received after the timeout duration expires.
 * @return Various errors on failure.
 */
AwaError AwaServerWriteAttributesOperation_Perform(AwaServerWriteAttributesOperation * operation, AwaTimeout timeout);

/**
 * @brief Obtain a Write Attributes Response instance from a processed Write Attributes operation. This may be
 *        iterated through to determine whether the Write Attributes operation succeeded on the requested paths.
 *        The Write Attributes Response is owned by the Write Attributes operation and should not be freed by the caller.
 * @param[in] operation The processed Write Attributes operation to obtain the Write Attributes response from.
 * @param[in] clientID The endpoint name of the connected client to search.
 * @return Pointer to AwaServerWrite AttributesResponse if operation is valid and has a valid response.
 * @return NULL if the operation is invalid, or has no valid response (may not have been processed).
 */
const AwaServerWriteAttributesResponse * AwaServerWriteAttributesOperation_GetResponse(const AwaServerWriteAttributesOperation * operation, const char * clientID);

/**
 * @brief Clean up a Write Attributes operation, freeing all allocated resources.
 *        Once freed, the operation is no longer valid.
 * @param[in,out] operation A pointer to a Write Attributes operation pointer that will be set to NULL.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is not valid.
 */
AwaError AwaServerWriteAttributesOperation_Free(AwaServerWriteAttributesOperation ** operation);

/**
 * @brief Create a new Client Iterator for a performed Write Attributes Operation, used to iterate through the list of client
 *        endpoint names retrieved by the corresponding Write Attributes operation.
 *        The resulting iterator is owned by the caller and should eventually be freed with AwaClientIterator_Free.
 *        This function can only be successful after a Write Attributes operation has been successfully processed.
 * @param[in] operation A pointer to a performed Write Attributes operation to iterate through
 * @return A pointer to a new ClientIterator instance on success.
 * @return NULL on failure.
 */
AwaClientIterator * AwaServerWriteAttributesOperation_NewClientIterator(const AwaServerWriteAttributesOperation * operation);

/**
 * @brief Create a new Path Iterator for a Write Attributes Response, used to iterate through the list of paths
 *        retrieved by the corresponding Write Attributes operation.
 *        The resulting iterator is owned by the caller and should eventually be freed with AwaPathIterator_Free.
 *        This function can only be successful after a Write Attributes operation has been successfully processed.
 * @param[in] response A pointer to the Write Attributes Response to search.
 * @return A pointer to a new PathIterator instance on success.
 * @return NULL on failure.
 */
AwaPathIterator * AwaServerWriteAttributesResponse_NewPathIterator(const AwaServerWriteAttributesResponse * response);

/**
 * @brief Retrieve a path result from a Write Attributes Response relating to a subscription to that path
 *        retrieved by the corresponding Write Attributes operation. Querying the path result will allow
 *        the caller to determine the status of a request to Write Attributes to an object, object instance or resource.
 * @param[in] response A pointer to the Write Attributes Response to search.
 * @param[in] path A path to an entity that had attributes written to.
 * @return NULL if no definition exists in the Write Attributes response for the specified path,
 *         or if the response, path or result is NULL.
 * @return Pointer to PathResult instance. Do not free.
 */
const AwaPathResult * AwaServerWriteAttributesResponse_GetPathResult(const AwaServerWriteAttributesResponse * response, const char * path);

/**
 * @brief Test if the Write Attributes Response has the specified path (resource, object instance or object) present.
 *        If the Write Attributes Response contains paths beyond the specified path, the path will be considered present.
 *        This function can only be successful after a Write Attributes operation has been successfully processed.
 * @param[in] response A pointer to a valid Write Attributes Response.
 * @param[in] path The path with which to query the Write Attributes Response.
 * @return True if the Write Attributes Response contains the specified resource path.
 * @return False if the Write Attributes Response does not contain the specified resource path, or if the path is invalid.
 */
bool AwaServerWriteAttributesResponse_ContainsPath(const AwaServerWriteAttributesResponse * response, const char * path);


/**************************************************************************************************
 * Discover Operation
 *************************************************************************************************/

/**
 * @brief Allocate and return a pointer to a new Discover operation, that can be used to discover
 *        attributes attached to objects, object instances and resources. The discover operation
 *        can also be used to discover which resources are implemented for a given object instance.
 *        The Discover operation is owned by the caller and should eventually be freed with
 *        AwaServerDiscoverOperation_Free.
 * @param[in] session A pointer to a valid session.
 * @return A pointer to a newly allocated Discover operation instance.
 * @return NULL on failure.
 */
AwaServerDiscoverOperation * AwaServerDiscoverOperation_New(const AwaServerSession * session);

/**
 * @brief Adds a path to a Discover operation, as a request to discover the specified resource.
 * @param[in] operation A pointer to a valid Discover operation.
 * @param[in] clientID The endpoint name of the client to discover entities.
 * @param[in] path The path of the resource, resource instance or object instance requested to discover.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_PathInvalid if the specified path is invalid.
 * @return AwaError_NotDefined if the specified path refers to an object or resource that is not defined.
 */
AwaError AwaServerDiscoverOperation_AddPath(AwaServerDiscoverOperation * operation, const char * clientID, const char * path);

/**
 * @brief Process the Discover operation by sending it to the Core.
 * @param[in] operation The Discover operation to process.
 * @param[in] timeout The function will wait at least as long as this value for a response.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_Timeout if no response is received after the timeout duration expires.
 * @return Various errors on failure.
 */
AwaError AwaServerDiscoverOperation_Perform(AwaServerDiscoverOperation * operation, AwaTimeout timeout);

/**
 * @brief Obtain a Discover Response instance from a processed Discover operation. This may be
 *        iterated through to determine whether the Discover operation succeeded on the requested paths.
 *        The Discover Response is owned by the Discover operation and should not be freed by the caller.
 * @param[in] operation The processed Discover operation to obtain the Discover response from.
 * @param[in] clientID The endpoint name of the connected client to perform the discover operation upon.
 * @return Pointer to AwaServerDiscoverResponse if operation is valid and has a valid response.
 * @return NULL if the operation is invalid, or has no valid response (may not have been processed).
 */
const AwaServerDiscoverResponse * AwaServerDiscoverOperation_GetResponse(const AwaServerDiscoverOperation * operation, const char * clientID);

/**
 * @brief Clean up a Discover operation, freeing all allocated resources.
 *        Once freed, the operation is no longer valid.
 * @param[in,out] operation A pointer to a Discover operation pointer that will be set to NULL.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is not valid.
 */
AwaError AwaServerDiscoverOperation_Free(AwaServerDiscoverOperation ** operation);

/**
 * @brief Create a new Client Iterator for a performed Discover Operation, used to iterate through the list of client
 *        endpoint names retrieved by the corresponding Discover operation.
 *        The resulting iterator is owned by the caller and should eventually be freed with AwaClientIterator_Free.
 *        This function can only be successful after a Discover operation has been successfully processed.
 * @param[in] operation A pointer to a performed Discover operation to iterate through.
 * @return A pointer to a new ClientIterator instance on success.
 * @return NULL on failure.
 */
AwaClientIterator * AwaServerDiscoverOperation_NewClientIterator(const AwaServerDiscoverOperation * operation);

/**
 * @brief Create a new Path Iterator for a Discover Response, used to iterate through the list of paths
 *        retrieved by the corresponding Discover operation.
 *        The resulting iterator is owned by the caller and should eventually be freed with AwaPathIterator_Free.
 *        This function can only be successful after a Discover operation has been successfully processed.
 * @param[in] response A pointer to the Discover Response to search.
 * @return A pointer to a new PathIterator instance on success.
 * @return NULL on failure.
 */
AwaPathIterator * AwaServerDiscoverResponse_NewPathIterator(const AwaServerDiscoverResponse * response);

/**
 * @brief Retrieve a path result from a Discover Response relating to a subscription to that path
 *        retrieved by the corresponding Discover operation. Querying the path result will allow
 *        the caller to determine the status of a request to Discover an object, object instance or resource.
 * @param[in] response A pointer to the Discover Response to search.
 * @param[in] path A path to an entity that had attributes written to.
 * @return NULL if no definition exists in the Discover response for the specified path,
 *         or if the response, path or result is NULL.
 * @return Pointer to PathResult instance. Do not free.
 */
const AwaPathResult * AwaServerDiscoverResponse_GetPathResult(const AwaServerDiscoverResponse * response, const char * path);

/**
 * @brief Test if the Discover Response has the specified path (resource, object instance or object) present.
 *        If the Discover Response contains paths beyond the specified path, the path will be considered present.
 *        This function can only be successful after a Discover operation has been successfully processed.
 * @param[in] response A pointer to a valid Discover Response.
 * @param[in] path The path with which to query the Discover Response.
 * @return True if the Discover Response contains the specified resource path.
 * @return False if the Discover Response does not contain the specified resource path, or if the path is invalid.
 */
bool AwaServerDiscoverResponse_ContainsPath(const AwaServerDiscoverResponse * response, const char * path);

/**
 * @defgroup DiscoverResponse_GetAttributeValueAs AwaServerDiscoverResponse_GetAttributeValueAs___Pointer
 * @addtogroup DiscoverResponse_GetAttributeValueAs
 * @brief Retrieve a temporary pointer to an object, object instance or resource attribute's value from a
 *        Discover Response. This function can only be successful after a Discover operation has been successfully processed.
 *        The pointer is only valid while the operation is valid. If the operation is freed, the
 *        pointer returned is immediately invalid and should not be used or dereferenced.
 * @param[in] response The current Discover Response to retrieve the value from.
 * @param[in] path The path of the resource requested for retrieval.
 * @param[in] link The link identifying which attribute value to be retrieved (currently limited to "pmin", "pmax", "gt", "lt", and "stp").
 * @param[in,out] value A pointer to a const pointer that will be modified to point to the requested value.
 * @return AwaError_Success on success.
 * @return AwaError_TypeMismatch if the resource type corresponding to the specified path is not of the correct type.
 * @return AwaError_PathNotFound if the specified resource path is not covered by the Discover Response.
 * @return AwaError_OperationInvalid if the specified operation is invalid or NULL.
 * @return AwaError_PathInvalid if the specified path is invalid or does not correspond to a resource path.
 * @{
 */
AwaError AwaServerDiscoverResponse_GetAttributeValueAsIntegerPointer(const AwaServerDiscoverResponse * response, const char * path, const char * link, const AwaInteger ** value);
AwaError AwaServerDiscoverResponse_GetAttributeValueAsFloatPointer  (const AwaServerDiscoverResponse * response, const char * path, const char * link, const AwaFloat ** value);
//AwaError AwaServerDiscoverResponse_GetAttributeValueAsCStringPointer(const AwaServerDiscoverResponse * response, const char * path, const char * link, const char ** value);
//AwaError AwaServerDiscoverResponse_GetAttributeValueAsBooleanPointer(const AwaServerDiscoverResponse * response, const char * path, const char * link, const AwaBoolean ** value);
//AwaError AwaServerDiscoverResponse_GetAttributeValueAsTimePointer   (const AwaServerDiscoverResponse * response, const char * path, const char * link, const AwaTime ** value);
//AwaError AwaServerDiscoverResponse_GetAttributeValueAsOpaque        (const AwaServerDiscoverResponse * response, const char * path, const char * link, AwaOpaque * value);
//AwaError AwaServerDiscoverResponse_GetAttributeValueAsObjectLink    (const AwaServerDiscoverResponse * response, const char * path, const char * link, AwaObjectLink * value);
/** @} */

/**
 * @brief Create a new Attribute Iterator for a Discover Response, used to iterate through the list of attributes
 *        retrieved by the corresponding Discover operation. Each path may have zero or more attributes.
 *        The resulting iterator is owned by the caller and should eventually be freed with AwaAttributeIterator_Free.
 *        This function can only be successful after a Discover operation has been successfully processed.
 * @param[in] response A pointer to the Discover Response to search.
 * @param[in] path The path to an entity to explore discovered attributes.
 * @return A pointer to a new PathIterator instance on success.
 * @return NULL on failure.
 */
AwaAttributeIterator * AwaServerDiscoverResponse_NewAttributeIterator(const AwaServerDiscoverResponse * response, const char * path);

/**
 * @brief Determine whether the Attribute Iterator has not reached the end of a list of attributes.
 *        If the iterator has not reached the end, advance its position to the next attribute.
 * @param[in] iterator A pointer to a valid Attribute Iterator.
 * @return True if the Attribute Iterator is valid and the iterator has not yet reached the end of its elements.
 * @return False if the Attribute Iterator is invalid, or has reached the end of its elements.
 */
bool AwaAttributeIterator_Next(AwaAttributeIterator * iterator);

/**
 * @brief Retrieve the link name associated with the current attribute in the Attribute Iterator.
 * @param[in] iterator A pointer to a valid Attribute Iterator.
 * @return The path if the Attribute Iterator is valid.
 * @return NULL if the Attribute Iterator is invalid.
 */
const char * AwaAttributeIterator_GetLink(const AwaAttributeIterator * iterator);

/**
 * @brief Clean up a Attribute Iterator, freeing all allocated resources.
 *        Once freed, the iterator is no longer valid.
 * @param[in,out] iterator A pointer to a Attribute Iterator pointer that will be set to NULL.
 */
void AwaAttributeIterator_Free(AwaAttributeIterator ** iterator);


/**************************************************************************************************
 * Observe Operation
 *************************************************************************************************/

/**
 * @brief Allocate and return a pointer to a new Observe operation,
 *        that can be used to observe to and be notified of changes to objects,
 *        object instances and resource values from the Core.
 *        Observe operations can be loaded with one or more paths of interest, corresponding to
 *        resources or trees of resources that the application wishes to observe. When
 *        processed, the response is made available via AwaServerObserveOperation_GetResponse
 *        and will contain result information for each of the paths returned by the Core in response to the query.
 *        The Observe operation is owned by the caller and should eventually be freed with AwaServerObserveOperation_Free.
 * @param[in] session A pointer to a valid session.
 * @return Pointer to a newly allocated Observe operation instance.
 * @return NULL on failure.
 */
AwaServerObserveOperation * AwaServerObserveOperation_New(const AwaServerSession * session);

/**
 * @brief Adds an Observation of interest to an Observe operation, as a request to be notified of
 *        changes to the observed entity from the Core.
 * @param[in] operation The Observe operation to add the path of interest to.
 * @param[in] observation A valid Change observation to an entity of interest.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_ObservationInvalid if the observation is invalid.
 */
AwaError AwaServerObserveOperation_AddObservation(AwaServerObserveOperation * operation, AwaServerObservation * observation);

/**
 * @brief Adds a Cancel flag to an Observation in a specified Observe operation, as a request to cancel
 *        being notified of changes to the observed entity from the Core.
 * @param[in] operation The Observe operation to add the path of interest to.
 * @param[in] observation A valid Change observation to an entity of interest.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_ObservationInvalid if the observation is invalid.
 */
AwaError AwaServerObserveOperation_AddCancelObservation(AwaServerObserveOperation * operation, AwaServerObservation * observation);

/**
 * @brief Process the Observe operation by sending it to the Core, notifying the Core to
 *        begin or cancel observation to changes of the observed entities inside
 *        the specified Observe operation.
 *        If successful, the response can be obtained with AwaClientObserveOperation_GetResponse
 *        and queried for path results, each of which relate to the result of a single request to begin or cancel an observation.
 * @param[in] operation The Observe operation to process.
 * @param[in] timeout The function will wait at least as long as this value for a response.
 * @return AwaError_Success on success.
 * @return AwaError_Timeout if no response is received after the timeout duration expires.
 * @return Various errors on failure.
 */
AwaError AwaServerObserveOperation_Perform(AwaServerObserveOperation * operation, AwaTimeout timeout);

/**
 * @brief Obtain an Observe Response instance from a processed Observe operation. This may be
 *        queried directly by path to obtain results of an observation request returned by the Core.
 *        A Path iterator can be used to obtain a list of all observation paths provided by
 *        the Observe Response.
 *        The Observe Response is owned by the Observe operation and should not be freed by the caller.
 * @param[in] operation The processed Observe operation to obtain the Observe response from.
 * @param[in] clientID The endpoint name of the connected client to process an observe response from.
 * @return Pointer to AwaClientObserveResponse if operation is valid and has a valid response.
 * @return NULL if the operation is invalid, or has no valid response (may not have been processed).
 */
const AwaServerObserveResponse * AwaServerObserveOperation_GetResponse(const AwaServerObserveOperation * operation, const char * clientID);

/**
 * @brief Clean up an Observe operation, freeing all allocated resources.
 *        Once freed, the operation is no longer valid.
 *        Note: freeing an Observe operation will invalidate all derived Observe Response instances.
 * @param[in,out] operation A pointer to an Observe operation pointer that will be set to NULL.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is not valid.
 */
AwaError AwaServerObserveOperation_Free(AwaServerObserveOperation ** operation);

/**
 * @brief Allocate and return a pointer to a new Observation, that can be used to observe
 *        to changes on either a resource, object instance or object. The specified callback function will be fired on
 *        AwaServerSession_DispatchCallbacks if the observed entity has changed since the session callbacks were
 *        last dispatched. The observation will not be active until it is added to an Observe operation and performed.
 *        Operations can be created within a callback, but they cannot be performed.
 *        The caller must keep any observations valid until the entire observe operation is freed.
 *        The Observation is owned by the caller and should eventually be freed with
 *        AwaServerObservation_Free.
 * @param[in] clientID The endpoint name of the connected client intended to begin or cancel observations upon.
 * @param[in] path The path of the resource, object instance or object to observe.
 * @param[in] callback Pointer to function to call when the entity specified by path is created, changed or deleted.
 * @param[in] context A pointer to a user specified object for use in the change callback
 * @return A pointer to a newly allocated Observation instance.
 * @return NULL on failure.
 */
AwaServerObservation * AwaServerObservation_New(const char * clientID, const char * path, AwaServerObservationCallback callback, void * context);

/**
 * @brief Retrieve the path from an Observation.
 * @param[in] observation A valid Observation.
 * @return The observation's path if the observation is valid.
 * @return NULL if the observation is not valid.
 */
const char * AwaServerObservation_GetPath(AwaServerObservation * observation);

/**
 * @brief Shut down an Observation, freeing any allocated memory. This function
 *        should eventually be called on every operation to avoid a memory leak.
 * @param[in,out] observation A pointer to an Observation pointer, which will be set to NULL by this function.
 * @return AwaError_Success on success
 * @return Error_OperationInvalid if the operation is not valid.
 */
AwaError AwaServerObservation_Free(AwaServerObservation ** observation);

/**
 * @brief Create a new Client Iterator for a performed Observe Operation, used to iterate through the list of client
 *        endpoint names retrieved by the corresponding Observe operation.
 *        The resulting iterator is owned by the caller and should eventually be freed with AwaClientIterator_Free.
 *        This function can only be successful after an Observe operation has been successfully processed.
 * @param[in] operation A pointer to a performed Discover operation to iterate through.
 * @return A pointer to a new ClientIterator instance on success.
 * @return NULL on failure.
 */
AwaClientIterator * AwaServerObserveOperation_NewClientIterator(const AwaServerObserveOperation * operation);

/**
 * @brief Create a new Path Iterator for an Observe Response, used to iterate through the list of object, object instance
 *        and resource paths retrieved by the corresponding Observe operation.
 *        The resulting iterator is owned by the caller and should eventually be freed with AwaPathIterator_Free.
 *        This function can only be successful after an Observe operation has been successfully processed.
 * @param[in] response A pointer to the Observe Response to search.
 * @return A pointer to a new PathIterator instance on success.
 * @return NULL on failure.
 */
AwaPathIterator * AwaServerObserveResponse_NewPathIterator(const AwaServerObserveResponse * response);

/**
 * @brief Retrieve a path result from an Observe Response relating to a observation to that path
 *        retrieved by the corresponding Observe operation. Querying the path result will allow
 *        the caller to determine the status of a request to observe a resource.
 * @param[in] response A pointer to the Observe Response to search.
 * @param[in] path A path to a resource that was requested to be observed.
 * @return NULL if no definition exists in the Observe response for the specified path,
 *         or if the response, path or result is NULL.
 * @return Pointer to PathResult instance. Do not free.
 */
const AwaPathResult * AwaServerObserveResponse_GetPathResult(const AwaServerObserveResponse * response, const char * path);

/**
 * @brief Test if the Observe Response has the specified object, object instance or resource path present.
 *        If the Observe Response contains paths beyond the specified path, the path will be considered present.
 *        This function can only be successful after an Observe operation has been successfully processed.
 * @param[in] response A pointer to a valid Observe Response.
 * @param[in] path The path with which to query the Observe Response.
 * @return True if the Observe Response contains the specified path.
 * @return False if the Observe Response does not contain the specified path, or if the path is invalid.
 */
bool AwaServerObserveResponse_ContainsPath(const AwaServerObserveResponse * response, const char * path);


/**************************************************************************************************
 * ChangeSet Functions for Observations
 *************************************************************************************************/

/**
 * @brief Retrieve the corresponding session for a ChangeSet.
 *        This function is only valid for a ChangeSet connected to a server session.
 * @param[in] changeSet A pointer to a valid ChangeSet.
 * @return The session for the given ChangeSet.
 * @return NULL if the ChangeSet is invalid.
 */
const AwaServerSession * AwaChangeSet_GetServerSession(const AwaChangeSet * changeSet);

/**
 * @brief Retrieve the corresponding client ID for a ChangeSet.
 *        This function is only valid for a ChangeSet connected to a server session.
 * @param[in] changeSet A pointer to a valid ChangeSet.
 * @return The session for the given ChangeSet.
 * @return NULL if the ChangeSet is invalid.
 */
const char * AwaChangeSet_GetClientID(const AwaChangeSet * changeSet);


/**************************************************************************************************
 * Server Events
 *************************************************************************************************/

/**
 * @brief Sets a callback function to be called whenever a Client registers with the server.
 *        Any existing callback function is replaced.
 *
 *        The callback of type ::AwaServerClientRegisterEventCallback will be passed an event
 *        object and the context pointer specified here.
 *
 * @param[in] session A pointer to a valid session.
 * @param[in] callback Function pointer to call when a client registers with the server. Pass NULL to clear.
 * @param[in] context A pointer to a user specified object for use in the callback.
 * @return AwaError_Success on success.
 * @return AwaError_SessionInvalid if the specified session is invalid.
 */
AwaError AwaServerSession_SetClientRegisterEventCallback(AwaServerSession * session, AwaServerClientRegisterEventCallback callback, void * context);

/**
 * @brief Sets a callback function to be called whenever a Client updates its registration with the server.
 *        Any existing callback function is replaced.
 *
 *        The callback of type ::AwaServerClientUpdateEventCallback will be passed an event
 *        object and the context pointer specified here.
 *
 * @param[in] session A pointer to a valid session.
 * @param[in] callback Function pointer to call when a client updates its registration with the server. Pass NULL to clear.
 * @param[in] context A pointer to a user specified object for use in the callback.
 * @return AwaError_Success on success.
 * @return AwaError_SessionInvalid if the specified session is invalid.
 */
AwaError AwaServerSession_SetClientUpdateEventCallback(AwaServerSession * session, AwaServerClientUpdateEventCallback callback, void * context);

/**
 * @brief Sets a callback function to be called whenever a Client deregisters with the server.
 *        Any existing callback function is replaced.
 *
 *        The callback of type ::AwaServerClientDeregisterEventCallback will be passed an event
 *        object and the context pointer specified here.
 *
 * @param[in] session A pointer to a valid session.
 * @param[in] callback Function pointer to call when a client deregisters with the server. Pass NULL to clear.
 * @param[in] context A pointer to a user specified object for use in the callback.
 * @return AwaError_Success on success.
 * @return AwaError_SessionInvalid if the specified session is invalid.
 */
AwaError AwaServerSession_SetClientDeregisterEventCallback(AwaServerSession * session, AwaServerClientDeregisterEventCallback callback, void * context);

/**
 * @brief Create a new Client Iterator from a Client Register Event, used to iterate through
 *        the list of client endpoint names (IDs) held by the event.
 *        The resulting iterator is owned by the caller and should eventually be freed with ::AwaClientIterator_Free.
 *        This function can only be successful within a Client Register Event callback.
 * @param[in] event A pointer to a Client Register Event to iterate through.
 * @return A pointer to a new AwaClientIterator instance on success.
 * @return NULL on failure.
 */
AwaClientIterator * AwaServerClientRegisterEvent_NewClientIterator(const AwaServerClientRegisterEvent * event);

/**
 * @brief Create a new Client Iterator from a Client Update Event, used to iterate through
 *        the list of client endpoint names (IDs) held by the event.
 *        The resulting iterator is owned by the caller and should eventually be freed with ::AwaClientIterator_Free.
 *        This function can only be successful within a Client Update Event callback.
 * @param[in] event A pointer to a Client Update Event to iterate through.
 * @return A pointer to a new AwaClientIterator instance on success.
 * @return NULL on failure.
 */
AwaClientIterator * AwaServerClientUpdateEvent_NewClientIterator(const AwaServerClientUpdateEvent * event);

/**
 * @brief Create a new Client Iterator from a Client Deregister Event, used to iterate through
 *        the list of client endpoint names (IDs) held by the event.
 *        The resulting iterator is owned by the caller and should eventually be freed with ::AwaClientIterator_Free.
 *        This function can only be successful within a Client Deregister Event callback.
 * @param[in] event A pointer to a Client Register Event to iterate through.
 * @return A pointer to a new AwaClientIterator instance on success.
 * @return NULL on failure.
 */
AwaClientIterator * AwaServerClientDeregisterEvent_NewClientIterator(const AwaServerClientDeregisterEvent * event);

/**
 * @brief Create a new Registered Entity iterator for a Client Register Event, used to iterate through the list of
 *        objects and object instances that exist within the client the event relates to.
 *        The resulting iterator is owned by the caller and should eventually be freed with ::AwaRegisteredEntityIterator_Free.
 *        This function can only be successful within a Client Register Event callback.
 * @param[in] event A pointer to the Client Register Event to search.
 * @param[in] clientID The endpoint name of the registered client to search.
 * @return A pointer to a new AwaRegisteredEntityIterator instance on success.
 * @return NULL on failure.
 */
AwaRegisteredEntityIterator * AwaServerClientRegisterEvent_NewRegisteredEntityIterator(const AwaServerClientRegisterEvent * event, const char * clientID);

/**
 * @brief Create a new Registered Entity iterator for a Client Update Event, used to iterate through the list of
 *        objects and object instances that exist within the client the event relates to.
 *        The resulting iterator is owned by the caller and should eventually be freed with ::AwaRegisteredEntityIterator_Free.
 *        This function can only be successful within a Client Update Event callback.
 * @param[in] event A pointer to the Client Update Event to search.
 * @param[in] clientID The endpoint name of the registered client to search.
 * @return A pointer to a new AwaRegisteredEntityIterator instance on success.
 * @return NULL on failure.
 */
AwaRegisteredEntityIterator * AwaServerClientUpdateEvent_NewRegisteredEntityIterator(const AwaServerClientUpdateEvent * event, const char * clientID);


#ifdef __cplusplus
}
#endif

#endif // AWA_SERVER_H

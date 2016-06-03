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
 * @file client.h
 * @brief This interface enables an application to interact with the resources hosted by the LWM2M Client Core.
 *
 * The Core may be running as a separate process (daemon), or it may be linked with the application directly.
 *
 * The Core hosts resources within a data model based on Objects, Object Instances and Resources.
 * Please consult the LWM2M specification for details of this model.
 *
 * A Management Application may interact with the Client via LWM2M, accessing the same resources. Therefore
 * it is essential that both the Client Application and Management Application are aware of the same data
 * model.
 *
 * It is recommended that registered IDs are used for objects that conform to registered LWM2M objects such
 * as IPSO objects.
 */

#ifndef AWA_CLIENT_H
#define AWA_CLIENT_H

// @cond
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
// @endcond

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
 * Example Usage
 *************************************************************************************************/

/**
* @example client-get-example.c
*        Create, process and free a basic Get request on a single resource.
* @example client-get-contains-path-example.c
*        Create, process and free a basic Get request on a single resource and check
*        whether a path exists in the response.
* @example client-get-array-example.c
*        Create, process and free a basic Get request on an array resource and iterate
*        through the response array.
* @example client-set-example.c
*        Create, process and free a basic Set request on a single resource.
* @example client-set-create-example.c
*        A set example on a custom object and resource which must be created.
* @example client-set-array-example.c
*        Create, process and free a basic Set request on an array resource.
* @example client-subscribe-to-change-example.c
*        Subscribe and receive notification to a change of a single resource.
* @example client-subscribe-to-execute-example.c
*        Subscribe and receive notification of an execution of a resource.
* @example client-defineset-example.c
*        Create, process and free a define request to define a custom object with resources.
* @example client-definition-iterator-example.c
*        Iterate through object and resource definitions and print them to stdout
* @example client-delete-example.c
*        Create, process and free a basic Delete request on a single resource.
*/


/**************************************************************************************************
 * Public Types and Enumerations
 *************************************************************************************************/

/// @cond ignore
typedef struct _AwaClientSession            AwaClientSession;
typedef struct _AwaClientGetOperation       AwaClientGetOperation;
typedef struct _AwaClientGetResponse        AwaClientGetResponse;
typedef struct _AwaClientDefineOperation    AwaClientDefineOperation;
typedef struct _AwaClientSetOperation       AwaClientSetOperation;
typedef struct _AwaClientSetResponse        AwaClientSetResponse;
typedef struct _AwaClientDeleteOperation    AwaClientDeleteOperation;
typedef struct _AwaClientDeleteResponse     AwaClientDeleteResponse;
typedef struct _AwaClientSubscribeOperation AwaClientSubscribeOperation;
typedef struct _AwaClientSubscribeResponse  AwaClientSubscribeResponse;
typedef struct _AwaClientSubscription       AwaClientChangeSubscription;
typedef struct _AwaClientSubscription       AwaClientExecuteSubscription;
/// @endcond

/**************************************************************************************************
 * Callbacks
 *************************************************************************************************/

/**
 * @brief A user-specified callback handler for a Change Subscription which will be fired on
 *        AwaClientSession_DispatchCallbacks if the subscribed entity has changed since
 *        the subscription's session callbacks were last dispatched.
 *        Warning: Do NOT process any operations while inside a change callback!
 * @param[in] changeSet A pointer to a valid ChangeSet.
 * @param[in] context A pointer to user-specified data passed to AwaClientChangeSubscription_New
 */
typedef void (*AwaClientSubscribeToChangeCallback)(const AwaChangeSet * changeSet, void * context);

/**
 * @brief A user-specified callback handler for an Execute Subscription which will be fired on
 *        AwaClientSession_DispatchCallbacks if the subscribed entity has been executed since
 *        the subscription's session callbacks were last dispatched.
 *        Warning: Do NOT process any operations while inside an execute callback!
 * @param[in] arguments A pointer to arguments passed to the execution
 * @param[in] context A pointer to user-specified data passed to AwaClientChangeSubscription_New
 */
typedef void (*AwaClientSubscribeToExecuteCallback)(const AwaExecuteArguments * arguments, void * context);


/**************************************************************************************************
 * Session Management
 *************************************************************************************************/

/**
 * @brief Initialise a new session of the Awa subsystem.
 *        A Session is required for interaction with the Awa Core.
 *        Operations to interact with Core resources are created in the context of a
 *        session, however the session must be connected before an operation can be processed.
 *        The session is owned by the caller and should eventually be freed with AwaClientSession_Free.
 * @return Pointer to a new client session.
 * @return NULL on error.
 */
AwaClientSession * AwaClientSession_New(void);

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
AwaError AwaClientSession_SetIPCAsUDP(AwaClientSession * session, const char * address, uint16_t port);

// Not yet implemented:
//AwaError AwaClientSession_SetIPCAsLocal(AwaClientSession * session);
//AwaError AwaClientSession_SetIPCAsMQTT(AwaClientSession * session /* ... */);
//AwaError AwaClientSession_SetIPCAsUbus(AwaClientSession * session /* ... */);
//AwaError AwaClientSession_SetIPCAsNanoMsg(AwaClientSession * session /* ... */);

/**
 * @brief Set IPC connect timeout.
 *        Used internally by ::AwaClientSession_Connect and ::AwaClientSession_Disconnect.
 * @param[in] session Pointer to a valid client session.
 * @param[in] timeout The time within which an IPC session Connect must complete to avoid timeout.
 *                    It must be greater than 0.
 * @return AwaError_Success on success.
 * @return Various errors on failure.
 */
AwaError AwaClientSession_SetDefaultTimeout(AwaClientSession * session, AwaTimeout timeout);

/**
 * @brief Connect a session (configured with an IPC mechanism) to the Core.
 *        A session must be connected before operations can be processed.
 * @param[in] session Pointer to an IPC-configured session.
 * @return AwaError_Success on success.
 * @return Various errors on failure.
 */
AwaError AwaClientSession_Connect(AwaClientSession * session);

/**
 * @brief Object and Resource Definitions are assembled by the application and sent to
 *        the client daemon for storage (a process called "Define"). After successful
 *        definition, the application can shut down and the daemon will retain the definitions.
 *        Later, an application may wish to interact with these objects and resources. In
 *        order to do this in a type-safe manner, the Session must be aware of the definitions.
 *        Definitions will be held by the Session if DefineObject/DefineResource functions were
 *        used to successfully define objects and resources. If not, then the definitions must
 *        be retrieved with AwaClientSession_Refresh.
 * @param[in] session A pointer to a valid session instance.
 * @return AwaError_Success on success.
 * @return AwaError_SessionInvalid if the specified session is invalid.
 */
AwaError AwaClientSession_Refresh(AwaClientSession * session);

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
bool AwaClientSession_IsObjectDefined(const AwaClientSession * session, AwaObjectID objectID);

/**
 * @brief Retrieves an object definition instance corresponding to the object identified by the given object ID.
 *        The object definition is owned by the session and should not be freed by the caller.
 * @param[in] session Pointer to a connected session.
 * @param[in] objectID Identifies the object for which the query is targeted.
 * @return A const pointer to an object definition instance, if it exists.
 * @return NULL if a definition for the object specified does not exist.
 */
const AwaObjectDefinition * AwaClientSession_GetObjectDefinition(const AwaClientSession * session, AwaObjectID objectID);

/**
 * @brief Retrieves a new object definition iterator that can be used to iterate through the set
 *        of defined objects within the context of the given session.
 *        The iterator is owned by the caller and should eventually be freed with AwaObjectDefinitionIterator_Free.
 * @param[in] session Pointer to a connected session.
 * @return A pointer to an object definition iterator, if the session is valid.
 * @return NULL if the session is not valid.
 */
AwaObjectDefinitionIterator * AwaClientSession_NewObjectDefinitionIterator(const AwaClientSession * session);

/**
 * @brief Process any incoming requests from the LWM2M Client. Callbacks are scheduled on the session but are not invoked.
 * @param[in] session Pointer to a connected session.
 * @param[in] timeout The function will wait at least as long as this value for a response.
 * @return AwaError_Success on success.
 * @return AwaError_Timeout if no response is received after the timeout duration expires.
 * @return Various errors on failure.
 */
AwaError AwaClientSession_Process(AwaClientSession * session, AwaTimeout timeout);

/**
 * @brief Invoke any callbacks scheduled since this function was last called.
 * @param[in] session Pointer to a connected session.
 * @return AwaError_Success on success.
 * @return Various errors on failure.
 */
AwaError AwaClientSession_DispatchCallbacks(AwaClientSession * session);

/**
 * @brief When a session is no longer required, or if the application intends to sleep for some time, the session can be
 *        disconnected from the Core. This maintains object and resource definition information, but prevents the
 *        processing of operations, or the reception of subscription notifications.
 *        The session can be reconnected to the Core with AwaClientSession_Connect.
 *        If a connected session is freed, it is automatically disconnected.
 * @param[in] session Pointer to a connected session.
 * @return AwaError_Success on success.
 * @return AwaError_IPCError if the session is not configured.
 * @return AwaError_SessionNotConnected if the session is not connected.
 * @return AwaError_SessionInvalid if the session is invalid.
 */
AwaError AwaClientSession_Disconnect(AwaClientSession * session);

/**
 * @brief Shut down an existing session, freeing any allocated memory. This function
 *        should eventually be called on every session to avoid a memory leak.
 *        If the session is connected, it will automatically be disconnected first.
 * @param[in,out] session A pointer to a session pointer that will be set to NULL.
 * @return AwaError_Success on success.
 * @return AwaError_SessionInvalid if the session is not valid.
 */
AwaError AwaClientSession_Free(AwaClientSession ** session);

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
AwaError AwaClientSession_PathToIDs(const AwaClientSession * session, const char * path, AwaObjectID * objectID, AwaObjectInstanceID * objectInstanceID, AwaResourceID * resourceID);


/**************************************************************************************************
 * Object & Resource Definition Operation
 *************************************************************************************************/

/**
 * @brief Allocate and return a pointer to a new Define operation,
 *        that can be used to define Objects and Resources.
 *        The operation is owned by the caller and should eventually be freed with AwaClientDefineOperation_Free.
 *
 * Object and Resource Definitions are assembled by the application and sent to
 * the client daemon for storage (a process called "Define"). After successful
 * definition, the application can shut down and the daemon will retain the definitions.
 *
 * Later, an application may wish to interact with these objects and resources. In
 * order to do this in a type-safe manner, the Session must be aware of the definition.
 * Definitions will be held by the Session if DefineObject/DefineResource functions were
 * used to successfully define objects and resources. If not, then the definition must
 * be retrieved with SessionRefresh.
 *
 * @param[in] session A pointer to a valid session.
 * @return Pointer to a newly allocated Define operation instance.
 * @return NULL on failure.
 */
AwaClientDefineOperation * AwaClientDefineOperation_New(const AwaClientSession * session);

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
AwaError AwaClientDefineOperation_Add(AwaClientDefineOperation * operation, const AwaObjectDefinition * objectDefinition);

/*
 * @brief Process the Define operation by sending it to the Core.
 *        If successful, the object definition is also added to the session.
 * @param[in] operation The Define operation to process.
 * @param[in] timeout The function will wait at least as long as this value for a response.
 * @return AwaError_Success on success.
 * @return AwaError_Timeout if no response is received after the timeout duration expires.
 * @return Various errors on failure.
 */
AwaError AwaClientDefineOperation_Perform(AwaClientDefineOperation * operation, AwaTimeout timeout);

/**
 * @brief Clean up a Define operation, freeing all allocated resources.
 *        Once freed, the operation is no longer valid.
 * @param[in,out] operation A pointer to a Define operation pointer that will be set to NULL.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is not valid.
 */
AwaError AwaClientDefineOperation_Free(AwaClientDefineOperation ** operation);



/**************************************************************************************************
 * Object & Resource Retrieval Operation
 *************************************************************************************************/

/**
 * @brief Allocate and return a pointer to a new Get operation,
 *        that can be used to retrieve resource values from the Core.
 *        Get operations can be loaded with one or more paths of interest, corresponding to
 *        resources or trees of resources that the application wishes to retrieve. When
 *        processed, the response is made available via AwaClientGetOperation_GetResponse
 *        and will contain the resources returned by the Core in response to the query.
 *        The Get operation is owned by the caller and should eventually be freed with AwaClientGetOperation_Free.
 * @param[in] session A pointer to a valid session.
 * @return Pointer to a newly allocated Get operation instance.
 * @return NULL on failure.
 */
AwaClientGetOperation * AwaClientGetOperation_New(const AwaClientSession * session);

/**
 * @brief Adds a path of interest to a Get operation, as a request to retrieve all resources at
 *        or covered by this path from the Core.
 *        Adding an object path will retrieve all resources held by all object instances of this object.
 *        Adding an object instance path will retrieve all resources held by this object instance.
 *        Adding a resource path will retrieve that specific resource.
 *        Multiple-instance resources are retrieved in their entirety (the entire array is retrieved),
 *        however array ranges can be retrieved with AwaClientGetOperation_AddPathWithArrayRange.
 *        Adding a path that does not correspond to any resources in the Core will result in the
 *        subsequent GetResponse lacking resources for that path.
 * @param[in] operation The Get operation to add the path of interest to.
 * @param[in] path The path of the resource, object instance or object requested for retrieval.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_PathInvalid if the path specified is not a valid data model path.
 */
AwaError AwaClientGetOperation_AddPath(AwaClientGetOperation * operation, const char * path);

/**
 * @brief Add a resource path to a multiple-instance resource and specify a range of resource instances to retrieve.
 *        The path must correspond to a resource path; object and object instance paths are not permitted.
 *        Only valid instances in this range are retrieved - unpopulated instances are ignored - so the number
 *        of instances in the GetResponse may be less than the width of the range.
 * @param[in] operation The Get operation to add the multiple-instance resource path to.
 * @param[in] path The path of the multiple-instance resource to retrieve.
 * @param[in] startIndex Array index to begin retrieval from.
 * @param[in] indexCount Number of array indices to retrieve, including unpopulated indices.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_PathInvalid if the path specified is not a valid data model path.
 * @return AwaError_AddInvalid if the path specified is not suitable for this function.
 */
AwaError AwaClientGetOperation_AddPathWithArrayRange(AwaClientGetOperation * operation, const char * path, AwaArrayIndex startIndex, AwaArrayLength indexCount);

/**
 * @brief Process the Get operation by sending it to the Core.
 *        If successful, the response can be obtained with AwaClientGetOperation_GetResponse
 *        and queried for retrieved resource values.
 * @param[in] operation The Get operation to process.
 * @param[in] timeout The function will wait at least as long as this value for a response.
 * @return AwaError_Success on success.
 * @return AwaError_Timeout if no response is received after the timeout duration expires.
 * @return Various errors on failure.
 */
AwaError AwaClientGetOperation_Perform(AwaClientGetOperation * operation, AwaTimeout timeout);

/**
 * @brief Clean up a Get operation, freeing all allocated resources.
 *        Once freed, the operation is no longer valid.
 *        Note: freeing a Get operation will invalidate all derived Get Response instances,
 *        and any Opaque or CString pointers retrieved from an associated Get Response.
 * @param[in,out] operation A pointer to a Get operation pointer that will be set to NULL.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is not valid.
 */
AwaError AwaClientGetOperation_Free(AwaClientGetOperation ** operation);

/**
 * @brief Obtain a Get Response instance from a processed Get operation. This may be
 *        queried directly by path to obtain the resource values returned by the Core.
 *        A Path iterator can be used to obtain a list of all resource paths provided by
 *        the Get Response.
 *        The Get Response is owned by the Get operation and should not be freed by the caller.
 * @param[in] operation The processed Get operation to obtain the Get response from.
 * @return Pointer to AwaClientGetResponse if operation is valid and has a valid response.
 * @return NULL if the operation is invalid, or has no valid response (may not have been processed).
 */
const AwaClientGetResponse * AwaClientGetOperation_GetResponse(const AwaClientGetOperation * operation);

/**
 * @brief Create a new Path Iterator for a Get Response, used to iterate through the list of resource paths
 *        retrieved by the corresponding Get operation.
 *        The resulting iterator is owned by the caller and should eventually be freed with AwaPathIterator_Free.
 *        This function can only be successful after a Get operation has been successfully processed.
 * @param[in] response A pointer to the Get Response to search.
 * @return A pointer to a new PathIterator instance on success.
 * @return NULL on failure.
 */
AwaPathIterator * AwaClientGetResponse_NewPathIterator(const AwaClientGetResponse * response);

/**
 * @brief Test if the Get Response has the specified path (resource, object instance or object) present.
 *        If the Get Response contains paths beyond the specified path, the path will be considered present.
 *        This function can only be successful after a Get operation has been successfully processed.
 * @param[in] response A pointer to a valid Get Response.
 * @param[in] path The path with which to query the Get Response.
 * @return True if the Get Response contains the specified resource path.
 * @return False if the Get Response does not contain the specified resource path, or if the path is invalid.
 */
bool AwaClientGetResponse_ContainsPath(const AwaClientGetResponse * response, const char * path);

/**
 * @brief Retrieve a path result from a Get Response relating to a write to that path
 *        retrieved by the corresponding Get Operation. Querying the path result will allow
 *        the caller to determine the status of a request to define objects and resources.
 * @param[in] response A pointer to the Get Response to search.
 * @param[in] path A path to an entity that was written to.
 * @return NULL if no definition exists in the Get response for the specified path,
 *         or if the response, path or result is NULL.
 * @return Pointer to PathResult instance. Do not free.
 */
const AwaPathResult * AwaClientGetResponse_GetPathResult(const AwaClientGetResponse * response, const char * path);

/**
 * @brief Test if the Get Response has a value for the specified resource path.
 *        If the Get Response contains a value for the specified path, and the Resource Type is known,
 *        it can be retrieved with the appropriate AwaClientGetResponse_GetValueAs_ function.
 *        This function can only be successful after a Get operation has been successfully processed.
 * @param[in] response A pointer to a valid Get Response.
 * @param[in] path The path with which to query the Get Response.
 * @return True if the Get Response contains a value for the specified resource path.
 * @return False if the Get Response does not contain a value for the specified resource path, or if the path is invalid.
 */
bool AwaClientGetResponse_HasValue(const AwaClientGetResponse * response, const char * path);

/**
 * @defgroup GetResponse_GetValueAs AwaClientGetResponse_GetValueAs___Pointer
 * @addtogroup GetResponse_GetValueAs
 * @brief Retrieve a temporary pointer to a resource's value from a Get Response. The resource is identified by the path.
 *        This function can only be successful after a Get operation has been successfully processed.
 *        The pointer is only valid while the operation is valid. If the operation is freed, the
 *        pointer returned is immediately invalid and should not be used or dereferenced.
 * @param[in] response The current Get Response to retrieve the value from.
 * @param[in] path The path of the resource requested for retrieval.
 * @param[in,out] value A pointer to a const pointer that will be modified to point to the requested value. Set to null on error.
 * @return AwaError_Success on success.
 * @return AwaError_TypeMismatch if the resource type corresponding to the specified path is not of the correct type.
 * @return AwaError_PathNotFound if the specified resource path is not covered by the Get Response.
 * @return AwaError_OperationInvalid if the specified operation is invalid or NULL.
 * @return AwaError_PathInvalid if the specified path is invalid or does not correspond to a resource path.
 * @{
 */
AwaError AwaClientGetResponse_GetValueAsCStringPointer   (const AwaClientGetResponse * response, const char * path, const char ** value);
AwaError AwaClientGetResponse_GetValueAsIntegerPointer   (const AwaClientGetResponse * response, const char * path, const AwaInteger ** value);
AwaError AwaClientGetResponse_GetValueAsFloatPointer     (const AwaClientGetResponse * response, const char * path, const AwaFloat ** value);
AwaError AwaClientGetResponse_GetValueAsBooleanPointer   (const AwaClientGetResponse * response, const char * path, const AwaBoolean ** value);
AwaError AwaClientGetResponse_GetValueAsTimePointer      (const AwaClientGetResponse * response, const char * path, const AwaTime ** value);
AwaError AwaClientGetResponse_GetValueAsObjectLinkPointer(const AwaClientGetResponse * response, const char * path, const AwaObjectLink ** value);
AwaError AwaClientGetResponse_GetValueAsOpaquePointer    (const AwaClientGetResponse * response, const char * path, const AwaOpaque ** value);
/** @} */

/**
 * @brief Retrieve an opaque resource's value from a Get Response. The resource is identified by the path.
 *        This function can only be successful after a Get operation has been successfully processed.
 *        A pointer to a AwaOpaque struct is passed in and populated by the function.
 *        The data pointer within the AwaOpaque struct is only valid as long as the Get operation remains valid.
 *        If the operation is freed, the data pointer is immediately invalid and should not be used or dereferenced.
 * @param[in] response The current Get Response to retrieve the value from.
 * @param[in] path The path of the opaque resource requested for retrieval.
 * @param[in,out] value A pointer to a AwaOpaque struct that will be modified to refer to the requested opaque value.
 * @return AwaError_Success on success.
 * @return AwaError_TypeMismatch if the resource type corresponding to the specified path is not of type AwaResourceType_Opaque.
 * @return AwaError_PathNotFound if the specified resource path is not covered by the Get Response.
 * @return AwaError_OperationInvalid if the specified operation is invalid or NULL.
 * @return AwaError_PathInvalid if the specified path is invalid or does not correspond to a resource path.
 */
AwaError AwaClientGetResponse_GetValueAsOpaque(const AwaClientGetResponse * response, const char * path, AwaOpaque * value);

/**
 * @brief Retrieve an object link resource's value from a Get Response. The resource is identified by the path.
 *        This function can only be successful after a Get operation has been successfully processed.
 *        A pointer to a AwaObjectLink struct is passed in and populated by the function.
 * @param[in] response The current Get Response to retrieve the value from.
 * @param[in] path The path of the object link resource requested for retrieval.
 * @param[in,out] value A pointer to a AwaObjectLink struct that will be modified to hold the requested object link value.
 * @return AwaError_Success on success.
 * @return AwaError_TypeMismatch if the resource type corresponding to the specified path is not of type AwaResourceType_ObjectLink.
 * @return AwaError_PathNotFound if the specified resource path is not covered by the Get Response.
 * @return AwaError_OperationInvalid if the specified operation is invalid or NULL.
 * @return AwaError_PathInvalid if the specified path is invalid or does not correspond to a resource path.
 */
AwaError AwaClientGetResponse_GetValueAsObjectLink(const AwaClientGetResponse * response, const char * path, AwaObjectLink * value);

/**
 * @defgroup GetResponse_GetValuesAsArray AwaClientGetResponse_GetValuesAs___ArrayPointer
 * @addtogroup GetResponse_GetValuesAsArray
 * @brief Retrieve a temporary pointer to a multiple-instance resource's array value from a Get Response. The resource is identified by the path.
 *        This function can only be successful after a Get operation has been successfully processed.
 *        The pointer is only valid while the operation is valid. If the operation is freed, the
 *        pointer returned is immediately invalid and should not be used or dereferenced.
 * @param[in] response The current Get Response to retrieve the value from.
 * @param[in] path The path of the multiple-instance resource requested for retrieval.
 * @param[in,out] valueArray A pointer to a const pointer that will be modified to point to the requested value. Set to null on error.
 * @return AwaError_Success on success.
 * @return AwaError_TypeMismatch if the resource type corresponding to the specified path is not of the correct type.
 * @return AwaError_PathNotFound if the specified resource path is not covered by the Get Response.
 * @return AwaError_OperationInvalid if the specified operation is invalid or NULL.
 * @return AwaError_PathInvalid if the specified path is invalid or does not correspond to a resource path.
 * @{
 */
AwaError AwaClientGetResponse_GetValuesAsStringArrayPointer    (const AwaClientGetResponse * response, const char * path, const AwaStringArray ** valueArray);
AwaError AwaClientGetResponse_GetValuesAsIntegerArrayPointer   (const AwaClientGetResponse * response, const char * path, const AwaIntegerArray ** valueArray);
AwaError AwaClientGetResponse_GetValuesAsFloatArrayPointer     (const AwaClientGetResponse * response, const char * path, const AwaFloatArray ** valueArray);
AwaError AwaClientGetResponse_GetValuesAsBooleanArrayPointer   (const AwaClientGetResponse * response, const char * path, const AwaBooleanArray ** valueArray);
AwaError AwaClientGetResponse_GetValuesAsTimeArrayPointer      (const AwaClientGetResponse * response, const char * path, const AwaTimeArray ** valueArray);
AwaError AwaClientGetResponse_GetValuesAsOpaqueArrayPointer    (const AwaClientGetResponse * response, const char * path, const AwaOpaqueArray ** valueArray);
AwaError AwaClientGetResponse_GetValuesAsObjectLinkArrayPointer(const AwaClientGetResponse * response, const char * path, const AwaObjectLinkArray ** valueArray);
/** @} */


/**************************************************************************************************
 * Object & Resource Value Setting Operation
 *************************************************************************************************/

/**
 * @brief Allocate and return a pointer to a new Set Operation, that can be used to set resource
 *        values.
 *        Optional resources can be created, taking the default value or set to a new value.
 *        New object instances can be created, automatically creating all covered mandatory resources.
 *        Set Operations can be loaded with one or more resource paths with corresponding value.
 *        The Set Operation is owned by the caller and should eventually be freed with AwaClientSetOperation_Free.
 * @param[in] session A pointer to a valid session.
 * @return A pointer to a newly allocated Set Operation instance.
 * @return NULL on failure.
 */
AwaClientSetOperation * AwaClientSetOperation_New(const AwaClientSession * session);

/**
 * @brief Adds an object or object instance path to a Set Operation, as a request to create an Object Instance.
 *        The target object must support creation of a new object instance, such as supporting multiple instances,
 *        or an optional instance that does not exist.
 *        When the operation is processed, if the object instance ID is specified in the path, the object instance
 *        requested will be created (or an error returned if it already exists).
 *        If the object instance ID is not specified in the path, the ID will be automatically assigned by the Core
 *        if the instance can be created.
 * @param[in] operation The Set Operation to add the path of interest to.
 * @param[in] path The path of the object or object instance requested for object instance creation.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_PathInvalid if the specified path is invalid.
 */
AwaError AwaClientSetOperation_CreateObjectInstance(AwaClientSetOperation * operation, const char * path);

/**
 * @brief Adds a resource path to a Set Operation, as a request to create an optional Resource.
 *        When the operation is processed, if the path refers to an optional resource that does not exist, the resource will be created.
 *        The resource will have the default value assigned, if not set to another value in the same operation.
 * @param[in] operation The Set Operation to add the path of interest to.
 * @param[in] path The path of the resource requested for optional resource creation.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_PathInvalid if the specified path is invalid.
 */
AwaError AwaClientSetOperation_CreateOptionalResource(AwaClientSetOperation * operation, const char * path);

/**
 * @defgroup SetOperation_AddValueAs AwaClientSetOperation_AddValueAs___
 * @addtogroup SetOperation_AddValueAs
 * @brief Adds a path and value to a Set Operation, as a request to change the resource to the specified value.
 * @param[in] operation The Set Operation to add the path and value to.
 * @param[in] path The path of the resource requested for change.
 * @param[in] value The new value of the resource.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_PathInvalid if the specified path is invalid.
 * @return AwaError_TypeMismatch if the specified path refers to a resource with a non-corresponding type.
 * @return AwaError_NotDefined if the specified path refers to an object or resource that is not defined.
 * @{
 */
AwaError AwaClientSetOperation_AddValueAsCString   (AwaClientSetOperation * operation, const char * path, const char * value);
AwaError AwaClientSetOperation_AddValueAsInteger   (AwaClientSetOperation * operation, const char * path, AwaInteger value);
AwaError AwaClientSetOperation_AddValueAsFloat     (AwaClientSetOperation * operation, const char * path, AwaFloat value);
AwaError AwaClientSetOperation_AddValueAsBoolean   (AwaClientSetOperation * operation, const char * path, AwaBoolean value);
AwaError AwaClientSetOperation_AddValueAsTime      (AwaClientSetOperation * operation, const char * path, AwaTime value);
AwaError AwaClientSetOperation_AddValueAsOpaque    (AwaClientSetOperation * operation, const char * path, AwaOpaque value);
AwaError AwaClientSetOperation_AddValueAsObjectLink(AwaClientSetOperation * operation, const char * path, AwaObjectLink value);
/** @} */

/**
 * @defgroup SetOperation_AddValueAsArray AwaClientSetOperation_AddValueAs___Array
 * @addtogroup SetOperation_AddValueAsArray
 * @brief Adds a path and array of values to a Set Operation, as a request to change a multiple-instance resource to the specified value.
 *        The array can be sparse and only contain entries for modification. Existing resource instances that are not replaced remain unmodified.
 * @param[in] operation The Set Operation to add the path and value to.
 * @param[in] path The path of the resource requested for change.
 * @param[in] array The new value of the resource, given as a sparse array of resource instances.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_PathInvalid if the specified path is invalid.
 * @return AwaError_TypeMismatch if the specified path refers to a resource with a non-corresponding type.
 * @return AwaError_NotDefined if the specified path refers to an object or resource that is not defined.
 * @{
 */
AwaError AwaClientSetOperation_AddValueAsStringArray    (AwaClientSetOperation * operation, const char * path, const AwaStringArray * array);
AwaError AwaClientSetOperation_AddValueAsIntegerArray   (AwaClientSetOperation * operation, const char * path, const AwaIntegerArray * array);
AwaError AwaClientSetOperation_AddValueAsFloatArray     (AwaClientSetOperation * operation, const char * path, const AwaFloatArray * array);
AwaError AwaClientSetOperation_AddValueAsBooleanArray   (AwaClientSetOperation * operation, const char * path, const AwaBooleanArray * array);
AwaError AwaClientSetOperation_AddValueAsTimeArray      (AwaClientSetOperation * operation, const char * path, const AwaTimeArray * array);
AwaError AwaClientSetOperation_AddValueAsOpaqueArray    (AwaClientSetOperation * operation, const char * path, const AwaOpaqueArray * array);
AwaError AwaClientSetOperation_AddValueAsObjectLinkArray(AwaClientSetOperation * operation, const char * path, const AwaObjectLinkArray * array);
/** @} */

/**
 * @defgroup SetOperation_AddArrayValueAs AwaClientSetOperation_AddArrayValueAs___
 * @addtogroup SetOperation_AddArrayValueAs
 * @brief Adds a path and value to a Set Operation, as a request to change a multiple-instance resource's instance to the specified value.
 *        This allows a Set Operation to modify a single instance of a multiple-instance resource, leaving all other instances unmodified.
 *        Only values with the same type as the array resource specified by the path can be added.
 * @param[in] operation The Set Operation to add the path and value to.
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
AwaError AwaClientSetOperation_AddArrayValueAsCString   (AwaClientSetOperation * operation, const char * path, int resourceInstanceID, const char * value);
AwaError AwaClientSetOperation_AddArrayValueAsInteger   (AwaClientSetOperation * operation, const char * path, int resourceInstanceID, AwaInteger value);
AwaError AwaClientSetOperation_AddArrayValueAsFloat     (AwaClientSetOperation * operation, const char * path, int resourceInstanceID, AwaFloat value);
AwaError AwaClientSetOperation_AddArrayValueAsBoolean   (AwaClientSetOperation * operation, const char * path, int resourceInstanceID, AwaBoolean value);
AwaError AwaClientSetOperation_AddArrayValueAsTime      (AwaClientSetOperation * operation, const char * path, int resourceInstanceID, AwaTime value);
AwaError AwaClientSetOperation_AddArrayValueAsOpaque    (AwaClientSetOperation * operation, const char * path, int resourceInstanceID, AwaOpaque value);
AwaError AwaClientSetOperation_AddArrayValueAsObjectLink(AwaClientSetOperation * operation, const char * path, int resourceInstanceID, AwaObjectLink value);
/** @} */

/**
 * @brief Process the Set Operation by sending it to the Core.
 * @param[in] operation The Set Operation to process.
 * @param[in] timeout The function will wait at least as long as this value for a response.
 * @return AwaError_Success on success.
 * @return AwaError_Timeout if no response is received after the timeout duration expires.
 * @return Various errors on failure.
 */
AwaError AwaClientSetOperation_Perform(AwaClientSetOperation * operation, AwaTimeout timeout);

/**
 * @brief Obtain a Set Response instance from a processed Set Operation. This may be
 *        iterated through to determine whether the set operation succeeded for the requested paths.
 *        The Set Response is owned by the Set Operation and should not be freed by the caller.
 * @param[in] operation The processed Set Operation to obtain the Set response from.
 * @return Pointer to AwaClientSetResponse if operation is valid and has a valid response.
 * @return NULL if the operation is invalid, or has no valid response (may not have been processed).
 */
const AwaClientSetResponse * AwaClientSetOperation_GetResponse(const AwaClientSetOperation * operation);

/**
 * @brief Clean up a Set Operation, freeing all allocated resources.
 *        Once freed, the operation is no longer valid.
 * @param[in,out] operation A pointer to a Set Operation pointer that will be set to NULL.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is not valid.
 */
AwaError AwaClientSetOperation_Free(AwaClientSetOperation ** operation);

/**
 * @brief Create a new Path Iterator for a Set Response, used to iterate through the list of paths
 *        returned in the response of the corresponding Set Operation.
 *        The resulting iterator is owned by the caller and should eventually be freed with AwaPathIterator_Free.
 *        This function can only be successful after a Set Operation has been successfully processed.
 * @param[in] response A pointer to the Set Response to search.
 * @return A pointer to a new PathIterator instance on success.
 * @return NULL on failure.
 */
AwaPathIterator * AwaClientSetResponse_NewPathIterator(const AwaClientSetResponse * response);

/**
 * @brief Retrieve a path result from a Set Response relating to a write to that path
 *        retrieved by the corresponding Set Operation. Querying the path result will allow
 *        the caller to determine the status of a request to define objects and resources.
 * @param[in] response A pointer to the Set Response to search.
 * @param[in] path A path to an entity that was written to.
 * @return NULL if no definition exists in the Set response for the specified path,
 *         or if the response, path or result is NULL.
 * @return Pointer to PathResult instance. Do not free.
 */
const AwaPathResult * AwaClientSetResponse_GetPathResult(const AwaClientSetResponse * response, const char * path);

/**
 * @brief Test if the Set Response has the specified path to a resource or object present.
 *        If the Set Response contains paths beyond the specified path, the path will be considered present.
 * @param[in] response A pointer to a valid Set Response.
 * @param[in] path The path with which to query the Set Response.
 * @return True if the Set Response contains the specified resource path.
 * @return False if the Set Response does not contain the specified resource path, or if the path is invalid.
 */
bool AwaClientSetResponse_ContainsPath(const AwaClientSetResponse * response, const char * path);


/**************************************************************************************************
 * Object Instance & Resource Deletion Operation
 *************************************************************************************************/

/**
 * @brief Allocate and return a pointer to a new Delete operation,
 *        that can be used to delete optional resources, resource instances, and object instances.
 * @param[in] session A pointer to a valid session instance.
 * @return A pointer to a newly allocated Delete operation instance, or NULL on failure.
 */
AwaClientDeleteOperation * AwaClientDeleteOperation_New(const AwaClientSession * session);

/**
 * @brief Adds a path to a Delete operation, as a request to delete the specified resource or object instance.
 * @param[in] operation A pointer to a valid delete operation.
 * @param[in] path The path of the resource, resource instance or object instance requested to delete.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_PathInvalid if the specified path is invalid.
 * @return AwaError_NotDefined if the specified path refers to an object or resource that is not defined.
 */
AwaError AwaClientDeleteOperation_AddPath(AwaClientDeleteOperation * operation, const char * path);

/**
 * @brief Add a path to a multiple-instance resource and specify a range of resource instances to delete.
 * @param[in] operation The Delete operation to add a delete request to.
 * @param[in] path The path of the resource to delete.
 * @param[in] startIndex Array index to begin deletion from.
 * @param[in] indexCount Number of array indices to delete, including missing items.
 * @return AwaError_Success on success.
 * @return AwaError_AddInvalid if startIndex is negative, indexCount is less than one or greater than AWA_MAX_ID
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_PathInvalid if the specified path is invalid.
 * @return AwaError_NotDefined if the specified path refers to an object or resource that is not defined.
 */
AwaError AwaClientDeleteOperation_AddPathWithArrayRange(AwaClientDeleteOperation * operation, const char * path, AwaArrayIndex startIndex, AwaArrayLength indexCount);

/**
 * @brief Process the Delete operation by sending it to the Core.
 * @param[in] operation The Delete operation to process.
 * @param[in] timeout The function will wait at least as long as this value for a response.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_Timeout if no response is received after the timeout duration expires.
 * @return Various errors on failure.
 */
AwaError AwaClientDeleteOperation_Perform(AwaClientDeleteOperation * operation, AwaTimeout timeout);

/**
 * @brief Obtain a Delete Response instance from a processed Delete operation. This may be
 *        iterated through to determine whether the delete operation succeeded for the requested paths.
 *        The Delete Response is owned by the Delete operation and should not be freed by the caller.
 * @param[in] operation The processed Delete operation to obtain the Delete response from.
 * @return Pointer to AwaClientDeleteResponse if operation is valid and has a valid response.
 * @return NULL if the operation is invalid, or has no valid response (may not have been processed).
 */
const AwaClientDeleteResponse * AwaClientDeleteOperation_GetResponse(const AwaClientDeleteOperation * operation);

/**
 * @brief Shut down an existing operation, freeing any allocated memory. This function
 *        should eventually be called on every operation to avoid a memory leak.
 *        Any pointers from GetValueAs*Pointer() functions will be invalid after this call.
 * @param[in,out] operation A pointer to a operation pointer, which will be set to NULL by this function.
 * @return AwaError_Success on success
 * @return Error_OperationInvalid if the operation is not valid.
 */
AwaError AwaClientDeleteOperation_Free(AwaClientDeleteOperation ** operation);

/**
 * @brief Create a new Path Iterator for a Delete Response, used to iterate through the list of resource paths
 *        retrieved by the corresponding Delete operation.
 *        The resulting iterator is owned by the caller and should eventually be freed with AwaPathIterator_Free.
 *        This function can only be successful after a Delete operation has been successfully processed.
 * @param[in] response A pointer to the Delete Response to search.
 * @return A pointer to a new PathIterator instance on success.
 * @return NULL on failure.
 */
AwaPathIterator * AwaClientDeleteResponse_NewPathIterator(const AwaClientDeleteResponse * response);

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
const AwaPathResult * AwaClientDeleteResponse_GetPathResult(const AwaClientDeleteResponse * response, const char * path);

/**
 * @brief Test if the Delete Response has the specified path (object instance or object) present.
 *        If the Delete Response contains paths beyond the specified path, the path will be considered present.
 *        This function can only be successful after a Delete operation has been successfully processed.
 * @param[in] response A pointer to a valid Delete Response.
 * @param[in] path The path with which to query the Delete Response.
 * @return True if the Delete Response contains the specified resource path.
 * @return False if the Delete Response does not contain the specified resource path, or if the path is invalid.
 */
bool AwaClientDeleteResponse_ContainsPath(const AwaClientDeleteResponse * response, const char * path);


/**************************************************************************************************
 * Subscription Procedures
 *************************************************************************************************/

/**
 * @brief Allocate and return a pointer to a new Subscribe operation,
 *        that can be used to subscribe to executions of resources and changes to objects,
 *        object instances and resource values from the Core.
 *        Subscribe operations can be loaded with one or more paths of interest, corresponding to
 *        resources or trees of resources that the application wishes to subscribe to. When
 *        processed, the response is made available via AwaClientSubscribeOperation_GetResponse
 *        and will contain result information for each of the paths returned by the Core in response to the query.
 *        The Subscribe operation is owned by the caller and should eventually be freed with AwaClientSubscribeOperation_Free.
 * @param[in] session A pointer to a valid session.
 * @return Pointer to a newly allocated Subscribe operation instance.
 * @return NULL on failure.
 */
AwaClientSubscribeOperation * AwaClientSubscribeOperation_New(const AwaClientSession * session);

/**
 * @brief Adds a Change Subscription of interest to a Subscribe operation, as a request to be notified of
 *        changes to the subscribed entity from the Core.
 * @param[in] operation The Subscribe operation to add the path of interest to.
 * @param[in] subscription A valid Change subscription to an entity of interest.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_SubscriptionInvalid if the subscription is invalid.
 */
AwaError AwaClientSubscribeOperation_AddChangeSubscription(AwaClientSubscribeOperation * operation, AwaClientChangeSubscription * subscription);

/**
 * @brief Adds a Execute Subscription of interest to a Subscribe operation, as a request to be notified of
 *        executions of the subscribed entity from the Core.
 * @param[in] operation The Subscribe operation to add the path of interest to.
 * @param[in] subscription A valid Execute subscription to an entity of interest.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_SubscriptionInvalid if the subscription is invalid.
 */
AwaError AwaClientSubscribeOperation_AddExecuteSubscription(AwaClientSubscribeOperation * operation, AwaClientExecuteSubscription * subscription);

/**
 * @brief Adds a Cancel flag to a Change Subscription in a specified Subscribe operation, as a request to cancel
 *        being notified of changes to the subscribed entity from the Core.
 * @param[in] operation The Subscribe operation to add the path of interest to.
 * @param[in] subscription A valid Change subscription to an entity of interest.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_SubscriptionInvalid if the subscription is invalid.
 */
AwaError AwaClientSubscribeOperation_AddCancelChangeSubscription(AwaClientSubscribeOperation * operation, AwaClientChangeSubscription * subscription);

/**
 * @brief Adds a Cancel flag to a Execute Subscription in a specified Subscribe operation, as a request to cancel
 *        being notified of executions of the subscribed entity from the Core.
 * @param[in] operation The Subscribe operation to add the path of interest to.
 * @param[in] subscription A valid Execute subscription to an entity of interest.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is invalid.
 * @return AwaError_SubscriptionInvalid if the subscription is invalid.
 */
AwaError AwaClientSubscribeOperation_AddCancelExecuteSubscription(AwaClientSubscribeOperation * operation, AwaClientExecuteSubscription * subscription);

/**
 * @brief Process the Subscribe operation by sending it to the Core, notifying the Core to
 *        begin or cancel subscription to changes and executions of the subscribed entities inside
 *        the specified Subscribe operation.
 *        If successful, the response can be obtained with AwaClientSubscribeOperation_GetResponse
 *        and queried for path results, each of which relate to the result of a single request to begin or cancel a subscription.
 * @param[in] operation The Subscribe operation to process.
 * @param[in] timeout The function will wait at least as long as this value for a response.
 * @return AwaError_Success on success.
 * @return AwaError_Timeout if no response is received after the timeout duration expires.
 * @return Various errors on failure.
 */
AwaError AwaClientSubscribeOperation_Perform(AwaClientSubscribeOperation * operation, AwaTimeout timeout);

/**
 * @brief Obtain a Subscribe Response instance from a processed Subscribe operation. This may be
 *        queried directly by path to obtain results of a subscription request returned by the Core.
 *        A Path iterator can be used to obtain a list of all subscription paths provided by
 *        the Subscribe Response.
 *        The Subscribe Response is owned by the Subscribe operation and should not be freed by the caller.
 * @param[in] operation The processed Subscribe operation to obtain the Subscribe response from.
 * @return Pointer to AwaClientSubscribeResponse if operation is valid and has a valid response.
 * @return NULL if the operation is invalid, or has no valid response (may not have been processed).
 */
const AwaClientSubscribeResponse * AwaClientSubscribeOperation_GetResponse(const AwaClientSubscribeOperation * operation);

/**
 * @brief Clean up a Subscribe operation, freeing all allocated resources.
 *        Once freed, the operation is no longer valid.
 *        Note: freeing a Subscribe operation will invalidate all derived Subscribe Response instances.
 * @param[in,out] operation A pointer to a Subscribe operation pointer that will be set to NULL.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the operation is not valid.
 */
AwaError AwaClientSubscribeOperation_Free(AwaClientSubscribeOperation ** operation); // caller must keep subscription valid during entire observe operation

/**
 * @brief Create a new Path Iterator for a Subscribe Response, used to iterate through the list of paths
 *        relating to results of subscriptions retrieved by the corresponding Subscribe operation.
 *        The resulting iterator is owned by the caller and should eventually be freed with AwaPathIterator_Free.
 *        This function can only be successful after a Subscribe operation has been successfully processed.
 * @param[in] response A pointer to the Subscribe Response to search.
 * @return A pointer to a new PathIterator instance on success.
 * @return NULL on failure.
 */
AwaPathIterator * AwaClientSubscribeResponse_NewPathIterator(const AwaClientSubscribeResponse * response);

/**
 * @brief Retrieve a path result from a Subscribe Response relating to a subscription to that path
 *        retrieved by the corresponding Subscribe operation. Querying the path result will allow
 *        the caller to determine the status of a request to begin or cancel a subscription to an entity.
 * @param[in] response A pointer to the Subscribe Response to search.
 * @param[in] path A path to an entity that was subscribed to.
 * @return NULL if no subscription exists in the subscribe response for the specified path, or if the response, path or result is NULL.
 * @return Pointer to PathResult instance. Do not free.
 */
const AwaPathResult * AwaClientSubscribeResponse_GetPathResult(const AwaClientSubscribeResponse * response, const char * path);

/**
 * @brief Allocate and return a pointer to a new Change Subscription, that can be used to subscribe
 *        to changes on either a resource, object instance or object. The specified callback function will be fired on
 *        AwaClientSession_DispatchCallbacks if the subscribed entity has changed since the session
 *        callbacks were last dispatched. The Change Subscription will not be active until it is added to
 *        a subscribe operation and performed. Operations can be created within a callback, but they cannot be performed.
 *        The Change Subscription is owned by the caller and should eventually be freed with
 *        AwaClientChangeSubscription_Free.
 * @param[in] path The path of the resource, object instance or object to subscribe to.
 * @param[in] callback Pointer to function to call when the entity specified by path is created, changed or deleted.
 * @param[in] context A pointer to a user specified object for use in the change callback
 * @return A pointer to a newly allocated Change Subscription instance.
 * @return NULL on failure.
 */
AwaClientChangeSubscription * AwaClientChangeSubscription_New(const char * path, AwaClientSubscribeToChangeCallback callback, void * context);

/**
 * @brief Retrieve the path from a Change subscription.
 * @param[in] subscription A valid Change subscription.
 * @return The subscription's path if the subscription is valid.
 * @return NULL if the subscription is not valid.
 */
const char * AwaClientChangeSubscription_GetPath(AwaClientChangeSubscription * subscription);

/**
 * @brief Shut down a Change Subscription, freeing any allocated memory. This function
 *        should eventually be called on every operation to avoid a memory leak.
 * @param[in,out] subscription A pointer to a Change Subscription pointer, which will be set to NULL by this function.
 * @return AwaError_Success on success
 * @return AwaError_SubscriptionInvalid if the operation is not valid.
 */
AwaError AwaClientChangeSubscription_Free(AwaClientChangeSubscription ** subscription);

/**
 * @brief Allocate and return a pointer to a new Execute Subscription, that can be used to subscribe
 *        to executions of a resource, or all resources of an object instance or object.
 *        The specified callback function will be fired on AwaClientSession_DispatchCallbacks if
 *        the subscribed entity or its children have been executed since the session callbacks were last dispatched.
 *        The Execute Subscription will not be active until it is added to a subscribe operation and performed.
 *        The Execute Subscription is owned by the caller and should eventually be freed with
 *        AwaClientExecuteSubscription_Free.
 * @param[in] path The path of the resource, object instance or object to subscribe to.
 * @param[in] callback Pointer to function to call when the entity specified by path or its children are executed.
 * @param[in] context A pointer to a user specified object for use in the change callback
 * @return A pointer to a newly allocated Change Subscription instance.
 * @return NULL on failure.
 */
AwaClientExecuteSubscription * AwaClientExecuteSubscription_New(const char * path, AwaClientSubscribeToExecuteCallback callback, void * context);

/**
 * @brief Retrieve the path from a Execute subscription.
 * @param[in] subscription A valid Execute subscription.
 * @return The subscription's path if the subscription is valid.
 * @return NULL if the subscription is not valid.
 */
const char * AwaClientExecuteSubscription_GetPath(AwaClientExecuteSubscription * subscription);

/**
 * @brief Shut down an Execute Subscription, freeing any allocated memory. This function
 *        should eventually be called on every operation to avoid a memory leak.
 * @param[in,out] subscription A pointer to an Execute Subscription pointer, which will be set to NULL by this function.
 * @return AwaError_Success on success
 * @return Error_OperationInvalid if the operation is not valid.
 */
AwaError AwaClientExecuteSubscription_Free(AwaClientExecuteSubscription ** subscription);


/**************************************************************************************************
 * ChangeSet Functions for Change Subscriptions
 *************************************************************************************************/

/**
 * @brief Retrieve the corresponding session for a ChangeSet.
 *        This function is only valid for a ChangeSet connected to a client session.
 * @param[in] changeSet A pointer to a valid ChangeSet.
 * @return The session for the given ChangeSet.
 * @return NULL if the ChangeSet is invalid.
 */
const AwaClientSession * AwaChangeSet_GetClientSession(const AwaChangeSet * changeSet);

#ifdef __cplusplus
}
#endif

#endif // AWA_CLIENT_H

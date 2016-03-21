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


/*
 * This interface enables an application to be statically embedded within the LWM2M Client Daemon and
 * interact with its resources.
 *
 * The Core hosts resources within a data model based on Objects, Object Instances and Resources.
 * Please consult the LWM2M specification for details of this model.
 *
 * A Management Application may interact with the Client vian LWM2M, accessing the same resources. Therefore
 * it is essential that both the Client Application and Management Application are aware of the same data
 * model.
 *
 * It is recommended that registered IDs are used for objects that conform to registered LWM2M objects such
 * as IPSO objects.
 */

#ifndef AWA_STATIC_H
#define AWA_STATIC_H

// @cond
#include <stdint.h>
#include <stdbool.h>
// @endcond

#include "types.h"
#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
 * Example Usage
 *************************************************************************************************/

/**
* @example static-client-tutorial1.c
*        Initialise a static client.
* @example static-client-tutorial2.c
*        Register an object and resources with pointers.
* @example static-client-tutorial3.c
*        Register an object and resources with a custom handler.
*/


/**************************************************************************************************
 * Public Types and Enumerations
 *************************************************************************************************/

/**
 * Creates a locally scoped Opaque struct containing an array of Data
 * of the given size.
 */
#define AWA_OPAQUE(name, size)     \
  struct name##_t {                \
      size_t Size;                 \
      uint8_t Data[size];          \
  } name

/**
 * Supported operations for resource and object handlers
 * registered with ::AwaStaticClient_RegisterResourceWithHandler.
 */
typedef enum
{
    AwaOperation_CreateObjectInstance,  /**< indicates a request to create an object instance */
    AwaOperation_DeleteObjectInstance,  /**< indicates a request to delete an object instance */
    AwaOperation_Read,                  /**< indicates a request to read from a resource */
    AwaOperation_Write,                 /**< indicates a request to write to a resource */
    AwaOperation_Execute,               /**< indicates a request to write to a resource */
    AwaOperation_CreateResource,        /**< indicates a request to create a resource */
    AwaOperation_DeleteResource,        /**< indicates a request to delete a resource */
} AwaOperation;

/**
 * Represents an Awa Static Client context, holding the necessary information
 * to initialise the client daemon, register objects and resources, and handle
 * callbacks.
 */
typedef struct _AwaStaticClient AwaStaticClient;

typedef enum
{
  AwaSecurityMode_PreSharedKey,   /**< indicates pre-shared key security mode (PSK) */
  AwaSecurityMode_RawPublicKey,   /**< indicates raw public key security mode (RPK) */
  AwaSecurityMode_Certificate,    /**< indicates certicate-based security mode */
  AwaSecurityMode_NoSec,          /**< indicates no security mode */
} AwaSecurityMode;

#define BOOTSTRAP_CONFIG_SERVER_URI_SIZE (256)
#define BOOTSTRAP_CONFIG_PUBLIC_KEY_SIZE (128)
#define BOOTSTRAP_CONFIG_SERVER_KEY_SIZE (128)
#define BOOTSTRAP_CONFIG_SECRET_KEY_SIZE (128)
#define BOOTSTRAP_CONFIG_BINDING_SIZE     (10)

/**
 * Information required to bootstrap the client daemon from a factory configuration.
 * The format of this information is directly drawn from the LWM2M specification.
 */
typedef struct
{
    /**
     * LWM2M Security Object information, required to securely bootstrap and
     * connect to LWM2M servers. See the LWM2M specification for the Security object.
     */
    struct
    {
        char ServerURI[BOOTSTRAP_CONFIG_SERVER_URI_SIZE];           /**< Specifies the URI of the LWM2M Server for the client to register with. For example: `coap://127.0.0.1:5683` */
        AwaSecurityMode SecurityMode;                               /**< Specifies the LWM2M Security Mode. */
        char PublicKeyOrIdentity[BOOTSTRAP_CONFIG_PUBLIC_KEY_SIZE]; /**< Specifies the LWM2M Client's certificate (Certificate mode), public key (RPK mode), or PSK identity (PSK mode). See the LWM2M specification for details of the required format for each mode. */
        char ServerPublicKey[BOOTSTRAP_CONFIG_SERVER_KEY_SIZE];     /**< Specifies the LWM2M Server’s or LWM2M Bootstrap Server’s Certificate (Certificate mode), public key (RPK mode). See the LWM2M specification for details of the required format for each mode. */
        char SecretKey[BOOTSTRAP_CONFIG_SECRET_KEY_SIZE];           /**< Specifies the secret key or private key of the security mode. See the LWM2M specification for details of the required format for each mode. */
    } SecurityInfo;

    /**
     * LWM2M Server Object information, providing the data related to an LWM2M Server.
     * See the LWM2M specification for the Server object.
     */
    struct
    {
        int Lifetime;                                /**< Specify the lifetime of the registration in seconds. */
        int DefaultMinPeriod;                        /**< Specify the default value the LWM2M Client should use for the Minimum Period of an Observation in the absence of this parameter being included in an Observation. */
        int DefaultMaxPeriod;                        /**< Specify the default value the LWM2M Client should use for the Maximum Period of an Observation in the absence of this parameter being included in an Observation. */
        int DisableTimeout;                          /**< Specify the period to disable the Server. After this period, the LWM2M Client MUST perform registration process to the Server. */
        bool Notification;                           /**< Specify Notification Storing When Disabled or Offline: if true, the LWM2M Client stores “Notify” operations to the LWM2M Server while the LWM2M Server account is disabled or the LWM2M Client is offline. After the LWM2M Server account is enabled or the LWM2M Client is online, the LWM2M Client reports the stored “Notify” operations to the Server. If false, the LWM2M Client discards all the “Notify” operationsor temporally disables the Observe function while the LWM2M Server is disabled or the LWM2M Client is offline. */
        char Binding[BOOTSTRAP_CONFIG_BINDING_SIZE]; /**< Specifies the transport binding configured for the LWM2M Client. See the LWM2M specification for details of the required format. */
    } ServerInfo;
} AwaFactoryBootstrapInfo;

/**
 * Supported result codes for handler callbacks.
 */
typedef enum
{
    AwaResult_Success = 200,           /**< indicates the requested operation completed successfully */
    AwaResult_SuccessCreated = 201,    /**< indicates the requested target was created successfully */
    AwaResult_SuccessDeleted = 202,    /**< indicates the requested target was deleted successfully */
    AwaResult_SuccessChanged = 204,    /**< indicates a successful write to a resource */
    AwaResult_SuccessContent = 205,    /**< indicates a successful read from a resource */

    AwaResult_BadRequest = 400,        /**< indicates an invalid argument or request to create an entity that already exists */
    AwaResult_Unauthorized = 401,      /**< indicates an attempt to call the requested operation with insufficient authorisation */
    AwaResult_Forbidden = 403,         /**< indicates a request to create an object instance */
    AwaResult_NotFound = 404,          /**< indicates the specified object, object instance or resource does not exist */
    AwaResult_MethodNotAllowed = 405,  /**< indicates the requested operation is not allowed for the specified target */
    AwaResult_NotAcceptable = 406,     /**< indicates a requested accept-header was not acceptable by the client daemon */

    AwaResult_InternalError = 500,     /**< indicates the handler failed internally while processing a request */

    AwaResult_OutOfMemory = 999,       /**< indicates the handler did not have sufficient memory to service the requested operation */
    AwaResult_AlreadyRegistered,       /**< indicates an attempt to register an already registered object or resource */
    AwaResult_MismatchedRegistration,  /**< indicates an attempt to register an object definition does not match the existing definition */
    AwaResult_AlreadyCreated,          /**< indicates an attempt to create an existing resource or object instance */
    AwaResult_Unsupported,             /**< indicates the requested operation is not supported by the called handler */
    AwaResult_Unspecified = -1,        /**< indicates the requested operation was ignored by the called handler */
} AwaResult;


/**************************************************************************************************
 * Callbacks
 *************************************************************************************************/

/**
 * @brief A user-specified callback handler for a LWM2M Operation on the specified /O/I/R/i path,
 *        which will be called whenever a management server performs a LWM2M operation on the
 *        specified client, allowing full control of the operation on the target entity as well
 *        as the return code to be sent back to the server that performed the operation.
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] operation The requested operation to perform on the Client Daemon.
 * @param[in] objectID Identifies the object for which the query is targeted.
 * @param[in] objectInstanceID Identifies the object instance for which the query is targeted, or AWA_INVALID_ID
 *            if the request is on an object.
 * @param[in] resourceID Identifies the resource for which the query is targeted, or AWA_INVALID_ID
 *            if the request is on an object instance or object.
 * @param[in] resourceInstanceID Identifies the resource instance for which the query is targeted, or AWA_INVALID_ID
 *            if the request is on a single-instance resource, object instance or object.
 * @param[in,out] dataPointer A pointer to a void pointer containing data for the requested resource to be read or modified.
 * @param[in,out] dataSize A pointer to an unsigned integer containing the length of the requested resource.
 * @param[out] changed Set by the handler to indicate whether the resource's value has been changed by the handler
 *                If set to true, a notification will be sent when possible to any observers
 *                of the target object, object instance or resource.
 * @return AwaResult_SuccessCreated on a successful create object instance or resource operation.
 * @return AwaResult_SuccessDeleted on a successful delete object instance or resource operation.
 * @return AwaResult_SuccessContent on a successful read operation.
 * @return AwaResult_SuccessChanged on a successful write or execute operation.
 * @return Various errors on failure.
 */
typedef AwaResult (*AwaStaticClientHandler)(AwaStaticClient * client, AwaOperation operation,
                                            AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID,
                                            void ** dataPointer, size_t * dataSize, bool * changed);


/************************************************************************************************************
 * Awa Static Client Initialisation and Teardown
 ************************************************************************************************************/

/**
 * @brief Allocate and return a pointer to a new Awa Static Client, that will be used to setup and process an
 *        Awa Client Daemon, register custom objects, resources and callback handlers.
 *        The Awa Static Client is owned by the caller and should eventually be freed with ::AwaStaticClient_Free.
 * @return A pointer to a newly allocated Awa Static Client instance.
 * @return NULL on failure.
 */
AwaStaticClient * AwaStaticClient_New();

/**
 * @brief Set the global Client Daemon and CoAP log level. This function is not
 *        tied to an AwaStaticClient and can be called at any time.
 * @param[in] level A valid Awa Log Level.
 * @return AwaError_Success on success.
 * @return AwaError_LogLevelInvalid if an unknown log level was specified.
 */
AwaError AwaStaticClient_SetLogLevel(AwaLogLevel level);

/**
 * @brief Set the client endpoint name - the unique ID the client will be identified by
 *        any server the Client Daemon is connected to. This function must be called
 *        before performing AwaStaticClient_Init.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] endPointName The unique endpoint name to identify this client.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the client has already been initialised.
 * @return AwaError_StaticClientInvalid if the client is NULL.
 */
AwaError AwaStaticClient_SetEndPointName(AwaStaticClient * client, const char * endPointName);

/**
 * @brief Set the address and port to listen on for incoming CoAP packets. This function must
 *        be called before performing AwaStaticClient_Init.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] address IP address to listen on.
 * @param[in] port Port number to listen on.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the client has already been initialised.
 * @return AwaError_StaticClientInvalid if the client is NULL.
 */
AwaError AwaStaticClient_SetCoAPListenAddressPort(AwaStaticClient * client, const char * address, int port);

/**
 * @brief Set the URI to connect to a trusted Bootstrap server, in the following
 *        format: "coap://[ip]:port". This function can only be called before
 *        performing AwaStaticClient_Init.
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] bootstrapServerURI URI the bootstrap server is listening on.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the client has already been initialised.
 * @return AwaError_StaticClientInvalid if the client is NULL.
 */
AwaError AwaStaticClient_SetBootstrapServerURI(AwaStaticClient * client, const char * bootstrapServerURI);

/**
 * @brief Bypass a bootstrap server by supplying factory bootstrap settings
 *        to connect to a trusted LWM2M server. This function can only be called before
 *        performing AwaStaticClient_Init.
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] factoryBootstrapInformation A pointer to valid Factory Bootstrap information.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the client has already been initialised.
 * @return AwaError_StaticClientInvalid if the client is NULL.
 */
AwaError AwaStaticClient_SetFactoryBootstrapInformation(AwaStaticClient * client, const AwaFactoryBootstrapInfo * factoryBootstrapInformation);

/**
 * @brief Set a user-specified application context which can be accessed within
 *        any registered callback handler owned by an Awa Static Client.
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] context A pointer to user-specified data.
 * @return AwaError_Success on success.
 * @return AwaError_StaticClientInvalid if the client is NULL.
 */
AwaError AwaStaticClient_SetApplicationContext(AwaStaticClient * client, void * context);

/**
 * @brief Retrieve a user-specified application context.
 * @param[in] client A pointer to a valid Awa Static Client.
 * @return The user-specified application context if it exists.
 * @return NULL if the client is NULL, or no application context was set by the user.
 */
void * AwaStaticClient_GetApplicationContext(AwaStaticClient * client);

/**
 * @brief Initialise an Awa Static client. This function can only be called
 *        once all information required for the client to connect to a
 *        LWM2M server has been set.
 * @param[in] client A pointer to a valid Awa Static Client.
 * @return AwaError_Success on successful initialisation of the Awa Static Client.
 * @return AwaError_StaticClientNotConfigured if the client's endpoint name,
 *         bootstrap configuration or CoAP listen port have not been set.
 * @return AwaError_StaticClientInvalid if the client is NULL.
 */
AwaError AwaStaticClient_Init(AwaStaticClient * client);

/**
 * @brief Free an Awa Static client. This function will close any connected
 *        sockets and safely shut down the LWM2M Daemon Core.
 * @param[in,out] client A pointer to an Awa Static Client pointer that will be set to NULL.
 */
void AwaStaticClient_Free(AwaStaticClient ** client);


/************************************************************************************************************
 * Awa Static Client Object and Resource Registration
 ************************************************************************************************************/

/**
 * @brief Register a custom LWM2M object with a user-specified callback handler,
 *        which will be called whenever an operation on any instances of the
 *        registered object is performed.
 *        In order for an LWM2M server to perform operations on the registered object,
 *        a matching object must defined on the LWM2M server.
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] objectName A human-friendly name for the registered object.
 * @param[in] objectID An ID that uniquely identifies the object.
 * @param[in] minimumInstances The minimum number of instances of this object that must exist at any time.
 * @param[in] maximumInstances The maximum number of instances of this object that must exist at any time.
 * @param[in] handler A user-specified callback handler.
 * @return AwaError_Success on success.
 * @return AwaError_DefinitionInvalid if the objectName is invalid or the objectID,
 *         minimumInstances, or maximumInstances are out of range.
 * @return AwaError_StaticClientInvalid if the client is NULL.
 */
AwaError AwaStaticClient_RegisterObjectWithHandler(AwaStaticClient * client, const char * objectName, AwaObjectID objectID,
                                                   uint16_t minimumInstances, uint16_t maximumInstances, AwaStaticClientHandler handler);

/**
 * @brief Register a custom LWM2M object, leaving handling of any instances of the object to the LWM2M Client Daemon.
 *        In order for an LWM2M server to perform operations on the registered object,
 *        a matching object must defined on the LWM2M server.
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] objectName A human-friendly name for the registered object.
 * @param[in] objectID An ID that uniquely identifies the object.
 * @param[in] minimumInstances The minimum number of instances of this object that must exist at any time.
 * @param[in] maximumInstances The maximum number of instances of this object that must exist at any time.
 * @return AwaError_Success on success.
 * @return AwaError_DefinitionInvalid if the objectName is invalid or the objectID,
 *         minimumInstances, or maximumInstances are out of range.
 * @return AwaError_StaticClientInvalid if the client is NULL.
 */
AwaError AwaStaticClient_RegisterObject(AwaStaticClient * client, const char * objectName, AwaObjectID objectID,
                                        uint16_t minimumInstances, uint16_t maximumInstances);

/**
 * @brief Register a resource to an existing object with a user-specified callback handler,
 *        which will be called whenever an operation on the resource is performed.
 *        In order for an LWM2M server to perform operations on the registered resource,
 *        a matching object with resources must defined on the LWM2M server.
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] resourceName A human-friendly name for the registered resource.
 * @param[in] objectID An ID that uniquely identifies the object which will contain the resource to be registered.
 * @param[in] resourceID An ID that uniquely identifies the resource within the object.
 * @param[in] resourceType The type of this resource.
 * @param[in] minimumInstances The minimum number of instances of this object that must exist at any time.
 * @param[in] maximumInstances The maximum number of instances of this object that must exist at any time.
 * @param[in] operations The allowed operations on the registered resource.
 * @param[in] handler A user-specified callback handler.
 *
 * @return AwaError_Success on success.
 * @return AwaError_DefinitionInvalid if the objectName is invalid or the objectID,
 *         minimumInstances, or maximumInstances are out of range.
 * @return AwaError_StaticClientInvalid if the client is NULL.
 */
AwaError AwaStaticClient_RegisterResourceWithHandler(AwaStaticClient * client, const char * resourceName,
                                                     AwaObjectID objectID, AwaResourceID resourceID, AwaResourceType resourceType,
                                                     uint16_t minimumInstances, uint16_t maximumInstances, AwaResourceOperations operations,
                                                     AwaStaticClientHandler handler);

/**
 * @brief Register a resource to an existing object with a pointer to the resource's data,
 *        leaving handling of the resource to the LWM2M Client Daemon. The resource's value
 *        within any of its instances may be directly modified at any time, however
 *        AwaStaticClient_ResourceChanged should be called to allow notifications to be sent to
 *        any observing LWM2M servers. In order for an LWM2M server to perform operations on the registered
 *        resource, a matching object with resources must defined on the LWM2M server.
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] resourceName A human-friendly name for the registered resource.
 * @param[in] objectID An ID that uniquely identifies the object which will contain the resource to be registered.
 * @param[in] resourceID An ID that uniquely identifies the resource within the object.
 * @param[in] resourceType The type of this resource.
 * @param[in] minimumInstances The minimum number of instances of this object that must exist at any time.
 * @param[in] maximumInstances The maximum number of instances of this object that must exist at any time.
 * @param[in] operations The allowed operations on the registered resource.
 * @param[in] dataPointer A pointer to the resource's data.
 * @param[in] dataElementSize The size in bytes of the resource's data.
 * @param[in] dataStepSize The step size in bytes between the resource's data per object instance.
 * @return AwaError_Success on success.
 * @return AwaError_DefinitionInvalid if the resourceName is invalid or the objectID, resourceID, minimumInstances, or
 *         maximumInstances are out of range, the dataPointer is NULL or dataElementSize is less than 1.
 * @return AwaError_StaticClientInvalid if the client is NULL.
 */
AwaError AwaStaticClient_RegisterResourceWithPointer(AwaStaticClient * client, const char * resourceName,
                                                     AwaObjectID objectID, AwaResourceID resourceID, AwaResourceType resourceType,
                                                     uint16_t minimumInstances, uint16_t maximumInstances, AwaResourceOperations operations,
                                                     void * dataPointer, size_t dataElementSize, size_t dataStepSize);

/**
 * @brief Register a resource to an existing object with an array of pointers to non-contiguous data,
 *        where each piece of data stores the resource for a single instance of its object.
 *        Handling of the resource is left to the LWM2M Client Daemon. The resource's value
 *        within any of its instances may be directly modified at any time, however
 *        AwaStaticClient_ResourceChanged should be called to allow notifications to be sent to any observing
 *        LWM2M servers. In order for an LWM2M server to perform operations on the registered resource,
 *        a matching object with resources must defined on the LWM2M server.
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] resourceName A human-friendly name for the registered resource.
 * @param[in] objectID An ID that uniquely identifies the object which will contain the resource to be registered.
 * @param[in] resourceID An ID that uniquely identifies the resource within the object.
 * @param[in] resourceType The type of this resource.
 * @param[in] minimumInstances The minimum number of instances of this object that must exist at any time.
 * @param[in] maximumInstances The maximum number of instances of this object that must exist at any time.
 * @param[in] operations The allowed operations on the registered resource.
 * @param[in] dataPointers An array of pointers, each containing the location of the resource's data for a single object instance.
 * @param[in] dataElementSize The size in bytes of the resource's data.
 * @return AwaError_Success on success.
 * @return AwaError_DefinitionInvalid if the resourceName is invalid or the objectID, resourceID, minimumInstances, or
 *         maximumInstances are out of range, dataPointers is NULL, empty or dataElementSize is less than 1.
 * @return AwaError_StaticClientInvalid if the client is NULL.
 */
AwaError AwaStaticClient_RegisterResourceWithPointerArray(AwaStaticClient * client, const char * resourceName,
                                                          AwaObjectID objectID, AwaResourceID resourceID, AwaResourceType resourceType,
                                                          uint16_t minimumInstances, uint16_t maximumInstances, AwaResourceOperations operations,
                                                          void * dataPointers[], size_t dataElementSize);


/************************************************************************************************************
 * Awa Static Client Processing
 ************************************************************************************************************/

/**
 * @brief Process the Awa Static Client. This is the main process function which in turn processes
 *        the LWM2M Client Daemon core, updating the LWM2M state machine, handling any incoming CoAP messages,
 *        sending notifications and registration updates to servers.
 * @param[in] client A pointer to a valid Awa Static Client.
 * @return The time until AwaStaticClient_Process should next be called.
 */
int AwaStaticClient_Process(AwaStaticClient * client);


/************************************************************************************************************
 * Awa Static Client Object and Resource Manipulation
 ************************************************************************************************************/

/**
 * @brief Retrieve a pointer to a resource instance's data, in order to modify it within a user-specified callback handler.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] objectID The ID of the object for the specified resource instance.
 * @param[in] objectInstanceID The ID of the object instance for the specified resource instance.
 * @param[in] resourceID The ID of the resource for the specified resource instance.
 * @param[in] resourceInstanceID The ID of the resource instance.
 * @return A pointer to the resource instance's data if it exists.
 * @return NULL if the client is invalid or the resource instance does not exist for the specified
 *         object, object instance, resource and resource instance IDs.
 */
void * AwaStaticClient_GetResourceInstancePointer(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID);

/**
 * @brief Create an optional resource within the specified object instance.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] objectID The ID of the object for the specified resource.
 * @param[in] objectInstanceID The ID of the object instance for the specified resource.
 * @param[in] resourceID The ID of the optional resource to create.
 * @return AwaError_Success if the resource is created successfully.
 * @return AwaError_CannotCreate if the resource already exists, or no resource is registered for the specified object, object instance and resource ID.
 * @return AwaError_StaticClientInvalid if the specified client is NULL or uninitialised.
 */
AwaError AwaStaticClient_CreateResource(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID);

/**
 * @brief Create an object instance within the specified object.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] objectID The ID of the object for the specified object instance.
 * @param[in] objectInstanceID The ID of the object instance to create.
 * @return AwaError_Success if the object instance is created successfully.
 * @return AwaError_CannotCreate if the object instance already exists, the object already contains a maximum number of object instances,
 *         or if no object instance is registered for the specified object ID.
 * @return AwaError_StaticClientInvalid if the specified client is NULL or uninitialised.
 */
AwaError AwaStaticClient_CreateObjectInstance(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID);

/**
 * @brief Mark the specified resource as changed, in order for the Awa Client Daemon to
 *        send notifications to any LWM2M servers observing that resource.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] objectID The ID of the object containing the specified resource.
 * @param[in] objectInstanceID The ID of the object instance containing the specified resource.
 * @param[in] resourceID The ID of the resource to mark as changed.
 * @return AwaError_Success if the specified resource exists and was successfully marked as changed.
 * @return AwaError_DefinitionInvalid if no resource exists for the specified object, object instance and resource IDs.
 * @return AwaError_StaticClientInvalid if the specified client is NULL or uninitialised.
 */
AwaError AwaStaticClient_ResourceChanged(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID);

/**
 * @brief Mark the specified object instance as changed, in order for the Awa Client Daemon to
 *        send notifications to any LWM2M servers observing that object instance.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] objectID The ID of the object for the specified object instance.
 * @param[in] objectInstanceID The ID of the object instance to mark as changed.
 * @return AwaError_Success if the specified resource exists and was successfully marked as changed.
 * @return AwaError_DefinitionInvalid if no resource exists for the specified object, object instance and resource IDs.
 * @return AwaError_StaticClientInvalid if the specified client is NULL or uninitialised.
 */
AwaError AwaStaticClient_ObjectInstanceChanged(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID);

#ifdef __cplusplus
}
#endif

#endif /* AWA_STATIC_CLIENT_H */

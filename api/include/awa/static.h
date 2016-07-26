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
 * @file static.h
 * @brief This interface enables an application to be statically embedded within the LWM2M Client and interact with its resources.
 *
 * The Client hosts resources within a data model based on Objects, Object Instances and Resources.
 * Please consult the LWM2M specification for details of this model.
 *
 * A Management Application may interact with the Client via LWM2M, accessing the same resources. Therefore
 * it is essential that both the Client Application and Management Application are aware of the same data
 * model and have definitions for the same objects and resources.
 *
 * It is recommended that OMA registered IDs are used for objects that conform to registered LWM2M objects such
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
*        Define an object and resources with pointers.
* @example static-client-tutorial3.c
*        Define an object and resources with a custom handler.
*/


/**************************************************************************************************
 * Public Types and Enumerations
 *************************************************************************************************/

/**
 * Supported operations for resource and object handlers defined with ::AwaStaticClient_DefineResource.
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
 * to initialise the client daemon, define objects and resources, and handle
 * callbacks.
 */
typedef struct _AwaStaticClient AwaStaticClient;

/**
 * Supported LWM2M security modes. See the LWM2M specification for details.
 */
typedef enum
{
  AwaSecurityMode_PreSharedKey,   /**< indicates Pre-Shared Key security mode (PSK) */
  AwaSecurityMode_RawPublicKey,   /**< indicates Raw Public Key security mode (RPK) */
  AwaSecurityMode_Certificate,    /**< indicates Certificate-based security mode */
  AwaSecurityMode_NoSec,          /**< indicates No Security mode */
} AwaSecurityMode;

#define BOOTSTRAP_CONFIG_SERVER_URI_SIZE (256)  /**< Maximum size in characters of Server URI string. */
#define BOOTSTRAP_CONFIG_PUBLIC_KEY_SIZE (128)  /**< Maximum size in bytes of Public Key / Identity opaque value. */
#define BOOTSTRAP_CONFIG_SERVER_KEY_SIZE (128)  /**< Maximum size in bytes of Server Key opaque value. */
#define BOOTSTRAP_CONFIG_SECRET_KEY_SIZE (128)  /**< Maximum size in bytes of Secret Key opaque value. */
#define BOOTSTRAP_CONFIG_BINDING_SIZE     (10)  /**< Maximum length in characters of Binding string. */

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
    AwaResult_AlreadyDefined,          /**< indicates an attempt to define an already defined object or resource */
    AwaResult_MismatchedDefinition,    /**< indicates an attempt to define an object that does not match the existing definition */
    AwaResult_AlreadyCreated,          /**< indicates an attempt to create an existing resource or object instance */
    AwaResult_Unsupported,             /**< indicates the requested operation is not supported by the called handler */
    AwaResult_Unspecified = -1,        /**< indicates the requested operation was ignored by the called handler */
} AwaResult;


/**************************************************************************************************
 * Callbacks
 *************************************************************************************************/

/**
 * @brief A user-specified callback handler for a LWM2M Operation on the specified /O/I/R/i path
 *        to an LWM2M entity that will be called whenever a management server performs a LWM2M
 *        operation on the specified client, allowing full control of the operation on the target
 *        entity as well as the return code to be sent back to the server that performed the operation.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] operation The requested operation to perform on the entity.
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
 * @brief Allocate and return a pointer to a new Awa Static Client. The new client can be used to
 *        define custom objects, resources and callback handlers, and respond to LWM2M operations.
 *
 *        The Awa Static Client is owned by the caller and should eventually be freed with ::AwaStaticClient_Free.
 *
 * @return A pointer to a newly allocated Awa Static Client instance.
 * @return NULL on failure.
 */
AwaStaticClient * AwaStaticClient_New();

/**
 * @brief Retrieve default content type to use when a request doesn't specify via accept header a preferred content type.
 *
 *        The default is initially AwaContentType_ApplicationPlainText.
 *
 * @return The default content type
 */
AwaContentType AwaStaticClient_GetDefaultContentType();

/**
 * @brief Set the default content type to use when a request doesn't specify via accept header a preferred content type.
 *
 * @param[in] contentType The default contentType eg AwaContentType_ApplicationOmaLwm2mTLV.
 */
void AwaStaticClient_SetDefaultContentType(AwaContentType contentType);

/**
 * @brief Set the client log level. This function is not
 *        tied to an AwaStaticClient and has global effect.
 *        Only messages with level equal to or less than the current log level will
 *        be displayed in the log.
 *
 *        This function can be called at any time.
 *
 * @param[in] level A valid log level.
 * @return AwaError_Success on success.
 * @return AwaError_LogLevelInvalid if an invalid log level is specified.
 */
AwaError AwaStaticClient_SetLogLevel(AwaLogLevel level);

/**
 * @brief Set the client endpoint name. This is the unique ID used to identify the client
 *        from any server the client has registered with.
 *
 *        This function must be called, and before calling ::AwaStaticClient_Init.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] endPointName The unique endpoint name to identify this client.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the client has already been initialised.
 * @return AwaError_StaticClientInvalid if the client is NULL.
 */
AwaError AwaStaticClient_SetEndPointName(AwaStaticClient * client, const char * endPointName);

/**
 * @brief Set the address and port to listen on for incoming CoAP packets. IPv4 and IPv6 addresses are valid.
 *        It must be the address of a local interface. Only local DNS names are valid.
 *
 *        Address "Any" (`0.0.0.0` for IPv4, `::` for IPv6) is valid.
 *
 *        This function must be called, and before calling ::AwaStaticClient_Init.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] address IP address to listen on.
 * @param[in] port Port number to listen on.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the client has already been initialised.
 * @return AwaError_StaticClientInvalid if @e client is NULL.
 */
AwaError AwaStaticClient_SetCoAPListenAddressPort(AwaStaticClient * client, const char * address, int port);

/**
 * @brief Set the network location of a trusted LWM2M Bootstrap server, in the URI
 *        format: "coap://[address]:[port]". The client will connect to the bootstrap
 *        server to obtain the LWM2M Server Bootstrap Information.
 *
 *        [address] can be an IPv4 or IPv6 address, or a resolvable DNS name.
 *
 *        This function must be called, and before calling ::AwaStaticClient_Init.
 *
 *        If ::AwaStaticClient_SetFactoryBootstrapInformation is called, @e bootstrapServerURI is ignored.
 *        However if the factory bootstrap information results in a failed registration, @e bootstrapServerURI will
 *        be used to do a client-initiated bootstrap.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] bootstrapServerURI Network location of bootstrap server.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the client has already been initialised.
 * @return AwaError_StaticClientInvalid if @e client is NULL.
 */
AwaError AwaStaticClient_SetBootstrapServerURI(AwaStaticClient * client, const char * bootstrapServerURI);

/**
 * @brief Configure factory bootstrap by supplying LWM2M Server Bootstrap Information to register with a LWM2M server.
 *
 *        This function is optional. It should only be called @e after ::AwaStaticClient_Init and before ::AwaStaticClient_Process.
 *
 *        It may be called multiple times to set additional LWM2M servers.
 *
 *        If this factory bootstrap information results in a failed registration, @e bootstrapServerURI set
 *        by ::AwaStaticClient_SetBootstrapServerURI will be used to do a client-initiated bootstrap.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] factoryBootstrapInformation A pointer to valid Factory Bootstrap information.
 * @return AwaError_Success on success.
 * @return AwaError_OperationInvalid if the client has already been initialised.
 * @return AwaError_StaticClientInvalid if @e client is NULL.
 */
AwaError AwaStaticClient_SetFactoryBootstrapInformation(AwaStaticClient * client, const AwaFactoryBootstrapInfo * factoryBootstrapInformation);

/**
 * @brief Configure default certificate to use when Awa Static Client connects to a secure Server/Bootstrap.
 *
 *        This function is optional. It should only be called @e after ::AwaStaticClient_Init and before ::AwaStaticClient_Process.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] certificate A pointer to a certificate which is in format.
 * @param[in] certificateLength Size of certificate.
 * @param[in] format Whether certificate is in either ASN1 or PEM format.
 */
void AwaStaticClient_SetCertificate(AwaStaticClient * client, const uint8_t * certificate, int certificateLength, AwaCertificateFormat format);

/**
 * @brief Configure default PSK to use when Awa Static Client connects to a secure Server/Bootstrap.
 *
 *        This function is optional. It should only be called @e after ::AwaStaticClient_Init and before ::AwaStaticClient_Process.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] identity A pointer to a NULL terminated string to identify client key.
 * @param[in] key A pointer to shared key.
 * @param[in] keyLength Size of key
 */
void AwaStaticClient_SetPSK(AwaStaticClient * client, const char * identity, const uint8_t * key, int keyLength);

/**
 * @brief Set a user-specified application context which is accessible to
 *        any defined callback handler owned by the Awa Static Client with
 *        ::AwaStaticClient_GetApplicationContext.
 *        The handler should cast this pointer to the correct type.
 *
 *        This function can be called at any time.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] context A pointer to user-specified data.
 * @return AwaError_Success on success.
 * @return AwaError_StaticClientInvalid if @e client is NULL.
 */
AwaError AwaStaticClient_SetApplicationContext(AwaStaticClient * client, void * context);

/**
 * @brief Retrieve a user-specified application context as a void pointer. This pointer is
 *        associated with the Awa Static Client with ::AwaStaticClient_SetApplicationContext.
 *        The returned pointer should be cast to the correct type.
 *
 *        This function can be called at any time.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @return A pointer to the user-specified application context if it exists.
 * @return NULL if @e client is NULL, or no application context was set by the user.
 */
void * AwaStaticClient_GetApplicationContext(AwaStaticClient * client);

/**
 * @brief Initialise an Awa Static client. This function can only be called
 *        once all information required for the client to connect to a
 *        LWM2M server has been set.
 *
 *        This function should only be called once for each Awa Static Client.
 *
 *        Before calling this function, the following functions @b must be called:
 *          - ::AwaStaticClient_SetEndPointName
 *          - ::AwaStaticClient_SetCoAPListenAddressPort
 *          - ::AwaStaticClient_SetBootstrapServerURI
 *
 *        After calling this function, the following functions @b may be called before calling ::AwaStaticClient_Process:
 *          - ::AwaStaticClient_DefineObject
 *          - ::AwaStaticClient_SetObjectOperationHandler
 *          - ::AwaStaticClient_DefineResource
 *          - ::AwaStaticClient_SetResourceOperationHandler
 *          - ::AwaStaticClient_SetResourceStorageWithPointer
 *          - ::AwaStaticClient_SetResourceStorageWithPointerArray
 *          - ::AwaStaticClient_SetFactoryBootstrapInformation
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @return AwaError_Success on successful initialisation of the Awa Static Client.
 * @return AwaError_StaticClientNotConfigured if the client's endpoint name,
 *         bootstrap configuration or CoAP listen information has not been set.
 * @return AwaError_StaticClientInvalid if @e client is NULL.
 */
AwaError AwaStaticClient_Init(AwaStaticClient * client);

/**
 * @brief Free an Awa Static Client. This function will close any connected
 *        sockets and free any allocated memory.
 *
 *        The pointer will be set to NULL. Copies of the pointer to the Awa
 *        Static Client should not be used after this call.
 *
 * @param[in,out] client A pointer to an Awa Static Client pointer that will be set to NULL.
 */
void AwaStaticClient_Free(AwaStaticClient ** client);


/************************************************************************************************************
 * Awa Static Client Object and Resource Definition
 ************************************************************************************************************/

/**
 * @brief Define a new custom LWM2M object. By default, the LWM2M Client will handle instance operations such as delete and create.
 *        Use ::AwaStaticClient_SetObjectOperationHandler to override this default behaviour with a specified handler.
 *
 *        This function may only be called after a successful call to ::AwaStaticClient_Init.
 *
 * @note  In order for an LWM2M server to perform operations on the defined object,
 *        a matching object must be defined on the LWM2M server.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] objectName A human-friendly name for the new object.
 * @param[in] objectID An ID that uniquely identifies the object.
 * @param[in] minimumInstances The minimum number of instances of this object that must exist at any time. Must be less than or equal to @e maximumInstances.
 * @param[in] maximumInstances The maximum number of instances of this object that must exist at any time. Must be greater than zero.
 * @return AwaError_Success on success.
 * @return AwaError_DefinitionInvalid if @e objectName is invalid or @e objectID is out of range, or
 *         @e minimumInstances or @e maximumInstances are invalid.
 * @return AwaError_StaticClientInvalid if @e client is NULL.
 */
AwaError AwaStaticClient_DefineObject(AwaStaticClient * client, AwaObjectID objectID, const char * objectName,
                                      uint16_t minimumInstances, uint16_t maximumInstances);

/**
 * @brief Set the Object Operation handler function, to be called by the LWM2M Client when object instances are created or deleted.
 *
 *        The target object must be defined with ::AwaStaticClient_DefineObject.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] objectID An ID that uniquely identifies the defined object for which the handler will be associated.
 * @param[in] handler A user-specified callback handler.
 * @return AwaError_Success on success.
 * @return AwaError_DefinitionInvalid if @e objectID is invalid.
 * @return AwaError_StaticClientInvalid if @e client is NULL.
 */
AwaError AwaStaticClient_SetObjectOperationHandler(AwaStaticClient * client, AwaObjectID objectID, AwaStaticClientHandler handler);

/**
 * @brief Define a new resource as part of a defined object. By default, the LWM2M Client will handle resource operations.
 *        Use ::AwaStaticClient_SetResourceOperationHandler, ::AwaStaticClient_SetResourceStorageWithPointer or ::AwaStaticClient_SetResourceStorageWithPointerArray
 *        to configure whether operations are handled by the user, or directed to operate on static memory.
 *
 *        This function may only be called after a successful call to ::AwaStaticClient_Init, and the target object must be defined.
 *
 * @note  In order for an LWM2M server to perform operations on the defined resource,
 *        a matching object with resources must be defined on the LWM2M server.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] resourceName A human-friendly name for the new resource.
 * @param[in] objectID An ID that uniquely identifies the defined object which will contain the new resource.
 * @param[in] resourceID An ID that uniquely identifies the new resource within the object.
 * @param[in] resourceType The type of the new resource.
 * @param[in] minimumInstances The minimum number of instances of this resource that must exist at any time. Must be less than or equal to @e maximumInstances.
 * @param[in] maximumInstances The maximum number of instances of this resource that must exist at any time. Must be greater than zero.
 * @param[in] operations The allowed LWM2M operations on the new resource.
 *
 * @return AwaError_Success on success.
 * @return AwaError_DefinitionInvalid if @e resourceName is invalid, or @e objectID or @e resourceID is out of range, or
 *         @e minimumInstances or @e maximumInstances are invalid.
 * @return AwaError_StaticClientInvalid if @e client is NULL.
 */
AwaError AwaStaticClient_DefineResource(AwaStaticClient * client, AwaObjectID objectID, AwaResourceID resourceID,
                                        const char * resourceName, AwaResourceType resourceType,
                                        uint16_t minimumInstances, uint16_t maximumInstances, AwaResourceOperations operations);

/**
 * @brief Set a user-specified callback handler that will be called whenever a LWM2M operation on the resource is performed.
 *
 *        The target resource must be defined with ::AwaStaticClient_DefineResource before this function is called.
 *
 * @note  This function overrides the effect of ::AwaStaticClient_SetResourceStorageWithPointer and ::AwaStaticClient_SetResourceStorageWithPointerArray.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] objectID An ID that uniquely identifies the defined object that contains the defined resource.
 * @param[in] resourceID An ID that uniquely identifies the defined resource within the object.
 * @param[in] handler A user-specified callback handler.
 * @return AwaError_Success on success.
 * @return AwaError_DefinitionInvalid if @e objectID or @e resourceID is invalid or out of range.
 * @return AwaError_StaticClientInvalid if @e client is NULL.
 *
 */
AwaError AwaStaticClient_SetResourceOperationHandler(AwaStaticClient * client, AwaObjectID objectID, AwaResourceID resourceID, AwaStaticClientHandler handler);

/**
 * @brief Set a resource's storage with a pointer to the resource's data,
 *        leaving handling of the resource to the LWM2M Client. The resource's value
 *        within any of its instances may be directly modified at any time, however
 *        ::AwaStaticClient_ResourceChanged should be called to allow notifications to be sent to
 *        any observing LWM2M servers.
 *
 *        The target resource must be defined with ::AwaStaticClient_DefineResource before this function is called.
 *
 * @note  This function overrides the effect of ::AwaStaticClient_SetResourceOperationHandler and ::AwaStaticClient_SetResourceStorageWithPointerArray.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] objectID An ID that uniquely identifies the defined object that contains the defined resource.
 * @param[in] resourceID An ID that uniquely identifies the defined resource within the object.
 * @param[in] dataPointer A pointer to the resource's data.
 * @param[in] dataElementSize The size in bytes of the resource's data. Must be greater than or equal to 1.
 * @param[in] dataStepSize The step size in bytes between the resource's data per object instance.
 * @return AwaError_Success on success.
 * @return AwaError_DefinitionInvalid if @e objectID or @e resourceID is invalid is out of range, or
 *         @e dataPointers is NULL, or @e dataElementSize is less than 1.
 * @return AwaError_StaticClientInvalid if the client is NULL.
 */
AwaError AwaStaticClient_SetResourceStorageWithPointer(AwaStaticClient * client, AwaObjectID objectID, AwaResourceID resourceID, void * dataPointer, size_t dataElementSize, size_t dataStepSize);

/**
 * @brief Set a resource's storage with an array of pointers to non-contiguous data,
 *        where each piece of data stores the resource for a single instance of its object.
 *        Handling of the resource is left to the LWM2M Client. The resource's value
 *        within any of its instances may be directly modified at any time, however
 *        ::AwaStaticClient_ResourceChanged should be called to allow notifications to be sent to any observing
 *        LWM2M servers.
 *
 *        The target resource must be defined with ::AwaStaticClient_DefineResource before this function is called.
 *
 * @note  This function overrides the effect of ::AwaStaticClient_SetResourceOperationHandler and ::AwaStaticClient_SetResourceStorageWithPointer.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] objectID An ID that uniquely identifies the defined object that contains the defined resource.
 * @param[in] resourceID An ID that uniquely identifies the defined resource within the object.
 * @param[in] dataPointers An array of pointers, each containing the location of the resource's data for a single object instance.
 * @param[in] dataElementSize The size in bytes of the resource's data. Must be greater than or equal to 1.
 * @return AwaError_Success on success.
 * @return AwaError_DefinitionInvalid if @e objectID or @e resourceID is invalid or out of range, or
 *         @e dataPointers is NULL, or @e dataElementSize is less than 1.
 * @return AwaError_StaticClientInvalid if the client is NULL.
 */
AwaError AwaStaticClient_SetResourceStorageWithPointerArray(AwaStaticClient * client, AwaObjectID objectID, AwaResourceID resourceID, void * dataPointers[], size_t dataElementSize);


/************************************************************************************************************
 * Awa Static Client Processing
 ************************************************************************************************************/

/**
 * @brief Process the Awa Static Client. This is the main process function which in turn processes
 *        the LWM2M Client core, updating the LWM2M state machine, handling any incoming CoAP messages,
 *        and sending notifications and registration updates to servers.
 *
 *        A successful call to ::AwaStaticClient_Init must precede a call to this function.
 *
 *        This function needs to be called regularly, usually as part of a loop, and returns the
 *        time to wait before calling again.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @return The time until this function should next be called, in milliseconds.
 */
int AwaStaticClient_Process(AwaStaticClient * client);


/************************************************************************************************************
 * Awa Static Client Object and Resource Manipulation
 ************************************************************************************************************/

/**
 * @brief Retrieve a pointer to a resource instance's data, when defined with ::AwaStaticClient_SetResourceStorageWithPointer or ::AwaStaticClient_SetResourceStorageWithPointerArray.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] objectID The ID of the object for the specified resource instance.
 * @param[in] objectInstanceID The ID of the object instance for the specified resource instance.
 * @param[in] resourceID The ID of the resource for the specified resource instance.
 * @param[in] resourceInstanceID The ID of the resource instance.
 * @param[out] resourceSize The size of the resource instance.
 *
 * @return A pointer to the resource instance's data, if it exists.
 * @return NULL if @e client is NULL or uninitialised, or the resource instance does not exist for the specified
 *         object, object instance, resource and resource instance IDs.
 */
const void * AwaStaticClient_GetResourceInstancePointer(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, size_t * resourceSize);

/**
 * @brief Create an optional resource within the specified object instance.
 *
 * For a resource registered with a handler this will invoke a callback with the AwaOperation_CreateResource.
 *
 * For a resource registered by pointer this will create the resource. For a multi-instance resource, the number of resource instances created will be the maximum defined.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] objectID The ID of the object for the specified resource.
 * @param[in] objectInstanceID The ID of the object instance for the specified resource.
 * @param[in] resourceID The ID of the optional resource to create.
 * @return AwaError_Success if the resource is created successfully.
 * @return AwaError_CannotCreate if the resource already exists, or no resource is defined for the specified object, object instance and resource ID.
 * @return AwaError_StaticClientInvalid if @e client is NULL or uninitialised.
 */
AwaError AwaStaticClient_CreateResource(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID);

/**
 * @brief Delete an optional resource within the specified object instance.
 *
 * For a resource registered with a handler this will invoke a callback with the AwaOperation_DeleteResource.
 *
 * For a resource registered by pointer this will delete the resource.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] objectID The ID of the object for the specified resource.
 * @param[in] objectInstanceID The ID of the object instance for the specified resource.
 * @param[in] resourceID The ID of the optional resource to delete.
 * @return AwaError_Success if the resource is created successfully.
 * @return AwaError_CannotCreate if the resource already exists, or no resource is defined for the specified object, object instance and resource ID.
 * @return AwaError_StaticClientInvalid if @e client is NULL or uninitialised.
 */
AwaError AwaStaticClient_DeleteResource(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID);

/**
 * @brief Create an object instance within the specified object.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] objectID The ID of the object for the specified object instance.
 * @param[in] objectInstanceID The ID of the object instance to create.
 * @return AwaError_Success if the object instance is created successfully.
 * @return AwaError_CannotCreate if the object instance already exists, the object already contains a maximum number of object instances,
 *         or if no object is defined for the specified object ID.
 * @return AwaError_StaticClientInvalid if @e client is NULL or uninitialised.
 */
AwaError AwaStaticClient_CreateObjectInstance(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID);

/**
 * @brief Delete an object instance within the specified object.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] objectID The ID of the object for the specified object instance.
 * @param[in] objectInstanceID The ID of the object instance to delete.
 * @return AwaError_Success if the object instance is created successfully.
 * @return AwaError_CannotCreate if the object instance already exists, the object already contains a maximum number of object instances,
 *         or if no object is defined for the specified object ID.
 * @return AwaError_StaticClientInvalid if @e client is NULL or uninitialised.
 */
AwaError AwaStaticClient_DeleteObjectInstance(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID);

/**
 * @brief Mark the specified resource as changed, in order for the Awa Static Client to
 *        send notifications to all LWM2M servers observing that resource.
 * *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] objectID The ID of the object containing the specified resource.
 * @param[in] objectInstanceID The ID of the object instance containing the specified resource.
 * @param[in] resourceID The ID of the resource to mark as changed.
 * @return AwaError_Success if the specified resource exists and was successfully marked as changed.
 * @return AwaError_DefinitionInvalid if no resource exists for the specified object, object instance and resource IDs.
 * @return AwaError_StaticClientInvalid if @e client is NULL or uninitialised.
 */
AwaError AwaStaticClient_ResourceChanged(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID);

/**
 * @brief Mark the specified object instance as changed, in order for the Awa Static Client to
 *        send notifications to all LWM2M servers observing that object instance.
 *
 *        This function should also be called when a resource is created or deleted in an object instance.
 *
 * @param[in] client A pointer to a valid Awa Static Client.
 * @param[in] objectID The ID of the object for the specified object instance.
 * @param[in] objectInstanceID The ID of the object instance to mark as changed.
 * @return AwaError_Success if the specified resource exists and was successfully marked as changed.
 * @return AwaError_DefinitionInvalid if no resource exists for the specified object, object instance and resource IDs.
 * @return AwaError_StaticClientInvalid if @e client is NULL or uninitialised.
 */
AwaError AwaStaticClient_ObjectInstanceChanged(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID);

#ifdef __cplusplus
}
#endif

#endif /* AWA_STATIC_CLIENT_H */

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
 * @file error.h
 * @brief Provides definitions for Awa LWM2M Client and Server API error codes and error function declarations.
 */

#ifndef AWA_ERROR_H
#define AWA_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * API Error Codes
 */
typedef enum
{
    // These enums must align with src/error.c:ErrorStrings

    AwaError_Success = 0,                /**< Indicates an operation was successful */
    AwaError_Unspecified,                /**< Used as a default error value */
    AwaError_Unsupported,                /**< Indicates an API function is not supported */
    AwaError_Internal,                   /**< Indicates an internal error has occurred */
    AwaError_OutOfMemory,                /**< Indicates an operation failed to allocate memory */

    // 5
    AwaError_SessionInvalid,             /**< Indicates an operation was attempted in the context of an invalid session */
    AwaError_SessionNotConnected,        /**< Indicates an operation was attempted in the context of a valid session that is not connected to IPC */
    AwaError_NotDefined,                 /**< Indicates an operation was attempted on an object or resource that is not defined in the session */
    AwaError_AlreadyDefined,             /**< Indicates an object or resource has already been defined */
    AwaError_OperationInvalid,           /**< Indicates an operation was attempted on an invalid operation */

    // 10
    AwaError_PathInvalid,                /**< Indicates a specified path is invalid in the context of the operation */
    AwaError_PathNotFound,               /**< Indicates a valid path is not found during an operation */
    AwaError_TypeMismatch,               /**< Indicates a type mismatch between data and function */
    AwaError_Timeout,                    /**< Indicates an operation has timed out */
    AwaError_Overrun,                    /**< Indicates an operation has overrun the supplied buffer */

    // 15
    AwaError_IDInvalid,                  /**< Indicates an invalid ID or ID pointer was provided */
    AwaError_AddInvalid,                 /**< Indicates an incorrect or malformed Add operation was attempted */
    AwaError_CannotCreate,               /**< Indicates an attempt to create something that can't be created */
    AwaError_CannotDelete,               /**< Indicates an attempt to delete something that can't be deleted */
    AwaError_DefinitionInvalid,          /**< Indicates an attempt to use an invalid object/resource definition instance */

    // 20
    AwaError_AlreadySubscribed,          /**< Indicates an attempt to subscribe to a resource, object instance or object more than once */
    AwaError_SubscriptionInvalid,        /**< Indicates an attempt to use an invalid subscription instance */
    AwaError_ObservationInvalid,         /**< Indicates an attempt to use an invalid observation instance */
    AwaError_IPCError,                   /**< Indicates a general IPC error occurred */
    AwaError_ResponseInvalid,            /**< Indicates an attempt to get or use an invalid response */

    // 25
    AwaError_ClientIDInvalid,            /**< Indicates a client ID is invalid */
    AwaError_ClientNotFound,             /**< Indicates a client with a matching ID was not found */
    AwaError_LWM2MError,                 /**< Indicates a LWM2M error has occurred */
    AwaError_IteratorInvalid,            /**< Indicates an invalid iterator was used */
    AwaError_Response,                   /**< Indicates an operation response has errors */

    // 30
    AwaError_RangeInvalid,               /**< Indicates an operation on an invalid array range */
    AwaError_StaticClientInvalid,        /**< Indicates an operation was attempted in the context of an invalid Static Client */
    AwaError_StaticClientNotConfigured,  /**< Indicates a Static Client was initialised without being configured */
    AwaError_StaticClientNotInitialized, /**< Indicates a Static Client was not initialized */
    AwaError_LogLevelInvalid,            /**< Indicates an attempt to set an unknown log level  */

    AwaError_LAST                        /**< Reserved value */
} AwaError;

typedef enum
{
    // These enums must align with src/error.c:ServerErrorStrings

    AwaLWM2MError_Success = 0,          /**< Indicates no server occurred */
    AwaLWM2MError_Unspecified,          /**< Used as a default error value */
    AwaLWM2MError_BadRequest,           /**< Indicates a LWM2M 4.00 Bad Request error was encountered */
    AwaLWM2MError_Unauthorized,         /**< Indicates a LWM2M 4.01 Unauthorized error was encountered */
    AwaLWM2MError_NotFound,             /**< Indicates a LWM2M 4.04 Not Found error was encountered */
    AwaLWM2MError_MethodNotAllowed,     /**< Indicates a LWM2M 4.05 Not Allowed error was encountered */
    AwaLWM2MError_NotAcceptable,        /**< Indicates a LWM2M 4.06 Not Acceptable error was encountered */
    AwaLWM2MError_Timeout,              /**< Indicates a CoAP 5.04 Gateway timeout error was encountered */
    AwaLWM2MError_LAST                  /**< Reserved value */
} AwaLWM2MError;

/**
 * @brief Get a descriptive name for the specified error code.
 * @param[in] error Error code returned by an API function.
 * @return Pointer to an immutable c string.
 */
const char * AwaError_ToString(AwaError error);

/**
 * @brief Get an error code from a descriptive name of an error.
 * @param[in] errorString An error string to convert into a AwaError error code.
 * @return Pointer to an immutable c string.
 */
AwaError AwaError_FromString(const char * errorString);

/**
 * @brief Get a descriptive name for the specified error code.
 * @param[in] error Server error code returned by an API function.
 * @return Pointer to an immutable c string.
 */
const char * AwaLWM2MError_ToString(AwaLWM2MError error);

/**
 * @brief Get an error code from a descriptive name of an error.
 * @param[in] errorString An error string to convert into a AwaLWM2MError error code.
 * @return Pointer to an immutable c string.
 */
AwaLWM2MError AwaLWM2MError_FromString(const char * errorString);

#ifdef __cplusplus
}
#endif

#endif // AWA_ERROR_H


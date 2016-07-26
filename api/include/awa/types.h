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
 * @file types.h
 * @brief Provides definitions for types and enums used by Awa LWM2M API and Static API.
 */

#ifndef AWA_TYPES_H
#define AWA_TYPES_H

// @cond
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
// @endcond

#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int AwaObjectID;
typedef int AwaObjectInstanceID;
typedef int AwaResourceID;
typedef int AwaResourceInstanceID;

/**
 * Used to mark an invalid object, object instance or resource ID.
 */
#define AWA_INVALID_ID (-1)

/**
 * Defines the maximum permissible object, object instance or resource ID.
 */
#define AWA_MAX_ID (65535)

/**
 * Corresponds to the LWM2M 64-bit Integer type.
 */
typedef int64_t AwaInteger;

/**
 * Corresponds to the LWM2M Float type.
 */
typedef double AwaFloat;

/**
 * Corresponds to the LWM2M Boolean type.
 */
typedef bool AwaBoolean;

/**
 * Corresponds to the LWM2M Time type.
 */
typedef int64_t AwaTime;

/**
 * A utility struct used to convey data pointer and size of an opaque data block.
 */
typedef struct AwaOpaque
{
    void * Data;        /**< pointer to opaque data */
    size_t Size;        /**< size of opaque data block */
} AwaOpaque;

/**
 * A utility struct used to convey object link data.
 */
typedef struct AwaObjectLink
{
    AwaObjectID ObjectID;                  /**< Object ID */
    AwaObjectInstanceID ObjectInstanceID;  /**< Object Instance ID */
} AwaObjectLink;

// AwaString reserved for future use (UTF-8)

/**
 * Supported resource types
 */
typedef enum
{
    AwaResourceType_Invalid = 0,     /**< indicates an invalid resource type */

    AwaResourceType_None,            /**< indicates a resource with no type */
    AwaResourceType_String,          /**< indicates a resource capable of holding an ASCII string (UTF-8 is not supported) */
    AwaResourceType_Integer,         /**< indicates a resource capable of holding a LWM2M Integer value */
    AwaResourceType_Float,           /**< indicates a resource capable of holding a LWM2M Float value */
    AwaResourceType_Boolean,         /**< indicates a resource capable of holding a LWM2M Boolean value */
    AwaResourceType_Opaque,          /**< indicates a resource capable of holding a LWM2M Opaque value */
    AwaResourceType_Time,            /**< indicates a resource capable of holding a LWM2M Time value */
    AwaResourceType_ObjectLink,      /**< indicates a resource capable of holding a LWM2M ObjectLink value */

    // arrays (for multiple resource instances) are their own type
    AwaResourceType_StringArray,     /**< indicates a multiple-instance resource capable of holding a number of ASCII string values */
    AwaResourceType_IntegerArray,    /**< indicates a multiple-instance resource capable of holding a number of LWM2M Integer values */
    AwaResourceType_FloatArray,      /**< indicates a multiple-instance resource capable of holding a number of LWM2M Float values */
    AwaResourceType_BooleanArray,    /**< indicates a multiple-instance resource capable of holding a number of LWM2M Boolean values */
    AwaResourceType_OpaqueArray,     /**< indicates a multiple-instance resource capable of holding a number of LWM2M Opaque values */
    AwaResourceType_TimeArray,       /**< indicates a multiple-instance resource capable of holding a number of LWM2M Time values */
    AwaResourceType_ObjectLinkArray, /**< indicates a multiple-instance resource capable of holding a number of LWM2M ObjectLink values */

    // sentinel, do not remove
    AwaResourceType_LAST,            /**< Reserved value */

    AwaResourceType_FirstArrayType = AwaResourceType_StringArray,    /**< Reserved value */
    AwaResourceType_LastArrayType = AwaResourceType_ObjectLinkArray, /**< Reserved value */
} AwaResourceType;

/**
 * Supported resource operations for management servers
 */
typedef enum
{
    AwaResourceOperations_Invalid = -1, /**< indicates an invalid resource operation */

    AwaResourceOperations_None,         /**< indicates no operations are permitted to a management server */
    AwaResourceOperations_ReadOnly,     /**< indicates the resource is read-only to a management server */
    AwaResourceOperations_WriteOnly,    /**< indicates the resource is write-only to a management server */
    AwaResourceOperations_ReadWrite,    /**< indicates the resource can be read and written by a management server */
    AwaResourceOperations_Execute,      /**< indicates the resource can be executed by a management server */

    // sentinel, do not remove
    AwaResourceOperations_LAST          /**< Reserved value */
} AwaResourceOperations;

/**
 * Supported log levels for a log command
 */
typedef enum
{
    AwaLogLevel_None = -1,  /**< No log information is reported. */
    AwaLogLevel_Error,      /**< Only errors are reported. */
    AwaLogLevel_Warning,    /**< Warnings are reported, in addition to Error. */
    AwaLogLevel_Verbose,    /**< High-level information is reported, in addition to Warning. */
    AwaLogLevel_Debug,      /**< Low-level information is reported, in addition to Verbose. */
} AwaLogLevel;

/**
 * Supported Certificate formats
 */
typedef enum
{
    AwaCertificateFormat_None, /**< Not set. */
    AwaCertificateFormat_ASN1, /**< Raw ASN1 sequence. */
    AwaCertificateFormat_PEM,  /**< PEM format. */
} AwaCertificateFormat;


typedef enum
{
    AwaContentType_None = -1,
    AwaContentType_ApplicationPlainText      = 0,       // The new standard suggests to use PlainText
    AwaContentType_ApplicationLinkFormat     = 40,      // Object link format
    AwaContentType_ApplicationOctetStream    = 42,      // The new standard uses OctetStream, rather than omg.lwm2m+opaque
    AwaContentType_ApplicationJson           = 50,      // The new standard uses Json, rather than omg.lwm2m+json
    AwaContentType_ApplicationOmaLwm2mText   = 1541,    // application/vnd.oma.lwm2m+text (leshan uses 1541)
    AwaContentType_ApplicationOmaLwm2mTLV    = 1542,    // application/vnd.oma.lwm2m+tlv (TBD)??
    AwaContentType_ApplicationOmaLwm2mJson   = 1543,
    AwaContentType_ApplicationOmaLwm2mOpaque = 1544,

} AwaContentType;

#ifdef __cplusplus
}
#endif

#endif // AWA_TYPES_H


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
 * @file common.h
 * @brief Provides declarations and definitions that are common to the Awa LWM2M API and Static API.
 */

#ifndef AWA_COMMON_H
#define AWA_COMMON_H

// @cond
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
// @endcond

#include "types.h"
#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
 * Public Types and Enumerations
 *************************************************************************************************/

/**
 * Supported change types for a change subscription
 */
typedef enum
{
    AwaChangeType_Invalid = 0,             /**< indicates an invalid change type for the specified path */

    AwaChangeType_ResourceCreated,         /**< indicates a resource was created on the specified path */
    AwaChangeType_ResourceModified,        /**< indicates a resource was modified on the specified path */
    AwaChangeType_ResourceDeleted,         /**< indicates a resource was deleted on the specified path */

    AwaChangeType_ObjectInstanceCreated,   /**< indicates an object instance was created on the specified path */
    AwaChangeType_ObjectInstanceModified,  /**< indicates an object instance was modified on the specified path */
    AwaChangeType_ObjectInstanceDeleted,   /**< indicates an object instance was deleted on the specified path */

    AwaChangeType_Current,                 /**< TODO: indicates the current value of an object instance or resource on the specified path on subscription creation */

} AwaChangeType;

/**
 * Represents a timeout value, in milliseconds
 */
typedef int AwaTimeout;

/**
 * Supported subscribe types
 */
typedef enum
{
    AwaSubscribeType_None,
    AwaSubscribeType_Change,
    AwaSubscribeType_Execute,
} AwaSubscribeType;

/**************************************************************************************************
 * Array Management for Resources
 *************************************************************************************************/

/**
 * Represents a multiple-instance array index value
 */
typedef size_t AwaArrayIndex;

/**
 * Represents a multiple-instance array length value
 */
typedef size_t AwaArrayLength;

// Public array types
typedef struct _AwaStringArray AwaStringArray;
typedef struct _AwaIntegerArray AwaIntegerArray;
typedef struct _AwaFloatArray AwaFloatArray;
typedef struct _AwaBooleanArray AwaBooleanArray;
typedef struct _AwaOpaqueArray AwaOpaqueArray;
typedef struct _AwaTimeArray AwaTimeArray;
typedef struct _AwaObjectLinkArray AwaObjectLinkArray;

// Public Array Iterator types
typedef struct _AwaCStringArrayIterator AwaCStringArrayIterator;
typedef struct _AwaIntegerArrayIterator AwaIntegerArrayIterator;
typedef struct _AwaFloatArrayIterator AwaFloatArrayIterator;
typedef struct _AwaBooleanArrayIterator AwaBooleanArrayIterator;
typedef struct _AwaOpaqueArrayIterator AwaOpaqueArrayIterator;
typedef struct _AwaTimeArrayIterator AwaTimeArrayIterator;
typedef struct _AwaObjectLinkArrayIterator AwaObjectLinkArrayIterator;

/**
 * @defgroup Array_New Awa___Array_New
 * @addtogroup Array_New
 * @brief Allocate and return a pointer to a new array of the specified type,
 *        that can be used to set and retrieve values.
 *        The array is owned by the caller and should eventually be freed with Awa___Array_Free.
 * @return the newly allocated array on success.
 * @return NULL on failure.
 * @{
 */
AwaStringArray     * AwaStringArray_New    (void);
AwaIntegerArray    * AwaIntegerArray_New   (void);
AwaFloatArray      * AwaFloatArray_New     (void);
AwaBooleanArray    * AwaBooleanArray_New   (void);
AwaOpaqueArray     * AwaOpaqueArray_New    (void);
AwaTimeArray       * AwaTimeArray_New      (void);
AwaObjectLinkArray * AwaObjectLinkArray_New(void);
/** @} */

/**
 * @defgroup Array_Free Awa___Array_Free
 * @addtogroup Array_Free
 * @brief Clean up an array of the specified type, freeing all allocated resources.
 *        Once freed, the array is no longer valid.
 * @param[in,out] array A pointer to a array pointer of the specified type that will be set to NULL.
 * @{
 */
void AwaStringArray_Free    (AwaStringArray ** array);
void AwaIntegerArray_Free   (AwaIntegerArray ** array);
void AwaFloatArray_Free     (AwaFloatArray ** array);
void AwaBooleanArray_Free   (AwaBooleanArray ** array);
void AwaOpaqueArray_Free    (AwaOpaqueArray ** array);
void AwaTimeArray_Free      (AwaTimeArray ** array);
void AwaObjectLinkArray_Free(AwaObjectLinkArray ** array);
/** @} */

/**
 * @defgroup Array_SetValue Awa___Array_SetValue
 * @addtogroup Array_SetValue
 * @brief Sets a resource instance value at the specified index of an array.
 * @param[in] array A pointer to a valid resource array of the specified type.
 * @param[in] index The destination index of the resource instance to set the value to.
 * @param[in] value The value of the resource instance.
 * @{
 */
// RESERVED for UTF8 support:
//void AwaStringArray_SetValue       (AwaStringArray * array,     AwaArrayIndex index, AwaString value);
void AwaStringArray_SetValueAsCString(AwaStringArray * array,     AwaArrayIndex index, const char * value);
void AwaIntegerArray_SetValue        (AwaIntegerArray * array,    AwaArrayIndex index, AwaInteger value);
void AwaFloatArray_SetValue          (AwaFloatArray * array,      AwaArrayIndex index, AwaFloat value);
void AwaBooleanArray_SetValue        (AwaBooleanArray * array,    AwaArrayIndex index, AwaBoolean value);
void AwaOpaqueArray_SetValue         (AwaOpaqueArray * array,     AwaArrayIndex index, AwaOpaque value);
void AwaTimeArray_SetValue           (AwaTimeArray * array,       AwaArrayIndex index, AwaTime value);
void AwaObjectLinkArray_SetValue     (AwaObjectLinkArray * array, AwaArrayIndex index, AwaObjectLink value);
/** @} */

/**
 * @defgroup Array_DeleteValue Awa___Array_DeleteValue
 * @addtogroup Array_DeleteValue
 * @brief Deletes a resource instance value at the specified index of an array.
 * @param[in] array A pointer to a valid resource array of the specified type.
 * @param[in] index The index of the resource instance to delete.
 * @{
 */
void AwaStringArray_DeleteValue    (AwaStringArray * array,     AwaArrayIndex index);
void AwaIntegerArray_DeleteValue   (AwaIntegerArray * array,    AwaArrayIndex index);
void AwaFloatArray_DeleteValue     (AwaFloatArray * array,      AwaArrayIndex index);
void AwaBooleanArray_DeleteValue   (AwaBooleanArray * array,    AwaArrayIndex index);
void AwaOpaqueArray_DeleteValue    (AwaOpaqueArray * array,     AwaArrayIndex index);
void AwaTimeArray_DeleteValue      (AwaTimeArray * array,       AwaArrayIndex index);
void AwaObjectLinkArray_DeleteValue(AwaObjectLinkArray * array, AwaArrayIndex index);
/** @} */

/**
 * @defgroup Array_GetValue Awa___Array_GetValue
 * @addtogroup Array_GetValue
 * @brief Retrieves a resource instance value at the specified index of an array.
 * @param[in] array A pointer to a valid resource array of the specified type.
 * @param[in] index The index of the resource instance to retrieve.
 * @return the value of resource instance at the specified index if it exists
 * @return NULL if the array is invalid, the index is out of range, or the resource instance at the specified index is NULL.
 * @{
 */
// RESERVED for UTF8 support:
//AwaString   AwaStringArray_GetValue         (const AwaStringArray * array,     AwaArrayIndex index);
const char *      AwaStringArray_GetValueAsCString(const AwaStringArray * array,     AwaArrayIndex index);
AwaInteger    AwaIntegerArray_GetValue        (const AwaIntegerArray * array,    AwaArrayIndex index);
AwaFloat      AwaFloatArray_GetValue          (const AwaFloatArray * array,      AwaArrayIndex index);
AwaBoolean    AwaBooleanArray_GetValue        (const AwaBooleanArray * array,    AwaArrayIndex index);
AwaOpaque     AwaOpaqueArray_GetValue         (const AwaOpaqueArray * array,     AwaArrayIndex index);
AwaTime       AwaTimeArray_GetValue           (const AwaTimeArray * array,       AwaArrayIndex index);
AwaObjectLink AwaObjectLinkArray_GetValue     (const AwaObjectLinkArray * array, AwaArrayIndex index);
/** @} */

/**
 * @defgroup Array_GetValueCount Awa___Array_GetValueCount
 * @addtogroup Array_GetValueCount
 * @brief Retrieves the number of resource instances with values in an array.
 * @param[in] array A pointer to a valid resource array of the specified type.
 * @return the number of resource instances with values in the specified array.
 * @return 0 if the array is invalid.
 * @{
 */
size_t AwaStringArray_GetValueCount    (const AwaStringArray * array);
size_t AwaIntegerArray_GetValueCount   (const AwaIntegerArray * array);
size_t AwaFloatArray_GetValueCount     (const AwaFloatArray * array);
size_t AwaBooleanArray_GetValueCount   (const AwaBooleanArray * array);
size_t AwaOpaqueArray_GetValueCount    (const AwaOpaqueArray * array);
size_t AwaTimeArray_GetValueCount      (const AwaTimeArray * array);
size_t AwaObjectLinkArray_GetValueCount(const AwaObjectLinkArray * array);
/** @} */

/**
 * @defgroup Array_NewArrayIterator Awa___Array_New___ArrayIterator
 * @addtogroup Array_NewArrayIterator
 * @brief Allocate and return a pointer to a new Array Iterator of the specified type,
 *        that can be used to walk through values inside the array.
 *        The Array Iterator is owned by the caller and should eventually be freed with Awa___ArrayIterator_Free.
 * @param[in] array A const pointer to a valid resource array of the specified type.
 * @return the newly allocated Array Iterator on success.
 * @return NULL on failure.
 * @{
 */
AwaCStringArrayIterator *    AwaStringArray_NewCStringArrayIterator       (const AwaStringArray * array);
AwaIntegerArrayIterator *    AwaIntegerArray_NewIntegerArrayIterator      (const AwaIntegerArray * array);
AwaFloatArrayIterator *      AwaFloatArray_NewFloatArrayIterator          (const AwaFloatArray * array);
AwaBooleanArrayIterator *    AwaBooleanArray_NewBooleanArrayIterator      (const AwaBooleanArray * array);
AwaOpaqueArrayIterator *     AwaOpaqueArray_NewOpaqueArrayIterator        (const AwaOpaqueArray * array);
AwaTimeArrayIterator *       AwaTimeArray_NewTimeArrayIterator            (const AwaTimeArray * array);
AwaObjectLinkArrayIterator * AwaObjectLinkArray_NewObjectLinkArrayIterator(const AwaObjectLinkArray * array);
/** @} */


/**
 * @defgroup Array_IsValid Awa___Array_IsValid
 * @addtogroup Array_IsValid
 * @brief Determine whether a resource instance at the specified index of an array is valid.
 * @param[in] array A pointer to a valid resource array of the specified type.
 * @param[in] index The index of the resource instance to retrieve.
 * @return true if the input array is valid and the resource instance at the specified index is not NULL
 * @return false if the input array is invalid, the index is out of range, or the value at the specified index is NULL.
 * @{
 */
bool AwaStringArray_IsValid    (const AwaStringArray * array,     AwaArrayIndex index);
bool AwaIntegerArray_IsValid   (const AwaIntegerArray * array,    AwaArrayIndex index);
bool AwaFloatArray_IsValid     (const AwaFloatArray * array,      AwaArrayIndex index);
bool AwaBooleanArray_IsValid   (const AwaBooleanArray * array,    AwaArrayIndex index);
bool AwaOpaqueArray_IsValid    (const AwaOpaqueArray * array,     AwaArrayIndex index);
bool AwaTimeArray_IsValid      (const AwaTimeArray * array,       AwaArrayIndex index);
bool AwaObjectLinkArray_IsValid(const AwaObjectLinkArray * array, AwaArrayIndex index);
/** @} */

// TODO: Array appending can be handled by the high-level API

/**
 * @defgroup ArrayIterator_Next Awa___ArrayIterator_Next
 * @addtogroup ArrayIterator_Next
 * @brief Check whether or not the iterator has not reached the end of reading its array's values.
 *        If the iterator has not reached the end, advance its position to the next resource instance in the array.
 * @param[in] iterator A pointer to a valid Array Iterator of the specified type.
 * @return true if the input Array Iterator is valid and the iterator has not yet reached the end of its array.
 * @return false if the input Array Iterator is invalid, or has reached the end of its array.
 * @{
 */
bool AwaCStringArrayIterator_Next   (AwaCStringArrayIterator *    iterator);
bool AwaIntegerArrayIterator_Next   (AwaIntegerArrayIterator *    iterator);
bool AwaFloatArrayIterator_Next     (AwaFloatArrayIterator *      iterator);
bool AwaBooleanArrayIterator_Next   (AwaBooleanArrayIterator *    iterator);
bool AwaOpaqueArrayIterator_Next    (AwaOpaqueArrayIterator *     iterator);
bool AwaTimeArrayIterator_Next      (AwaTimeArrayIterator *       iterator);
bool AwaObjectLinkArrayIterator_Next(AwaObjectLinkArrayIterator * iterator);
/** @} */

/**
 * @defgroup ArrayIterator_GetIndex Awa___ArrayIterator_GetIndex
 * @addtogroup ArrayIterator_GetIndex
 * @brief Return the current index of the resource instance of the specified Array Iterator.
 * @param[in] iterator A pointer to a valid Array Iterator of the specified type.
 * @return the current index of the Array Iterator if the input Array Iterator is valid.
 * @return 0 if the input Array Iterator is invalid.
 * @{
 */
AwaArrayIndex AwaCStringArrayIterator_GetIndex   (const AwaCStringArrayIterator *     iterator);
AwaArrayIndex AwaIntegerArrayIterator_GetIndex   (const AwaIntegerArrayIterator *    iterator);
AwaArrayIndex AwaFloatArrayIterator_GetIndex     (const AwaFloatArrayIterator *      iterator);
AwaArrayIndex AwaBooleanArrayIterator_GetIndex   (const AwaBooleanArrayIterator *    iterator);
AwaArrayIndex AwaOpaqueArrayIterator_GetIndex    (const AwaOpaqueArrayIterator *     iterator);
AwaArrayIndex AwaTimeArrayIterator_GetIndex      (const AwaTimeArrayIterator *       iterator);
AwaArrayIndex AwaObjectLinkArrayIterator_GetIndex(const AwaObjectLinkArrayIterator * iterator);
/** @} */

/**
 * @defgroup ArrayIterator_GetValue Awa___ArrayIterator_GetValue
 * @addtogroup ArrayIterator_GetValue
 * @brief Return the value of the resource instance at the current index of the array contained in the input Array Iterator.
 * @param[in] iterator A pointer to a valid Array Iterator of the specified type.
 * @return the current index of the Array Iterator if the input Array Iterator is valid.
 * @return NULL if the input Array Iterator is invalid, or the iterator has reached the end of its array.
 * @{
 */
// RESERVED for UTF8 support:
//AwaString   AwaStringArrayIterator_GetValue          (const AwaStringArrayIterator * iterator);
const char *  AwaCStringArrayIterator_GetValueAsCString(const AwaCStringArrayIterator * iterator);
AwaInteger    AwaIntegerArrayIterator_GetValue         (const AwaIntegerArrayIterator * iterator);
AwaFloat      AwaFloatArrayIterator_GetValue           (const AwaFloatArrayIterator * iterator);
AwaBoolean    AwaBooleanArrayIterator_GetValue         (const AwaBooleanArrayIterator * iterator);
AwaOpaque     AwaOpaqueArrayIterator_GetValue          (const AwaOpaqueArrayIterator * iterator);
AwaTime       AwaTimeArrayIterator_GetValue            (const AwaTimeArrayIterator * iterator);
AwaObjectLink AwaObjectLinkArrayIterator_GetValue      (const AwaObjectLinkArrayIterator * iterator);
/** @} */

/**
 * @defgroup ArrayIterator_Free Awa___ArrayIterator_Free
 * @addtogroup ArrayIterator_Free
 * @brief Clean up an Array Iterator of the specified type, freeing all allocated resources.
 *        Once freed, the Array Iterator is no longer valid.
 * @param[in,out] Array Iterator A pointer to a Array Iterator pointer of the specified type that will be set to NULL.
 * @{
 */
void AwaCStringArrayIterator_Free   (AwaCStringArrayIterator ** iterator);
void AwaIntegerArrayIterator_Free   (AwaIntegerArrayIterator ** iterator);
void AwaFloatArrayIterator_Free     (AwaFloatArrayIterator ** iterator);
void AwaBooleanArrayIterator_Free   (AwaBooleanArrayIterator ** iterator);
void AwaOpaqueArrayIterator_Free    (AwaOpaqueArrayIterator ** iterator);
void AwaTimeArrayIterator_Free      (AwaTimeArrayIterator ** iterator);
void AwaObjectLinkArrayIterator_Free(AwaObjectLinkArrayIterator ** iterator);
/** @} */

/**************************************************************************************************
 * Definition Functions common to Server and Client
 *************************************************************************************************/

typedef struct _ObjectDefinition AwaObjectDefinition;
typedef struct _ResourceDefinition AwaResourceDefinition;
typedef struct _AwaObjectDefinitionIterator AwaObjectDefinitionIterator;
typedef struct _AwaResourceDefinitionIterator AwaResourceDefinitionIterator;

/**
 * @brief Define object metadata against an Object ID. This is required before resources can
 *        be defined against an Object ID.
 * @param[in] objectID The numeric object ID to define.
 * @param[in] objectName A descriptive name that is used to describe the Object.
 * @param[in] minimumInstances Minimum number of instances required. If greater than zero, the object is mandatory.
 * @param[in] maximumInstances The maximum number of instances allowed. If 1, the object is single-instance. Maximum is AWA_MAX_ID.
 * @return A pointer to a new ObjectDefinition instance. This pointer should not be freed. Return NULL on failure.
 */
AwaObjectDefinition * AwaObjectDefinition_New(AwaObjectID objectID, const char * objectName, int minimumInstances, int maximumInstances);

/**
 * @brief Clean up an Object Definition, freeing all allocated resources.
 *        Once freed, the definition is no longer valid.
 * @param[in,out] objectDefinition A pointer to an Object Definition pointer that will be set to NULL.
 */
void AwaObjectDefinition_Free(AwaObjectDefinition ** objectDefinition);

/**
 * @brief Add a Resource Definition to an existing Object Definition.
 *        Paths are not used here as Object Instance IDs are not relevant.
 * @param[in] objectDefinition A pointer to an Object Definition to add the Resource Definition to.
 * @param[in] resourceID The numeric resource ID to define.
 * @param[in] resourceName A descriptive name that is used to describe the Resource.
 * @param[in] isMandatory whether or not object instances must contain this resource.
 *        If not, this is an optional resource which must be created within its object instance before being used.
 * @param[in] operations The valid operations allowed on this resource.
 * @return AwaError_Success on success.
 * @return AwaError_DefinitionInvalid if the Object Definition is not valid.
 * @return AwaError_AlreadyDefined if a Resource Definition already exists for the Object Definition with the same resourceID.
 * @return AwaError_IDInvalid if resourceID is negative or larger than AWA_MAX_ID.
 * @return AwaError_OutOfMemory if there is not enough memory to create the Resource Definition.
 */
AwaError AwaObjectDefinition_AddResourceDefinitionAsNoType(AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, bool isMandatory, AwaResourceOperations operations);

/**
 * @defgroup ObjectDefinition_AddResourceDefinition AwaObjectDefinition_AddResourceDefinitionAs___
 * @addtogroup ObjectDefinition_AddResourceDefinition
 * @brief Add a Resource Definition to an existing Object Definition.
 *        Paths are not used here as Object Instance IDs are not relevant.
 * @param[in] objectDefinition A pointer to an Object Definition to add the Resource Definition to.
 * @param[in] resourceID The numeric resource ID to define.
 * @param[in] resourceName A descriptive name that is used to describe the Resource.
 * @param[in] isMandatory whether or not object instances must contain this resource.
 *        If not, this is an optional resource which must be created within its object instance before being used.
 * @param[in] operations The valid operations allowed on this resource.
 * @param[in] default The default value for created resources, and the value which a mandatory resource will be set to
 *        if a delete operation is called upon the resource.
 * @return AwaError_Success on success.
 * @return AwaError_DefinitionInvalid if the Object Definition is not valid.
 * @return AwaError_AlreadyDefined if a Resource Definition already exists for the Object Definition with the same resourceID.
 * @return AwaError_IDInvalid if resourceID is negative or larger than AWA_MAX_ID.
 * @return AwaError_OutOfMemory if there is not enough memory to create the Resource Definition.
 * @{
 */
AwaError AwaObjectDefinition_AddResourceDefinitionAsString    (AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, bool isMandatory, AwaResourceOperations operations, const char * defaultValue);
AwaError AwaObjectDefinition_AddResourceDefinitionAsInteger   (AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, bool isMandatory, AwaResourceOperations operations, AwaInteger defaultValue);
AwaError AwaObjectDefinition_AddResourceDefinitionAsFloat     (AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, bool isMandatory, AwaResourceOperations operations, AwaFloat defaultValue);
AwaError AwaObjectDefinition_AddResourceDefinitionAsBoolean   (AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, bool isMandatory, AwaResourceOperations operations, AwaBoolean defaultValue);
AwaError AwaObjectDefinition_AddResourceDefinitionAsOpaque    (AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, bool isMandatory, AwaResourceOperations operations, AwaOpaque defaultValue);
AwaError AwaObjectDefinition_AddResourceDefinitionAsTime      (AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, bool isMandatory, AwaResourceOperations operations, AwaTime defaultValue);
AwaError AwaObjectDefinition_AddResourceDefinitionAsObjectLink(AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, bool isMandatory, AwaResourceOperations operations, AwaObjectLink defaultValue);
/** @} */

/**
 * @defgroup ObjectDefinition_AddResourceDefinitionAsArray AwaObjectDefinition_AddResourceDefinitionAs___Array
 * @addtogroup ObjectDefinition_AddResourceDefinitionAsArray
 * @brief Add a resource array definition to an existing Object Definition.
 *        Paths are not used here as Object Instance IDs are not relevant.
 * @param[in] objectDefinition A pointer to an Object Definition to add the Resource Definition to.
 * @param[in] resourceID The numeric resource ID to define.
 * @param[in] resourceName A descriptive name that is used to describe the Resource.
 * @param[in] minimumInstances the minimum resource instances or array elements for the resource array.
 *        If minimumInstances is greater than zero, it is a mandatory resource.
 *        If minimumInstances is zero, this is an optional resource which must be created within its object instance before being used.
 * @param[in] maximumInstances the maximum resource instances or array elements for the resource array.
 * @param[in] operations The valid operations allowed on this resource.
 * @param[in] defaultArray The default value for created resource arrays, and the value which a mandatory resource will be set to
 *        if a delete operation is called upon the entire resource array.
 * @return AwaError_Success on success.
 * @return AwaError_DefinitionInvalid if the Object Definition is not valid.
 * @return AwaError_AlreadyDefined if a Resource Definition already exists for the Object Definition with the same resourceID.
 * @return AwaError_IDInvalid if resourceID is negative or larger than AWA_MAX_ID.
 * @return AwaError_OutOfMemory if there is not enough memory to create the Resource Definition.
 * @{
 */
AwaError AwaObjectDefinition_AddResourceDefinitionAsStringArray    (AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, int minimumInstances, int maximumInstances, AwaResourceOperations operations, const AwaStringArray * defaultArray);
AwaError AwaObjectDefinition_AddResourceDefinitionAsIntegerArray   (AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, int minimumInstances, int maximumInstances, AwaResourceOperations operations, const AwaIntegerArray * defaultArray);
AwaError AwaObjectDefinition_AddResourceDefinitionAsFloatArray     (AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, int minimumInstances, int maximumInstances, AwaResourceOperations operations, const AwaFloatArray * defaultArray);
AwaError AwaObjectDefinition_AddResourceDefinitionAsBooleanArray   (AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, int minimumInstances, int maximumInstances, AwaResourceOperations operations, const AwaBooleanArray * defaultArray);
AwaError AwaObjectDefinition_AddResourceDefinitionAsTimeArray      (AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, int minimumInstances, int maximumInstances, AwaResourceOperations operations, const AwaTimeArray * defaultArray);
AwaError AwaObjectDefinition_AddResourceDefinitionAsOpaqueArray    (AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, int minimumInstances, int maximumInstances, AwaResourceOperations operations, const AwaOpaqueArray * defaultArray);
AwaError AwaObjectDefinition_AddResourceDefinitionAsObjectLinkArray(AwaObjectDefinition * objectDefinition, AwaResourceID resourceID, const char * resourceName, int minimumInstances, int maximumInstances, AwaResourceOperations operations, const AwaObjectLinkArray * defaultArray);
/** @} */

/**
 * @brief Retrieve the unique ID of an Object Definition.
 * @param[in] objectDefinition A pointer to a valid Object Definition.
 * @return the ID of the Object Definition if it is valid.
 * @return AWA_INVALID_ID if the Object Definition is invalid.
 */
AwaObjectID AwaObjectDefinition_GetID (const AwaObjectDefinition * objectDefinition);

/**
 * @brief Retrieve the name of an Object Definition.
 * @param[in] objectDefinition A pointer to an Object Definition.
 * @return the name of the Object Definition if it is valid.
 * @return NULL if the Object Definition is invalid.
 */
const char * AwaObjectDefinition_GetName (const AwaObjectDefinition * objectDefinition);

/**
 * @brief Retrieve the minimum number of instances which can be created for an Object Definition.
 * @param[in] objectDefinition A pointer to an Object Definition.
 * @return the minimum number of instances of the Object Definition if it is valid.
 * @return -1 if the Object Definition is invalid.
 */
int AwaObjectDefinition_GetMinimumInstances (const AwaObjectDefinition * objectDefinition);

/**
 * @brief Retrieve the maximum number of instances which can be created for an Object Definition.
 * @param[in] objectDefinition A pointer to a valid Object Definition.
 * @return the maximum number of instances of the Object Definition if it is valid.
 * @return -1 if the Object Definition is invalid.
 */
int AwaObjectDefinition_GetMaximumInstances (const AwaObjectDefinition * objectDefinition);

/**
 * @brief Determine whether a Resource Definition exists within an Object Definition for the specified ID.
 * @param[in] objectDefinition A pointer to a valid Object Definition.
 * @param[in] resourceID The numeric resource ID.
 * @return true if the Object Definition is valid, and it contains a Resource Definition with the specified resourceID.
 * @return false if the Object Definition is invalid, or it does not contain a Resource Definition with the specified resourceID.
 */
bool AwaObjectDefinition_IsResourceDefined (const AwaObjectDefinition * objectDefinition, AwaResourceID resourceID);

/**
 * @brief Retrieve a Resource Definition from an Object Definition using a resource ID.
 * @param[in] objectDefinition A pointer to a valid Object Definition.
 * @param[in] resourceID The numeric resource ID of the Resource Definition to retrieve.
 * @return the Resource Definition that matches the specified resourceID, if the Object Definition is valid,
 *        and the Resource Definition exists.
 * @return NULL if the Object Definition is invalid, or it does not contain a Resource Definition with the specified resourceID.
 */
const AwaResourceDefinition * AwaObjectDefinition_GetResourceDefinition(const AwaObjectDefinition * objectDefinition, AwaResourceID resourceID);

/**
 * @brief Determine whether the Object Definition iterator has not reached the end of a set of Object Definitions.
 *        If the iterator has not reached the end, advance its position to the next Object Definition.
 * @param[in] iterator A pointer to a valid Object Definition iterator.
 * @return true if the Object Definition iterator is valid and the iterator has not yet reached the end of its elements.
 * @return false if the Object Definition iterator is invalid, or has reached the end of its elements.
 */
bool AwaObjectDefinitionIterator_Next(AwaObjectDefinitionIterator * iterator);

/**
 * @brief Retrieve the current Object Definition from an Object Definition iterator.
 * @param[in] iterator A pointer to a valid Object Definition iterator.
 * @return the current Object Definition of the Object Definition iterator if it is valid
 * @return NULL if the Object Definition iterator is invalid.
 */
const AwaObjectDefinition * AwaObjectDefinitionIterator_Get(const AwaObjectDefinitionIterator * iterator);

/**
 * @brief Clean up an Object Definition iterator, freeing all allocated resources.
 *        Once freed, the iterator is no longer valid.
 * @param[in,out] iterator A pointer to an Object Definition iterator pointer that will be set to NULL.
 */
void AwaObjectDefinitionIterator_Free(AwaObjectDefinitionIterator ** iterator);

/**
 * @brief Retrieve the unique ID of a Resource Definition.
 * @param[in] resourceDefinition A pointer to a valid Resource Definition.
 * @return the ID of the Resource Definition if it is valid.
 * @return AWA_INVALID_ID if the Resource Definition is invalid.
 */
AwaResourceID AwaResourceDefinition_GetID(const AwaResourceDefinition * resourceDefinition);

/**
 * @brief Retrieve the resource type of a Resource Definition.
 * @param[in] resourceDefinition A pointer to a valid Resource Definition.
 * @return the type of the Resource Definition if it is valid.
 * @return AwaResourceType_Invalid if the Resource Definition is invalid.
 */
AwaResourceType AwaResourceDefinition_GetType(const AwaResourceDefinition * resourceDefinition);

/**
 * @brief Retrieve the name of a Resource Definition.
 * @param[in] resourceDefinition A pointer to a valid Resource Definition.
 * @return the name of the Resource Definition if it is valid.
 * @return NULL if the Resource Definition is invalid.
 */
const char * AwaResourceDefinition_GetName(const AwaResourceDefinition * resourceDefinition);

/**
 * @brief Retrieve the minimum number of instances which can be created for an Resource Definition.
 * @param[in] resourceDefinition A pointer to an Resource Definition.
 * @return the minimum number of instances of the Resource Definition if it is valid.
 * @return -1 if the Resource Definition is invalid.
 */
int AwaResourceDefinition_GetMinimumInstances(const AwaResourceDefinition * resourceDefinition);

/**
 * @brief Retrieve the maximum number of instances which can be created for an Resource Definition.
 * @param[in] resourceDefinition A pointer to a valid Resource Definition.
 * @return the maximum number of instances of the Resource Definition if it is valid.
 * @return -1 if the Resource Definition is invalid.
 */
int AwaResourceDefinition_GetMaximumInstances(const AwaResourceDefinition * resourceDefinition);

/**
 * @brief Retrieve the supported operations for an Resource Definition.
 * @param[in] resourceDefinition A pointer to a valid Resource Definition.
 * @return the supported operations for the Resource Definition if it is valid.
 * @return AwaResourceOperations_None if the Resource Definition is invalid.
 */
AwaResourceOperations AwaResourceDefinition_GetSupportedOperations(const AwaResourceDefinition * resourceDefinition);

/**
 * @brief Determine whether or not a Resource Definition is mandatory.
 * @param[in] resourceDefinition A pointer to an Resource Definition.
 * @return true if the Resource Definition if is valid, and is mandatory.
 * @return false if the Resource Definition is invalid, or optional.
 */
bool AwaResourceDefinition_IsMandatory(const AwaResourceDefinition * resourceDefinition);

/**
 * @brief Retrieves a new Resource Definition iterator that can be used to iterate through the set
 *        of defined resources within an Object Definition.
 *        The iterator is owned by the caller and should eventually be freed with AwaResourceDefinitionIterator_Free.
 * @param[in] objectDefinition Pointer to a valid Object Definition.
 * @return A pointer to a Resource Definition iterator, if the objectDefinition is valid.
 * @return NULL if the objectDefinition is not valid.
 */
AwaResourceDefinitionIterator * AwaObjectDefinition_NewResourceDefinitionIterator(const AwaObjectDefinition * objectDefinition);

/**
 * @brief Determine whether the Resource Definition iterator has not reached the end of a set of Resource Definitions.
 *        If the iterator has not reached the end, advance its position to the next Resource Definition.
 * @param[in] iterator A pointer to a valid Resource Definition iterator.
 * @return true if the Resource Definition iterator is valid and the iterator has not yet reached the end of its elements.
 * @return false if the Resource Definition iterator is invalid, or has reached the end of its elements.
 */
bool AwaResourceDefinitionIterator_Next(AwaResourceDefinitionIterator * iterator);

/**
 * @brief Retrieve the current Resource Definition from an Resource Definition iterator.
 * @param[in] iterator A pointer to a valid Resource Definition iterator.
 * @return the current Resource Definition of the Resource Definition iterator if it is valid
 * @return NULL if the Resource Definition iterator is invalid.
 */
const AwaResourceDefinition * AwaResourceDefinitionIterator_Get(const AwaResourceDefinitionIterator * iterator);

/**
 * @brief Clean up an Resource Definition iterator, freeing all allocated resources.
 *        Once freed, the iterator is no longer valid.
 * @param[in,out] iterator A pointer to an Resource Definition iterator pointer that will be set to NULL.
 */
void AwaResourceDefinitionIterator_Free(AwaResourceDefinitionIterator ** iterator);


/**************************************************************************************************
 * Utility Functions
 *************************************************************************************************/

// Path Iterator
typedef struct _AwaPathIterator AwaPathIterator;

/**
 * @brief Determine whether the Path Iterator has not reached the end of a set of paths.
 *        If the iterator has not reached the end, advance its position to the next path.
 * @param[in] iterator A pointer to a valid Path Iterator.
 * @return true if the Path Iterator is valid and the iterator has not yet reached the end of its elements.
 * @return false if the Path Iterator is invalid, or has reached the end of its elements.
 */
bool AwaPathIterator_Next(AwaPathIterator * iterator);

/**
 * @brief Retrieve the current path from a Path Iterator.
 *        If the iterator has not reached the end, advance its position to the next path.
 * @param[in] iterator A pointer to a valid Path Iterator.
 * @return the current path of the Path Iterator if it is valid
 * @return NULL if the Path Iterator is invalid.
 */
const char * AwaPathIterator_Get(const AwaPathIterator * iterator);

/**
 * @brief Clean up a Path Iterator, freeing all allocated resources.
 *        Once freed, the iterator is no longer valid.
 * @param[in,out] iterator A pointer to a Path Iterator pointer that will be set to NULL.
 */
void AwaPathIterator_Free(AwaPathIterator ** iterator);

/**
 * @brief Construct a path from an object ID.
 * @param[in,out] path Caller-allocated destination path.
 * @param[in] pathSize the allocated size of the path.
 * @param[in] objectID the numerical object ID.
 * @return AwaError_Success on success.
 * @return AwaError_IDInvalid if the ID is invalid.
 * @return AwaError_PathInvalid if the path is NULL.
 */
AwaError AwaAPI_MakeObjectPath(char * path, size_t pathSize, AwaObjectID objectID);

/**
 * @brief Construct a path from an object and object instance ID.
 * @param[in,out] path Caller-allocated destination path.
 * @param[in] pathSize the allocated size of the path.
 * @param[in] objectID the numerical object ID.
 * @param[in] objectInstanceID the numerical object instance ID.
 * @return AwaError_Success on success.
 * @return AwaError_IDInvalid if the ID is invalid.
 * @return AwaError_PathInvalid if the path is NULL.
 */
AwaError AwaAPI_MakeObjectInstancePath(char * path, size_t pathSize, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID);

/**
 * @brief Construct a path from an object, object instance, and resource ID.
 * @param[in,out] path Caller-allocated destination path.
 * @param[in] pathSize the allocated size of the path.
 * @param[in] objectID the numerical object ID.
 * @param[in] objectInstanceID the numerical object instance ID.
 * @param[in] resourceID the numerical resource ID.
 * @return AwaError_Success on success.
 * @return AwaError_IDInvalid if the ID is invalid.
 * @return AwaError_PathInvalid if the path is NULL.
 */
AwaError AwaAPI_MakeResourcePath(char * path, size_t pathSize, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID);

/**
 * @brief Construct a path from an object, object instance, and resource ID.
 * @param[in,out] path Caller-allocated destination path.
 * @param[in] pathSize the allocated size of the path.
 * @param[in] objectID the numerical object ID.
 * @param[in] objectInstanceID the numerical object instance ID.
 * @param[in] resourceID the numerical resource ID.
 * @return AwaError_Success on success.
 * @return AwaError_IDInvalid if the ID is invalid.
 * @return AwaError_PathInvalid if the path is NULL.
 */
AwaError AwaAPI_MakePath(char * path, size_t pathSize, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID);

/**
 * @brief Check if a path string conforms to the valid path syntax.
 * @param[in] path a valid object, object instance or resource path.
 * @return true if the path is a valid /O/I/R path.
 * @return false if the path is invalid.
 */
bool AwaAPI_IsPathValid(const char * path);


// Encapsulates Execute arguments
typedef AwaOpaque AwaExecuteArguments;


/**************************************************************************************************
 * Response Functions
 *************************************************************************************************/

typedef struct _AwaPathResult AwaPathResult;

/**
 * @brief Retrieve an error from a Path Result.
 * @param[in] result A pointer to a valid Path Result.
 * @return AwaError_Success if the result contains no error.
 * @return AwaError_OperationInvalid if the result is invalid.
 * @return Various errors on a valid but unsuccessful Path Result.
 */
AwaError AwaPathResult_GetError(const AwaPathResult * result);

/**
 * @brief Retrieve a LWM2M error from a Path Result.
 * @param[in] result A pointer to a valid Path Result.
 * @return AwaLWM2MError_Success if the result contains no error.
 * @return Various errors on a valid but unsuccessful Path Result.
 */
AwaLWM2MError AwaPathResult_GetLWM2MError(const AwaPathResult * result);


/**************************************************************************************************
 * ChangeSet Functions for Change Subscriptions and Observations
 *************************************************************************************************/

typedef struct _AwaChangeSet AwaChangeSet;

/**
 * @brief Create a new Path Iterator for a ChangeSet, used to iterate through the list of resource paths
 *        retrieved by the corresponding Change Subscription or Observation.
 *        The resulting iterator is owned by the caller and should eventually be freed with AwaPathIterator_Free.
 *        This function can only be successful inside a callback of a successful change notification.
 * @param[in] changeSet A pointer to the ChangeSet to search.
 * @return A pointer to a new PathIterator instance on success.
 * @return NULL on failure.
 */
AwaPathIterator * AwaChangeSet_NewPathIterator(const AwaChangeSet * changeSet);

/**
 * @brief Retrieve the change type for a specified path in a ChangeSet.
 * @param[in] changeSet A pointer to a valid ChangeSet.
 * @param[in] path The path with which to query the Get Response.
 * @return the change type for the specified path on success.
 * @return AwaClientChangeType_Invalid on an invalid operation.
 */
AwaChangeType AwaChangeSet_GetChangeType(const AwaChangeSet * changeSet, const char * path);

/**
 * @brief Test if the ChangeSet has a value for the specified resource path.
 *        If the ChangeSet contains a value for the specified path, and the Resource Type is known,
 *        it can be retrieved with the appropriate AwaChangeSet_GetValueAs_ function.
 * @param[in] changeSet A pointer to a valid ChangeSet
 * @param[in] path The path with which to query the ChangeSet
 * @return true if the ChangeSet contains a value for the specified resource path.
 * @return false if the ChangeSet does not contain a value for the specified resource path, or if the path is invalid.
 */
bool AwaChangeSet_HasValue(const AwaChangeSet * changeSet, const char * path);

/**
 * @brief Test if the ChangeSet has the specified path (resource, object instance or object) present.
 *        If the ChangeSet contains paths beyond the specified path, the path will be considered present.
 * @param[in] changeSet A pointer to a valid ChangeSet.
 * @param[in] path The path with which to query the ChangeSet.
 * @return true if the ChangeSet contains the specified resource path.
 * @return false if the ChangeSet does not contain the specified resource path, or if the path is invalid.
 */
bool AwaChangeSet_ContainsPath(const AwaChangeSet * changeSet, const char * path);

/**
 * @brief Get the resource type for the specified change. Only valid for resource changes.
 * @param[in] changeSet A pointer to a valid ChangeSet.
 * @param[in] path The path with which to query the Get Response.
 * @return Resource type on success.
 * @return AwaResourceType_Invalid if change is not a resource change.
 */
AwaResourceType AwaChangeSet_GetResourceType(const AwaChangeSet * changeSet, const char * path);

/**
 * @defgroup AwaChangeSet_GetValueAs AwaChangeSet_GetValueAs___Pointer
 * @addtogroup AwaChangeSet_GetValueAs
 * @brief Retrieve a temporary pointer to a resource's value from a ChangeSet. The resource is identified by the path.
 * ChangeSets will contain values for any resources that have been created or modified.
 *        The pointer is only valid while within its Subscription or Observation callback. Once the callback exits the changeSet is freed
 *        and any pointers returned are immediately invalid and should no longer be used or dereferenced.
 * @param[in] changeSet The current ChangeSet to retrieve the value from.
 * @param[in] path The path of the resource requested for retrieval.
 * @param[in,out] value A pointer to a const pointer that will be modified to point to the requested value.
 * @return AwaError_Success on success.
 * @return AwaError_TypeMismatch if the resource type corresponding to the specified path is not of the correct type.
 * @return AwaError_PathNotFound if the specified resource path is not covered by the Get Response.
 * @return AwaError_OperationInvalid if the specified operation is invalid or NULL.
 * @return AwaError_PathInvalid if the specified path is invalid or does not correspond to a resource path.
 * @{
 */
AwaError AwaChangeSet_GetValueAsCStringPointer   (const AwaChangeSet * changeSet, const char * path, const char ** value);
AwaError AwaChangeSet_GetValueAsIntegerPointer   (const AwaChangeSet * changeSet, const char * path, const AwaInteger ** value);
AwaError AwaChangeSet_GetValueAsFloatPointer     (const AwaChangeSet * changeSet, const char * path, const AwaFloat ** value);
AwaError AwaChangeSet_GetValueAsBooleanPointer   (const AwaChangeSet * changeSet, const char * path, const AwaBoolean ** value);
AwaError AwaChangeSet_GetValueAsTimePointer      (const AwaChangeSet * changeSet, const char * path, const AwaTime ** value);
AwaError AwaChangeSet_GetValueAsObjectLinkPointer(const AwaChangeSet * changeSet, const char * path, const AwaObjectLink ** value);
AwaError AwaChangeSet_GetValueAsOpaquePointer    (const AwaChangeSet * changeSet, const char * path, const AwaOpaque ** value);
/** @} */

/**
 * @brief Retrieve an opaque resource's value from a ChangeSet. The resource is identified by the path.
 *        ChangeSets will contain values for any resources that have been created or modified.
 *        A pointer to a AwaOpaque struct is passed in and populated by the function.
 *        The data pointer within the AwaOpaque struct is only valid while within its Subscription or Observation callback.
 *        Once the callback exits the changeSet is freed, the data pointer is immediately invalid and should not be
 *        used or dereferenced.

 * @param[in] changeSet The ChangeSet to retrieve the value from.
 * @param[in] path The path of the opaque resource requested for retrieval.
 * @param[in,out] value A pointer to a AwaOpaque struct that will be modified to refer to the requested opaque value.
 * @return AwaError_Success on success.
 * @return AwaError_TypeMismatch if the resource type corresponding to the specified path is not of type AwaResourceType_Opaque.
 * @return AwaError_PathNotFound if the specified resource path is not covered by the Get Response.
 * @return AwaError_OperationInvalid if the specified operation is invalid or NULL.
 * @return AwaError_PathInvalid if the specified path is invalid or does not correspond to a resource path.
 */
AwaError AwaChangeSet_GetValueAsOpaque           (const AwaChangeSet * changeSet, const char * path, AwaOpaque * value);

/**
 * @brief Retrieve an object link resource's value from a ChangeSet. The resource is identified by the path.
 *        ChangeSets will contain values for any resources that have been created or modified.
 *        A pointer to a AwaObjectLink struct is passed in and populated by the function.
 *
 * @param[in] changeSet The ChangeSet to retrieve the value from.
 * @param[in] path The path of the object link resource requested for retrieval.
 * @param[in,out] value A pointer to a AwaObjectLink struct that will be modified to hold the requested object link value.
 * @return AwaError_Success on success.
 * @return AwaError_TypeMismatch if the resource type corresponding to the specified path is not of type AwaResourceType_ObjectLink.
 * @return AwaError_PathNotFound if the specified resource path is not covered by the Get Response.
 * @return AwaError_OperationInvalid if the specified operation is invalid or NULL.
 * @return AwaError_PathInvalid if the specified path is invalid or does not correspond to a resource path.
 */
AwaError AwaChangeSet_GetValueAsObjectLink       (const AwaChangeSet * changeSet, const char * path, AwaObjectLink * value);

/**
 * @defgroup ChangeSet_GetValuesAsArray AwaChangeSet_GetValuesAs___ArrayPointer
 * @addtogroup ChangeSet_GetValuesAsArray
 * @brief Retrieve a temporary pointer to a multiple-instance resource's array value from a ChangeSet. The resource is identified by the path.
 *        ChangeSets will contain values for any resources that have been created or modified.
 *        The pointer is only valid while within its Subscription or Observation callback. Once the callback exits the changeSet is freed
 *        and any pointers returned are immediately invalid and should no longer be used or dereferenced.
 *
 * @param[in] changeSet The ChangeSet to retrieve the value from.
 * @param[in] path The path of the multiple-instance resource requested for retrieval.
 * @param[in,out] valueArray A pointer to a const pointer that will be modified to point to the requested value.
 * @return AwaError_Success on success.
 * @return AwaError_TypeMismatch if the resource type corresponding to the specified path is not of the correct type.
 * @return AwaError_PathNotFound if the specified resource path is not covered by the Get Response.
 * @return AwaError_OperationInvalid if the specified operation is invalid or NULL.
 * @return AwaError_PathInvalid if the specified path is invalid or does not correspond to a resource path.
 * @{
 */
AwaError AwaChangeSet_GetValuesAsStringArrayPointer    (const AwaChangeSet * changeSet, const char * path, const AwaStringArray ** valueArray);
AwaError AwaChangeSet_GetValuesAsIntegerArrayPointer   (const AwaChangeSet * changeSet, const char * path, const AwaIntegerArray ** valueArray);
AwaError AwaChangeSet_GetValuesAsFloatArrayPointer     (const AwaChangeSet * changeSet, const char * path, const AwaFloatArray ** valueArray);
AwaError AwaChangeSet_GetValuesAsBooleanArrayPointer   (const AwaChangeSet * changeSet, const char * path, const AwaBooleanArray ** valueArray);
AwaError AwaChangeSet_GetValuesAsTimeArrayPointer      (const AwaChangeSet * changeSet, const char * path, const AwaTimeArray ** valueArray);
AwaError AwaChangeSet_GetValuesAsOpaqueArrayPointer    (const AwaChangeSet * changeSet, const char * path, const AwaOpaqueArray ** valueArray);
AwaError AwaChangeSet_GetValuesAsObjectLinkArrayPointer(const AwaChangeSet * changeSet, const char * path, const AwaObjectLinkArray ** valueArray);
/** @} */


/**************************************************************************************************
 * Logging
 *************************************************************************************************/

/**
 * @brief Set the current API log level.
 * @param[in] level New log level.
 * @return Previous log level.
 */
AwaLogLevel AwaLog_SetLevel(AwaLogLevel level);


#ifdef __cplusplus
}
#endif

#endif // AWA_COMMON_H


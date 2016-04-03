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


#ifndef SET_WRITE_OPERATION_H
#define SET_WRITE_OPERATION_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "lwm2m_tree_node.h"
#include "lwm2m_definition.h"
#include "xml.h"
#include "error.h"
#include "client_session.h"
#include "server_session.h"
#include "operation_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Supported set modes
 */
typedef enum
{
    SetArrayMode_Unspecified = 0, /**< Indicates that no set array mode has been set */
    SetArrayMode_Replace,         /**< Indicates that performing a set operation on a multiple instance resource should replace the entire array */
    SetArrayMode_Update,          /**< Indicates that performing a set operation on a multiple instance resource should only update resource instances that have changed */
    SetArrayMode_LAST,            /**< Sentinel */
} SetArrayMode;

/**
 * @brief Add a value to an existing resource node returned from the process of building a tree from one or more paths
 * @param[in] propertyNode The resource node of an Objects tree to add the value to
 * @param[in] resourceInstanceID the ID of the value we are adding
 * @param[in] value The value to store in the node
 * @return InternalError_Success if the value is added successfully
 * @return InternalError_Tree if the resource node is invalid or the resource instance value node cannot be created.
 */
InternalError SetWriteCommon_AddValueToResourceNode(TreeNode propertyNode, int resourceInstanceID, const char * value);

/**
 * @brief Set a value to a single instance resource node returned from the process of building a tree from one or more paths
 * @param[in] propertyNode The resource node of an Objects tree to set the value to
 * @param[in] value The value to store in the node
 * @return InternalError_Success if the value is added successfully
 * @return InternalError_Tree if the resource node is invalid or the value node cannot be created.
 */
InternalError SetWriteCommon_SetResourceNodeValue(TreeNode resourceNode, const char * value);

AwaError SetWriteCommon_AddValue(OperationCommon * operation, SessionType sessionType, const char * path, int resourceInstanceID, void * value, size_t size, AwaResourceType type, SetArrayMode setArrayMode);

// Encode a value into a string to be copied into a LWM2M Tree Node.
// The encoded value that is returned must be explicitly freed by the caller.
char * SetWriteCommon_EncodeValue(const void * value, size_t size, AwaResourceType internalResourceType);

#ifdef __cplusplus
}
#endif

#endif // SET_WRITE_OPERATION_H


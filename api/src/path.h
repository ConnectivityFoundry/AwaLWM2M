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


#ifndef PATH_H
#define PATH_H

#include <stdbool.h>

#include "awa/common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PATH_LENGTH (256)

bool Path_IsIDValid(int ID);

bool Path_IsValid(const char * path);

/**
 * @brief Determine if specified path is of the right format to represent an object.
 *        Note that this does not check that the path represents a valid object,
 *        only that it conforms with the expected format of "/O".
 * @param[in] path Specified path to test.
 * @return true if path is valid for an object location.
 * @return false if path is invalid for an object location.
 */
bool Path_IsValidForObject(const char * path);

/**
 * @brief Determine if specified path is of the right format to represent an object instance.
 *        Note that this does not check that the path represents a valid object instance,
 *        only that it conforms with the expected format of "/O/I".
 * @param[in] path Specified path to test.
 * @return true if path is valid for an object instance location.
 * @return false if path is invalid for an object instance location.
 */
bool Path_IsValidForObjectInstance(const char * path);

/**
 * @brief Determine if specified path is of the right format to represent a resource.
 *        Note that this does not check that the path represents a valid resource,
 *        only that it conforms with the expected format of "/O/I/R".
 * @param[in] path Specified path to test.
 * @return true if path is valid for a resource location.
 * @return false if path is invalid for a resource location.
 */
bool Path_IsValidForResource(const char * path);

/**
 * @return true if path is valid
 * @return false if path is not valid
 */
bool Path_Parse(const char * path, int * matches, AwaObjectID * objectID, AwaObjectInstanceID * objectInstanceID, AwaResourceID * resourceID);

AwaObjectID Path_GetObjectID(const char * path);
AwaObjectInstanceID Path_GetObjectInstanceID(const char * path);
AwaResourceID Path_GetResourceID(const char * path);

AwaError Path_MakePath(char * path, size_t pathSize, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID);

#ifdef __cplusplus
}
#endif

#endif // PATH_H


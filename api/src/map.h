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


#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _MapType MapType;
typedef void (*MapForEachFunction)(const char * key, void * value, void * context);

/**
 * @brief Create and initialise a new map type.
 * @return a new map type.
 */
MapType * Map_New(void);

/**
 * @brief Set a value to a key in the specified map. Any existing value mapped to
 *        the key will be replaced without being freed.
 * @param[in] map
 * @param[in] key
 * @param[in] value
 * @return true if the value was successfully set
 * @return false if the map, key or value are invalid.
 */
bool Map_Put(MapType * map, const char * key, void * value);

/**
 * @brief Retrieve the value mapped to a key in the specified map.
 * @param[in] map
 * @param[in] key
 * @param[in,out] value
 * @return true if a value exists for the specified key
 * @return false if the map, key or value are invalid, or no value exists in the map for the specified key.
 */
bool Map_Get(MapType * map, const char * key, void ** value);

/**
 * @brief Determine whether a value exists for a key in the specified map.
 * @param[in] map
 * @param[in] key
 * @return true if a value exists for the specified key
 * @return false if the map, key or value are invalid, or no value exists in the map for the specified key.
 */
bool Map_Contains(MapType * map, const char * key);

/**
 * @brief Remove a key-value pair from the specified map.
 * @param[in] map
 * @param[in] key
 * @return the removed value if a value exists for the specified key, otherwise NULL
 */
void * Map_Remove(MapType * map, const char * key);

/**
 * @brief Remove all key-value pairs from the specified map. Values will not be freed.
 * @param[in] map
 */
void Map_Flush(MapType * map);

/**
 * @brief Flush and free the specified map.
 * @param[in] map
 */
void Map_Free(MapType ** map);

/**
 * @brief Free all values within the specified map, then flush the map.
 * @param[in] map
 */
void Map_FreeValues(MapType * map);

/**
 * @brief Remove a key-value pair from the specified map.
 * @param[in] map
 * @param[in] key
 * @return the number of key-value pairs in the map, or 0 if the map is invalid.
 */
size_t Map_Length(MapType * map);

/**
 * Note: callback should not modify the map
 */
size_t Map_ForEach(MapType * map, MapForEachFunction callback, void * context);

#ifdef __cplusplus
}
#endif

#endif // MAP_H


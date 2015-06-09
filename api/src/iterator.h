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


#ifndef ITERATOR_H
#define ITERATOR_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Iterator Iterator;

/**
 * @brief Construct a new iterator instance.
 * @return Pointer to newly allocated iterator instance.
 */
Iterator * Iterator_New(void);

/**
 * @brief Free all void pointer values held by iterator, but do not free the Iterator.
 * @param[in] iterator Pointer to the iterator instance.
 */
void Iterator_FreeValues(Iterator * iterator);

/**
 * @brief Free an existing iterator instance and all internal data.
 *        NOTE: values are not freed! Use Iterator_FreeValues() or free them externally.
 * @param[in] iterator Pointer to the pointer to the iterator instance.
 */
void Iterator_Free(Iterator ** iterator);

/**
 * @brief Add a new value pointer to an existing iterator instance.
 * @param[in] iterator Pointer to the iterator instance.
 * @param[in] value Void pointer to value.
 */
void Iterator_Add(Iterator * iterator, void * value);

/**
 * @brief Advance the iterator to the next state, or return false if at the last state when called.
 * @param[in] iterator Pointer to the iterator instance.
 * @return True if iterator has advanced.
 * @return False if iterator is invalid, or cannot advance to the next state.
 */
bool Iterator_Next(Iterator * iterator);

/**
 * @brief Retrieve the value as a void pointer at the current iterator state.
 * @param[in] iterator Pointer to the iterator instance.
 * @return The void pointer associated with the iterator's current state.
 */
void * Iterator_Get(const Iterator * iterator);


#ifdef __cplusplus
}
#endif

#endif // ITERATOR_H


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


#ifndef ARRAYS_H
#define ARRAYS_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _AwaArray AwaArray;
typedef struct _AwaArrayIterator AwaArrayIterator;
typedef struct _ArrayItem ArrayItem;

AwaArray * AwaArray_New(void);
void AwaArray_Free(AwaArray ** array, AwaResourceType resourceType);
void AwaArray_SetContext(AwaArray * array, void * context);
void * AwaArray_GetContext(AwaArray * array);
ArrayItem * Array_SetValue(AwaArray * array, AwaArrayIndex index, void * value, size_t valueLength);
void * Array_GetValue(const AwaArray * array, AwaArrayIndex index);
ArrayItem * Array_GetArrayItem(const AwaArray * array, AwaArrayIndex index);
int Array_GetValueLength(const AwaArray * array, AwaArrayIndex index);
void Array_DeleteItem(AwaArray * array, AwaArrayIndex index, AwaResourceType resourceType);
size_t Array_GetValueCount(const AwaArray * array);
int Array_Compare(AwaArray * array1, AwaArray * array2, AwaResourceType resourceType);

AwaArrayIterator * ArrayIterator_New(const AwaArray * array);
bool ArrayIterator_Next(AwaArrayIterator * iterator);
ArrayItem * ArrayIterator_GetItem(const AwaArrayIterator * iterator);
AwaArrayIndex ArrayIterator_GetIndex(const AwaArrayIterator * iterator);
void * ArrayIterator_GetValue(const AwaArrayIterator * iterator);
size_t ArrayIterator_GetValueLength(const AwaArrayIterator * iterator);
void ArrayIterator_Free(AwaArrayIterator ** iterator);

#ifdef __cplusplus
}
#endif

#endif

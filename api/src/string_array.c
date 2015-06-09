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


#include "awa/common.h"
#include "log.h"
#include "arrays.h"

typedef struct _AwaArray _AwaStringArray;
typedef struct _AwaArrayIterator _AwaStringArrayIterator;

AwaStringArray * AwaStringArray_New(void)
{
    return (AwaStringArray *)AwaArray_New();
}

void AwaStringArray_Free(AwaStringArray ** array)
{
    AwaArray_Free((AwaArray **)array);
}

void AwaStringArray_SetValueAsCString(AwaStringArray * array, AwaArrayIndex index, const char * value)
{
    Array_SetValue((AwaArray *)array, index, (void *)value, strlen(value) + 1);
}

void AwaStringArray_DeleteValue(AwaStringArray * array, AwaArrayIndex index)
{
    Array_DeleteItem((AwaArray *)array, index);
}

const char * AwaStringArray_GetValueAsCString(const AwaStringArray * array, AwaArrayIndex index)
{
    return Array_GetValue((const AwaArray *)array, index);
}

size_t AwaStringArray_GetValueCount(const AwaStringArray * array)
{
    return Array_GetValueCount((const AwaArray *)array);
}

AwaStringArrayIterator * AwaStringArray_NewStringArrayIterator(const AwaStringArray * array)
{
    return (AwaStringArrayIterator *)ArrayIterator_New((const AwaArray *)array);
}

bool AwaStringArray_IsValid(const AwaStringArray * array, AwaArrayIndex index)
{
    return (Array_GetArrayItem((const AwaArray *)array, index) == NULL) ? false : true;
}

void AwaStringArrayIterator_Free(AwaStringArrayIterator ** iterator)
{
    ArrayIterator_Free((AwaArrayIterator **)iterator);
}

AwaArrayIndex AwaStringArrayIterator_GetIndex(const AwaStringArrayIterator * iterator)
{
    return ArrayIterator_GetIndex((const AwaArrayIterator *)iterator);
}

bool AwaStringArrayIterator_Next(AwaStringArrayIterator * iterator)
{
    return ArrayIterator_Next((AwaArrayIterator *)iterator);
}

const char * AwaStringArrayIterator_GetValueAsCString(const AwaStringArrayIterator * iterator)
{
    return ArrayIterator_GetValue((AwaArrayIterator *)iterator);
}


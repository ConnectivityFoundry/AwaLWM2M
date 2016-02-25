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

typedef struct _AwaArray _AwaFloatArray;
typedef struct _AwaArrayIterator _AwaFloatArrayIterator;

AwaFloatArray * AwaFloatArray_New(void)
{
    return (AwaFloatArray *)AwaArray_New();
}

void AwaFloatArray_Free(AwaFloatArray ** array)
{
    AwaArray_Free((AwaArray **)array, AwaResourceType_FloatArray);
}

void AwaFloatArray_SetValue(AwaFloatArray * array, AwaArrayIndex index, AwaFloat value)
{
    Array_SetValue((AwaArray *)array, index, (void *)&value, sizeof(AwaFloat));
}

void AwaFloatArray_DeleteValue(AwaFloatArray * array, AwaArrayIndex index)
{
    Array_DeleteItem((AwaArray *)array, index, AwaResourceType_FloatArray);
}

AwaFloat AwaFloatArray_GetValue(const AwaFloatArray * array, AwaArrayIndex index)
{
    AwaFloat value = 0;
    AwaFloat * valuePtr = NULL;

    valuePtr = Array_GetValue((AwaArray *)array, index);

    if (valuePtr != NULL)
    {
        value = *valuePtr;
    }

    return value;
}

size_t AwaFloatArray_GetValueCount(const AwaFloatArray * array)
{
    return Array_GetValueCount((AwaArray *)array);
}

AwaFloatArrayIterator * AwaFloatArray_NewFloatArrayIterator(const AwaFloatArray * array)
{
    return (AwaFloatArrayIterator*)ArrayIterator_New((AwaArray*)array);
}

bool AwaFloatArray_IsValid(const AwaFloatArray * array, AwaArrayIndex index)
{
    return (Array_GetArrayItem((AwaArray *)array, index) == NULL) ? false : true;
}

void AwaFloatArrayIterator_Free(AwaFloatArrayIterator ** iterator)
{
    ArrayIterator_Free((AwaArrayIterator **)iterator);
}

AwaArrayIndex AwaFloatArrayIterator_GetIndex(const AwaFloatArrayIterator * iterator)
{
    return ArrayIterator_GetIndex((AwaArrayIterator *)iterator);
}

bool AwaFloatArrayIterator_Next(AwaFloatArrayIterator * iterator)
{
    return ArrayIterator_Next((AwaArrayIterator *)iterator);
}

AwaFloat AwaFloatArrayIterator_GetValue(const AwaFloatArrayIterator * iterator)
{
    AwaFloat value = 0;
    AwaFloat * valuePtr = NULL;

    valuePtr = ArrayIterator_GetValue((AwaArrayIterator *)iterator);

    if(valuePtr)
    {
        value = *valuePtr;
    }

    return value;
}


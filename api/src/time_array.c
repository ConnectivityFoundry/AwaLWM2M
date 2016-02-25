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

typedef struct _AwaArray _AwaTimeArray;
typedef struct _AwaArrayIterator _AwaTimeArrayIterator;

AwaTimeArray * AwaTimeArray_New(void)
{
    return (AwaTimeArray *)AwaArray_New();
}

void AwaTimeArray_Free(AwaTimeArray ** array)
{
    AwaArray_Free((AwaArray **)array, AwaResourceType_TimeArray);
}

void AwaTimeArray_SetValue(AwaTimeArray * array, AwaArrayIndex index, AwaTime value)
{
    Array_SetValue((AwaArray *)array, index, (void *)&value, sizeof(AwaTime));
}

void AwaTimeArray_DeleteValue(AwaTimeArray * array, AwaArrayIndex index)
{
    Array_DeleteItem((AwaArray *)array, index, AwaResourceType_TimeArray);
}

AwaTime AwaTimeArray_GetValue(const AwaTimeArray * array, AwaArrayIndex index)
{
    AwaTime value = 0;
    AwaTime * valuePtr = NULL;

    valuePtr = Array_GetValue((const AwaArray *)array, index);

    if (valuePtr != NULL)
    {
        value = *valuePtr;
    }

    return value;
}

size_t AwaTimeArray_GetValueCount(const AwaTimeArray * array)
{
    return Array_GetValueCount((const AwaArray *)array);
}

AwaTimeArrayIterator * AwaTimeArray_NewTimeArrayIterator(const AwaTimeArray * array)
{
    return (AwaTimeArrayIterator *)ArrayIterator_New((AwaArray *)array);
}

bool AwaTimeArray_IsValid(const AwaTimeArray * array, AwaArrayIndex index)
{
    return (Array_GetArrayItem((const AwaArray *)array, index) == NULL) ? false : true;
}

void AwaTimeArrayIterator_Free(AwaTimeArrayIterator ** iterator)
{
    ArrayIterator_Free((AwaArrayIterator **)iterator);
}

AwaArrayIndex AwaTimeArrayIterator_GetIndex(const AwaTimeArrayIterator * iterator)
{
    return ArrayIterator_GetIndex((const AwaArrayIterator *)iterator);
}

bool AwaTimeArrayIterator_Next(AwaTimeArrayIterator * iterator)
{
    return ArrayIterator_Next((AwaArrayIterator *)iterator);
}

AwaTime AwaTimeArrayIterator_GetValue(const AwaTimeArrayIterator * iterator)
{
    AwaTime value = 0;
    AwaTime * valuePtr = NULL;

    valuePtr = ArrayIterator_GetValue((const AwaArrayIterator *)iterator);

    if (valuePtr != NULL)
    {
        value = *valuePtr;
    }

    return value;
}


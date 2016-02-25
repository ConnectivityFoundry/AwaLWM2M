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

typedef struct _AwaArray _AwaIntegerArray;
typedef struct _AwaArrayIterator _AwaIntegerArrayIterator;

AwaIntegerArray * AwaIntegerArray_New(void)
{
    return (AwaIntegerArray *)AwaArray_New();
}

void AwaIntegerArray_Free(AwaIntegerArray ** array)
{
    AwaArray_Free((AwaArray **)array, AwaResourceType_IntegerArray);
}

void AwaIntegerArray_SetValue(AwaIntegerArray * array, AwaArrayIndex index, AwaInteger value)
{
    Array_SetValue((AwaArray *)array, index, (void *)&value, sizeof(AwaInteger));
}

void AwaIntegerArray_DeleteValue(AwaIntegerArray * array, AwaArrayIndex index)
{
    Array_DeleteItem((AwaArray *)array, index, AwaResourceType_IntegerArray);
}

AwaInteger AwaIntegerArray_GetValue(const AwaIntegerArray * array, AwaArrayIndex index)
{
    AwaInteger value = 0;
    AwaInteger * valuePtr = NULL;

    valuePtr = Array_GetValue((const AwaArray*)array, index);

    if (valuePtr != NULL)
    {
        value = *valuePtr;
    }

    return value;
}

size_t AwaIntegerArray_GetValueCount(const AwaIntegerArray * array)
{
    return Array_GetValueCount((const AwaArray *)array);
}

AwaIntegerArrayIterator * AwaIntegerArray_NewIntegerArrayIterator(const AwaIntegerArray * array)
{
    return (AwaIntegerArrayIterator *)ArrayIterator_New((const AwaArray *)array);
}

bool AwaIntegerArray_IsValid(const AwaIntegerArray * array, AwaArrayIndex index)
{
    return Array_GetArrayItem((const AwaArray *)array, index) == NULL ? false : true;
}

void AwaIntegerArrayIterator_Free(AwaIntegerArrayIterator ** iterator)
{
    ArrayIterator_Free((AwaArrayIterator **)iterator);
}

AwaArrayIndex AwaIntegerArrayIterator_GetIndex(const AwaIntegerArrayIterator * iterator)
{
    return ArrayIterator_GetIndex((const AwaArrayIterator *)iterator);
}

bool AwaIntegerArrayIterator_Next(AwaIntegerArrayIterator * iterator)
{
    return ArrayIterator_Next((AwaArrayIterator *)iterator);
}

AwaInteger AwaIntegerArrayIterator_GetValue(const AwaIntegerArrayIterator * iterator)
{
    AwaInteger value = 0;
    AwaInteger * valuePtr = NULL;

    valuePtr = ArrayIterator_GetValue((const AwaArrayIterator *)iterator);

    if(valuePtr)
    {
        value = *valuePtr;
    }

    return value;
}


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

typedef struct _AwaArray _AwaObjectLinkArray;
typedef struct _AwaArrayIterator _AwaObjectLinkArrayIterator;

AwaObjectLinkArray * AwaObjectLinkArray_New(void)
{
    return (AwaObjectLinkArray *)AwaArray_New();
}

void AwaObjectLinkArray_Free(AwaObjectLinkArray ** array)
{
    AwaArray_Free((AwaArray **)array, AwaResourceType_ObjectLinkArray);
}

void AwaObjectLinkArray_SetValue(AwaObjectLinkArray * array, AwaArrayIndex index, AwaObjectLink value)
{
    Array_SetValue((AwaArray *)array, index, (void *)&value, sizeof(AwaObjectLink));
}

void AwaObjectLinkArray_DeleteValue(AwaObjectLinkArray * array, AwaArrayIndex index)
{
    Array_DeleteItem((AwaArray *)array, index, AwaResourceType_ObjectLinkArray);
}

AwaObjectLink AwaObjectLinkArray_GetValue(const AwaObjectLinkArray * array, AwaArrayIndex index)
{
    AwaObjectLink value;
    void * valuePtr;

    memset(&value, 0, sizeof(AwaObjectLink));

    valuePtr = Array_GetValue((const AwaArray *)array, index);

    if (valuePtr != NULL)
    {
        memcpy(&value, valuePtr, sizeof(AwaObjectLink));
    }
    return value;
}

size_t AwaObjectLinkArray_GetValueCount(const AwaObjectLinkArray * array)
{
    return Array_GetValueCount((const AwaArray *)array);
}

AwaObjectLinkArrayIterator * AwaObjectLinkArray_NewObjectLinkArrayIterator(const AwaObjectLinkArray * array)
{
    return (AwaObjectLinkArrayIterator *)ArrayIterator_New((const AwaArray *)array);
}

bool AwaObjectLinkArray_IsValid(const AwaObjectLinkArray * array, AwaArrayIndex index)
{
    return (Array_GetArrayItem((const AwaArray *)array, index) == NULL) ? false : true;
}

void AwaObjectLinkArrayIterator_Free(AwaObjectLinkArrayIterator ** iterator)
{
    ArrayIterator_Free((AwaArrayIterator **)iterator);
}

AwaArrayIndex AwaObjectLinkArrayIterator_GetIndex(const AwaObjectLinkArrayIterator * iterator)
{
    return ArrayIterator_GetIndex((const AwaArrayIterator *)iterator);
}

bool AwaObjectLinkArrayIterator_Next(AwaObjectLinkArrayIterator * iterator)
{
    return ArrayIterator_Next((AwaArrayIterator *)iterator);
}

AwaObjectLink AwaObjectLinkArrayIterator_GetValue(const AwaObjectLinkArrayIterator * iterator)
{
    AwaObjectLink value;

    void * valuePtr = ArrayIterator_GetValue((const AwaArrayIterator *)iterator);

    memset(&value, 0, sizeof(AwaObjectLink));

    if (valuePtr != NULL)
    {
        memcpy(&value, valuePtr, sizeof(AwaObjectLink));
    }

    return value;
}


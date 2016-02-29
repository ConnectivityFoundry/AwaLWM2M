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
#include "memalloc.h"

typedef struct _AwaArray _AwaOpaqueArray;
typedef struct _AwaArrayIterator _AwaOpaqueArrayIterator;

AwaOpaqueArray * AwaOpaqueArray_New(void)
{
    return (AwaOpaqueArray *)AwaArray_New();
}

void AwaOpaqueArray_Free(AwaOpaqueArray ** array)
{
    AwaArray_Free((AwaArray **)array, AwaResourceType_OpaqueArray);
}

void AwaOpaqueArray_SetValue(AwaOpaqueArray * array, AwaArrayIndex index, AwaOpaque value)
{
    if (array != NULL)
    {
        if (((value.Data != NULL) && (value.Size > 0)) || ((value.Data == NULL) && (value.Size == 0)))
        {
            void * valuePtr = Array_GetValue((const AwaArray *)array, index);
            if (valuePtr != NULL)
            {
                // free existing entry, if it exists
                AwaOpaque * storedOpaque = (AwaOpaque *)valuePtr;
                Awa_MemSafeFree(storedOpaque->Data);
                storedOpaque->Data = NULL;
                storedOpaque->Size = 0;
            }

            AwaOpaque copy;
            memset(&copy, 0, sizeof(copy));
            copy.Size = value.Size;
            if (copy.Size > 0)
            {
                copy.Data = Awa_MemAlloc(copy.Size);
                if (copy.Data != NULL)
                {
                    memcpy(copy.Data, value.Data, value.Size);
                }
            }

            Array_SetValue((AwaArray *)array, index, (void *)&copy, sizeof(copy));
        }
        else
        {
            if (value.Data != NULL)
            {
                LogError("AwaOpaque value is not NULL but has a length of zero");
            }
            else
            {
                LogError("AwaOpaque value is NULL but has a non-zero length");
            }
        }
    }
    else
    {
        LogError("AwaOpaqueArray is NULL");
    }
}

void AwaOpaqueArray_DeleteValue(AwaOpaqueArray * array, AwaArrayIndex index)
{
    Array_DeleteItem((AwaArray *)array, index, AwaResourceType_OpaqueArray);
}

AwaOpaque AwaOpaqueArray_GetValue(const AwaOpaqueArray * array, AwaArrayIndex index)
{
    AwaOpaque value;

    void * valuePtr = Array_GetValue((const AwaArray *)array, index);

    memset(&value, 0, sizeof(AwaOpaque));

    if (valuePtr != NULL)
    {
        AwaOpaque * storedOpaque = (AwaOpaque * )valuePtr;
        value.Data = storedOpaque->Data;
        value.Size = storedOpaque->Size;
    }

    return value;
}

size_t AwaOpaqueArray_GetValueCount(const AwaOpaqueArray * array)
{
    return Array_GetValueCount((const AwaArray *)array);
}

AwaOpaqueArrayIterator * AwaOpaqueArray_NewOpaqueArrayIterator(const AwaOpaqueArray * array)
{
    return (AwaOpaqueArrayIterator *)ArrayIterator_New((const AwaArray *)array);
}

bool AwaOpaqueArray_IsValid(const AwaOpaqueArray * array, AwaArrayIndex index)
{
    return (Array_GetArrayItem((const AwaArray *)array, index) == NULL) ? false : true;
}

void AwaOpaqueArrayIterator_Free(AwaOpaqueArrayIterator ** iterator)
{
    ArrayIterator_Free((AwaArrayIterator **)iterator);
}

AwaArrayIndex AwaOpaqueArrayIterator_GetIndex(const AwaOpaqueArrayIterator * iterator)
{
    return ArrayIterator_GetIndex((const AwaArrayIterator *)iterator);
}

bool AwaOpaqueArrayIterator_Next(AwaOpaqueArrayIterator * iterator)
{
    return ArrayIterator_Next((AwaArrayIterator *)iterator);
}

AwaOpaque AwaOpaqueArrayIterator_GetValue(const AwaOpaqueArrayIterator * iterator)
{
    AwaOpaque value;

    void * valuePtr = ArrayIterator_GetValue((const AwaArrayIterator *)iterator);

    memset(&value, 0, sizeof(AwaOpaque));

    if (valuePtr != NULL)
    {
        AwaOpaque * storedOpaque = (AwaOpaque * )valuePtr;
        value.Data = storedOpaque->Data;
        value.Size = storedOpaque->Size;
    }

    return value;
}


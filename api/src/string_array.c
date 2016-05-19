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
typedef struct _AwaArrayIterator _AwaCStringArrayIterator;

AwaStringArray * AwaStringArray_New(void)
{
    AwaStringArray * result = NULL;
    AwaArray * nulledStrings = AwaArray_New();

    if (nulledStrings != NULL)
    {
        result = (AwaStringArray *)AwaArray_New();
        AwaArray_SetContext((AwaArray *)result, nulledStrings);
    }

    return result;
}

void AwaStringArray_Free(AwaStringArray ** array)
{
    if ((array != NULL) && (*array != NULL))
    {
        AwaArray * nulledStrings = AwaArray_GetContext((AwaArray *)*array);
        if (nulledStrings != NULL)
        {
            AwaArray_Free(&nulledStrings, AwaResourceType_StringArray);
        }
        AwaArray_Free((AwaArray **)array, AwaResourceType_StringArray);
    }
}

void AwaStringArray_SetValueAsCString(AwaStringArray * array, AwaArrayIndex index, const char * value)
{
    if (array != NULL)
    {
        AwaArray * nulledStrings = AwaArray_GetContext((AwaArray *)array);
        if (nulledStrings != NULL)
        {
            Array_SetValue((AwaArray *)nulledStrings, index, (void *)value, strlen(value) + 1);
        }
        Array_SetValue((AwaArray *)array, index, (void *)value, strlen(value));
    }
}

void AwaStringArray_DeleteValue(AwaStringArray * array, AwaArrayIndex index)
{
    if (array != NULL)
    {
        AwaArray * nulledStrings = AwaArray_GetContext((AwaArray *)array);
        if (nulledStrings != NULL)
        {
            Array_DeleteItem((AwaArray *)nulledStrings, index, AwaResourceType_StringArray);
        }
        Array_DeleteItem((AwaArray *)array, index, AwaResourceType_StringArray);
    }
}

const char * AwaStringArray_GetValueAsCString(const AwaStringArray * array, AwaArrayIndex index)
{
    char * value = NULL;

    if (array != NULL)
    {
        AwaArray * nulledStrings = AwaArray_GetContext((AwaArray *)array);
        if (nulledStrings != NULL)
        {
            value = Array_GetValue((const AwaArray *)nulledStrings, index);

            if (value == NULL)
            {
                const char * nonNulledValue = Array_GetValue((AwaArray *)array, index);
                int nonNulledLength = Array_GetValueLength((AwaArray *)array, index);
                char *  nulledValue = (char *)malloc(nonNulledLength + 1);

                if ((nonNulledValue != NULL) && (nulledValue != NULL) && (nonNulledLength >= 0))
                {
                    memcpy(nulledValue, nonNulledValue, (size_t)nonNulledLength);
                    nulledValue[nonNulledLength] = '\0';
                    Array_SetValue(nulledStrings, index, nulledValue, nonNulledLength + 1);
                    free(nulledValue);
                    value = Array_GetValue((const AwaArray *)nulledStrings, index);
                }
                else
                {
                    free(nulledValue);
                }
            }
        }
    }

    return value;
}

size_t AwaStringArray_GetValueCount(const AwaStringArray * array)
{
    return Array_GetValueCount((const AwaArray *)array);
}

AwaCStringArrayIterator * AwaStringArray_NewCStringArrayIterator(const AwaStringArray * array)
{
    AwaCStringArrayIterator * iterator = NULL;
    if (array != NULL)
    {
        AwaArray * nulledStrings = AwaArray_GetContext((AwaArray *)array);
        if (nulledStrings != NULL)
        {
            iterator = (AwaCStringArrayIterator *)ArrayIterator_New((const AwaArray *)nulledStrings);
        }
    }
    return iterator;
}

bool AwaStringArray_IsValid(const AwaStringArray * array, AwaArrayIndex index)
{
    return (Array_GetArrayItem((const AwaArray *)array, index) == NULL) ? false : true;
}

void AwaCStringArrayIterator_Free(AwaCStringArrayIterator ** iterator)
{
    ArrayIterator_Free((AwaArrayIterator **)iterator);
}

AwaArrayIndex AwaCStringArrayIterator_GetIndex(const AwaCStringArrayIterator * iterator)
{
    return ArrayIterator_GetIndex((const AwaArrayIterator *)iterator);
}

bool AwaCStringArrayIterator_Next(AwaCStringArrayIterator * iterator)
{
    return ArrayIterator_Next((AwaArrayIterator *)iterator);
}

const char * AwaCStringArrayIterator_GetValueAsCString(const AwaCStringArrayIterator * iterator)
{
    return ArrayIterator_GetValue((AwaArrayIterator *)iterator);
}


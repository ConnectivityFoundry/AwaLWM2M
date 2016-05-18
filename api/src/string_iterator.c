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


#include "string_iterator.h"
#include "iterator.h"
#include "memalloc.h"
#include "log.h"

struct _StringIterator
{
    Iterator * Iterator;
};

StringIterator * StringIterator_New(void)
{
    StringIterator * iterator = Awa_MemAlloc(sizeof(*iterator));
    if (iterator != NULL)
    {
        memset(iterator, 0, sizeof(*iterator));
        iterator->Iterator = Iterator_New();
        LogNew("StringIterator", iterator);
    }
    else
    {
        LogErrorWithEnum(AwaError_OutOfMemory);
    }
    return iterator;
}

void StringIterator_Free(StringIterator ** iterator)
{
    if ((iterator != NULL) && (*iterator != NULL))
    {
        // free memory managed by iterator:
        Iterator_FreeValues((*iterator)->Iterator);
        Iterator_Free(&(*iterator)->Iterator);
        LogFree("StringIterator", *iterator);
        Awa_MemSafeFree(*iterator);
        *iterator = NULL;
    }
}

void StringIterator_Add(StringIterator * iterator, const char * string)
{
    if (iterator != NULL)
    {
        if (string != NULL)
        {
            // allocate memory, to be managed by iterator:
            char * newString = strdup(string);
            if (newString != NULL)
            {
                Iterator_Add(iterator->Iterator, newString);
            }
            else
            {
                LogErrorWithEnum(AwaError_Internal, "unable to allocate memory for string");
            }
        }
        else
        {
            LogErrorWithEnum(AwaError_Internal, "string is NULL");
        }
    }
    else
    {
        LogErrorWithEnum(AwaError_IteratorInvalid, "iterator is NULL");
    }
}

bool StringIterator_Next(StringIterator * iterator)
{
    bool result = false;
    if (iterator != NULL)
    {
        result = Iterator_Next(iterator->Iterator);
    }
    else
    {
        LogErrorWithEnum(AwaError_IteratorInvalid, "iterator is NULL");
    }
    return result;
}

const char * StringIterator_Get(const StringIterator * iterator)
{
    const char * string = NULL;
    if (iterator != NULL)
    {
        string = (const char *)Iterator_Get(iterator->Iterator);
    }
    else
    {
        LogErrorWithEnum(AwaError_IteratorInvalid, "iterator is NULL");
    }
    return string;
}


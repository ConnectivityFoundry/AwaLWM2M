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


#include "iterator.h"
#include "memalloc.h"
#include "log.h"
#include "lwm2m_list.h"

typedef struct
{
    struct ListHead List;        // prev/next pointers
    void * Value;
} Item;

struct _Iterator
{
    struct ListHead ItemList;   // pointer to the first item in the list
    struct ListHead * Current;  // pointer to the current item in the list
};

Iterator * Iterator_New(void)
{
    Iterator * iterator = Awa_MemAlloc(sizeof (*iterator));
    if (iterator != NULL)
    {
        memset(iterator, 0, sizeof(*iterator));
        ListInit(&iterator->ItemList);
        iterator->Current = NULL;
        LogNew("Iterator", iterator);
    }
    else
    {
        LogErrorWithEnum(AwaError_OutOfMemory);
    }
    return iterator;
}

void Iterator_FreeValues(Iterator * iterator)
{
    if (iterator != NULL)
    {
        struct ListHead * current;
        ListForEach(current, &iterator->ItemList)
        {
            Item * item = ListEntry(current, Item, List);
            Awa_MemSafeFree(item->Value);
            item->Value = NULL;
        }
    }
}

void Iterator_Free(Iterator ** iterator)
{
    // Does not attempt to free any values in the iterator - assumes
    // all values are either owned by the iterator, or previously freed.

    if ((iterator != NULL) && (*iterator != NULL))
    {
        struct ListHead * current, * next;
        ListForEachSafe(current, next, &(*iterator)->ItemList)
        {
            Item * item = ListEntry(current, Item, List);
            ListRemove(current);
            Awa_MemSafeFree(item);
        }

        LogFree("Iterator", *iterator);
        Awa_MemSafeFree(*iterator);
        *iterator = NULL;
    }
}

void Iterator_Add(Iterator * iterator, void * value)
{
    if (iterator != NULL)
    {
        Item * item = Awa_MemAlloc(sizeof(*item));
        if (item != NULL)
        {
            memset(item, 0, sizeof(*item));
            item->Value = value;
            ListAdd(&item->List, &iterator->ItemList);
        }
        else
        {
            LogErrorWithEnum(AwaError_OutOfMemory);
        }
    }
    else
    {
        LogErrorWithEnum(AwaError_IteratorInvalid, "iterator is NULL");
    }
}

bool Iterator_Next(Iterator * iterator)
{
    bool result = false;
    if (iterator != NULL)
    {
        if (iterator->Current == NULL)
        {
            // point at first item
            iterator->Current = iterator->ItemList.Next;
        }
        else
        {
            // advance the current pointer, if not at end
            if (iterator->Current != &iterator->ItemList)
            {
                iterator->Current = iterator->Current->Next;
            }
        }
        result = (iterator->Current != &iterator->ItemList);
    }
    else
    {
        LogErrorWithEnum(AwaError_IteratorInvalid, "iterator is NULL");
    }
    return result;
}

void * Iterator_Get(const Iterator * iterator)
{
    void * value = NULL;
    if (iterator != NULL)
    {
        if ((iterator->Current != NULL) && (iterator->Current != &iterator->ItemList))
        {
            Item * item = ListEntry(iterator->Current, Item, List);
            if (item != NULL)
            {
                value = item->Value;
            }
        }
    }
    else
    {
        LogErrorWithEnum(AwaError_IteratorInvalid, "iterator is NULL");
    }
    return value;
}


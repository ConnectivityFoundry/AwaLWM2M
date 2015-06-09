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


#include <string.h>

#include "list.h"
#include "memalloc.h"
#include "lwm2m_list.h"

struct _ListType
{
    struct ListHead Entries;
};

typedef struct
{
    struct ListHead List;
    void * Data;

} ListItem;

ListType * List_New(void)
{
    ListType * list = Awa_MemAlloc(sizeof(*list));
    if (list != NULL)
    {
        ListInit(&list->Entries);
    }
    return list;
}

void List_Free(ListType ** list)
{
    if ((list != NULL) && (*list != NULL))
    {
        List_Flush(*list);
        Awa_MemSafeFree(*list);
        *list = NULL;
    }
}

bool List_Add(ListType * list, void * value)
{
    int result = false;

    if ((list != NULL) && (value != NULL))
    {
        ListItem * listItem = Awa_MemAlloc(sizeof(ListItem));
        if (listItem)
        {
            listItem->Data = value;

            ListAdd(&listItem->List, &list->Entries);
            result = true;
        }
    }
    return result;
}

bool List_Get(ListType * list, size_t index, void ** value)
{
    bool result = false;

    if ((list != NULL) && (value != NULL))
    {
        struct ListHead * current, * next;
        size_t i = 0u;
        ListForEachSafe(current, next, &list->Entries)
        {
            ListItem * listItem = ListEntry(current, ListItem, List);
            if (i == index)
            {
                *value = listItem->Data;
                result = true;
                break;
            }
            ++i;
        }
    }
    return result;
}

size_t List_Length(ListType * list)
{
   return ListCount(&list->Entries);
}

void List_Flush(ListType * list)
{
    if (list != NULL)
    {
        struct ListHead * current, * next;
        ListForEachSafe(current, next, &list->Entries)
        {
            ListItem * listItem = ListEntry(current, ListItem, List);
            ListRemove(&listItem->List);
            Awa_MemSafeFree(listItem);
        }
    }
}

size_t List_ForEach(ListType * list, ListForEachFunction callback, void * context)
{
    size_t count = 0;
    if (list != NULL)
    {
        struct ListHead * current;
        ListForEach(current, &list->Entries)
        {
            ListItem * listItem = ListEntry(current, ListItem, List);
            callback(count, listItem->Data, context);
            ++count;
        }
    }
    return count;
}

size_t List_Find(ListType * list, ListFindFunction callback, void * context, void ** value)
{
    size_t count = 0;
    if (list != NULL)
    {
        struct ListHead * current;
        ListForEach(current, &list->Entries)
        {
            ListItem * listItem = ListEntry(current, ListItem, List);
            if (callback(count, listItem->Data, context))
            {
                *value = listItem->Data;
                break;
            }
            ++count;
        }
    }
    return count;
}

bool List_Remove(ListType * list, void * value)
{
    bool removed = false;
    if (list != NULL)
    {
        struct ListHead * current, * next;
        ListForEachSafe(current, next, &list->Entries)
        {
            ListItem * listItem = ListEntry(current, ListItem, List);
            if (listItem->Data == value)
            {
                ListRemove(&listItem->List);
                Awa_MemSafeFree(listItem);
                removed = true;
                break;
            }
        }
    }
    return removed;
}

bool List_Contains(ListType * list, void * value)
{
    bool found = false;
    if (list != NULL)
    {
        struct ListHead * current;
        ListForEach(current, &list->Entries)
        {
            ListItem * listItem = ListEntry(current, ListItem, List);
            if (listItem->Data == value)
            {
                found = true;
                break;
            }
        }
    }
    return found;
}


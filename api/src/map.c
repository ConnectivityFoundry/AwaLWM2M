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
#include "map.h"
#include "memalloc.h"
#include "lwm2m_list.h"

struct _MapType
{
    struct ListHead Entries;
};

typedef struct
{
    struct ListHead List;
    void * Data;
    const char * Path;

} MapItem;

MapType * Map_New(void)
{
    MapType * map = Awa_MemAlloc(sizeof(*map));
    if (map)
    {
        ListInit(&map->Entries);
    }
    return map;
}

void Map_Free(MapType ** map)
{
    if ((map != NULL) && (*map != NULL))
    {
        Map_Flush(*map);
        Awa_MemSafeFree(*map);
        *map = NULL;
    }
}

void Map_FreeValues(MapType * map)
{
    if (map != NULL)
    {
        struct ListHead * current, * next;
        ListForEachSafe(current, next, &map->Entries)
        {
            MapItem * mapItem = ListEntry(current, MapItem, List);
            Awa_MemSafeFree(mapItem->Data);
        }
        Map_Flush(map);
    }
}
void * Map_Remove(MapType * map, const char * key)
{
    void * removedData = NULL;

    if ((map != NULL) && (key != NULL))
    {
        struct ListHead * current, * next;
        ListForEachSafe(current, next, &map->Entries)
        {
            MapItem * mapItem = ListEntry(current, MapItem, List);
            if (strcmp(mapItem->Path, key) == 0)
            {
                ListRemove(&mapItem->List);
                removedData = mapItem->Data;
                free((void *)mapItem->Path);
                Awa_MemSafeFree(mapItem);
                break;
            }
        }
    }
    return removedData;
}

bool Map_Put(MapType * map, const char * key, void * value)
{
    int result = false;

    if ((map != NULL) && (key != NULL) && (value != NULL))
    {
        // Replace if path already exists
        struct ListHead * current, * next;
        ListForEachSafe(current, next, &map->Entries)
        {
            MapItem * mapItem = ListEntry(current, MapItem, List);
            if (strcmp(mapItem->Path, key) == 0)
            {
                mapItem->Data = value;
                result = true;
                break;
            }
        }

        if (result == false)
        {
            MapItem * mapItem = Awa_MemAlloc(sizeof(MapItem));
            if (mapItem != NULL)
            {
                mapItem->Path = strdup(key);
                mapItem->Data = value;

                ListAdd(&mapItem->List, &map->Entries);
                result = true;
            }
        }
    }
    return result;
}

bool Map_Contains(MapType * map, const char * key)
{
    void * value;
    return (Map_Get(map, key, &value) == true);
}

bool Map_Get(MapType * map, const char * key, void ** value)
{
    bool result = false;

    if ((map != NULL) && (key != NULL) && (value != NULL))
    {
        struct ListHead * current, * next;
        ListForEachSafe(current, next, &map->Entries)
        {
            MapItem * mapItem = ListEntry(current, MapItem, List);
            if (strcmp(mapItem->Path, key) == 0)
            {
                *value = mapItem->Data;
                result = true;
                break;
            }
        }
    }
    return result;
}

size_t Map_Length(MapType * map)
{
   return ListCount(&map->Entries);
}

void Map_Flush(MapType * map)
{
    if (map != NULL)
    {
        struct ListHead * current, * next;
        ListForEachSafe(current, next, &map->Entries)
        {
            MapItem * mapItem = ListEntry(current, MapItem, List);
            ListRemove(&mapItem->List);
            free((void *)mapItem->Path);
            Awa_MemSafeFree(mapItem);
        }
    }
}

size_t Map_ForEach(MapType * map, MapForEachFunction callback, void * context)
{
    size_t count = 0;
    if (map != NULL)
    {
        struct ListHead * current;
        ListForEach(current, &map->Entries)
        {
            MapItem * mapItem = ListEntry(current, MapItem, List);
            callback(mapItem->Path, mapItem->Data, context);
            ++count;
        }
    }
    return count;
}


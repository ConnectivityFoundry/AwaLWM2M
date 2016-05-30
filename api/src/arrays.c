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
#include "lwm2m_list.h"
#include "memalloc.h"
#include "log.h"
#include "arrays.h"

struct _ArrayItem
{
    struct ListHead List; // prev/next pointers
    void * Value;
    size_t Length;
    AwaArrayIndex Index;
};

struct _AwaArray
{
    struct ListHead ValueList;
    void * Context;
};

struct _AwaArrayIterator
{
    const struct ListHead * ValueList;
    struct ListHead * CurrentValue;
};


AwaArray * AwaArray_New(void)
{
    AwaArray * array = Awa_MemAlloc(sizeof(*array));

    if (array != NULL)
    {
        memset(array, 0, sizeof(*array));
        ListInit(&array->ValueList);
        LogNew("AwaArray", array);
    }
    else
    {
        LogErrorWithEnum(AwaError_OutOfMemory);
    }

    return array;
}

void AwaArray_SetContext(AwaArray * array, void * context)
{
    if (array != NULL)
    {
        array->Context = context;
    }
}

void * AwaArray_GetContext(AwaArray * array)
{
    void * result = NULL;

    if (array != NULL)
    {
        result = array->Context;
    }

    return result;
}

void Array_FreeItemValue(ArrayItem * valueItem)
{
    LogFree("AwaArray.Item.Value",  valueItem->Value);
    Awa_MemSafeFree(valueItem->Value);
    valueItem->Value = NULL;
}

void Array_FreeItem(ArrayItem * valueItem, AwaResourceType resourceType)
{
    if (resourceType == AwaResourceType_OpaqueArray)
    {
        AwaOpaque * opaque = (AwaOpaque *)valueItem->Value;
        LogFree("AwaArray.OpaqueData", opaque->Data);
        Awa_MemSafeFree(opaque->Data);
    }
    Array_FreeItemValue(valueItem);
    LogFree("AwaArray.Item", valueItem);
    Awa_MemSafeFree(valueItem);
}

void Array_Free(struct ListHead * ValueList, AwaResourceType resourceType)
{
    struct ListHead * current, * next;
    ListForEachSafe(current, next, ValueList)
    {
        ArrayItem * valueItem = ListEntry(current, ArrayItem, List);
        ListRemove(current);
        Array_FreeItem(valueItem, resourceType);
    }
}

void AwaArray_Free(AwaArray ** array, AwaResourceType resourceType)
{
    if ((array != NULL) && (*array != NULL))
    {
        Array_Free(&(*array)->ValueList, resourceType);
        LogFree("AwaArray", *array);
        Awa_MemSafeFree(*array);
        *array = NULL;
    }
    else
    {
        LogError("array is NULL");
    }
}

ArrayItem * Array_GetArrayItem(const AwaArray * array, AwaArrayIndex index)
{
    ArrayItem * item = NULL;

    if (array != NULL)
    {
        struct ListHead * current;
        ListForEach(current, &array->ValueList)
        {
            ArrayItem * valueItem = ListEntry(current, ArrayItem, List);
            if(valueItem)
            {
                if(valueItem->Index == index)
                    item = valueItem;
            }
        }
    }
    else
    {
        LogError("array is NULL");
    }

    return item;
}

ArrayItem * Array_NewItemValue(AwaArray * array, AwaArrayIndex index)
{
    ArrayItem * item = NULL;

    if (array != NULL)
    {
        item = Awa_MemAlloc(sizeof(*item));
        if (item != NULL)
        {
            memset(item, 0, sizeof(*item));
            ListAdd(&item->List, &array->ValueList);
            item->Index = index;
            LogNew("AwaArray.Item", item);
        }
        else
        {
            LogErrorWithEnum(AwaError_OutOfMemory);
        }

    }
    else
    {
        LogError("array is NULL");
    }

    return item;
}

ArrayItem * Array_SetValue(AwaArray * array, AwaArrayIndex index, void * value, size_t valueLength)
{
    ArrayItem * item = NULL;

    if (array != NULL)
    {
        if (value != NULL && valueLength != 0)
        {
            item = Array_GetArrayItem(array, index);

            if (item)
            {
                Array_FreeItemValue(item);
            }
            else
            {
                item = Array_NewItemValue(array, index);
            }

            if (item)
            {
                item->Value = Awa_MemAlloc(valueLength);
                if (item->Value)
                {
                    // avoid an unnecessary memcpy if Value is NULL (cppcheck detects null pointer dereference)
                    if (value)
                    {
                        memcpy(item->Value, value, valueLength);
                    }
                    item->Length = valueLength;
                    LogNew("AwaArray.Item.Value",  item->Value);
                }
                else
                {
                    LogError("Memory allocation failed");
                }
            }
            else
            {
                LogError("Could not create item to add to AwaArray");
            }
        }
        else
        {
            if (valueLength != 0)
            {
                LogError("Item value pointer is NULL");
            }
            else
            {
                LogError("Item value length is 0");
            }
        }
    }
    else
    {
        LogError("array is NULL");
    }

    return item;
}

void * Array_GetValue(const AwaArray * array, AwaArrayIndex index)
{
    void * value = NULL;

    if (array != NULL)
    {
        ArrayItem * item = Array_GetArrayItem(array, index);
        if(item)
        {
            if(item->Value)
            {
                if (item->Length == 0)
                {
                    value = NULL;
                    LogDebug("Item has zero length - returning NULL");
                }
                else
                {
                    value = item->Value;
                }
            }
            else
            {
                LogError("Item value pointer is NULL");
            }
        }
        else
        {
            LogWarning("Item with index %llu not found", (unsigned long long)index);
        }
    }
    else
    {
        LogError("array is NULL");
    }

    return value;
}

int Array_GetValueLength(const AwaArray * array, AwaArrayIndex index)
{
    int valueLength = - 1;

    if (array != NULL)
    {
        ArrayItem * item = Array_GetArrayItem(array, index);
        if (item != NULL)
        {
            if (item->Value)
            {
                valueLength = item->Length;
            }
            else
            {
                LogError("Item value pointer is NULL");
            }
        }
        else
        {
            LogWarning("Item with index %llu not found", (unsigned long long)index);
        }
    }
    else
    {
        LogError("array is NULL");
    }

    return valueLength;
}

void Array_DeleteItem(AwaArray * array, AwaArrayIndex index, AwaResourceType resourceType)
{
    if (array != NULL)
    {
        ArrayItem * item = Array_GetArrayItem(array, index);

        if (item != NULL)
        {
            ListRemove(&item->List);
            Array_FreeItem(item, resourceType);
        }
        else
        {
            LogWarning("Item with index %llu not found", (unsigned long long)index);
        }
    }
    else
    {
        LogError("array is NULL");
    }
}

size_t Array_GetValueCount(const AwaArray * array)
{
    size_t count = 0;

    if (array != NULL)
    {
        count = ListCount(&array->ValueList);
    }
    else
    {
        LogError("AwaArray is NULL");
    }

    return count;
}

int Array_Compare(AwaArray * array1, AwaArray * array2, AwaResourceType resourceType)
{
    int result = 0;
    AwaArrayIterator * iterator1 = ArrayIterator_New(array1);
    AwaArrayIterator * iterator2 = ArrayIterator_New(array2);

    while (ArrayIterator_Next(iterator1))
    {
        if (!ArrayIterator_Next(iterator2))
        {
            result = -1;
            goto end;
        }
        if (ArrayIterator_GetIndex(iterator1) != ArrayIterator_GetIndex(iterator2))
        {
            result = -1;
            goto end;
        }
        if (ArrayIterator_GetValueLength(iterator1) != ArrayIterator_GetValueLength(iterator2))
        {
            result = -1;
            goto end;
        }
        if (resourceType == AwaResourceType_OpaqueArray)
        {
            AwaOpaque * opaque1 = (AwaOpaque *)ArrayIterator_GetValue(iterator1);
            AwaOpaque * opaque2 = (AwaOpaque *)ArrayIterator_GetValue(iterator2);
            if ((opaque1->Size != opaque2->Size) || (memcmp(opaque1->Data, opaque2->Data, opaque1->Size) != 0))
            {
                result = -1;
                goto end;
            }
        }
        else
        {
            if (memcmp(ArrayIterator_GetValue(iterator1),ArrayIterator_GetValue(iterator2), ArrayIterator_GetValueLength(iterator1)) != 0)
            {
                result = -1;
                goto end;
            }
        }
    }
    if (ArrayIterator_Next(iterator2))
    {
        result = -1;
        goto end;
    }

end:
    ArrayIterator_Free(&iterator1);
    ArrayIterator_Free(&iterator2);
    return result;
}

AwaArrayIterator * ArrayIterator_New(const AwaArray * array)
{
    AwaArrayIterator * iterator = NULL;
    if (array != NULL)
    {
        iterator = Awa_MemAlloc(sizeof(*iterator));

        if (iterator != NULL)
        {
            memset(iterator, 0, sizeof(*iterator));
            iterator->ValueList = &array->ValueList;
        }
        else
        {
            LogErrorWithEnum(AwaError_OutOfMemory);
        }
    }
    else
    {
        LogError("array is NULL");
    }
    return iterator;
}

bool ArrayIterator_Next(AwaArrayIterator * iterator)
{
    bool result = false;

    if (iterator != NULL)
    {
        if (iterator->CurrentValue == NULL)
        {
            iterator->CurrentValue = iterator->ValueList->Next;
        }
        else
        {
            if (iterator->CurrentValue != iterator->ValueList)
            {
                iterator->CurrentValue = iterator->CurrentValue->Next;
            }
        }

        result = iterator->CurrentValue != iterator->ValueList;
    }
    else
    {
        LogError("iterator is NULL");
    }

    return result;
}

ArrayItem * ArrayIterator_GetItem(const AwaArrayIterator * iterator)
{
    ArrayItem * arrayItem = NULL;

    if (iterator != NULL)
    {
        if ((iterator->CurrentValue != NULL) && (iterator->CurrentValue != iterator->ValueList))
        {
            arrayItem = ListEntry(iterator->CurrentValue, ArrayItem, List);
        }
    }
    else
    {
        LogError("iterator is NULL");
    }
    return arrayItem;
}

AwaArrayIndex ArrayIterator_GetIndex(const AwaArrayIterator * iterator)
{
    AwaArrayIndex index = 0;
    ArrayItem * arrayItem = ArrayIterator_GetItem(iterator);

    if (arrayItem != NULL)
    {
        index = arrayItem->Index;
    }
    return index;
}

void * ArrayIterator_GetValue(const AwaArrayIterator * iterator)
{
    void * value = NULL;
    ArrayItem * arrayItem = ArrayIterator_GetItem(iterator);

    if (arrayItem != NULL)
    {
        value = arrayItem->Value;
    }
    return value;
}

size_t ArrayIterator_GetValueLength(const AwaArrayIterator * iterator)
{
    size_t length = 0;
    ArrayItem * arrayItem = ArrayIterator_GetItem(iterator);

    if (arrayItem != NULL)
    {
        length = arrayItem->Length;
    }
    return length;
}

void ArrayIterator_Free(AwaArrayIterator ** iterator)
{
    if ((iterator != NULL) && (*iterator != NULL))
    {
        Awa_MemSafeFree(*iterator);
        *iterator = NULL;
    }
}


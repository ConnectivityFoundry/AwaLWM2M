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

#include "queue.h"
#include "memalloc.h"
#include "lwm2m_list.h"

struct _QueueType
{
    struct ListHead Entries;
};

typedef struct
{
    struct ListHead List;
    void * Data;

} QueueItem;

QueueType * Queue_New(void)
{
    QueueType * queue = Awa_MemAlloc(sizeof(*queue));
    if (queue != NULL)
    {
        ListInit(&queue->Entries);
    }
    return queue;
}

void Queue_Free(QueueType ** queue)
{
    if ((queue != NULL) && (*queue != NULL))
    {
        Queue_Flush(*queue);
        Awa_MemSafeFree(*queue);
        *queue = NULL;
    }
}

bool Queue_Push(QueueType * queue, void * item)
{
    int result = false;

    if ((queue != NULL) && (item != NULL))
    {
        QueueItem * queueItem = Awa_MemAlloc(sizeof(QueueItem));
        if (queueItem)
        {
            queueItem->Data = item;

            ListAdd(&queueItem->List, &queue->Entries);
            result = true;
        }
    }

    return result;
}

bool Queue_Pop(QueueType * queue, void ** item)
{
    bool result = false;

    if ((queue != NULL) && (item != NULL))
    {
        if (queue->Entries.Next != &queue->Entries)
        {
            QueueItem * queueItem = ListEntry(queue->Entries.Next, QueueItem, List);
            ListRemove(&queueItem->List);
            *item = queueItem->Data;
            Awa_MemSafeFree(queueItem);

            result = true;
        }
    }

    return result;
}

void Queue_Flush(QueueType * queue)
{
    if (queue != NULL)
    {
        struct ListHead * current, * next;
        ListForEachSafe(current, next, &queue->Entries)
        {
            QueueItem * queueItem = ListEntry(current, QueueItem, List);
            ListRemove(&queueItem->List);
            Awa_MemSafeFree(queueItem); 
        }
    }
}


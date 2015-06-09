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


#ifndef LWM2M_LIST_H
#define LWM2M_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  example usage:
 *     
 *     struct {
 *         struct ListHead List;
 *         ... other content ...
 *     } Entry;
 *
 *     static ListHead itemList;
 *
 *     Entry * newEntry = malloc(sizeof(Entry));
 *     ListAdd(newEntry, &itemList);
 *
 *     struct ListHead * item;
 *     ListForEach(item, &itemList)
 *     {
 *         Entry_t * entry = ListEntry(item, Entry, List);
 *         ... do something with entry ..
 *         ListRemove(&entry->List);
 *         free(entry);
 */

struct ListHead
{
    struct ListHead * Prev;
    struct ListHead * Next;
};

/* locate the offset of the ListHead named "member" within a structure of
 * type "type"
 */
#define ListEntry(ptr, type, member) \
    ({(type *)((char *)ptr - ((size_t) &((type*)0)->member));})

#define ListForEach(pos, head) \
    for (pos = (head)->Next; pos != (head); pos = (pos)->Next)

#define ListForEachSafe(pos, n, head) \
    for (pos = (head)->Next, n = pos->Next; pos != (head); \
            pos = n, n = pos->Next)

#define LIST_INIT(list) { &(list), &(list) }

void ListAdd(struct ListHead * newEntry, struct ListHead * head);
void ListRemove(struct ListHead * entry);
void ListInit(struct ListHead * list);
int ListCount(const struct ListHead * list);

#ifdef __cplusplus
}
#endif

#endif // LWM2M_LIST_H

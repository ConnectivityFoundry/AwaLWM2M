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


#include <stdbool.h>
#include <string.h>

#include "awa/server.h"
#include "registered_entity_iterator.h"
#include "xml.h"
#include "log.h"
#include "objects_tree.h"
#include "path_iterator.h"
#include "response_common.h"

#define MAX_PATH_LENGTH (256)

// This struct is used for API type safety and is never instantiated.
// DO NOT USE THIS STRUCTURE!
struct _AwaRegisteredEntityIterator {};

bool AwaRegisteredEntityIterator_Next(AwaRegisteredEntityIterator * iterator)
{
    return RegisteredEntityIterator_Next((RegisteredEntityIterator *)iterator);
}

const char * AwaRegisteredEntityIterator_GetPath(const AwaRegisteredEntityIterator * iterator)
{
    return RegisteredEntityIterator_GetPath((RegisteredEntityIterator *)iterator);
}

void AwaRegisteredEntityIterator_Free(AwaRegisteredEntityIterator ** iterator)
{
    RegisteredEntityIterator_Free((RegisteredEntityIterator **)iterator);
}

RegisteredEntityIterator * RegisteredEntityIterator_New(const ResponseCommon * response)
{
    PathIterator * iterator = PathIterator_New();
    if (iterator != NULL)
    {
        TreeNode currentNode = ResponseCommon_GetObjectsNode(response);
        while ((currentNode = ObjectsTree_GetNextLeafNode(currentNode)) != NULL)
        {
            char path[MAX_PATH_LENGTH] = { 0 };
            if (ObjectsTree_GetPath(currentNode, path, sizeof(path)) != NULL)
            {
                PathIterator_Add(iterator, path);
            }
        }
    }
    else
    {
        LogErrorWithEnum(AwaError_OutOfMemory);
    }
    return (RegisteredEntityIterator *)iterator;
}

void RegisteredEntityIterator_Free(RegisteredEntityIterator ** iterator)
{
    PathIterator_Free((PathIterator **)iterator);
}

void RegisteredEntityIterator_Add(RegisteredEntityIterator * iterator, const char * path)
{
    PathIterator_Add((PathIterator *)iterator, path);
}

bool RegisteredEntityIterator_Next(RegisteredEntityIterator * iterator)
{
    return PathIterator_Next((PathIterator *)iterator);
}

const char * RegisteredEntityIterator_GetPath(const RegisteredEntityIterator * iterator)
{
    return PathIterator_Get((PathIterator *)iterator);
}


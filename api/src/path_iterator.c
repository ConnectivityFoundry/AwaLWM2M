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

#include "awa/common.h"
#include "path_iterator.h"
#include "string_iterator.h"

// This struct is used for API type safety and is never instantiated.
// DO NOT USE THIS STRUCTURE!
struct _AwaPathIterator {};

void AwaPathIterator_Free(AwaPathIterator ** iterator)
{
    PathIterator_Free((PathIterator **)iterator);
}

bool AwaPathIterator_Next(AwaPathIterator * iterator)
{
    return PathIterator_Next((PathIterator *)iterator);
}

const char * AwaPathIterator_Get(const AwaPathIterator * iterator)
{
    return PathIterator_Get((PathIterator *)iterator);
}

PathIterator * PathIterator_New(void)
{
    return (PathIterator *)StringIterator_New();
}

void PathIterator_Free(PathIterator ** iterator)
{
    StringIterator_Free((StringIterator **)iterator);
}

void PathIterator_Add(PathIterator * iterator, const char * path)
{
    StringIterator_Add((StringIterator *)iterator, path);
}

bool PathIterator_Next(PathIterator * iterator)
{
    return StringIterator_Next((StringIterator *)iterator);
}

const char * PathIterator_Get(const PathIterator * iterator)
{
    return StringIterator_Get((StringIterator *)iterator);
}


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


#ifndef DEFINE_COMMON_H
#define DEFINE_COMMON_H

#include "awa/common.h"

// Use a different commandline definition depending on the tool being built:
#ifdef TOOL_CLIENT
#  include "awa-client-define_cmdline.h"
#endif

#ifdef TOOL_SERVER
#  include "awa-server-define_cmdline.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct gengetopts_args_info;

typedef struct
{
    AwaResourceID ID;
    char * Name;
    AwaResourceType Type;
    int MinInstances;
    int MaxInstances;
    AwaResourceOperations Operations;
} ResourceDescription;

typedef struct
{
    AwaObjectID ID;
    char * Name;
    int MinInstances;
    int MaxInstances;
    size_t NumResources;
    ResourceDescription ** ResourceDescription;
} ObjectDescription;


AwaResourceType ResourceTypeFromEnum(enum enum_resourceType value, bool isArray);

AwaResourceOperations OperationsFromEnum(enum enum_resourceOperations value);

ResourceDescription * ResourceDescription_NewFromCmdline(struct gengetopt_args_info * ai, int index);

void ResourceDescription_Free(ResourceDescription ** resourceDescription);

ObjectDescription * ObjectDescription_NewFromCmdline(struct gengetopt_args_info * ai);

void ObjectDescription_Free(ObjectDescription ** objectDescription);

AwaError AddResource(AwaObjectDefinition * objectDefinition, const ResourceDescription * resourceDescription);

AwaObjectDefinition * DefineObject(const ObjectDescription * objectDescription);


#ifdef __cplusplus
}
#endif

#endif // DEFINE_COMMON_H




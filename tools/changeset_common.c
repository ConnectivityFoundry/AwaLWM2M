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

#include "changeset_common.h"
#include "tools_common.h"

typedef struct
{
    AwaChangeType ChangeType;
    char* ChangeStr;
} ChangeTypeEnumMapEntry;

static ChangeTypeEnumMapEntry changeTypeToStringMap[] =
{
    { AwaChangeType_ResourceCreated,          "Resource Created" },
    { AwaChangeType_ResourceModified,         "Resource Modified"},
    { AwaChangeType_ResourceDeleted,          "Resource Deleted" },
    { AwaChangeType_ObjectInstanceCreated,    "Object Instance Created" },
    { AwaChangeType_ObjectInstanceModified,   "Object Instance Modified"},
    { AwaChangeType_ObjectInstanceDeleted,    "Object Instance Deleted"},
    { AwaChangeType_Current,                  "Current Value"}
};

void PrintChanged(AwaChangeType  changeType, const char * path)
{
    int i;
    for (i = 0; i < sizeof(changeTypeToStringMap) / sizeof(changeTypeToStringMap[0]); ++i)
    {
        if (changeType == changeTypeToStringMap[i].ChangeType)
        {
            Verbose("Changed: %s %s:\n", path, changeTypeToStringMap[i].ChangeStr);
            break;
        }
    }
}

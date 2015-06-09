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


#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>

#include "utils.h"
#include "memalloc.h"
#include "awa/common.h"
#include "log.h"

size_t msprintf(char ** string, const char * format, ...)
{
    int len = 0;
    if (string != NULL)
    {
        *string = NULL;

        {
            va_list args;
            va_start(args, format);
            len = vsnprintf(NULL, 0, format, args);
            va_end(args);
        }

        if (len > 0)
        {
            *string = Awa_MemAlloc(len + 1);
            if (*string != NULL)
            {
                va_list args;
                va_start(args, format);
                len = vsnprintf(*string, len + 1, format, args);
                va_end(args);
            }
        }
    }
    return (len > 0) ? len : 0;
}

// This table must align with awa/common.h:AwaResourceType
static const char * ResourceTypeStrings[] =
{
        "Invalid",
        "None",
        "String",
        "Integer",
        "Float",
        "Boolean",
        "Opaque",
        "Time",
        "ObjectLink",
        "String Array",
        "Integer Array",
        "Float Array",
        "Boolean Array",
        "Opaque Array",
        "Time Array",
        "ObjectLink Array"
};

// Check first and last Number of entries in table is same as last enum
const char * Utils_ResourceTypeToString(AwaResourceType resourceType)
{
    static const char * result = "Unknown Resource Type";
    size_t numEntries = Utils_GetNumberOfResourceTypeStrings();

    if ((resourceType >= 0) && (resourceType < numEntries))
    {
        result = ResourceTypeStrings[resourceType];
    }
    return result;
}

size_t Utils_GetNumberOfResourceTypeStrings(void)
{
    size_t numEntries = sizeof(ResourceTypeStrings) / sizeof(ResourceTypeStrings[0]);
    return numEntries;
}

ResourceTypeType Utils_GetResourceType(AwaResourceType resourceType)
{
    ResourceTypeType result = ResourceTypeEnum_TypeInvalid;

    switch (resourceType)
    {
        case AwaResourceType_Invalid:
            result = ResourceTypeEnum_TypeInvalid;
            break;

        case AwaResourceType_None:
            result = ResourceTypeEnum_TypeNone;
            break;

        case AwaResourceType_StringArray:
        case AwaResourceType_String:
            result = ResourceTypeEnum_TypeString;
            break;

        case AwaResourceType_IntegerArray:
        case AwaResourceType_Integer:
            result = ResourceTypeEnum_TypeInteger;
            break;

        case AwaResourceType_FloatArray:
        case AwaResourceType_Float:
            result = ResourceTypeEnum_TypeFloat;
            break;

        case AwaResourceType_BooleanArray:
        case AwaResourceType_Boolean:
            result = ResourceTypeEnum_TypeBoolean;
            break;

        case AwaResourceType_OpaqueArray:
        case AwaResourceType_Opaque:
            result = ResourceTypeEnum_TypeOpaque;
            break;

        case AwaResourceType_TimeArray:
        case AwaResourceType_Time:
            result = ResourceTypeEnum_TypeTime;
            break;

        case AwaResourceType_ObjectLinkArray:
        case AwaResourceType_ObjectLink:
            result = ResourceTypeEnum_TypeObjectLink;
            break;

        default:
            LogError("Invalid resource type %d", resourceType);
            break;
    }

    return result;
}

// NB: Does not support arrays!
AwaResourceType Utils_GetAwaResourceType(ResourceTypeType resourceType)
{
    AwaResourceType result = AwaResourceType_Invalid;

    switch (resourceType)
    {
        case ResourceTypeEnum_TypeInvalid:
            result = AwaResourceType_Invalid;
            break;

        case ResourceTypeEnum_TypeNone:
            result = AwaResourceType_None;
            break;

        case ResourceTypeEnum_TypeString:
            result = AwaResourceType_String;
            break;

        case ResourceTypeEnum_TypeInteger:
            result = AwaResourceType_Integer;
            break;

        case ResourceTypeEnum_TypeFloat:
            result = AwaResourceType_Float;
            break;

        case ResourceTypeEnum_TypeBoolean:
            result = AwaResourceType_Boolean;
            break;

        case ResourceTypeEnum_TypeOpaque:
            result = AwaResourceType_Opaque;
            break;

        case ResourceTypeEnum_TypeTime:
            result = AwaResourceType_Time;
            break;

        case ResourceTypeEnum_TypeObjectLink:
            result = AwaResourceType_ObjectLink;
            break;

        default:
            LogError("Invalid resource type %d", resourceType);
            break;
    }

    return result;
}

Operations Utils_GetResourceTypeOperations(AwaResourceOperations operation)
{
    Operations result = Operations_None;

    switch (operation)
    {
        case AwaResourceOperations_None:
            result = Operations_None;
            break;
        case AwaResourceOperations_ReadOnly:
            result = Operations_R;
            break;
        case AwaResourceOperations_WriteOnly:
            result = Operations_W;
            break;
        case AwaResourceOperations_ReadWrite:
            result = Operations_RW;
            break;
        case AwaResourceOperations_Execute:
            result = Operations_E;
            break;
        default:
            LogError("Invalid operation %d", operation);
            result = Operations_None;
            break;
    }

    return result;
}


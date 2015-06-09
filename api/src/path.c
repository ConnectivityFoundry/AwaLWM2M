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

#include "awa/common.h"
#include "path.h"
#include "log.h"
#include "utils.h"
#include "memalloc.h"

#define OBJECT_MATCH 1
#define OBJECT_INSTANCE_MATCH 2
#define RESOURCE_MATCH 3

bool Path_IsIDValid(int ID)
{
    return ((ID >= 0) && (ID <= AWA_MAX_ID));
}

bool Path_Parse(const char * path, int * _matches, AwaObjectID * _objectID, AwaObjectInstanceID * _objectInstanceID, AwaResourceID * _resourceID)
{
    bool result = false;

    int matches = 0;
    AwaObjectID objectID = AWA_INVALID_ID;
    AwaObjectInstanceID objectInstanceID = AWA_INVALID_ID;
    AwaResourceID resourceID = AWA_INVALID_ID;

    if (path != NULL)
    {
        matches = sscanf(path, "/%10d/%10d/%10d", &objectID, &objectInstanceID, &resourceID);

        char * compare = NULL;
        switch (matches)
        {
        case OBJECT_MATCH:
            if (Path_IsIDValid(objectID))
            {
                msprintf(&compare, "/%d", objectID);
            }
            break;
        case OBJECT_INSTANCE_MATCH:
            if (Path_IsIDValid(objectID) && Path_IsIDValid(objectInstanceID))
            {
                msprintf(&compare, "/%d/%d", objectID, objectInstanceID);
            }
            break;
        case RESOURCE_MATCH:
            if (Path_IsIDValid(objectID) && Path_IsIDValid(objectInstanceID) && Path_IsIDValid(resourceID))
            {
                msprintf(&compare, "/%d/%d/%d", objectID, objectInstanceID, resourceID);
            }
            break;
        case EOF:  // no-break
        default:
            break;
        }

        if (compare != NULL)
        {
            // compare a rebuilt string with the original. Path is valid if they match.
            result = (strcmp(path, compare) == 0);
            Awa_MemSafeFree(compare);
        }

        if (result == false)
        {
            // clear results on failure
            matches = 0;
            objectID = AWA_INVALID_ID;
            objectInstanceID = AWA_INVALID_ID;
            resourceID = AWA_INVALID_ID;
        }
    }

    if (_matches != NULL)
    {
        *_matches = matches;
    }
    if (_objectID != NULL)
    {
        *_objectID = objectID;
    }
    if (_objectInstanceID != NULL)
    {
        *_objectInstanceID = objectInstanceID;
    }
    if (_resourceID != NULL)
    {
        *_resourceID = resourceID;
    }

    return result;
}

bool Path_IsValid(const char * path)
{
    bool result = false;

    // Path must be strictly of the form "/X", "/X/X" or "/X/X/X",
    // where X is numerical between 0 and 65535.

    if (path != NULL)
    {
        AwaObjectID objectID = 0;
        AwaObjectInstanceID objectInstanceID = 0;
        AwaResourceID resourceID = 0;
        int matches = 0;

        result = Path_Parse(path, &matches, &objectID, &objectInstanceID, &resourceID);
    }

    if (result == false)
    {
        LogDebug("Invalid path: %s", path);
    }
    return result;
}

bool Path_IsValidForObject(const char * path)
{
    // Path must be strictly of the form "/X"
    bool result = false;

    if (path != NULL)
    {
        AwaObjectID objectID = 0;
        int matches = 0;

        if (Path_Parse(path, &matches, &objectID, NULL, NULL) != false)
        {
            result = (matches == OBJECT_MATCH);
        }
    }
    return result;
}

bool Path_IsValidForObjectInstance(const char * path)
{
    // Path must be strictly of the form "/X/X"
    bool result = false;

    if (path != NULL)
    {
        AwaObjectID objectID = 0;
        AwaObjectInstanceID objectInstanceID = 0;
        int matches = 0;

        if (Path_Parse(path, &matches, &objectID, &objectInstanceID, NULL) != false)
        {
            result = (matches == OBJECT_INSTANCE_MATCH);
        }
    }
    return result;
}

bool Path_IsValidForResource(const char * path)
{
    // Path must be strictly of the form "/X/X/X"
    bool result = false;

    if (path != NULL)
    {
        AwaObjectID objectID = 0;
        AwaObjectInstanceID objectInstanceID = 0;
        AwaResourceID resourceID = 0;
        int matches = 0;

        if (Path_Parse(path, &matches, &objectID, &objectInstanceID, &resourceID) != false)
        {
            result = (matches == RESOURCE_MATCH);
        }
    }
    return result;
}

AwaObjectID Path_GetObjectID(const char * path)
{
    AwaObjectID objectID = 0;
    Path_Parse(path, NULL, &objectID, NULL, NULL);
    return objectID;
}

AwaObjectInstanceID Path_GetObjectInstanceID(const char * path)
{
    AwaObjectInstanceID objectInstanceID = 0;
    Path_Parse(path, NULL, NULL, &objectInstanceID, NULL);
    return objectInstanceID;
}

AwaResourceID Path_GetResourceID(const char * path)
{
    AwaResourceID resourceID = 0;
    Path_Parse(path, NULL, NULL, NULL, &resourceID);
    return resourceID;
}

AwaError Path_MakePath(char * path, size_t pathSize, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID)
{
    AwaError result = AwaError_Unspecified;

    if (path != NULL)
    {
        // determine the format string
        const char * format = NULL;

        bool objectIDValid = Path_IsIDValid(objectID);
        bool objectInstanceIDValid = Path_IsIDValid(objectInstanceID);
        bool resourceIDValid = Path_IsIDValid(resourceID);

        if (objectIDValid && objectInstanceIDValid && resourceIDValid)
        {
            format = "/%d/%d/%d";
        }
        else if (objectIDValid && objectInstanceIDValid && !resourceIDValid)
        {
            format = "/%d/%d";
        }
        else if (objectIDValid && !objectInstanceIDValid && !resourceIDValid)
        {
            format = "/%d";
        }
        else
        {
            result = AwaError_IDInvalid;
            if (!objectIDValid)
            {
                LogErrorWithEnum(AwaError_IDInvalid, "Invalid Object ID %d", objectID);
            }
            if (!objectInstanceIDValid)
            {
                LogErrorWithEnum(AwaError_IDInvalid, "Invalid Object Instance ID %d", objectInstanceID);
            }
            if (!resourceIDValid)
            {
                LogErrorWithEnum(AwaError_IDInvalid, "Invalid Resource ID %d", resourceID);
            }
            path[0] = '\0';
        }

        if (format != NULL)
        {
            int len = snprintf(path, pathSize, format, objectID, objectInstanceID, resourceID);

            if (len >= pathSize)
            {
                result = LogErrorWithEnum(AwaError_Overrun, "String overrun");
                path[0] = '\0';
            }
            else
            {
                result = AwaError_Success;
            }
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_PathInvalid, "Path is NULL");
    }

    return result;
}

AwaError AwaAPI_MakeObjectPath(char * path, size_t pathSize, AwaObjectID objectID)
{
    AwaError result = AwaError_Unspecified;
    if (Path_IsIDValid(objectID))
    {
        result = Path_MakePath(path, pathSize, objectID, AWA_INVALID_ID, AWA_INVALID_ID);
    }
    else
    {
        result = LogErrorWithEnum(AwaError_IDInvalid, "Invalid Object ID specified");
    }
    return result;
}

AwaError AwaAPI_MakeObjectInstancePath(char * path, size_t pathSize, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID)
{
    AwaError result = AwaError_Unspecified;
    if (Path_IsIDValid(objectID))
    {
        if (Path_IsIDValid(objectInstanceID))
        {
            result = Path_MakePath(path, pathSize, objectID, objectInstanceID, AWA_INVALID_ID);
        }
        else
        {
            result = LogErrorWithEnum(AwaError_IDInvalid, "Invalid Object ID specified");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_IDInvalid, "Invalid Object Instance ID specified");
    }
    return result;
}

AwaError AwaAPI_MakeResourcePath(char * path, size_t pathSize, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID)
{
    AwaError result = AwaError_Unspecified;
    if (Path_IsIDValid(objectID))
    {
        if (Path_IsIDValid(objectInstanceID))
        {
            if (Path_IsIDValid(resourceID))
            {
                result = Path_MakePath(path, pathSize, objectID, objectInstanceID, resourceID);
            }
            else
            {
                result = LogErrorWithEnum(AwaError_IDInvalid, "Invalid Object ID specified");
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_IDInvalid, "Invalid Object Instance ID specified");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_IDInvalid, "Invalid Resource ID specified");
    }
    return result;
}

AwaError AwaAPI_MakePath(char * path, size_t pathSize, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID)
{
    return Path_MakePath(path, pathSize, objectID, objectInstanceID, resourceID);
}

bool AwaAPI_IsPathValid(const char * path)
{
    return Path_IsValid(path);
}


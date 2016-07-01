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
#include <inttypes.h>
#include <signal.h>

#include "tools_common.h"
#include "b64.h"


int g_logLevel = 0;
bool g_signal = false;


// Borrowed from the API library
size_t msprintf2(char ** string, const char * format, ...)
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
            *string = malloc(len + 1);
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

void Log(int level, FILE * outFile, const char * format, ...)
{
    if (g_logLevel >= level)
    {
        va_list args;
        outFile = outFile == NULL ? stdout : outFile;
        va_start(args, format);
        vfprintf(outFile, format, args);
        va_end(args);
        fflush(outFile);
    }
}

const char * OirToPath(char * path, size_t pathLen, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID)
{
    // AwaAPI_MakePath will return an empty path for most errors:
    AwaAPI_MakePath(path, pathLen, objectID, objectInstanceID, resourceID);
    return path;
}

Target * CreateTarget(const char * arg)
{
    Target * target = NULL;

    if (!IsTargetValid(arg))
    {
        Error("Target %s is not valid\n", arg);
        return NULL;
    }

    AwaObjectID objectID = AWA_INVALID_ID;
    AwaObjectInstanceID objectInstanceID = AWA_INVALID_ID;
    AwaResourceID resourceID = AWA_INVALID_ID;
    int resourceInstanceID = -1;

    sscanf(arg, "/%10d/%10d/%10d/%10d", &objectID, &objectInstanceID, &resourceID, &resourceInstanceID);

    char path[256] = { 0 };
    size_t pathSize = 256;
    if (AwaAPI_MakePath(path, pathSize, objectID, objectInstanceID, resourceID) == AwaError_Success)
    {
        if (AwaAPI_IsPathValid(path))
        {
            target = malloc(sizeof(*target));
            if (target != NULL)
            {
                memset(target, 0, sizeof(*target));
                target->Path = strdup(path);
                target->ResourceInstanceID = resourceInstanceID;
            }
            else
            {
                Error("Out of memory\n");
            }
        }
        else
        {
            Error("Path %s is not valid\n", path);
        }
    }
    else
    {
        Error("Target %s is not valid\n", arg);
    }
    return target;
}

void FreeTarget(Target ** targetPtr)
{
    if ((targetPtr != NULL) && (*targetPtr != NULL))
    {
        free((*targetPtr)->Path);
        (*targetPtr)->Path = NULL;
        free(*targetPtr);
        *targetPtr = NULL;
    }
}

bool IsIDValid(int id)
{
    // An ID is valid if it is >= 0, or < 0 (including the value FLOW_DEVICE_MGMT_KEY_ID_NOT_USED)
    return (id >= 0) && (id <= AWA_MAX_ID);
}

const char * ResourceTypeToString(AwaResourceType type)
{
    const char * result = "BAD TYPE";
    const char * table[] = {
            "Invalid",
            "None",
            "String",
            "Integer",
            "Float",
            "Boolean",
            "Opaque",
            "Time",
            "ObjectLink",
            "StringArray",
            "IntegerArray",
            "FloatArray",
            "BooleanArray",
            "OpaqueArray",
            "TimeArray",
            "ObjectLinkArray",
    };

    if (sizeof(table) / sizeof(table[0]) != AwaResourceType_LAST)
    {
        Error("ResourceTypeToString table is wrong size!\n");
    }
    else
    {
        if ((type >= 0) && (type < AwaResourceType_LAST))
        {
            result = table[type];
        }
    }
    return result;
}

const char * DeviceServerXML_ResourceTypeToString(AwaResourceType type)
{
    const char * result = "BAD TYPE";
    const char * table[] = {
            "Invalid",
            "None",
            "String",
            "Integer",
            "Float",
            "Boolean",
            "Opaque",
            "DateTime",
            "ObjectLink",
            "String",
            "Integer",
            "Float",
            "Boolean",
            "Opaque",
            "DateTime",
            "ObjectLink",
    };

    if (sizeof(table) / sizeof(table[0]) != AwaResourceType_LAST)
    {
        Error("ResourceTypeToString table is wrong size!\n");
    }
    else
    {
        if ((type >= 0) && (type < AwaResourceType_LAST))
        {
            result = table[type];
        }
    }
    return result;
}

const char * ResourceOperationToString(AwaResourceOperations operation)
{
    const char * result = "BAD OPERATION";
    const char * table[] = {
            "Invalid",
            "None",
            "ReadOnly",
            "WriteOnly",
            "ReadWrite",
            "Execute",
    };

    if (sizeof(table) / sizeof(table[0]) != AwaResourceOperations_LAST + 1)
    {
        Error("ResourceOperationToString table is wrong size!");
    }
    else
    {
        operation += 1; //AwaResourceOperations_Invalid does not start at 0, so offset required.
        if ((operation >= 0) && (operation < AwaResourceOperations_LAST + 1))
        {
            result = table[operation];
        }
    }
    return result;
}

const char * DeviceServerXML_ResourceOperationToString(AwaResourceOperations operation)
{
    const char * result = "BAD OPERATION";
    const char * table[] = {
            "Invalid",
            "None",
            "Read",
            "Write",
            "ReadWrite",
            "Execute",
    };

    if (sizeof(table) / sizeof(table[0]) != AwaResourceOperations_LAST + 1)
    {
        Error("ResourceOperationToString table is wrong size!");
    }
    else
    {
        operation += 1; //AwaResourceOperations_Invalid does not start at 0, so offset required.
        if ((operation >= 0) && (operation < AwaResourceOperations_LAST + 1))
        {
            result = table[operation];
        }
    }
    return result;
}

bool IsArrayType(AwaResourceType type)
{
    return (type >= AwaResourceType_StringArray) && (type <= AwaResourceType_ObjectLinkArray);
}

bool IsTargetValid(const char * target)
{
    // The following patterns are valid as command-line targets:
    //
    //  /\d+
    //  /\d+/\d+
    //  /\d+/\d+/\d+(=|=(.*))?
    //  /\d+/\d+/\d+/\d+(=|=(.*))?
    //
    // Anything else is invalid

    typedef enum
    {
        Start,
        ObjectIDSlash,
        ObjectIDDigit,
        ObjectInstanceIDSlash,
        ObjectInstanceIDDigit,
        ResourceIDSlash,
        ResourceIDDigit,
        ResourceInstanceIDSlash,
        ResourceInstanceIDDigit,
        Equals,
        Value,
        Error,
    } State;

    State state = Start;

    // Acceptor:
    const char * c = &target[0];
    while (*c != '\0')
    {
        switch (state)
        {
            case Start:
                if (*c == '/')
                    state = ObjectIDSlash;
                else
                    state = Error;
                break;
            case ObjectIDSlash:
                if (*c >= '0' && *c <= '9')
                    state = ObjectIDDigit;
                else
                    state = Error;
                break;
            case ObjectIDDigit:
                if (*c >= '0' && *c <= '9')
                    ; // Remain
                else if (*c == '/')
                    state = ObjectInstanceIDSlash;
                else
                    state = Error;
                break;
            case ObjectInstanceIDSlash:
                if (*c >= '0' && *c <= '9')
                    state = ObjectInstanceIDDigit;
                else
                    state = Error;
                break;
            case ObjectInstanceIDDigit:
                if (*c >= '0' && *c <= '9')
                    ; // Remain
                else if (*c == '/')
                    state = ResourceIDSlash;
                else
                    state = Error;
                break;
            case ResourceIDSlash:
                if (*c >= '0' && *c <= '9')
                    state = ResourceIDDigit;
                else
                    state = Error;
                break;
            case ResourceIDDigit:
                if (*c >= '0' && *c <= '9')
                    ; // Remain
                else if (*c == '/')
                    state = ResourceInstanceIDSlash;
                else if (*c == '=')
                    state = Equals;
                else
                    state = Error;
                break;
            case ResourceInstanceIDSlash:
                if (*c >= '0' && *c <= '9')
                    state = ResourceInstanceIDDigit;
                else
                    state = Error;
                break;
            case ResourceInstanceIDDigit:
                if (*c >= '0' && *c <= '9')
                    ; // Remain
                else if (*c == '=')
                    state = Equals;
                else
                    state = Error;
                break;
            case Equals:
                state = Value;
                break;
            case Value:
                // Accept anything
                break;
            case Error:
                // Terminate early
                return false;
                break;
            default:
                Error("Target acceptor failed\n");
                return false;
        }
        ++c;
    }

    bool accept =
            (state == ObjectIDDigit) ||
            (state == ObjectInstanceIDDigit) ||
            (state == ResourceIDDigit) ||
            (state == ResourceInstanceIDDigit) ||
            (state == Equals) || (state == Value);

    return accept;
}

AwaClientSession * Client_EstablishSession(const char * address, unsigned int port)
{
    // Initialise Device Management session
    AwaClientSession * session;
    session = AwaClientSession_New();

    if (session != NULL)
    {
        if (AwaClientSession_SetIPCAsUDP(session, address, port) == AwaError_Success)
        {
            if (AwaClientSession_Connect(session) == AwaError_Success)
            {
                Debug("Session Established\n");
            }
            else
            {
                Error("AwaClientSession_Connect() failed\n");
                AwaClientSession_Free(&session);
            }
        }
        else
        {
            Error("AwaClientSession_SetIPCAsUDP() failed\n");
            AwaClientSession_Free(&session);
        }
    }
    else
    {
        Error("AwaClientSession_New() failed\n");
    }
    return session;
}

void Client_ReleaseSession(AwaClientSession ** session)
{
    if (AwaClientSession_Disconnect(*session) != AwaError_Success)
    {
        Error("AwaClientSession_Disconnect() failed\n");
    }

    if (AwaClientSession_Free(session) != AwaError_Success)
    {
        Error("AwaClientSession_Free() failed\n");
    }
}

char * Client_GetValue(const AwaClientSession * session, const Target * target, const char * arg)
{
    char * value = NULL;

    if (target != NULL)
    {
        if (arg != NULL)
        {
            // Only resources can be set
            AwaResourceID resourceID = AWA_INVALID_ID;
            AwaClientSession_PathToIDs(session, target->Path, NULL, NULL, &resourceID);
            if (resourceID == AWA_INVALID_ID)
            {
                Error("Resource or Resource Instance must be specified: %s\n", arg);
            }
            else
            {
                const char * valueStr = strchr(arg, '=');
                if (valueStr == NULL)
                {
                    Error("A value must be specified: %s\n", arg);
                }
                else
                {
                    valueStr++;  // Skip the '=' character
                    int valueLen = strlen(valueStr);
                    if (valueLen == 0)
                    {
                        Error("A value must be specified: %s\n", arg);
                    }
                    else
                    {
                        value = malloc(sizeof(char) * valueLen + 1);
                        if (value != NULL)
                        {
                            strncpy(value, valueStr, valueLen + 1);  // Add 1 to accommodate the null terminator otherwise strncpy doesn't add one
                        }
                        else
                        {
                            Error("Out of memory\n");
                        }
                    }
                }
            }
        }
        else
        {
            Error("arg is NULL\n");
        }
    }
    else
    {
        Error("target is NULL\n");
    }

    return value;
}

bool Client_IsObjectTarget(const AwaClientSession * session, const Target * target)
{
    bool result = false;
    AwaObjectID objectID = AWA_INVALID_ID;
    AwaObjectInstanceID objectInstanceID = AWA_INVALID_ID;
    AwaResourceID resourceID = AWA_INVALID_ID;

    if (target != NULL)
    {
        if (AwaClientSession_PathToIDs(session, target->Path, &objectID, &objectInstanceID, &resourceID) == AwaError_Success)
        {
            result = IsIDValid(objectID) && !IsIDValid(objectInstanceID) && !IsIDValid(resourceID) && !IsIDValid(target->ResourceInstanceID);
        }
    }
    else
    {
        Error("target is NULL\n");
    }
    return result;
}

bool Client_IsObjectInstanceTarget(const AwaClientSession * session, const Target * target)
{
    bool result = false;
    AwaObjectID objectID = AWA_INVALID_ID;
    AwaObjectInstanceID objectInstanceID = AWA_INVALID_ID;
    AwaResourceID resourceID = AWA_INVALID_ID;

    if (target != NULL)
    {
        if (AwaClientSession_PathToIDs(session, target->Path, &objectID, &objectInstanceID, &resourceID) == AwaError_Success)
        {
            result = IsIDValid(objectID) && IsIDValid(objectInstanceID) && !IsIDValid(resourceID) && !IsIDValid(target->ResourceInstanceID);
        }
    }
    else
    {
        Error("target is NULL\n");
    }
    return result;
}

bool Client_IsResourceTarget(const AwaClientSession * session, const Target * target)
{
    bool result = false;
    AwaObjectID objectID = AWA_INVALID_ID;
    AwaObjectInstanceID objectInstanceID = AWA_INVALID_ID;
    AwaResourceID resourceID = AWA_INVALID_ID;

    if (target != NULL)
    {
        if (AwaClientSession_PathToIDs(session, target->Path, &objectID, &objectInstanceID, &resourceID) == AwaError_Success)
        {
            result = IsIDValid(objectID) && IsIDValid(objectInstanceID) && IsIDValid(resourceID) && !IsIDValid(target->ResourceInstanceID);
        }
    }
    else
    {
        Error("target is NULL\n");
    }
    return result;
}

bool Client_IsResourceInstanceTarget(const AwaClientSession * session, const Target * target)
{
    bool result = false;
    AwaObjectID objectID = AWA_INVALID_ID;
    AwaObjectInstanceID objectInstanceID = AWA_INVALID_ID;
    AwaResourceID resourceID = AWA_INVALID_ID;

    if (target != NULL)
    {
        if (AwaClientSession_PathToIDs(session, target->Path, &objectID, &objectInstanceID, &resourceID) == AwaError_Success)
        {
            result = IsIDValid(objectID) && IsIDValid(objectInstanceID) && IsIDValid(resourceID) && IsIDValid(target->ResourceInstanceID);
        }
    }
    else
    {
        Error("target is NULL\n");
    }
    return result;
}

bool Client_IsResourceDefined(const AwaClientSession * session, const char * path)
{
    return Client_GetResourceDefinition(session, path) != NULL;
}

const AwaResourceDefinition * Client_GetResourceDefinition(const AwaClientSession * session, const char * path)
{
    AwaObjectID objectID;
    AwaResourceID resourceID;
    const AwaResourceDefinition * resourceDefinition = NULL;

    if (AwaClientSession_PathToIDs(session, path, &objectID, NULL, &resourceID) == AwaError_Success)
    {
        const AwaObjectDefinition * objectDefinition = AwaClientSession_GetObjectDefinition(session, objectID);
        if (objectDefinition != NULL)
        {
            resourceDefinition = AwaObjectDefinition_GetResourceDefinition(objectDefinition, resourceID);
        }
        else
        {
            Debug("objectDefinition is NULL\n");
        }
    }
    else
    {
        Error("AwaClientSession_PathToIDs() failed\n");
    }
    return resourceDefinition;
}

AwaResourceType Client_GetResourceType(const AwaClientSession * session, const char * path)
{
    AwaResourceType resourceType = AwaResourceType_Invalid;
    const AwaResourceDefinition * resourceDefinition = Client_GetResourceDefinition(session, path);
    if (resourceDefinition != NULL)
    {
        resourceType = AwaResourceDefinition_GetType(resourceDefinition);
    }
    else
    {
        Error("resourceDefinition is NULL\n");
        resourceType = AwaResourceType_Invalid;
    }
    return resourceType;
}

AwaResourceOperations Client_GetResourceOperations(const AwaClientSession * session, const char * path)
{
    AwaResourceOperations operations = AwaResourceOperations_Invalid;
    const AwaResourceDefinition * resourceDefinition = Client_GetResourceDefinition(session, path);
    if (resourceDefinition != NULL)
    {
        operations = AwaResourceDefinition_GetSupportedOperations(resourceDefinition);
    }
    else
    {
        Error("resourceDefinition is NULL\n");
        operations = AwaResourceOperations_Invalid;
    }
    return operations;
}

bool Client_IsMultiValuedResource(const AwaClientSession * session, const char * path)
{
    bool result = false;
    const AwaResourceDefinition * resourceDefinition = Client_GetResourceDefinition(session, path);
    if (resourceDefinition != NULL)
    {
        result = AwaResourceDefinition_GetMaximumInstances(resourceDefinition) > 1;
    }
    else
    {
        Error("resourceDefinition is NULL\n");
        result = false;
    }
    return result;
}

static void AddBanner(char ** cstring, char ** resourceValue, const char * path, const char * objectName, const char * resourceName,
                      AwaObjectID objectID, AwaObjectID * lastObjectID, AwaObjectID objectInstanceID, AwaObjectID * lastObjectInstanceID,
                      AwaArrayIndex resourceArrayIndex, bool quiet)
{
    // Insert a banner for each new object or instance
    if ((!quiet) && ((objectID != *lastObjectID) || (objectInstanceID != *lastObjectInstanceID)))
    {
        char * tmp = NULL;
        msprintf2(&tmp, "%s%s[/%d/%d]:\n", *cstring, objectName, objectID, objectInstanceID);
        free(*cstring);
        *cstring = tmp;
    }

    char * tmp = NULL;
    if (quiet)
    {
        msprintf2(&tmp, "%s%s\n", *cstring, *resourceValue);
    }
    else
    {
        if (resourceArrayIndex == -1)
        {
            msprintf2(&tmp, "%s    %s[%s]: %s\n", *cstring, resourceName, path, *resourceValue);
        }
        else
        {
            msprintf2(&tmp, "%s    %s[%s/%d]: %s\n", *cstring, resourceName, path, resourceArrayIndex, *resourceValue);
        }
    }

    free(*cstring);
    *cstring = tmp;
    free(*resourceValue);
    *resourceValue = NULL;

    *lastObjectID = objectID;
    *lastObjectInstanceID = objectInstanceID;
}

static char * EncodeOpaque(AwaOpaque * value)
{
    enum { OPAQUE_DISPLAY_LEN = 32 };
    enum { OPAQUE_BUFFER_LEN = (OPAQUE_DISPLAY_LEN * 3) + 32 }; // 3 bytes per character, plus extra for header
    char * resourceValue = NULL;

    if (value != NULL)
    {
        resourceValue = (char *)malloc(OPAQUE_BUFFER_LEN);
        if (resourceValue != NULL)
        {
            memset(resourceValue, 0, OPAQUE_BUFFER_LEN);

            // Print opaque in hex, but truncate the output and place continuation characters on the end.
            // cppcheck-suppress redundantCopy
            snprintf(resourceValue, OPAQUE_BUFFER_LEN, "Opaque (%zu):", value->Size);
            int i;
            for (i = 0; i < value->Size; i++)
            {
                if ((i <= OPAQUE_DISPLAY_LEN) || (i == value->Size - 1))
                {
                    char hexValue[4] = {0};
                    snprintf(hexValue, 4, "%02X ", ((uint8_t*)value->Data)[i]);
                    strcat(resourceValue, hexValue);
                }
                else
                {
                    strcat(resourceValue, "...");
                    break;
                }
            }
        }
    }
    else
    {
        Error("value is NULL\n");
    }
    return resourceValue;
}

static AwaError AddPathToCString(char ** cstring, const char * path, void * response, ResponseType responseType, bool quiet,
                                  AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, int resourceInstanceID,
                                  AwaObjectID * lastObjectID, AwaObjectInstanceID * lastObjectInstanceID,
                                  const AwaObjectDefinition * objectDefinition, const AwaResourceDefinition * resourceDefinition)
{
    AwaError result = AwaError_Unspecified;
    const AwaClientGetResponse * getResponse = NULL;
    const AwaChangeSet * changeSet = NULL;
    const AwaServerReadResponse * readResponse = NULL;

    switch (responseType)
    {
        case ResponseType_GetResponse:
            getResponse = (const AwaClientGetResponse *)response;
            break;
        case ResponseType_ChangeSet:
            changeSet = (const AwaChangeSet *)response;
            break;
        case ResponseType_ReadResponse:
            readResponse = (const AwaServerReadResponse *)response;
            break;
        default:
            Error("Unhandled response type");
            result = AwaError_Unsupported;
            response = NULL;
            break;
    }
    if (response != NULL)
    {
        const char * objectName = AwaObjectDefinition_GetName(objectDefinition);

        if (objectInstanceID == AWA_INVALID_ID)
        {
            // no instances, so no resources either:
            char * tmp = NULL;
            msprintf2(&tmp, "%s%s[/%d]: No instances\n", *cstring, objectName, objectID);
            free(*cstring);
            *cstring = tmp;
        }
        else if (resourceID == AWA_INVALID_ID)
        {
            // instance exists, but no resources created:
            char * tmp = NULL;
            msprintf2(&tmp, "%s%s[/%d/%d]:\n    No resources\n", *cstring, objectName, objectID, objectInstanceID);
            free(*cstring);
            *cstring = tmp;
        }
        else
        {
            const char * resourceName = AwaResourceDefinition_GetName(resourceDefinition);

            char * resourceValue = NULL;
            AwaResourceType resourceType = AwaResourceDefinition_GetType(resourceDefinition);
            switch (resourceType)
            {
                case AwaResourceType_String:
                {
                    const char * value = NULL;
                    switch (responseType)
                    {
                        case ResponseType_GetResponse:
                            result = AwaClientGetResponse_GetValueAsCStringPointer(getResponse, path, &value);
                            break;
                        case ResponseType_ChangeSet:
                            result = AwaChangeSet_GetValueAsCStringPointer(changeSet, path, &value);
                            break;
                        case ResponseType_ReadResponse:
                            result = AwaServerReadResponse_GetValueAsCStringPointer(readResponse, path, &value);
                            break;
                        default:
                            Error("Unhandled response type");
                            result = AwaError_Unsupported;
                            break;
                    }

                    if (result == AwaError_Success)
                    {
                        resourceValue = strdup(value);
                    }
                }
                break;

                case AwaResourceType_Integer:
                {
                    const AwaInteger * value = NULL;
                    switch (responseType)
                    {
                        case ResponseType_GetResponse:
                            result = AwaClientGetResponse_GetValueAsIntegerPointer(getResponse, path, &value);
                            break;
                        case ResponseType_ChangeSet:
                            result = AwaChangeSet_GetValueAsIntegerPointer(changeSet, path, &value);
                            break;
                        case ResponseType_ReadResponse:
                            result = AwaServerReadResponse_GetValueAsIntegerPointer(readResponse, path, &value);
                            break;
                        default:
                            Error("Unhandled response type");
                            result = AwaError_Unsupported;
                            break;
                    }
                    if (result == AwaError_Success)
                    {
                        msprintf2(&resourceValue, "%"PRId64, *value);
                    }
                }
                break;

                case AwaResourceType_Time:
                {
                    const AwaTime * value = NULL;
                    switch (responseType)
                    {
                        case ResponseType_GetResponse:
                            result = AwaClientGetResponse_GetValueAsTimePointer(getResponse, path, &value);
                            break;
                        case ResponseType_ChangeSet:
                            result = AwaChangeSet_GetValueAsTimePointer(changeSet, path, &value);
                            break;
                        case ResponseType_ReadResponse:
                            result = AwaServerReadResponse_GetValueAsTimePointer(readResponse, path, &value);
                            break;
                        default:
                            Error("Unhandled response type");
                            result = AwaError_Unsupported;
                            break;
                    }
                    if (result == AwaError_Success)
                    {
                        msprintf2(&resourceValue, "%"PRId64, *value);
                    }
                }
                break;

                case AwaResourceType_Float:
                {
                    const AwaFloat * value = NULL;
                    switch (responseType)
                    {
                        case ResponseType_GetResponse:
                            result = AwaClientGetResponse_GetValueAsFloatPointer(getResponse, path, &value);
                            break;
                        case ResponseType_ChangeSet:
                            result = AwaChangeSet_GetValueAsFloatPointer(changeSet, path, &value);
                            break;
                        case ResponseType_ReadResponse:
                            result = AwaServerReadResponse_GetValueAsFloatPointer(readResponse, path, &value);
                            break;
                        default:
                            Error("Unhandled response type");
                            result = AwaError_Unsupported;
                            break;
                    }
                    if (result == AwaError_Success)
                    {
                        // IEEE 754 supports up to 17 significant digits
                        msprintf2(&resourceValue, "%.17g", *value);
                    }
                }
                break;

                case AwaResourceType_Boolean:
                {
                    const AwaBoolean * value = NULL;
                    switch (responseType)
                    {
                        case ResponseType_GetResponse:
                            result = AwaClientGetResponse_GetValueAsBooleanPointer(getResponse, path, &value);
                            break;
                        case ResponseType_ChangeSet:
                            result = AwaChangeSet_GetValueAsBooleanPointer(changeSet, path, &value);
                            break;
                        case ResponseType_ReadResponse:
                            result = AwaServerReadResponse_GetValueAsBooleanPointer(readResponse, path, &value);
                            break;
                        default:
                            Error("Unhandled response type");
                            result = AwaError_Unsupported;
                            break;
                    }
                    if (result == AwaError_Success)
                    {
                        msprintf2(&resourceValue, "%s", *value != 0? "True" : "False");
                    }
                }
                break;

                case AwaResourceType_Opaque:
                {
                    AwaOpaque value = { 0 };
                    switch (responseType)
                    {
                        case ResponseType_GetResponse:
                            result = AwaClientGetResponse_GetValueAsOpaque(getResponse, path, &value);
                            break;
                        case ResponseType_ChangeSet:
                            result = AwaChangeSet_GetValueAsOpaque(changeSet, path, &value);
                            break;
                        case ResponseType_ReadResponse:
                            result = AwaServerReadResponse_GetValueAsOpaque(readResponse, path, &value);
                            break;
                        default:
                            Error("Unhandled response type");
                            result = AwaError_Unsupported;
                            break;
                    }
                    if (result == AwaError_Success)
                    {
                        resourceValue = EncodeOpaque(&value);
                    }
                }
                break;

                case AwaResourceType_ObjectLink:
                {
                    AwaObjectLink value = { 0 };
                    switch (responseType)
                    {
                        case ResponseType_GetResponse:
                            result = AwaClientGetResponse_GetValueAsObjectLink(getResponse, path, &value);
                            break;
                        case ResponseType_ChangeSet:
                            result = AwaChangeSet_GetValueAsObjectLink(changeSet, path, &value);
                            break;
                        case ResponseType_ReadResponse:
                            result = AwaServerReadResponse_GetValueAsObjectLink(readResponse, path, &value);
                            break;
                        default:
                            Error("Unhandled response type");
                            result = AwaError_Unsupported;
                            break;
                    }
                    if (result == AwaError_Success)
                    {
                        msprintf2(&resourceValue, "ObjectLink[%d:%d]", value.ObjectID, value.ObjectInstanceID);
                    }
                }
                break;

                case AwaResourceType_StringArray:
                {
                    const AwaStringArray * array = NULL;
                    switch (responseType)
                    {
                        case ResponseType_GetResponse:
                            result = AwaClientGetResponse_GetValuesAsStringArrayPointer(getResponse, path, &array);
                            break;
                        case ResponseType_ChangeSet:
                            result = AwaChangeSet_GetValuesAsStringArrayPointer(changeSet, path, &array);
                            break;
                        case ResponseType_ReadResponse:
                            result = AwaServerReadResponse_GetValuesAsStringArrayPointer(readResponse, path, &array);
                            break;
                        default:
                            Error("Unhandled response type");
                            result = AwaError_Unsupported;
                            break;
                    }
                    if (result == AwaError_Success)
                    {
                        AwaCStringArrayIterator * iterator = AwaStringArray_NewCStringArrayIterator(array);
                            while (AwaCStringArrayIterator_Next(iterator))
                        {
                            AwaArrayIndex index = AwaCStringArrayIterator_GetIndex(iterator);
                            if ((resourceInstanceID == index) || (resourceInstanceID == AWA_INVALID_ID))
                            {
                                const char * value = AwaCStringArrayIterator_GetValueAsCString(iterator);
                                resourceValue = strdup(value);
                                AddBanner(cstring, &resourceValue, path, objectName, resourceName, objectID, lastObjectID, objectInstanceID, lastObjectInstanceID, index, quiet);
                            }
                        }
                        AwaCStringArrayIterator_Free(&iterator);
                    }
                }
                break;

                case AwaResourceType_IntegerArray:
                {
                    const AwaIntegerArray * array = NULL;
                    switch (responseType)
                    {
                        case ResponseType_GetResponse:
                            result = AwaClientGetResponse_GetValuesAsIntegerArrayPointer(getResponse, path, &array);
                            break;
                        case ResponseType_ChangeSet:
                            result = AwaChangeSet_GetValuesAsIntegerArrayPointer(changeSet, path, &array);
                            break;
                        case ResponseType_ReadResponse:
                            result = AwaServerReadResponse_GetValuesAsIntegerArrayPointer(readResponse, path, &array);
                            break;
                        default:
                            Error("Unhandled response type");
                            result = AwaError_Unsupported;
                            break;
                    }
                    if (result == AwaError_Success)
                    {
                        AwaIntegerArrayIterator * iterator = AwaIntegerArray_NewIntegerArrayIterator(array);
                        while (AwaIntegerArrayIterator_Next(iterator))
                        {
                            AwaArrayIndex index = AwaIntegerArrayIterator_GetIndex(iterator);
                            if ((resourceInstanceID == index) || (resourceInstanceID == AWA_INVALID_ID))
                            {
                                AwaInteger value = AwaIntegerArrayIterator_GetValue(iterator);
                                msprintf2(&resourceValue, "%"PRId64, value);
                                AddBanner(cstring, &resourceValue, path, objectName, resourceName, objectID, lastObjectID, objectInstanceID, lastObjectInstanceID, index, quiet);
                            }
                        }
                        AwaIntegerArrayIterator_Free(&iterator);
                    }
                }
                break;

                case AwaResourceType_FloatArray:
                {
                    const AwaFloatArray * array = NULL;
                    switch (responseType)
                    {
                        case ResponseType_GetResponse:
                            result = AwaClientGetResponse_GetValuesAsFloatArrayPointer(getResponse, path, &array);
                            break;
                        case ResponseType_ChangeSet:
                            result = AwaChangeSet_GetValuesAsFloatArrayPointer(changeSet, path, &array);
                            break;
                        case ResponseType_ReadResponse:
                            result = AwaServerReadResponse_GetValuesAsFloatArrayPointer(readResponse, path, &array);
                            break;
                        default:
                            Error("Unhandled response type");
                            result = AwaError_Unsupported;
                            break;
                    }
                    if (result == AwaError_Success)
                    {
                        AwaFloatArrayIterator * iterator = AwaFloatArray_NewFloatArrayIterator(array);
                        while (AwaFloatArrayIterator_Next(iterator))
                        {
                            AwaArrayIndex index = AwaFloatArrayIterator_GetIndex(iterator);
                            if ((resourceInstanceID == index) || (resourceInstanceID == AWA_INVALID_ID))
                            {
                                AwaFloat value = AwaFloatArrayIterator_GetValue(iterator);
                                // IEEE 754 supports up to 17 significant digits
                                msprintf2(&resourceValue, "%.17g", value);
                                AddBanner(cstring, &resourceValue, path, objectName, resourceName, objectID, lastObjectID, objectInstanceID, lastObjectInstanceID, index, quiet);
                            }
                        }
                        AwaFloatArrayIterator_Free(&iterator);
                    }
                }
                break;

                case AwaResourceType_BooleanArray:
                {
                    const AwaBooleanArray * array = NULL;
                    switch (responseType)
                    {
                        case ResponseType_GetResponse:
                            result = AwaClientGetResponse_GetValuesAsBooleanArrayPointer(getResponse, path, &array);
                            break;
                        case ResponseType_ChangeSet:
                            result = AwaChangeSet_GetValuesAsBooleanArrayPointer(changeSet, path, &array);
                            break;
                        case ResponseType_ReadResponse:
                            result = AwaServerReadResponse_GetValuesAsBooleanArrayPointer(readResponse, path, &array);
                            break;
                        default:
                            Error("Unhandled response type");
                            result = AwaError_Unsupported;
                            break;
                    }
                    if (result == AwaError_Success)
                    {
                        AwaBooleanArrayIterator * iterator = AwaBooleanArray_NewBooleanArrayIterator(array);
                        while (AwaBooleanArrayIterator_Next(iterator))
                        {
                            AwaArrayIndex index = AwaBooleanArrayIterator_GetIndex(iterator);
                            if ((resourceInstanceID == index) || (resourceInstanceID == AWA_INVALID_ID))
                            {
                                AwaBoolean value = AwaBooleanArrayIterator_GetValue(iterator);
                                msprintf2(&resourceValue, "%s", value != 0? "True" : "False");
                                AddBanner(cstring, &resourceValue, path, objectName, resourceName, objectID, lastObjectID, objectInstanceID, lastObjectInstanceID, index, quiet);
                            }
                        }
                        AwaBooleanArrayIterator_Free(&iterator);
                    }
                }
                break;

                case AwaResourceType_OpaqueArray:
                {
                    const AwaOpaqueArray * array = NULL;
                    switch (responseType)
                    {
                        case ResponseType_GetResponse:
                            result = AwaClientGetResponse_GetValuesAsOpaqueArrayPointer(getResponse, path, &array);
                            break;
                        case ResponseType_ChangeSet:
                            result = AwaChangeSet_GetValuesAsOpaqueArrayPointer(changeSet, path, &array);
                            break;
                        case ResponseType_ReadResponse:
                            result = AwaServerReadResponse_GetValuesAsOpaqueArrayPointer(readResponse, path, &array);
                            break;
                        default:
                            Error("Unhandled response type");
                            result = AwaError_Unsupported;
                            break;
                    }
                    if (result == AwaError_Success)
                    {
                        AwaOpaqueArrayIterator * iterator = AwaOpaqueArray_NewOpaqueArrayIterator(array);
                        while (AwaOpaqueArrayIterator_Next(iterator))
                        {
                            AwaArrayIndex index = AwaOpaqueArrayIterator_GetIndex(iterator);
                            if ((resourceInstanceID == index) || (resourceInstanceID == AWA_INVALID_ID))
                            {
                                AwaOpaque value = AwaOpaqueArrayIterator_GetValue(iterator);
                                resourceValue = EncodeOpaque(&value);
                                AddBanner(cstring, &resourceValue, path, objectName, resourceName, objectID, lastObjectID, objectInstanceID, lastObjectInstanceID, index, quiet);
                            }
                        }
                        AwaOpaqueArrayIterator_Free(&iterator);
                    }
                }
                break;

                case AwaResourceType_TimeArray:
                {
                    const AwaTimeArray * array = NULL;
                    switch (responseType)
                    {
                        case ResponseType_GetResponse:
                            result = AwaClientGetResponse_GetValuesAsTimeArrayPointer(getResponse, path, &array);
                            break;
                        case ResponseType_ChangeSet:
                            result = AwaChangeSet_GetValuesAsTimeArrayPointer(changeSet, path, &array);
                            break;
                        case ResponseType_ReadResponse:
                            result = AwaServerReadResponse_GetValuesAsTimeArrayPointer(readResponse, path, &array);
                            break;
                        default:
                            Error("Unhandled response type");
                            result = AwaError_Unsupported;
                            break;
                    }
                    if (result == AwaError_Success)
                    {
                        AwaTimeArrayIterator * iterator = AwaTimeArray_NewTimeArrayIterator(array);
                        while (AwaTimeArrayIterator_Next(iterator))
                        {
                            AwaArrayIndex index = AwaTimeArrayIterator_GetIndex(iterator);
                            if ((resourceInstanceID == index) || (resourceInstanceID == AWA_INVALID_ID))
                            {
                                AwaTime value = AwaTimeArrayIterator_GetValue(iterator);
                                msprintf2(&resourceValue, "%"PRId64, value);
                                AddBanner(cstring, &resourceValue, path, objectName, resourceName, objectID, lastObjectID, objectInstanceID, lastObjectInstanceID, index, quiet);
                            }
                        }
                        AwaTimeArrayIterator_Free(&iterator);
                    }
                }
                break;

                case AwaResourceType_ObjectLinkArray:
                {
                    const AwaObjectLinkArray * array = NULL;
                    switch (responseType)
                    {
                        case ResponseType_GetResponse:
                            result = AwaClientGetResponse_GetValuesAsObjectLinkArrayPointer(getResponse, path, &array);
                            break;
                        case ResponseType_ChangeSet:
                            result = AwaChangeSet_GetValuesAsObjectLinkArrayPointer(changeSet, path, &array);
                            break;
                        case ResponseType_ReadResponse:
                            result = AwaServerReadResponse_GetValuesAsObjectLinkArrayPointer(readResponse, path, &array);
                            break;
                        default:
                            Error("Unhandled response type");
                            result = AwaError_Unsupported;
                            break;
                    }
                    if (result == AwaError_Success)
                    {
                        AwaObjectLinkArrayIterator * iterator = AwaObjectLinkArray_NewObjectLinkArrayIterator(array);
                        while (AwaObjectLinkArrayIterator_Next(iterator))
                        {
                            AwaArrayIndex index = AwaObjectLinkArrayIterator_GetIndex(iterator);
                            if ((resourceInstanceID == index) || (resourceInstanceID == AWA_INVALID_ID))
                            {
                                AwaObjectLink value = AwaObjectLinkArrayIterator_GetValue(iterator);
                                msprintf2(&resourceValue, "ObjectLink[%d:%d]", value.ObjectID, value.ObjectInstanceID);
                                AddBanner(cstring, &resourceValue, path, objectName, resourceName, objectID, lastObjectID, objectInstanceID, lastObjectInstanceID, index, quiet);
                            }
                        }
                        AwaObjectLinkArrayIterator_Free(&iterator);
                    }
                }
                break;

                case AwaResourceType_None:
                {
                    AwaResourceOperations resourceOperations = AwaResourceDefinition_GetSupportedOperations(resourceDefinition);
                    if (resourceOperations == AwaResourceOperations_Execute)
                    {
                        resourceValue = strdup("[Executable]");
                    }
                    else
                    {
                        resourceValue = strdup("[None]");
                    }
                    result = AwaError_Success;
                }
                break;

                default:
                    resourceValue = strdup("[Unhandled]");
                    result = AwaError_Success;
                    break;
            }

            if (result == AwaError_Success)
            {
                if (!IsArrayType(resourceType))
                {
                    AddBanner(cstring, &resourceValue, path, objectName, resourceName, objectID, lastObjectID, objectInstanceID, lastObjectInstanceID, -1, quiet);
                }
            }
        }
    }

    return result;
}

/**
 * @brief append a pre-allocated string with a response in human-readable format.
 * @param[in,out] cstring user-allocated string to append
 * @param[in] path path to a single object, object instance or resource.
 * @param[in] response Pointer to a valid Get Response or ChangeSet instance.
 * @param[in] responseType Type of response (Get response or ChangeSet)
 * @param[in] quiet If true, output is simplistic.
 * @return 0 if the path was successfully added, otherwise -1
 */
int Client_AddPathToCString(char ** cstring, const char * path, const AwaClientSession * session, void * response, ResponseType responseType, bool quiet,
                            AwaObjectID * lastObjectID, AwaObjectInstanceID * lastObjectInstanceID)
{
    int result = -1;
    if (path != NULL)
    {
        AwaObjectID objectID = AWA_INVALID_ID;
        AwaObjectInstanceID objectInstanceID = AWA_INVALID_ID;
        AwaResourceID resourceID = AWA_INVALID_ID;

        AwaClientSession_PathToIDs(session, path, &objectID, &objectInstanceID, &resourceID);
        const AwaObjectDefinition * objectDefinition = AwaClientSession_GetObjectDefinition(session, objectID);
        const AwaResourceDefinition * resourceDefinition = AwaObjectDefinition_GetResourceDefinition(objectDefinition, resourceID);

        result = AddPathToCString(cstring, path, response, responseType, quiet,
                                  objectID, objectInstanceID, resourceID, AWA_INVALID_ID,
                                  lastObjectID, lastObjectInstanceID, objectDefinition, resourceDefinition);
    }
    return result;
}

/**
 * @brief append a pre-allocated string with a response in human-readable format.
 * @param[in,out] cstring user-allocated string to append
 * @param[in] path path to a single object, object instance or resource.
 * @param[in] response Pointer to a valid Get Response or ChangeSet instance.
 * @param[in] responseType Type of response (Get response or ChangeSet)
 * @param[in] quiet If true, output is simplistic.
 * @return 0 if the path was successfully added, otherwise -1
 */
int Server_AddPathToCString(char ** cstring, const char * path, const AwaServerSession * session, void * response, ResponseType responseType, bool quiet,
                            AwaObjectID * lastObjectID, AwaObjectInstanceID * lastObjectInstanceID, int resourceInstanceID)
{
    int result = -1;
    if (path != NULL)
    {
        AwaObjectID objectID = AWA_INVALID_ID;
        AwaObjectInstanceID objectInstanceID = AWA_INVALID_ID;
        AwaResourceID resourceID = AWA_INVALID_ID;

        AwaServerSession_PathToIDs(session, path, &objectID, &objectInstanceID, &resourceID);
        const AwaObjectDefinition * objectDefinition = AwaServerSession_GetObjectDefinition(session, objectID);
        const AwaResourceDefinition * resourceDefinition = AwaObjectDefinition_GetResourceDefinition(objectDefinition, resourceID);

        result = AddPathToCString(cstring, path, response, responseType, quiet,
                                  objectID, objectInstanceID, resourceID, resourceInstanceID,
                                  lastObjectID, lastObjectInstanceID, objectDefinition, resourceDefinition);
    }
    return result;
}

AwaServerSession * Server_EstablishSession(const char * address, unsigned int port)
{
    // Initialise Device Management session
    AwaServerSession * session;
    session = AwaServerSession_New();

    if (session != NULL)
    {
        // Call set IPC as UDP, pass address and port
        if (AwaServerSession_SetIPCAsUDP(session, address, port) == AwaError_Success)
        {
            if (AwaServerSession_Connect(session) == AwaError_Success)
            {
                Debug("Session Established\n");
            }
            else
            {
                Error("AwaServerSession_Connect() failed\n");
                AwaServerSession_Free(&session);
            }
        }
        else
        {
            Error("AwaServerSession_SetIPCAsUDP() failed\n");
            AwaServerSession_Free(&session);
        }
    }
    else
    {
        Error("AwaServerSession_New() failed\n");
    }
    return session;
}

void Server_ReleaseSession(AwaServerSession ** session)
{
    if (AwaServerSession_Disconnect(*session) != AwaError_Success)
    {
        Error("AwaServerSession_Disconnect() failed\n");
    }

    if (AwaServerSession_Free(session) != AwaError_Success)
    {
        Error("AwaServerSession_Free() failed\n");
    }
}

const AwaResourceDefinition * Server_GetResourceDefinition(const AwaServerSession * session, const char * path)
{
    AwaObjectID objectID;
    AwaResourceID resourceID;
    const AwaResourceDefinition * resourceDefinition = NULL;

    if (AwaServerSession_PathToIDs(session, path, &objectID, NULL, &resourceID) == AwaError_Success)
    {
        const AwaObjectDefinition * objectDefinition = AwaServerSession_GetObjectDefinition(session, objectID);
        if (objectDefinition != NULL)
        {
            resourceDefinition = AwaObjectDefinition_GetResourceDefinition(objectDefinition, resourceID);
        }
        else
        {
            Debug("objectDefinition is NULL\n");
        }
    }
    else
    {
        Error("AwaServerSession_PathToIDs() failed\n");
    }
    return resourceDefinition;
}

bool Server_IsResourceDefined(const AwaServerSession * session, const char * path)
{
    return Server_GetResourceDefinition(session, path) != NULL;
}

AwaResourceType Server_GetResourceType(const AwaServerSession * session, const char * path)
{
    AwaResourceType resourceType = AwaResourceType_Invalid;
    const AwaResourceDefinition * resourceDefinition = Server_GetResourceDefinition(session, path);
    if (resourceDefinition != NULL)
    {
        resourceType = AwaResourceDefinition_GetType(resourceDefinition);
    }
    else
    {
        Error("resourceDefinition is NULL\n");
        resourceType = AwaResourceType_Invalid;
    }
    return resourceType;
}

char * Server_GetValue(const AwaServerSession * session, const Target * target, const char * arg)
{
    char * value = NULL;

    if (target != NULL)
    {
        if (arg != NULL)
        {
            // Only resources can be set
            AwaResourceID resourceID = AWA_INVALID_ID;
            AwaServerSession_PathToIDs(session, target->Path, NULL, NULL, &resourceID);
            if (resourceID == AWA_INVALID_ID)
            {
                Error("Resource or Resource Instance must be specified: %s\n", arg);
            }
            else
            {
                const char * valueStr = strchr(arg, '=');
                if (valueStr == NULL)
                {
                    Error("A value must be specified: %s\n", arg);
                }
                else
                {
                    valueStr++;  // skip the '='
                    int valueLen = strlen(valueStr);
                    if (valueLen == 0)
                    {
                        Error("A value must be specified: %s\n", arg);
                    }
                    else
                    {
                        value = malloc(sizeof(char) * valueLen + 1);
                        if (value != NULL)
                        {
                            strncpy(value, valueStr, valueLen + 1);  // note: add 1 to accommodate the null terminator otherwise strncpy doesn't add one
                        }
                        else
                        {
                            Error("Out of memory\n");
                        }
                    }
                }
            }
        }
        else
        {
            Error("arg is NULL\n");
        }
    }
    else
    {
        Error("target is NULL\n");
    }
    return value;
}

bool Server_IsObjectTarget(const AwaServerSession * session, const Target * target)
{
    bool result = false;
    AwaObjectID objectID = AWA_INVALID_ID;
    AwaObjectInstanceID objectInstanceID = AWA_INVALID_ID;
    AwaResourceID resourceID = AWA_INVALID_ID;

    if (target != NULL)
    {
        if (AwaServerSession_PathToIDs(session, target->Path, &objectID, &objectInstanceID, &resourceID) == AwaError_Success)
        {
            result = IsIDValid(objectID) && !IsIDValid(objectInstanceID) && !IsIDValid(resourceID) && !IsIDValid(target->ResourceInstanceID);
        }
    }
    else
    {
        Error("target is NULL\n");
    }
    return result;
}

bool Server_IsObjectInstanceTarget(const AwaServerSession * session, const Target * target)
{
    bool result = false;
    AwaObjectID objectID = AWA_INVALID_ID;
    AwaObjectInstanceID objectInstanceID = AWA_INVALID_ID;
    AwaResourceID resourceID = AWA_INVALID_ID;

    if (target != NULL)
    {
        if (AwaServerSession_PathToIDs(session, target->Path, &objectID, &objectInstanceID, &resourceID) == AwaError_Success)
        {
            result = IsIDValid(objectID) && IsIDValid(objectInstanceID) && !IsIDValid(resourceID) && !IsIDValid(target->ResourceInstanceID);
        }
    }
    else
    {
        Error("target is NULL\n");
    }
    return result;
}

bool Server_IsResourceTarget(const AwaServerSession * session, const Target * target)
{
    bool result = false;
    AwaObjectID objectID = AWA_INVALID_ID;
    AwaObjectInstanceID objectInstanceID = AWA_INVALID_ID;
    AwaResourceID resourceID = AWA_INVALID_ID;

    if (target != NULL)
    {
        if (AwaServerSession_PathToIDs(session, target->Path, &objectID, &objectInstanceID, &resourceID) == AwaError_Success)
        {
            result = IsIDValid(objectID) && IsIDValid(objectInstanceID) && IsIDValid(resourceID) && !IsIDValid(target->ResourceInstanceID);
        }
    }
    else
    {
        Error("target is NULL\n");
    }
    return result;
}

bool Server_IsResourceInstanceTarget(const AwaServerSession * session, const Target * target)
{
    bool result = false;
    AwaObjectID objectID = AWA_INVALID_ID;
    AwaObjectInstanceID objectInstanceID = AWA_INVALID_ID;
    AwaResourceID resourceID = AWA_INVALID_ID;

    if (target != NULL)
    {
        if (AwaServerSession_PathToIDs(session, target->Path, &objectID, &objectInstanceID, &resourceID) == AwaError_Success)
        {
            result = IsIDValid(objectID) && IsIDValid(objectInstanceID) && IsIDValid(resourceID) && IsIDValid(target->ResourceInstanceID);
        }
    }
    else
    {
        Error("target is NULL\n");
    }
    return result;
}

int GetNextTargetResourceInstanceIDFromPath(Target ** targets, int numTargets, const char * path, int * index)
{
    AwaResourceInstanceID resourceInstanceID = AWA_INVALID_ID;
    if (targets != NULL)
    {
        if (path != NULL)
        {
            if (index != NULL)
            {
                for (/* no init */; *index < numTargets; (*index)++)
                {
                    if ((targets[*index] != NULL) && (strcmp(targets[*index]->Path, path) == 0))
                    {
                        resourceInstanceID = targets[(*index)++]->ResourceInstanceID;
                        break;
                    }
                }
            }
            else
            {
                Error("index is NULL\n");
            }
        }
        else
        {
            Error("path is NULL\n");
        }
    }
    return resourceInstanceID;
}

void PrintAllObjectDefinitions(AwaObjectDefinitionIterator * iterator, OutputFormat format)
{
    // Unsorted
    while (AwaObjectDefinitionIterator_Next(iterator))
    {
        const AwaObjectDefinition * objectDefinition = AwaObjectDefinitionIterator_Get(iterator);
        if (objectDefinition != NULL)
        {
            PrintObjectDefinitionHeader(objectDefinition, format);
            PrintObjectDefinition(objectDefinition, format);
            PrintObjectDefinitionFooter(objectDefinition, format);
        }
    }
}

void PrintDefinitionTarget(const AwaObjectDefinition * objectDefinition, OutputFormat format, AwaObjectID objectID, AwaResourceID resourceID, AwaObjectID * lastObjectIDPrinted)
{
    if (objectDefinition != NULL)
    {
        AwaObjectID objectID = AwaObjectDefinition_GetID(objectDefinition);

        if (*lastObjectIDPrinted != objectID)
        {
            PrintObjectDefinitionHeader(objectDefinition, format);
            *lastObjectIDPrinted = objectID;
        }

        if (IsIDValid(resourceID))
        {
            const AwaResourceDefinition * resourceDefinition = AwaObjectDefinition_GetResourceDefinition(objectDefinition, resourceID);
            // Print a single resource, with object header if required
            if (resourceDefinition != NULL)
            {
                PrintResourceDefinition(resourceDefinition, format, objectID);
            }
            else
            {
                if (format == OutputFormat_PlainTextVerbose)
                {
                    Error("    Resource: ID:%d Not defined\n", resourceID);
                }
                else
                {
                    Warning("Object %d Resource %d not defined\n", objectID, resourceID);
                }
            }
        }
        else
        {
            // Print an entire object
            PrintObjectDefinition(objectDefinition, format);
            PrintObjectDefinitionFooter(objectDefinition, format);
        }
    }
    else
    {
        if (format == OutputFormat_PlainTextVerbose)
        {
            Error("Object: ID:%d Not defined\n", objectID);
        }
        else
        {
            Warning("Object: ID:%d Not defined\n", objectID);
        }
    }
}

void PrintObjectDefinitionHeader(const AwaObjectDefinition * objectDefinition, OutputFormat outputFormat)
{
    if (objectDefinition != NULL)
    {
        const char * format = NULL;
        char * mandatory = NULL;
        char * collection = NULL;

        switch(outputFormat)
        {
            case OutputFormat_PlainTextVerbose:
                format = "\nObject: ID:%d name:%s minInstances:%s maxInstances:%s\n";
                msprintf2(&mandatory, "%d", AwaObjectDefinition_GetMinimumInstances(objectDefinition));
                msprintf2(&collection, "%d", AwaObjectDefinition_GetMaximumInstances(objectDefinition));
                break;
            case OutputFormat_PlainTextQuiet:
                format = "OBJECT %d %s %s %s\n";
                msprintf2(&mandatory, "%d", AwaObjectDefinition_GetMinimumInstances(objectDefinition));
                msprintf2(&collection, "%d", AwaObjectDefinition_GetMaximumInstances(objectDefinition));
                break;
            case OutputFormat_DeviceServerXML:
                format = "<ObjectDefinition>\n"
                         "    <ObjectID>%d</ObjectID>\n"
                         "    <SerialisationName>%s</SerialisationName>\n"
                         "    <IsMandatory>%s</IsMandatory>\n"
                         "    <Singleton>%s</Singleton>\n"
                         "    <Properties>\n";
                msprintf2(&mandatory, "%s", AwaObjectDefinition_GetMinimumInstances(objectDefinition) == 0 ? "False" : "True");
                msprintf2(&collection, "%s", AwaObjectDefinition_GetMaximumInstances(objectDefinition) > 1 ? "False" : "True");
                break;

            default:
                format = NULL;
        }

        if(format != NULL)
        {
            printf(format,
                   AwaObjectDefinition_GetID(objectDefinition),
                   AwaObjectDefinition_GetName(objectDefinition),
                   mandatory,
                   collection);

            free(mandatory);
            free(collection);
        }
    }
}

void PrintObjectDefinitionFooter(const AwaObjectDefinition * objectDefinition, OutputFormat outputFormat)
{
    if (objectDefinition != NULL)
    {
        const char * format = NULL;

        switch(outputFormat)
        {
            case OutputFormat_DeviceServerXML:
                format = "    </Properties>\n"
                         "</ObjectDefinition>\n";
                break;

            default:
                format = NULL;
        }

        if(format != NULL)
        {
            printf("%s",format);
        }
    }
}

void PrintObjectDefinition(const AwaObjectDefinition * objectDefinition, OutputFormat outputFormat)
{
    // Print all resources defined as part of this object
    if (objectDefinition != NULL)
    {
        AwaResourceDefinitionIterator * resourceIterator = AwaObjectDefinition_NewResourceDefinitionIterator(objectDefinition);
        if (resourceIterator != NULL)
        {
            while (AwaResourceDefinitionIterator_Next(resourceIterator))
            {
                const AwaResourceDefinition * resourceDefinition = AwaResourceDefinitionIterator_Get(resourceIterator);
                PrintResourceDefinition(resourceDefinition, outputFormat, AwaObjectDefinition_GetID(objectDefinition));
            }
        }
        AwaResourceDefinitionIterator_Free(&resourceIterator);
    }
}

void PrintResourceDefinition(const AwaResourceDefinition * resourceDefinition, OutputFormat outputFormat, AwaObjectID objectID)
{
    // Use objectID for quiet output
    if (resourceDefinition != NULL)
    {
        const char * newLine = "";
        const char * format = NULL;
        char * id = NULL;
        const char * resourceType = "";
        char * mandatory = NULL;
        char * collection = NULL;
        const char * resourceOperation = "";
        AwaResourceID resourceID = AwaResourceDefinition_GetID(resourceDefinition);

        switch(outputFormat)
        {
            case OutputFormat_PlainTextVerbose:
                newLine = strlen(AwaResourceDefinition_GetName(resourceDefinition)) > 20 ? "\n                                              " : "";
                format = "    %s name:%-20s%s type:%-16s minInstances:%s maxInstances:%s operations:%s\n";
                msprintf2(&id, "Resource: ID:%-3d", resourceID);
                msprintf2(&mandatory, "%d", AwaResourceDefinition_GetMinimumInstances(resourceDefinition));
                msprintf2(&collection, "%d", AwaResourceDefinition_GetMaximumInstances(resourceDefinition));
                resourceType = ResourceTypeToString(AwaResourceDefinition_GetType(resourceDefinition));
                resourceOperation = ResourceOperationToString(AwaResourceDefinition_GetSupportedOperations(resourceDefinition));
                break;
            case OutputFormat_PlainTextQuiet:
                format = "%s %s %.0s%s %s %s %s\n";
                msprintf2(&id, "%d %d", objectID, resourceID);
                msprintf2(&mandatory, "%d", AwaResourceDefinition_GetMinimumInstances(resourceDefinition));
                msprintf2(&collection, "%d", AwaResourceDefinition_GetMaximumInstances(resourceDefinition));
                resourceType = ResourceTypeToString(AwaResourceDefinition_GetType(resourceDefinition));
                resourceOperation = ResourceOperationToString(AwaResourceDefinition_GetSupportedOperations(resourceDefinition));
                break;

            case OutputFormat_DeviceServerXML:
                format = "       <PropertyDefinition>\n"
                        "            <PropertyID>%s</PropertyID>\n"
                        "            <SerialisationName>%s</SerialisationName>\n"
                        "            <DataType>%s%s</DataType>\n"
                        "            <IsMandatory>%s</IsMandatory>\n"
                        "            <IsCollection>%s</IsCollection>\n"
                        "            <Access>%s</Access>\n"
                        "        </PropertyDefinition>\n";
                msprintf2(&id, "%d", resourceID);
                msprintf2(&mandatory, "%s", AwaResourceDefinition_GetMinimumInstances(resourceDefinition) == 0 ? "False" : "True");
                msprintf2(&collection, "%s", AwaResourceDefinition_GetMaximumInstances(resourceDefinition) <= 1 ? "False" : "True");
                resourceType = DeviceServerXML_ResourceTypeToString(AwaResourceDefinition_GetType(resourceDefinition));
                resourceOperation = DeviceServerXML_ResourceOperationToString(AwaResourceDefinition_GetSupportedOperations(resourceDefinition));
                break;
            default:
                format = NULL;
                break;
        }

        if(format != NULL)
        {
            printf(format,
                   id,
                   AwaResourceDefinition_GetName(resourceDefinition),
                   newLine,
                   resourceType,
                   mandatory,
                   collection,
                   resourceOperation);
            free(id);
            free(mandatory);
            free(collection);
        }
    }
}

void INThandler(int sig)
{
    g_signal = true;
    signal(sig, SIG_IGN);
}

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


#ifndef CONTIKI

#include <stdio.h>

#include "lwm2m_xml_serdes.h"
#include "xml.h"

static const char * TypeStrings[] = { "Opaque", "Integer", "Float", "Boolean", "String", "DateTime", "None", "ObjectLink" };
static const char * OperationStrings[] = { "None", "Read", "Write", "ReadWrite", "Execute", "ReadExecute", "WriteExecute", "ReadWriteExecute" };

#define NUM_TYPES (sizeof(TypeStrings) / sizeof(const char *))

AwaStaticResourceType xmlif_StringToDataType(const char * value)
{
    int i;

    for (i = 0; i < (sizeof(TypeStrings) / sizeof(char *)); i++)
    {
        if (!strcasecmp(TypeStrings[i], value))
        {
            return i;
        }
    }
    return -1;
}

const char * xmlif_DataTypeToString(AwaStaticResourceType type)
{
    if ((type < 0) || (type >= NUM_TYPES))
    {
        return "None";
    }
    return TypeStrings[type];
}

Operations xmlif_StringToOperation(const char * value)
{
    int i;

    for (i = 0; i < (sizeof(OperationStrings) / sizeof(char *)); i++)
    {
        if (!strcasecmp(OperationStrings[i], value))
        {
            return i;
        }
    }
    return Operations_None;
}

const char * xmlif_OperationToString(Operations operation)
{
    if ((operation < Operations_None) || (operation > Operations_E))
    {
        return "None";
    }
    return OperationStrings[operation];
}

int xmlif_GetInteger(TreeNode content, const char * name)
{
    TreeNode node = TreeNode_Navigate(content, (char*)name);
    if (node != NULL)
    {
        const uint8_t * value = TreeNode_GetValue(node);
        if (value != NULL)
        {
            return atoi((const char*)value);
        }
    }
    return -1;
}

const char * xmlif_GetOpaque(TreeNode content, const char * name)
{
    TreeNode node = TreeNode_Navigate(content, (char*)name);
    if (node != NULL)
    {
        return (const char *)TreeNode_GetValue(node);
    }
    return NULL;
}

char * xmlif_EncodeValue(AwaStaticResourceType dataType, const char * buffer, int bufferLength)
{
    int outLength = 0;
    char * dataValue = NULL;

    if (buffer == NULL)
    {
        return NULL;
    }

    switch(dataType)
    {
        case AwaStaticResourceType_String:
        {
            // adjust string length because object store expects Pascal strings
            if (bufferLength > 0 && buffer[bufferLength - 1] == 0)
            {
                int lengthAsString = strlen(buffer);
                bufferLength = lengthAsString < bufferLength ? lengthAsString : bufferLength;
            }
        }
        // no break - intentional fall-through

        case AwaStaticResourceType_Opaque:
            outLength = (((bufferLength + 2) * 4) / 3);
            dataValue = malloc(outLength + 1);
            if (dataValue == NULL)
            {
                AwaLwm2mResult_SetResult(AwaLwm2mResult_OutOfMemory);
                goto error;
            }
            dataValue[outLength] = '\0';
            outLength = b64Encode(dataValue, outLength, (char*)buffer, bufferLength);
            break;

        case AwaStaticResourceType_Float:

            if (bufferLength == sizeof(double))
            {
                // IEEE 754 supports up to 17 significant digits
                double temp;
                memcpy(&temp, buffer, sizeof(temp));
                outLength = asprintf(&dataValue, "%.17g", temp);
            }
            else if (bufferLength == sizeof(float))
            {
                // IEEE 754 supports up to 9 significant digits
                float temp;
                memcpy(&temp, buffer, sizeof(temp));
                outLength = asprintf(&dataValue, "%.9g", temp);
            }

            if ((outLength <= 0) || (dataValue == NULL))
            {
                AwaLwm2mResult_SetResult(AwaLwm2mResult_InternalError);
                goto error;
            }
            break;

        case AwaStaticResourceType_Integer:  // no break
        case AwaStaticResourceType_Time:

            switch (bufferLength)
            {
                case sizeof(int8_t):
                    outLength = asprintf(&dataValue, "%"PRId8, *(int8_t*)&buffer[0]);
                    break;
                case sizeof(int16_t):
                    outLength = asprintf(&dataValue, "%"PRId16, *(int16_t*)&buffer[0]);
                    break;
                case sizeof(int32_t):
                    outLength = asprintf(&dataValue, "%"PRId32, *(int32_t*)&buffer[0]);
                    break;
                case sizeof(int64_t):
                    outLength = asprintf(&dataValue, "%"PRId64, *(int64_t*)&buffer[0]);
                    break;
                default:
                    outLength = -1;
                    break;
            }

            if ((outLength <= 0) || (dataValue == NULL))
            {
                AwaLwm2mResult_SetResult(AwaLwm2mResult_OutOfMemory);
                goto error;
            }

            break;

        case AwaStaticResourceType_Boolean:
            outLength = asprintf(&dataValue, "%s", (*(bool*)&buffer[0]) ? "True" : "False");

            if ((outLength <= 0) || (dataValue == NULL))
            {
                AwaLwm2mResult_SetResult(AwaLwm2mResult_OutOfMemory);
                goto error;
            }
            break;

        case AwaStaticResourceType_ObjectLink:;
            AwaObjectLink * objectLink = (AwaObjectLink *) buffer;
            outLength = asprintf(&dataValue, "%d:%d", objectLink->ObjectID, objectLink->ObjectInstanceID);

            if ((outLength <= 0) || (dataValue == NULL))
            {
                AwaLwm2mResult_SetResult(AwaLwm2mResult_OutOfMemory);
                goto error;
            }
            break;

        case AwaStaticResourceType_None:
        default:
            AwaLwm2mResult_SetResult(AwaLwm2mResult_BadRequest);
            goto error;
    }

    AwaLwm2mResult_SetResult(AwaLwm2mResult_Success);
error:
    return dataValue;
}

int xmlif_DecodeValue(char ** dataValue, AwaStaticResourceType dataType, const char * buffer, int bufferLength)
{
    int dataLength = -1;
    int outLength;

    switch(dataType)
    {
        case AwaStaticResourceType_String:
            //
            outLength = ((bufferLength * 3) / 4);  // every 4 base encoded bytes are decoded to 3 bytes,
            *dataValue = malloc(outLength);
            if (*dataValue == NULL)
            {
                AwaLwm2mResult_SetResult(AwaLwm2mResult_OutOfMemory);
                goto error;
            }
            dataLength = b64Decode(*dataValue, outLength, (char*)buffer, bufferLength);

            if (dataLength == -1)
            {
                AwaLwm2mResult_SetResult(AwaLwm2mResult_BadRequest);
                goto error;
            }
            else
            {
                dataLength++;  // add space for null terminator
                char * nullTerminated = malloc(dataLength);
                memcpy(nullTerminated, *dataValue, dataLength - 1);
                free(*dataValue);
                nullTerminated[dataLength-1] = '\0';
                *dataValue = nullTerminated;
            }
            break;

        case AwaStaticResourceType_Opaque: 
        case AwaStaticResourceType_None: // TypeNone for Executable payload which is an Opaque
            outLength = ((bufferLength * 3) / 4);  // every 4 base encoded bytes are decoded to 3 bytes
            *dataValue = malloc(outLength);
            if (*dataValue == NULL)
            {
                AwaLwm2mResult_SetResult(AwaLwm2mResult_OutOfMemory);
                goto error;
            }
            dataLength = b64Decode(*dataValue, outLength, (char*)buffer, bufferLength);
            if (dataLength == -1)
            {
                AwaLwm2mResult_SetResult(AwaLwm2mResult_BadRequest);
                goto error;
            }
            break;

        case AwaStaticResourceType_Float:
        {
            double d;
            *dataValue = malloc(sizeof(double));
            if (*dataValue == NULL)
            {
                AwaLwm2mResult_SetResult(AwaLwm2mResult_OutOfMemory);
                goto error;
            }
            sscanf(buffer, "%20lf", &d);
            dataLength = sizeof(double);
            memcpy(*dataValue, &d, dataLength);
            break;
        }

        case AwaStaticResourceType_Integer:  // no break
        case AwaStaticResourceType_Time:
        {
            uint64_t u;
            *dataValue = malloc(sizeof(uint64_t));
            if (*dataValue == NULL)
            {
                AwaLwm2mResult_SetResult(AwaLwm2mResult_OutOfMemory);
                goto error;
            }
            sscanf(buffer, "%"SCNd64, &u);
            dataLength = sizeof(uint64_t);
            memcpy(*dataValue, &u, dataLength);
            break;
        }

        case AwaStaticResourceType_Boolean:
            *dataValue = malloc(sizeof(bool));
            if (*dataValue == NULL)
            {
                AwaLwm2mResult_SetResult(AwaLwm2mResult_OutOfMemory);
                goto error;
            }
            (*dataValue)[0] = (strcmp(buffer, "True") == 0);
            dataLength = sizeof(bool);
            break;

        case AwaStaticResourceType_ObjectLink:
        {
            dataLength = sizeof(AwaObjectLink);
            *dataValue = malloc(dataLength);

            if (*dataValue == NULL)
            {
                AwaLwm2mResult_SetResult(AwaLwm2mResult_OutOfMemory);
                goto error;
            }
            AwaObjectLink * objectLink = (AwaObjectLink *)*dataValue;
            sscanf(buffer, "%10d:%10d", &objectLink->ObjectID, &objectLink->ObjectInstanceID);

            Lwm2m_Debug("objectID: %d objectInstanceID %d\n", objectLink->ObjectID, objectLink->ObjectInstanceID);

            break;
        }

        //case AwaStaticResourceType_None:
        default:
            AwaLwm2mResult_SetResult(AwaLwm2mResult_BadRequest);
            goto error;
    }

    AwaLwm2mResult_SetResult(AwaLwm2mResult_Success);
error:
    return dataLength;
}

#endif

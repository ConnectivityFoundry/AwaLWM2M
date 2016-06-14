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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <float.h>

#include "lwm2m_serdes.h"
#include "lwm2m_core.h"
#include "lwm2m_object_store.h"
#include "lwm2m_plaintext.h"
#include "lwm2m_debug.h"

static int PTSerialiseResourceInstance(Lwm2mTreeNode * node, ResourceDefinition * definition, int objectID,
                                       int instanceID, int resourceID, int resID, uint8_t * buffer, int len)
{
    if (Lwm2mTreeNode_GetType(node) != Lwm2mTreeNodeType_ResourceInstance)
    {
       Lwm2m_Error("ERROR: Resource Instance node type expected. Received %d\n", Lwm2mTreeNode_GetType(node));
       return -1;
    }

    char * buf = (char*)buffer;
    int valueLength = -1;
    uint16_t size;
    uint8_t * value;

    value = (uint8_t * )Lwm2mTreeNode_GetValue(node, &size);

    switch (definition->Type)
    {
        case AwaResourceType_String:
            if (len >= size)
            {
                memcpy(buf, (char*)value, size);
                buf[size] = '\0';
                valueLength = strlen(buf);
            }
            break;
        case AwaResourceType_Boolean:
            sprintf(buf, "%s", *(bool*)value ? "1" : "0");
            valueLength = strlen(buf);
            break;
        case AwaResourceType_Time:
        case AwaResourceType_Integer:
            switch (size)
            {
                case sizeof(int8_t):
                    sprintf(buf, "%" PRId8, ptrToInt8(value));
                    break;
                case sizeof(int16_t):
                    sprintf(buf, "%" PRId16, ptrToInt16(value));
                    break;
                case sizeof(int32_t):
                    sprintf(buf, "%" PRId32, ptrToInt32(value));
                    break;
                case sizeof(int64_t):
                    sprintf(buf, "%" PRId64, ptrToInt64(value));
                    break;
                default:
                    break;
            }
            valueLength = strlen(buf);
            break;
        case AwaResourceType_Float:
            switch (size)
            {
            case sizeof(float):
                sprintf(buf, "%f", *(float*)value);
                break;
            case sizeof(double):
                sprintf(buf, "%f", *(double*)value);
                break;
            default:
                Lwm2m_Error("ERROR: invalid length for float\n");
                break;
            }
            valueLength = strlen(buf);
            break;
        case AwaResourceType_Opaque:
            Lwm2m_Error("ERROR: Opaque is not supported for plain text mode\n");
            break;
        case AwaResourceType_ObjectLink:
            {
               AwaObjectLink * objectLink = (AwaObjectLink *) value;
               sprintf(buf, "%d:%d", objectLink->ObjectID, objectLink->ObjectInstanceID);
               valueLength = strlen(buf);
            }
            break;
        default:
            Lwm2m_Error("ERROR: unknown type: %d\n", definition->Type);
            break;
    }

    return valueLength;
}

static int PTSerialiseResource(SerdesContext * serdesContext, Lwm2mTreeNode * node, ObjectIDType objectID,
                               ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, uint8_t * buffer, int len)
{
    int resourceLength = 0;

    if (Lwm2mTreeNode_GetType(node) != Lwm2mTreeNodeType_Resource)
    {
        Lwm2m_Error("ERROR: Resource node type expected. Received %d\n", Lwm2mTreeNode_GetType(node));
        return -1;
    }

    ResourceDefinition * definition = (ResourceDefinition *)Lwm2mTreeNode_GetDefinition(node);

    if (IS_MULTIPLE_INSTANCE(definition))
    {
        Lwm2m_Error("ERROR: plain text only works on singular resources!!\n");
        return -1;
    }

    Lwm2mTreeNode * child = Lwm2mTreeNode_GetFirstChild(node);

    int resourceInstanceID;
    Lwm2mTreeNode_GetID(child, &resourceInstanceID);

    if (resourceInstanceID != 0)
    {
        Lwm2m_Error("ERROR: resource instance ID of single instance resource must be 0\n");
        return -1;
    }

    resourceLength = PTSerialiseResourceInstance(child, definition, objectID, objectInstanceID, resourceID, resourceInstanceID, buffer, len);
    return resourceLength;
}

static int PTDeserialiseResource(SerdesContext * serdesContext, Lwm2mTreeNode ** dest, const DefinitionRegistry * registry, ObjectIDType objectID,
                                 ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, const uint8_t * buffer, int bufferLen)
{
    int result = -1;
    ResourceDefinition * definition;

    *dest = Lwm2mTreeNode_Create();
    Lwm2mTreeNode_SetID(*dest, resourceID);
    Lwm2mTreeNode_SetType(*dest, Lwm2mTreeNodeType_Resource);

    definition = Definition_LookupResourceDefinition(registry, objectID, resourceID);

    if (definition == NULL)
    {
        Lwm2m_Error("ERROR: Failed to determine resource definition Object %d Resource %d\n", objectID, resourceID);
        return -1;
    }

    if (Lwm2mTreeNode_SetDefinition(*dest, definition) != 0)
    {
        Lwm2m_Error("ERROR: Failed to set definition Object %d Resource %d\n", objectID, resourceID);
        return -1;
    }

    Lwm2mTreeNode * resourceValueNode = Lwm2mTreeNode_Create();
    Lwm2mTreeNode_SetID(resourceValueNode, 0);
    Lwm2mTreeNode_SetType(resourceValueNode, Lwm2mTreeNodeType_ResourceInstance);

    AwaResourceType resourceType = Definition_GetResourceType(registry, objectID, resourceID);
    switch (resourceType)
    {
        case AwaResourceType_Integer:  // no break
        case AwaResourceType_Time:
            {
                int64_t temp = 0;
                result = sscanf((char*)buffer, "%" SCNu64, &temp);
                if (result > 0)
                {
                    result = Lwm2mTreeNode_SetValue(resourceValueNode, (const uint8_t*)&temp, sizeof(temp));
                }
            }
            break;
        case AwaResourceType_Boolean:
                if (bufferLen == 1)
                {
                    bool temp = false;

                    if (buffer[0] == '0')
                    {
                        temp = false;
                        result = 0;
                    }
                    else if (buffer[0] == '1')
                    {
                        temp = true;
                        result = 0;
                    }

                    if (result == 0)
                    {
                        result = Lwm2mTreeNode_SetValue(resourceValueNode, (const uint8_t*)&temp, sizeof(temp));
                    }
                    else
                    {
                        Lwm2m_Error("ERROR: %s is not a valid boolean value\n", (char*)buffer);
                    }
                }
                break;
        case AwaResourceType_Float:
            {
                double temp = 0;
                result = sscanf((char*)buffer, "%24lf", &temp);
                if (result > 0)
                {
                    result = Lwm2mTreeNode_SetValue(resourceValueNode, (const uint8_t*)&temp, sizeof(temp));
                }
            }
            break;
        case AwaResourceType_Opaque:
            Lwm2m_Error("Opaque is not supported for plain text\n");
            break;
        case AwaResourceType_String:
            {
                const uint8_t * str = buffer != NULL ? buffer : (uint8_t*)"";
                result = Lwm2mTreeNode_SetValue(resourceValueNode, str, bufferLen);
            }
            break;
        case AwaResourceType_ObjectLink:
            {
                AwaObjectLink objectLink;
                result = sscanf((char*)buffer, "%10d:%10d", &objectLink.ObjectID, &objectLink.ObjectInstanceID);
                if (result > 0)
                {
                    result = Lwm2mTreeNode_SetValue(resourceValueNode, (const uint8_t*)&objectLink, sizeof(objectLink));
                }
            }
            break;
        default:
            Lwm2m_Error("ERROR: unknown type: %d\n", resourceType);
            break;
    }

    if (result >= 0)
    {
        Lwm2mTreeNode_AddChild(*dest, resourceValueNode);
    }
    else
    {
        Lwm2mTreeNode_DeleteRecursive(resourceValueNode);
    }

    return result;
}

// Map PlainText serdes function delegates
const SerialiserDeserialiser plainTextSerDes =
{
    .SerialiseObject   = NULL,
    .SerialiseObjectInstance = NULL,
    .SerialiseResource = PTSerialiseResource,
    .DeserialiseObject = NULL,
    .DeserialiseObjectInstance = NULL,
    .DeserialiseResource = PTDeserialiseResource,
};

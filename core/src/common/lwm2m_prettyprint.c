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
#include <float.h>
#include <inttypes.h>
#include <string.h>

#include "lwm2m_prettyprint.h"
#include "lwm2m_serdes.h"
#include "lwm2m_debug.h"
#include "lwm2m_util.h"

#define PP_NUMBER_BUFFER_SIZE (20)
#define PP_BOOLEAN_BUFFER_SIZE (6)

static int PPEncodeString(uint8_t * buffer, int bufferLen, char * value, int len)
{
    char * temp = (char *)malloc(len + 1);
    int result = 0;

    if (temp != NULL)
    {
        memcpy(temp, value, len);
        temp[len] = '\0';
        result =  snprintf((char*)buffer, bufferLen, "%s", temp);
        free(temp);
    }

    return result;
}

static int PPEncodeOpaque(uint8_t * buffer, int bufferLen, char * value, int len)
{
    char * valueBuffer = (char * )malloc((len*3)+1);
    int result = 0;

    if (valueBuffer != NULL)
    {
        int pos = 0;
        char * valuePos = valueBuffer;
        memset(valueBuffer, 0, (len*3)+1);

        for(pos = 0; pos < len; pos++)
        {
            snprintf(valuePos, 4, "%02X ", (unsigned char)value[pos]);
            valuePos += 3;
        }

        result = snprintf((char*)buffer, bufferLen, "%s", valueBuffer);

        free(valueBuffer);
    }

    return result;
}

static int PPEncodeInteger(uint8_t * buffer, int bufferLen, int64_t value)
{
    char valueBuffer[PP_NUMBER_BUFFER_SIZE] = {0};

    if (snprintf(valueBuffer, sizeof(valueBuffer), "%" PRId64, value))
    {
        return snprintf((char*)buffer, bufferLen, "%s", valueBuffer);
    }
    else
    {
        return 0;
    }
}

static int PPEncodeFloat(uint8_t * buffer, int bufferLen, double value)
{
    char valueBuffer[PP_NUMBER_BUFFER_SIZE] = {0};

    if (snprintf(valueBuffer, sizeof(valueBuffer), "%f", value))
    {
        return snprintf((char*)buffer, bufferLen, "%s", valueBuffer);
    }
    else
    {
        return 0;
    }
}

static int PPEncodeBoolean(uint8_t * buffer, int bufferLen, bool value)
{
    char valueBuffer [PP_BOOLEAN_BUFFER_SIZE] = {0};

    if (snprintf(valueBuffer, sizeof(valueBuffer), "%s", value ? "True" : "False"))
    {
        return snprintf((char*)buffer, bufferLen, "%s", valueBuffer);
    }
    else
    {
        return 0;
    }
}

static int PPSerialiseResourceInstance(Lwm2mTreeNode * node, ResourceDefinition * definition, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID,
                                       ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID, uint8_t * buffer, int len)
{
    if (Lwm2mTreeNode_GetType(node) != Lwm2mTreeNodeType_ResourceInstance)
    {
       Lwm2m_Error("ERROR: Resource Instance node type expected. Received %d\n", Lwm2mTreeNode_GetType(node));
       return -1;
    }

    uint8_t * valueBuffer = buffer;
    int valueLength = 0;
    int headerLen = 0;
    uint16_t size;
    uint8_t * value;

    value = (uint8_t * )Lwm2mTreeNode_GetValue(node, &size);

    headerLen += snprintf((char*)buffer, len, "\t%s[%d/%d/%d/%d]: ", definition->ResourceName, objectID, objectInstanceID, resourceID, resourceInstanceID);

    if (headerLen <= 0)
    {
        return -1;
    }

    valueBuffer += headerLen;

    switch (definition->Type)
    {
        case AwaResourceType_String:
            valueLength = PPEncodeString(valueBuffer, len - headerLen, (char*)value, size);
            break;

        case AwaResourceType_Boolean:
            valueLength = PPEncodeBoolean(valueBuffer, len - headerLen, *(bool*)value);
            break;

        case AwaResourceType_Time:  // no break
        case AwaResourceType_Integer:
            switch (size)
            {
            case sizeof(int8_t):
                valueLength = PPEncodeInteger(valueBuffer, len - headerLen, ptrToInt8(value));
                break;
            case sizeof(int16_t):
                valueLength = PPEncodeInteger(valueBuffer, len - headerLen, ptrToInt16(value));
                break;
            case sizeof(int32_t):
                valueLength = PPEncodeInteger(valueBuffer, len - headerLen, ptrToInt32(value));
                break;
            case sizeof(int64_t):
                valueLength = PPEncodeInteger(valueBuffer, len - headerLen, ptrToInt64(value));
                break;
            default:
                break;
            }
            break;

        case AwaResourceType_Float:
            switch (size)
            {
            case sizeof(float):
                valueLength =  PPEncodeFloat(valueBuffer, len - headerLen, *(float*)value);
                break;
            case sizeof(double):
                valueLength =  PPEncodeFloat(valueBuffer, len - headerLen, *(double*)value);
                break;
            default:
                Lwm2m_Error("ERROR: prettyprint - invalid length for float\n");
                break;
            }
            break;

        case AwaResourceType_Opaque:
            valueLength = PPEncodeOpaque(valueBuffer, len - headerLen, (char*)value, size);
            break;
        default:
            Lwm2m_Error("ERROR: unknown type: %d\n", definition->Type);
            break;
    }

    strcat((char*)valueBuffer, "\n");

    return valueLength + headerLen + 1;
}

static int PPSerialiseResource(SerdesContext * serdesContext, Lwm2mTreeNode * node, ObjectIDType objectID,
                               ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, uint8_t * buffer, int len)
{
    int resourceLength = 0;

    if (Lwm2mTreeNode_GetType(node) != Lwm2mTreeNodeType_Resource)
    {
       Lwm2m_Error("ERROR: Resource node type expected. Received %d\n", Lwm2mTreeNode_GetType(node));
       return -1;
    }

    ResourceDefinition * definition = (ResourceDefinition*)Lwm2mTreeNode_GetDefinition(node);

    Lwm2mTreeNode * child = Lwm2mTreeNode_GetFirstChild(node);
    while (child != NULL)
    {
       int resourceInstanceID;
       Lwm2mTreeNode_GetID(child, &resourceInstanceID);

       int valueLength = PPSerialiseResourceInstance(child, definition, objectID, objectInstanceID, resourceID, resourceInstanceID, &buffer[resourceLength], len - resourceLength);
       if (valueLength <= 0)
       {
           Lwm2m_Error("ERROR: Failed to serialise resource instance\n");
           return -1;
       }
       resourceLength += valueLength;
       child = Lwm2mTreeNode_GetNextChild(node, child);
    }

    return resourceLength;
}


static int PPSerialiseObjectInstance(SerdesContext * serdesContext, Lwm2mTreeNode * node, ObjectIDType objectID,
                                     ObjectInstanceIDType objectInstanceID, uint8_t * buffer, int len)
{
    int instanceLength = 0;

    if (Lwm2mTreeNode_GetType(node) != Lwm2mTreeNodeType_ObjectInstance)
    {
        Lwm2m_Error("ERROR: Object instance node type expected. Received %d\n", Lwm2mTreeNode_GetType(node));
        return -1;
    }

    Lwm2mTreeNode * objectNode = Lwm2mTreeNode_GetParent(node);
    ObjectDefinition * definition = (ObjectDefinition *)Lwm2mTreeNode_GetDefinition(objectNode);
    if (definition == NULL)
    {
        return -1;
    }
    instanceLength += snprintf((char*)buffer, len, "%s[%d/%d]:\n", definition->ObjectName, objectID, objectInstanceID);

    Lwm2mTreeNode * child = Lwm2mTreeNode_GetFirstChild(node);
    while (child != NULL)
    {
        int resourceID;
        Lwm2mTreeNode_GetID(child, &resourceID);

        int resourceLength = PPSerialiseResource(serdesContext, child, objectID, objectInstanceID, resourceID, &buffer[instanceLength], len - instanceLength);
        if (resourceLength <= 0)
        {
            Lwm2m_Error("Failed to serialise resource\n");
            return -1;
        }
        instanceLength += resourceLength;

        child = Lwm2mTreeNode_GetNextChild(node, child);
    }
    return instanceLength;
}

static int PPSerialiseObject(SerdesContext * serdesContext, Lwm2mTreeNode * node, ObjectIDType objectID, uint8_t * buffer, int len)
{
    int pos = 0;

    if (Lwm2mTreeNode_GetType(node) != Lwm2mTreeNodeType_Object)
    {
        Lwm2m_Error("ERROR: Object node type expected. Received %d\n", Lwm2mTreeNode_GetType(node));
        return -1;
    }

    Lwm2mTreeNode * child = Lwm2mTreeNode_GetFirstChild(node);
    while (child != NULL)
    {
        int objectInstanceID;

        Lwm2mTreeNode_GetID(child, &objectInstanceID);

        int instanceLength = PPSerialiseObjectInstance(serdesContext, child, objectID, objectInstanceID, &buffer[pos], len - pos);
        if (instanceLength <= 0)
        {
            Lwm2m_Error("Failed to serialise object instance\n");
            return -1;
        }

        pos += instanceLength;

        child = Lwm2mTreeNode_GetNextChild(node, child);
    }

    return pos;
}

// Map Pretty Print serdes function delegates
const SerialiserDeserialiser ppSerDes =
{
    .SerialiseObject           = PPSerialiseObject,
    .SerialiseObjectInstance   = PPSerialiseObjectInstance,
    .SerialiseResource         = PPSerialiseResource,
    .DeserialiseObject         = NULL,
    .DeserialiseObjectInstance = NULL,
    .DeserialiseResource       = NULL,
};

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


#ifndef JSMN_FIRST_CHILD_NEXT_SIBLING
    #define JSMN_FIRST_CHILD_NEXT_SIBLING
#endif

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <float.h>
#include <jsmn.h>

#include "lwm2m_json.h"
#include "lwm2m_serdes.h"
#include "lwm2m_object_store.h"
#include "b64.h"
#include "lwm2m_debug.h"

#define MAX_JSON_TOKENS 128

typedef enum
{
    JSON_TYPE_FLOAT,
    JSON_TYPE_STRING,
    JSON_TYPE_BOOLEAN,
    JSON_TYPE_OBJECT_LINK,

} JsonDataType;

typedef enum
{
    NONE_URI = 0,
    RESOURCE_URI,
    INSTANCE_URI,
    OBJECT_URI,

} UriLevelType;

typedef struct
{
    UriLevelType uriLevel;
    bool lastInstance;
    bool lastResource;
} JsonSerdesContext;

static void JsonInitSerdesContext(SerdesContext * serdesContext, UriLevelType level)
{
    JsonSerdesContext * context = (JsonSerdesContext *)malloc(sizeof(JsonSerdesContext));

    context->uriLevel = level;
    context->lastInstance = true;
    context->lastResource = true;

    *serdesContext = context;
}

static void JsonDestroySerdesContext(SerdesContext * serdesContext)
{
    free(*serdesContext);
    *serdesContext = NULL;
}

static int JsonTokenStrcmp(const char * buffer, jsmntok_t * t, const char * string)
{
    return (strlen(string) == (size_t)(t->end - t->start)) && (strncmp(&(buffer[t->start]), string, strlen(string)) == 0);
}

static int JsonTokenFind(jsmntok_t tokens[], const char * buffer, jsmntok_t * t, const char * string)
{
    if (t->size <= 0)
    {
        return -1;
    }

    int child = t->first_child;
    do
    {
        if (JsonTokenStrcmp(buffer, &tokens[child], string))
        {
            return child;
        }
    }
    while ((child = tokens[child].next_sibling) != -1);

    return -1;
}

static char * JsonTokenToString(const char *buffer, jsmntok_t *t)
{
    static char buf[256];
    memset(buf, 0, 256);
    memcpy(buf, buffer + t->start, t->end - t->start);
    return buf;
}

static int JsonStartElement(char * buffer, int bufferLen)
{
    strcpy(buffer, "{\"e\":[\n");

    return strlen(buffer);
}

static int JsonEndElement(char * buffer, int bufferLen)
{
    strcat(buffer, "}\n");

    return strlen(buffer);
}

// Write a base64 encoded opaque value to the buffer provided
static int JsonEncodeOpaque(char * buffer, int bufferLen, char * id, char * value, int len, bool last)
{
    char * buf;
    int outLength;

    if ((buffer == NULL) || (value == NULL))
    {
        Lwm2m_Error("ERROR: input or output buffers cannot be NULL\n");
        return -1;
    }

    outLength = (((len + 2) * 4) / 3);
    buf = (char * )malloc(outLength);
    if (buf == NULL)
    {
        return 0;
    }

    b64Encode(buf, outLength, value, len);

    sprintf(buffer, "{\"n\":\"%s\",\"sv\":\"%s\"}", id, buf);
    if (!last)
    {
        strcat(buffer,",\n");
    }
    else
    {
        strcat(buffer,"]\n");
    }

    free(buf);
    return strlen(buffer);
}

// Write a JSON encoded string to the buffer provided
static int JsonEncodeString(char * buffer, int bufferLen, char * id, char * value, size_t size, bool last)
{
    int result = 0;

    char * temp = (char *)malloc(size + 1);
    if (temp != NULL)
    {
        memcpy(temp, value, size);
        temp[size] = '\0';

        sprintf(buffer, "{\"n\":\"%s\",\"sv\":\"%s\"}", id, temp);
        if (!last)
        {
            strcat(buffer,",\n");
        }
        else
        {
            strcat(buffer,"]\n");
        }
        free(temp);
        result = strlen(buffer);
    }

    return result;
}

// Write a JSON encoded integer to the buffer provided
static int JsonEncodeInteger(char * buffer, int bufferLen, char * id, int64_t value, bool last)
{
    sprintf(buffer, "{\"n\":\"%s\",\"v\":%" PRIu64"}", id, value);
    if (!last)
    {
        strcat(buffer,",\n");
    }
    else
    {
        strcat(buffer,"]\n");
    }

    return strlen(buffer);
}

// Write a JSON encoded float to the buffer provided
static int JsonEncodeFloat(char * buffer, int bufferLen, char * id, double value, bool last)
{
    sprintf(buffer, "{\"n\":\"%s\",\"v\":%f}", id, value);
    if (!last)
    {
        strcat(buffer,",\n");
    }
    else
    {
        strcat(buffer,"]\n");
    }
    return strlen(buffer);
}

// Write a JSON encoded boolean to the buffer provided
static int JsonEncodeBoolean(char * buffer, int bufferLen, char * id, bool value, bool last)
{
    sprintf(buffer, "{\"n\":\"%s\",\"bv\":\"%s\"}", id, value ? "true" : "false");
    if (!last)
    {
        strcat(buffer,",\n");
    }
    else
    {
        strcat(buffer,"]\n");
    }
    return strlen(buffer);
}

// Write a JSON encoded object-link to the buffer provided
static int JsonEncodeObjectLink(char * buffer, int bufferLen, char * id, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, bool last)
{
    sprintf(buffer, "{\"n\":\"%s\",\"sv\":\"%d:%d\"}", id, objectID, objectInstanceID);
    if (!last)
    {
        strcat(buffer,",\n");
    }
    else
    {
        strcat(buffer,"]\n");
    }
    return strlen(buffer);
}

// Write a JSON encoded resource instance to the buffer provided
static int JsonSerialiseResourceInstance(Lwm2mTreeNode * node, ResourceDefinition * definition, char * id, uint8_t * buffer, int len, bool last)
{
    int valueLength = -1;
    uint16_t size;
    uint8_t * value;

    if (Lwm2mTreeNode_GetType(node) != Lwm2mTreeNodeType_ResourceInstance)
    {
       Lwm2m_Error("ERROR: Resource Instance node type expected. Received %d\n", Lwm2mTreeNode_GetType(node));
       return -1;
    }

    value = (uint8_t * )Lwm2mTreeNode_GetValue(node, &size);
    switch (definition->Type)
    {
        case AwaResourceType_String:
            valueLength = JsonEncodeString((char *)buffer, len, id, (char *)value, size, last);
            break;
        case AwaResourceType_Boolean:
            valueLength = JsonEncodeBoolean((char *)buffer, len, id, *(bool*)value, last);
            break;
        case AwaResourceType_Time:  // no break
        case AwaResourceType_Integer:
            switch (size)
            {
               case sizeof(int8_t):
                   valueLength = JsonEncodeInteger((char *)buffer, len, id, ptrToInt8(value), last);
                   break;
               case sizeof(int16_t):
                   valueLength = JsonEncodeInteger((char *)buffer, len, id, ptrToInt16(value), last);
                   break;
               case sizeof(int32_t):
                   valueLength = JsonEncodeInteger((char *)buffer, len, id, ptrToInt32(value), last);
                   break;
               case sizeof(int64_t):
                   valueLength = JsonEncodeInteger((char *)buffer, len, id, ptrToInt64(value), last);
                   break;
               default:
                   break;
            }
            break;
        case AwaResourceType_Float:
            switch (size)
            {
                case sizeof(float):
                    valueLength = JsonEncodeFloat((char *)buffer, len, id, *(float*)value, last);
                    break;
                case sizeof(double):
                    valueLength = JsonEncodeFloat((char *)buffer, len, id, *(double*)value, last);
                    break;
                default:
                    Lwm2m_Error("ERROR: JSON - invalid length for float\n");
                    break;
            }
            break;
        case AwaResourceType_Opaque:
            valueLength = JsonEncodeOpaque((char *)buffer, len, id, (char *)value, size, last);
            break;
        case AwaResourceType_ObjectLink:
           {
               AwaObjectLink * objectLink = (AwaObjectLink *) value;
               valueLength = JsonEncodeObjectLink((char *)buffer, len, id, objectLink->ObjectID, objectLink->ObjectInstanceID, last);
               break;
           }
        default:
            printf("Unknown type\n");
            break;
    }

    return valueLength;
}

// Write a JSON encoded resource to the buffer provided
static int JsonSerialiseResource(SerdesContext * serdesContext, Lwm2mTreeNode * node, ObjectIDType objectID,
                                 ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, uint8_t * buffer, int len)
{
    if (Lwm2mTreeNode_GetType(node) != Lwm2mTreeNodeType_Resource)
    {
        Lwm2m_Error("ERROR: Resource node type expected. Received %d\n", Lwm2mTreeNode_GetType(node));
        return -1;
    }

    int resourceLength = 0;
    ResourceDefinition * definition = (ResourceDefinition *)Lwm2mTreeNode_GetDefinition(node);

    if (*serdesContext == NULL)
    {
        JsonInitSerdesContext(serdesContext, RESOURCE_URI);
        resourceLength += JsonStartElement((char *)buffer, len);
    }
    JsonSerdesContext * context = (JsonSerdesContext *)*serdesContext;

    Lwm2mTreeNode * child = Lwm2mTreeNode_GetFirstChild(node);
    while (child != NULL)
    {
        Lwm2mTreeNode * nextChild = Lwm2mTreeNode_GetNextChild(node, child);
        bool last = context->lastResource && (nextChild == NULL);
        char id[32];

        int resourceInstanceID;
        Lwm2mTreeNode_GetID(child, &resourceInstanceID);

        //uri = /O      id = I/R or I/R/Ri
        //uri = /O/I    id = R or R/Ri
        //uri = /O/I/R  id = 0 or Ri
        if (context->uriLevel == RESOURCE_URI)
        {
            sprintf(id, "%d", IS_MULTIPLE_INSTANCE(definition) ? resourceInstanceID : 0);
        }
        else if (context->uriLevel == INSTANCE_URI)
        {
            if (IS_MULTIPLE_INSTANCE(definition))
            {
                sprintf(id, "%d/%d", resourceID, resourceInstanceID);
            }
            else
            {
                sprintf(id, "%d", resourceID);
            }
        }
        else  // OBJECT_URI
        {
            if (IS_MULTIPLE_INSTANCE(definition))
            {
                sprintf(id, "%d/%d/%d", objectInstanceID, resourceID, resourceInstanceID);
            }
            else
            {
                sprintf(id, "%d/%d", objectInstanceID, resourceID);
            }
        }

        int valueLength = JsonSerialiseResourceInstance(child, definition, id, &buffer[resourceLength], len - resourceLength, last);
        if (valueLength <= 0)
        {
           Lwm2m_Error("ERROR: Failed to serialise resource instance\n");
           JsonDestroySerdesContext(serdesContext);
           return -1;
        }
        resourceLength += valueLength;
        child = nextChild;
    }

    if (context->uriLevel == RESOURCE_URI)
    {
        resourceLength += JsonEndElement((char *)&buffer[resourceLength], len - resourceLength);
        JsonDestroySerdesContext(serdesContext);
    }

    return resourceLength;
}

// Write a Json encoded instance to the buffer provided
static int JsonSerialiseObjectInstance(SerdesContext * serdesContext, Lwm2mTreeNode * node, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, uint8_t * buffer, int len)
{
    int instanceLength = 0;

    if (Lwm2mTreeNode_GetType(node) != Lwm2mTreeNodeType_ObjectInstance)
    {
        Lwm2m_Error("ERROR: Object instance node type expected. Received %d\n", Lwm2mTreeNode_GetType(node));
        return -1;
    }

    if (*serdesContext == NULL)
    {
        JsonInitSerdesContext(serdesContext, INSTANCE_URI);
        instanceLength += JsonStartElement((char *)buffer, len);
    }
    JsonSerdesContext * context = (JsonSerdesContext *)*serdesContext;

    Lwm2mTreeNode * child = Lwm2mTreeNode_GetFirstChild(node);
    while (child != NULL)
    {
        Lwm2mTreeNode * nextChild = Lwm2mTreeNode_GetNextChild(node, child);
        int resourceID;
        Lwm2mTreeNode_GetID(child, &resourceID);
        context->lastResource = context->lastInstance && (nextChild == NULL);

        int resourceLength = JsonSerialiseResource(serdesContext, child, objectID, objectInstanceID, resourceID, &buffer[instanceLength], len - instanceLength);
        if (resourceLength <= 0)
        {
            Lwm2m_Error("Failed to serialise resource\n");
            JsonDestroySerdesContext(serdesContext);
            return -1;
        }
        instanceLength += resourceLength;

        child = nextChild;
    }

    if (context->uriLevel == INSTANCE_URI)
    {
        instanceLength += JsonEndElement((char *)&buffer[instanceLength], len - instanceLength);
        JsonDestroySerdesContext(serdesContext);
    }

    return instanceLength;
}

// Write a Json encoded object to the buffer provided
static int JsonSerialiseObject(SerdesContext * serdesContext, Lwm2mTreeNode * node, ObjectIDType objectID, uint8_t * buffer, int len)
{
    int pos = 0;

    if (Lwm2mTreeNode_GetType(node) != Lwm2mTreeNodeType_Object)
    {
        Lwm2m_Error("ERROR: Object node type expected. Received %d\n", Lwm2mTreeNode_GetType(node));
        return -1;
    }

    JsonInitSerdesContext(serdesContext, OBJECT_URI);
    JsonSerdesContext * context = (JsonSerdesContext *)*serdesContext;
    pos += JsonStartElement((char *)buffer, len);

    Lwm2mTreeNode * child = Lwm2mTreeNode_GetFirstChild(node);
    while (child != NULL)
    {
        Lwm2mTreeNode * nextChild = Lwm2mTreeNode_GetNextChild(node, child);
        context->lastInstance = nextChild == NULL;
        int objectInstanceID;
        Lwm2mTreeNode_GetID(child, &objectInstanceID);

        int instanceLength = JsonSerialiseObjectInstance(serdesContext, child, objectID, objectInstanceID, &buffer[pos], len - pos);
        if (instanceLength <= 0)
        {
            Lwm2m_Error("Failed to serialise object instance\n");
            JsonDestroySerdesContext(serdesContext);
            return -1;
        }

        pos += instanceLength;

        child = nextChild;
    }

    pos += JsonEndElement((char *)&buffer[pos], len - pos);
    JsonDestroySerdesContext(serdesContext);

    return pos;
}

static Lwm2mTreeNode * AddObjectNode(Lwm2mTreeNode * root, const DefinitionRegistry * registry, ObjectIDType objectID)
{
    Lwm2mTreeNode * objectNode = (root != NULL) ? Lwm2mTreeNode_FindNode(root, objectID) : NULL;
    if (objectNode == NULL)
    {
        ObjectDefinition * definition = Definition_LookupObjectDefinition(registry, objectID);

        if (definition == NULL)
        {
            Lwm2m_Error("ERROR: Failed to determine resource definition Object %d\n", objectID);
            return NULL;
        }

        objectNode = Lwm2mTreeNode_Create();
        Lwm2mTreeNode_SetID(objectNode, objectID);
        Lwm2mTreeNode_SetType(objectNode, Lwm2mTreeNodeType_Object);

        if (Lwm2mTreeNode_SetDefinition(objectNode, definition) != 0)
        {
            Lwm2m_Error("ERROR: Failed to set definition Object %d\n", objectID);
            Lwm2mTreeNode_DeleteRecursive(objectNode);
            return NULL;
        }

        if (root != NULL)
        {
            Lwm2mTreeNode_AddChild(root, objectNode);
        }
    }

    return objectNode;
}

static Lwm2mTreeNode * AddObjectInstanceNode(Lwm2mTreeNode * objectNode, const DefinitionRegistry * registry, ObjectIDType instanceID)
{
    // Lookup instance node and create it if it doesn't exist.
    Lwm2mTreeNode * instanceNode = (objectNode != NULL) ? Lwm2mTreeNode_FindNode(objectNode, instanceID) : NULL;
    if (instanceNode == NULL)
    {
        instanceNode = Lwm2mTreeNode_Create();
        Lwm2mTreeNode_SetID(instanceNode, instanceID);
        Lwm2mTreeNode_SetType(instanceNode, Lwm2mTreeNodeType_ObjectInstance);
        Lwm2mTreeNode_SetDefinition(instanceNode, Lwm2mTreeNode_GetDefinition(objectNode));
        if (objectNode != NULL)
        {
            Lwm2mTreeNode_AddChild(objectNode, instanceNode);
        }
    }

    return instanceNode;
}

static Lwm2mTreeNode * AddResourceNode(Lwm2mTreeNode * instanceNode, const DefinitionRegistry * registry, ObjectIDType objectID, ResourceIDType resourceID)
{
    Lwm2mTreeNode * resourceNode = (instanceNode != NULL) ? Lwm2mTreeNode_FindNode(instanceNode, resourceID) : NULL;
    if (resourceNode == NULL)
    {
        ResourceDefinition * definition = Definition_LookupResourceDefinition(registry, objectID, resourceID);

        if (definition == NULL)
        {
            Lwm2m_Error("ERROR: Failed to determine resource definition Object %d Resource %d\n", objectID, resourceID);
            return NULL;
        }

        resourceNode = Lwm2mTreeNode_Create();
        Lwm2mTreeNode_SetID(resourceNode, resourceID);
        Lwm2mTreeNode_SetType(resourceNode, Lwm2mTreeNodeType_Resource);

        if (Lwm2mTreeNode_SetDefinition(resourceNode, definition) != 0)
        {
            Lwm2m_Error("ERROR: Failed to set definition Object %d Resource %d\n", objectID, resourceID);
            Lwm2mTreeNode_DeleteRecursive(resourceNode);
            return NULL;
        }

        if (instanceNode != NULL)
        {
            Lwm2mTreeNode_AddChild(instanceNode, resourceNode);
        }
    }

    return resourceNode;
}

static int JsonDeserialise(Lwm2mTreeNode ** dest, const DefinitionRegistry * registry, ObjectIDType objectID,
                           ObjectInstanceIDType instanceID, ResourceIDType resourceID, const uint8_t * buf, int bufferLen)
{
    int result;
    int index;
    jsmn_parser p;
    jsmntok_t tokens[MAX_JSON_TOKENS];
    enum { BASENAME_SIZE = 128 };
    char basename[BASENAME_SIZE];
    uint64_t basetime = 0;
    const char * buffer = (const char *)buf;

    jsmn_init(&p);

    result = jsmn_parse(&p, buffer, bufferLen, tokens, (sizeof(tokens) / sizeof(*tokens)));
    if (result < 0)
    {
        Lwm2m_Error("ERROR: deserialising JSON, malformed!\n");
        return -1;
    }


    jsmntok_t *root = &tokens[0]; // {
    if (root->type != JSMN_OBJECT)
    {
        return -1;
    }

    jsmntok_t * t;
    // First lookup the basename
    if ((index = JsonTokenFind(tokens, buffer, root, "bn")) != -1)
    {
        t = &tokens[tokens[index].first_child];
        if (t->type != JSMN_STRING)
        {
            return -1;
        }
        strncpy(basename, JsonTokenToString(buffer, t), BASENAME_SIZE);
        basename[BASENAME_SIZE - 1] = '\0'; // Defensive

        *dest = Lwm2mTreeNode_Create();
        Lwm2mTreeNode_SetType(*dest, Lwm2mTreeNodeType_Root);
    }
    else
    {
        if (resourceID != -1)
        {
            sprintf(basename, "/%d/%d/%d/", objectID, instanceID, resourceID);
            *dest = AddResourceNode(NULL, registry, objectID, resourceID);
        }
        else
        {
            if (instanceID != -1)
            {
                sprintf(basename, "/%d/%d/", objectID, instanceID);
                *dest = AddObjectInstanceNode(NULL, registry, objectID);
            }
            else
            {
                sprintf(basename, "/%d/", objectID);
                *dest = AddObjectNode(NULL, registry, objectID);
            }
        }
    }

    // Lookup base time
    if ((index = JsonTokenFind(tokens, buffer, root, "bt")) != -1)
    {
        t = &tokens[tokens[index].first_child];
        if (t->type != JSMN_PRIMITIVE)
        {
            return -1;
        }

        char * string = JsonTokenToString(buffer, t);
        sscanf(string, "%24" SCNu64, &basetime);
    }

    // Lookup resource array
    if ((index = JsonTokenFind(tokens, buffer, root, "e")) != -1)
    {
        jsmntok_t * resourceArray = &tokens[tokens[index].first_child];
        if (resourceArray->type != JSMN_ARRAY)
        {
            return -1;
        }

        // Add Resource nodes
        if (resourceArray->size <= 0)
        {
            return -1;
        }

        index = resourceArray->first_child;
        do
        {
            // {"n":"<name>","<type>":"<data>"},
            t = &tokens[index];
            if (t->type != JSMN_OBJECT)
            {
                return -1;
            }

            // "n"
            t = &tokens[t->first_child];
            if (t->type != JSMN_STRING)
            {
                return -1;
            }

            if (!JsonTokenStrcmp(buffer, t, "n"))
            {
                return -1;
            }

            // resource name
            jsmntok_t * key = &tokens[t->first_child];
            if (key->type != JSMN_STRING)
            {
                return -1;
            }

            char * name = JsonTokenToString(buffer, key);

            // append the basename and then parse the O/I/R/Ri
            int resourceInstanceID = 0;
            char base[256];
            snprintf(base, 256, "%s%s", basename, name);
            sscanf(base, "/%5d/%5d/%5d/%5d", &objectID, &instanceID, &resourceID, &resourceInstanceID);

            Lwm2mTreeNode * resourceNode;
            Lwm2mTreeNode * resourceValueNode;

            if (Lwm2mTreeNode_GetType(*dest) == Lwm2mTreeNodeType_Root)
            {
                Lwm2mTreeNode * objectNode = AddObjectNode(*dest, registry, objectID);
                Lwm2mTreeNode * instanceNode = AddObjectInstanceNode(objectNode, registry, instanceID);
                resourceNode = AddResourceNode(instanceNode, registry, objectID, resourceID);
            }
            else if (Lwm2mTreeNode_GetType(*dest) == Lwm2mTreeNodeType_Object)
            {
                Lwm2mTreeNode * instanceNode = AddObjectInstanceNode(*dest, registry, instanceID);
                resourceNode = AddResourceNode(instanceNode, registry, objectID, resourceID);
            }
            else if (Lwm2mTreeNode_GetType(*dest) == Lwm2mTreeNodeType_ObjectInstance)
            {
                // lookup resource node, create if doesn't exist.
                resourceNode = AddResourceNode(*dest, registry, objectID, resourceID);
            }
            else
            {
                // Deserialise a single resource.
                resourceNode = *dest;
            }

            // Data type, "sv", "v", "bv" etc
            JsonDataType jsonDataType;
            t = &tokens[t->next_sibling];
            if (t->type != JSMN_STRING)
            {
                return -1;
            }

            if (JsonTokenStrcmp(buffer, t, "sv"))
            {
                jsonDataType = JSON_TYPE_STRING;
            }
            else if (JsonTokenStrcmp(buffer, t, "v"))
            {
                jsonDataType = JSON_TYPE_FLOAT;
            }
            else if (JsonTokenStrcmp(buffer, t, "bv"))
            {
                jsonDataType = JSON_TYPE_BOOLEAN;
            }
            else if (JsonTokenStrcmp(buffer, t, "ov"))
            {
                jsonDataType = JSON_TYPE_OBJECT_LINK;
            }
            else
            {
                return -1;
            }

            // Process value
            t = &tokens[t->first_child];
            if ((t->type != JSMN_STRING) && (t->type != JSMN_PRIMITIVE))
            {
                return -1;
            }
            char * value = JsonTokenToString(buffer, t);

            resourceValueNode = Lwm2mTreeNode_Create();
            Lwm2mTreeNode_SetID(resourceValueNode, resourceInstanceID);
            Lwm2mTreeNode_SetType(resourceValueNode, Lwm2mTreeNodeType_ResourceInstance);

            int resourceType = Definition_GetResourceType(registry, objectID, resourceID);
            switch (resourceType)
            {
                case AwaResourceType_Time:
                    if (jsonDataType == JSON_TYPE_FLOAT)
                    {
                        int64_t temp = 0;
                        result = sscanf((char *)value, "%24" SCNu64, &temp);
                        if (result > 0)
                        {
                            // adjust time based on the basetime.
                            temp -= basetime;
                            result = Lwm2mTreeNode_SetValue(resourceValueNode, (const uint8_t *)&temp, sizeof(temp));
                        }
                    }
                    else
                    {
                        Lwm2mTreeNode_DeleteRecursive(resourceValueNode);
                        return -1;
                    }
                    break;

                case AwaResourceType_Float:  // no break
                case AwaResourceType_Integer:

                    if (jsonDataType != JSON_TYPE_FLOAT)
                    {
                        Lwm2mTreeNode_DeleteRecursive(resourceValueNode);
                        return -1;
                    }

                    if (resourceType == AwaResourceType_Float)
                    {
                        double temp = 0;
                        result = sscanf((char *)value, "%24lf", &temp);
                        if (result > 0)
                        {
                            result = Lwm2mTreeNode_SetValue(resourceValueNode, (const uint8_t *)&temp, sizeof(temp));
                        }
                    }
                    else
                    {
                        int64_t temp = 0;
                        result = sscanf((char *)value, "%24" SCNu64, &temp);
                        if (result > 0)
                        {
                            result = Lwm2mTreeNode_SetValue(resourceValueNode, (const uint8_t *)&temp, sizeof(temp));
                        }
                    }
                    break;

                case AwaResourceType_Boolean:
                    if (jsonDataType == JSON_TYPE_BOOLEAN)
                    {
                        int64_t temp = (strcmp(value, "true")) ? 1: 0;
                        result = Lwm2mTreeNode_SetValue(resourceValueNode, (const uint8_t *)&temp, sizeof(temp));
                    }
                    else
                    {
                        Lwm2mTreeNode_DeleteRecursive(resourceValueNode);
                        return -1;
                    }
                    break;

                case AwaResourceType_Opaque:
                    {
                        if (jsonDataType != JSON_TYPE_STRING)
                        {
                            Lwm2mTreeNode_DeleteRecursive(resourceValueNode);
                            return -1;
                        }

                        int outLength = ((strlen(value) * 3) / 4);  // every 4 base encoded bytes are decoded to 3 bytes,
                        char * decodedValue = (char *)malloc(outLength);
                        if (decodedValue == NULL)
                        {
                            Lwm2mTreeNode_DeleteRecursive(resourceValueNode);
                            return -1;
                        }
                        int decodedLength = b64Decode(decodedValue, outLength, value, strlen(value));
                        if (decodedLength >= 0)
                        {
                            result = Lwm2mTreeNode_SetValue(resourceValueNode, (const uint8_t *)decodedValue, decodedLength);
                        }
                        else
                        {
                            result = -1;
                        }
                        free(decodedValue);
                    }
                    break;

                case AwaResourceType_String:
                    if (jsonDataType != JSON_TYPE_STRING)
                    {
                        Lwm2mTreeNode_DeleteRecursive(resourceValueNode);
                        return -1;
                    }

                    result = Lwm2mTreeNode_SetValue(resourceValueNode, (const uint8_t *)&value[0], strlen(value) + 1);
                    break;

                case AwaResourceType_ObjectLink:
                    {
                        if (jsonDataType != JSON_TYPE_OBJECT_LINK)
                        {
                            Lwm2mTreeNode_DeleteRecursive(resourceValueNode);
                            return -1;
                        }

                        AwaObjectLink objectLink;
                        result = sscanf(value, "%10d:%10d", &objectLink.ObjectID, &objectLink.ObjectInstanceID);
                        if (result > 0)
                        {
                            result = Lwm2mTreeNode_SetValue(resourceValueNode, (const uint8_t *)&objectLink, sizeof(objectLink));
                        }
                    }
                    break;

                default:
                    break;
            }

            if (result >= 0)
            {
                Lwm2mTreeNode_AddChild(resourceNode, resourceValueNode);
            }
            else
            {
                Lwm2mTreeNode_DeleteRecursive(resourceValueNode);
            }

        } while((index = tokens[index].next_sibling) != -1);
    } // "e"

    return result;
}

static int JsonDeserialiseResource(SerdesContext * serdesContext, Lwm2mTreeNode ** dest, const DefinitionRegistry * registry, ObjectIDType objectID,
                                   ObjectInstanceIDType instanceID, ResourceIDType resourceID, const uint8_t * buf, int bufferLen)
{
    return JsonDeserialise(dest, registry, objectID, instanceID, resourceID, buf, bufferLen);
}

static int JsonDeserialiseObjectInstance(SerdesContext * serdesContext, Lwm2mTreeNode ** dest, const DefinitionRegistry * registry,
                                         ObjectIDType objectID, ObjectInstanceIDType instanceID, const uint8_t * buf, int bufferLen)
{
    return JsonDeserialise(dest, registry, objectID, instanceID, -1, buf, bufferLen);
}

static int JsonDeserialiseObject(SerdesContext * serdesContext, Lwm2mTreeNode ** dest, const DefinitionRegistry * registry,
                                 ObjectIDType objectID, const uint8_t * buf, int bufferLen)
{
    return JsonDeserialise(dest, registry, objectID, -1, -1, buf, bufferLen);
}

// Map JSON serdes function delegates
const SerialiserDeserialiser jsonSerDes =
{
    .SerialiseObject           = JsonSerialiseObject,
    .SerialiseObjectInstance   = JsonSerialiseObjectInstance,
    .SerialiseResource         = JsonSerialiseResource,
    .DeserialiseObject         = JsonDeserialiseObject,
    .DeserialiseObjectInstance = JsonDeserialiseObjectInstance,
    .DeserialiseResource       = JsonDeserialiseResource,
};

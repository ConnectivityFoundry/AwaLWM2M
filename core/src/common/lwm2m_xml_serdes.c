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

ResourceTypeEnum xmlif_StringToDataType(const char * value)
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

const char * xmlif_DataTypeToString(ResourceTypeEnum type)
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

char * xmlif_EncodeValue(ResourceTypeEnum dataType, const char * buffer, int bufferLength)
{
    int outLength = 0;
    char * dataValue = NULL;

    if (buffer == NULL)
    {
        return NULL;
    }

    switch(dataType)
    {
        case ResourceTypeEnum_TypeString:
        {
            // adjust string length because object store expects Pascal strings
            if (bufferLength > 0 && buffer[bufferLength - 1] == 0)
            {
                int lengthAsString = strlen(buffer);
                bufferLength = lengthAsString < bufferLength ? lengthAsString : bufferLength;
            }
        }
        // no break - intentional fall-through

        case ResourceTypeEnum_TypeOpaque:
            outLength = (((bufferLength + 2) * 4) / 3);
            dataValue = malloc(outLength + 1);
            if (dataValue == NULL)
            {
                Lwm2mResult_SetResult(Lwm2mResult_OutOfMemory);
                goto error;
            }
            dataValue[outLength] = '\0';
            outLength = b64Encode(dataValue, outLength, (char*)buffer, bufferLength);
            break;

        case ResourceTypeEnum_TypeFloat:

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
                Lwm2mResult_SetResult(Lwm2mResult_InternalError);
                goto error;
            }
            break;

        case ResourceTypeEnum_TypeInteger:  // no break
        case ResourceTypeEnum_TypeTime:

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
                Lwm2mResult_SetResult(Lwm2mResult_OutOfMemory);
                goto error;
            }

            break;

        case ResourceTypeEnum_TypeBoolean:
            outLength = asprintf(&dataValue, "%s", (*(bool*)&buffer[0]) ? "True" : "False");

            if ((outLength <= 0) || (dataValue == NULL))
            {
                Lwm2mResult_SetResult(Lwm2mResult_OutOfMemory);
                goto error;
            }
            break;

        case ResourceTypeEnum_TypeObjectLink:;
            ObjectLink * objectLink = (ObjectLink *) buffer;
            outLength = asprintf(&dataValue, "%"PRIu16":%"PRIu16, objectLink->ObjectID, objectLink->ObjectInstanceID);

            if ((outLength <= 0) || (dataValue == NULL))
            {
                Lwm2mResult_SetResult(Lwm2mResult_OutOfMemory);
                goto error;
            }
            break;

        case ResourceTypeEnum_TypeNone:
        default:
            Lwm2mResult_SetResult(Lwm2mResult_BadRequest);
            goto error;
    }

    Lwm2mResult_SetResult(Lwm2mResult_Success);
error:
    return dataValue;
}

int xmlif_DecodeValue(char ** dataValue, ResourceTypeEnum dataType, const char * buffer, int bufferLength)
{
    int dataLength = -1;
    int outLength;

    switch(dataType)
    {
        case ResourceTypeEnum_TypeString:
            //
            outLength = ((bufferLength * 3) / 4);  // every 4 base encoded bytes are decoded to 3 bytes,
            *dataValue = malloc(outLength);
            if (*dataValue == NULL)
            {
                Lwm2mResult_SetResult(Lwm2mResult_OutOfMemory);
                goto error;
            }
            dataLength = b64Decode(*dataValue, outLength, (char*)buffer, bufferLength);

            if (dataLength == -1)
            {
                Lwm2mResult_SetResult(Lwm2mResult_BadRequest);
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
        case ResourceTypeEnum_TypeOpaque: case ResourceTypeEnum_TypeNone: // TypeNone for Executable payload which is an Opaque
            outLength = ((bufferLength * 3) / 4);  // every 4 base encoded bytes are decoded to 3 bytes
            *dataValue = malloc(outLength);
            if (*dataValue == NULL)
            {
                Lwm2mResult_SetResult(Lwm2mResult_OutOfMemory);
                goto error;
            }
            dataLength = b64Decode(*dataValue, outLength, (char*)buffer, bufferLength);
            if (dataLength == -1)
            {
                Lwm2mResult_SetResult(Lwm2mResult_BadRequest);
                goto error;
            }
            break;

        case ResourceTypeEnum_TypeFloat:
        {
            double d;
            *dataValue = malloc(sizeof(double));
            if (*dataValue == NULL)
            {
                Lwm2mResult_SetResult(Lwm2mResult_OutOfMemory);
                goto error;
            }
            sscanf(buffer, "%20lf", &d);
            dataLength = sizeof(double);
            memcpy(*dataValue, &d, dataLength);
            break;
        }

        case ResourceTypeEnum_TypeInteger:  // no break
        case ResourceTypeEnum_TypeTime:
        {
            uint64_t u;
            *dataValue = malloc(sizeof(uint64_t));
            if (*dataValue == NULL)
            {
                Lwm2mResult_SetResult(Lwm2mResult_OutOfMemory);
                goto error;
            }
            sscanf(buffer, "%"SCNd64, &u);
            dataLength = sizeof(uint64_t);
            memcpy(*dataValue, &u, dataLength);
            break;
        }

        case ResourceTypeEnum_TypeBoolean:
            *dataValue = malloc(sizeof(bool));
            if (*dataValue == NULL)
            {
                Lwm2mResult_SetResult(Lwm2mResult_OutOfMemory);
                goto error;
            }
            (*dataValue)[0] = (strcmp(buffer, "True") == 0);
            dataLength = sizeof(bool);
            break;

        case ResourceTypeEnum_TypeObjectLink:
        {
            dataLength = sizeof(ObjectLink);
            *dataValue = malloc(dataLength);

            if (*dataValue == NULL)
            {
                Lwm2mResult_SetResult(Lwm2mResult_OutOfMemory);
                goto error;
            }
            ObjectLink * objectLink = (ObjectLink *)*dataValue;
            sscanf(buffer, "%"SCNu16":%"SCNu16,&objectLink->ObjectID, &objectLink->ObjectInstanceID);

            break;
        }

        //case ResourceTypeEnum_TypeNone:
        default:
            Lwm2mResult_SetResult(Lwm2mResult_BadRequest);
            goto error;
    }

    Lwm2mResult_SetResult(Lwm2mResult_Success);
error:
    return dataLength;
}

Lwm2mTreeNode * xmlif_xmlObjectToLwm2mObject(Lwm2mContextType * context, const TreeNode xmlObjectNode, bool readValues)
{
    Lwm2mTreeNode * objectNode = Lwm2mTreeNode_Create();
    Lwm2mTreeNode_SetType(objectNode, Lwm2mTreeNodeType_Object);
    int objectID = xmlif_GetInteger(xmlObjectNode, "Object/ID");
    Lwm2mTreeNode_SetCreateFlag(objectNode, Xml_Find(xmlObjectNode, "Create"));
    Lwm2mTreeNode_SetID(objectNode, objectID);

    ObjectDefinition * definition = Definition_LookupObjectDefinition(Lwm2mCore_GetDefinitions(context), objectID);
    if (definition != NULL)
    {
        Lwm2mTreeNode_SetDefinition(objectNode, definition);

        uint32_t instanceIndex = 0;
        TreeNode xmlObjectInstanceNode = NULL;
        while ((xmlObjectInstanceNode = Xml_FindFrom(xmlObjectNode, "ObjectInstance", &instanceIndex)) != NULL)
        {
            int instanceID;

            bool createInstance = Xml_Find(xmlObjectInstanceNode, "Create");
            instanceID = xmlif_GetInteger(xmlObjectInstanceNode, "ObjectInstance/ID");

            if (instanceID == -1 && !createInstance)
            {
                Lwm2m_Error("Missing instance ID node in instance for object ID %d\n", objectID);
                Lwm2mTreeNode_DeleteRecursive(objectNode);
                objectNode = NULL;
                goto error;
            }

            Lwm2mTreeNode * objectInstanceNode = Lwm2mTreeNode_Create();
            if (instanceID != -1)
            {
                Lwm2mTreeNode_SetID(objectInstanceNode, (uint16_t)instanceID);
            }
            Lwm2mTreeNode_SetType(objectInstanceNode, Lwm2mTreeNodeType_ObjectInstance);
            Lwm2mTreeNode_SetCreateFlag(objectInstanceNode, createInstance);
            Lwm2mTreeNode_AddChild(objectNode, objectInstanceNode);

            if (instanceID != -1)
            {
                uint32_t propertyIndex = 0;
                TreeNode xmlResourceNode = NULL;
                while ((xmlResourceNode = Xml_FindFrom(xmlObjectInstanceNode, "Resource", &propertyIndex)) != NULL)
                {
                    uint16_t resourceID;
                    bool createOptionalResource = Xml_Find(xmlResourceNode, "Create") != NULL;

                    if ((resourceID = xmlif_GetInteger(xmlResourceNode, "Resource/ID")) == -1)
                    {
                        Lwm2m_Error("Missing resource ID node in for object instance %d/%d\n", objectID, instanceID);
                        Lwm2mTreeNode_DeleteRecursive(objectNode);
                        objectNode = NULL;
                        goto error;
                    }

                    Lwm2mTreeNode * resourceNode = Lwm2mTreeNode_Create();
                    Lwm2mTreeNode_SetID(resourceNode, resourceID);
                    Lwm2mTreeNode_SetType(resourceNode, Lwm2mTreeNodeType_Resource);
                    Lwm2mTreeNode_SetCreateFlag(resourceNode, createOptionalResource);
                    Lwm2mTreeNode_AddChild(objectInstanceNode, resourceNode);

                    ResourceDefinition * resourceDefinition = Definition_LookupResourceDefinition(Lwm2mCore_GetDefinitions(context), objectID, resourceID);
                    if (resourceDefinition != NULL)
                    {
                        Lwm2mTreeNode_SetDefinition(resourceNode, resourceDefinition);

                        if (!IS_MULTIPLE_INSTANCE(resourceDefinition))
                        {
                            uint16_t resourceInstanceID = 0;
                            Lwm2mTreeNode * resourceInstanceNode = Lwm2mTreeNode_Create();
                            Lwm2mTreeNode_SetID(resourceInstanceNode, resourceInstanceID);
                            Lwm2mTreeNode_SetType(resourceInstanceNode, Lwm2mTreeNodeType_ResourceInstance);

                            if (readValues)
                            {
                                if (!createOptionalResource || Xml_Find(xmlResourceNode, "Value"))
                                {
                                    const char * data;
                                    int dataLength;
                                    char * dataValue = NULL;
                                    if ((data = (char*)xmlif_GetOpaque(xmlResourceNode, "Resource/Value")) == NULL)
                                    {
                                        Lwm2m_Error("Missing value data for resource %d/%d/%d\n", objectID, instanceID, resourceID);
                                        Lwm2mTreeNode_DeleteRecursive(objectNode);
                                        objectNode = NULL;
                                        Lwm2mTreeNode_DeleteRecursive(resourceInstanceNode);
                                        resourceInstanceNode = NULL;
                                        goto error;
                                    }

                                    dataLength = xmlif_DecodeValue(&dataValue, resourceDefinition->Type, data, strlen(data));
                                    if (dataLength < 0)
                                    {
                                        Lwm2m_Error("Failed to decode value data for resource %d/%d/%d\n", objectID, instanceID, resourceID);
                                        free(dataValue);
                                        Lwm2mTreeNode_DeleteRecursive(objectNode);
                                        objectNode = NULL;
                                        Lwm2mTreeNode_DeleteRecursive(resourceInstanceNode);
                                        resourceInstanceNode = NULL;
                                        goto error;
                                    }

                                    Lwm2mTreeNode_SetValue(resourceInstanceNode, (const uint8_t*)dataValue, dataLength);
                                    free(dataValue);

                                    Lwm2mTreeNode_AddChild(resourceNode, resourceInstanceNode);
                                    resourceInstanceNode = NULL;
                                }
                            }

                            if (resourceInstanceNode != NULL)
                            {
                                Lwm2mTreeNode_DeleteRecursive(resourceInstanceNode);
                                resourceInstanceNode = NULL;
                            }
                        }
                        else
                        {
                            uint32_t valueIndex = 0;
                            TreeNode xmlResourceInstanceNode = NULL;
                            while ((xmlResourceInstanceNode = Xml_FindFrom(xmlResourceNode, "ResourceInstance", &valueIndex)) != NULL)
                            {
                                uint16_t valueID = xmlif_GetInteger(xmlResourceInstanceNode, "ResourceInstance/ID");
                                Lwm2mTreeNode * resourceInstanceNode = Lwm2mTreeNode_Create();
                                Lwm2mTreeNode_SetID(resourceInstanceNode, valueID);
                                Lwm2mTreeNode_SetType(resourceInstanceNode, Lwm2mTreeNodeType_ResourceInstance);

                                if (readValues)
                                {
                                    const char * data;
                                    int dataLength;
                                    char * dataValue = NULL;
                                    if ((data = (char*)xmlif_GetOpaque(xmlResourceInstanceNode, "ResourceInstance/Value")) == NULL)
                                    {
                                        Lwm2m_Error("Missing value data for resource %d/%d/%d\n", objectID, instanceID, resourceID);
                                        Lwm2mTreeNode_DeleteRecursive(objectNode);
                                        objectNode = NULL;
                                        Lwm2mTreeNode_DeleteRecursive(resourceInstanceNode);
                                        resourceInstanceNode = NULL;
                                        goto error;
                                    }

                                    dataLength = xmlif_DecodeValue(&dataValue, resourceDefinition->Type, data, strlen(data));
                                    if (dataLength < 0)
                                    {
                                        Lwm2m_Error("Failed to decode value data for resource %d/%d/%d\n", objectID, instanceID, resourceID);
                                        free(dataValue);
                                        Lwm2mTreeNode_DeleteRecursive(objectNode);
                                        objectNode = NULL;
                                        Lwm2mTreeNode_DeleteRecursive(resourceInstanceNode);
                                        resourceInstanceNode = NULL;
                                        goto error;
                                    }

                                    Lwm2mTreeNode_SetValue(resourceInstanceNode, (const uint8_t*)dataValue, dataLength);
                                    free(dataValue);

                                    Lwm2mTreeNode_AddChild(resourceNode, resourceInstanceNode);
                                }
                            }
                        }
                    }
                    else
                    {
                        Lwm2m_Debug("No definition for object %d resource %d\n", objectID, resourceID);
                        Lwm2mTreeNode_DeleteRecursive(objectNode);
                        objectNode = NULL;
                        goto error;
                    }
                }
            }
        }
    }
    else
    {
        Lwm2m_Debug("No definition for object /%d\n", objectID);
        Lwm2mTreeNode_DeleteRecursive(objectNode);
        objectNode = NULL;
        goto error;
    }

error:
    return objectNode;
}


#endif

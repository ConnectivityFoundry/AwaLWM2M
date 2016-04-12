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


#include <string.h>

#include "value.h"
#include "memalloc.h"
#include "lwm2m_types.h"
#include "utils.h"
#include "lwm2m_xml_serdes.h"
#include "xml.h"
#include "arrays.h"
#include "log.h"

struct _Value {
    AwaResourceType Type;
    void * Data;
    size_t Length;
};

Value * Value_New(TreeNode rootNode, AwaResourceType type)
{
    Value * value = NULL;
    if (rootNode != NULL)
    {
        TreeNode valueNode;

        if (!(type >= AwaResourceType_FirstArrayType && type <= AwaResourceType_LastArrayType))
        {
            valueNode = Xml_Find(rootNode, "Value");
            if (valueNode != NULL)
            {
                value = Awa_MemAlloc(sizeof(*value));
                if (value != NULL)
                {
                    memset(value, 0, sizeof(*value));
                    value->Type = type;

                    const char * data = (const char *)TreeNode_GetValue(valueNode);
                    char * dataValue = NULL;
                    AwaResourceType lwm2mType = Utils_GetPrimativeResourceType(type);
                    int dataLength = xmlif_DecodeValue(&dataValue, lwm2mType, data, strlen(data));
                    if (dataLength >= 0)
                    {
                        int valueID = xmlif_GetInteger(valueNode, "Value/ValueID");
                        if ((valueID == 0) || (valueID == -1))
                        {
                            // Success
                            switch(type)
                            {
                                case AwaResourceType_None:  // no break
                                case AwaResourceType_Opaque:
                                {
                                    AwaOpaque * opaque = Awa_MemAlloc(sizeof(*opaque));
                                    opaque->Data = NULL;
                                    opaque->Size = dataLength;
                                    if (dataLength > 0)
                                    {
                                        opaque->Data = Awa_MemAlloc(dataLength);
                                        memcpy(opaque->Data, dataValue, dataLength);
                                    }
                                    value->Length = sizeof(*opaque);
                                    value->Data = (void *)opaque;
                                    free(dataValue);
                                    break;
                                }

                                default:
                                    value->Length = dataLength;
                                    value->Data = dataValue;
                                    break;
                             }
                            LogNew("Value", value);
                        }
                        else
                        {
                            LogErrorWithEnum(AwaError_TypeMismatch, "value is not multiple instance");
                            free(dataValue);
                            Awa_MemSafeFree(value);
                            value = NULL;
                        }
                    }
                    else
                    {
                        LogErrorWithEnum(AwaError_Internal, "Failed to decode value data");
                        free(dataValue);
                        Awa_MemSafeFree(value);
                        value = NULL;
                    }
                }
            }
            else
            {
                // No value is fine - occurs in operations where we only expect path results in the response.
            }
        }
        else
        {
            uint32_t childIndex = 0;
            TreeNode resourceInstance = TreeNode_GetChild(rootNode, childIndex);
            AwaArray * array = NULL;
            switch(type)
            {
                case AwaResourceType_StringArray:
                    array = (AwaArray *) AwaStringArray_New();
                    break;
                case AwaResourceType_IntegerArray:
                    array = (AwaArray *) AwaIntegerArray_New();
                    break;
                case AwaResourceType_FloatArray:
                    array = (AwaArray *) AwaFloatArray_New();
                    break;
                case AwaResourceType_BooleanArray:
                    array = (AwaArray *) AwaBooleanArray_New();
                    break;
                case AwaResourceType_OpaqueArray:
                    array = (AwaArray *) AwaOpaqueArray_New();
                    break;
                case AwaResourceType_TimeArray:
                    array = (AwaArray *) AwaTimeArray_New();
                    break;
                case AwaResourceType_ObjectLinkArray:
                    array = (AwaArray *) AwaObjectLinkArray_New();
                    break;
                default:
                    LogErrorWithEnum(AwaError_Internal, "Could not create array of type %d", type);
                    break;
            }

            if (array != NULL)
            {
                while ((resourceInstance = TreeNode_GetChild(rootNode, childIndex)) != NULL)
                {
                    if (strcmp(TreeNode_GetName(resourceInstance), "ResourceInstance") == 0)
                    {
                        TreeNode idNode = Xml_Find(resourceInstance, "ID");
                        valueNode = Xml_Find(resourceInstance, "Value");

                        if ((valueNode != NULL) && (idNode != NULL))
                        {
                            AwaArrayIndex index = xmlif_GetInteger(idNode, "ID");

                            const char * data = (const char *)TreeNode_GetValue(valueNode);
                            char * dataValue = NULL;
                            AwaResourceType lwm2mType = Utils_GetPrimativeResourceType(type);
                            int dataLength = xmlif_DecodeValue(&dataValue, lwm2mType, data, strlen(data));

                            if ((dataValue != NULL) && (dataLength >= 0))
                            {
                                switch(type)
                                {
                                    case AwaResourceType_StringArray:
                                    {
                                        char * stringValue = Awa_MemAlloc(dataLength + 1);
                                        if (stringValue != NULL)
                                        {
                                            memcpy(stringValue, dataValue, (size_t)dataLength);
                                            stringValue[dataLength] = 0;
                                            AwaStringArray_SetValueAsCString((AwaStringArray *)array, index, stringValue);
                                            Awa_MemSafeFree(stringValue);
                                            break;
                                        }
                                        else
                                        {
                                            LogErrorWithEnum(AwaError_OutOfMemory);
                                            break;
                                        }
                                    }
                                    case AwaResourceType_OpaqueArray:
                                    {
                                        AwaOpaque opaque;
                                        opaque.Data = dataLength > 0 ? dataValue : NULL;
                                        opaque.Size = dataLength;
                                        AwaOpaqueArray_SetValue((AwaOpaqueArray *)array, index, opaque);
                                        break;
                                    }
                                    default:
                                        // Array_SetValue should eventually be replaced with the explicit
                                        // Awa*Array_SetValue functions.
                                        if (dataLength >= 0)
                                        {
                                            Array_SetValue(array, index, dataValue, dataLength);
                                        }
                                        break;
                                }
                                free(dataValue);
                            }
                            else
                            {
                                LogErrorWithEnum(AwaError_Internal, "resource instance failed to decode");
                                AwaArray_Free(&array, type);
                                break;
                            }
                        }
                        else
                        {
                            LogErrorWithEnum(AwaError_Internal, "invalid resource instance");
                            AwaArray_Free(&array, type);
                            break;
                        }
                    }
                    else
                    {
                        //skip ID etc
                        LogDebug("value skip %s", TreeNode_GetName(resourceInstance));
                    }

                    childIndex++;
                }

                if (array != NULL)
                {
                    value = Awa_MemAlloc(sizeof(*value));
                    if (value != NULL)
                    {
                        memset(value, 0, sizeof(*value));
                        value->Type = type;
                        value->Length = sizeof(array);
                        value->Data = array;
                        LogNew("Value", value);
                    }
                    else
                    {
                        AwaArray_Free(&array, type);
                    }
                }
            }
        }
    }
    else
    {
        LogErrorWithEnum(AwaError_OutOfMemory);
    }
    return value;
}

void Value_Free(Value ** value)
{
    if ((value != NULL) && (*value != NULL))
    {
        LogFree("Value", *value);
        if (((*value)->Type >= AwaResourceType_FirstArrayType) && ( (*value)->Type <= AwaResourceType_LastArrayType))
        {
            switch((*value)->Type)
            {
                case AwaResourceType_StringArray:
                    AwaStringArray_Free((AwaStringArray **)&((*value)->Data));
                    break;
                case AwaResourceType_IntegerArray:
                    AwaIntegerArray_Free((AwaIntegerArray **)&((*value)->Data));
                    break;
                case AwaResourceType_FloatArray:
                    AwaFloatArray_Free((AwaFloatArray **)&((*value)->Data));
                    break;
                case AwaResourceType_BooleanArray:
                    AwaBooleanArray_Free((AwaBooleanArray **)&((*value)->Data));
                    break;
                case AwaResourceType_OpaqueArray:
                    AwaOpaqueArray_Free((AwaOpaqueArray **)&((*value)->Data));
                    break;
                case AwaResourceType_TimeArray:
                    AwaTimeArray_Free((AwaTimeArray **)&((*value)->Data));
                    break;
                case AwaResourceType_ObjectLinkArray:
                    AwaObjectLinkArray_Free((AwaObjectLinkArray **)&((*value)->Data));
                    break;
                default:
                    LogErrorWithEnum(AwaError_Internal, "Could not free array of type %d", (*value)->Type);
                    break;
            }
        }
        else
        {
            if ((*value)->Type == AwaResourceType_Opaque || (*value)->Type == AwaResourceType_None)
            {
                AwaOpaque * opaque = (AwaOpaque * )((*value)->Data);
                free(opaque->Data);
            }

            free((*value)->Data);
            (*value)->Data = NULL;
        }
        (*value)->Type = AwaResourceType_Invalid;
        (*value)->Length = 0;
        Awa_MemSafeFree(*value);
        *value = NULL;
    }
}

AwaResourceType Value_GetType(const Value * value)
{
    AwaResourceType type = AwaResourceType_Invalid;
    if (value != NULL)
    {
        type = value->Type;
    }
    else
    {
        LogErrorWithEnum(AwaError_Internal, "value is NULL");
    }
    return type;
}

void * Value_GetData(const Value * value)
{
    void * data = NULL;
    if (value != NULL)
    {
        data = value->Data;
    }
    else
    {
        LogErrorWithEnum(AwaError_Internal, "value is NULL");
    }
    return data;

}

size_t Value_GetLength(const Value * value)
{
    size_t length = 0;
    if (value != NULL)
    {
        length = value->Length;
    }
    else
    {
        LogErrorWithEnum(AwaError_Internal, "value is NULL");
    }
    return length;
}


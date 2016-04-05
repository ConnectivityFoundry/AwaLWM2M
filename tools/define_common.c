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

#include "define_common.h"
#include "tools_common.h"

AwaResourceType ResourceTypeFromEnum(enum enum_resourceType value, bool isArray)
{
    typedef struct
    {
        enum enum_resourceType Value;
        AwaResourceType Type;
    } AwaResourceTypeMapEntry;

    AwaResourceTypeMapEntry mapSingle[] = {
            { resourceType_arg_opaque,     AwaResourceType_Opaque },
            { resourceType_arg_integer,    AwaResourceType_Integer },
            { resourceType_arg_float,      AwaResourceType_Float },
            { resourceType_arg_boolean,    AwaResourceType_Boolean },
            { resourceType_arg_string,     AwaResourceType_String },
            { resourceType_arg_time,       AwaResourceType_Time },
            { resourceType_arg_objlink,    AwaResourceType_ObjectLink },
            { resourceType_arg_none,       AwaResourceType_None },
    };
    size_t mapSingleLen = sizeof(mapSingle) / sizeof(mapSingle[0]);

    AwaResourceTypeMapEntry mapArray[] = {
            { resourceType_arg_opaque,     AwaResourceType_OpaqueArray },
            { resourceType_arg_integer,    AwaResourceType_IntegerArray },
            { resourceType_arg_float,      AwaResourceType_FloatArray },
            { resourceType_arg_boolean,    AwaResourceType_BooleanArray },
            { resourceType_arg_string,     AwaResourceType_StringArray },
            { resourceType_arg_time,       AwaResourceType_TimeArray },
            { resourceType_arg_objlink,    AwaResourceType_ObjectLinkArray },
    };
    size_t mapArrayLen = sizeof(mapArray) / sizeof(mapArray[0]);

    AwaResourceTypeMapEntry * map = mapSingle;
    size_t mapLen = mapSingleLen;
    if (isArray)
    {
        map = mapArray;
        mapLen = mapArrayLen;
    }

    AwaResourceType type = AwaResourceType_None;
    int i = 0;
    for (i = 0; i < mapLen && type == AwaResourceType_None; ++i)
    {
        if (value == map[i].Value)
        {
            type = map[i].Type;
        }
    }

    Debug("Type %d, %d\n", value, type);
    return type;
}

AwaResourceOperations OperationsFromEnum(enum enum_resourceOperations value)
{
    typedef struct
    {
        enum enum_resourceType Value;
        AwaResourceOperations Type;
    } AwaResourceTypeMapEntry;

    AwaResourceTypeMapEntry map[] = {
            { resourceOperations_arg_r,   AwaResourceOperations_ReadOnly },
            { resourceOperations_arg_w,   AwaResourceOperations_WriteOnly },
            { resourceOperations_arg_e,   AwaResourceOperations_Execute },
            { resourceOperations_arg_rw,  AwaResourceOperations_ReadWrite },
    };

    AwaResourceOperations type = AwaResourceOperations_Invalid;
    int i = 0;
    for (i = 0; i < sizeof(map) / sizeof(map[0]) && type == AwaResourceOperations_Invalid; ++i)
    {
        if ((int)value == (int)map[i].Value)
        {
            type = map[i].Type;
        }
    }

    Debug("Operation %d, %d\n", value, type);
    return type;
}

ResourceDescription * ResourceDescription_NewFromCmdline(struct gengetopt_args_info * ai, int index)
{
    ResourceDescription * resourceDescription = NULL;
    if (ai != NULL)
    {
        resourceDescription = malloc(sizeof(*resourceDescription));
        if (resourceDescription != NULL)
        {
            memset(resourceDescription, 0, sizeof(*resourceDescription));

            resourceDescription->ID = ai->resourceID_arg[index];
            resourceDescription->Name = strdup(ai->resourceName_arg[index]);
            resourceDescription->Type = ResourceTypeFromEnum(ai->resourceType_arg[index], ai->resourceInstances_arg[index] == resourceInstances_arg_multiple);
            resourceDescription->MinInstances = ai->resourceRequired_arg[index] == resourceRequired_arg_optional ? 0 : 1;
            resourceDescription->MaxInstances = ai->resourceInstances_arg[index] == resourceInstances_arg_single ? 1 : AWA_MAX_ID;
            resourceDescription->Operations = OperationsFromEnum(ai->resourceOperations_arg[index]);
        }
        else
        {
            Error("Out of memory\n");
        }
    }
    return resourceDescription;
}

void ResourceDescription_Free(ResourceDescription ** resourceDescription)
{
    if ((resourceDescription != NULL) && (*resourceDescription != NULL))
    {
        free((*resourceDescription)->Name);
        free(*resourceDescription);
        *resourceDescription = NULL;
    }
}

ObjectDescription * ObjectDescription_NewFromCmdline(struct gengetopt_args_info * ai)
{
    ObjectDescription * objectDescription = NULL;
    AwaObjectID objectID = AWA_INVALID_ID;
    const char * objectName = (ai && ai->objectName_given) ? ai->objectName_arg : "UNNAMED";

    if (ai != NULL)
    {
        if (ai->objectID_given)
        {
            int minInstances = 0;
            int maxInstances = 1;
            objectID = ai->objectID_arg;

            if (ai->objectMandatory_given)
            {
                minInstances = ai->objectMandatory_flag ? 1 : 0;
            }
            else
            {
                minInstances = 0; // Optional
            }

            if (ai->objectInstances_given)
            {
                maxInstances = (ai->objectInstances_arg == objectInstances_arg_multiple) ? AWA_MAX_ID : 1;
            }
            else
            {
                maxInstances = 1;
            }

            // Check that the same number of options are given for each resource
            int numResources = ai->resourceID_given;
            const char * errorMsg = NULL;
            if (numResources != ai->resourceName_given)
            {
                errorMsg = "Insufficient number of resourceName parameters\n";
            }

            if (numResources != ai->resourceType_given)
            {
                errorMsg = "Insufficient number of resourceType parameters\n";
            }

            if (numResources != ai->resourceInstances_given)
            {
                errorMsg = "Insufficient number of resourceInstances parameters\n";
            }

            if (numResources != ai->resourceRequired_given)
            {
                errorMsg = "Insufficient number of resourceRequired parameters\n";
            }

            if (numResources != ai->resourceOperations_given)
            {
                errorMsg = "Insufficient number of resourceOperations parameters\n";
            }

            if (errorMsg == NULL)
            {
                objectDescription = malloc(sizeof(*objectDescription));
                if (objectDescription != NULL)
                {
                    memset(objectDescription, 0, sizeof(*objectDescription));

                    objectDescription->ID = objectID;
                    objectDescription->Name = strdup(objectName);
                    objectDescription->MinInstances = minInstances;
                    objectDescription->MaxInstances = maxInstances;
                    objectDescription->NumResources = numResources;

                    objectDescription->ResourceDescription = malloc(sizeof(ResourceDescription *) * numResources);
                    if (objectDescription->ResourceDescription != NULL)
                    {
                        memset(objectDescription->ResourceDescription, 0, sizeof(ResourceDescription *) * numResources);
                        int i = 0;
                        for (i = 0; i < numResources; ++i)
                        {
                            objectDescription->ResourceDescription[i] = ResourceDescription_NewFromCmdline(ai, i);
                        }
                    }
                    else
                    {
                        Error("Out of memory\n");
                    }
                }
                else
                {
                    Error("Out of memory\n");
                }
            }
            else
            {
                Error("%s\n", errorMsg);
            }
        }
        else
        {
            Error("Object ID must be specified.\n");
            cmdline_parser_print_help();
        }
    }
    else
    {
        Error("ai is NULL\n");
    }
    return objectDescription;
}

void ObjectDescription_Free(ObjectDescription ** objectDescription)
{
    if (objectDescription != NULL && *objectDescription != NULL)
    {
        int i = 0;
        for (i = 0; i < (*objectDescription)->NumResources; ++i)
        {
            ResourceDescription_Free(&((*objectDescription)->ResourceDescription[i]));
        }
        free((*objectDescription)->ResourceDescription);
        free((*objectDescription)->Name);
        free(*objectDescription);
        *objectDescription = NULL;
    }
}

AwaError AddResource(AwaObjectDefinition * objectDefinition, const ResourceDescription * resourceDescription)
{
    // Default values declaration
    AwaObjectLink objLink = {0, 0};
    AwaOpaque defOpaque = {NULL, 0};
    AwaError result = AwaError_Unspecified;

    bool mandatory = resourceDescription->MinInstances > 0;

    switch (resourceDescription->Type)
    {
        case AwaResourceType_None:
            result = AwaObjectDefinition_AddResourceDefinitionAsNoType(objectDefinition, resourceDescription->ID, resourceDescription->Name, mandatory, resourceDescription->Operations);
            break;
        case AwaResourceType_String:
            result = AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, resourceDescription->ID, resourceDescription->Name, mandatory, resourceDescription->Operations, NULL);
            break;
        case AwaResourceType_Integer:
            result = AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefinition, resourceDescription->ID, resourceDescription->Name, mandatory, resourceDescription->Operations,0);
            break;
        case AwaResourceType_Float:
            result = AwaObjectDefinition_AddResourceDefinitionAsFloat(objectDefinition, resourceDescription->ID, resourceDescription->Name, mandatory, resourceDescription->Operations, 0.0);
            break;
        case AwaResourceType_Boolean:
            result = AwaObjectDefinition_AddResourceDefinitionAsBoolean(objectDefinition, resourceDescription->ID, resourceDescription->Name, mandatory, resourceDescription->Operations, false);
            break;
        case AwaResourceType_Opaque:
            result = AwaObjectDefinition_AddResourceDefinitionAsOpaque(objectDefinition, resourceDescription->ID, resourceDescription->Name, mandatory, resourceDescription->Operations, defOpaque);
            break;
        case AwaResourceType_Time:
            result = AwaObjectDefinition_AddResourceDefinitionAsTime(objectDefinition, resourceDescription->ID, resourceDescription->Name, mandatory, resourceDescription->Operations, 0);
            break;
        case AwaResourceType_ObjectLink:
            result = AwaObjectDefinition_AddResourceDefinitionAsObjectLink(objectDefinition, resourceDescription->ID, resourceDescription->Name, mandatory, resourceDescription->Operations, objLink);
            break;

        // For multiple resource instance types
        case AwaResourceType_StringArray:
        {
            const AwaStringArray * defaultStringArray = NULL;
            result = AwaObjectDefinition_AddResourceDefinitionAsStringArray(objectDefinition, resourceDescription->ID, resourceDescription->Name, resourceDescription->MinInstances, resourceDescription->MaxInstances, resourceDescription->Operations, defaultStringArray);
        }
        break;
        case AwaResourceType_IntegerArray:
        {
            const AwaIntegerArray * defaultIntArray = NULL;
            result = AwaObjectDefinition_AddResourceDefinitionAsIntegerArray(objectDefinition, resourceDescription->ID, resourceDescription->Name, resourceDescription->MinInstances, resourceDescription->MaxInstances, resourceDescription->Operations, defaultIntArray);
        }
        break;
        case AwaResourceType_FloatArray:
        {
            const AwaFloatArray * defaultFloatArray = NULL;
            result = AwaObjectDefinition_AddResourceDefinitionAsFloatArray(objectDefinition, resourceDescription->ID, resourceDescription->Name, resourceDescription->MinInstances, resourceDescription->MaxInstances, resourceDescription->Operations, defaultFloatArray);
        }
        break;
        case AwaResourceType_BooleanArray:
        {
            const AwaBooleanArray * defaultBoolArray = NULL;
            result = AwaObjectDefinition_AddResourceDefinitionAsBooleanArray(objectDefinition, resourceDescription->ID, resourceDescription->Name, resourceDescription->MinInstances, resourceDescription->MaxInstances, resourceDescription->Operations, defaultBoolArray);
        }
        break;
        case AwaResourceType_OpaqueArray:
        {
            const AwaOpaqueArray * defaultOpaqueArray = NULL;
            result = AwaObjectDefinition_AddResourceDefinitionAsOpaqueArray(objectDefinition, resourceDescription->ID, resourceDescription->Name, resourceDescription->MinInstances, resourceDescription->MaxInstances, resourceDescription->Operations, defaultOpaqueArray);
        }
        break;
        case AwaResourceType_TimeArray:
        {
            const AwaTimeArray * defaultTimeArray = NULL;
            result = AwaObjectDefinition_AddResourceDefinitionAsTimeArray(objectDefinition, resourceDescription->ID, resourceDescription->Name, resourceDescription->MinInstances, resourceDescription->MaxInstances, resourceDescription->Operations, defaultTimeArray);
        }
        break;
        case AwaResourceType_ObjectLinkArray:
        {
            const AwaObjectLinkArray * defaultObjLinkArray = NULL;
            result = AwaObjectDefinition_AddResourceDefinitionAsObjectLinkArray(objectDefinition, resourceDescription->ID, resourceDescription->Name, resourceDescription->MinInstances, resourceDescription->MaxInstances, resourceDescription->Operations,defaultObjLinkArray);
        }
        break;

        default:
            Error("Resource type %d not supported\n", resourceDescription->Type);
            result = AwaError_DefinitionInvalid;
    }

    return result;
}

AwaObjectDefinition * DefineObject(const ObjectDescription * objectDescription)
{
    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(objectDescription->ID,
                                                                       objectDescription->Name,
                                                                       objectDescription->MinInstances,
                                                                       objectDescription->MaxInstances);
    if (objectDefinition != NULL)
    {
        int i = 0;
        for (i = 0; i < objectDescription->NumResources; ++i)
        {
            if (AddResource(objectDefinition, objectDescription->ResourceDescription[i]) != AwaError_Success)
            {
                Error("Could not add resource definition (resource [%d] %s) to object definition\n", objectDescription->ResourceDescription[i]->ID, objectDescription->ResourceDescription[i]->Name);

                // Give up
                AwaObjectDefinition_Free(&objectDefinition);
                break;
            }
        }
    }
    else
    {
        Error("Could not create object %d definition\n", objectDescription->ID);
    }

    return objectDefinition;
}

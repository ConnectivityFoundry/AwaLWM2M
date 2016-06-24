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
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "lwm2m_types.h"
#include "lwm2m_limits.h"
#include "lwm2m_core.h"
#include "lwm2m_object_store.h"
#include "lwm2m_attributes.h"
#include "lwm2m_observers.h"
#include "lwm2m_list.h"
#include "lwm2m_debug.h"
#include "lwm2m_util.h"
#include "lwm2m_tree_builder.h"
#include "lwm2m_tree_node.h"
#include "lwm2m_result.h"
#include "lwm2m_request_origin.h"

static AwaResult ReadResourceInstanceFromStoreAndCreateTree(Lwm2mTreeNode ** dest, Lwm2mContextType * context, ObjectIDType objectID,
                                                            ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, ResourceInstanceIDType resourceInstanceID)
{
    AwaResult result = AwaResult_Unspecified;
    *dest = Lwm2mTreeNode_Create();
    const void * value = NULL;
    size_t valueLength = 0;

    Lwm2mTreeNode_SetID(*dest, resourceInstanceID);
    Lwm2mTreeNode_SetType(*dest, Lwm2mTreeNodeType_ResourceInstance);

    if (Lwm2mCore_GetResourceInstanceValue(context, objectID, objectInstanceID, resourceID, resourceInstanceID, &value, &valueLength) < 0)
    {
        Lwm2m_Error("ERROR: Failed to retrieve resource instance from object store\n");
        result = AwaResult_NotFound;
        goto error;
    }

    Lwm2m_Debug("Treebuilder length: %d\n", (int)valueLength);

    if (Lwm2mTreeNode_SetValue(*dest, (const uint8_t*)value, valueLength) != 0)
    {
        Lwm2m_Error("ERROR: Failed to set value for resource instance node\n");
        result = AwaResult_BadRequest;
        goto error;
    }
    result = AwaResult_Success;
error:
    return result;
}

AwaResult TreeBuilder_CreateTreeFromResource(Lwm2mTreeNode ** dest, Lwm2mContextType * context, Lwm2mRequestOrigin requestOrigin,
                                       ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    AwaResult result = AwaResult_Unspecified;
    *dest = Lwm2mTreeNode_Create();
    Lwm2mTreeNode_SetID(*dest, resourceID);
    Lwm2mTreeNode_SetType(*dest, Lwm2mTreeNodeType_Resource);
    ResourceDefinition * definition = Definition_LookupResourceDefinition(Lwm2mCore_GetDefinitions(context), objectID, resourceID);

    if (definition == NULL)
    {
        Lwm2m_Error("ERROR: Failed to determine resource definition Object %d Resource %d\n", objectID, resourceID);
        result = AwaResult_NotFound;
        goto error;
    }

    if (Lwm2mTreeNode_SetDefinition(*dest, definition) != 0)
    {
        Lwm2m_Error("ERROR: Failed to set definition Object %d Resource %d\n", objectID, resourceID);
        result = AwaResult_InternalError;
        goto error;
    }

    if (requestOrigin == Lwm2mRequestOrigin_Server && !Operations_IsResourceTypeReadable(definition->Operation))
    {
        Lwm2m_Error("ERROR: Request origin is server and resource operation is %d\n", definition->Operation);
        result = AwaResult_MethodNotAllowed;
        goto error;
    }

    if (IS_MULTIPLE_INSTANCE(definition))
    {
        int resourceInstanceID = -1;
        while ((resourceInstanceID = Lwm2mCore_GetNextResourceInstanceID(context, objectID, objectInstanceID, resourceID, resourceInstanceID)) != -1)
        {
            Lwm2mTreeNode * resourceValueNode;

            if ((result = ReadResourceInstanceFromStoreAndCreateTree(&resourceValueNode, context, objectID, objectInstanceID, resourceID, resourceInstanceID)) == AwaResult_Success)
            {
                Lwm2mTreeNode_AddChild(*dest, resourceValueNode);
            }
            else
            {
                Lwm2m_Error("ERROR: Failed to create resource instance node: /%d/%d/%d/%d\n", objectID, objectInstanceID, resourceID, resourceInstanceID);
                Lwm2mTreeNode_DeleteRecursive(resourceValueNode);
                goto error;
            }
        }
    }
    else
    {
        Lwm2mTreeNode * resourceValueNode;
        int resourceInstanceID = 0;
        if ((result = ReadResourceInstanceFromStoreAndCreateTree(&resourceValueNode, context, objectID, objectInstanceID, resourceID, resourceInstanceID)) == AwaResult_Success)
        {
            Lwm2mTreeNode_AddChild(*dest, resourceValueNode);
        }
        else
        {
            Lwm2m_Error("ERROR: Failed to create resource instance node: /%d/%d/%d/%d\n", objectID, objectInstanceID, resourceID, resourceInstanceID);
            Lwm2mTreeNode_DeleteRecursive(resourceValueNode);
            goto error;
        }
    }
    result = AwaResult_Success;
error:
    return result;
}

int TreeBuilder_CreateTreeFromObjectInstance(Lwm2mTreeNode ** dest, Lwm2mContextType * context, Lwm2mRequestOrigin requestOrigin,
                                             ObjectIDType objectID, ObjectInstanceIDType objectInstanceID)
{
    AwaResult result = AwaResult_Success;
    *dest = Lwm2mTreeNode_Create();
    Lwm2mTreeNode_SetID(*dest, objectInstanceID);
    Lwm2mTreeNode_SetType(*dest, Lwm2mTreeNodeType_ObjectInstance);

    int resourceID = -1;

    while ((resourceID = Lwm2mCore_GetNextResourceID(context, objectID, objectInstanceID, resourceID)) != -1)
    {
        if (Definition_IsResourceTypeExecutable(Lwm2mCore_GetDefinitions(context), objectID, resourceID) == 0)
        {
            Lwm2mTreeNode * resourceNode;

            if ((result = TreeBuilder_CreateTreeFromResource(&resourceNode, context, requestOrigin, objectID, objectInstanceID, resourceID)) == AwaResult_Success)
            {
                Lwm2mTreeNode_AddChild(*dest, resourceNode);
            }
            else
            {
                Lwm2m_Error("ERROR: Failed to create resource node: /%d/%d/%d\n", objectID, objectInstanceID, resourceID);
                Lwm2mTreeNode_DeleteRecursive(resourceNode);
                break;
            }
        }
    }

    if ((result == AwaResult_Success) && (!Lwm2mTreeNode_HasChildren(*dest)))
    {
        result = AwaResult_NotFound;
    }

    return result;
}

int TreeBuilder_CreateTreeFromObject(Lwm2mTreeNode ** dest, Lwm2mContextType * context, Lwm2mRequestOrigin requestOrigin, ObjectIDType objectID)
{
    AwaResult result = AwaResult_Success;
    *dest = Lwm2mTreeNode_Create();
    Lwm2mTreeNode_SetID(*dest, objectID);
    Lwm2mTreeNode_SetType(*dest, Lwm2mTreeNodeType_Object);

    ObjectDefinition * definition = Definition_LookupObjectDefinition(Lwm2mCore_GetDefinitions(context), objectID);

    if (definition == NULL)
    {
       Lwm2m_Error("ERROR: Failed to determine object definition Object %d\n", objectID);
       result = AwaResult_NotFound;
       goto error;
    }

    if (Lwm2mTreeNode_SetDefinition(*dest, definition) != 0)
    {
        Lwm2m_Error("ERROR: Failed to set definition Object %d\n", objectID);
        result = AwaResult_InternalError;
        goto error;
    }

    int instanceID = -1;
    while ((instanceID = Lwm2mCore_GetNextObjectInstanceID(context, objectID, instanceID)) != -1)
    {
        Lwm2mTreeNode * objectInstanceNode;
        if ((result = TreeBuilder_CreateTreeFromObjectInstance(&objectInstanceNode, context, requestOrigin, objectID, instanceID)) == AwaResult_Success)
        {
            Lwm2mTreeNode_AddChild(*dest, objectInstanceNode);
        }
        else
        {
            Lwm2m_Error("ERROR: Failed to create object instance node: /%d/%d\n", objectID, instanceID);
            Lwm2mTreeNode_DeleteRecursive(objectInstanceNode);
            break;
        }
    }

    if ((result == AwaResult_Success) && (!Lwm2mTreeNode_HasChildren(*dest)))
    {
        result = AwaResult_NotFound;
    }

error:
    return result;
}

AwaResult TreeBuilder_CreateTreeFromOIR(Lwm2mTreeNode ** dest, Lwm2mContextType * context, Lwm2mRequestOrigin requestOrigin, int OIR[], int OIRLength)
{
    AwaResult result = AwaResult_Unspecified;
    if (dest != NULL)
    {
        if (OIRLength == 1)
        {
            result = TreeBuilder_CreateTreeFromObject(dest, context, requestOrigin, OIR[0]);
        }
        else if (OIRLength == 2)
        {
            result = TreeBuilder_CreateTreeFromObjectInstance(dest, context, requestOrigin, OIR[0], OIR[1]);
        }
        else if (OIRLength == 3)
        {
            result = TreeBuilder_CreateTreeFromResource(dest, context, requestOrigin, OIR[0], OIR[1], OIR[2]);
        }
        else
        {
            Lwm2m_Error("Invalid OIR, length %d\n", OIRLength);
            result = AwaResult_BadRequest;
            *dest = NULL;
        }
    }
    else
    {
        Lwm2m_Error("dest is NULL\n");
        result = AwaResult_BadRequest;
    }

    return result;
}

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
#include "lwm2m_opaque.h"
#include "lwm2m_debug.h"

static int OpaqueSerialiseResource(SerdesContext * serdesContext, Lwm2mTreeNode * node, ObjectIDType objectID,
                                   ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, uint8_t * buffer, int len)
{
    uint16_t resourceLength = 0;

    if (Lwm2mTreeNode_GetType(node) != Lwm2mTreeNodeType_Resource)
    {
       Lwm2m_Error("ERROR: Resource node type expected. Received %d\n", Lwm2mTreeNode_GetType(node));
       return -1;
    }

    ResourceDefinition * definition = (ResourceDefinition *)Lwm2mTreeNode_GetDefinition(node);

    if (definition->Type != AwaResourceType_Opaque)
    {
        Lwm2m_Error("ERROR: opaque data format requested, but the requested resource is not of opaque type\n");
        return -1;
    }

    if (IS_MULTIPLE_INSTANCE(definition))
    {
        Lwm2m_Error("ERROR: opaque only works on singular resources!!\n");
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

    uint8_t * value = (uint8_t * )Lwm2mTreeNode_GetValue(child, &resourceLength);

    if (len < resourceLength)
    {
        Lwm2m_Error("ERROR: Buffer is not large enough to serialise resource\n");
        return -1;
    }

    memcpy(buffer, (char*)value, resourceLength);

    return resourceLength;
}

static int OpaqueDeserialiseResource(SerdesContext * serdesContext, Lwm2mTreeNode ** dest, const DefinitionRegistry * registry, ObjectIDType objectID,
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

    if (Definition_GetResourceType(registry, objectID, resourceID) != AwaResourceType_Opaque)
    {
        Lwm2m_Error("ERROR: opaque data format requested, but the requested resource is not of opaque type\n");
        return -1;
    }

    Lwm2mTreeNode * resourceValueNode = Lwm2mTreeNode_Create();
    Lwm2mTreeNode_SetID(resourceValueNode, 0);
    Lwm2mTreeNode_SetType(resourceValueNode, Lwm2mTreeNodeType_ResourceInstance);

    result = Lwm2mTreeNode_SetValue(resourceValueNode, (const uint8_t*)&buffer[0], bufferLen);

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

// Map Opaque serdes function delegates
const SerialiserDeserialiser opaqueSerDes =
{
    .SerialiseObject   = NULL,
    .SerialiseObjectInstance = NULL,
    .SerialiseResource = OpaqueSerialiseResource,
    .DeserialiseObject = NULL,
    .DeserialiseObjectInstance = NULL,
    .DeserialiseResource = OpaqueDeserialiseResource,
};


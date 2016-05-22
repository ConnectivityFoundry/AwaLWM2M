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


#include "lwm2m_result.h"
#include "lwm2m_xml_serdes.h"

#include "response_common.h"
#include "error.h"
#include "path.h"
#include "xml.h"
#include "log.h"
#include "memalloc.h"
#include "arrays.h"
#include "map.h"
#include "objects_tree.h"
#include "path_result.h"
#include "path_iterator.h"
#include "value.h"
#include "utils.h"

struct _ResponseCommon
{
    const OperationCommon * OperationCommon;   // pointer to parent operation
    TreeNode ObjectsNode;

    MapType * Values;
    MapType * PathResults;
    MapType * NulledValues;
};

ResponseCommon * ResponseCommon_New(const OperationCommon * operation, TreeNode objectsNode)
{
    ResponseCommon * response = NULL;

    if (operation != NULL)
    {
        if (objectsNode != NULL)
        {
            response = Awa_MemAlloc(sizeof(*response));
            if (response != NULL)
            {
                memset(response, 0, sizeof(*response));
                response->OperationCommon = operation;
                response->ObjectsNode = Tree_Copy(objectsNode);

                // build the Values data structure
                if (ResponseCommon_BuildValues(response) == AwaError_Success)
                {
                    // not all responses have Values, so success includes no values
                }
                else
                {
                    // there was an error building values
                    LogDebug("Failed to build response values - continuing");
                }

                if (ResponseCommon_BuildPathResults(response) == AwaError_Success)
                {
                    // not all responses have path results
                }
                else
                {
                    // there was an error building values
                    LogErrorWithEnum(AwaError_ResponseInvalid, "Failed to build path results - continuing");
                }

                response->NulledValues = Map_New();

                LogNew("ResponseCommon", response);

            }
            else
            {
                LogErrorWithEnum(AwaError_OutOfMemory);
            }
        }
        else
        {
            LogError("Content Response is NULL");
        }
    }
    else
    {
        LogError("Operation is NULL");
    }
    return response;
}

// FIXME Currently The client is using the old xml spec and therefore can't be used with the new objects tree.
// This is a temporary workaround to prevent error messages when the response does not contain an 'Objects' node
ResponseCommon * ResponseCommon_NewClient(const OperationCommon * operation, TreeNode objectsNode)
{
    ResponseCommon * response = NULL;

    if (operation != NULL)
    {
        if (objectsNode != NULL)
        {
            response = Awa_MemAlloc(sizeof(*response));
            if (response != NULL)
            {
                memset(response, 0, sizeof(*response));
                response->OperationCommon = operation;
                response->ObjectsNode = Tree_Copy(objectsNode);
                LogNew("ResponseCommon", response);
            }
            else
            {
                LogErrorWithEnum(AwaError_OutOfMemory);
            }
        }
        else
        {
            LogError("Content Response is NULL");
        }
    }
    else
    {
        LogError("Operation is NULL");
    }
    return response;
}

static void FreeSimpleMap(MapType ** map)
{
    if ((map != NULL) && (*map != NULL))
    {
        Map_FreeValues(*map);
        Map_Free(map);
    }
}

static void FreeValue(const char * key, void * value, void * context)
{
    Value_Free((Value **)(&value));
}

static void FreeValues(MapType ** valuesMap)
{
    if ((valuesMap != NULL) && (*valuesMap != NULL))
    {
        Map_ForEach(*valuesMap, FreeValue, NULL);
        Map_Free(valuesMap);
    }
}

AwaError ResponseCommon_Free(ResponseCommon ** response)
{
    AwaError result = AwaError_Unspecified;
    if ((response != NULL) && (*response != NULL))
    {
        LogFree("ResponseCommon", *response);
        FreeSimpleMap(&((*response)->PathResults));
        FreeSimpleMap(&((*response)->NulledValues));
        FreeValues(&((*response)->Values));
        Tree_Delete((*response)->ObjectsNode);
        Awa_MemSafeFree(*response);
        *response = NULL;
        result = AwaError_Success;
    }
    else
    {
        result = AwaError_OperationInvalid;
    }
    return result;
}

const OperationCommon * ResponseCommon_GetOperation(const ResponseCommon * response)
{
    const OperationCommon * operation = NULL;
    if (response != NULL)
    {
        operation = response->OperationCommon;
    }
    return operation;
}

TreeNode ResponseCommon_GetObjectsNode(const ResponseCommon * response)
{
    TreeNode objectsNode = NULL;
    if (response != NULL)
    {
        objectsNode = response->ObjectsNode;
    }
    return objectsNode;
}

AwaError ResponseCommon_CheckForErrors(const ResponseCommon * response)
{
    AwaError result = AwaError_Success;
    if (response != NULL)
    {
        if (response->ObjectsNode != NULL)
        {
            TreeNode currentNode = response->ObjectsNode;
            if (currentNode != NULL)
            {
                while ((currentNode = ObjectsTree_GetNextLeafNode(currentNode)) != NULL)
                {
                    // check for a Result tag
                    TreeNode resultNode = Xml_Find(currentNode, "Result");
                    if (resultNode != NULL)
                    {
                        TreeNode errorNode = Xml_Find(resultNode, "Error");
                        if (errorNode != NULL)
                        {
                            AwaError error = Error_FromString((const char *)TreeNode_GetValue(errorNode));
                            if (error != AwaError_Success)
                            {
                                result = AwaError_Response;
                            }
                            // keep looking; examine all leaf nodes
                        }
                        else
                        {
                            // result without error node - error
                            result = LogErrorWithEnum(AwaError_ResponseInvalid, "Response Result has missing Error node");
                            break;
                        }
                    }
                    else
                    {
                        // leaf without result node - error
                        result = LogErrorWithEnum(AwaError_ResponseInvalid, "Response has missing Result node");
                        break;
                    }
                }
            }
            else
            {
                result = LogErrorWithEnum(AwaError_Internal, "Response has no objects tree");
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_Internal, "Response has no objects node");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_ResponseInvalid, "response is NULL");
    }
    return result;
}

AwaError ResponseCommon_BuildValues(ResponseCommon * response)
{
    AwaError result = AwaError_Success;  // success if no values are found
    if (response != NULL)
    {
        FreeValues(&response->Values);
        response->Values = Map_New();
        if (response->Values != NULL)
        {
            // get the resource type for the path
            const OperationCommon * operation = ResponseCommon_GetOperation(response);
            if (operation != NULL)
            {
                const SessionCommon * sessionCommon = OperationCommon_GetSessionCommon(operation);
                if (sessionCommon != NULL)
                {
                    TreeNode currentNode = response->ObjectsNode;
                    while ((currentNode = ObjectsTree_GetNextLeafNode(currentNode)) != NULL)
                    {
                        char path[MAX_PATH_LENGTH] = { 0 };
                        ObjectsTree_GetPath(currentNode, path, sizeof(path));

                        if (Path_IsValidForResource(path))
                        {
                            LogDebug("Build values path: %s", path);

                            const AwaResourceDefinition * resourceDefinition = SessionCommon_GetResourceDefinitionFromPath(sessionCommon, path);
                            if (resourceDefinition != NULL)
                            {
                                AwaResourceType resourceType = AwaResourceDefinition_GetType(resourceDefinition);
                                if (resourceType != AwaResourceType_Invalid)
                                {
                                    Value * value = Value_New(currentNode, resourceType);

                                    if (value != NULL)
                                    {
                                        if (!Map_Contains(response->Values, path))
                                        {
                                            LogDebug("MAP item path: %s", path);
                                            Map_Put(response->Values, path, value);
                                            result = AwaError_Success;
                                        }
                                        else
                                        {
                                            result = LogErrorWithEnum(AwaError_Internal, "A value already exists for %s", path);
                                        }
                                    }
                                    else
                                    {
                                        // No value is fine - occurs in operations where we only expect path results in the response.
                                    }

                                }
                                else
                                {
                                    result = LogErrorWithEnum(AwaError_DefinitionInvalid, "resourceDefinition for %s has invalid type", path);
                                }
                            }
                            else
                            {
                                result = LogErrorWithEnum(AwaError_DefinitionInvalid, "resourceDefinition for %s is NULL", path);
                            }
                        }
                    }
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_SessionInvalid, "session is NULL");
                }
            }
            else
            {
                result = LogErrorWithEnum(AwaError_OperationInvalid, "operation is NULL");
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_OutOfMemory);
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_ResponseInvalid, "response is NULL");
    }
    return result;
}

AwaError ResponseCommon_BuildPathResults(ResponseCommon * response)
{
    AwaError result = AwaError_Success;  // success if no path results are found
    if (response != NULL)
    {
        FreeSimpleMap(&response->PathResults);
        response->PathResults = Map_New();
        if (response->PathResults != NULL)
        {
            TreeNode currentLeafNode = response->ObjectsNode;
            while ((currentLeafNode = ObjectsTree_GetNextLeafNode(currentLeafNode)) != NULL)
            {
                TreeNode currentNode = currentLeafNode;  // start at leaf and add path results for parents as well
                while (currentNode != NULL)
                {
                    char path[MAX_PATH_LENGTH] = { 0 };
                    ObjectsTree_GetPath(currentNode, path, sizeof(path));

                    TreeNode resultNode = Xml_Find(currentNode, "Result");
                    if (resultNode != NULL)
                    {
                        PathResult * pathResult = PathResult_New(resultNode);
                        if (pathResult != NULL)
                        {
                            if (!Map_Contains(response->PathResults, path))
                            {
                                Map_Put(response->PathResults, path, pathResult);
                                result = AwaError_Success;
                            }
                            else if (currentNode == currentLeafNode)
                            {
                                PathResult_Free(&pathResult);
                                result = LogErrorWithEnum(AwaError_Internal, "A pathResult already exists for %s\n", path);
                                goto error;
                            }
                            else
                            {
                                // Already added parent node
                                PathResult_Free(&pathResult);
                            }
                        }
                        else
                        {
                            result = LogErrorWithEnum(AwaError_Internal, "Could not create pathResult for %s", path);
                            goto error;
                        }
                    }
                    else
                    {
                        // not all leaves or responses have PathResults, so skip
                    }
                    currentNode = ObjectsTree_IsObjectNode(currentNode)? NULL : TreeNode_GetParent(currentNode);
                }
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_OutOfMemory);
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_ResponseInvalid, "response is NULL");
    }
error:
    return result;
}

bool ResponseCommon_HasValue(const ResponseCommon * response, const char * path)
{
    bool hasValue = false;
    if ((response != NULL) && (path != NULL))
    {
        hasValue = Map_Contains(response->Values, path);
    }
    return hasValue;
}
bool ResponseCommon_ContainsPath(const ResponseCommon * response, const char * path)
{
    bool containsPath = false;
    if ((response != NULL) && (path != NULL))
    {
        containsPath = ObjectsTree_ContainsPath(response->ObjectsNode, path);
    }
    return containsPath;
}

AwaError ResponseCommon_GetPathResult(const ResponseCommon * response, const char * path, const PathResult ** result)
{
    AwaError error = AwaError_Unspecified;

    if (response != NULL)
    {
        if (path != NULL)
        {
            if (result)
            {
                if (Map_Get(response->PathResults, path, (void**)result))
                {
                    error = AwaError_Success;
                }
                else
                {
                    *result = NULL;
                    error = LogErrorWithEnum(AwaError_PathNotFound, "path %s not found", path);
                }
            }
            else
            {
                error = LogErrorWithEnum(AwaError_Internal, "Result is null");
            }
        }
        else
        {
            error = LogErrorWithEnum(AwaError_PathInvalid, "Path is null");
        }
    }
    else
    {
        error = LogErrorWithEnum(AwaError_ResponseInvalid, "Response is null");
    }
    return error;
}

PathIterator * ResponseCommon_NewPathIterator(const ResponseCommon * response)
{
    // keys from PathResults map, if map had a key iterator, are not suitable
    // as not every response has a PathResult for every leaf node.
    PathIterator * iterator = NULL;
    if (response != NULL)
    {
        if (response->ObjectsNode != NULL)
        {
            iterator = PathIterator_New();
            if (iterator != NULL)
            {
                TreeNode currentNode = response->ObjectsNode;
                if (currentNode != NULL)
                {
                    while ((currentNode = ObjectsTree_GetNextLeafNode(currentNode)) != NULL)
                    {
                        char path[MAX_PATH_LENGTH];
                        ObjectsTree_GetPath(currentNode, path, sizeof(path));

                        PathIterator_Add(iterator, path);
                    }
                }
                else
                {
                    LogError("Response has no objects tree");
                    goto error;
                }
            }
            else
            {
                LogErrorWithEnum(AwaError_OutOfMemory);
            }
        }
        else
        {
            LogError("Response has no response objects");
        }
    }
    else
    {
        LogError("response is NULL");
    }
    return iterator;
error:
    LogError("Bad Response");
    PathIterator_Free(&iterator);
    return NULL;
}

static const Value * ResponseCommon_GetValue(const ResponseCommon * response, const char * path)
{
    const Value * storedValue = NULL;
    if (Map_Get(response->Values, path, (void **)&storedValue))
    {
        // success
    }
    else
    {
        LogErrorWithEnum(AwaError_PathNotFound, "path %s not found", path);
    }
    return storedValue;
}

AwaError GetValuePointer(const ResponseCommon * response, const char * path, const void ** value, size_t * valueSize, AwaResourceType resourceType, int resourceSize, bool withNull)
{
    AwaError result = AwaError_Unspecified;
    if (path != NULL)
    {
        if (Path_IsValidForResource(path))
        {
            if (response != NULL)
            {
                if (value != NULL)
                {
                    const Value * storedValue = ResponseCommon_GetValue(response, path);
                    if (storedValue != NULL)
                    {
                        AwaResourceType type = Value_GetType(storedValue);
                        if (type == resourceType || (type == AwaResourceType_None && resourceType == AwaResourceType_Opaque /* Execute arguments payload */))
                        {
                            const void * data = Value_GetData(storedValue);
                            size_t length = Value_GetLength(storedValue);
                            // -1 used for e.g. string values as we can't know how long they are without reading them first
                            if (resourceSize != -1 && length != resourceSize)
                            {
                                result = LogErrorWithEnum(AwaError_Internal, "Unexpected length for %s value: %zu expects %d", Utils_ResourceTypeToString(resourceType), length, resourceSize);
                            }
                            else
                            {
                                if (withNull)
                                {
                                    char * nulledValue = (char *)malloc(length+1);

                                    if ((nulledValue != NULL))
                                    {
                                        memcpy(nulledValue, data, length);
                                        nulledValue[length] = '\0';
                                        if (valueSize != NULL)
                                        {
                                            *valueSize = length + 1;
                                        }

                                        Map_Put(response->NulledValues, path, nulledValue);

                                        *value = nulledValue;
                                        result = AwaError_Success;
                                    }
                                    else
                                    {
                                        result = LogErrorWithEnum(AwaError_OutOfMemory);
                                    }
                                }
                                else
                                {
                                    *value = (length > 0) ? data : NULL;
                                    if (valueSize != NULL)
                                    {
                                        *valueSize = length;
                                    }

                                    result = AwaError_Success;
                                }
                            }
                        }
                        else
                        {
                            result = LogErrorWithEnum(AwaError_TypeMismatch, "Resource %s is not of type %s", path, Utils_ResourceTypeToString(resourceType));
                        }
                    }
                    else
                    {
                        // no value stored for this path
                        result = LogErrorWithEnum(AwaError_PathNotFound, "No value for path %s", path);
                    }
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_OperationInvalid, "Value is null");
                }
            }
            else
            {
                result = LogErrorWithEnum(AwaError_OperationInvalid, "Invalid Get Response");
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_PathInvalid, "%s is not a resource path", path);
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_PathInvalid, "No path specified");
    }
    return result;
}


AwaError ResponseCommon_GetValuePointer(const ResponseCommon * response, const char * path, const void ** value, size_t * valueSize, AwaResourceType resourceType, int resourceSize)
{
    return GetValuePointer(response, path, value, valueSize, resourceType, resourceSize, false);
}


AwaError ResponseCommon_GetValuePointerWithNull(const ResponseCommon * response, const char * path, const void ** value, size_t * valueSize, AwaResourceType resourceType, int resourceSize)
{
    return GetValuePointer(response, path, value, valueSize, resourceType, resourceSize, true);
}

AwaError ResponseCommon_GetValueAsObjectLink(const ResponseCommon * response, const char * path, AwaObjectLink * value)
{
    AwaObjectLink * storedObjectLink;

    AwaError result = ResponseCommon_GetValuePointer(response, path, (const void **)&storedObjectLink, NULL, AwaResourceType_ObjectLink, sizeof(*value));

    if ((result == AwaError_Success) && (value != NULL))
    {
        value->ObjectID = storedObjectLink->ObjectID;
        value->ObjectInstanceID = storedObjectLink->ObjectInstanceID;
    }
    return result;
}

AwaError ResponseCommon_GetValueAsOpaque(const ResponseCommon * response, const char * path, AwaOpaque * value)
{
    AwaOpaque * storedOpaque;

    AwaError result = ResponseCommon_GetValuePointer(response, path, (const void **)&storedOpaque, NULL, AwaResourceType_Opaque, sizeof(*value));

    if ((result == AwaError_Success) && (value != NULL))
    {
        value->Data = storedOpaque->Data;
        value->Size = storedOpaque->Size;
    }
    return result;
}

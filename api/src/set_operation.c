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
#include <stdlib.h>
#include <inttypes.h>

#include "awa/client.h"
#include "client_session.h"
#include "memalloc.h"
#include "log.h"
#include "xml.h"
#include "path.h"
#include "operation_common.h"
#include "lwm2m_xml_interface.h"
#include "utils.h"
#include "arrays.h"
#include "objects_tree.h"
#include "set_write_common.h"
#include "response_common.h"

struct _AwaClientSetOperation
{
    OperationCommon * Common;
    ResponseCommon * Response;
};

AwaClientSetOperation * AwaClientSetOperation_New(const AwaClientSession * session)
{
    AwaClientSetOperation * operation = NULL;

    if (session != NULL)
    {
        if (ClientSession_IsConnected(session) != false)
        {
            operation = Awa_MemAlloc(sizeof(*operation));
            if (operation != NULL)
            {
                memset(operation, 0, sizeof(*operation));
                operation->Common = OperationCommon_NewWithClientSession(session);
                if (operation->Common != NULL)
                {
                    LogNew("AwaClientSetOperation", operation);
                }
                else
                {
                    LogErrorWithEnum(AwaError_Internal, "Unable to initialise operation");
                    Awa_MemSafeFree(operation);
                    operation = NULL;
                }
            }
            else
            {
                LogErrorWithEnum(AwaError_OutOfMemory);
            }
        }
        else
        {
            LogError("Session is not connected");
        }
    }
    else
    {
        LogError("Session is NULL");
    }
    return operation;
}

AwaError AwaClientSetOperation_Free(AwaClientSetOperation ** operation)
{
    AwaError result = AwaError_OperationInvalid;
    if ((operation != NULL) && (*operation != NULL))
    {
        OperationCommon_Free(&(*operation)->Common);

        if ((*operation)->Response != NULL)
        {
            ResponseCommon_Free(&(*operation)->Response);
        }

        LogFree("AwaClientSetOperation", *operation);
        Awa_MemSafeFree(*operation);
        *operation = NULL;
        result = AwaError_Success;
    }
    return result;
}

// Given an object, object instance or resource node, add a create tag. In the case only an object node is given,
// an object instance node without an ID will be created, marking that the new instance should have a generated ID.
InternalError ClientSetOperation_AddCreate(TreeNode node)
{
    InternalError result = InternalError_Unspecified;

    if ((node != NULL) && (ObjectsTree_IsObjectNode(node) || ObjectsTree_IsObjectInstanceNode(node) || ObjectsTree_IsResourceNode(node)))
    {
        if (ObjectsTree_IsObjectNode(node))
        {
            node = ObjectsTreeInternal_AddChildNode(node, "ObjectInstance");
        }

        if (TreeNode_AddChild(node, Xml_CreateNode(IPC_MESSAGE_TAG_CREATE)))
        {
            result = InternalError_Success;
        }
        else
        {
            LogError("Failed to add create tag to object instance node");
            result = InternalError_Tree;
        }
    }
    else
    {
        LogError("node is NULL");
        result = InternalError_ParameterInvalid;
    }

    return result;
}


AwaError AwaClientSetOperation_CreateObjectInstance(AwaClientSetOperation * operation, const char * path)
{
    AwaError result = AwaError_Unspecified;

    if (path != NULL)
    {
        AwaObjectID objectID;
        AwaResourceID resourceID;

        if (Path_IsValid(path))
        {
            objectID = Path_GetObjectID(path);
            resourceID = Path_GetResourceID(path);

            if ((objectID != AWA_INVALID_ID) && (resourceID == AWA_INVALID_ID))
            {
                if (operation != NULL)
                {
                    TreeNode resultNode;
                    if ((result = OperationCommon_AddPathV2(operation->Common, path, &resultNode)) == AwaError_Success && resultNode != NULL)
                    {
                        //NB: if object instance ID isn't specified in the path we have to check the response for the generated ID
                        if (ClientSetOperation_AddCreate(resultNode) == InternalError_Success)
                        {
                            result = AwaError_Success;
                        }
                        else
                        {
                            result = LogErrorWithEnum(AwaError_Internal, "Failed to add value to path");
                        }
                    }
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_OperationInvalid, "Operation is NULL");
                }
            }
            else
            {
                result = LogErrorWithEnum(AwaError_PathInvalid,  "%s is not a valid object or object instance path", path);
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_PathInvalid,  "%s is not a valid path", path);
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_PathInvalid, "Path is NULL");
    }
    return result;
}

/**
 * @brief Adds a path of interest to a Set operation, as a request to create an optional Resource.
 *        If the path refers to an optional resource that does not exist, the operation will result in the creation of the resource.
 *        The resource will have the default value assigned, if not overridden in the same operation.
 * @param[in] operation The current Set operation to add the create request to.
 * @param[in] path The path of the resource requested for creation.
 * @return Error_Success on success, error code on failure.
 */
AwaError AwaClientSetOperation_CreateOptionalResource(AwaClientSetOperation * operation, const char * path)
{
    AwaError result = AwaError_Unspecified;

    if (operation != NULL)
    {
        TreeNode objectsTree = OperationCommon_GetObjectsTree(operation->Common);
        if (objectsTree != NULL)
        {
            if (path != NULL)
            {
                if (Path_IsValidForResource(path))
                {
                    TreeNode resultNode;
                    if (ObjectsTree_AddPath(objectsTree, path, &resultNode) == InternalError_Success && resultNode != NULL)
                    {
                        if (ClientSetOperation_AddCreate(resultNode) == InternalError_Success)
                        {
                            result = AwaError_Success;
                        }
                        else
                        {
                            result = LogErrorWithEnum(AwaError_Internal, "Failed to add value to path");
                        }
                    }
                    else
                    {
                        result = LogErrorWithEnum(AwaError_Internal, "AddPath failed");
                    }
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_PathInvalid, "%s is not a resource path", path);
                }
            }
            else
            {
                result = LogErrorWithEnum(AwaError_PathInvalid, "Path is NULL");
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_Internal, "ObjectsTree is NULL");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "Operation is NULL");
    }
    return result;
}

static AwaError ClientSetOperation_AddValue(AwaClientSetOperation * operation, const char * path, int resourceInstanceID, void * value, size_t size, AwaResourceType type, SetArrayMode setArrayMode)
{
    AwaError result = AwaError_Unspecified;
    if (operation != NULL)
    {
        result = SetWriteCommon_AddValue(operation->Common, SessionType_Client, path, resourceInstanceID, value, size, type, setArrayMode);
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "operation is NULL");
    }
    return result;
}

static AwaError ClientSetOperation_AddValues(AwaClientSetOperation * operation, const char * path, const AwaArray * array, AwaResourceType type)
{
    AwaError result = AwaError_Unspecified;

    AwaArrayIterator * iterator = ArrayIterator_New(array);

    if (iterator)
    {
        while(ArrayIterator_Next(iterator))
        {
            AwaArrayIndex index = ArrayIterator_GetIndex(iterator);
            void * value = ArrayIterator_GetValue(iterator);
            size_t length = ArrayIterator_GetValueLength(iterator);

            if (type == AwaResourceType_OpaqueArray)
            {
                AwaOpaque * opaque = value;
                value = opaque->Data;
                length = opaque->Size;
            }

            LogDebug("Adding value %p", value);

            if (value)
            {
                result = ClientSetOperation_AddValue(operation, path, index, value, length, type, SetArrayMode_Replace);
                if (result != AwaError_Success)
                {
                    LogError("Unable to add array resource Instance");
                    break;
                }
            }
            else
            {
                result = LogErrorWithEnum(AwaError_Internal , "Problem with array");
                break;
            }
        }

        ArrayIterator_Free(&iterator);
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "Operation is NULL");
    }

    return result;
}

AwaError AwaClientSetOperation_AddValueAsCString(AwaClientSetOperation * operation, const char * path, const char * value)
{
    AwaError result = AwaError_Unspecified;
    if (value != NULL)
    {
        result = ClientSetOperation_AddValue(operation, path, 0, (void *)value, strlen(value), AwaResourceType_String, SetArrayMode_Unspecified);
    }
    else
    {
        result = LogErrorWithEnum(AwaError_TypeMismatch, "value is NULL");
    }
    return result;
}

AwaError AwaClientSetOperation_AddValueAsInteger(AwaClientSetOperation * operation, const char * path, AwaInteger value)
{
    return ClientSetOperation_AddValue(operation, path, 0, (void *)&value, sizeof(AwaInteger), AwaResourceType_Integer, SetArrayMode_Unspecified);
}

AwaError AwaClientSetOperation_AddValueAsFloat(AwaClientSetOperation * operation, const char * path, AwaFloat value)
{
    return ClientSetOperation_AddValue(operation, path, 0, (void *)&value, sizeof(AwaFloat), AwaResourceType_Float, SetArrayMode_Unspecified);
}

AwaError AwaClientSetOperation_AddValueAsBoolean(AwaClientSetOperation * operation, const char * path, AwaBoolean value)
{
    return ClientSetOperation_AddValue(operation, path, 0, (void *)&value, sizeof(AwaBoolean), AwaResourceType_Boolean, SetArrayMode_Unspecified);
}

AwaError AwaClientSetOperation_AddValueAsTime(AwaClientSetOperation * operation, const char * path, AwaTime value)
{   
    return ClientSetOperation_AddValue(operation, path, 0, (void *)&value, sizeof(AwaTime), AwaResourceType_Time, SetArrayMode_Unspecified);
} 

AwaError AwaClientSetOperation_AddValueAsOpaque(AwaClientSetOperation * operation, const char * path, AwaOpaque value)
{   
    return ClientSetOperation_AddValue(operation, path, 0, value.Data, value.Size, AwaResourceType_Opaque, SetArrayMode_Unspecified);
} 

AwaError AwaClientSetOperation_AddValueAsObjectLink(AwaClientSetOperation * operation, const char * path, AwaObjectLink value)
{   
    return ClientSetOperation_AddValue(operation, path, 0, (void *)&value, sizeof(AwaObjectLink), AwaResourceType_ObjectLink, SetArrayMode_Unspecified);
}

AwaError AwaClientSetOperation_AddValueAsIntegerArray(AwaClientSetOperation * operation, const char * path, const AwaIntegerArray * array)
{
    return ClientSetOperation_AddValues(operation, path, (const AwaArray *)array, AwaResourceType_IntegerArray);
}

AwaError AwaClientSetOperation_AddValueAsFloatArray(AwaClientSetOperation * operation, const char * path, const AwaFloatArray * array)
{
    return ClientSetOperation_AddValues(operation, path, (const AwaArray *)array, AwaResourceType_FloatArray);
}

AwaError AwaClientSetOperation_AddValueAsBooleanArray(AwaClientSetOperation * operation, const char * path, const AwaBooleanArray * array)
{
    return ClientSetOperation_AddValues(operation, path, (const AwaArray *)array, AwaResourceType_BooleanArray);
}

AwaError AwaClientSetOperation_AddValueAsTimeArray(AwaClientSetOperation * operation, const char * path, const AwaTimeArray * array)
{
    return ClientSetOperation_AddValues(operation, path, (const AwaArray *)array, AwaResourceType_TimeArray);
}

AwaError AwaClientSetOperation_AddValueAsStringArray(AwaClientSetOperation * operation, const char * path, const AwaStringArray * array)
{
    return ClientSetOperation_AddValues(operation, path, (const AwaArray *)array, AwaResourceType_StringArray);
}

AwaError AwaClientSetOperation_AddValueAsOpaqueArray(AwaClientSetOperation * operation, const char * path, const AwaOpaqueArray * array)
{
    return ClientSetOperation_AddValues(operation, path, (const AwaArray *)array, AwaResourceType_OpaqueArray);
}

AwaError AwaClientSetOperation_AddValueAsObjectLinkArray(AwaClientSetOperation * operation, const char * path, const AwaObjectLinkArray * array)
{
    return ClientSetOperation_AddValues(operation, path, (const AwaArray *)array, AwaResourceType_ObjectLinkArray);
}

AwaError AwaClientSetOperation_AddArrayValueAsCString(AwaClientSetOperation * operation, const char * path, int resourceInstanceID, const char * value)
{
    return ClientSetOperation_AddValue(operation, path, resourceInstanceID, (void *)value, strlen(value), AwaResourceType_StringArray, SetArrayMode_Update);
}

AwaError AwaClientSetOperation_AddArrayValueAsInteger(AwaClientSetOperation * operation, const char * path, int resourceInstanceID, AwaInteger value)
{
    return ClientSetOperation_AddValue(operation, path, resourceInstanceID, (void *)&value, sizeof(AwaInteger), AwaResourceType_IntegerArray, SetArrayMode_Update);
}

AwaError AwaClientSetOperation_AddArrayValueAsFloat(AwaClientSetOperation * operation, const char * path, int resourceInstanceID, AwaFloat value)
{
    return ClientSetOperation_AddValue(operation, path, resourceInstanceID, (void *)&value, sizeof(AwaFloat), AwaResourceType_FloatArray, SetArrayMode_Update);
}

AwaError AwaClientSetOperation_AddArrayValueAsBoolean(AwaClientSetOperation * operation, const char * path, int resourceInstanceID, AwaBoolean value)
{
    return ClientSetOperation_AddValue(operation, path, resourceInstanceID, (void *)&value, sizeof(AwaBoolean), AwaResourceType_BooleanArray, SetArrayMode_Update);
}

AwaError AwaClientSetOperation_AddArrayValueAsTime(AwaClientSetOperation * operation, const char * path, int resourceInstanceID, AwaTime value)
{
    return ClientSetOperation_AddValue(operation, path, resourceInstanceID, (void *)&value, sizeof(AwaTime), AwaResourceType_TimeArray, SetArrayMode_Update);
}

AwaError AwaClientSetOperation_AddArrayValueAsOpaque(AwaClientSetOperation * operation, const char * path, int resourceInstanceID, AwaOpaque value)
{
    return ClientSetOperation_AddValue(operation, path, resourceInstanceID, value.Data, value.Size, AwaResourceType_OpaqueArray, SetArrayMode_Update);
}

AwaError AwaClientSetOperation_AddArrayValueAsObjectLink(AwaClientSetOperation * operation, const char * path, int resourceInstanceID, AwaObjectLink value)
{
    return ClientSetOperation_AddValue(operation, path, resourceInstanceID, (void *)&value, sizeof(AwaObjectLink), AwaResourceType_ObjectLinkArray, SetArrayMode_Update);
}

AwaError AwaClientSetOperation_Perform(AwaClientSetOperation * operation, AwaTimeout timeout)
{
    AwaError result = AwaError_Unspecified;

    if (timeout >= 0)
    {
        if (operation != NULL)
        {
            const AwaClientSession * session = OperationCommon_GetSession(operation->Common, NULL);
            if (session != NULL)
            {
                if (ClientSession_IsConnected(session))
                {
                    TreeNode objectsTree = OperationCommon_GetObjectsTree(operation->Common);
                    if (objectsTree != NULL)
                    {
                        if (TreeNode_GetChildCount(objectsTree) > 0)
                        {
                            IPCMessage * setRequest = IPCMessage_NewPlus(IPC_MESSAGE_TYPE_REQUEST, IPC_MESSAGE_SUB_TYPE_SET, OperationCommon_GetSessionID(operation->Common));
                            IPCMessage_AddContent(setRequest, objectsTree);

                            // Serialise message
                            char * requestBuffer = IPC_SerialiseMessageToXML(setRequest);
                            Awa_MemSafeFree(requestBuffer);

                            // Send via IPC
                            IPCMessage * setResponse = NULL;
                            result = IPC_SendAndReceive(ClientSession_GetChannel(session), setRequest, &setResponse, timeout);

                            // Process the response
                            if (result == AwaError_Success)
                            {
                                // Free an old Read response record if it exists
                                if (operation->Response != NULL)
                                {
                                    ResponseCommon_Free(&operation->Response);
                                }

                                TreeNode contentNode = IPCMessage_GetContentNode(setResponse);
                                TreeNode objectsNode = Xml_Find(contentNode, "Objects");
                                operation->Response = ResponseCommon_New(operation->Common, objectsNode);

                                result = ResponseCommon_CheckForErrors(operation->Response);

                                LogDebug("Perform Set Operation successful");
                            }
                            // Free allocated memory
                            IPCMessage_Free(&setRequest);
                            IPCMessage_Free(&setResponse);
                        }
                        else
                        {
                            result = LogErrorWithEnum(AwaError_OperationInvalid, "No paths specified");
                        }
                    }
                    else
                    {
                        result = LogErrorWithEnum(AwaError_Internal, "objectsTree is NULL");
                    }
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_SessionNotConnected, "session is not connected");
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
        result = LogErrorWithEnum(AwaError_OperationInvalid, "Invalid timeout specified");
    }
    return result;
}

const AwaClientSetResponse * AwaClientSetOperation_GetResponse(const AwaClientSetOperation * operation)
{
    const ResponseCommon * response = operation != NULL? operation->Response : NULL;
    // AwaClientSetResponse is an alias for ResponseCommon
    return (const AwaClientSetResponse *)response;
}

AwaPathIterator * AwaClientSetResponse_NewPathIterator(const AwaClientSetResponse * response)
{
    // AwaClientSetResponse is an alias for ResponseCommon
    // AwaPathIterator is an alias for PathIterator
    return (AwaPathIterator *)ResponseCommon_NewPathIterator((const ResponseCommon *)response);
}

const AwaPathResult * AwaClientSetResponse_GetPathResult(const AwaClientSetResponse * response, const char * path)
{
    // AwaServerSetResponse is an alias for ResponseCommon
    const PathResult * pathResult = NULL;
    ResponseCommon_GetPathResult((const ResponseCommon *)response, path, &pathResult);
    // AwaPathResult is an alias for PathResult
    return (AwaPathResult *)pathResult;
}

bool AwaClientSetResponse_ContainsPath(const AwaClientSetResponse * response, const char * path)
{
    // AwaServerSetResponse is an alias for ResponseCommon
    return ResponseCommon_ContainsPath((const ResponseCommon *)response, path);
}


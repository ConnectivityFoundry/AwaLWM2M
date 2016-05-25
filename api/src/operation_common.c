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
#include "operation_common.h"
#include "error.h"
#include "path.h"
#include "xml.h"
#include "log.h"
#include "memalloc.h"
#include "objects_tree.h"
#include "session_common.h"

// A generic operation doesn't care whether it holds a Client or a Server session,
// but it does need to provide this information to its users.
struct _OperationCommon
{
    const Session * Session;  // intended to hold AwaClientSession or AwaServerSession
    SessionType SessionType;
    TreeNode ObjectsTree;
};

OperationCommon * OperationCommon_NewWithClientSession(const AwaClientSession * session)
{
    return OperationCommon_New(session, SessionType_Client);
}

OperationCommon * OperationCommon_NewWithServerSession(const AwaServerSession * session)
{
    return OperationCommon_New(session, SessionType_Server);
}

OperationCommon * OperationCommon_NewWithExistingObjectsTree(const Session * session, SessionType sessionType, TreeNode objectsTree)
{
    OperationCommon * operation = NULL;
    if (session != NULL)
    {
        operation = Awa_MemAlloc(sizeof(*operation));
        if (operation != NULL)
        {
            memset(operation, 0, sizeof(*operation));
            operation->Session = session;
            operation->SessionType = sessionType;
            operation->ObjectsTree = objectsTree;
            if (operation->ObjectsTree != NULL)
            {
                LogNew("OperationCommon", operation);
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
        LogErrorWithEnum(AwaError_SessionInvalid, "Session is NULL");
    }
    return operation;
}

OperationCommon * OperationCommon_New(const Session * session, SessionType sessionType)
{
    TreeNode objectsTree = ObjectsTree_New();
    OperationCommon * operation = OperationCommon_NewWithExistingObjectsTree(session, sessionType, objectsTree);
    if (operation == NULL)
    {
        Tree_Delete(objectsTree);
    }
    return operation;
}

void OperationCommon_Free(OperationCommon ** operation)
{
    if ((operation != NULL) && (*operation != NULL))
    {
        // do not free the session, it is not owned by the operation

        Tree_DetachNode((*operation)->ObjectsTree);
        ObjectsTree_Free((*operation)->ObjectsTree);

        LogFree("OperationCommon", operation);
        Awa_MemSafeFree(*operation);
        *operation = NULL;
    }
}

const Session * OperationCommon_GetSession(const OperationCommon * operation, SessionType * sessionType)
{
    const Session * session = NULL;
    if (operation != NULL)
    {
        session = operation->Session;

        // optionally return sessionType, if needed:
        if (sessionType != NULL)
        {
            *sessionType = operation->SessionType;
        }
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "operation is NULL");
    }
    return session;
}

const SessionCommon * OperationCommon_GetSessionCommon(const OperationCommon * operation)
{
    const SessionCommon * sessionCommon = NULL;
    if (operation != NULL)
    {
        // to get the common Session, we must know what kind of session it is
        SessionType sessionType = SessionType_Invalid;
        const Session * sessionBlind = OperationCommon_GetSession(operation, &sessionType);

        // use SessionType to safely cast and dereference the Session pointer
        switch (sessionType)
        {
            case SessionType_Client:
                sessionCommon = ClientSession_GetSessionCommon((const AwaClientSession *)sessionBlind);
                break;
            case SessionType_Server:
                sessionCommon = ServerSession_GetSessionCommon((const AwaServerSession *)sessionBlind);
                break;
            default:
                LogErrorWithEnum(AwaError_Internal, "Invalid SessionType %d", sessionType);
                break;
        }
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "operation is NULL");
    }
    return sessionCommon;
}

TreeNode OperationCommon_GetObjectsTree(const OperationCommon * operation)
{
    TreeNode objectsTree = NULL;
    if (operation != NULL)
    {
        objectsTree = operation->ObjectsTree;
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "operation is NULL");
    }
    return objectsTree;
}

AwaError OperationCommon_AddPathToObjectsTree(TreeNode objectsTree, const char * path, TreeNode * resultNode)
{
    AwaError result = AwaError_Unspecified;

    if (objectsTree != NULL)
    {
        if (path != NULL)
        {
            if (Path_IsValid(path))
            {
                // Drop paths that are already represented
                // E.g. /3/0/0 should be dropped if /3/0 is already present
                if (ObjectsTree_IsPathCovered(objectsTree, path, resultNode) == false)
                {
                    // if a new path that covers existing paths is added, remove any existing path nodes
                    TreeNode existing = NULL;
                    if (ObjectsTree_FindPathNode(objectsTree, path, &existing) == InternalError_Success)
                    {
                        if (resultNode != NULL)
                        {
                            *resultNode = existing;
                        }
                        ObjectsTree_RemovePathNodes(existing);
                        LogDebug("Removing nodes below %s", path);
                        result = AwaError_Success;
                    }
                    else
                    {
                        if (ObjectsTree_AddPath(objectsTree, path, resultNode) == InternalError_Success)
                        {
                            result = AwaError_Success;
                        }
                        else
                        {
                            result = LogErrorWithEnum(AwaError_Internal, "AddPath failed");
                        }
                    }
                }
                else
                {
                    LogDebug("Dropping path %s", path);
                    result = AwaError_Success;
                }
            }
            else
            {
                result = LogErrorWithEnum(AwaError_PathInvalid, "Path %s is not valid", path);
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_PathInvalid, "Path is NULL");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "Objects Tree is NULL");
    }
    return result;
}

AwaError OperationCommon_AddPathV2(OperationCommon * operation, const char * path, TreeNode * resultNode)
{
    AwaError result = AwaError_Unspecified;

    if (operation != NULL)
    {
        result = OperationCommon_AddPathToObjectsTree(operation->ObjectsTree, path, resultNode);
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "Operation is NULL");
    }
    return result;
}

static TreeNode AddIDRange(TreeNode resourceNode, AwaArrayIndex startIndex, AwaArrayIndex endIndexExclusive)
{
    TreeNode rangeNode = NULL;
    if (resourceNode != NULL)
    {
        // add an <IDRange> node to the supplied valuesNode
        rangeNode = Xml_CreateNode("IDRange");
        TreeNode rangeStartNode = Xml_CreateNodeWithValue("Start", "%d", startIndex);
        TreeNode rangeEndNode = Xml_CreateNodeWithValue("EndExclusive", "%d", endIndexExclusive);

        TreeNode_AddChild(rangeNode, rangeStartNode);
        TreeNode_AddChild(rangeNode, rangeEndNode);
        TreeNode_AddChild(resourceNode, rangeNode);
    }
    else
    {
        LogErrorWithEnum(AwaError_Internal, "resourceNode is NULL");
    }
    return rangeNode;
}

AwaError OperationCommon_AddPathWithArrayRange(OperationCommon * operation, const char * path, AwaArrayIndex startIndex, AwaArrayLength indexCount)
{
    AwaError result = AwaError_Unspecified;
    if (operation != NULL)
    {
        if (path != NULL)
        {
            if (Path_IsValid(path))
            {
                if (Path_IsValidForResource(path))
                {
                    // catch zero count and index overflow
                    if (startIndex + indexCount > startIndex)
                    {
                        // build up the required tree (may already exist)
                        if (ObjectsTree_AddPath(operation->ObjectsTree, path, NULL) == InternalError_Success)
                        {
                            // find the corresponding Property/Resource node
                            TreeNode resourceNode = NULL;
                            if (ObjectsTree_FindPathNode(operation->ObjectsTree, path, &resourceNode) == InternalError_Success)
                            {
                                if (resourceNode != NULL)
                                {
                                    if (AddIDRange(resourceNode, startIndex, startIndex + indexCount) != NULL)
                                    {
                                        result = AwaError_Success;
                                    }
                                    else
                                    {
                                        result = LogErrorWithEnum(AwaError_Internal, "failed to add IDRange");
                                    }
                                }
                                else
                                {
                                    result = LogErrorWithEnum(AwaError_Internal, "propertyNode is NULL");
                                }
                            }
                            else
                            {
                                result = LogErrorWithEnum(AwaError_Internal, "propertyNode not found");
                            }
                        }
                        else
                        {
                            result = LogErrorWithEnum(AwaError_Internal, "unable to build tree");
                        }

                    }
                    else
                    {
                        if (indexCount == 0)
                        {
                            result = LogErrorWithEnum(AwaError_AddInvalid, "Index count is not greater than zero");
                        }
                        else
                        {
                            result = LogErrorWithEnum(AwaError_AddInvalid, "Range overflow");
                        }
                    }
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_PathInvalid, "Path %s is not supported for this operation", path);
                }
            }
            else
            {
                result = LogErrorWithEnum(AwaError_PathInvalid, "Path %s is not valid", path);
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_PathInvalid, "Path is NULL");
        }
    }
    else
    {
        result = LogErrorWithEnum(AwaError_OperationInvalid, "Operation is NULL");
    }
    return result;
}

IPCSessionID OperationCommon_GetSessionID(const OperationCommon * operation)
{
    return SessionCommon_GetSessionID(OperationCommon_GetSessionCommon(operation));
}


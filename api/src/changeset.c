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

#include "awa/client.h"
#include "awa/common.h"
#include "changeset.h"
#include "log.h"
#include "ipc.h"
#include "lwm2m_tree_node.h"
#include "memalloc.h"
#include "path_iterator.h"
#include "path.h"
#include "utils.h"
#include "arrays.h"
#include "operation_common.h"
#include "response_common.h"
#include "get_response.h"
#include "client_subscribe.h"
#include "client_session.h"

struct _AwaChangeSet
{
    OperationCommon * OperationCommon;
    ResponseCommon * ResponseCommon;
    const char * ClientID; // server side only
};

AwaChangeSet * ChangeSet_NewWithClientID(Session * session, SessionType sessionType, TreeNode objectsTree, const char * clientID)
{
    AwaChangeSet * changeSet = NULL;
    if (objectsTree != NULL)
    {
        changeSet = Awa_MemAlloc(sizeof(*changeSet));
        if (changeSet != NULL)
        {
            memset(changeSet, 0, sizeof(*changeSet));

            changeSet->OperationCommon = OperationCommon_New(session, sessionType);

            if (changeSet->OperationCommon != NULL)
            {
                changeSet->ResponseCommon = ResponseCommon_New(changeSet->OperationCommon, objectsTree);
                if (changeSet->ResponseCommon != NULL)
                {
                    changeSet->ClientID = clientID;
                    LogNew("AwaChangeSet New", changeSet);
                }
                else
                {
                    LogErrorWithEnum(AwaError_OutOfMemory);
                    OperationCommon_Free(&changeSet->OperationCommon);
                    Awa_MemSafeFree(changeSet);
                    changeSet = NULL;
                }
            }
            else
            {
                LogErrorWithEnum(AwaError_OutOfMemory);
                Awa_MemSafeFree(changeSet);
                changeSet = NULL;
            }
        }
        else
        {
            LogErrorWithEnum(AwaError_OutOfMemory);
        }
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "Objects tree is NULL");
    }
    return changeSet;
}

AwaChangeSet * ChangeSet_New(Session * session, SessionType sessionType, TreeNode objectsTree)
{
    return ChangeSet_NewWithClientID(session, sessionType, objectsTree, NULL);
}

AwaError ChangeSet_Free(AwaChangeSet ** changeSet)
{
    AwaError result = AwaError_OperationInvalid;
    if ((changeSet != NULL) && (*changeSet != NULL))
    {
        LogFree("AwaChangeSet", *changeSet);
        ResponseCommon_Free(&(*changeSet)->ResponseCommon);
        OperationCommon_Free(&(*changeSet)->OperationCommon);
        Awa_MemSafeFree(*changeSet);
        *changeSet = NULL;
        result = AwaError_Success;
    }
    return result;
}

const char * AwaChangeSet_GetClientID(const AwaChangeSet * changeSet)
{
    const char * clientID = NULL;
    if (changeSet != NULL)
    {
        clientID = changeSet->ClientID;
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "changeSet is NULL");
    }
    return clientID;
}

AwaPathIterator * AwaChangeSet_NewPathIterator(const AwaChangeSet * changeSet)
{
    PathIterator * iterator = NULL;
    if (changeSet != NULL)
    {
        iterator = ResponseCommon_NewPathIterator(changeSet->ResponseCommon);
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "changeSet is NULL");
    }
    // AwaPathIterator is an alias for PathIterator
    return (AwaPathIterator *)iterator;
}

static const Session * AwaChangeSet_GetSession(const AwaChangeSet * changeSet, SessionType sessionType)
{
    const Session * session = NULL;
    if (changeSet != NULL)
    {
        SessionType actualSessionType = SessionType_Invalid;
        session = OperationCommon_GetSession(ResponseCommon_GetOperation(changeSet->ResponseCommon), &actualSessionType);
        if (sessionType != actualSessionType)
        {
            LogErrorWithEnum(AwaError_OperationInvalid, "Changeset does not hold a session of type %d. Requesting Session type %d", actualSessionType, sessionType);
            session = NULL;
        }
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "ChangeSet is NULL");
    }
    return session;
}
const AwaClientSession * AwaChangeSet_GetClientSession(const AwaChangeSet * changeSet)
{
    return (const AwaClientSession * )AwaChangeSet_GetSession(changeSet, SessionType_Client);
}
const AwaServerSession * AwaChangeSet_GetServerSession(const AwaChangeSet * changeSet)
{
    return (const AwaServerSession * )AwaChangeSet_GetSession(changeSet, SessionType_Server);
}

AwaChangeType AwaChangeSet_GetChangeType(const AwaChangeSet * changeSet, const char * path)
{
    /* TODO: Currently this function only returns AwaChangeType_ResourceModified,
     *       but should be updated to also support
     *
     *   AwaClientChangeType_Invalid = 0,
     *   AwaClientChangeType_ResourceCreated,
     *   AwaClientChangeType_ResourceModified,
     *   AwaClientChangeType_ResourceDeleted,
     *   AwaClientChangeType_ObjectInstanceCreated,
     *   AwaClientChangeType_ObjectInstanceModified,
     *   AwaClientChangeType_ObjectInstanceDeleted,
     *   AwaClientChangeType_Current,
     */
    if (changeSet == NULL || path == NULL)
    {
        return AwaChangeType_Invalid;
    }
    return AwaChangeType_ResourceModified; //AwaClientChangeType_Invalid;
}

AwaResourceType AwaChangeSet_GetResourceType(const AwaChangeSet * changeSet, const char * path)
{
    if (changeSet != NULL)
    {
        const AwaResourceDefinition * resourceDefinition = SessionCommon_GetResourceDefinitionFromPath(OperationCommon_GetSessionCommon(changeSet->OperationCommon), path);
        if (resourceDefinition)
        {
            return AwaResourceDefinition_GetType(resourceDefinition);
        }
    }
    else
    {
        LogError("ChangeSet is null");
    }
    return AwaResourceType_Invalid;
}

bool AwaChangeSet_HasValue(const AwaChangeSet * changeSet, const char * path)
{
    bool hasValue = false;

    if (changeSet != NULL)
    {
        hasValue = ResponseCommon_HasValue(changeSet->ResponseCommon, path);
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "ChangeSet is NULL");
    }
    return hasValue;
}

bool AwaChangeSet_ContainsPath(const AwaChangeSet * changeSet, const char * path)
{
    bool containsPath = false;

    if (changeSet != NULL)
    {
        containsPath = ResponseCommon_ContainsPath(changeSet->ResponseCommon, path);
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "ChangeSet is NULL");
    }
    return containsPath;
}

AwaError ChangeSet_GetExecuteArguments(const AwaChangeSet *changeSet, const char * path, AwaExecuteArguments * arguments)
{
    AwaError error = AwaError_Unspecified;
    if (changeSet != NULL)
    {
        if (arguments != NULL)
        {
            AwaOpaque executePayload = {NULL, 0};
            error = AwaChangeSet_GetValueAsOpaque(changeSet, path, &executePayload);
            if (error == AwaError_Success)
            {
                arguments->Data = executePayload.Data;
                arguments->Size = executePayload.Size;
            }
        }
        else
        {
            error = LogErrorWithEnum(AwaError_OperationInvalid, "Arguments are NULL");
        }
    }
    else
    {
        error = LogErrorWithEnum(AwaError_OperationInvalid, "ChangeSet is NULL");
    }
    return error;
}

TreeNode ChangeSet_GetObjectsTree(const AwaChangeSet * changeSet)
{
    TreeNode objectsTree = NULL;

    if (changeSet != NULL)
    {
        objectsTree = ResponseCommon_GetObjectsNode(changeSet->ResponseCommon);
    }
    else
    {
        LogErrorWithEnum(AwaError_OperationInvalid, "ChangeSet is NULL");
    }
    return objectsTree;
}

AwaError AwaChangeSet_GetValueAsCStringPointer(const AwaChangeSet * changeSet, const char * path, const char ** value)
{
    return ResponseCommon_GetValuePointerWithNull(changeSet->ResponseCommon, path, (const void **)value, NULL, AwaResourceType_String, -1);
}

AwaError AwaChangeSet_GetValueAsIntegerPointer(const AwaChangeSet * changeSet, const char * path, const AwaInteger ** value)
{
    return ResponseCommon_GetValuePointer(changeSet->ResponseCommon, path, (const void **)value, NULL, AwaResourceType_Integer, sizeof(AwaInteger));
}

AwaError AwaChangeSet_GetValueAsFloatPointer(const AwaChangeSet * changeSet, const char * path, const AwaFloat ** value)
{
    return ResponseCommon_GetValuePointer(changeSet->ResponseCommon, path, (const void **)value, NULL, AwaResourceType_Float, sizeof(AwaFloat));
}

AwaError AwaChangeSet_GetValueAsBooleanPointer(const AwaChangeSet * changeSet, const char * path, const AwaBoolean ** value)
{
    return ResponseCommon_GetValuePointer(changeSet->ResponseCommon, path, (const void **)value, NULL, AwaResourceType_Boolean, sizeof(AwaBoolean));
}

AwaError AwaChangeSet_GetValueAsTimePointer(const AwaChangeSet * changeSet, const char * path, const AwaTime ** value)
{
    return ResponseCommon_GetValuePointer(changeSet->ResponseCommon, path, (const void **)value, NULL, AwaResourceType_Time, sizeof(AwaTime));
}

AwaError AwaChangeSet_GetValueAsObjectLinkPointer(const AwaChangeSet * changeSet, const char * path, const AwaObjectLink ** value)
{
    return ResponseCommon_GetValuePointer(changeSet->ResponseCommon, path, (const void **)value, NULL, AwaResourceType_ObjectLink, sizeof(AwaObjectLink));
}

AwaError AwaChangeSet_GetValueAsOpaquePointer(const AwaChangeSet * changeSet, const char * path, const AwaOpaque ** value)
{
    return ResponseCommon_GetValuePointer(changeSet->ResponseCommon, path, (const void **)value, NULL, AwaResourceType_Opaque, sizeof(AwaOpaque));
}

AwaError AwaChangeSet_GetValueAsOpaque(const AwaChangeSet * changeSet, const char * path, AwaOpaque * value)
{
    return ResponseCommon_GetValueAsOpaque(changeSet->ResponseCommon, path, value);
}

AwaError AwaChangeSet_GetValueAsObjectLink(const AwaChangeSet * changeSet, const char * path, AwaObjectLink * value)
{
    return ResponseCommon_GetValueAsObjectLink(changeSet->ResponseCommon, path, value);
}

AwaError AwaChangeSet_GetValuesAsStringArrayPointer(const AwaChangeSet * changeSet, const char * path, const AwaStringArray ** valueArray)
{
    return ResponseCommon_GetValuePointer(changeSet->ResponseCommon, path, (const void **)valueArray, NULL, AwaResourceType_StringArray, sizeof(AwaStringArray *));
}

AwaError AwaChangeSet_GetValuesAsIntegerArrayPointer(const AwaChangeSet * changeSet, const char * path, const AwaIntegerArray ** valueArray)
{
    return ResponseCommon_GetValuePointer(changeSet->ResponseCommon, path, (const void **)valueArray, NULL, AwaResourceType_IntegerArray, sizeof(AwaIntegerArray *));
}

AwaError AwaChangeSet_GetValuesAsFloatArrayPointer(const AwaChangeSet * changeSet, const char * path, const AwaFloatArray ** valueArray)
{
    return ResponseCommon_GetValuePointer(changeSet->ResponseCommon, path, (const void **)valueArray, NULL, AwaResourceType_FloatArray, sizeof(AwaFloatArray *));
}

AwaError AwaChangeSet_GetValuesAsBooleanArrayPointer(const AwaChangeSet * changeSet, const char * path, const AwaBooleanArray ** valueArray)
{
    return ResponseCommon_GetValuePointer(changeSet->ResponseCommon, path, (const void **)valueArray, NULL, AwaResourceType_BooleanArray, sizeof(AwaBooleanArray *));
}

AwaError AwaChangeSet_GetValuesAsTimeArrayPointer(const AwaChangeSet * changeSet, const char * path, const AwaTimeArray ** valueArray)
{
    return ResponseCommon_GetValuePointer(changeSet->ResponseCommon, path, (const void **)valueArray, NULL, AwaResourceType_TimeArray, sizeof(AwaTimeArray *));
}

AwaError AwaChangeSet_GetValuesAsOpaqueArrayPointer(const AwaChangeSet * changeSet, const char * path, const AwaOpaqueArray ** valueArray)
{
    return ResponseCommon_GetValuePointer(changeSet->ResponseCommon, path, (const void **)valueArray, NULL, AwaResourceType_OpaqueArray, sizeof(AwaOpaqueArray *));
}

AwaError AwaChangeSet_GetValuesAsObjectLinkArrayPointer(const AwaChangeSet * changeSet, const char * path, const AwaObjectLinkArray ** valueArray)
{
    return ResponseCommon_GetValuePointer(changeSet->ResponseCommon, path, (const void **)valueArray, NULL, AwaResourceType_ObjectLinkArray, sizeof(AwaObjectLinkArray *));
}


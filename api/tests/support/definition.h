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

#ifndef DEFINITION_H
#define DEFINITION_H

#include <string>
#include <vector>

#include "awa/common.h"
#include "awa/client.h"
#include "support.h"

namespace Awa {

struct ResourceDescription
{
    ResourceDescription(AwaResourceID id,
                        std::string name,
                        AwaResourceType type,
                        int minInstances,
                        int maxInstances,  // ignored for non-array resources
                        AwaResourceOperations operations) :
        ID(id),
        Name(name),
        Type(type),
        MinInstances(minInstances),
        MaxInstances(maxInstances),
        Operations(operations)
    {
    }

    AwaResourceID ID;
    std::string Name;
    AwaResourceType Type;
    int MinInstances;
    int MaxInstances;
    AwaResourceOperations Operations;
};

class ObjectDescription
{
public:
    ObjectDescription(AwaObjectID id,
                      std::string name,
                      int minInstances,
                      int maxInstances,
                      const std::vector<ResourceDescription> & resources) :
        objectDefinition_(nullptr)
    {
        objectDefinition_ = AwaObjectDefinition_New(id, name.c_str(), minInstances, maxInstances);
        if (objectDefinition_ != NULL)
        {
            for (auto it = resources.begin(); it != resources.end(); ++it)
            {
                const ResourceDescription & rd = *it;
                bool mandatory = rd.MinInstances > 0;
                switch (rd.Type)
                {
                    case AwaResourceType_None:
                        AwaObjectDefinition_AddResourceDefinitionAsNoType(objectDefinition_, rd.ID, rd.Name.c_str(), mandatory, rd.Operations);
                        break;
                    case AwaResourceType_String:
                        AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition_, rd.ID, rd.Name.c_str(), mandatory, rd.Operations, "");
                        break;
                    case AwaResourceType_Integer:
                        AwaObjectDefinition_AddResourceDefinitionAsInteger(objectDefinition_, rd.ID, rd.Name.c_str(), mandatory, rd.Operations, 0);
                        break;
                    case AwaResourceType_Float:
                        AwaObjectDefinition_AddResourceDefinitionAsFloat(objectDefinition_, rd.ID, rd.Name.c_str(), mandatory, rd.Operations, 0.0);
                        break;
                    case AwaResourceType_Boolean:
                        AwaObjectDefinition_AddResourceDefinitionAsBoolean(objectDefinition_, rd.ID, rd.Name.c_str(), mandatory, rd.Operations, false);
                        break;
                    case AwaResourceType_Time:
                        AwaObjectDefinition_AddResourceDefinitionAsTime(objectDefinition_, rd.ID, rd.Name.c_str(), mandatory, rd.Operations, 0);
                        break;
                    case AwaResourceType_ObjectLink:
                        AwaObjectDefinition_AddResourceDefinitionAsObjectLink(objectDefinition_, rd.ID, rd.Name.c_str(), mandatory, rd.Operations, AwaObjectLink { 0, 0 } );
                        break;
                    case AwaResourceType_Opaque:
                        AwaObjectDefinition_AddResourceDefinitionAsOpaque(objectDefinition_, rd.ID, rd.Name.c_str(), mandatory, rd.Operations, AwaOpaque { nullptr, 0 } );
                        break;

                    // Arrays:
                    case AwaResourceType_StringArray:
                        AwaObjectDefinition_AddResourceDefinitionAsStringArray(objectDefinition_, rd.ID, rd.Name.c_str(), rd.MinInstances, rd.MaxInstances, rd.Operations, NULL);
                        break;
                    case AwaResourceType_IntegerArray:
                        AwaObjectDefinition_AddResourceDefinitionAsIntegerArray(objectDefinition_, rd.ID, rd.Name.c_str(), rd.MinInstances, rd.MaxInstances, rd.Operations, NULL);
                        break;
                    case AwaResourceType_FloatArray:
                        AwaObjectDefinition_AddResourceDefinitionAsFloatArray(objectDefinition_, rd.ID, rd.Name.c_str(), rd.MinInstances, rd.MaxInstances, rd.Operations, NULL);
                        break;
                    case AwaResourceType_BooleanArray:
                        AwaObjectDefinition_AddResourceDefinitionAsBooleanArray(objectDefinition_, rd.ID, rd.Name.c_str(), rd.MinInstances, rd.MaxInstances, rd.Operations, NULL);
                        break;
                    case AwaResourceType_TimeArray:
                        AwaObjectDefinition_AddResourceDefinitionAsTimeArray(objectDefinition_, rd.ID, rd.Name.c_str(), rd.MinInstances, rd.MaxInstances, rd.Operations, NULL);
                        break;
                    case AwaResourceType_ObjectLinkArray:
                        AwaObjectDefinition_AddResourceDefinitionAsObjectLinkArray(objectDefinition_, rd.ID, rd.Name.c_str(), rd.MinInstances, rd.MaxInstances, rd.Operations, NULL);
                        break;
                    case AwaResourceType_OpaqueArray:
                        AwaObjectDefinition_AddResourceDefinitionAsOpaqueArray(objectDefinition_, rd.ID, rd.Name.c_str(), rd.MinInstances, rd.MaxInstances, rd.Operations, NULL);
                        break;

                    default:
                        printf("TEST ERROR: Unhandled resource type %d\n", rd.Type);
                        break;
                }
            }
        }
    }


    ~ObjectDescription() { AwaObjectDefinition_Free(&objectDefinition_); }
    const AwaObjectDefinition * GetDefinition() const { return objectDefinition_; }

private:
    AwaObjectDefinition * objectDefinition_;
};

class ClientDefineWrapper
{
public:
    typedef AwaClientDefineOperation DefineOperation;
    typedef AwaClientSession Session;

    explicit ClientDefineWrapper(Session * session) : session_(session) {}
    DefineOperation * DefineOperation_New() {
        return AwaClientDefineOperation_New(session_);
    }
    AwaError DefineOperation_Free(DefineOperation ** operation) {
        return AwaClientDefineOperation_Free(operation);
    }
    AwaError DefineOperation_Add(DefineOperation * operation, const AwaObjectDefinition * objectDefinition) {
        return AwaClientDefineOperation_Add(operation, objectDefinition);
    }
    AwaError DefineOperation_Perform(DefineOperation * operation, AwaTimeout timeout) {
        return AwaClientDefineOperation_Perform(operation, timeout);
    }

private:
    Session * session_;
};

class ServerDefineWrapper
{
public:
    typedef AwaServerDefineOperation DefineOperation;
    typedef AwaServerSession Session;

    explicit ServerDefineWrapper(Session * session) : session_(session) {}
    DefineOperation * DefineOperation_New() {
        return AwaServerDefineOperation_New(session_);
    }
    AwaError DefineOperation_Free(DefineOperation ** operation) {
        return AwaServerDefineOperation_Free(operation);
    }
    AwaError DefineOperation_Add(DefineOperation * operation, const AwaObjectDefinition * objectDefinition) {
        return AwaServerDefineOperation_Add(operation, objectDefinition);
    }
    AwaError DefineOperation_Perform(DefineOperation * operation, AwaTimeout timeout) {
        return AwaServerDefineOperation_Perform(operation, timeout);
    }

private:
    Session * session_;
};

template <typename Interface, typename Session>
AwaError DoDefine(Session session, const ObjectDescription & objectDescription)
{
    AwaError result = AwaError_Unspecified;
    Interface interface(session);

    auto defineOperation = interface.DefineOperation_New();
    if (defineOperation != NULL)
    {
        auto objectDefinition = objectDescription.GetDefinition();
        if (objectDefinition != NULL)
        {
            result = interface.DefineOperation_Add(defineOperation, objectDefinition);
            if (result == AwaError_Success)
            {
                result = interface.DefineOperation_Perform(defineOperation, global::timeout);
                if (result == AwaError_Success)
                {
                    result = AwaError_Success;
                }
                else
                {
                    printf("DefineOperation_Add failed\n");
                }
            }
            else
            {
                printf("DefineOperation_Add failed\n");
            }
        }
        else
        {
            printf("objectDefinition is NULL\n");
            result = AwaError_DefinitionInvalid;
        }
    }
    else
    {
        printf("defineOperation is NULL\n");
        result = AwaError_OperationInvalid;
    }

    // objectDescription owns objectDefinition and will free in destructor
    interface.DefineOperation_Free(&defineOperation);
    return result;
}


template <typename T>
AwaError Define(T * session, const ObjectDescription & objectDescription);

template <>
inline
AwaError Define(AwaClientSession * session, const ObjectDescription & objectDescription)
{
    return DoDefine<ClientDefineWrapper>(session, objectDescription);
}

template <>
inline
AwaError Define(AwaServerSession * session, const ObjectDescription & objectDescription)
{
    return DoDefine<ServerDefineWrapper>(session, objectDescription);
}


} // namespace Awa

#endif // DEFINITION_H



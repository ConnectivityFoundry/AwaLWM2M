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

#include <gtest/gtest.h>

#include "awa/client.h"
#include "support/support.h"
#include "define_common.h"

namespace Awa {

namespace detail {
    const AwaObjectID TEST_OBJECT_ID_VALID = 3;
    const AwaObjectID TEST_OBJECT_ID_INVALID = 65000;

    const AwaObjectID TEST_TEMPORARY_OBJECT_ID = 9999;


    const int TEST_MULTIPLE_INSTANCES_MAX = 20;
} // namespace detail

class TestDefineCommon : public TestClientBase {};
class TestDefineCommonOperationWithConnectedSession : public TestClientWithConnectedSession {};

TEST_F(TestDefineCommon, AwaObjectDefinition_New_and_Free_valid_inputs)
{
    AwaObjectDefinition * definition = AwaObjectDefinition_New(detail::TEST_OBJECT_ID_VALID, "Device Object", 1, 1);
    ASSERT_TRUE(NULL != definition);

    AwaObjectDefinition_Free(&definition);
    ASSERT_EQ(NULL, definition);
}

TEST_F(TestDefineCommon, AwaObjectDefinition_GetMaximumInstances_valid_inputs)
{
    for(int i = 1; i < AWA_MAX_ID; i+=10000)
    {
        AwaObjectDefinition * definition = AwaObjectDefinition_New(detail::TEST_OBJECT_ID_VALID, "Device Object", 0, i);
        EXPECT_TRUE(NULL != definition);

        ASSERT_EQ(i, AwaObjectDefinition_GetMaximumInstances(definition));

        AwaObjectDefinition_Free(&definition);
    }
}

TEST_F(TestDefineCommon, AwaObjectDefinition_GetMaximumInstances_invalid_inputs)
{
    ASSERT_EQ(AWA_INVALID_ID, AwaObjectDefinition_GetMaximumInstances(NULL));
}

TEST_F(TestDefineCommon, AwaObjectDefinition_GetMinimumInstances_valid_inputs)
{
    for(int i = 0; i < AWA_MAX_ID - 1; i+=10000)
    {
        AwaObjectDefinition * definition = AwaObjectDefinition_New(detail::TEST_OBJECT_ID_VALID, "Device Object", i, i+1);
        EXPECT_TRUE(NULL != definition);

        ASSERT_EQ(i, AwaObjectDefinition_GetMinimumInstances(definition));

        AwaObjectDefinition_Free(&definition);
    }
}

TEST_F(TestDefineCommon, AwaObjectDefinition_GetMinimumInstances_invalid_inputs)
{
    ASSERT_EQ(AWA_INVALID_ID, AwaObjectDefinition_GetMinimumInstances(NULL));
}

TEST_F(TestDefineCommon, AwaObjectDefinition_GetIDs_valid_inputs)
{
    for(int i = 1; i < AWA_MAX_ID - 1; i+=10000)
    {
        AwaObjectDefinition * definition = AwaObjectDefinition_New(i, "Device Object", 1, 1);
        EXPECT_TRUE(NULL != definition);

        ASSERT_EQ(i, AwaObjectDefinition_GetID(definition));

        AwaObjectDefinition_Free(&definition);
    }
}

TEST_F(TestDefineCommon, AwaObjectDefinition_GetIDs_invalid_inputs)
{
    ASSERT_EQ(AWA_INVALID_ID, AwaObjectDefinition_GetID(NULL));
}

TEST_F(TestDefineCommon, AwaObjectDefinition_GetName_valid_inputs)
{
    const char * expectedName = "Device Object";

    AwaObjectDefinition * definition = AwaObjectDefinition_New(detail::TEST_OBJECT_ID_VALID, expectedName, 1, 1);
    EXPECT_TRUE(NULL != definition);

    ASSERT_STREQ(expectedName, AwaObjectDefinition_GetName(definition));

    AwaObjectDefinition_Free(&definition);
}

TEST_F(TestDefineCommon, AwaObjectDefinition_GetName_invalid_inputs)
{
    ASSERT_TRUE(NULL == AwaObjectDefinition_GetName(NULL));
}

TEST_F(TestDefineCommon, AwaObjectDefinition_NeW_handles_MinimumInstances_greater_than_MaximumInstances)
{
    AwaObjectDefinition * definition = AwaObjectDefinition_New(detail::TEST_OBJECT_ID_VALID, "Device Object", 50, 1);
    ASSERT_TRUE(NULL == definition);
}

TEST_F(TestDefineCommon, AwaObjectDefinition_New_handles_invalid_inputs)
{
    AwaObjectDefinition * definition = AwaObjectDefinition_New(detail::TEST_OBJECT_ID_VALID, NULL, 1, 1);
    ASSERT_TRUE(NULL == definition);
    definition = AwaObjectDefinition_New(detail::TEST_OBJECT_ID_VALID, "Device Object", 0, 0);
    ASSERT_TRUE(NULL == definition);
    definition = AwaObjectDefinition_New(AWA_INVALID_ID, "Device Object", 1, 1);
    ASSERT_TRUE(NULL == definition);
    definition = AwaObjectDefinition_New(detail::TEST_OBJECT_ID_VALID, "Device Object", AWA_INVALID_ID, 1);
    ASSERT_TRUE(NULL == definition);
    definition = AwaObjectDefinition_New(detail::TEST_OBJECT_ID_VALID, "Device Object", 1, AWA_INVALID_ID);
    ASSERT_TRUE(NULL == definition);
}

TEST_F(TestDefineCommon, AwaObjectDefinition_AddResourceDefinitionAsNoType_valid_inputs)
{
    AwaObjectDefinition * definition = AwaObjectDefinition_New(detail::TEST_OBJECT_ID_VALID, "Test Object", 1, 1);
    EXPECT_TRUE(NULL != definition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsNoType(definition, 1, "Test Resource No Type", false, AwaResourceOperations_Execute));

    AwaObjectDefinition_Free(&definition);
}

TEST_F(TestDefineCommon, AwaObjectDefinition_AddResourceDefinition_handles_duplicate_resource_id)
{
    AwaObjectDefinition * definition = AwaObjectDefinition_New(detail::TEST_OBJECT_ID_VALID, "Test Object", 1, 1);
    EXPECT_TRUE(NULL != definition);

    ASSERT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsNoType(definition, 1, "Test Resource No Type", false, AwaResourceOperations_Execute));
    ASSERT_EQ(AwaError_AlreadyDefined, AwaObjectDefinition_AddResourceDefinitionAsNoType(definition, 1, "Test Resource No Type", false, AwaResourceOperations_Execute));

    AwaObjectDefinition_Free(&definition);
}

TEST_F(TestDefineCommon, AwaObjectDefinition_AddResourceDefinitionAsNoType_invalid_inputs)
{
    AwaObjectDefinition * definition = AwaObjectDefinition_New(detail::TEST_OBJECT_ID_VALID, "Test Object", 1, 1);
    EXPECT_TRUE(NULL != definition);

    ASSERT_EQ(AwaError_IDInvalid, AwaObjectDefinition_AddResourceDefinitionAsNoType(definition, AWA_INVALID_ID, "Test Resource No Type", false, AwaResourceOperations_Execute));
    ASSERT_EQ(AwaError_DefinitionInvalid, AwaObjectDefinition_AddResourceDefinitionAsNoType(NULL, 1, "Test Resource No Type", false, AwaResourceOperations_Execute));
    ASSERT_EQ(AwaError_DefinitionInvalid, AwaObjectDefinition_AddResourceDefinitionAsNoType(definition, 1, NULL, false, AwaResourceOperations_Execute));
    ASSERT_EQ(AwaError_DefinitionInvalid, AwaObjectDefinition_AddResourceDefinitionAsNoType(definition, 1, "Test Resource No Type", false, AwaResourceOperations_ReadWrite));

    AwaObjectDefinition_Free(&definition);
}

TEST_F(TestDefineCommon, AwaObjectDefinition_IsResourceDefined_valid_inputs)
{
    AwaObjectDefinition * definition = AwaObjectDefinition_New(detail::TEST_OBJECT_ID_VALID, "Test Object", 1, 1);
    EXPECT_TRUE(NULL != definition);

    EXPECT_EQ(0, AwaObjectDefinition_IsResourceDefined(definition, 1000));
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(definition, 1000, "Resource 1000", false, AwaResourceOperations_ReadWrite, 0));
    EXPECT_EQ(1, AwaObjectDefinition_IsResourceDefined(definition, 1000));

    AwaObjectDefinition_Free(&definition);
}

TEST_F(TestDefineCommon, AwaObjectDefinition_IsResourceDefined_invalid_inputs)
{
    EXPECT_EQ(0, AwaObjectDefinition_IsResourceDefined(NULL, 1000));
}

TEST_F(TestDefineCommon, AwaResourceDefinition_GetType_handles_invalid_inputs)
{
    ASSERT_EQ(AwaResourceType_Invalid, AwaResourceDefinition_GetType(NULL));
}

TEST_F(TestDefineCommon, AwaResourceDefinition_GetID_handles_invalid_inputs)
{
    ASSERT_EQ(AWA_INVALID_ID, AwaResourceDefinition_GetID(NULL));
}

TEST_F(TestDefineCommon, AwaResourceDefinition_GetSupportedOperations_handles_invalid_inputs)
{
    ASSERT_EQ(AwaResourceOperations_None, AwaResourceDefinition_GetSupportedOperations(NULL));
}

TEST_F(TestDefineCommon, AwaResourceDefinition_GetName_handles_invalid_inputs)
{
    ASSERT_TRUE(NULL == AwaResourceDefinition_GetName(NULL));
}

TEST_F(TestDefineCommon, AwaResourceDefinition_IsMandatory_handles_invalid_inputs)
{
    ASSERT_FALSE(AwaResourceDefinition_IsMandatory(NULL));
}

TEST_F(TestDefineCommon, AwaResourceDefinition_GetMaximumInstances_handles_invalid_inputs)
{
    ASSERT_EQ(-1, AwaResourceDefinition_GetMaximumInstances(NULL));
}

TEST_F(TestDefineCommon, AwaResourceDefinition_GetMinimumInstances_handles_invalid_inputs)
{
    ASSERT_EQ(-1, AwaResourceDefinition_GetMinimumInstances(NULL));
}


/***********************************************************************************************************
 * AddResourceDefinition parameterised tests
 */

namespace detail
{
struct DefineResource
{
    AwaError expectedResult;
    bool UseDefinition;
    AwaResourceID resourceID;
    const char * name;
    AwaResourceType type;
    bool isMandatory;
    AwaResourceOperations operation;
    void * DefaultValue;
};

::std::ostream& operator<<(::std::ostream& os, const DefineResource& item)
{
  return os << "Item: expectedResult " << item.expectedResult
            << ", UseDefinition " << item.UseDefinition
            << ", resourceID " << item.resourceID
            << ", name " << item.name
            << ", type " << item.type
            << ", isMandatory " << item.isMandatory
            << ", operation " << item.operation
            << ", DefaultValue" << item.DefaultValue;
}

AwaInteger dummyInteger = 0;
AwaFloat dummyFloat = 0.0;
AwaBoolean dummyBoolean = false;
AwaOpaque dummyOpaque = {0};
AwaTime dummyTime = 0;
AwaObjectLink dummyObjectLink = {0};

const char * dummyNonDefaultString = "abcdef";
AwaInteger dummyNonDefaultInteger = 12345;
AwaFloat dummyNonDefaultFloat = 123.45;
AwaBoolean dummyNonDefaultBoolean = false;
AwaBoolean dummyNonDefaultBoolean2 = true;

const char dummyData[] = {'a', '\0', 123};
AwaOpaque dummyNonDefaultOpaque = {(void*)dummyData, 3};
AwaTime dummyNonDefaultTime = 12345;
AwaObjectLink dummyNonDefaultObjectLink = {22,4};
}

class TestDefineCommonResource : public TestClientBase, public ::testing::WithParamInterface< detail::DefineResource >
{
protected:

    virtual void SetUp() {
        definition_ = AwaObjectDefinition_New(detail::TEST_TEMPORARY_OBJECT_ID, "Test Object", 1, 1);
        ASSERT_TRUE(NULL != definition_);
        //TODO: SetIPCAsUDP
    }

    virtual void TearDown() {
        AwaObjectDefinition_Free(&definition_);
    }

    AwaObjectDefinition * definition_;
};

class TestDefineCommonResource_AddResourceDefinition : public TestDefineCommonResource {};

TEST_P(TestDefineCommonResource_AddResourceDefinition, AddResourceDefinition)
{
    detail::DefineResource data = GetParam();
    AwaObjectDefinition * definition = data.UseDefinition ? this->definition_ : NULL;

    switch(data.type)
    {
        case AwaResourceType_String:
            ASSERT_EQ(data.expectedResult, AwaObjectDefinition_AddResourceDefinitionAsString(definition, data.resourceID, data.name, data.isMandatory, data.operation, (const char *)data.DefaultValue));
            break;
        case AwaResourceType_Integer:
            ASSERT_EQ(data.expectedResult, AwaObjectDefinition_AddResourceDefinitionAsInteger(definition, data.resourceID, data.name, data.isMandatory, data.operation, data.DefaultValue ? *((AwaInteger *)data.DefaultValue) : 0));
            break;
        case AwaResourceType_Float:
            ASSERT_EQ(data.expectedResult, AwaObjectDefinition_AddResourceDefinitionAsFloat(definition, data.resourceID, data.name, data.isMandatory, data.operation, data.DefaultValue ? *((AwaFloat *)data.DefaultValue) : 0.0));
            break;
        case AwaResourceType_Boolean:
            ASSERT_EQ(data.expectedResult, AwaObjectDefinition_AddResourceDefinitionAsBoolean(definition, data.resourceID, data.name, data.isMandatory, data.operation, data.DefaultValue ? *((AwaBoolean *)data.DefaultValue) : false));
            break;
        case AwaResourceType_Opaque:
            ASSERT_EQ(data.expectedResult, AwaObjectDefinition_AddResourceDefinitionAsOpaque(definition, data.resourceID, data.name, data.isMandatory, data.operation, data.DefaultValue ? *((AwaOpaque *)data.DefaultValue) : AwaOpaque {0}));
            break;
        case AwaResourceType_Time:
            ASSERT_EQ(data.expectedResult, AwaObjectDefinition_AddResourceDefinitionAsTime(definition, data.resourceID, data.name, data.isMandatory, data.operation, data.DefaultValue ? *((AwaTime *)data.DefaultValue) : 0));
            break;
        case AwaResourceType_ObjectLink:
            ASSERT_EQ(data.expectedResult, AwaObjectDefinition_AddResourceDefinitionAsObjectLink(definition, data.resourceID, data.name, data.isMandatory, data.operation, data.DefaultValue ? *((AwaObjectLink *)data.DefaultValue) : AwaObjectLink {0} ));
            break;

        case AwaResourceType_StringArray:
            EXPECT_EQ(data.expectedResult, AwaObjectDefinition_AddResourceDefinitionAsStringArray(definition, data.resourceID, data.name, data.isMandatory ? 1 : 0, detail::TEST_MULTIPLE_INSTANCES_MAX, data.operation, (AwaStringArray *)data.DefaultValue));
            break;
        case AwaResourceType_IntegerArray:
            EXPECT_EQ(data.expectedResult, AwaObjectDefinition_AddResourceDefinitionAsIntegerArray(definition, data.resourceID, data.name, data.isMandatory ? 1 : 0, detail::TEST_MULTIPLE_INSTANCES_MAX, data.operation, (AwaIntegerArray *)data.DefaultValue));
            break;
        case AwaResourceType_FloatArray:
            EXPECT_EQ(data.expectedResult, AwaObjectDefinition_AddResourceDefinitionAsFloatArray(definition, data.resourceID, data.name, data.isMandatory ? 1 : 0, detail::TEST_MULTIPLE_INSTANCES_MAX, data.operation, (AwaFloatArray *)data.DefaultValue));
            break;
        case AwaResourceType_BooleanArray:
            EXPECT_EQ(data.expectedResult, AwaObjectDefinition_AddResourceDefinitionAsBooleanArray(definition, data.resourceID, data.name, data.isMandatory ? 1 : 0, detail::TEST_MULTIPLE_INSTANCES_MAX, data.operation, (AwaBooleanArray *)data.DefaultValue));
            break;
        case AwaResourceType_OpaqueArray:
            EXPECT_EQ(data.expectedResult, AwaObjectDefinition_AddResourceDefinitionAsOpaqueArray(definition, data.resourceID, data.name, data.isMandatory ? 1 : 0, detail::TEST_MULTIPLE_INSTANCES_MAX, data.operation, (AwaOpaqueArray *)data.DefaultValue));
            break;
        case AwaResourceType_TimeArray:
            EXPECT_EQ(data.expectedResult, AwaObjectDefinition_AddResourceDefinitionAsTimeArray(definition, data.resourceID, data.name, data.isMandatory ? 1 : 0, detail::TEST_MULTIPLE_INSTANCES_MAX, data.operation, (AwaTimeArray *)data.DefaultValue));
            break;
        case AwaResourceType_ObjectLinkArray:
            EXPECT_EQ(data.expectedResult, AwaObjectDefinition_AddResourceDefinitionAsObjectLinkArray(definition, data.resourceID, data.name, data.isMandatory ? 1 : 0, detail::TEST_MULTIPLE_INSTANCES_MAX, data.operation, (AwaObjectLinkArray *)data.DefaultValue));
            break;

        default:
            ASSERT_TRUE(false);
    }

    EXPECT_TRUE(true);
}

INSTANTIATE_TEST_CASE_P(
        TestDefineCommonResource_AddResourceDefinition_Single,
        TestDefineCommonResource_AddResourceDefinition,
        ::testing::Values(
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_String,     true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_String,     false, AwaResourceOperations_ReadWrite, (void *)"test"},
          detail::DefineResource {AwaError_IDInvalid,         true,  -1, "Test Resource 1", AwaResourceType_String,     true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, false, 1,  "Test Resource 1", AwaResourceType_String,     true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  NULL,              AwaResourceType_String,     true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  "Test Resource 1", AwaResourceType_String,     true, AwaResourceOperations_Execute,   NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Integer,    true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Integer,    false, AwaResourceOperations_ReadWrite, (void *)(&detail::dummyInteger)},
          detail::DefineResource {AwaError_IDInvalid,         true,  -1, "Test Resource 1", AwaResourceType_Integer,    true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, false, 1,  "Test Resource 1", AwaResourceType_Integer,    true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  NULL,              AwaResourceType_Integer,    true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  "Test Resource 1", AwaResourceType_Integer,    true, AwaResourceOperations_Execute,   NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Float,      true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Float,      false, AwaResourceOperations_ReadWrite, (void *)(&detail::dummyFloat)},
          detail::DefineResource {AwaError_IDInvalid,         true,  -1, "Test Resource 1", AwaResourceType_Float,      true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, false, 1,  "Test Resource 1", AwaResourceType_Float,      true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  NULL,              AwaResourceType_Float,      true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  "Test Resource 1", AwaResourceType_Float,      true, AwaResourceOperations_Execute,   NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Boolean,    true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Boolean,    false, AwaResourceOperations_ReadWrite, (void *)(&detail::dummyBoolean)},
          detail::DefineResource {AwaError_IDInvalid,         true,  -1, "Test Resource 1", AwaResourceType_Boolean,    true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, false, 1,  "Test Resource 1", AwaResourceType_Boolean,    true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  NULL,              AwaResourceType_Boolean,    true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  "Test Resource 1", AwaResourceType_Boolean,    true, AwaResourceOperations_Execute,   NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Opaque,     true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Opaque,     false, AwaResourceOperations_ReadWrite, (void *)(&detail::dummyOpaque)},
          detail::DefineResource {AwaError_IDInvalid,         true,  -1, "Test Resource 1", AwaResourceType_Opaque,     true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, false, 1,  "Test Resource 1", AwaResourceType_Opaque,     true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  NULL,              AwaResourceType_Opaque,     true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  "Test Resource 1", AwaResourceType_Opaque,     true, AwaResourceOperations_Execute,   NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Time,       true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Time,       false, AwaResourceOperations_ReadWrite, (void *)(&detail::dummyTime)},
          detail::DefineResource {AwaError_IDInvalid,         true,  -1, "Test Resource 1", AwaResourceType_Time,       true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, false, 1,  "Test Resource 1", AwaResourceType_Time,       true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  NULL,              AwaResourceType_Time,       true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  "Test Resource 1", AwaResourceType_Time,       true, AwaResourceOperations_Execute,   NULL},


          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_ObjectLink, true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_ObjectLink, false, AwaResourceOperations_ReadWrite, (void *)(&detail::dummyObjectLink)},
          detail::DefineResource {AwaError_IDInvalid,         true,  -1, "Test Resource 1", AwaResourceType_ObjectLink, true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, false, 1,  "Test Resource 1", AwaResourceType_ObjectLink, true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  NULL,              AwaResourceType_ObjectLink, true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  "Test Resource 1", AwaResourceType_ObjectLink, true, AwaResourceOperations_Execute,   NULL}



        ));


INSTANTIATE_TEST_CASE_P(
        TestDefineCommonResource_AddResourceDefinition_Array,
        TestDefineCommonResource_AddResourceDefinition,
        ::testing::Values(

/* DISABLED - default array types */

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_StringArray,     true, AwaResourceOperations_ReadWrite, NULL},
          //detail::TestDefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_String,     false, AwaResourceOperations_ReadWrite, (void *)"test"},
          detail::DefineResource {AwaError_IDInvalid,         true,  -1, "Test Resource 1", AwaResourceType_StringArray,     true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, false, 1,  "Test Resource 1", AwaResourceType_StringArray,     true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  NULL,              AwaResourceType_StringArray,     true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  "Test Resource 1", AwaResourceType_StringArray,     true, AwaResourceOperations_Execute,   NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_IntegerArray,    true, AwaResourceOperations_ReadWrite, NULL},
          //detail::TestDefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Integer,    false, AwaResourceOperations_ReadWrite, (void *)(&detail::dummyInteger)},
          detail::DefineResource {AwaError_IDInvalid,         true,  -1, "Test Resource 1", AwaResourceType_IntegerArray,    true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, false, 1,  "Test Resource 1", AwaResourceType_IntegerArray,    true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  NULL,              AwaResourceType_IntegerArray,    true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  "Test Resource 1", AwaResourceType_IntegerArray,    true, AwaResourceOperations_Execute,   NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_FloatArray,      true, AwaResourceOperations_ReadWrite, NULL},
          //detail::TestDefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Float,      false, AwaResourceOperations_ReadWrite, (void *)(&detail::dummyFloat)},
          detail::DefineResource {AwaError_IDInvalid,         true,  -1, "Test Resource 1", AwaResourceType_FloatArray,      true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, false, 1,  "Test Resource 1", AwaResourceType_FloatArray,      true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  NULL,              AwaResourceType_FloatArray,      true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  "Test Resource 1", AwaResourceType_FloatArray,      true, AwaResourceOperations_Execute,   NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_BooleanArray,    true, AwaResourceOperations_ReadWrite, NULL},
          //detail::TestDefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Boolean,    false, AwaResourceOperations_ReadWrite, (void *)(&detail::dummyBoolean)},
          detail::DefineResource {AwaError_IDInvalid,         true,  -1, "Test Resource 1", AwaResourceType_BooleanArray,    true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, false, 1,  "Test Resource 1", AwaResourceType_BooleanArray,    true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  NULL,              AwaResourceType_BooleanArray,    true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  "Test Resource 1", AwaResourceType_BooleanArray,    true, AwaResourceOperations_Execute,   NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_OpaqueArray,     true, AwaResourceOperations_ReadWrite, NULL},
          //detail::TestDefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Opaque,     false, AwaResourceOperations_ReadWrite, (void *)(&detail::dummyOpaque)},
          detail::DefineResource {AwaError_IDInvalid,         true,  -1, "Test Resource 1", AwaResourceType_OpaqueArray,     true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, false, 1,  "Test Resource 1", AwaResourceType_OpaqueArray,     true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  NULL,              AwaResourceType_OpaqueArray,     true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  "Test Resource 1", AwaResourceType_OpaqueArray,     true, AwaResourceOperations_Execute,   NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_TimeArray,       true, AwaResourceOperations_ReadWrite, NULL},
          //detail::TestDefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Time,       false, AwaResourceOperations_ReadWrite, (void *)(&detail::dummyTime)},
          detail::DefineResource {AwaError_IDInvalid,         true,  -1, "Test Resource 1", AwaResourceType_TimeArray,       true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, false, 1,  "Test Resource 1", AwaResourceType_TimeArray,       true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  NULL,              AwaResourceType_TimeArray,       true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  "Test Resource 1", AwaResourceType_TimeArray,       true, AwaResourceOperations_Execute,   NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_ObjectLinkArray, true, AwaResourceOperations_ReadWrite, NULL},
          //detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_ObjectLinkArray, false, AwaResourceOperations_ReadWrite, (void *)(&detail::dummyObjectLink)},
          detail::DefineResource {AwaError_IDInvalid,         true,  -1, "Test Resource 1", AwaResourceType_ObjectLinkArray, true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, false, 1,  "Test Resource 1", AwaResourceType_ObjectLinkArray, true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  NULL,              AwaResourceType_ObjectLinkArray, true, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_DefinitionInvalid, true,  1,  "Test Resource 1", AwaResourceType_ObjectLinkArray, true, AwaResourceOperations_Execute,   NULL}


        ));

class TestDefineCommonResource_GetResourceDefinition : public TestDefineCommonResource {};

TEST_P(TestDefineCommonResource_GetResourceDefinition, GetResourceDefinition)
{
    detail::DefineResource data = GetParam();
    AwaObjectDefinition * definition = this->definition_;
    int Maximum = 1;

    switch(data.type)
    {
        case AwaResourceType_String:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsString(definition, data.resourceID, data.name, data.isMandatory, data.operation, NULL));
            break;
        case AwaResourceType_Integer:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(definition, data.resourceID, data.name, data.isMandatory, data.operation, 0));
            break;
        case AwaResourceType_Float:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloat(definition, data.resourceID, data.name, data.isMandatory, data.operation, 0.0));
            break;
        case AwaResourceType_Boolean:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBoolean(definition, data.resourceID, data.name, data.isMandatory, data.operation, false));
            break;
        case AwaResourceType_Opaque:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque(definition, data.resourceID, data.name, data.isMandatory, data.operation, AwaOpaque {0}));
            break;
        case AwaResourceType_Time:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTime(definition, data.resourceID, data.name, data.isMandatory, data.operation, 0));
            break;
        case AwaResourceType_ObjectLink:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLink(definition, data.resourceID, data.name, data.isMandatory, data.operation, AwaObjectLink {0} ));
            break;
        case AwaResourceType_StringArray:
            Maximum = detail::TEST_MULTIPLE_INSTANCES_MAX;
            EXPECT_EQ(data.expectedResult, AwaObjectDefinition_AddResourceDefinitionAsStringArray(definition, data.resourceID, data.name, data.isMandatory ? 1 : 0, Maximum, data.operation, (AwaStringArray *)data.DefaultValue));
            break;
        case AwaResourceType_IntegerArray:
            Maximum = detail::TEST_MULTIPLE_INSTANCES_MAX;
            EXPECT_EQ(data.expectedResult, AwaObjectDefinition_AddResourceDefinitionAsIntegerArray(definition, data.resourceID, data.name, data.isMandatory ? 1 : 0, Maximum, data.operation, (AwaIntegerArray *)data.DefaultValue));
            break;
        case AwaResourceType_FloatArray:
            Maximum = detail::TEST_MULTIPLE_INSTANCES_MAX;
            EXPECT_EQ(data.expectedResult, AwaObjectDefinition_AddResourceDefinitionAsFloatArray(definition, data.resourceID, data.name, data.isMandatory ? 1 : 0, Maximum, data.operation, (AwaFloatArray *)data.DefaultValue));
            break;
        case AwaResourceType_BooleanArray:
            Maximum = detail::TEST_MULTIPLE_INSTANCES_MAX;
            EXPECT_EQ(data.expectedResult, AwaObjectDefinition_AddResourceDefinitionAsBooleanArray(definition, data.resourceID, data.name, data.isMandatory ? 1 : 0, Maximum, data.operation, (AwaBooleanArray *)data.DefaultValue));
            break;
        case AwaResourceType_OpaqueArray:
            Maximum = detail::TEST_MULTIPLE_INSTANCES_MAX;
            EXPECT_EQ(data.expectedResult, AwaObjectDefinition_AddResourceDefinitionAsOpaqueArray(definition, data.resourceID, data.name, data.isMandatory ? 1 : 0, Maximum, data.operation, (AwaOpaqueArray *)data.DefaultValue));
            break;
        case AwaResourceType_TimeArray:
            Maximum = detail::TEST_MULTIPLE_INSTANCES_MAX;
            EXPECT_EQ(data.expectedResult, AwaObjectDefinition_AddResourceDefinitionAsTimeArray(definition, data.resourceID, data.name, data.isMandatory ? 1 : 0, Maximum, data.operation, (AwaTimeArray *)data.DefaultValue));
            break;
        case AwaResourceType_ObjectLinkArray:
            Maximum = detail::TEST_MULTIPLE_INSTANCES_MAX;
            EXPECT_EQ(data.expectedResult, AwaObjectDefinition_AddResourceDefinitionAsObjectLinkArray(definition, data.resourceID, data.name, data.isMandatory ? 1 : 0, Maximum, data.operation, (AwaObjectLinkArray *)data.DefaultValue));
            break;

        default:
            ASSERT_TRUE(false);
    }

    const AwaResourceDefinition * resourceDefinition = AwaObjectDefinition_GetResourceDefinition(definition, data.resourceID);
    ASSERT_TRUE(NULL != resourceDefinition);

    ASSERT_EQ(data.resourceID, AwaResourceDefinition_GetID(resourceDefinition));
    ASSERT_EQ(data.type, AwaResourceDefinition_GetType(resourceDefinition));
    ASSERT_STREQ(data.name, AwaResourceDefinition_GetName(resourceDefinition));

    int Minimum = data.isMandatory ? 1 : 0;
    ASSERT_EQ(Minimum, AwaResourceDefinition_GetMinimumInstances(resourceDefinition));
    ASSERT_EQ(Maximum, AwaResourceDefinition_GetMaximumInstances(resourceDefinition));

    ASSERT_EQ(data.operation, AwaResourceDefinition_GetSupportedOperations(resourceDefinition));
    ASSERT_EQ(data.isMandatory, AwaResourceDefinition_IsMandatory(resourceDefinition));
}

INSTANTIATE_TEST_CASE_P(
        TestDefineCommonResource_GetResourceDefinition_Single,
        TestDefineCommonResource_GetResourceDefinition,
        ::testing::Values(
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_String,     true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_String,     false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Integer,    true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Integer,    false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Float,      true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Float,      false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Boolean,    true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Boolean,    false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Opaque,     true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Opaque,     false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Time,       true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Time,       false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_ObjectLink, true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_ObjectLink, false, AwaResourceOperations_ReadWrite, NULL}

        ));


INSTANTIATE_TEST_CASE_P(
        TestDefineCommonResource_GetResourceDefinition_Array,
        TestDefineCommonResource_GetResourceDefinition,
        ::testing::Values(
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_StringArray,     true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_StringArray,     false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_IntegerArray,    true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_IntegerArray,    false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_FloatArray,      true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_FloatArray,      false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_BooleanArray,    true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_BooleanArray,    false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_OpaqueArray,     true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_OpaqueArray,     false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_TimeArray,       true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_TimeArray,       false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_ObjectLink, true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_ObjectLink, false, AwaResourceOperations_ReadWrite, NULL}

        ));

class TestDefineCommonResource_ResourceDefinitionIterator : public TestDefineCommonResource {};

TEST_F(TestDefineCommonResource_ResourceDefinitionIterator, AwaResourceDefinitionIterator_New_and_Free_valid_inputs)
{
    AwaResourceDefinitionIterator * iterator = AwaObjectDefinition_NewResourceDefinitionIterator(this->definition_);
    ASSERT_TRUE(NULL != iterator);
    AwaResourceDefinitionIterator_Free(&iterator);
    ASSERT_TRUE(NULL == iterator);
}

TEST_F(TestDefineCommonResource_ResourceDefinitionIterator, AwaResourceDefinitionIterator_New_Free_invalid_inputs)
{
    AwaResourceDefinitionIterator * iterator = AwaObjectDefinition_NewResourceDefinitionIterator(NULL);
    ASSERT_TRUE(NULL == iterator);
    AwaResourceDefinitionIterator_Free(NULL);
    ASSERT_TRUE(NULL == iterator);
}

TEST_F(TestDefineCommonResource_ResourceDefinitionIterator, AwaResourceDefinitionIterator_Can_iterate_valid_inputs)
{
    AwaObjectDefinition * definition = this->definition_;

    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(definition, 1000, "Resource 1000", false, AwaResourceOperations_ReadWrite, 0));
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(definition, 1001, "Resource 1001", false, AwaResourceOperations_ReadWrite, 0));
    EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(definition, 1002, "Resource 1002", false, AwaResourceOperations_ReadWrite, 0));
         
    AwaResourceDefinitionIterator * iterator = AwaObjectDefinition_NewResourceDefinitionIterator(definition);
    EXPECT_TRUE(NULL != iterator);

    const AwaResourceDefinition * resourceDefinition;

    ASSERT_EQ(true, AwaResourceDefinitionIterator_Next(iterator));
    resourceDefinition = AwaResourceDefinitionIterator_Get(iterator);
    EXPECT_TRUE(NULL != resourceDefinition);
    ASSERT_EQ(1000, AwaResourceDefinition_GetID(resourceDefinition));

    ASSERT_EQ(true, AwaResourceDefinitionIterator_Next(iterator));
    resourceDefinition = AwaResourceDefinitionIterator_Get(iterator);
    EXPECT_TRUE(NULL != resourceDefinition);
    ASSERT_EQ(1001, AwaResourceDefinition_GetID(resourceDefinition));

    ASSERT_EQ(true, AwaResourceDefinitionIterator_Next(iterator));
    resourceDefinition = AwaResourceDefinitionIterator_Get(iterator);
    EXPECT_TRUE(NULL != resourceDefinition);
    ASSERT_EQ(1002, AwaResourceDefinition_GetID(resourceDefinition));

    ASSERT_EQ(0, AwaResourceDefinitionIterator_Next(iterator));
    resourceDefinition = AwaResourceDefinitionIterator_Get(iterator);
    EXPECT_TRUE(NULL == resourceDefinition);

    AwaResourceDefinitionIterator_Free(&iterator);
    EXPECT_TRUE(NULL == iterator);
}

TEST_F(TestDefineCommonResource_ResourceDefinitionIterator, AwaResourceDefinitionIterator_Check_invalid_inputs)
{
     ASSERT_EQ(0, AwaResourceDefinitionIterator_Next(NULL));
     ASSERT_EQ(NULL, AwaResourceDefinitionIterator_Get(NULL));
}

/*
 * End of AddResourceDefinition test
 ***********************************************************************************************************/

/***********************************************************************************************************
 * AddResourceDefinition with IPC parameterised tests
 */

class TestDefineCommonResource_WithIPC : public TestClientWithConnectedSession, public ::testing::WithParamInterface< detail::DefineResource >
{
protected:
    virtual void SetUp() {
        TestClientWithConnectedSession::SetUp();
        definition_ = AwaObjectDefinition_New(detail::TEST_TEMPORARY_OBJECT_ID, "Test Object", 1, 1);
        ASSERT_TRUE(NULL != definition_);
    }

    virtual void TearDown() {
        AwaObjectDefinition_Free(&definition_);
        TestClientWithConnectedSession::TearDown();
    }

    AwaObjectDefinition * definition_;
};

class TestDefineCommonResource_ProcessDefineOperation : public TestDefineCommonResource_WithIPC {};

TEST_P(TestDefineCommonResource_ProcessDefineOperation, TestDefineCommonResource_ProcessDefineOperation)
{
    detail::DefineResource data = GetParam();
    AwaObjectDefinition * definition = this->definition_;

    switch(data.type)
    {
        case AwaResourceType_String:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsString(definition, data.resourceID, data.name, data.isMandatory, data.operation, NULL));
            break;
        case AwaResourceType_Integer:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(definition, data.resourceID, data.name, data.isMandatory, data.operation, 1234567));
            break;
        case AwaResourceType_Float:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloat(definition, data.resourceID, data.name, data.isMandatory, data.operation, 0.0));
            break;
        case AwaResourceType_Boolean:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBoolean(definition, data.resourceID, data.name, data.isMandatory, data.operation, false));
            break;
        case AwaResourceType_Opaque:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque(definition, data.resourceID, data.name, data.isMandatory, data.operation, AwaOpaque {0}));
            break;
        case AwaResourceType_Time:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTime(definition, data.resourceID, data.name, data.isMandatory, data.operation, 0));
            break;
        case AwaResourceType_ObjectLink:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLink(definition, data.resourceID, data.name, data.isMandatory, data.operation, AwaObjectLink {0} ));
            break;

        default:
            ASSERT_TRUE(false);
    }

    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, definition));

    ASSERT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, global::timeout));

    AwaClientDefineOperation_Free(&defineOperation);
}

INSTANTIATE_TEST_CASE_P(
        TestDefineCommonResource_ProcessDefineOperation,
        TestDefineCommonResource_ProcessDefineOperation,
        ::testing::Values(
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_String,     true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_String,     false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Integer,    true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Integer,    false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Float,      true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Float,      false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Boolean,    true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Boolean,    false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Opaque,     true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Opaque,     false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Time,       true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Time,       false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_ObjectLink, true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_ObjectLink, false, AwaResourceOperations_ReadWrite, NULL}
        ));

class TestDefineCommonResource_Definition_persists_in_daemon_after_disconnect_connect : public TestDefineCommonResource_WithIPC {};

TEST_P(TestDefineCommonResource_Definition_persists_in_daemon_after_disconnect_connect, TestDefineCommonResource_Definition_persists_in_daemon_after_disconnect_connect)
{
    detail::DefineResource data = GetParam();
    AwaObjectDefinition * definition = this->definition_;
    int Maximum = 1;

    switch(data.type)
    {
        case AwaResourceType_String:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsString(definition, data.resourceID, data.name, data.isMandatory, data.operation, data.DefaultValue != NULL? (const char*)data.DefaultValue : NULL));
            break;
        case AwaResourceType_Integer:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsInteger(definition, data.resourceID, data.name, data.isMandatory, data.operation, data.DefaultValue != NULL? *((AwaInteger*)data.DefaultValue) : 0));
            break;
        case AwaResourceType_Float:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloat(definition, data.resourceID, data.name, data.isMandatory, data.operation, data.DefaultValue != NULL? *((AwaFloat*)data.DefaultValue) : 0));
            break;
        case AwaResourceType_Boolean:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBoolean(definition, data.resourceID, data.name, data.isMandatory, data.operation, data.DefaultValue != NULL? *((AwaBoolean*)data.DefaultValue) : false));
            break;
        case AwaResourceType_Opaque:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaque(definition, data.resourceID, data.name, data.isMandatory, data.operation, data.DefaultValue != NULL? *((AwaOpaque*)data.DefaultValue) : AwaOpaque {0}));
            break;
        case AwaResourceType_Time:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTime(definition, data.resourceID, data.name, data.isMandatory, data.operation, data.DefaultValue != NULL? *((AwaTime*)data.DefaultValue) : 0));
            break;
        case AwaResourceType_ObjectLink:
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLink(definition, data.resourceID, data.name, data.isMandatory, data.operation, data.DefaultValue != NULL? *((AwaObjectLink*)data.DefaultValue) : AwaObjectLink {0} ));
            break;

        case AwaResourceType_StringArray:
            Maximum = detail::TEST_MULTIPLE_INSTANCES_MAX;
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsStringArray(definition, data.resourceID, data.name, data.isMandatory ? 1 : 0, Maximum, data.operation, NULL));
            break;
        case AwaResourceType_IntegerArray:
            Maximum = detail::TEST_MULTIPLE_INSTANCES_MAX;
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsIntegerArray(definition, data.resourceID, data.name, data.isMandatory ? 1 : 0, Maximum, data.operation, NULL));
            break;
        case AwaResourceType_FloatArray:
            Maximum = detail::TEST_MULTIPLE_INSTANCES_MAX;
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsFloatArray(definition, data.resourceID, data.name, data.isMandatory ? 1 : 0, Maximum, data.operation, NULL));
            break;
        case AwaResourceType_BooleanArray:
            Maximum = detail::TEST_MULTIPLE_INSTANCES_MAX;
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsBooleanArray(definition, data.resourceID, data.name, data.isMandatory ? 1 : 0, Maximum, data.operation, NULL));
            break;
        case AwaResourceType_OpaqueArray:
            Maximum = detail::TEST_MULTIPLE_INSTANCES_MAX;
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsOpaqueArray(definition, data.resourceID, data.name, data.isMandatory ? 1 : 0, Maximum, data.operation, NULL));
            break;
        case AwaResourceType_TimeArray:
            Maximum = detail::TEST_MULTIPLE_INSTANCES_MAX;
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsTimeArray(definition, data.resourceID, data.name, data.isMandatory ? 1 : 0, Maximum, data.operation, NULL));
            break;
        case AwaResourceType_ObjectLinkArray:
            Maximum = detail::TEST_MULTIPLE_INSTANCES_MAX;
            EXPECT_EQ(AwaError_Success, AwaObjectDefinition_AddResourceDefinitionAsObjectLinkArray(definition, data.resourceID, data.name, data.isMandatory ? 1 : 0, Maximum, data.operation, NULL));
            break;

        default:
            ASSERT_TRUE(false);
    }

    AwaClientDefineOperation * defineOperation = AwaClientDefineOperation_New(session_);
    EXPECT_TRUE(defineOperation != NULL);

    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Add(defineOperation, definition));

    EXPECT_EQ(AwaError_Success, AwaClientDefineOperation_Perform(defineOperation, global::timeout));

    AwaClientDefineOperation_Free(&defineOperation);

    AwaClientSession_Disconnect(session_);

    EXPECT_EQ(AwaError_Success, AwaClientSession_Connect(session_));

    const AwaObjectDefinition * newdefinition = AwaClientSession_GetObjectDefinition(session_, AwaObjectDefinition_GetID(definition));
    ASSERT_TRUE(NULL != newdefinition);

    const AwaResourceDefinition * resourceDefinition = AwaObjectDefinition_GetResourceDefinition(newdefinition, data.resourceID);
    ASSERT_TRUE(NULL != resourceDefinition);

    ASSERT_EQ(data.resourceID, AwaResourceDefinition_GetID(resourceDefinition));
    ASSERT_EQ(data.type, AwaResourceDefinition_GetType(resourceDefinition));
    ASSERT_STREQ(data.name, AwaResourceDefinition_GetName(resourceDefinition));

    int Minimum = data.isMandatory ? 1 : 0;
    ASSERT_EQ(Minimum, AwaResourceDefinition_GetMinimumInstances(resourceDefinition));
    ASSERT_EQ(Maximum, AwaResourceDefinition_GetMaximumInstances(resourceDefinition));

    ASSERT_EQ(data.operation, AwaResourceDefinition_GetSupportedOperations(resourceDefinition));
    ASSERT_EQ(data.isMandatory, AwaResourceDefinition_IsMandatory(resourceDefinition));

    if (data.DefaultValue != NULL)
    {
        Lwm2mTreeNode * child = Lwm2mTreeNode_GetFirstChild(resourceDefinition->DefaultValueNode);
        ASSERT_TRUE(NULL != child);

        uint16_t valueLength = 0;
        const uint8_t * value = Lwm2mTreeNode_GetValue(child, &valueLength);
        ASSERT_TRUE(NULL != value);

        switch (data.type)
        {
        case AwaResourceType_String:
            ASSERT_EQ(0, memcmp((const char*)data.DefaultValue, (const char *)value, strlen((const char*)data.DefaultValue)));
            break;
        case AwaResourceType_Integer:
            ASSERT_EQ(*((AwaInteger*)data.DefaultValue), *((AwaInteger*)value));
            break;
        case AwaResourceType_Float:
            ASSERT_EQ(*((AwaFloat*)data.DefaultValue), *((AwaFloat*)value));
            break;
        case AwaResourceType_Boolean:
            ASSERT_EQ(*((AwaBoolean*)data.DefaultValue), *((AwaBoolean*)value));
            break;
        case AwaResourceType_Opaque:
            ASSERT_EQ(((AwaOpaque*)data.DefaultValue)->Size, valueLength);
            ASSERT_EQ(0, memcmp(((AwaOpaque*)data.DefaultValue)->Data, value, ((AwaOpaque*)data.DefaultValue)->Size));
            break;
        case AwaResourceType_Time:
            ASSERT_EQ(*((AwaTime*)data.DefaultValue), *((AwaTime*)value));
            break;
        case AwaResourceType_ObjectLink:
            ASSERT_EQ(((AwaObjectLink*)data.DefaultValue)->ObjectID, ((AwaObjectLink*)value)->ObjectID);
            ASSERT_EQ(((AwaObjectLink*)data.DefaultValue)->ObjectInstanceID, ((AwaObjectLink*)value)->ObjectInstanceID);
            break;
        default:
            ASSERT_TRUE(false); // TODO array types
            break;
        }

    }

}

INSTANTIATE_TEST_CASE_P(
        TestDefineCommonResource_Definition_persists_in_daemon_after_disconnect_connect,
        TestDefineCommonResource_Definition_persists_in_daemon_after_disconnect_connect,
        ::testing::Values(
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_String,     true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_String,     false, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_String,     false, AwaResourceOperations_ReadWrite, (void*)detail::dummyNonDefaultString},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Integer,    true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Integer,    false, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Integer,    false, AwaResourceOperations_ReadWrite, (void*)&detail::dummyNonDefaultInteger},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Float,      true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Float,      false, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Float,      true,  AwaResourceOperations_ReadWrite, (void*)&detail::dummyNonDefaultFloat},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Boolean,    true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Boolean,    false, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Boolean,    true,  AwaResourceOperations_ReadWrite, (void*)&detail::dummyNonDefaultBoolean},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Boolean,    true,  AwaResourceOperations_ReadWrite, (void*)&detail::dummyNonDefaultBoolean2},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Opaque,     true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Opaque,     false, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Opaque,     false, AwaResourceOperations_ReadWrite, (void*)&detail::dummyNonDefaultOpaque},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Time,       true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Time,       false, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_Time,       false, AwaResourceOperations_ReadWrite, (void*)&detail::dummyNonDefaultTime},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_ObjectLink, true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_ObjectLink, false, AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_ObjectLink, false, AwaResourceOperations_ReadWrite, (void*)&detail::dummyNonDefaultObjectLink},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_StringArray,     true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_StringArray,     false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_IntegerArray,    true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_IntegerArray,    false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_FloatArray,      true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_FloatArray,      false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_BooleanArray,    true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_BooleanArray,    false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_OpaqueArray,     true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_OpaqueArray,     false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_TimeArray,       true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_TimeArray,       false, AwaResourceOperations_ReadWrite, NULL},

          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_ObjectLinkArray, true,  AwaResourceOperations_ReadWrite, NULL},
          detail::DefineResource {AwaError_Success,           true,  1,  "Test Resource 1", AwaResourceType_ObjectLinkArray, false, AwaResourceOperations_ReadWrite, NULL}
        ));


/*
 * End of AddResourceDefinition with IPC test
 ***********************************************************************************************************/

} // namespace Awa

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
#include <string>
#include <stdio.h>
#include <stdint.h>

// https://meekrosoft.wordpress.com/2009/11/09/unit-testing-c-code-with-the-googletest-framework/
// 1. Define fake functions for the dependencies you want to stub out
// 2. If the module depends on a global (gasp!) you need to define your fake one
// 3. include your module implementation (#include module.c)
// 4. Define a method to reset all the static data to a known state.
// 5. Define your tests

#include "common/lwm2m_object_store.h"
#include "common/lwm2m_prettyprint.c"
#include "common/lwm2m_tree_node.h"
#include "common/lwm2m_tree_builder.h"
#include "lwm2m_core.h"
#include "common/lwm2m_request_origin.h"

class PrettyPrintTestSuite : public testing::Test
{
    void SetUp() { context = Lwm2mCore_Init(NULL, NULL); }
    void TearDown() { Lwm2mCore_Destroy(context); }

protected:
    Lwm2mContextType * context;
};

TEST_F(PrettyPrintTestSuite, test_serialise_string)
{
    Lwm2m_SetLogLevel(DebugLevel_Debug);

    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 0, 1, 0, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 0, 0, AwaResourceType_String, 1, 1, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 0, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 0, 0, 0, 0, (char*)"Open Mobile Alliance", strlen("Open Mobile Alliance"));

    uint8_t buffer[512];
    memset(buffer, 0, 512);

    const char * expected = "Test[0/0]:\n"
    "\tRes1[0/0/0/0]: Open Mobile Alliance\n";

    Lwm2mTreeNode * dest;
    int OIR[] = {0};
    TreeBuilder_CreateTreeFromOIR(&dest, context, Lwm2mRequestOrigin_Client, OIR, 1);

    SerdesContext serdesContext;
    int len = PPSerialiseObject(&serdesContext, dest, 0, (uint8_t * )buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    EXPECT_EQ(static_cast<int>(strlen(expected)), len);
    EXPECT_EQ(0, memcmp(buffer, expected, strlen(expected)));
}

TEST_F(PrettyPrintTestSuite, test_serialise_string_multiple)
{
    Lwm2m_SetLogLevel(DebugLevel_Debug);

    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 0, 1, 0, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 0, 0, AwaResourceType_String, 1, 1, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res2", 0, 1, AwaResourceType_String, 1, 1, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 0, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 0, 0, 0, 0, (char*)"Open Mobile Alliance", strlen("Open Mobile Alliance"));
    Lwm2mCore_SetResourceInstanceValue(context, 0, 0, 1, 0, (char*)"Lightweight M2M Client", strlen("Lightweight M2M Client"));

    uint8_t buffer[512];
    memset(buffer, 0, 512);

    const char * expected = "Test[0/0]:\n"
    "\tRes1[0/0/0/0]: Open Mobile Alliance\n"
    "\tRes2[0/0/1/0]: Lightweight M2M Client\n";

    Lwm2mTreeNode * dest;
    int OIR[] = {0};
    TreeBuilder_CreateTreeFromOIR(&dest, context, Lwm2mRequestOrigin_Client, OIR, 1);

    SerdesContext serdesContext;
    int len = PPSerialiseObject(&serdesContext, dest, 0, (uint8_t * )buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    EXPECT_EQ(static_cast<int>(strlen(expected)), len);
    EXPECT_EQ(0, memcmp(buffer, expected, strlen(expected)));
}

TEST_F(PrettyPrintTestSuite, test_multiple_object_instance)
{
    int64_t temp = 44;
    int64_t temp2 = 55;

    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 15, 2, 0, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 15, 0, AwaResourceType_Integer, 1, 1, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);

    Lwm2mCore_CreateObjectInstance(context, 15, 1);
    Lwm2mCore_CreateObjectInstance(context, 15, 0);

    Lwm2mCore_SetResourceInstanceValue(context, 15, 1, 0, 0, &temp2, sizeof(temp2));
    Lwm2mCore_SetResourceInstanceValue(context, 15, 0, 0, 0, &temp, sizeof(temp));

    Lwm2mTreeNode * dest;
    int OIR[] = {15};
    TreeBuilder_CreateTreeFromOIR(&dest, context, Lwm2mRequestOrigin_Client, OIR, 1);

    uint8_t buffer[512];
    memset(buffer, 0, sizeof(buffer));
    char expected[] =
"Test[15/1]:\n"
"\tRes1[15/1/0/0]: 55\n"
"Test[15/0]:\n"
"\tRes1[15/0/0/0]: 44\n";
    SerdesContext serdesContext;
    int len = PPSerialiseObject(&serdesContext, dest, 15, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    ASSERT_GE(len, 0);
    ASSERT_EQ(strlen(expected), static_cast<size_t>(len));
    ASSERT_EQ(0, memcmp(buffer, expected, len));
}

//TODO: Test other resource types (int, boolean, float etc)

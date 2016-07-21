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

#include "common/lwm2m_tlv.c"
#include "common/lwm2m_tree_node.h"
#include "common/lwm2m_tree_builder.h"
#include "client/lwm2m_core.h"
#include "common/lwm2m_request_origin.h"
#include "common/lwm2m_objects.h"
#include "lwm2m_device_object.h"

class TlvTestSuite : public testing::Test
{
    void SetUp() { context = Lwm2mCore_Init(NULL, NULL); }
    void TearDown() { Lwm2mCore_Destroy(context); }

protected:
    void test_with_without_object_instance_header_common(const uint8_t * input, int inputSize);
    Lwm2mContextType * context;
};

void TlvTestSuite::test_with_without_object_instance_header_common(const uint8_t * input, int inputSize)
{
    int objectID = 3;
    int objectInstanceID = 0;

    Lwm2m_RegisterDeviceObject(context);

    Lwm2mTreeNode * dest;
    SerdesContext serdesContext;
    int len = TlvDeserialiseObjectInstance(&serdesContext, &dest, Lwm2mCore_GetDefinitions(context), objectID, objectInstanceID, input, inputSize);
    EXPECT_EQ(static_cast<int>(inputSize), len);
    EXPECT_EQ(19, Lwm2mTreeNode_GetChildCount(dest));
    Lwm2mTreeNode * child = Lwm2mTreeNode_GetFirstChild(dest);
    while (child)
    {
        int childID;
        Lwm2mTreeNode_GetID(child, &childID);
        Lwm2mTreeNode * resourceInstance = Lwm2mTreeNode_GetFirstChild(child);
        uint16_t valueLength;
        const uint8_t * value = Lwm2mTreeNode_GetValue(resourceInstance, &valueLength);
        switch (childID)
        {
        case 0:
            EXPECT_EQ(0, memcmp("Imagination Technologies", (const char *) value, strlen("Imagination Technologies")));
            break;
        case 1:
            EXPECT_EQ(0, memcmp("FlowM2M Client", (const char *) value, strlen("FlowM2M Client")));
            break;
        case 13:
            EXPECT_EQ(0xA20AD72B, *((const uint64_t*)value));
            break;
        default:
            // assume the rest are OK
            EXPECT_GE(childID, 0);
            EXPECT_LE(childID, 21);
            break;
        }

        child = Lwm2mTreeNode_GetNextChild(dest, child);
    }

    Lwm2mTreeNode_DeleteRecursive(dest);
}

TEST_F(TlvTestSuite, test_deserialise_single_object_instance_with_unnecessary_object_instance_header)
{
    //case where we do say, GET /3/0. The object instance ID is in the path, so the object instance header is not required.
    //However, some clients may do this and we should support it.

    const uint8_t input[] = {0x08, 0x00, 0xBC, 0x83, 0x0B, 0x41, 0x00, 0x00, 0xC1, 0x10, 0x55, 0xC8, 0x00, 0x18, 0x49, 0x6D, 0x61, 0x67, 0x69,
                             0x6E, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x54, 0x65, 0x63, 0x68, 0x6E, 0x6F, 0x6C, 0x6F, 0x67, 0x69, 0x65, 0x73,
                             0xC8, 0x01, 0x0E, 0x46, 0x6C, 0x6F, 0x77, 0x4D, 0x32, 0x4D, 0x20, 0x43, 0x6C, 0x69, 0x65, 0x6E, 0x74, 0xC8, 0x02,
                             0x0A, 0x53, 0x4E, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0xC4, 0x03, 0x30, 0x2E, 0x31, 0x61, 0x86, 0x06,
                             0x41, 0x00, 0x01, 0x41, 0x01, 0x05, 0x88, 0x07, 0x08, 0x42, 0x00, 0x0E, 0xD8, 0x42, 0x01, 0x13, 0x88, 0x87, 0x08,
                             0x41, 0x00, 0x7D, 0x42, 0x01, 0x03, 0x84, 0xC1, 0x09, 0x64, 0xC1, 0x0A, 0x0F, 0xC8, 0x0D, 0x08, 0x00, 0x00, 0x00,
                             0x00, 0xA2, 0x0A, 0xD7, 0x2B, 0xC6, 0x0E, 0x2B, 0x31, 0x32, 0x3A, 0x30, 0x30, 0xC8, 0x0F, 0x12, 0x50, 0x61, 0x63,
                             0x69, 0x66, 0x69, 0x63, 0x2F, 0x57, 0x65, 0x6C, 0x6C, 0x69, 0x6E, 0x67, 0x74, 0x6F, 0x6E, 0xC8, 0x11, 0x0E, 0x46,
                             0x6C, 0x6F, 0x77, 0x4D, 0x32, 0x4D, 0x20, 0x43, 0x6C, 0x69, 0x65, 0x6E, 0x74, 0xC7, 0x12, 0x30, 0x2E, 0x30, 0x2E,
                             0x30, 0x2E, 0x31, 0xC8, 0x13, 0x08, 0x30, 0x2E, 0x30, 0x2E, 0x30, 0x2E, 0x31, 0x31, 0xC1, 0x14, 0x02, 0xC1, 0x15, 0x2A};

    test_with_without_object_instance_header_common(input, sizeof(input));
}

TEST_F(TlvTestSuite, test_deserialise_single_object_instance_with_unnecessary_object_instance_header_wrong_object_instance_id_in_path)
{
    //Test case where a client serialised resources with a single object instance header,
    // and the object instance ID in the header does not match the object instance id in the path

    const uint8_t input[] = {0x08, 0x00, 0xBC, 0x83, 0x0B, 0x41, 0x00, 0x00, 0xC1, 0x10, 0x55, 0xC8, 0x00, 0x18, 0x49, 0x6D, 0x61, 0x67, 0x69,
                             0x6E, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x54, 0x65, 0x63, 0x68, 0x6E, 0x6F, 0x6C, 0x6F, 0x67, 0x69, 0x65, 0x73,
                             0xC8, 0x01, 0x0E, 0x46, 0x6C, 0x6F, 0x77, 0x4D, 0x32, 0x4D, 0x20, 0x43, 0x6C, 0x69, 0x65, 0x6E, 0x74, 0xC8, 0x02,
                             0x0A, 0x53, 0x4E, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0xC4, 0x03, 0x30, 0x2E, 0x31, 0x61, 0x86, 0x06,
                             0x41, 0x00, 0x01, 0x41, 0x01, 0x05, 0x88, 0x07, 0x08, 0x42, 0x00, 0x0E, 0xD8, 0x42, 0x01, 0x13, 0x88, 0x87, 0x08,
                             0x41, 0x00, 0x7D, 0x42, 0x01, 0x03, 0x84, 0xC1, 0x09, 0x64, 0xC1, 0x0A, 0x0F, 0xC8, 0x0D, 0x08, 0x00, 0x00, 0x00,
                             0x00, 0xA2, 0x0A, 0xD7, 0x2B, 0xC6, 0x0E, 0x2B, 0x31, 0x32, 0x3A, 0x30, 0x30, 0xC8, 0x0F, 0x12, 0x50, 0x61, 0x63,
                             0x69, 0x66, 0x69, 0x63, 0x2F, 0x57, 0x65, 0x6C, 0x6C, 0x69, 0x6E, 0x67, 0x74, 0x6F, 0x6E, 0xC8, 0x11, 0x0E, 0x46,
                             0x6C, 0x6F, 0x77, 0x4D, 0x32, 0x4D, 0x20, 0x43, 0x6C, 0x69, 0x65, 0x6E, 0x74, 0xC7, 0x12, 0x30, 0x2E, 0x30, 0x2E,
                             0x30, 0x2E, 0x31, 0xC8, 0x13, 0x08, 0x30, 0x2E, 0x30, 0x2E, 0x30, 0x2E, 0x31, 0x31, 0xC1, 0x14, 0x02, 0xC1, 0x15, 0x2A};

    int objectID = 3;
    int objectInstanceID = 1;

    Lwm2m_RegisterDeviceObject(context);

    Lwm2mTreeNode * dest; 
    SerdesContext serdesContext;
    int len = TlvDeserialiseObjectInstance(&serdesContext, &dest, Lwm2mCore_GetDefinitions(context), objectID, objectInstanceID, input, sizeof(input));
    EXPECT_EQ(-1, len);
    Lwm2mTreeNode_DeleteRecursive(dest);
}

TEST_F(TlvTestSuite, test_deserialise_single_object_instance_without_unnecessary_object_instance_header)
{
    //case where we do say, GET /3/0. The object instance ID is in the path, so the object instance header is not required.

    const uint8_t input[] = {0x83, 0x0B, 0x41, 0x00, 0x00, 0xC1, 0x10, 0x55, 0xC8, 0x00, 0x18, 0x49, 0x6D, 0x61, 0x67, 0x69,
                             0x6E, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x54, 0x65, 0x63, 0x68, 0x6E, 0x6F, 0x6C, 0x6F, 0x67, 0x69, 0x65, 0x73,
                             0xC8, 0x01, 0x0E, 0x46, 0x6C, 0x6F, 0x77, 0x4D, 0x32, 0x4D, 0x20, 0x43, 0x6C, 0x69, 0x65, 0x6E, 0x74, 0xC8, 0x02,
                             0x0A, 0x53, 0x4E, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0xC4, 0x03, 0x30, 0x2E, 0x31, 0x61, 0x86, 0x06,
                             0x41, 0x00, 0x01, 0x41, 0x01, 0x05, 0x88, 0x07, 0x08, 0x42, 0x00, 0x0E, 0xD8, 0x42, 0x01, 0x13, 0x88, 0x87, 0x08,
                             0x41, 0x00, 0x7D, 0x42, 0x01, 0x03, 0x84, 0xC1, 0x09, 0x64, 0xC1, 0x0A, 0x0F, 0xC8, 0x0D, 0x08, 0x00, 0x00, 0x00,
                             0x00, 0xA2, 0x0A, 0xD7, 0x2B, 0xC6, 0x0E, 0x2B, 0x31, 0x32, 0x3A, 0x30, 0x30, 0xC8, 0x0F, 0x12, 0x50, 0x61, 0x63,
                             0x69, 0x66, 0x69, 0x63, 0x2F, 0x57, 0x65, 0x6C, 0x6C, 0x69, 0x6E, 0x67, 0x74, 0x6F, 0x6E, 0xC8, 0x11, 0x0E, 0x46,
                             0x6C, 0x6F, 0x77, 0x4D, 0x32, 0x4D, 0x20, 0x43, 0x6C, 0x69, 0x65, 0x6E, 0x74, 0xC7, 0x12, 0x30, 0x2E, 0x30, 0x2E,
                             0x30, 0x2E, 0x31, 0xC8, 0x13, 0x08, 0x30, 0x2E, 0x30, 0x2E, 0x30, 0x2E, 0x31, 0x31, 0xC1, 0x14, 0x02, 0xC1, 0x15, 0x2A};

    test_with_without_object_instance_header_common(input, sizeof(input));
}

TEST_F(TlvTestSuite, test_serialise_string)
{
    Lwm2m_SetLogLevel(DebugLevel_Debug);

    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 0, 1, 0, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 0, 0, AwaResourceType_String, 1, 0, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 0, 0);
    Lwm2mCore_CreateOptionalResource(context, 0, 0, 0);
    //Lwm2mCore_CreateOptionalResource(...)
    Lwm2mCore_SetResourceInstanceValue(context, 0, 0, 0, 0, (char*)"coap://bootstrap.example.com:5684/", strlen("coap://bootstrap.example.com:5684/"));

    uint8_t buffer[512];

    uint8_t expected[] = { 0x8, 0, 0x25, 0xc8, 0, 34, 'c','o','a','p',':','/','/','b','o','o','t','s','t','r','a','p',
            '.','e','x','a','m','p','l','e','.','c','o','m',':','5','6','8','4','/' };

    Lwm2mTreeNode * dest;
    int OIR[] = {0};
    TreeBuilder_CreateTreeFromOIR(&dest, context, Lwm2mRequestOrigin_Client, OIR, 1);

    //TODO: change other tests to use single instance of an object.
    SerdesContext serdesContext;
    int len = TlvSerialiseObject(&serdesContext, dest, 0, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    EXPECT_EQ(static_cast<int>(sizeof(expected)), len);  // encoded 34 chars + header
    EXPECT_EQ(0, memcmp(buffer, expected, sizeof(expected)));
}

TEST_F(TlvTestSuite, test_small_outputbuffer)
{
    Lwm2m_SetLogLevel(DebugLevel_Emerg);  // disable output

    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 0, 1, 0, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 0, 0, AwaResourceType_String, 1, 0, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 0, 0);
    Lwm2mCore_CreateOptionalResource(context, 0, 0, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 0, 0, 0, 0, (char*)"coap://bootstrap.example.com:5684/", strlen("coap://bootstrap.example.com:5684/"));

    Lwm2mTreeNode * dest;
    int OIR[] = {0};
    TreeBuilder_CreateTreeFromOIR(&dest, context, Lwm2mRequestOrigin_Client, OIR, 1);

    uint8_t buffer[15];

    SerdesContext serdesContext;
    int len = TlvSerialiseObject(&serdesContext, dest, 0, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    ASSERT_EQ(-1, len);
}


TEST_F(TlvTestSuite, test_serialise_bool)
{
    bool one = 1;
    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 1, 1, 0, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 1, 0, AwaResourceType_Boolean, 1, 1, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 1, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 1, 0, 0, 0, &one, 1);

    Lwm2mTreeNode * dest;
    int OIR[] = {1};
    TreeBuilder_CreateTreeFromOIR(&dest, context, Lwm2mRequestOrigin_Client, OIR, 1);

    uint8_t buffer[512];
    uint8_t expected[] = { 0x3, 0, 0xc1, 0, 1 };

    SerdesContext serdesContext;
    int len = TlvSerialiseObject(&serdesContext, dest, 1, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    ASSERT_EQ(static_cast<int>(sizeof(expected)), len);
    ASSERT_EQ(0, memcmp(buffer, expected, sizeof(expected)));
}

TEST_F(TlvTestSuite, test_serialise_int8)
{
    uint64_t temp = 17;
    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 2, 1, 0, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 2, 0, AwaResourceType_Integer, 1, 1, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 2, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 2, 0, 0, 0, &temp, sizeof(temp));

    Lwm2mTreeNode * dest;
    int OIR[] = {2};
    TreeBuilder_CreateTreeFromOIR(&dest, context, Lwm2mRequestOrigin_Client, OIR, 1);

    uint8_t buffer[512];
    uint8_t expected[] = { 0x3, 0, 0xc1, 0, 17 };

    SerdesContext serdesContext;
    int len = TlvSerialiseObject(&serdesContext, dest, 2, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    ASSERT_EQ(static_cast<int>(sizeof(expected)), len);
    ASSERT_EQ(0, memcmp(buffer, expected, sizeof(expected)));
}

TEST_F(TlvTestSuite, test_deserialise_int8)
{
    int64_t dest;
    uint8_t buffer[] = { 17 };

    TlvDecodeInteger(&dest, buffer, sizeof(buffer));

    ASSERT_EQ(17, dest);
}

TEST_F(TlvTestSuite, test_serialise_negative_int8)
{
    uint64_t temp = -17;
    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 3, 1, 0, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 3, 0, AwaResourceType_Integer, 1, 1, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 3, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 3, 0, 0, 0, &temp, sizeof(temp));

    Lwm2mTreeNode * dest;
    int OIR[] = {3};
    TreeBuilder_CreateTreeFromOIR(&dest, context, Lwm2mRequestOrigin_Client, OIR, 1);

    uint8_t buffer[512];

#ifdef LWM2M_V1_0
    uint8_t expected[] = { 0x3, 0, 0xc1, 0, 0x91 };
#else
    uint8_t expected[] = { 0x3, 0, 0xc1, 0, 0xEF };
#endif
    SerdesContext serdesContext;
    int len = TlvSerialiseObject(&serdesContext, dest, 3, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    ASSERT_EQ(static_cast<int>(sizeof(expected)), len);
    ASSERT_EQ(0, memcmp(buffer, expected, sizeof(expected)));
}

TEST_F(TlvTestSuite, test_deserialise_negative_int8)
{
    int64_t dest;
#ifdef LWM2M_V1_0
    uint8_t buffer[] = { 0x91 };
#else
    uint8_t buffer[] = { 0xEF };
#endif

    TlvDecodeInteger(&dest, buffer, sizeof(buffer));

    ASSERT_EQ(-17, dest);
}

TEST_F(TlvTestSuite, test_serialise_int16)
{
    uint64_t temp = 1024;
    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 4, 1, 0, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 4, 0, AwaResourceType_Integer, 1, 1, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 4, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 4, 0, 0, 0, &temp, sizeof(temp));

    Lwm2mTreeNode * dest;
    int OIR[] = {4};
    TreeBuilder_CreateTreeFromOIR(&dest, context, Lwm2mRequestOrigin_Client, OIR, 1);

    uint8_t buffer[512];
    uint8_t expected[] = { 0x4, 0, 0xc2, 0, 0x04, 0x00 }; // type, id, msb, lsb

    SerdesContext serdesContext;
    int len = TlvSerialiseObject(&serdesContext, dest, 4, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    ASSERT_EQ(static_cast<int>(sizeof(expected)), len);
    ASSERT_EQ(0, memcmp(buffer, expected, sizeof(expected)));
}

TEST_F(TlvTestSuite, test_deserialise_int16)
{
    int64_t dest;
    uint8_t buffer[] = { 0x04, 0x0 };

    int res = TlvDecodeInteger(&dest, buffer, sizeof(buffer));

    ASSERT_EQ(0, res);
    ASSERT_EQ(1024, dest);
}

TEST_F(TlvTestSuite, test_serialise_negative_int16)
{
    uint64_t temp = -1024;
    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 5, 1, 0, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 5, 0, AwaResourceType_Integer, 1, 1, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 5, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 5, 0, 0, 0, &temp, sizeof(temp));

    Lwm2mTreeNode * dest;
    int OIR[] = {5};
    TreeBuilder_CreateTreeFromOIR(&dest, context, Lwm2mRequestOrigin_Client, OIR, 1);

    uint8_t buffer[512];
#ifdef LWM2M_V1_0
    uint8_t expected[] = { 0x4, 0, 0xc2, 0, 0x84, 0x00 }; // type, id, msb, lsb
#else
    uint8_t expected[] = { 0x4, 0, 0xc2, 0, 0xFC, 0x00 }; // type, id, msb, lsb
#endif
    SerdesContext serdesContext;
    int len = TlvSerialiseObject(&serdesContext, dest, 5, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    ASSERT_EQ(static_cast<int>(sizeof(expected)), len);
    ASSERT_EQ(0, memcmp(buffer, expected, sizeof(expected)));
}

TEST_F(TlvTestSuite, test_deserialise_negative_int16)
{
    int64_t dest;
#ifdef LWM2M_V1_0
    uint8_t buffer[] = { 0x84, 0x0 };
#else
    uint8_t buffer[] = { 0xFC, 0x0 };
#endif
    int res = TlvDecodeInteger(&dest, buffer, sizeof(buffer));

    ASSERT_EQ(0, res);
    ASSERT_EQ(-1024, dest);
}

TEST_F(TlvTestSuite, test_serialise_int32)
{
    uint64_t temp = 65536;
    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 6, 1, 0, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 6, 0, AwaResourceType_Integer, 1, 1, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 6, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 6, 0, 0, 0, &temp, sizeof(temp));

    Lwm2mTreeNode * dest;
    int OIR[] = {6};
    TreeBuilder_CreateTreeFromOIR(&dest, context, Lwm2mRequestOrigin_Client, OIR, 1);

    uint8_t buffer[512];
    uint8_t expected[] = { 0x6, 0, 0xc4, 0, 0x0, 0x1, 0x0, 0x0 }; // type, id, msb, .. ,lsb

    SerdesContext serdesContext;
    int len = TlvSerialiseObject(&serdesContext, dest, 6, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    ASSERT_EQ(static_cast<int>(sizeof(expected)), len);
    ASSERT_EQ(0, memcmp(buffer, expected, sizeof(expected)));
}

TEST_F(TlvTestSuite, test_deserialise_int32)
{
    int64_t dest;
    uint8_t buffer[] = { 0x00, 0x1, 0x0, 0x0 }; // type, id, msb, .. ,lsb

    TlvDecodeInteger(&dest, buffer, sizeof(buffer));

    ASSERT_EQ(65536, dest);
}

TEST_F(TlvTestSuite, test_serialise_negative_int32)
{
    uint64_t temp = -65536;
    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 7, 1, 0, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 7, 0, AwaResourceType_Integer, 1, 1, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 7, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 7, 0, 0, 0, &temp, sizeof(temp));

    Lwm2mTreeNode * dest;
    int OIR[] = {7};
    TreeBuilder_CreateTreeFromOIR(&dest, context, Lwm2mRequestOrigin_Client, OIR, 1);

    uint8_t buffer[512];
#ifdef LWM2M_V1_0
    uint8_t expected[] = { 0x6, 0, 0xc4, 0, 0x80, 0x1, 0x0, 0x0 }; // type, id, msb, .. ,lsb
#else
    uint8_t expected[] = { 0x6, 0, 0xc4, 0, 0xFF, 0xFF, 0x0, 0x00 }; // type, id, msb, .. ,lsb
#endif

    SerdesContext serdesContext;
    int len = TlvSerialiseObject(&serdesContext, dest, 7, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    ASSERT_EQ(static_cast<int>(sizeof(expected)), len);
    ASSERT_EQ(0, memcmp(buffer, expected, sizeof(expected)));
}

TEST_F(TlvTestSuite, test_deserialise_negative_int32)
{
    int64_t dest;
#ifdef LWM2M_V1_0
    uint8_t buffer[] = { 0x80, 0x1, 0x0, 0x0 }; // type, id, msb, .. ,lsb
#else
    uint8_t buffer[] = { 0xFF, 0xFF, 0x0, 0x0 }; // type, id, msb, .. ,lsb
#endif
    int res = TlvDecodeInteger(&dest, buffer, sizeof(buffer));

    ASSERT_EQ(0, res);
    ASSERT_EQ(-65536, dest);
}

TEST_F(TlvTestSuite, test_serialise_int64)
{
    uint64_t temp =  8589934636;
    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 8, 1, 0, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 8, 0, AwaResourceType_Integer, 1, 1, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 8, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 8, 0, 0, 0, &temp, sizeof(temp));

    Lwm2mTreeNode * dest;
    int OIR[] = {8};
    TreeBuilder_CreateTreeFromOIR(&dest, context, Lwm2mRequestOrigin_Client, OIR, 1);

    uint8_t buffer[512];
    uint8_t expected[] = { 0x8, 0, 0xb, 0xc8, 0, 0x8, 0x0, 0x0, 0x0, 0x02, 0x00, 0x00, 0x00, 0x2c }; // type, id, msb, .. ,lsb

    SerdesContext serdesContext;
    int len = TlvSerialiseObject(&serdesContext, dest, 8, buffer, sizeof(buffer));
 
    Lwm2mTreeNode_DeleteRecursive(dest);

    ASSERT_EQ(static_cast<int>(sizeof(expected)), len);
    ASSERT_EQ(0, memcmp(buffer, expected, sizeof(expected)));
}

TEST_F(TlvTestSuite, test_deserialise_int64)
{
    int64_t dest;
    uint8_t buffer[] = { 0x0, 0x0, 0x0, 0x02, 0x00, 0x00, 0x00, 0x2c }; // type, id, msb, .. ,lsb

    TlvDecodeInteger(&dest, buffer, sizeof(buffer));

    ASSERT_EQ(8589934636, dest);
}

TEST_F(TlvTestSuite, test_serialise_negative_int64)
{
    uint64_t temp =  -8589934636;
    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 9, 1, 0, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 9, 0, AwaResourceType_Integer, 1, 1, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 9, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 9, 0, 0, 0, &temp, sizeof(temp));

    Lwm2mTreeNode * dest;
    int OIR[] = {9};
    TreeBuilder_CreateTreeFromOIR(&dest, context, Lwm2mRequestOrigin_Client, OIR, 1);

    uint8_t buffer[512];
#ifdef LWM2M_V1_0
    uint8_t expected[] = { 0x8, 0, 0xb, 0xc8, 0, 0x8, 0x80, 0x0, 0x0, 0x02, 0x00, 0x00, 0x00, 0x2c }; // type, id, msb, .. ,lsb
#else
    uint8_t expected[] = { 0x8, 0, 0xb, 0xc8, 0, 0x8, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xd4 }; // type, id, msb, .. ,lsb
#endif
    SerdesContext serdesContext;
    int len = TlvSerialiseObject(&serdesContext, dest, 9, buffer, sizeof(buffer));
 
    Lwm2mTreeNode_DeleteRecursive(dest);

    ASSERT_EQ(static_cast<int>(sizeof(expected)), len);
    ASSERT_EQ(0, memcmp(buffer, expected, sizeof(expected)));
}

TEST_F(TlvTestSuite, test_deserialise_negative_int64)
{
    int64_t dest;
#ifdef LWM2M_V1_0
    uint8_t buffer[] = { 0x80, 0x0, 0x0, 0x02, 0x00, 0x00, 0x00, 0x2c }; // type, id, msb, .. ,lsb
#else
    uint8_t buffer[] = { 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xd4 }; // type, id, msb, .. ,lsb
#endif
    TlvDecodeInteger(&dest, buffer, sizeof(buffer));

    ASSERT_EQ(-8589934636, dest);
}

TEST_F(TlvTestSuite, test_serialise_float32)
{
    float temp = 10.56f;
    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 10, 1, 0, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 10, 0, AwaResourceType_Float, 1, 1, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 10, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 10, 0, 0, 0, &temp, sizeof(temp));

    Lwm2mTreeNode * dest;
    int OIR[] = {10};
    TreeBuilder_CreateTreeFromOIR(&dest, context, Lwm2mRequestOrigin_Client, OIR, 1);

    uint8_t buffer[512];
    uint8_t expected[] = { 0x6, 0, 0xc4, 0, 0x41, 0x28, 0xf5, 0xc3}; // type, id, msb, .. ,lsb

    SerdesContext serdesContext;
    int len = TlvSerialiseObject(&serdesContext, dest, 10, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    ASSERT_EQ(static_cast<int>(sizeof(expected)), len);
    ASSERT_EQ(0, memcmp(buffer, expected, sizeof(expected)));
}

TEST_F(TlvTestSuite, test_deserialise_float32)
{
    Lwm2m_SetLogLevel(DebugLevel_Emerg);  // disable output

    double dest;
    uint8_t buffer[] = { 0x41, 0x28, 0xf5, 0xc3 }; // type, id, msb, .., lsb

    int res = TlvDecodeFloat(&dest, buffer, sizeof(buffer));

    ASSERT_EQ(0, res);
    ASSERT_FLOAT_EQ(10.56, dest);

    // range check inputs
    res = TlvDecodeFloat(NULL, buffer, sizeof(buffer));
    ASSERT_EQ(-1, res);
    res = TlvDecodeFloat(&dest, NULL, sizeof(buffer));
    ASSERT_EQ(-1, res);
    res = TlvDecodeFloat(&dest, buffer, 1);
    ASSERT_EQ(-1, res);
    res = TlvDecodeFloat(&dest, buffer, 9);
    ASSERT_EQ(-1, res);
}

TEST_F(TlvTestSuite, test_serialise_float64)
{
    double temp = 3.40282347E+39;
    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 11, 1, 0, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 11, 0, AwaResourceType_Float, 1, 1, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 11, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 11, 0, 0, 0, &temp, sizeof(temp));

    Lwm2mTreeNode * dest;
    int OIR[] = {11};
    TreeBuilder_CreateTreeFromOIR(&dest, context, Lwm2mRequestOrigin_Client, OIR, 1);

    uint8_t buffer[512];
    uint8_t expected[] = { 0x8, 0, 0xb, 0xc8, 0, 8, 0x48, 0x23, 0xff, 0xff, 0xec, 0x5b, 0x3f, 0x86 };

    SerdesContext serdesContext;
    int len = TlvSerialiseObject(&serdesContext, dest, 11, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    ASSERT_EQ(static_cast<int>(sizeof(expected)), len);
    ASSERT_EQ(0, memcmp(buffer, expected, sizeof(expected)));
}

TEST_F(TlvTestSuite, test_deserialise_float64)
{
    double dest;
    uint8_t buffer[] = { 0x48, 0x23, 0xff, 0xff, 0xec, 0x5b, 0x3f, 0x86 };

    TlvDecodeFloat(&dest, buffer, sizeof(buffer));

    ASSERT_DOUBLE_EQ(3.40282347E+39, dest);
}
#if 0
TEST_F(TlvTestSuite, test_serialise_objectlink)
{
    ObjectType object0;

    memset(&object0, 0, sizeof(object0));
    object0.id = 0;
    object0.instance[0].id = 0;
    object0.instance[0].resource[0].id = 0;
    object0.instance[0].resource[0].instance[0].data_obj.objectId = 10;
    object0.instance[0].resource[0].instance[0].data_obj.objectInstanceId = 40;
    object0.instance[0].resource[0].instance[0].type = OBJLINK;
    object0.instance[0].resource[0].numInstances = 1;
    object0.instance[0].numResources = 1;
    object0.numInstances = 1;

    uint8_t buffer[512];
    uint8_t expected[] = { 0xc4, 0, 0x0, 0x0A, 0x0, 0x28 }; // type, id, msb, .. ,lsb

    int len = TlvSerialiseObject(&object0, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    ASSERT_EQ(static_cast<int>(sizeof(expected)), len);
    ASSERT_EQ(0, memcmp(buffer, expected, sizeof(expected)));
}

TEST_F(TlvTestSuite, test_deserialise_objectlink)
{
    uint16_t objectId, instanceId;
    uint8_t buffer[] = { 0x0, 0xa, 0x0, 0x28 };

    int res = TlvDecodeObjectLink(&objectId, &instanceId, buffer, sizeof(buffer));

    ASSERT_EQ(0, res);
    ASSERT_EQ(10, objectId);
    ASSERT_EQ(40, instanceId);

    // test invalid buffer length
    res = TlvDecodeObjectLink(&objectId, &instanceId, buffer, 3);
    ASSERT_EQ(-1, res);

    // test NULL buffer pointer
    res = TlvDecodeObjectLink(&objectId, &instanceId, NULL, 4);
    ASSERT_EQ(-1, res);

    // test NULL object Id
    res = TlvDecodeObjectLink(NULL, &instanceId, buffer, sizeof(buffer));
    ASSERT_EQ(-1, res);

    // test NULL instance Id
    res = TlvDecodeObjectLink(NULL, &instanceId, buffer, sizeof(buffer));
    ASSERT_EQ(-1, res);
}
#endif

TEST_F(TlvTestSuite, test_no_instance)
{
    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 55, 1, 0, &defaultObjectOperationHandlers);

    Lwm2mTreeNode * dest;
    int OIR[] = {55};
    TreeBuilder_CreateTreeFromOIR(&dest, context, Lwm2mRequestOrigin_Client, OIR, 1);

    uint8_t buffer[512];
    SerdesContext serdesContext;
    int len = TlvSerialiseObject(&serdesContext,dest, 55, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    ASSERT_EQ(0, len);
}

TEST_F(TlvTestSuite, test_ident_8bit)
{
    int32_t temp = 1;
    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 12, 1, 0, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 12, 1, AwaResourceType_Integer, 1, 0, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 12, 0);
    Lwm2mCore_CreateOptionalResource(context, 12, 0, 1);
    Lwm2mCore_SetResourceInstanceValue(context, 12, 0, 1, 0, &temp, sizeof(temp));

    Lwm2mTreeNode * dest;
    int OIR[] = {12};
    TreeBuilder_CreateTreeFromOIR(&dest, context, Lwm2mRequestOrigin_Client, OIR, 1);

    uint8_t buffer[512];
    uint8_t expected[] = { 0x3, 0, 0xc1, 1, 1 };

    SerdesContext serdesContext;
    int len = TlvSerialiseObject(&serdesContext, dest, 12, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    ASSERT_EQ(static_cast<int>(sizeof(expected)), len);
    ASSERT_EQ(0, memcmp(buffer, expected, sizeof(expected)));
}

TEST_F(TlvTestSuite, test_ident_16bit)
{
    int64_t temp = 1;
    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 13, 1, 0, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 13, 1024, AwaResourceType_Integer, 1, 1, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 13, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 13, 0, 1024, 0, &temp, sizeof(temp));

    Lwm2mTreeNode * dest;
    int OIR[] = {13};
    TreeBuilder_CreateTreeFromOIR(&dest, context, Lwm2mRequestOrigin_Client, OIR, 1);

    uint8_t buffer[512];
    uint8_t expected[] = { 0x4, 0, 0xe1, 0x4, 0x0, 1 };

    SerdesContext serdesContext;
    int len = TlvSerialiseObject(&serdesContext, dest, 13, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    ASSERT_EQ(static_cast<int>(sizeof(expected)), len);
    ASSERT_EQ(0, memcmp(buffer, expected, sizeof(expected)));
}

TEST_F(TlvTestSuite, test_encode_large_opaque)
{
    int valueLen = 0x1ffff;
    int bufferLen = valueLen + 6;
    uint8_t * value = (uint8_t*)malloc(valueLen);
    uint8_t * buffer = (uint8_t*)malloc(bufferLen);
    int len;

    memset(buffer, 0, bufferLen);

    for (int i = 0; i < valueLen; i++)
    {
        value [i] = i % 0xff;
    }

    len = TlvEncodeOpaque(buffer, bufferLen,  TLV_TYPE_IDENT_OBJECT_INSTANCE, 1024, value, valueLen);

    uint8_t expected[] = { 0x38, 0x04, 0x00, 0x01, 0xff, 0xff };
    
    ASSERT_EQ(static_cast<int>(bufferLen), len);

    // check header block
    ASSERT_EQ(0, memcmp(buffer, expected, sizeof(expected)));

    for (int i = 6; i < len; i++) 
    {
        ASSERT_EQ(buffer[i], (i - 6) % 0xff);
    }

    free(value);
    free(buffer);
}

TEST_F(TlvTestSuite, test_multiple_instance_resource)
{
    int16_t temp = 0x44;
    int8_t temp2 = 0x55;
    ASSERT_EQ(0, Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 14, 1, 0, &defaultObjectOperationHandlers));
    ASSERT_EQ(0, Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 14, 0, AwaResourceType_Integer, 2, 1, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers));
    Lwm2mCore_CreateObjectInstance(context, 14, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 14, 0, 0, 0, &temp, sizeof(temp));
    Lwm2mCore_SetResourceInstanceValue(context, 14, 0, 0, 1, &temp2, sizeof(temp2));

    Lwm2mTreeNode * dest;
    int OIR[] = {14};
    ASSERT_EQ(AwaResult_Success, TreeBuilder_CreateTreeFromOIR(&dest, context, Lwm2mRequestOrigin_Client, OIR, 1));

    uint8_t buffer[512];
    // Note: sometimes the encoding order of the last 3 bytes switches position with the previous 3 bytes
    uint8_t expected[] = { 0x8, 0, 0x8, 0x86, 0, 0x41, 0, 0x44, 0x41, 1, 0x55 };

    SerdesContext serdesContext;
    int len = TlvSerialiseObject(&serdesContext, dest, 14, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    ASSERT_EQ(static_cast<int>(sizeof(expected)), len);
    ASSERT_EQ(0, memcmp(buffer, expected, sizeof(expected)));
}

TEST_F(TlvTestSuite, test_multiple_object_instance)
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
    uint8_t expected[] = { 0x3, 1, 0xc1, 0, 55, 0x3, 0, 0xc1, 0, 44 };

    SerdesContext serdesContext;
    int len = TlvSerialiseObject(&serdesContext, dest, 15, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    ASSERT_EQ(static_cast<int>(sizeof(expected)), len);
    ASSERT_EQ(0, memcmp(buffer, expected, sizeof(expected)));
}

TEST_F(TlvTestSuite, test_serialise_object_with_two_resources)
{
    int64_t temp = 77;
    Definition_RegisterObjectType(Lwm2mCore_GetDefinitions(context), (char*)"Test", 16, 1, 0, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res1", 16, 0, AwaResourceType_String, 1, 1, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, (char*)"Res2", 16, 1, AwaResourceType_Integer, 1, 1, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_CreateObjectInstance(context, 16, 0);
    Lwm2mCore_SetResourceInstanceValue(context, 16, 0, 0, 0, (char*)"coap://bootstrap.example.com:5684/", strlen("coap://bootstrap.example.com:5684/"));
    Lwm2mCore_SetResourceInstanceValue(context, 16, 0, 1, 0, &temp, sizeof(temp));

    Lwm2mTreeNode * dest;
    int OIR[] = {16};
    TreeBuilder_CreateTreeFromOIR(&dest, context, Lwm2mRequestOrigin_Client, OIR, 1);

    uint8_t buffer[512];
    uint8_t expected[] = { 8, 0, 0x28, 0xc8, 0, 34, 'c','o','a','p',':','/','/','b','o','o','t','s','t','r','a','p',
            '.','e','x','a','m','p','l','e','.','c','o','m',':','5','6','8','4','/',
            0xc1, 1, 77};

    SerdesContext serdesContext;
    int len = TlvSerialiseObject(&serdesContext, dest, 16, buffer, sizeof(buffer));

    Lwm2mTreeNode_DeleteRecursive(dest);

    ASSERT_EQ(static_cast<int>(sizeof(expected)), len);  // encoded 34 chars + header
    ASSERT_EQ(0, memcmp(buffer, expected, sizeof(expected)));
}

namespace detail {

struct FloatItem
{
    double Value;
    int Size;
};

::std::ostream& operator<<(::std::ostream& os, const FloatItem& item)
{
    std::streamsize ss = os.precision();
    os << std::setprecision(15)
       << "FloatItem: "
       << "Value " << item.Value
       << ", Size " << item.Size;
    os.precision(ss);
    return os;
}

} // namespace detail

class TlvTestSuiteFloat : public TlvTestSuite, public ::testing::WithParamInterface<detail::FloatItem> {};

TEST_P(TlvTestSuiteFloat, test_float_precision_selection)
{
    const size_t BUFFER_LEN = 100;
    uint8_t buffer[BUFFER_LEN] = { 0 };
    int bufferLen = BUFFER_LEN;
    int type = TLV_TYPE_IDENT_RESOURCE_VALUE;
    unsigned short identifier = 42;
    int resourceLen = 0;
    double expectedValue = GetParam().Value;

    // encode a value and check that appropriate precision is selected:
    TlvEncodeFloat(buffer, bufferLen, type, identifier, expectedValue);

    // decode header first
    int headerLen = TlvDecodeHeader(&type, &identifier, &resourceLen, buffer, bufferLen);
    EXPECT_EQ(TLV_TYPE_IDENT_RESOURCE_VALUE, type);
    EXPECT_EQ(42, identifier);
    EXPECT_EQ(GetParam().Size, resourceLen);

    double actualValue = 0.0;
    ASSERT_GE(headerLen, 0);
    ASSERT_LT(headerLen, static_cast<decltype(headerLen)>(BUFFER_LEN));
    EXPECT_EQ(0, TlvDecodeFloat(&actualValue, &buffer[headerLen], resourceLen));
    EXPECT_NEAR(expectedValue, actualValue, FLT_EPSILON);
}

INSTANTIATE_TEST_CASE_P(
        TlvTestSuiteFloat,
        TlvTestSuiteFloat,
        ::testing::Values(
                detail::FloatItem { 0.0, 4 },
                detail::FloatItem { (float)1.0, 4 },
                detail::FloatItem { (double)1.0, 4 },
                detail::FloatItem { FLT_MIN, 4 },
                detail::FloatItem { FLT_MAX, 4 },
                detail::FloatItem { DBL_MIN, 8 },
                detail::FloatItem { DBL_MAX, 8 },
                detail::FloatItem { (float)(1 + 1e-2), 4 },
                detail::FloatItem { (double)(1 + 1e-2), 8 },
                detail::FloatItem { 1 + 1e-10, 8 },
                detail::FloatItem { 1.0 / 7.0, 8 },
                detail::FloatItem { 1001.00002, 8 },
                detail::FloatItem { -1.234567e-30, 8 }
        ));


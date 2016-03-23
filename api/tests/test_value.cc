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

#include "support/support.h"
#include "value.h"
#include "xmltree.h"
#include "xml.h"
#include "lwm2m_types.h"
#include "lwm2m_xml_interface.h"

namespace Awa {

class TestValue : public TestAwaBase {};

TEST_F(TestValue, Value_New_handles_null)
{
    EXPECT_EQ(NULL, Value_New(NULL, AwaResourceType_Invalid));
}

TEST_F(TestValue, DISABLED_Value_New_handles_invalid_value_node)
{
    { TreeNode node = Xml_CreateNodeWithValue("Value",    "%d", 1);  EXPECT_EQ(NULL, Value_New(node, AwaResourceType_Invalid)); Tree_Delete(node); }
    { TreeNode node = Xml_CreateNodeWithValue("NotValue", "%d", 42); EXPECT_EQ(NULL, Value_New(node, AwaResourceType_Integer)); Tree_Delete(node); }
    { TreeNode node = Xml_CreateNodeWithValue("Values",   "%d", 17); EXPECT_EQ(NULL, Value_New(node, AwaResourceType_Integer)); Tree_Delete(node); }
    { TreeNode node = Xml_CreateNodeWithValue("NotValue", "%d", 99); EXPECT_EQ(NULL, Value_New(node, AwaResourceType_String)); Tree_Delete(node); }
}

TEST_F(TestValue, DISABLED_Value_New_handles_integer_value_node)
{
    TreeNode node = Xml_CreateNodeWithValue("Value", "%d", 7);
    Value * value = Value_New(node, AwaResourceType_Integer);
    EXPECT_TRUE(value != NULL);
    EXPECT_EQ(AwaResourceType_Integer, Value_GetType(value));
    EXPECT_EQ(8u, Value_GetLength(value));

    const AwaInteger * data = (const AwaInteger *)Value_GetData(value);
    EXPECT_EQ(7, *data);

    Tree_Delete(node);
    Value_Free(&value);
}

TEST_F(TestValue, DISABLED_Value_New_handles_string_value_node)
{
    char * encodedString = xmlif_EncodeValue(AwaResourceType_String, "Hello!", strlen("Hello!"));
    TreeNode node = Xml_CreateNodeWithValue("Value", "%s", encodedString);
    free(encodedString);
    Value * value = Value_New(node, AwaResourceType_String);
    EXPECT_TRUE(value != NULL);
    EXPECT_EQ(AwaResourceType_String, Value_GetType(value));
    EXPECT_EQ(7u, Value_GetLength(value));

    const char * data = (const char *)Value_GetData(value);
    EXPECT_STREQ("Hello!", data);

    Tree_Delete(node);
    Value_Free(&value);
}

TEST_F(TestValue, Value_New_handles_boolean_value_node)
{
    // TODO
}

TEST_F(TestValue, Value_New_handles_time_value_node)
{
    // TODO
}

TEST_F(TestValue, Value_New_handles_opaque_value_node)
{
    // TODO
}

TEST_F(TestValue, Value_New_handles_objlink_value_node)
{
    // TODO
}



} // namespace Awa

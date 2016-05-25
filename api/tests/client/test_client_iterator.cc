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

#include "client_iterator.h"
#include "support/support.h"

namespace Awa {

class TestClientIterator : public TestClientBase {};

TEST_F(TestClientIterator, ClientIterator_New_and_Free)
{
    ClientIterator * iterator = ClientIterator_New();
    ASSERT_TRUE(NULL != iterator);
    ClientIterator_Free(&iterator);
    EXPECT_EQ(NULL, iterator);
}

TEST_F(TestClientIterator, ClientIterator_Free_handles_null_pointer)
{
    ClientIterator * iterator = NULL;
    ClientIterator_Free(&iterator);
    EXPECT_EQ(NULL, iterator);
}

TEST_F(TestClientIterator, ClientIterator_Free_handles_null)
{
    ClientIterator_Free(NULL);
}

TEST_F(TestClientIterator, ClientIterator_Next_handles_empty_iterator)
{
    ClientIterator * iterator = ClientIterator_New();
    EXPECT_TRUE(false == ClientIterator_Next(iterator));
    ClientIterator_Free(&iterator);
}

TEST_F(TestClientIterator, ClientIterator_Next_handles_non_empty_iterator)
{
    ClientIterator * iterator = ClientIterator_New();
    ClientIterator_Add(iterator, "TestIMG1");
    ClientIterator_Add(iterator, "Imagination0");
    ClientIterator_Add(iterator, "ClientABC123____________abcdef");
    EXPECT_EQ(true, ClientIterator_Next(iterator));
    EXPECT_STREQ("TestIMG1", ClientIterator_GetClientID(iterator));
    EXPECT_EQ(true, ClientIterator_Next(iterator));
    EXPECT_STREQ("Imagination0", ClientIterator_GetClientID(iterator));
    EXPECT_EQ(true, ClientIterator_Next(iterator));
    EXPECT_STREQ("ClientABC123____________abcdef", ClientIterator_GetClientID(iterator));
    EXPECT_TRUE(false == ClientIterator_Next(iterator));
    EXPECT_TRUE(false == ClientIterator_Next(iterator));
    EXPECT_TRUE(false == ClientIterator_Next(iterator));
    EXPECT_EQ(NULL, ClientIterator_GetClientID(iterator));
    ClientIterator_Free(&iterator);
}

TEST_F(TestClientIterator, ClientIterator_Next_handles_null)
{
    EXPECT_TRUE(false == ClientIterator_Next(NULL));
}

TEST_F(TestClientIterator, AwaClientIterator_Next_handles_null)
{
    EXPECT_TRUE(false == AwaClientIterator_Next(NULL));
}

TEST_F(TestClientIterator, ClientIterator_handles_Get_before_Next)
{
    ClientIterator * iterator = ClientIterator_New();
    ClientIterator_Add(iterator, "TestIMG1");
    ClientIterator_Add(iterator, "Imagination0");
    ASSERT_TRUE(NULL == ClientIterator_GetClientID(iterator));
    ClientIterator_Free(&iterator);
}



} // namespace Awa

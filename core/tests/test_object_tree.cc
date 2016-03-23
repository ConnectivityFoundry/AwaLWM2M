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
#include "client/lwm2m_object_tree.h"

class ObjectTreeTestSuite : public testing::Test
{
  void SetUp() { }
  void TearDown() { }
};

TEST_F(ObjectTreeTestSuite, test_add_object)
{
    Lwm2mObjectTree objectTree;

    Lwm2mObjectTree_Init(&objectTree);

    // test NULL pointer for objectTree
    ASSERT_EQ(-1, Lwm2mObjectTree_AddObject(NULL, 0));
 
    // test create object entry
    ASSERT_EQ(0, Lwm2mObjectTree_AddObject(&objectTree, 0));

    // test duplicate entry
    ASSERT_EQ(-1, Lwm2mObjectTree_AddObject(&objectTree, 0));

    // test NULL pointer for object tree passed to delete object
    ASSERT_EQ(-1, Lwm2mObjectTree_DeleteObject(NULL, 0));
   
    // test delete object success 
    ASSERT_EQ(0, Lwm2mObjectTree_DeleteObject(&objectTree, 0));

    // test delete non-existent object

    ASSERT_EQ(-1, Lwm2mObjectTree_DeleteObject(&objectTree, 1000));

    ASSERT_EQ(-1, Lwm2mObjectTree_Destroy(NULL));

    ASSERT_EQ(0, Lwm2mObjectTree_Destroy(&objectTree));
}





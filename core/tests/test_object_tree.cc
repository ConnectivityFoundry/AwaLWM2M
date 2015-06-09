
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





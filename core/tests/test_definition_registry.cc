
#include <gtest/gtest.h>
#include <string>
#include <stdio.h>
#include "lwm2m_definition.h"

class Lwm2mDefinitionRegistryTestSuite : public testing::Test
{
  void SetUp() { }
  void TearDown() { }
};

TEST_F(Lwm2mDefinitionRegistryTestSuite, test_create_destroy)
{
    DefinitionRegistry * registry = DefinitionRegistry_Create();
     
    ASSERT_EQ(-1, DefinitionRegistry_Destroy(NULL));
    ASSERT_EQ(0,  DefinitionRegistry_Destroy(registry));
}

TEST_F(Lwm2mDefinitionRegistryTestSuite, test_register_lookup_object)
{
    DefinitionRegistry * registry = DefinitionRegistry_Create();
     
    ASSERT_EQ(-1, Definition_RegisterObjectType(NULL, "test object", 1000, MultipleInstancesEnum_Single, MandatoryEnum_Optional, NULL));
    // test successful registration
    ASSERT_EQ(0, Definition_RegisterObjectType(registry, "test object", 1000, MultipleInstancesEnum_Single, MandatoryEnum_Optional, NULL));
    // test register definition twice but with different definition
    ASSERT_EQ(-1, Definition_RegisterObjectType(registry, "test object 1", 1000, MultipleInstancesEnum_Single, MandatoryEnum_Optional, NULL));

    ASSERT_EQ(-1, Definition_RegisterResourceType(NULL, "test resource", 1000, 101, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL, NULL));
    ASSERT_EQ(-1, Definition_RegisterResourceType(registry, "test resource", 2000, 101, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL, NULL));
    ASSERT_EQ(0, Definition_RegisterResourceType(registry, "test resource", 1000, 101, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL, NULL));
    ASSERT_EQ(-1, Definition_RegisterResourceType(registry, "test resource 1", 1000, 101, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Optional, Operations_R, NULL, NULL));
    

    ASSERT_EQ(-1, DefinitionRegistry_Destroy(NULL));
    ASSERT_EQ(0,  DefinitionRegistry_Destroy(registry));
}



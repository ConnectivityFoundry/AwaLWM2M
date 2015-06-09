
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

#include "common/xml.h"
#include "common/lwm2m_tree_node.h"
#include "common/lwm2m_tree_builder.h"
#include "lwm2m_core.h"
#include "common/lwm2m_request_origin.h"

class XmlTestSuite : public testing::Test
{
    void SetUp() { context = Lwm2mCore_Init(NULL, NULL, NULL); }
    void TearDown() { Lwm2mCore_Destroy(context); }

protected:
    Lwm2mContextType * context;
};


TEST_F(XmlTestSuite, test_deserialise_serialise)
{
    Lwm2m_SetLogLevel(DebugLevel_Debug);
    const char * testMessage = "<Response>\n\
 <Code>400</Code>\n\
</Response>\n";

    TreeNode rootNode = TreeNode_ParseXML((uint8_t *)testMessage, strlen(testMessage), true);
    ASSERT_TRUE(rootNode != NULL);

    char buffer[65536];
    Xml_TreeToString(rootNode, buffer, sizeof(buffer));
    //printf("AFTER PARSE XML: %s\n", buffer);

    Tree_Delete(rootNode);
    EXPECT_STREQ(testMessage, buffer);
}

TEST_F(XmlTestSuite, test_deserialise_serialise_2)
{
    const char * testMessage = "<Request>\n\
 <Type>Set</Type>\n\
 <Content>\n\
  <Objects>\n\
   <Object>\n\
    <ObjectID>10000</ObjectID>\n\
    <Instance>\n\
     <InstanceID>0</InstanceID>\n\
     <Create></Create>\n\
     <Property>\n\
      <PropertyID>7</PropertyID>\n\
      <Value>\n\
       <ValueID>0</ValueID>\n\
       <ValueData>/3:0</ValueData>\n\
      </Value>\n\
     </Property>\n\
    </Instance>\n\
   </Object>\n\
  </Objects>\n\
 </Content>\n\
</Request>\n";

    TreeNode rootNode = TreeNode_ParseXML((uint8_t *)testMessage, strlen(testMessage), true);
    ASSERT_TRUE(rootNode != NULL);

    char buffer[65536];
    Xml_TreeToString(rootNode, buffer, sizeof(buffer));
    //printf("AFTER PARSE XML: %s\n", buffer);

    Tree_Delete(rootNode);
    EXPECT_STREQ(testMessage, buffer);
}



TEST_F(XmlTestSuite, test_Xml_FindChildWithGrandchildValue)
{
    const char * testNode = "<Resource>\n\
 <ResourceInstance>\n\
  <ID>1</ID>\n\
 </ResourceInstance>\n\
</Resource>\n";

    TreeNode rootNode = TreeNode_ParseXML((uint8_t *)testNode, strlen(testNode), true);
    ASSERT_TRUE(rootNode != NULL);

    char buffer[65536];
    Xml_TreeToString(rootNode, buffer, sizeof(buffer));
    //printf("AFTER PARSE XML: %s\n", buffer);

    EXPECT_TRUE(NULL != Xml_FindChildWithGrandchildValue(rootNode, "ResourceInstance", "ID", "1"));
    EXPECT_TRUE(NULL == Xml_FindChildWithGrandchildValue(rootNode, "ResourceInstance", "ID", "2"));
    // TODO: check value
    Tree_Delete(rootNode);
}

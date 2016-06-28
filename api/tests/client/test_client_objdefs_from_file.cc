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

namespace Awa {

namespace detail {

static const char * multipleObjDefsXML = R"(
<ObjectDefinitions>
  <Items>

    <!-- mandatory, singleton -->
    <ObjectDefinition>
      <ObjectID>7301</ObjectID>
      <Name>Test Object 1</Name>
      <SerialisationName>TestObject1</SerialisationName>
      <Singleton>True</Singleton>
      <IsMandatory>True</IsMandatory>
      <Properties>
        <PropertyDefinition>
          <PropertyID>0</PropertyID>
          <Name>TestString</Name>
          <SerialisationName>TestString</SerialisationName>
          <DataType>String</DataType>
          <IsCollection>False</IsCollection>
          <IsMandatory>False</IsMandatory>
          <Access>Read</Access>
        </PropertyDefinition>
      </Properties>
    </ObjectDefinition>

    <!-- mandatory, multiple -->
    <ObjectDefinition>
      <ObjectID>7302</ObjectID>
      <Name>Test Object 2</Name>
      <SerialisationName>TestObject2</SerialisationName>
      <!-- <Singleton>False</Singleton>  -->
      <IsMandatory>True</IsMandatory>
      <Properties>
        <!-- no properties -->
      </Properties>
    </ObjectDefinition>

    <!-- optional, multiple -->
    <ObjectDefinition>
      <ObjectID>7303</ObjectID>
      <Name>Test Object 3</Name>
      <SerialisationName>TestObject3</SerialisationName>
      <!-- <Singleton>False</Singleton>  -->
      <!-- <IsMandatory>False</IsMandatory> -->
      <Properties>
        <!-- no properties -->
      </Properties>
    </ObjectDefinition>

    <!-- optional, singleton -->
    <ObjectDefinition>
      <ObjectID>7304</ObjectID>
      <Name>Test Object 4</Name>
      <SerialisationName>TestObject2</SerialisationName>
      <Singleton>True</Singleton>
      <!-- <IsMandatory>False</IsMandatory> -->
      <Properties>
        <!-- no properties -->
      </Properties>
    </ObjectDefinition>
  
    <!-- multiple resources -->
    <ObjectDefinition>
      <ObjectID>7310</ObjectID>
      <Name>Test Object 10</Name>
      <SerialisationName>TestObject10</SerialisationName>
      <Properties>
        <PropertyDefinition>
          <PropertyID>42</PropertyID>
          <Name>Test Integer</Name>
          <SerialisationName>TestInteger</SerialisationName>
          <DataType>Integer</DataType>
          <IsCollection>False</IsCollection>
          <IsMandatory>True</IsMandatory>
          <Access>ReadWrite</Access>
        </PropertyDefinition>
        <PropertyDefinition>
          <PropertyID>43</PropertyID>
          <Name>Test Float</Name>
          <SerialisationName>TestFloat</SerialisationName>
          <DataType>Float</DataType>
          <IsCollection>True</IsCollection>
          <IsMandatory>False</IsMandatory>
          <!-- default access -->
        </PropertyDefinition>
      </Properties>
    </ObjectDefinition>
  </Items>
</ObjectDefinitions>
)";

static const char * singleObjDefXML = R"(
    <ObjectDefinition>
      <ObjectID>7300</ObjectID>
      <Name>Test Object</Name>
      <SerialisationName>TestObject</SerialisationName>
      <Singleton>True</Singleton>
      <Properties>
        <PropertyDefinition>
          <PropertyID>0</PropertyID>
          <Name>Test String</Name>
          <SerialisationName>TestString</SerialisationName>
          <DataType>String</DataType>
          <IsCollection>False</IsCollection>
          <IsMandatory>False</IsMandatory>
          <Access>Read</Access>
        </PropertyDefinition>
      </Properties>
    </ObjectDefinition>
)";

static const AwaObjectID EXPECTED_OBJECT_ID = 7300;

} // namespace detail

class ObjDefsFile {
public:
    ObjDefsFile(const char * content) {
        // save objDefs to a temporary file
        filename_ = TempFilename().GetFilename();
        std::ofstream file(filename_, std::ios::out);
        file << content << std::endl;
        file.close();
    }
    ~ObjDefsFile() {
        if (std::remove(filename_.c_str()) < 0)
        {
            std::perror("Error deleting file");
        }
    }

    std::string GetFilename() const { return filename_; }

private:
    std::string filename_;
};



class TestClientObjDefsMultipleFromFile : public TestClientWithConnectedSession {
public:
    TestClientObjDefsMultipleFromFile() : objDefsFile_(detail::multipleObjDefsXML) {}
    virtual void SetUp() {
        daemon_.SetAdditionalOptions({ "--objDefs", objDefsFile_.GetFilename()});
        TestClientWithConnectedSession::SetUp();
    }
private:
    ObjDefsFile objDefsFile_;
};


TEST_F(TestClientObjDefsMultipleFromFile, test_valid_file) {
    const AwaObjectDefinition * objectDefinition = AwaClientSession_GetObjectDefinition(session_, detail::EXPECTED_OBJECT_ID);
    EXPECT_TRUE(NULL != objectDefinition);

    // check definition
    const AwaObjectDefinition * obj7300 = AwaClientSession_GetObjectDefinition(session_, 7300);
    ASSERT_TRUE(NULL != obj7300);

    EXPECT_EQ(7300, AwaObjectDefinition_GetID(obj7300));

    // should use serialisation name for object name
    EXPECT_STREQ("TestObject", AwaObjectDefinition_GetName(obj7300));
    EXPECT_EQ(1, AwaObjectDefinition_GetMinimumInstances(obj7300));
    EXPECT_EQ(1, AwaObjectDefinition_GetMinimumInstances(obj7300));

    const AwaResourceDefinition * res7300_0 = AwaObjectDefinition_GetResourceDefinition(obj7300, 0);
    ASSERT_TRUE(NULL != res7300_0);

    EXPECT_EQ(0, AwaResourceDefinition_GetID(res7300_0));

    // should use serialisation name for resource name
    EXPECT_STREQ("TestString", AwaResourceDefinition_GetName(res7300_0));


}



class TestClientObjDefsSingleFromFile : public TestClientWithConnectedSession {
public:
    TestClientObjDefsSingleFromFile() : objDefsFile_(detail::singleObjDefXML) {}
    virtual void SetUp() {
        daemon_.SetAdditionalOptions({ "--objDefs", objDefsFile_.GetFilename()});
        TestClientWithConnectedSession::SetUp();
    }
private:
    ObjDefsFile objDefsFile_;
};


TEST_F(TestClientObjDefsSingleFromFile, test_valid_file) {
    const AwaObjectDefinition * objectDefinition = AwaClientSession_GetObjectDefinition(session_, detail::EXPECTED_OBJECT_ID);
    EXPECT_TRUE(NULL != objectDefinition);

    // TODO: test definition is correct
}



class TestClientObjDefsFromFileNotFound : public TestClientWithConnectedSession {};

TEST_F(TestClientObjDefsFromFileNotFound, test_file_not_found) {
    const AwaObjectDefinition * objectDefinition = AwaClientSession_GetObjectDefinition(session_, detail::EXPECTED_OBJECT_ID);
    EXPECT_EQ(NULL, objectDefinition);
}


} // namespace Awa

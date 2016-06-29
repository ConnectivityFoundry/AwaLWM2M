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
          <Name>Test Resource</Name>
          <SerialisationName>TestResource</SerialisationName>
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
      <SerialisationName>TestObject4</SerialisationName>
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
        <!-- defaults -->
        <PropertyDefinition>
          <PropertyID>0</PropertyID>
          <Name>Test Resource 0</Name>
          <SerialisationName>TestResource0</SerialisationName>
          <!-- <DataType>None</DataType> -->
          <!-- <IsCollection>False</IsCollection> -->
          <!-- <IsMandatory>False</IsMandatory> -->
          <!-- <Access>None</Access> -->
        </PropertyDefinition>

        <!-- mandatory, singleton -->
        <PropertyDefinition>
          <PropertyID>42</PropertyID>
          <Name>Test Resource 42</Name>
          <SerialisationName>TestResource42</SerialisationName>
          <DataType>Integer</DataType>
          <IsCollection>False</IsCollection>
          <IsMandatory>True</IsMandatory>
          <Access>ReadWrite</Access>
        </PropertyDefinition>

        <!-- mandatory, multiple -->
        <PropertyDefinition>
          <PropertyID>43</PropertyID>
          <Name>Test Resource 43</Name>
          <SerialisationName>TestResource43</SerialisationName>
          <DataType>Float</DataType>
          <IsCollection>True</IsCollection>
          <IsMandatory>True</IsMandatory>
          <Access>Read</Access>
        </PropertyDefinition>

        <!-- optional, singleton, executable -->
        <PropertyDefinition>
          <PropertyID>44</PropertyID>
          <Name>Test Resource 44</Name>
          <SerialisationName>TestResource44</SerialisationName>
          <DataType>Float</DataType>
          <IsCollection>False</IsCollection>
          <IsMandatory>False</IsMandatory>
          <Access>Execute</Access>
        </PropertyDefinition>

        <!-- optional, multiple -->
        <PropertyDefinition>
          <PropertyID>43</PropertyID>
          <Name>Test Resource 45</Name>
          <SerialisationName>TestResource45</SerialisationName>
          <DataType>String</DataType>
          <IsCollection>True</IsCollection>
          <IsMandatory>False</IsMandatory>
          <Access>ReadWrite</Access>
        </PropertyDefinition>

      </Properties>
    </ObjectDefinition>
  </Items>
</ObjectDefinitions>
)";

static const char * singleObjDefXML = R"(
    <ObjectDefinition>
      <ObjectID>7301</ObjectID>
      <Name>Test Object 1</Name>
      <SerialisationName>TestObject1</SerialisationName>
      <Singleton>True</Singleton>
      <IsMandatory>True</IsMandatory>
      <Properties>
        <PropertyDefinition>
          <PropertyID>0</PropertyID>
          <Name>Test Resource</Name>
          <SerialisationName>TestResource</SerialisationName>
          <DataType>String</DataType>
          <IsCollection>False</IsCollection>
          <IsMandatory>False</IsMandatory>
          <Access>Read</Access>
        </PropertyDefinition>
      </Properties>
    </ObjectDefinition>
)";


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
//        if (std::remove(filename_.c_str()) < 0)
//        {
//            std::perror("Error deleting file");
//        }
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

static ::testing::AssertionResult expectNoResources(const AwaObjectDefinition * obj) {
    AwaResourceDefinitionIterator * resIt = AwaObjectDefinition_NewResourceDefinitionIterator(obj);
    ::testing::AssertionResult result = AwaResourceDefinitionIterator_Next(resIt) ? ::testing::AssertionFailure() << "Resources defined" : ::testing::AssertionSuccess();
    AwaResourceDefinitionIterator_Free(&resIt);
    return result;
}

static void checkResource(const AwaObjectDefinition * obj, AwaResourceID resID, const char * resName,
                          int minInstances, int maxInstances, AwaResourceType resType, AwaResourceOperations resOps) {
    const AwaResourceDefinition * res = AwaObjectDefinition_GetResourceDefinition(obj, resID);
    EXPECT_TRUE(NULL != res);
    EXPECT_EQ(resID, AwaResourceDefinition_GetID(res));
    EXPECT_STREQ(resName, AwaResourceDefinition_GetName(res));
    EXPECT_EQ(minInstances, AwaResourceDefinition_GetMinimumInstances(res));
    EXPECT_EQ(maxInstances, AwaResourceDefinition_GetMaximumInstances(res));
    EXPECT_EQ(resType, AwaResourceDefinition_GetType(res));
    EXPECT_EQ(resOps, AwaResourceDefinition_GetSupportedOperations(res));
}

#define EXPECT_RESOURCE(...) { SCOPED_TRACE(""); checkResource(__VA_ARGS__); }

TEST_F(TestClientObjDefsMultipleFromFile, test_valid_file) {

    // check definition of object 7301
    const AwaObjectDefinition * obj7301 = AwaClientSession_GetObjectDefinition(session_, 7301);
    ASSERT_TRUE(NULL != obj7301);

    EXPECT_EQ(7301, AwaObjectDefinition_GetID(obj7301));
    EXPECT_STREQ("TestObject1", AwaObjectDefinition_GetName(obj7301));          // should use serialisation name for object name
    EXPECT_EQ(1, AwaObjectDefinition_GetMinimumInstances(obj7301));
    EXPECT_EQ(1, AwaObjectDefinition_GetMinimumInstances(obj7301));
    EXPECT_FALSE(expectNoResources(obj7301));

    // check resources of 7301
    const AwaResourceDefinition * res7301_0 = AwaObjectDefinition_GetResourceDefinition(obj7301, 0);
    ASSERT_TRUE(NULL != res7301_0);
    EXPECT_EQ(0, AwaResourceDefinition_GetID(res7301_0));
    EXPECT_STREQ("TestResource", AwaResourceDefinition_GetName(res7301_0));      // should use serialisation name for resource name
    EXPECT_EQ(0, AwaResourceDefinition_GetMinimumInstances(res7301_0));
    EXPECT_EQ(1, AwaResourceDefinition_GetMaximumInstances(res7301_0));
    EXPECT_EQ(AwaResourceType_String, AwaResourceDefinition_GetType(res7301_0));
    EXPECT_EQ(AwaResourceOperations_ReadOnly, AwaResourceDefinition_GetSupportedOperations(res7301_0));

    // check definition of object 7302
    const AwaObjectDefinition * obj7302 = AwaClientSession_GetObjectDefinition(session_, 7302);
    ASSERT_TRUE(NULL != obj7302);
    EXPECT_EQ(7302, AwaObjectDefinition_GetID(obj7302));
    EXPECT_STREQ("TestObject2", AwaObjectDefinition_GetName(obj7302));          // should use serialisation name for object name
    EXPECT_EQ(1, AwaObjectDefinition_GetMinimumInstances(obj7302));
    EXPECT_EQ(AWA_MAX_ID, AwaObjectDefinition_GetMaximumInstances(obj7302));
    EXPECT_TRUE(expectNoResources(obj7302));

    // check definition of object 7303
    const AwaObjectDefinition * obj7303 = AwaClientSession_GetObjectDefinition(session_, 7303);
    ASSERT_TRUE(NULL != obj7303);
    EXPECT_EQ(7303, AwaObjectDefinition_GetID(obj7303));
    EXPECT_STREQ("TestObject3", AwaObjectDefinition_GetName(obj7303));          // should use serialisation name for object name
    EXPECT_EQ(0, AwaObjectDefinition_GetMinimumInstances(obj7303));
    EXPECT_EQ(AWA_MAX_ID, AwaObjectDefinition_GetMaximumInstances(obj7303));
    EXPECT_TRUE(expectNoResources(obj7303));

    // check definition of object 7304
    const AwaObjectDefinition * obj7304 = AwaClientSession_GetObjectDefinition(session_, 7304);
    ASSERT_TRUE(NULL != obj7304);
    EXPECT_EQ(7304, AwaObjectDefinition_GetID(obj7304));
    EXPECT_STREQ("TestObject4", AwaObjectDefinition_GetName(obj7304));          // should use serialisation name for object name
    EXPECT_EQ(0, AwaObjectDefinition_GetMinimumInstances(obj7304));
    EXPECT_EQ(1, AwaObjectDefinition_GetMaximumInstances(obj7304));
    EXPECT_TRUE(expectNoResources(obj7304));

    // check definition of object 7310
    const AwaObjectDefinition * obj7310 = AwaClientSession_GetObjectDefinition(session_, 7310);
    ASSERT_TRUE(NULL != obj7310);
    EXPECT_EQ(7310, AwaObjectDefinition_GetID(obj7310));
    EXPECT_STREQ("TestObject10", AwaObjectDefinition_GetName(obj7310));          // should use serialisation name for object name
    EXPECT_EQ(0, AwaObjectDefinition_GetMinimumInstances(obj7310));
    EXPECT_EQ(AWA_MAX_ID, AwaObjectDefinition_GetMaximumInstances(obj7310));
    EXPECT_FALSE(expectNoResources(obj7310));

    // check resources of 7310
    EXPECT_RESOURCE(obj7310, 0,  "TestResource0",  0, 1,          AwaResourceType_None,        AwaResourceOperations_None);
    EXPECT_RESOURCE(obj7310, 42, "TestResource42", 1, 1,          AwaResourceType_Integer,     AwaResourceOperations_ReadWrite);
    EXPECT_RESOURCE(obj7310, 43, "TestResource43", 1, AWA_MAX_ID, AwaResourceType_FloatArray,  AwaResourceOperations_ReadOnly);
    EXPECT_RESOURCE(obj7310, 44, "TestResource44", 0, 1,          AwaResourceType_Float,       AwaResourceOperations_Execute);
    EXPECT_RESOURCE(obj7310, 45, "TestResource45", 0, AWA_MAX_ID, AwaResourceType_StringArray, AwaResourceOperations_ReadWrite);

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

    // check definition of object 7301
    const AwaObjectDefinition * obj7301 = AwaClientSession_GetObjectDefinition(session_, 7301);
    ASSERT_TRUE(NULL != obj7301);

    EXPECT_EQ(7301, AwaObjectDefinition_GetID(obj7301));
    EXPECT_STREQ("TestObject1", AwaObjectDefinition_GetName(obj7301));          // should use serialisation name for object name
    EXPECT_EQ(1, AwaObjectDefinition_GetMinimumInstances(obj7301));
    EXPECT_EQ(1, AwaObjectDefinition_GetMinimumInstances(obj7301));
    EXPECT_FALSE(expectNoResources(obj7301));

    const AwaResourceDefinition * res7301_0 = AwaObjectDefinition_GetResourceDefinition(obj7301, 0);
    ASSERT_TRUE(NULL != res7301_0);
    EXPECT_EQ(0, AwaResourceDefinition_GetID(res7301_0));
    EXPECT_STREQ("TestResource", AwaResourceDefinition_GetName(res7301_0));      // should use serialisation name for resource name
    EXPECT_EQ(0, AwaResourceDefinition_GetMinimumInstances(res7301_0));
    EXPECT_EQ(1, AwaResourceDefinition_GetMaximumInstances(res7301_0));
    EXPECT_EQ(AwaResourceType_String, AwaResourceDefinition_GetType(res7301_0));
    EXPECT_EQ(AwaResourceOperations_ReadOnly, AwaResourceDefinition_GetSupportedOperations(res7301_0));
}

class TestClientObjDefsFromFileNotFound : public TestClientWithConnectedSession {
public:
    virtual void SetUp() {
        daemon_.SetAdditionalOptions({ "--objDefs", "does_not_exist"});
        TestClientWithConnectedSession::SetUp();
    }
};

TEST_F(TestClientObjDefsFromFileNotFound, test_file_not_found) {
    EXPECT_EQ(NULL, AwaClientSession_GetObjectDefinition(session_, 7301));
    EXPECT_EQ(NULL, AwaClientSession_GetObjectDefinition(session_, 7302));
    EXPECT_EQ(NULL, AwaClientSession_GetObjectDefinition(session_, 7303));
    EXPECT_EQ(NULL, AwaClientSession_GetObjectDefinition(session_, 7304));
    EXPECT_EQ(NULL, AwaClientSession_GetObjectDefinition(session_, 7310));
}


} // namespace Awa

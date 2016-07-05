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
#include <memory>

#include "awa/server.h"
#include "support/support.h"
#include "support/file_resource.h"

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
          <DataType>None</DataType>
          <IsCollection>False</IsCollection>
          <IsMandatory>False</IsMandatory>
          <Access>Execute</Access>
        </PropertyDefinition>

        <!-- optional, multiple -->
        <PropertyDefinition>
          <PropertyID>45</PropertyID>
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

// an executable resource must be of type 'None'
#if 0
static const char * badObjDefXML = R"(
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
          <Access>Execute</Access>
        </PropertyDefinition>
      </Properties>
    </ObjectDefinition>
)";
#endif //0

static const char * multipleFile1 = R"(
<ObjectDefinitions>
  <Items>
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
  </Items>
</ObjectDefinitions>
)";

static const char * multipleFile2 = R"(
    <ObjectDefinition>
      <ObjectID>7302</ObjectID>
      <Name>Test Object 2</Name>
      <SerialisationName>TestObject2</SerialisationName>
      <Singleton>False</Singleton>
      <IsMandatory>False</IsMandatory>
      <Properties>
        <PropertyDefinition>
          <PropertyID>100</PropertyID>
          <Name>Test Resource 100</Name>
          <SerialisationName>TestResource100</SerialisationName>
          <DataType>Integer</DataType>
          <IsCollection>False</IsCollection>
          <IsMandatory>False</IsMandatory>
          <Access>ReadWrite</Access>
        </PropertyDefinition>
      </Properties>
    </ObjectDefinition>
)";

} // namespace detail

class TestServerObjDefsMultipleFromFile : public TestServerWithConnectedSession {
public:
    TestServerObjDefsMultipleFromFile() : objDefsFile_(detail::multipleObjDefsXML) {}
    virtual void SetUp() {
        daemon_.SetAdditionalOptions({ "--objDefs", objDefsFile_.GetFilename()});
        TestServerWithConnectedSession::SetUp();
    }
private:
    FileResource objDefsFile_;
};

static size_t CountResources(const AwaObjectDefinition * obj) {
    AwaResourceDefinitionIterator * resIt = AwaObjectDefinition_NewResourceDefinitionIterator(obj);
    size_t i = 0;
    while (AwaResourceDefinitionIterator_Next(resIt))
    {
        ++i;
    }
    AwaResourceDefinitionIterator_Free(&resIt);
    return i;
}

static void CheckObject(const AwaServerSession * session, AwaObjectID objID, const char * objName, int minInstances, int maxInstances, int numResources) {
    const AwaObjectDefinition * obj = AwaServerSession_GetObjectDefinition(session, objID);
    EXPECT_TRUE(NULL != obj);
    EXPECT_EQ(objID, AwaObjectDefinition_GetID(obj));
    EXPECT_STREQ(objName, AwaObjectDefinition_GetName(obj));
    EXPECT_EQ(minInstances, AwaObjectDefinition_GetMinimumInstances(obj));
    EXPECT_EQ(maxInstances, AwaObjectDefinition_GetMaximumInstances(obj));
    if (numResources >= 0) {
        EXPECT_EQ(static_cast<size_t>(numResources), CountResources(obj));
    }
}

static void CheckResource(const AwaObjectDefinition * obj, AwaResourceID resID, const char * resName,
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

static void CheckResource(AwaServerSession * session, AwaObjectID objID, AwaResourceID resID, const char * resName,
                          int minInstances, int maxInstances, AwaResourceType resType, AwaResourceOperations resOps) {
    const AwaObjectDefinition * obj = AwaServerSession_GetObjectDefinition(session, objID);
    return CheckResource(obj, resID, resName, minInstances, maxInstances, resType, resOps);
}


#define EXPECT_RESOURCE(...) { SCOPED_TRACE(""); CheckResource(__VA_ARGS__); }
#define EXPECT_OBJECT(...) { SCOPED_TRACE(""); CheckObject(__VA_ARGS__); }

TEST_F(TestServerObjDefsMultipleFromFile, test_valid_file) {

    // check definition of object 7301
    EXPECT_OBJECT(session_, 7301, "TestObject1", 1, 1, 1);
    EXPECT_RESOURCE(session_, 7301, 0,  "TestResource",  0, 1,          AwaResourceType_String,        AwaResourceOperations_ReadOnly);

    EXPECT_OBJECT(session_, 7302, "TestObject2", 1, AWA_MAX_ID, 0);

    EXPECT_OBJECT(session_, 7303, "TestObject3", 0, AWA_MAX_ID, 0);

    EXPECT_OBJECT(session_, 7304, "TestObject4", 0, 1, 0);

    EXPECT_OBJECT(session_, 7310, "TestObject10", 0, AWA_MAX_ID, 5);
    EXPECT_RESOURCE(session_, 7310, 0,  "TestResource0",  0, 1,          AwaResourceType_None,        AwaResourceOperations_None);
    EXPECT_RESOURCE(session_, 7310, 42, "TestResource42", 1, 1,          AwaResourceType_Integer,     AwaResourceOperations_ReadWrite);
    EXPECT_RESOURCE(session_, 7310, 43, "TestResource43", 1, AWA_MAX_ID, AwaResourceType_FloatArray,  AwaResourceOperations_ReadOnly);
    EXPECT_RESOURCE(session_, 7310, 44, "TestResource44", 0, 1,          AwaResourceType_None,        AwaResourceOperations_Execute);
    EXPECT_RESOURCE(session_, 7310, 45, "TestResource45", 0, AWA_MAX_ID, AwaResourceType_StringArray, AwaResourceOperations_ReadWrite);
}



class TestServerObjDefsSingleFromFile : public TestServerWithConnectedSession {
public:
    TestServerObjDefsSingleFromFile() : objDefsFile_(detail::singleObjDefXML) {}
    virtual void SetUp() {
        daemon_.SetAdditionalOptions({ "--objDefs", objDefsFile_.GetFilename()});
        TestServerWithConnectedSession::SetUp();
    }
private:
    FileResource objDefsFile_;
};


TEST_F(TestServerObjDefsSingleFromFile, test_valid_file) {

    EXPECT_OBJECT(session_, 7301, "TestObject1", 1, 1, 1);
    EXPECT_RESOURCE(session_, 7301, 0,  "TestResource",  0, 1,          AwaResourceType_String,        AwaResourceOperations_ReadOnly);
}

class TestServerObjDefsFromFileNotFound : public TestServerWithConnectedSession {
public:
    virtual void SetUp() {
        daemon_.SetAdditionalOptions({ "--objDefs", "does_not_exist"});
        TestServerWithConnectedSession::SetUp();
    }
};

// Disabled: missing file causes daemon to abort, not able to test via API
TEST_F(TestServerObjDefsFromFileNotFound, DISABLED_test_file_not_found) {
    EXPECT_EQ(NULL, AwaServerSession_GetObjectDefinition(session_, 7301));
    EXPECT_EQ(NULL, AwaServerSession_GetObjectDefinition(session_, 7302));
    EXPECT_EQ(NULL, AwaServerSession_GetObjectDefinition(session_, 7303));
    EXPECT_EQ(NULL, AwaServerSession_GetObjectDefinition(session_, 7304));
    EXPECT_EQ(NULL, AwaServerSession_GetObjectDefinition(session_, 7310));
}


class TestServerObjDefsFromMultipleFiles : public TestServerWithConnectedSession {
public:
    TestServerObjDefsFromMultipleFiles() : objDefsFiles_() {
        auto contents = { detail::multipleFile1, detail::multipleFile2 };
        for (auto & it : contents)
        {
            auto p = FileResourcePtr(new FileResource(it));
            objDefsFiles_.push_back(std::move(p));
        }
    }
    virtual void SetUp() {
        std::vector<std::string> additionalOptions;
        for (auto & it : objDefsFiles_) {
            additionalOptions.push_back("--objDefs");
            additionalOptions.push_back(it->GetFilename());
        }
        daemon_.SetAdditionalOptions(additionalOptions);
        TestServerWithConnectedSession::SetUp();
    }
private:
    typedef std::unique_ptr<FileResource> FileResourcePtr;
    std::vector<FileResourcePtr> objDefsFiles_;
};

TEST_F(TestServerObjDefsFromMultipleFiles, multiple_files) {

    // check definition of object 7301
    EXPECT_OBJECT(session_, 7301, "TestObject1", 1, 1, 1);
    EXPECT_RESOURCE(session_, 7301, 0,  "TestResource",  0, 1,          AwaResourceType_String,        AwaResourceOperations_ReadOnly);

    // check definition of object 7302
    EXPECT_OBJECT(session_, 7302, "TestObject2", 0, AWA_MAX_ID, 1);
    EXPECT_RESOURCE(session_, 7302, 100,  "TestResource100",  0, 1,     AwaResourceType_Integer,        AwaResourceOperations_ReadWrite);
}

} // namespace Awa

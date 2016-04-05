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
#include "arrays.h"
#include "support/support.h"

namespace Awa {

class TestArray : public TestClientBase {};

////
// Integer Arrays
////

TEST_F(TestArray, AwaIntegerArray_New_and_Free_valid_inputs)
{
    AwaIntegerArray * array = AwaIntegerArray_New();
    ASSERT_TRUE(NULL != array);
    AwaIntegerArray_Free(&array);
    ASSERT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaIntegerArray_SetValue_GetValue_valid_inputs)
{
    AwaIntegerArray * array = AwaIntegerArray_New();
    EXPECT_TRUE(NULL != array);

    AwaIntegerArray_SetValue(array, 0, 123456);
    ASSERT_EQ(123456, AwaIntegerArray_GetValue(array, 0));

    AwaIntegerArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaIntegerArray_IsValid_valid_inputs)
{
    AwaIntegerArray * array = AwaIntegerArray_New();
    EXPECT_TRUE(NULL != array);

    AwaIntegerArray_SetValue(array, 0, 123456);
    ASSERT_EQ(true, AwaIntegerArray_IsValid(array, 0));
    ASSERT_EQ(0, AwaIntegerArray_IsValid(array, 1));

    AwaIntegerArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaIntegerArray_SetValue_GetValue_invalid_inputs)
{
    AwaIntegerArray_SetValue(NULL, 0, 123456);
    ASSERT_EQ(0, AwaIntegerArray_GetValue(NULL, 0));
}

TEST_F(TestArray, AwaIntegerArray_DeleteValue_valid_inputs)
{
    AwaIntegerArray * array = AwaIntegerArray_New();
    EXPECT_TRUE(NULL != array);

    AwaIntegerArray_SetValue(array, 0, 123456);

    AwaIntegerArray_DeleteValue(array, 0);

    ASSERT_EQ(0, AwaIntegerArray_GetValue(array, 0));

    AwaIntegerArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaIntegerArray_DeleteValue_invalid_inputs)
{
    AwaIntegerArray * array = AwaIntegerArray_New();
    EXPECT_TRUE(NULL != array);

    AwaIntegerArray_SetValue(array, 0, 123456);

    AwaIntegerArray_DeleteValue(NULL, 0);
    AwaIntegerArray_DeleteValue(array, 1);

    ASSERT_EQ(123456, AwaIntegerArray_GetValue(array, 0));

    AwaIntegerArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaIntegerArray_GetValueCount_valid_inputs)
{
    AwaIntegerArray * array = AwaIntegerArray_New();
    EXPECT_TRUE(NULL != array);

    AwaIntegerArray_SetValue(array, 0, 123456);

    AwaIntegerArray_SetValue(array, 1, 123456);

    AwaIntegerArray_SetValue(array, 2, 123456);

    ASSERT_EQ(static_cast<size_t>(3), AwaIntegerArray_GetValueCount(array));

    AwaIntegerArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaIntegerArray_GetValueCount_invalid_inputs)
{
    ASSERT_EQ(static_cast<size_t>(0), AwaIntegerArray_GetValueCount(NULL));
}

TEST_F(TestArray, AwaIntegerArray_DeleteValue_and_GetValueCount_valid)
{
    AwaIntegerArray * array = AwaIntegerArray_New();
    EXPECT_TRUE(NULL != array);

    AwaIntegerArray_SetValue(array, 0, 123456);

    AwaIntegerArray_SetValue(array, 1, 123456);

    AwaIntegerArray_SetValue(array, 2, 123456);

    AwaIntegerArray_DeleteValue(array, 0);

    ASSERT_EQ(static_cast<size_t>(2), AwaIntegerArray_GetValueCount(array));

    AwaIntegerArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaIntegerArray_GetValueCount_valid_after_replace)
{
    AwaIntegerArray * array = AwaIntegerArray_New();
    EXPECT_TRUE(NULL != array);

    AwaIntegerArray_SetValue(array, 0, 123456);

    AwaIntegerArray_SetValue(array, 1, 123456);

    AwaIntegerArray_SetValue(array, 1, 654321);

    ASSERT_EQ(static_cast<size_t>(2), AwaIntegerArray_GetValueCount(array));

    AwaIntegerArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}


TEST_F(TestArray, AwaIntegerArrayIterator_New_and_Free_valid_inputs)
{
    AwaIntegerArray * array = AwaIntegerArray_New();
    EXPECT_TRUE(NULL != array);

    AwaIntegerArrayIterator * iterator = AwaIntegerArray_NewIntegerArrayIterator(array);
    ASSERT_TRUE(NULL != iterator);

    AwaIntegerArrayIterator_Free(&iterator);
    ASSERT_TRUE(NULL == iterator);

    AwaIntegerArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaIntegerArrayIterator_New_invalid_inputs)
{
    AwaIntegerArrayIterator * iterator = AwaIntegerArray_NewIntegerArrayIterator(NULL);
    ASSERT_TRUE(NULL == iterator);
}


TEST_F(TestArray, AwaIntegerArrayIterator_Set_Get_values)
{
    AwaIntegerArray * array = AwaIntegerArray_New();
    EXPECT_TRUE(NULL != array);

    for (int i = 0; i < 1000; i+=5)
    {
        AwaIntegerArray_SetValue(array, i, i*2);
    }

    AwaIntegerArrayIterator * iterator = AwaIntegerArray_NewIntegerArrayIterator(array);
    EXPECT_TRUE(NULL != iterator);

    for (int i = 0; i < 1000; i+=5)
    {
        ASSERT_TRUE(AwaIntegerArrayIterator_Next(iterator));
        EXPECT_EQ(static_cast<size_t>(i), AwaIntegerArrayIterator_GetIndex(iterator));
        EXPECT_EQ(i*2, AwaIntegerArrayIterator_GetValue(iterator));
//        ASSERT_EQ(sizeof(AwaInteger), ArrayIterator_GetValueLength(iterator));
    }

    ASSERT_FALSE(AwaIntegerArrayIterator_Next(iterator));

    AwaIntegerArrayIterator_Free(&iterator);
    EXPECT_TRUE(NULL == iterator);

    AwaIntegerArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}


////
// Float Arrays
////

TEST_F(TestArray, AwaFloatArray_New_and_Free_valid_inputs)
{
    AwaFloatArray * array = AwaFloatArray_New();
    ASSERT_TRUE(NULL != array);
    AwaFloatArray_Free(&array);
    ASSERT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaFloatArray_SetValue_GetValue_valid_inputs)
{
    AwaFloatArray * array = AwaFloatArray_New();
    EXPECT_TRUE(NULL != array);

    AwaFloatArray_SetValue(array, 0, 123.456);
    ASSERT_EQ(123.456, AwaFloatArray_GetValue(array, 0));

    AwaFloatArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaFloatArray_IsValid_valid_inputs)
{
    AwaFloatArray * array = AwaFloatArray_New();
    EXPECT_TRUE(NULL != array);

    AwaFloatArray_SetValue(array, 0, 123.456);
    ASSERT_EQ(true, AwaFloatArray_IsValid(array, 0));
    ASSERT_EQ(0, AwaFloatArray_IsValid(array, 1));

    AwaFloatArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaFloatArray_SetValue_GetValue_invalid_inputs)
{
    AwaFloatArray_SetValue(NULL, 0, 123.456);
    ASSERT_EQ(0, AwaFloatArray_GetValue(NULL, 0));
}

TEST_F(TestArray, AwaFloatArray_DeleteValue_valid_inputs)
{
    AwaFloatArray * array = AwaFloatArray_New();
    EXPECT_TRUE(NULL != array);

    AwaFloatArray_SetValue(array, 0, 123.456);

    AwaFloatArray_DeleteValue(array, 0);

    ASSERT_EQ(0, AwaFloatArray_GetValue(array, 0));

    AwaFloatArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaFloatArray_DeleteValue_invalid_inputs)
{
    AwaFloatArray * array = AwaFloatArray_New();
    EXPECT_TRUE(NULL != array);

    AwaFloatArray_SetValue(array, 0, 123.456);

    AwaFloatArray_DeleteValue(NULL, 0);
    AwaFloatArray_DeleteValue(array, 1);

    ASSERT_EQ(123.456, AwaFloatArray_GetValue(array, 0));

    AwaFloatArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaFloatArray_GetValueCount_valid_inputs)
{
    AwaFloatArray * array = AwaFloatArray_New();
    EXPECT_TRUE(NULL != array);

    AwaFloatArray_SetValue(array, 0, 123.456);

    AwaFloatArray_SetValue(array, 1, 123.456);

    AwaFloatArray_SetValue(array, 2, 123.456);

    ASSERT_EQ(static_cast<size_t>(3), AwaFloatArray_GetValueCount(array));

    AwaFloatArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaFloatArray_GetValueCount_invalid_inputs)
{
    ASSERT_EQ(static_cast<size_t>(0), AwaFloatArray_GetValueCount(NULL));
}

TEST_F(TestArray, AwaFloatArray_DeleteValue_and_GetValueCount_valid)
{
    AwaFloatArray * array = AwaFloatArray_New();
    EXPECT_TRUE(NULL != array);

    AwaFloatArray_SetValue(array, 0, 123.456);

    AwaFloatArray_SetValue(array, 1, 123.456);

    AwaFloatArray_SetValue(array, 2, 123.456);

    AwaFloatArray_DeleteValue(array, 0);

    ASSERT_EQ(static_cast<size_t>(2), AwaFloatArray_GetValueCount(array));

    AwaFloatArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaFloatArray_GetValueCount_valid_after_replace)
{
    AwaFloatArray * array = AwaFloatArray_New();
    EXPECT_TRUE(NULL != array);

    AwaFloatArray_SetValue(array, 0, 123.456);

    AwaFloatArray_SetValue(array, 1, 123.456);

    AwaFloatArray_SetValue(array, 1, 654.321);

    ASSERT_EQ(static_cast<size_t>(2), AwaFloatArray_GetValueCount(array));

    AwaFloatArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}


TEST_F(TestArray, AwaFloatArrayIterator_New_and_Free_valid_inputs)
{
    AwaFloatArray * array = AwaFloatArray_New();
    EXPECT_TRUE(NULL != array);

    AwaFloatArrayIterator * iterator = AwaFloatArray_NewFloatArrayIterator(array);
    ASSERT_TRUE(NULL != iterator);

    AwaFloatArrayIterator_Free(&iterator);
    ASSERT_TRUE(NULL == iterator);

    AwaFloatArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaFloatArrayIterator_New_invalid_inputs)
{
    AwaFloatArrayIterator * iterator = AwaFloatArray_NewFloatArrayIterator(NULL);
    ASSERT_TRUE(NULL == iterator);
}


TEST_F(TestArray, AwaFloatArrayIterator_Set_Get_values)
{
    AwaFloatArray * array = AwaFloatArray_New();
    EXPECT_TRUE(NULL != array);

    for(int i = 1; i < 1000; i+=5)
    {
        AwaFloatArray_SetValue(array, i, 100000.0 / i*2);
    }

    AwaFloatArrayIterator * iterator = AwaFloatArray_NewFloatArrayIterator(array);
    EXPECT_TRUE(NULL != iterator);

    for(int i = 1; i < 1000; i+=5)
    {
        ASSERT_TRUE(AwaFloatArrayIterator_Next(iterator));
        ASSERT_EQ(static_cast<size_t>(i), AwaFloatArrayIterator_GetIndex(iterator));
        ASSERT_EQ(100000.0 / i*2, AwaFloatArrayIterator_GetValue(iterator));
//        ASSERT_EQ(sizeof(AwaFloat), ArrayIterator_GetValueLength(iterator));
    }

    ASSERT_FALSE(AwaFloatArrayIterator_Next(iterator));

    AwaFloatArrayIterator_Free(&iterator);
    EXPECT_TRUE(NULL == iterator);

    AwaFloatArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

////
// Time Arrays
////

TEST_F(TestArray, AwaTimeArray_New_and_Free_valid_inputs)
{
    AwaTimeArray * array = AwaTimeArray_New();
    ASSERT_TRUE(NULL != array);
    AwaTimeArray_Free(&array);
    ASSERT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaTimeArray_SetValue_GetValue_valid_inputs)
{
    AwaTimeArray * array = AwaTimeArray_New();
    EXPECT_TRUE(NULL != array);

    AwaTimeArray_SetValue(array, 0, 123456);
    ASSERT_EQ(123456, AwaTimeArray_GetValue(array, 0));

    AwaTimeArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaTimeArray_IsValid_valid_inputs)
{
    AwaTimeArray * array = AwaTimeArray_New();
    EXPECT_TRUE(NULL != array);

    AwaTimeArray_SetValue(array, 0, 123456);
    ASSERT_EQ(true, AwaTimeArray_IsValid(array, 0));
    ASSERT_EQ(0, AwaTimeArray_IsValid(array, 1));

    AwaTimeArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaTimeArray_SetValue_GetValue_invalid_inputs)
{
    AwaTimeArray_SetValue(NULL, 0, 123456);
    ASSERT_EQ(0, AwaTimeArray_GetValue(NULL, 0));
}

TEST_F(TestArray, AwaTimeArray_DeleteValue_valid_inputs)
{
    AwaTimeArray * array = AwaTimeArray_New();
    EXPECT_TRUE(NULL != array);

    AwaTimeArray_SetValue(array, 0, 123456);

    AwaTimeArray_DeleteValue(array, 0);

    ASSERT_EQ(0, AwaTimeArray_GetValue(array, 0));

    AwaTimeArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaTimeArray_DeleteValue_invalid_inputs)
{
    AwaTimeArray * array = AwaTimeArray_New();
    EXPECT_TRUE(NULL != array);

    AwaTimeArray_SetValue(array, 0, 123456);

    AwaTimeArray_DeleteValue(NULL, 0);
    AwaTimeArray_DeleteValue(array, 1);

    ASSERT_EQ(123456, AwaTimeArray_GetValue(array, 0));

    AwaTimeArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaTimeArray_GetValueCount_valid_inputs)
{
    AwaTimeArray * array = AwaTimeArray_New();
    EXPECT_TRUE(NULL != array);

    AwaTimeArray_SetValue(array, 0, 123456);

    AwaTimeArray_SetValue(array, 1, 123456);

    AwaTimeArray_SetValue(array, 2, 123456);

    ASSERT_EQ(static_cast<size_t>(3), AwaTimeArray_GetValueCount(array));

    AwaTimeArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaTimeArray_GetValueCount_invalid_inputs)
{
    ASSERT_EQ(static_cast<size_t>(0), AwaTimeArray_GetValueCount(NULL));
}

TEST_F(TestArray, AwaTimeArray_DeleteValue_and_GetValueCount_valid)
{
    AwaTimeArray * array = AwaTimeArray_New();
    EXPECT_TRUE(NULL != array);

    AwaTimeArray_SetValue(array, 0, 123456);

    AwaTimeArray_SetValue(array, 1, 123456);

    AwaTimeArray_SetValue(array, 2, 123456);

    AwaTimeArray_DeleteValue(array, 0);

    ASSERT_EQ(static_cast<size_t>(2), AwaTimeArray_GetValueCount(array));

    AwaTimeArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaTimeArray_GetValueCount_valid_after_replace)
{
    AwaTimeArray * array = AwaTimeArray_New();
    EXPECT_TRUE(NULL != array);

    AwaTimeArray_SetValue(array, 0, 123456);

    AwaTimeArray_SetValue(array, 1, 123456);

    AwaTimeArray_SetValue(array, 1, 654321);

    ASSERT_EQ(static_cast<size_t>(2), AwaTimeArray_GetValueCount(array));

    AwaTimeArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}


TEST_F(TestArray, AwaTimeArrayIterator_New_and_Free_valid_inputs)
{
    AwaTimeArray * array = AwaTimeArray_New();
    EXPECT_TRUE(NULL != array);

    AwaTimeArrayIterator * iterator = AwaTimeArray_NewTimeArrayIterator(array);
    ASSERT_TRUE(NULL != iterator);

    AwaTimeArrayIterator_Free(&iterator);
    ASSERT_TRUE(NULL == iterator);

    AwaTimeArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaTimeArrayIterator_New_invalid_inputs)
{
    AwaTimeArrayIterator * iterator = AwaTimeArray_NewTimeArrayIterator(NULL);
    ASSERT_TRUE(NULL == iterator);
}


TEST_F(TestArray, AwaTimeArrayIterator_Set_Get_values)
{
    AwaTimeArray * array = AwaTimeArray_New();
    EXPECT_TRUE(NULL != array);

    for(int i = 0; i < 1000; i+=5)
    {
        AwaTimeArray_SetValue(array, i, i*2);
    }

    AwaTimeArrayIterator * iterator = AwaTimeArray_NewTimeArrayIterator(array);
    EXPECT_TRUE(NULL != iterator);

    for(int i = 0; i < 1000; i+=5)
    {
        ASSERT_TRUE(AwaTimeArrayIterator_Next(iterator));
        ASSERT_EQ(static_cast<size_t>(i), AwaTimeArrayIterator_GetIndex(iterator));
        ASSERT_EQ(i*2, AwaTimeArrayIterator_GetValue(iterator));
//        ASSERT_EQ(sizeof(AwaTime), ArrayIterator_GetValueLength(iterator));
    }

    ASSERT_FALSE(AwaTimeArrayIterator_Next(iterator));

    AwaTimeArrayIterator_Free(&iterator);
    EXPECT_TRUE(NULL == iterator);

    AwaTimeArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

////
// Boolean Arrays
////

TEST_F(TestArray, AwaBooleanArray_New_and_Free_valid_inputs)
{
    AwaBooleanArray * array = AwaBooleanArray_New();
    ASSERT_TRUE(NULL != array);
    AwaBooleanArray_Free(&array);
    ASSERT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaBooleanArray_SetValue_GetValue_valid_inputs)
{
    AwaBooleanArray * array = AwaBooleanArray_New();
    EXPECT_TRUE(NULL != array);

    AwaBooleanArray_SetValue(array, 0, true);
    ASSERT_EQ(true, AwaBooleanArray_IsValid(array, 0));
    ASSERT_EQ(true, AwaBooleanArray_GetValue(array, 0));

    AwaBooleanArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaBooleanArray_IsValid_valid_inputs)
{
    AwaBooleanArray * array = AwaBooleanArray_New();
    EXPECT_TRUE(NULL != array);

    AwaBooleanArray_SetValue(array, 0, true);
    ASSERT_EQ(true, AwaBooleanArray_IsValid(array, 0));
    ASSERT_EQ(0, AwaBooleanArray_IsValid(array, 1));

    AwaBooleanArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaBooleanArray_SetValue_GetValue_invalid_inputs)
{
    AwaBooleanArray_SetValue(NULL, 0, true);
    ASSERT_EQ(0, AwaBooleanArray_IsValid(NULL, 0));
    ASSERT_EQ(0, AwaBooleanArray_GetValue(NULL, 0));
}

TEST_F(TestArray, AwaBooleanArray_DeleteValue_valid_inputs)
{
    AwaBooleanArray * array = AwaBooleanArray_New();
    EXPECT_TRUE(NULL != array);

    AwaBooleanArray_SetValue(array, 0, true);

    AwaBooleanArray_DeleteValue(array, 0);

    ASSERT_EQ(0, AwaBooleanArray_GetValue(array, 0));

    AwaBooleanArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaBooleanArray_DeleteValue_invalid_inputs)
{
    AwaBooleanArray * array = AwaBooleanArray_New();
    EXPECT_TRUE(NULL != array);

    AwaBooleanArray_SetValue(array, 0, true);

    AwaBooleanArray_DeleteValue(NULL, 0);
    AwaBooleanArray_DeleteValue(array, 1);

    ASSERT_EQ(true, AwaBooleanArray_GetValue(array, 0));

    AwaBooleanArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaBooleanArray_GetValueCount_valid_inputs)
{
    AwaBooleanArray * array = AwaBooleanArray_New();
    EXPECT_TRUE(NULL != array);

    AwaBooleanArray_SetValue(array, 0, true);

    AwaBooleanArray_SetValue(array, 1, true);

    AwaBooleanArray_SetValue(array, 2, true);

    ASSERT_EQ(static_cast<size_t>(3), AwaBooleanArray_GetValueCount(array));

    AwaBooleanArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaBooleanArray_GetValueCount_invalid_inputs)
{
    ASSERT_EQ(static_cast<size_t>(0), AwaBooleanArray_GetValueCount(NULL));
}

TEST_F(TestArray, AwaBooleanArray_DeleteValue_and_GetValueCount_valid)
{
    AwaBooleanArray * array = AwaBooleanArray_New();
    EXPECT_TRUE(NULL != array);

    AwaBooleanArray_SetValue(array, 0, true);

    AwaBooleanArray_SetValue(array, 1, true);

    AwaBooleanArray_SetValue(array, 2, true);

    AwaBooleanArray_DeleteValue(array, 0);

    ASSERT_EQ(static_cast<size_t>(2), AwaBooleanArray_GetValueCount(array));

    AwaBooleanArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaBooleanArray_GetValueCount_valid_after_replace)
{
    AwaBooleanArray * array = AwaBooleanArray_New();
    EXPECT_TRUE(NULL != array);

    AwaBooleanArray_SetValue(array, 0, true);

    AwaBooleanArray_SetValue(array, 1, true);

    AwaBooleanArray_SetValue(array, 1, false);

    ASSERT_EQ(static_cast<size_t>(2), AwaBooleanArray_GetValueCount(array));

    AwaBooleanArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}


TEST_F(TestArray, AwaBooleanArrayIterator_New_and_Free_valid_inputs)
{
    AwaBooleanArray * array = AwaBooleanArray_New();
    EXPECT_TRUE(NULL != array);

    AwaBooleanArrayIterator * iterator = AwaBooleanArray_NewBooleanArrayIterator(array);
    ASSERT_TRUE(NULL != iterator);

    AwaBooleanArrayIterator_Free(&iterator);
    ASSERT_TRUE(NULL == iterator);

    AwaBooleanArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaBooleanArrayIterator_New_invalid_inputs)
{
    AwaBooleanArrayIterator * iterator = AwaBooleanArray_NewBooleanArrayIterator(NULL);
    ASSERT_TRUE(NULL == iterator);
}


TEST_F(TestArray, AwaBooleanArrayIterator_Set_Get_values)
{
    AwaBooleanArray * array = AwaBooleanArray_New();
    EXPECT_TRUE(NULL != array);

    for(int i = 0; i < 1000; i+=5)
    {
        AwaBooleanArray_SetValue(array, i, (i % 2 == 0) ? true : false);
    }

    AwaBooleanArrayIterator * iterator = AwaBooleanArray_NewBooleanArrayIterator(array);
    EXPECT_TRUE(NULL != iterator);

    for(int i = 0; i < 1000; i+=5)
    {
        ASSERT_TRUE(AwaBooleanArrayIterator_Next(iterator));
        ASSERT_EQ(static_cast<size_t>(i), AwaBooleanArrayIterator_GetIndex(iterator));
        ASSERT_EQ((i % 2 == 0) ? true : false, AwaBooleanArrayIterator_GetValue(iterator));
//        ASSERT_EQ(sizeof(AwaBoolean), ArrayIterator_GetValueLength(iterator));
    }

    ASSERT_FALSE(AwaBooleanArrayIterator_Next(iterator));

    AwaBooleanArrayIterator_Free(&iterator);
    EXPECT_TRUE(NULL == iterator);

    AwaBooleanArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

////
// String Arrays
////

TEST_F(TestArray, AwaStringArray_New_and_Free_valid_inputs)
{
    AwaStringArray * array = AwaStringArray_New();
    ASSERT_TRUE(NULL != array);
    AwaStringArray_Free(&array);
    ASSERT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaStringArray_SetValue_GetValue_valid_inputs)
{
    AwaStringArray * array = AwaStringArray_New();
    EXPECT_TRUE(NULL != array);

    AwaStringArray_SetValueAsCString(array, 0, "Hello LWM2M");
    ASSERT_EQ(0, strcmp("Hello LWM2M", AwaStringArray_GetValueAsCString(array, 0)));
    printf("%s\n", AwaStringArray_GetValueAsCString(array, 0));
    AwaStringArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaStringArray_IsValid_valid_inputs)
{
    AwaStringArray * array = AwaStringArray_New();
    EXPECT_TRUE(NULL != array);

    AwaStringArray_SetValueAsCString(array, 0, "Hello LWM2M");
    ASSERT_EQ(true, AwaStringArray_IsValid(array, 0));
    ASSERT_EQ(0, AwaStringArray_IsValid(array, 1));

    AwaStringArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaStringArray_SetValue_GetValue_invalid_inputs)
{
    AwaStringArray_SetValueAsCString(NULL, 0, "Testing 123456");
    ASSERT_EQ(0, AwaStringArray_GetValueAsCString(NULL, 0));
}

TEST_F(TestArray, AwaStringArray_DeleteValue_valid_inputs)
{
    AwaStringArray * array = AwaStringArray_New();
    EXPECT_TRUE(NULL != array);

    AwaStringArray_SetValueAsCString(array, 0, "Testing 123456");

    AwaStringArray_DeleteValue(array, 0);

    ASSERT_EQ(NULL, AwaStringArray_GetValueAsCString(array, 0));

    AwaStringArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaStringArray_DeleteValue_invalid_inputs)
{
    AwaStringArray * array = AwaStringArray_New();
    EXPECT_TRUE(NULL != array);

    AwaStringArray_SetValueAsCString(array, 0, "Testing 123456");

    AwaStringArray_DeleteValue(NULL, 0);
    AwaStringArray_DeleteValue(array, 1);

    ASSERT_EQ(0, strcmp("Testing 123456", AwaStringArray_GetValueAsCString(array, 0)));

    AwaStringArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaStringArray_GetValueCount_valid_inputs)
{
    AwaStringArray * array = AwaStringArray_New();
    EXPECT_TRUE(NULL != array);

    AwaStringArray_SetValueAsCString(array, 0, "Testing 123456");

    AwaStringArray_SetValueAsCString(array, 1, "Testing 123456");

    AwaStringArray_SetValueAsCString(array, 2, "Testing 123456");

    ASSERT_EQ(static_cast<size_t>(3), AwaStringArray_GetValueCount(array));

    AwaStringArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaStringArray_GetValueCount_invalid_inputs)
{
    ASSERT_EQ(static_cast<size_t>(0), AwaStringArray_GetValueCount(NULL));
}

TEST_F(TestArray, AwaStringArray_DeleteValue_and_GetValueCount_valid)
{
    AwaStringArray * array = AwaStringArray_New();
    EXPECT_TRUE(NULL != array);

    AwaStringArray_SetValueAsCString(array, 0, "Testing 123456");

    AwaStringArray_SetValueAsCString(array, 1, "Testing 123456");

    AwaStringArray_SetValueAsCString(array, 2, "Testing 123456");

    AwaStringArray_DeleteValue(array, 0);

    ASSERT_EQ(static_cast<size_t>(2), AwaStringArray_GetValueCount(array));

    AwaStringArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaStringArray_GetValueCount_valid_after_replace)
{
    AwaStringArray * array = AwaStringArray_New();
    EXPECT_TRUE(NULL != array);

    AwaStringArray_SetValueAsCString(array, 0, "Testing 123456");

    AwaStringArray_SetValueAsCString(array, 1, "Testing 123456");

    AwaStringArray_SetValueAsCString(array, 1, "Testing 654321");

    ASSERT_EQ(static_cast<size_t>(2), AwaStringArray_GetValueCount(array));

    AwaStringArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}


TEST_F(TestArray, AwaStringArrayIterator_New_and_Free_valid_inputs)
{
    AwaStringArray * array = AwaStringArray_New();
    EXPECT_TRUE(NULL != array);

    AwaCStringArrayIterator * iterator = AwaStringArray_NewCStringArrayIterator(array);
    ASSERT_TRUE(NULL != iterator);

    AwaCStringArrayIterator_Free(&iterator);
    ASSERT_TRUE(NULL == iterator);

    AwaStringArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaStringArrayIterator_New_invalid_inputs)
{
    AwaCStringArrayIterator * iterator = AwaStringArray_NewCStringArrayIterator(NULL);
    ASSERT_TRUE(NULL == iterator);
}


TEST_F(TestArray, AwaStringArrayIterator_Set_Get_values)
{
    AwaStringArray * array = AwaStringArray_New();
    EXPECT_TRUE(NULL != array);

    for(int i = 0; i < 1000; i+=5)
    {
        char temp [64];
        sprintf(temp, "%d",  i*2);
        AwaStringArray_SetValueAsCString(array, i, temp);
    }

    AwaCStringArrayIterator * iterator = AwaStringArray_NewCStringArrayIterator(array);
    EXPECT_TRUE(NULL != iterator);

    for(int i = 0; i < 1000; i+=5)
    {
        char temp [64];

        ASSERT_TRUE(AwaCStringArrayIterator_Next(iterator));
        ASSERT_EQ(static_cast<size_t>(i), AwaCStringArrayIterator_GetIndex(iterator));
        sprintf(temp, "%d",  i*2);
        ASSERT_EQ(0, strcmp(temp, AwaCStringArrayIterator_GetValueAsCString(iterator)));
//        ASSERT_EQ(strlen(temp) + 1, ArrayIterator_GetValueLength(iterator));
    }

    ASSERT_FALSE(AwaCStringArrayIterator_Next(iterator));

    AwaCStringArrayIterator_Free(&iterator);
    EXPECT_TRUE(NULL == iterator);

    AwaStringArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

////
// Opaque Arrays
////

TEST_F(TestArray, AwaOpaqueArray_New_and_Free_valid_inputs)
{
    AwaOpaqueArray * array = AwaOpaqueArray_New();
    ASSERT_TRUE(NULL != array);
    AwaOpaqueArray_Free(&array);
    ASSERT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaOpaqueArray_SetValue_GetValue_valid_inputs)
{
    AwaOpaqueArray * array = AwaOpaqueArray_New();
    EXPECT_TRUE(NULL != array);

    char value[16] = { 0, 1, 2, 3, 4, 5, 0, 6, 7, 8, 9, 10, 0, 0, 15, 16 };

    AwaOpaqueArray_SetValue(array, 0, ((AwaOpaque) { value, sizeof(value) }));

    AwaOpaque getOpaque = AwaOpaqueArray_GetValue(array, 0);
    ASSERT_EQ(sizeof(value), getOpaque.Size);
    ASSERT_EQ(0, memcmp(value, getOpaque.Data, sizeof(value)));

    AwaOpaqueArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaOpaqueArray_SetValue_GetValue_invalid_inputs)
{
    char value[16] = { 0, 1, 2, 3, 4, 5, 0, 6, 7, 8, 9, 10, 0, 0, 15, 16 };
    AwaOpaqueArray_SetValue(NULL, 0, ((AwaOpaque) { value, sizeof(value) }));
    AwaOpaqueArray_SetValue(NULL, 42, ((AwaOpaque) { value, sizeof(value) }));

    AwaOpaqueArray * array = AwaOpaqueArray_New();
    AwaOpaqueArray_SetValue(array, 0, ((AwaOpaque) { NULL, sizeof(value) }));
    AwaOpaqueArray_SetValue(array, 42, ((AwaOpaque) { value, 0 }));
    AwaOpaqueArray_Free(&array);
}

TEST_F(TestArray, AwaOpaqueArray_DeleteValue_valid_inputs)
{
    AwaOpaqueArray * array = AwaOpaqueArray_New();
    EXPECT_TRUE(NULL != array);

    char value[] = { 0, 1, 2, 3, 4, 5, 0, 6, 7, 8, 9, 10, 0, 0, 15, 16 };
    AwaOpaqueArray_SetValue(array, 0, ((AwaOpaque) { value, sizeof(value) }));

    ASSERT_EQ(true, AwaOpaqueArray_IsValid(array, 0));

    AwaOpaqueArray_DeleteValue(array, 0);

    ASSERT_EQ(0, AwaOpaqueArray_IsValid(array, 0));

    AwaOpaque getOpaque = AwaOpaqueArray_GetValue(array, 0);
    ASSERT_EQ(static_cast<size_t>(0), getOpaque.Size);
    ASSERT_EQ(NULL, getOpaque.Data);

    AwaOpaqueArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaOpaqueArray_DeleteValue_invalid_inputs)
{
    AwaOpaqueArray * array = AwaOpaqueArray_New();
    EXPECT_TRUE(NULL != array);

    char value[] = { 0, 1, 2, 3, 4, 5, 0, 6, 7, 8, 9, 10, 0, 0, 15, 16 };
    AwaOpaqueArray_SetValue(array, 0, ((AwaOpaque) { value, sizeof(value) }));

    AwaOpaqueArray_DeleteValue(NULL, 0);
    AwaOpaqueArray_DeleteValue(array, 1);

    AwaOpaque getOpaque = AwaOpaqueArray_GetValue(array, 0);
    ASSERT_EQ(sizeof(value), getOpaque.Size);
    ASSERT_EQ(0, memcmp(value, getOpaque.Data, sizeof(value)));

    AwaOpaqueArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaOpaqueArray_GetValueCount_valid_inputs)
{
    AwaOpaqueArray * array = AwaOpaqueArray_New();
    EXPECT_TRUE(NULL != array);

    char value[] = { 0, 1, 2, 3, 4, 5, 0, 6, 7, 8, 9, 10, 0, 0, 15, 16 };

    AwaOpaqueArray_SetValue(array, 0, ((AwaOpaque) { value, sizeof(value) }));
    AwaOpaqueArray_SetValue(array, 1, ((AwaOpaque) { value, sizeof(value) }));
    AwaOpaqueArray_SetValue(array, 2, ((AwaOpaque) { value, sizeof(value) }));

    ASSERT_EQ(static_cast<size_t>(3), AwaOpaqueArray_GetValueCount(array));

    AwaOpaqueArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaOpaqueArray_GetValueCount_invalid_inputs)
{
    ASSERT_EQ(static_cast<size_t>(0), AwaOpaqueArray_GetValueCount(NULL));
}

TEST_F(TestArray, AwaOpaqueArray_DeleteValue_and_GetValueCount_valid)
{
    AwaOpaqueArray * array = AwaOpaqueArray_New();
    EXPECT_TRUE(NULL != array);

    char value[] = { 0, 1, 2, 3, 4, 5, 0, 6, 7, 8, 9, 10, 0, 0, 15, 16 };

    AwaOpaqueArray_SetValue(array, 0, ((AwaOpaque) { value, sizeof(value) }));
    AwaOpaqueArray_SetValue(array, 1, ((AwaOpaque) { value, sizeof(value) }));
    AwaOpaqueArray_SetValue(array, 2, ((AwaOpaque) { value, sizeof(value) }));

    AwaOpaqueArray_DeleteValue(array, 0);

    ASSERT_EQ(static_cast<size_t>(2), AwaOpaqueArray_GetValueCount(array));

    AwaOpaqueArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaOpaqueArray_GetValue_valid_after_replace)
{
    AwaOpaqueArray * array = AwaOpaqueArray_New();
    EXPECT_TRUE(NULL != array);

    char value[] = { 0, 1, 2, 3, 4, 5, 0, 6, 7, 8, 9, 10, 0, 0, 15, 16 };
    char value2[] = { 16, 15, 0, 0, 0, 3, 5, 6, 7 };
    char value3[] = { 5, 6, 7, 5, 5 ,5 , 0, 0, 0, 127, 32, 54, 127, 54, 42 ,42 ,42 ,42, 100, 6, 5, 4 };

    AwaOpaqueArray_SetValue(array, 0, ((AwaOpaque) { value, sizeof(value) }));

    AwaOpaque getOpaque = AwaOpaqueArray_GetValue(array, 0);
    ASSERT_EQ(sizeof(value), getOpaque.Size);
    ASSERT_EQ(0, memcmp(value, getOpaque.Data, sizeof(value)));

    AwaOpaqueArray_SetValue(array, 0, ((AwaOpaque) { value2, sizeof(value2) }));

    getOpaque = AwaOpaqueArray_GetValue(array, 0);
    ASSERT_EQ(sizeof(value2), getOpaque.Size);
    ASSERT_EQ(0, memcmp(value2, getOpaque.Data, sizeof(value2)));

    AwaOpaqueArray_SetValue(array, 0, ((AwaOpaque) { value3, sizeof(value3) }));

    getOpaque = AwaOpaqueArray_GetValue(array, 0);
    ASSERT_EQ(sizeof(value3), getOpaque.Size);
    ASSERT_EQ(0, memcmp(value3, getOpaque.Data, sizeof(value3)));

    AwaOpaqueArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaOpaqueArray_GetValueCount_valid_after_replace)
{
    AwaOpaqueArray * array = AwaOpaqueArray_New();
    EXPECT_TRUE(NULL != array);

    char value[] = { 0, 1, 2, 3, 4, 5, 0, 6, 7, 8, 9, 10, 0, 0, 15, 16 };
    char value2[] = { 16,15, 0, 0, 0, 3, 5, 6, 7 };

    AwaOpaqueArray_SetValue(array, 0, ((AwaOpaque) { value, sizeof(value) }));
    AwaOpaqueArray_SetValue(array, 1, ((AwaOpaque) { value, sizeof(value) }));
    AwaOpaqueArray_SetValue(array, 0, ((AwaOpaque) { value2, sizeof(value2) }));

    ASSERT_EQ(static_cast<size_t>(2), AwaOpaqueArray_GetValueCount(array));

    AwaOpaqueArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaOpaqueArrayIterator_New_and_Free_valid_inputs)
{
    AwaOpaqueArray * array = AwaOpaqueArray_New();
    EXPECT_TRUE(NULL != array);

    AwaOpaqueArrayIterator * iterator = AwaOpaqueArray_NewOpaqueArrayIterator(array);
    ASSERT_TRUE(NULL != iterator);

    AwaOpaqueArrayIterator_Free(&iterator);
    ASSERT_TRUE(NULL == iterator);

    AwaOpaqueArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaOpaqueArrayIterator_New_invalid_inputs)
{
    AwaOpaqueArrayIterator * iterator = AwaOpaqueArray_NewOpaqueArrayIterator(NULL);
    ASSERT_TRUE(NULL == iterator);
}

TEST_F(TestArray, AwaOpaqueArrayIterator_Set_Get_values)
{
    AwaOpaqueArray * array = AwaOpaqueArray_New();
    EXPECT_TRUE(NULL != array);

    char temp [64];
    int j = 0;

    for (int i = 0; i < 64; i ++)
    {
        temp[i] = i;
    }

    for(int i = 0; i < 1000; i+=5)
    {
        temp[j] = i % 256;
        AwaOpaqueArray_SetValue(array, i, ((AwaOpaque) { temp, static_cast<size_t>(64 - (i % 64)) }));
        j = (j + 1) % 64;
    }

    AwaOpaqueArrayIterator * iterator = AwaOpaqueArray_NewOpaqueArrayIterator(array);
    EXPECT_TRUE(NULL != iterator);

    for (int i = 0; i < 64; i ++)
    {
        temp[i] = i;
    }

    j = 0;
    for(int i = 0; i < 1000; i+=5)
    {
        ASSERT_TRUE(AwaOpaqueArrayIterator_Next(iterator));
        ASSERT_EQ(static_cast<size_t>(i), AwaOpaqueArrayIterator_GetIndex(iterator));
        temp[j] = i % 256;
        AwaOpaque getOpaque = AwaOpaqueArrayIterator_GetValue(iterator);
        ASSERT_EQ(static_cast<size_t>(64 - (i % 64)), getOpaque.Size);
        ASSERT_EQ(0, memcmp(temp, getOpaque.Data, 64 - (i % 64)));
        j = (j + 1) % 64;
    }

    ASSERT_FALSE(AwaOpaqueArrayIterator_Next(iterator));

    AwaOpaqueArrayIterator_Free(&iterator);
    EXPECT_TRUE(NULL == iterator);

    AwaOpaqueArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

////
// ObjectLink Arrays
////

TEST_F(TestArray, AwaObjectLinkArray_New_and_Free_valid_inputs)
{
    AwaObjectLinkArray * array = AwaObjectLinkArray_New();
    ASSERT_TRUE(NULL != array);
    AwaObjectLinkArray_Free(&array);
    ASSERT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaObjectLinkArray_SetValue_GetValue_valid_inputs)
{
    AwaObjectLinkArray * array = AwaObjectLinkArray_New();
    EXPECT_TRUE(NULL != array);

    AwaObjectLinkArray_SetValue(array, 0, (AwaObjectLink){100, 1});
    AwaObjectLink objectLink = AwaObjectLinkArray_GetValue(array, 0);
    ASSERT_EQ(100, objectLink.ObjectID);
    ASSERT_EQ(1, objectLink.ObjectInstanceID);

    AwaObjectLinkArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaObjectLinkArray_IsValid_valid_inputs)
{
    AwaObjectLinkArray * array = AwaObjectLinkArray_New();
    EXPECT_TRUE(NULL != array);

    AwaObjectLinkArray_SetValue(array, 0, (AwaObjectLink){100, 1});
    ASSERT_EQ(true, AwaObjectLinkArray_IsValid(array, 0));
    ASSERT_EQ(0, AwaObjectLinkArray_IsValid(array, 1));

    AwaObjectLinkArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaObjectLinkArray_SetValue_GetValue_invalid_inputs)
{
    AwaObjectLinkArray_SetValue(NULL, 0, (AwaObjectLink){100, 1});
    ASSERT_EQ(0, AwaObjectLinkArray_IsValid(NULL, 0));
    AwaObjectLink objectLink = AwaObjectLinkArray_GetValue(NULL, 0);
    ASSERT_EQ(0, objectLink.ObjectID);
    ASSERT_EQ(0, objectLink.ObjectInstanceID);
}

TEST_F(TestArray, AwaObjectLinkArray_DeleteValue_valid_inputs)
{
    AwaObjectLinkArray * array = AwaObjectLinkArray_New();
    EXPECT_TRUE(NULL != array);

    AwaObjectLinkArray_SetValue(array, 0, (AwaObjectLink){100, 1});

    AwaObjectLinkArray_DeleteValue(array, 0);

    ASSERT_EQ(0, AwaObjectLinkArray_IsValid(array, 0));

    AwaObjectLink objectLink = AwaObjectLinkArray_GetValue(array, 0);
    ASSERT_EQ(0, objectLink.ObjectID);
    ASSERT_EQ(0, objectLink.ObjectInstanceID);

    AwaObjectLinkArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaObjectLinkArray_DeleteValue_invalid_inputs)
{
    AwaObjectLinkArray * array = AwaObjectLinkArray_New();
    EXPECT_TRUE(NULL != array);

    AwaObjectLinkArray_SetValue(array, 0, (AwaObjectLink){100, 1});

    AwaObjectLinkArray_DeleteValue(NULL, 0);
    AwaObjectLinkArray_DeleteValue(array, 1);

    AwaObjectLink objectLink = AwaObjectLinkArray_GetValue(array, 0);
    ASSERT_EQ(100, objectLink.ObjectID);
    ASSERT_EQ(1, objectLink.ObjectInstanceID);

    AwaObjectLinkArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaObjectLinkArray_GetValueCount_valid_inputs)
{
    AwaObjectLinkArray * array = AwaObjectLinkArray_New();
    EXPECT_TRUE(NULL != array);

    AwaObjectLinkArray_SetValue(array, 0, (AwaObjectLink){100,1});

    AwaObjectLinkArray_SetValue(array, 1, (AwaObjectLink){100,1});

    AwaObjectLinkArray_SetValue(array, 2, (AwaObjectLink){100,1});

    ASSERT_EQ(static_cast<size_t>(3), AwaObjectLinkArray_GetValueCount(array));

    AwaObjectLinkArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaObjectLinkArray_GetValueCount_invalid_inputs)
{
    ASSERT_EQ(static_cast<size_t>(0), AwaObjectLinkArray_GetValueCount(NULL));
}

TEST_F(TestArray, AwaObjectLinkArray_DeleteValue_and_GetValueCount_valid)
{
    AwaObjectLinkArray * array = AwaObjectLinkArray_New();
    EXPECT_TRUE(NULL != array);

    AwaObjectLinkArray_SetValue(array, 0, (AwaObjectLink){100,1});

    AwaObjectLinkArray_SetValue(array, 1, (AwaObjectLink){100,1});

    AwaObjectLinkArray_SetValue(array, 2, (AwaObjectLink){100,1});

    AwaObjectLinkArray_DeleteValue(array, 0);

    ASSERT_EQ(static_cast<size_t>(2), AwaObjectLinkArray_GetValueCount(array));

    AwaObjectLinkArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaObjectLinkArray_GetValueCount_valid_after_replace)
{
    AwaObjectLinkArray * array = AwaObjectLinkArray_New();
    EXPECT_TRUE(NULL != array);

    AwaObjectLinkArray_SetValue(array, 0, (AwaObjectLink){100,1});
    AwaObjectLinkArray_SetValue(array, 1, (AwaObjectLink){100,1});
    AwaObjectLinkArray_SetValue(array, 1, (AwaObjectLink){600, 100});

    ASSERT_EQ(static_cast<size_t>(2), AwaObjectLinkArray_GetValueCount(array));

    AwaObjectLinkArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}


TEST_F(TestArray, AwaObjectLinkArrayIterator_New_and_Free_valid_inputs)
{
    AwaObjectLinkArray * array = AwaObjectLinkArray_New();
    EXPECT_TRUE(NULL != array);

    AwaObjectLinkArrayIterator * iterator = AwaObjectLinkArray_NewObjectLinkArrayIterator(array);
    ASSERT_TRUE(NULL != iterator);

    AwaObjectLinkArrayIterator_Free(&iterator);
    ASSERT_TRUE(NULL == iterator);

    AwaObjectLinkArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

TEST_F(TestArray, AwaObjectLinkArrayIterator_New_invalid_inputs)
{
    AwaObjectLinkArrayIterator * iterator = AwaObjectLinkArray_NewObjectLinkArrayIterator(NULL);
    ASSERT_TRUE(NULL == iterator);
}


TEST_F(TestArray, AwaObjectLinkArrayIterator_Set_Get_values)
{
    AwaObjectLinkArray * array = AwaObjectLinkArray_New();
    EXPECT_TRUE(NULL != array);

    for(int i = 0; i < 1000; i+=5)
    {
        AwaObjectLinkArray_SetValue(array, i, (AwaObjectLink){i, i / 2});
    }

    AwaObjectLinkArrayIterator * iterator = AwaObjectLinkArray_NewObjectLinkArrayIterator(array);
    EXPECT_TRUE(NULL != iterator);

    for(int i = 0; i < 1000; i+=5)
    {
        ASSERT_TRUE(AwaObjectLinkArrayIterator_Next(iterator));
        ASSERT_EQ(static_cast<size_t>(i), AwaObjectLinkArrayIterator_GetIndex(iterator));
        AwaObjectLink objectLink = AwaObjectLinkArrayIterator_GetValue(iterator);
        ASSERT_EQ(i, objectLink.ObjectID);
        ASSERT_EQ(i / 2, objectLink.ObjectInstanceID);
//        ASSERT_EQ(sizeof(AwaObjectLink), ArrayIterator_GetValueLength(iterator));
    }

    ASSERT_FALSE(AwaObjectLinkArrayIterator_Next(iterator));

    AwaObjectLinkArrayIterator_Free(&iterator);
    EXPECT_TRUE(NULL == iterator);

    AwaObjectLinkArray_Free(&array);
    EXPECT_TRUE(NULL == array);
}

} // namespace Awa

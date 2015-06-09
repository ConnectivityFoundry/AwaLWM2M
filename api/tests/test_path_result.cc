#include <gtest/gtest.h>

#include "path_result.h"
#include "support/support.h"

namespace Awa {

class TestPathResult : public TestClientBase {};

TEST_F(TestPathResult, PathResult_New_and_Free)
{
    const char * xml =
            "<Result>"
            " <Error>AwaError_ClientNotFound</Error>"
            "</Result>";
    TreeNode resultNode = TreeNode_ParseXML((uint8_t*)xml, strlen(xml), true);
    PathResult * result = PathResult_New(resultNode);
    EXPECT_TRUE(NULL != result);
    EXPECT_EQ(AwaError_ClientNotFound, PathResult_GetError(result));
    PathResult_Free(&result);
    EXPECT_EQ(NULL, result);
    Tree_Delete(resultNode);
}

TEST_F(TestPathResult, PathResult_New_handles_null)
{
    PathResult * result = PathResult_New(NULL);
    EXPECT_EQ(AwaError_Unspecified, PathResult_GetError(result));
    PathResult_Free(&result);
}

TEST_F(TestPathResult, PathResult_Free_handles_null_pointer)
{
    PathResult * result = NULL;
    PathResult_Free(&result);
    EXPECT_EQ(NULL, result);
}

TEST_F(TestPathResult, PathResult_Free_handles_null)
{
    PathResult_Free(NULL);
}

TEST_F(TestPathResult, PathResult_GetError_handles_null)
{
    EXPECT_EQ(AwaError_Unspecified, PathResult_GetError(NULL));
}

} // namespace Awa

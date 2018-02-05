
#include "sunriset_version.h"
#include "sunriset.h"
#include "gtest/gtest.h"

#include <stdio.h>


GTEST_TEST(a, b)
{
    EXPECT_TRUE(true);
}


int main(int argc, char *argv[])
{
    printf("Testing %s: %s\n", SUNRISET_NAME, SUNRISET_VERSION_STRING);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

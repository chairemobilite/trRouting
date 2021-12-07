#include "gtest/gtest.h" // we will add the path to C preprocessor later

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

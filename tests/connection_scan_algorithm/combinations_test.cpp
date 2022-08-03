#include "gtest/gtest.h"
#include "combinations.hpp"

TEST(CombinationsTest, simple) {
    std::vector<int> testSet = {1,2,3};
    
    Combinations<int> combo(testSet, 2);

    auto out = combo.get();

    EXPECT_EQ(3, out.size());
    EXPECT_EQ(std::vector<int>({1,2}), out[0]);
    EXPECT_EQ(std::vector<int>({1,3}), out[1]);
    EXPECT_EQ(std::vector<int>({2,3}), out[2]);
}

TEST(CombinationsTest, referenceWrapper) {
    std::vector<int> testSet = {1,2,3};
    std::vector<std::reference_wrapper<int>> testRefSet;
    testRefSet.push_back(std::ref(testSet[0]));
    testRefSet.push_back(std::ref(testSet[1]));
    testRefSet.push_back(std::ref(testSet[2]));
    //Change last number to make sure we had a reference
    testSet[2] = 4;
    
    Combinations<std::reference_wrapper<int>> combo(testRefSet, 2);

    auto out = combo.get();

    EXPECT_EQ(3, out.size());
    EXPECT_EQ(1, out[0][0].get());
    EXPECT_EQ(2, out[0][1].get());
    EXPECT_EQ(1, out[1][0].get());
    EXPECT_EQ(4, out[1][1].get());
    EXPECT_EQ(2, out[2][0].get());
    EXPECT_EQ(4, out[2][1].get());
}

TEST(CombinationsTest, long) {
    std::vector<int> testSet = {1,2,3,4,5,6};
    
    Combinations<int> combo(testSet, 3);

    auto out = combo.get();

    EXPECT_EQ(20, out.size());
}

#include <gtest/gtest.h>
#include "Math/Statistics.hpp"

TEST(MedianTest, Odd)
{
  std::vector<int> vec{9, 8, 5, 2, 1};
  EXPECT_EQ(Median(vec), 5);
}

TEST(MedianTest, Even)
{
  std::vector<int> vec{9, 8, 5, 4, 2, 1};
  EXPECT_EQ(Median(vec), 4.5);
}

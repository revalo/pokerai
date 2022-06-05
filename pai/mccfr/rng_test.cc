#include "rng.h"

#include <gtest/gtest.h>

#include <iostream>

using namespace std;

TEST(RNGTest, SeedConsistent) {
  pokerai::RandomNumberGenerator rng(0);
  EXPECT_EQ(rng.next(), 7757);
  EXPECT_EQ(rng.next(), 318781527);
}

TEST(RNGTest, RandIntRange) {
  pokerai::RandomNumberGenerator rng(0);

  int counts[5] = {0};

  for (int i = 0; i < 1000; i++) {
    int randomInteger = rng.randInt(5, 9);
    EXPECT_GE(randomInteger, 5);
    EXPECT_LE(randomInteger, 9);

    counts[randomInteger - 5]++;
  }

  // Expect that the distribution is uniform.
  for (int i = 0; i < 5; i++) {
    EXPECT_NEAR(counts[i], 200, 20);
  }
}

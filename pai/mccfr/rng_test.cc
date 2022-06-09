#include "rng.h"

#include <gtest/gtest.h>

#include <iostream>

using namespace std;

TEST(RNGTest, SeedConsistent) {
  pokerai::RandomNumberGenerator rng(0);
  EXPECT_EQ(rng.next(), 8257471688044718387);
  EXPECT_EQ(rng.next(), 17188344695734816891);
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
    EXPECT_NEAR(counts[i], 200, 50);
  }
}

TEST(RNGTest, RandFloatRange) {
  pokerai::RandomNumberGenerator rng(0);

  int counts[5] = {0};

  for (int i = 0; i < 1000; i++) {
    float randomFloat = rng.randFloat();
    EXPECT_GE(randomFloat, 0.0f);
    EXPECT_LE(randomFloat, 1.0f);

    counts[(int)(randomFloat * 5)]++;
  }

  // Expect that the distribution is uniform.
  for (int i = 0; i < 5; i++) {
    EXPECT_NEAR(counts[i], 200, 50);
  }
}

TEST(RNGTest, SampleFromProbabilities) {
  pokerai::RandomNumberGenerator rng(0);

  float probabilities[5] = {0.1f, 0.1f, 0.5f, 0.1f, 0.2f};
  int counts[5] = {0};

  for (int i = 0; i < 1000; i++) {
    int sampledIndex = rng.sampleFromProbabilities(probabilities, 5);
    EXPECT_GE(sampledIndex, 0);
    EXPECT_LE(sampledIndex, 4);

    counts[sampledIndex]++;
  }

  // Expect that the distribution is that of the probabilities.
  for (int i = 0; i < 5; i++) {
    EXPECT_NEAR(counts[i], 1000 * probabilities[i], 50);
  }
}

TEST(RNGTest, UniqueRNG) {
  pokerai::UniqueRandomNumberGenerator rng(51);
  bool seenAlready[52] = {false};

  for (int trial = 0; trial < 1000; trial++) {
    for (int i = 0; i < 5; i++) {
      int randomInteger = rng.next();
      EXPECT_GE(randomInteger, 0);
      EXPECT_LE(randomInteger, 51);
      EXPECT_FALSE(seenAlready[randomInteger]);
      seenAlready[randomInteger] = true;
    }

    rng.reset();

    // Reset the seenAlready array.
    for (int i = 0; i < 52; i++) {
      seenAlready[i] = false;
    }
  }
}

TEST(RNGTest, UniqueRNGExclude) {
  // Never generate the following numbers, 7 and 15.
  uint64_t excludeMask = (1ull << 7) | (1ull << 15);
  pokerai::UniqueRandomNumberGenerator rng(51, excludeMask);
  bool seenAlready[52] = {false};

  for (int trial = 0; trial < 1000; trial++) {
    for (int i = 0; i < 5; i++) {
      int randomInteger = rng.next();
      EXPECT_GE(randomInteger, 0);
      EXPECT_LE(randomInteger, 51);
      EXPECT_FALSE(seenAlready[randomInteger]);
      EXPECT_NE(randomInteger, 7);
      EXPECT_NE(randomInteger, 15);
      seenAlready[randomInteger] = true;
    }

    rng.reset();

    // Reset the seenAlready array.
    for (int i = 0; i < 52; i++) {
      seenAlready[i] = false;
    }
  }
}
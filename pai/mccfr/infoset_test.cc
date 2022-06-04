#include "infoset.h"

#include <gtest/gtest.h>

TEST(InfosetTest, GetStrategy) {
  pokerai::InfoSet infoSet(5);
  EXPECT_EQ(5, infoSet.numActions);
  EXPECT_FLOAT_EQ(infoSet.regretSums[3], 0.0f);

  infoSet.regretSums[3] += 3.0f;
  infoSet.regretSums[4] += 3.0f;
  infoSet.regretSums[0] = -4.0f;

  float *strategy = infoSet.getStrategy();

  EXPECT_FLOAT_EQ(strategy[0], 0.0f);
  EXPECT_FLOAT_EQ(strategy[1], 0.0f);
  EXPECT_FLOAT_EQ(strategy[3], 0.5f);
  EXPECT_FLOAT_EQ(strategy[4], 0.5f);
}

TEST(InfosetTest, GetStrategyZero) {
  pokerai::InfoSet infoSet(5);
  EXPECT_EQ(5, infoSet.numActions);
  EXPECT_FLOAT_EQ(infoSet.regretSums[3], 0.0f);
  float *strategy = infoSet.getStrategy();

  EXPECT_FLOAT_EQ(strategy[3], 0.2f);
}

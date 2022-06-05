#include "infotable.h"

#include <gtest/gtest.h>

#include "infoset.h"

TEST(InfoTableTest, GetInfoTable) {
  pokerai::InfoTable infoTable("test_dummy.table");
  pokerai::InfoSet* infoSetK1 = infoTable.get("k1", 5);
  pokerai::InfoSet* infoSetK2 = infoTable.get("k2", 3);

  EXPECT_EQ(5, infoSetK1->numActions);
  EXPECT_FLOAT_EQ(infoSetK1->regretSums[3], 0.0f);
  EXPECT_EQ(3, infoSetK2->numActions);
  EXPECT_FLOAT_EQ(infoSetK2->regretSums[1], 0.0f);

  infoSetK1->regretSums[3] += 3.0f;
  pokerai::InfoSet* infoSetK1_2 = infoTable.get("k1", 5);
  EXPECT_FLOAT_EQ(infoSetK1_2->regretSums[3], 3.0f);
}

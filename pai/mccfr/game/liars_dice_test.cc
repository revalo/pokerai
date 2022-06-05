#include "liars_dice.h"

#include <gtest/gtest.h>

#include <iostream>

using namespace std;

TEST(LiarsDiceTest, SampleChance) {
  pokerai::game::LiarsDice game(2, 3, 6);
  auto rootNode = game.getRootNode();
  EXPECT_TRUE(game.isChance(rootNode));

  auto sampledNode =
      (pokerai::game::LiarsDiceGameNode*)(game.sampleChance(rootNode));
  EXPECT_FALSE(game.isChance(sampledNode));
  EXPECT_TRUE(sampledNode->diceOwner);
  EXPECT_EQ(sampledNode->decidingPlayerIndex, 0);
}

TEST(LiarsDiceTest, GameTest) {
  pokerai::game::LiarsDice game(2, 3, 6);
  auto rootNode = game.getRootNode();
  auto sampledNode =
      (pokerai::game::LiarsDiceGameNode*)(game.sampleChance(rootNode));
  EXPECT_EQ(sampledNode->decidingPlayerIndex, 0);
  EXPECT_FALSE(game.isTerminal(sampledNode));
  EXPECT_EQ(game.getValidActions(sampledNode).size(), 36);

  // Bet move 8 = (2, 3).
  auto moveNode =
      (pokerai::game::LiarsDiceGameNode*)game.takeAction(sampledNode, 8);
  EXPECT_EQ(moveNode->decidingPlayerIndex, 1);
  EXPECT_EQ(moveNode->history.size(), 1);
  EXPECT_EQ(moveNode->history[0], 8);
  EXPECT_EQ(sampledNode->history.size(), 0);
  EXPECT_FALSE(game.isTerminal(moveNode));

  auto validActions = game.getValidActions(moveNode);
  EXPECT_EQ(validActions.size(), 23);

  // Bet move 20 = (4, 3).
  auto moveNode2 =
      (pokerai::game::LiarsDiceGameNode*)game.takeAction(moveNode, 20);
  EXPECT_EQ(moveNode2->decidingPlayerIndex, 0);
  EXPECT_EQ(moveNode2->history.size(), 2);
  EXPECT_EQ(moveNode2->history[0], 8);
  EXPECT_EQ(moveNode2->history[1], 20);
  EXPECT_EQ(moveNode->history.size(), 1);
  EXPECT_EQ(sampledNode->history.size(), 0);
  EXPECT_FALSE(game.isTerminal(moveNode2));

  // Call LIAR.
  auto moveNode3 = (pokerai::game::LiarsDiceGameNode*)game.takeAction(
      moveNode2, game.liarAction);
  EXPECT_EQ(moveNode3->decidingPlayerIndex, 1);
  EXPECT_EQ(moveNode3->history.size(), 3);
  EXPECT_EQ(moveNode3->history[0], 8);
  EXPECT_EQ(moveNode3->history[1], 20);
  EXPECT_EQ(moveNode3->history[2], game.liarAction);
  EXPECT_EQ(moveNode2->history.size(), 2);
  EXPECT_TRUE(game.isTerminal(moveNode3));
}

#include "liars_dice.h"

#include <gtest/gtest.h>

#include <iostream>

using namespace std;

TEST(LiarsDiceTest, SampleChance) {
  pokerai::game::LiarsDice game(2, 3, 6);

  pokerai::game::LiarsDiceGameNode rootNode =
      pokerai::game::LiarsDiceGameNode();
  game.getRootNode(&rootNode);
  EXPECT_TRUE(game.isChance(&rootNode));

  pokerai::game::LiarsDiceGameNode sampledNode =
      pokerai::game::LiarsDiceGameNode();
  game.sampleChance(&rootNode, &sampledNode);

  EXPECT_FALSE(game.isChance(&sampledNode));
  EXPECT_TRUE(sampledNode.diceOwner);
  EXPECT_EQ(sampledNode.decidingPlayerIndex, 0);
}

TEST(LiarsDiceTest, GameTest) {
  pokerai::game::LiarsDice game(2, 3, 6, 0);

  pokerai::game::LiarsDiceGameNode rootNode =
      pokerai::game::LiarsDiceGameNode();
  game.getRootNode(&rootNode);

  pokerai::game::LiarsDiceGameNode sampledNode =
      pokerai::game::LiarsDiceGameNode();
  game.sampleChance(&rootNode, &sampledNode);

  EXPECT_EQ(sampledNode.decidingPlayerIndex, 0);
  EXPECT_FALSE(game.isTerminal(&sampledNode));
  EXPECT_EQ(game.getValidActions(&sampledNode)->size(), 36);

  // Bet move 8 = (2, 3).
  pokerai::game::LiarsDiceGameNode moveNode =
      pokerai::game::LiarsDiceGameNode();
  game.takeAction(&sampledNode, 8, &moveNode);

  EXPECT_EQ(moveNode.decidingPlayerIndex, 1);
  EXPECT_EQ(moveNode.history.size(), 1);
  EXPECT_EQ(moveNode.history[0], 8);
  EXPECT_EQ(sampledNode.history.size(), 0);
  EXPECT_FALSE(game.isTerminal(&moveNode));

  auto validActions = game.getValidActions(&moveNode);
  EXPECT_EQ(validActions->size(), 23);

  // Bet move 20 = (4, 3).
  pokerai::game::LiarsDiceGameNode moveNode2 =
      pokerai::game::LiarsDiceGameNode();
  game.takeAction(&moveNode, 20, &moveNode2);

  EXPECT_EQ(moveNode2.decidingPlayerIndex, 0);
  EXPECT_EQ(moveNode2.history.size(), 2);
  EXPECT_EQ(moveNode2.history[0], 8);
  EXPECT_EQ(moveNode2.history[1], 20);
  EXPECT_EQ(moveNode.history.size(), 1);
  EXPECT_EQ(sampledNode.history.size(), 0);
  EXPECT_FALSE(game.isTerminal(&moveNode2));

  // Call LIAR.
  pokerai::game::LiarsDiceGameNode moveNode3 =
      pokerai::game::LiarsDiceGameNode();
  game.takeAction(&moveNode2, game.liarAction, &moveNode3);

  EXPECT_EQ(moveNode3.decidingPlayerIndex, 1);
  EXPECT_EQ(moveNode3.history.size(), 3);
  EXPECT_EQ(moveNode3.history[0], 8);
  EXPECT_EQ(moveNode3.history[1], 20);
  EXPECT_EQ(moveNode3.history[2], game.liarAction);
  EXPECT_EQ(moveNode2.history.size(), 2);
  EXPECT_TRUE(game.isTerminal(&moveNode3));

  // Player 6 does not participate in the game.
  EXPECT_EQ(game.getTerminalValue(&moveNode3, 6), 0);

  // Player 0 has 1 2 6 and player 1 has 2 3 3.
  // Player 0 called out Player 1's bet of (4, 3), which is false.
  EXPECT_EQ(game.getTerminalValue(&moveNode3, 0), 1);
  EXPECT_EQ(game.getTerminalValue(&moveNode3, 1), -1);

  EXPECT_EQ(game.getInfosetKey(&moveNode3), "2,3,3|8,20,36");
}

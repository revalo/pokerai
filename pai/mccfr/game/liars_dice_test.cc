#include "liars_dice.h"

#include <gtest/gtest.h>

#include <iostream>

using namespace std;

TEST(LiarsDiceTest, Initialize) {
  pokerai::game::LiarsDice game(2, 1, 6);
  cout << "POOP " << game.name() << endl;
}

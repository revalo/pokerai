#include "liars_dice.h"

#include <algorithm>

#include "../rng.h"

namespace pokerai {
namespace game {
LiarsDiceGameNode::LiarsDiceGameNode(LiarsDice* game, bool deal,
                                     int decidingPlayerIndex, int** dice) {
  this->game = game;
  this->deal = deal;
  this->decidingPlayerIndex = decidingPlayerIndex;

  // Shadow the dice array.
  this->dice = dice;
  this->diceOwner = false;
}

int LiarsDiceGameNode::getDecidingPlayerIndex() { return decidingPlayerIndex; }

bool LiarsDiceGameNode::isChance() { return deal; }

bool LiarsDiceGameNode::isTerminal() { return false; }

LiarsDice::LiarsDice(int numPlayers, int numDice, int maxDiceFace) {
  this->numPlayers = numPlayers;
  this->numDice = numDice;
  this->maxDiceFace = maxDiceFace;

  this->rng = RandomNumberGenerator();
}

std::string LiarsDice::name() const { return "LiarsDice"; }

GameNode* LiarsDice::getRootNode() {
  return new LiarsDiceGameNode(this, true, 0, NULL);
}

GameNode* LiarsDice::sampleChance(GameNode* node) {
  int** dice = new int*[numPlayers];
  for (int playerIndex = 0; playerIndex < numPlayers; playerIndex++) {
    dice[playerIndex] = new int[numDice];

    for (int diceIndex = 0; diceIndex < numDice; diceIndex++) {
      dice[playerIndex][diceIndex] = rng.randInt(1, maxDiceFace);
    }

    // Sort the dice.
    std::sort(dice[playerIndex], dice[playerIndex] + numDice);
  }

  auto rv = new LiarsDiceGameNode(this, false, 0, dice);
  rv->diceOwner = true;
  return rv;
}

int LiarsDice::getTerminalValue(GameNode* node) { return 0; }

}  // namespace game
}  // namespace pokerai

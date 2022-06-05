#include "liars_dice.h"

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
}

std::string LiarsDice::name() const { return "LiarsDice"; }

GameNode* LiarsDice::getRootNode() {
  return new LiarsDiceGameNode(this, true, 0, NULL);
}

GameNode* LiarsDice::sampleChance(GameNode* node) {
  return new LiarsDiceGameNode(this, true, 0, NULL);
}

int LiarsDice::getTerminalValue(GameNode* node) { return 0; }

}  // namespace game
}  // namespace pokerai

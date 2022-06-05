#include "liars_dice.h"

namespace pokerai {
namespace game {
LiarsDiceGameNode::LiarsDiceGameNode(LiarsDice* game, bool deal,
                                     int decidingPlayerIndex) {
  this->game = game;
  this->deal = deal;
  this->decidingPlayerIndex = decidingPlayerIndex;
}

LiarsDice::LiarsDice(int numPlayers, int numDice, int maxDiceFace) {
  this->numPlayers = numPlayers;
  this->numDice = numDice;
  this->maxDiceFace = maxDiceFace;
}

std::string LiarsDice::name() const { return "LiarsDice"; }
}  // namespace game
}  // namespace pokerai

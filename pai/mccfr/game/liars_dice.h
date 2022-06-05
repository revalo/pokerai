#ifndef GAME_LIARS_DICE_H
#define GAME_LIARS_DICE_H

#include <string>

#include "game.h"

namespace pokerai {
namespace game {
class LiarsDice;

class LiarsDiceGameNode : public GameNode {
  LiarsDice* game;
  bool deal;
  int decidingPlayerIndex;

 public:
  LiarsDiceGameNode(LiarsDice* game, bool deal = false,
                    int decidingPlayerIndex = 0);
  bool isChance();
  bool isTerminal();
  int getDecidingPlayerIndex();
};

class LiarsDice : public Game {
  int numPlayers;
  int numDice;
  int maxDiceFace;

 public:
  LiarsDice(int numPlayers = 2, int numDice = 1, int maxDiceFace = 6);
  std::string name() const;
};
}  // namespace game
}  // namespace pokerai

#endif  // GAME_LIARS_DICE_H

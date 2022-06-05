#ifndef GAME_LIARS_DICE_H
#define GAME_LIARS_DICE_H

#include <string>

#include "game.h"

namespace pokerai {
namespace game {
class LiarsDiceGameNode : public GameNode {
  bool deal;
  int decidingPlayerIndex;

 public:
  LiarsDiceGameNode(bool deal = false, int decidingPlayerIndex = 0);
  bool isChance();
  bool isTerminal();
  int getDecidingPlayerIndex();
};

class LiarsDice : public Game {
 public:
  LiarsDice();
  std::string name() const = 0;
};
}  // namespace game
}  // namespace pokerai

#endif  // GAME_LIARS_DICE_H

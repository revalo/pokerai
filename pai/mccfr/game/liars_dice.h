#ifndef GAME_LIARS_DICE_H
#define GAME_LIARS_DICE_H

#include <string>

#include "game.h"

namespace pokerai {
namespace game {
class LiarsDice;

class LiarsDiceGameNode : public GameNode {
 public:
  LiarsDice* game;
  bool deal;
  int decidingPlayerIndex;
  int** dice;
  bool diceOwner;

  LiarsDiceGameNode(LiarsDice* game, bool deal = false,
                    int decidingPlayerIndex = 0, int** dice = NULL);
  bool isChance();
  bool isTerminal();
  int getDecidingPlayerIndex();
  int getTerminalValue();
};

class LiarsDice : public Game {
  int numPlayers;
  int numDice;
  int maxDiceFace;

 public:
  LiarsDice(int numPlayers = 2, int numDice = 1, int maxDiceFace = 6);
  std::string name() const;
  GameNode* getRootNode();
  GameNode* sampleChance(GameNode* node);
  int getTerminalValue(GameNode* node);
};
}  // namespace game
}  // namespace pokerai

#endif  // GAME_LIARS_DICE_H

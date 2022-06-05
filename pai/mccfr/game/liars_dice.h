#ifndef GAME_LIARS_DICE_H
#define GAME_LIARS_DICE_H

#include <string>
#include <vector>

#include "../rng.h"
#include "game.h"

namespace pokerai {
namespace game {
class LiarsDice;

class LiarsDiceGameNode : public GameNode {
public:
  LiarsDice *game;
  bool deal;
  int decidingPlayerIndex;
  int **dice;
  bool diceOwner;
  std::vector<int> history;

  LiarsDiceGameNode(LiarsDice *game, bool deal = false,
                    int decidingPlayerIndex = 0, int **dice = NULL);
};

class LiarsDice : public Game {
  int numPlayers;
  int numDice;
  int maxDiceFace;

public:
  RandomNumberGenerator rng;
  int *actionQuantity;
  int *actionFace;
  std::vector<int> *validActionsLUT;
  std::vector<int> initActions;
  int liarAction;
  int maxNumActions;

  LiarsDice(int numPlayers = 2, int numDice = 1, int maxDiceFace = 6,
            int seed = -1);
  ~LiarsDice();
  std::string name() const;
  GameNode *getRootNode();
  GameNode *sampleChance(GameNode *node);
  GameNode *takeAction(GameNode *node, int action);

  bool isChance(GameNode *node);
  bool isTerminal(GameNode *node);
  int getDecidingPlayerIndex(GameNode *node);
  std::string getInfosetKey(GameNode *node);

  int getTerminalValue(GameNode *node, int player);
  std::vector<int> *getValidActions(GameNode *node);
};
} // namespace game
} // namespace pokerai

#endif // GAME_LIARS_DICE_H

#ifndef GAME_LIARS_DICE_H
#define GAME_LIARS_DICE_H

#include <string>
#include <vector>

#include "../rng.h"
#include "game.h"

namespace pokerai {
namespace game {
class LiarsDiceGameNode : public GameNode {
 public:
  bool deal;
  int decidingPlayerIndex;
  int **dice;
  bool diceOwner;
  std::vector<int> history;

  LiarsDiceGameNode(bool deal = false, int decidingPlayerIndex = 0,
                    int **dice = NULL);
};

class LiarsDice : public Game<LiarsDiceGameNode> {
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
  void getRootNode(LiarsDiceGameNode *resNode);
  void sampleChance(LiarsDiceGameNode *node, LiarsDiceGameNode *resNode);
  void takeAction(LiarsDiceGameNode *node, int action,
                  LiarsDiceGameNode *resNode);

  bool isChance(LiarsDiceGameNode *node);
  bool isTerminal(LiarsDiceGameNode *node);
  int getDecidingPlayerIndex(LiarsDiceGameNode *node);
  std::string getInfosetKey(LiarsDiceGameNode *node);

  int getTerminalValue(LiarsDiceGameNode *node, int player);
  std::vector<int> *getValidActions(LiarsDiceGameNode *node);
};
}  // namespace game
}  // namespace pokerai

#endif  // GAME_LIARS_DICE_H

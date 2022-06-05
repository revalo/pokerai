#include "liars_dice.h"

#include <algorithm>
#include <vector>

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

LiarsDice::LiarsDice(int numPlayers, int numDice, int maxDiceFace) {
  this->numPlayers = numPlayers;
  this->numDice = numDice;
  this->maxDiceFace = maxDiceFace;

  this->rng = RandomNumberGenerator();

  // Generate action map, +1 for LIAR.
  this->maxNumActions = numPlayers * numDice * maxDiceFace + 1;
  this->actionQuantity = new int[maxNumActions];
  this->actionFace = new int[maxNumActions];

  int actionIndex = 0;
  for (int diceNum = 1; diceNum <= numDice * numPlayers; diceNum++) {
    for (int diceFace = 1; diceFace <= maxDiceFace; diceFace++) {
      this->actionQuantity[actionIndex] = diceNum;
      this->actionFace[actionIndex] = diceFace;
      this->initActions.push_back(actionIndex);
      actionIndex++;
    }
  }
  // Init actions does not include LIAR.
  this->liarAction = actionIndex;

  // For each action, generate a next valid action, and save in LUT.
  this->validActionsLUT = new std::vector<int>[maxNumActions - 1];

  for (int lastAction = 0; lastAction < maxNumActions - 1; lastAction++) {
    int lastActionQuantity = this->actionQuantity[lastAction];
    int lastActionFace = this->actionFace[lastAction];

    std::vector<int> nextActions;
    for (int potentialNextAction = 0; potentialNextAction < maxNumActions;
         potentialNextAction++) {
      int potentialNextActionQuantity =
          this->actionQuantity[potentialNextAction];
      int potentialNextActionFace = this->actionFace[potentialNextAction];

      if (potentialNextActionFace > lastActionFace) {
        nextActions.push_back(potentialNextAction);
        continue;
      }

      if (potentialNextActionFace == lastActionFace &&
          potentialNextActionQuantity > lastActionQuantity) {
        nextActions.push_back(potentialNextAction);
        continue;
      }
    }
    nextActions.push_back(this->liarAction);

    this->validActionsLUT[lastAction] = nextActions;
  }
}

LiarsDice::~LiarsDice() {
  delete[] this->actionQuantity;
  delete[] this->actionFace;
  delete[] this->validActionsLUT;
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

int LiarsDice::getTerminalValue(GameNode* node, int player) { return 0; }

bool LiarsDice::isChance(GameNode* node) {
  auto rv = reinterpret_cast<LiarsDiceGameNode*>(node);
  return rv->deal;
}
bool LiarsDice::isTerminal(GameNode* node) {
  auto rv = reinterpret_cast<LiarsDiceGameNode*>(node);

  if (rv->history.size() == 0) {
    return false;
  }

  if (rv->history.size() >= 20) {
    return true;
  }

  if (rv->history.back() == this->liarAction) {
    return true;
  }

  return false;
}
int LiarsDice::getDecidingPlayerIndex(GameNode* node) {
  auto rv = reinterpret_cast<LiarsDiceGameNode*>(node);
  return rv->decidingPlayerIndex;
}
std::string LiarsDice::getInfosetKey(GameNode* node) {
  auto rv = reinterpret_cast<LiarsDiceGameNode*>(node);
  return "";
}

std::vector<int>* LiarsDice::getValidActions(GameNode* node) {
  auto rv = reinterpret_cast<LiarsDiceGameNode*>(node);
  if (rv->history.size() == 0) {
    return &(this->initActions);
  }

  int lastAction = rv->history.back();
  return &(this->validActionsLUT[lastAction]);
}

GameNode* LiarsDice::takeAction(GameNode* node, int action) {
  auto rv = reinterpret_cast<LiarsDiceGameNode*>(node);

  LiarsDiceGameNode* newNode = new LiarsDiceGameNode(
      this, false, (rv->decidingPlayerIndex + 1) % numPlayers, rv->dice);

  newNode->history = rv->history;
  newNode->history.push_back(action);

  return newNode;
}

}  // namespace game
}  // namespace pokerai

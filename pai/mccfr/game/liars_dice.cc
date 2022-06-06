#include "liars_dice.h"

#include <algorithm>
#include <vector>

#include "../rng.h"
#include "absl/strings/str_join.h"

#define COERCE_PLAYER_INDEX(playerIndex, numPlayers) \
  (numPlayers + (playerIndex % numPlayers)) % numPlayers

namespace pokerai {
namespace game {
LiarsDiceGameNode::LiarsDiceGameNode(bool deal, int decidingPlayerIndex,
                                     int **dice) {
  this->deal = deal;
  this->decidingPlayerIndex = decidingPlayerIndex;

  // Shadow the dice array.
  this->dice = dice;
  this->diceOwner = false;
}

LiarsDiceGameNode::~LiarsDiceGameNode() {
  if (this->diceOwner) {
    delete[] this->dice;
  }
}

LiarsDice::LiarsDice(int numPlayers, int numDice, int maxDiceFace, int seed,
                     int abstractionMoveMemory) {
  this->numPlayers = numPlayers;
  this->numDice = numDice;
  this->maxDiceFace = maxDiceFace;
  this->abstractionMoveMemory = abstractionMoveMemory;
  this->rng = RandomNumberGenerator(seed);

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

std::string LiarsDice::name() const { return "liars_dice"; }

void LiarsDice::getRootNode(LiarsDiceGameNode *resNode) {
  resNode->deal = true;
  resNode->decidingPlayerIndex = 0;
  resNode->dice = NULL;
}

void LiarsDice::sampleChance(LiarsDiceGameNode *node,
                             LiarsDiceGameNode *resNode) {
  int **dice = new int *[numPlayers];
  for (int playerIndex = 0; playerIndex < numPlayers; playerIndex++) {
    dice[playerIndex] = new int[numDice];

    for (int diceIndex = 0; diceIndex < numDice; diceIndex++) {
      dice[playerIndex][diceIndex] = rng.randInt(1, maxDiceFace);
    }

    // Sort the dice.
    std::sort(dice[playerIndex], dice[playerIndex] + numDice);
  }

  resNode->deal = false;
  resNode->decidingPlayerIndex = 0;
  resNode->dice = dice;
  resNode->diceOwner = true;
}

int LiarsDice::getTerminalValue(LiarsDiceGameNode *node, int player) {
  if (node->history.size() == 0) {
    return 0;
  }

  if (node->history.at(node->history.size() - 1) != this->liarAction) {
    return 0;
  }

  auto betInQuestion = node->history[node->history.size() - 2];
  auto playerQuestioning =
      COERCE_PLAYER_INDEX(node->decidingPlayerIndex - 1, numPlayers);
  auto playerInQuestion =
      COERCE_PLAYER_INDEX(node->decidingPlayerIndex - 2, numPlayers);
  auto betNum = this->actionQuantity[betInQuestion];
  auto betFace = this->actionFace[betInQuestion];

  if (player != playerInQuestion && player != playerQuestioning) {
    // Player not involved in the bet.
    return 0;
  }

  int betFaceCount = 0;
  for (int playerIndex = 0; playerIndex < numPlayers; playerIndex++) {
    for (int diceIndex = 0; diceIndex < numDice; diceIndex++) {
      if (node->dice[playerIndex][diceIndex] == betFace) {
        betFaceCount++;
      }
    }
  }

  if (betFaceCount >= betNum) {
    // Player in question wins.
    if (player == playerInQuestion) {
      return 1;
    } else {
      return -1;
    }
  } else {
    // Player in question loses.
    if (player == playerInQuestion) {
      return -1;
    } else {
      return 1;
    }
  }

  return 0;
}

bool LiarsDice::isChance(LiarsDiceGameNode *node) { return node->deal; }

bool LiarsDice::isTerminal(LiarsDiceGameNode *node) {
  if (node->history.size() == 0) {
    return false;
  }

  if (node->history.size() >= 20) {
    return true;
  }

  if (node->history.back() == this->liarAction) {
    return true;
  }

  return false;
}
int LiarsDice::getDecidingPlayerIndex(LiarsDiceGameNode *node) {
  return node->decidingPlayerIndex;
}
std::string LiarsDice::getInfosetKey(LiarsDiceGameNode *node) {
  if (this->isChance(node)) {
    return "chance";
  }

  auto decidingPlayerDice = node->dice[node->decidingPlayerIndex];
  std::string diceKey =
      absl::StrJoin(decidingPlayerDice, decidingPlayerDice + numDice, ",");

  std::string historyKey = "";
  if (node->history.size() < abstractionMoveMemory) {
    historyKey = absl::StrJoin(node->history, ",");
  } else {
    // Only keep the last 3 actions.
    historyKey = absl::StrJoin(node->history.end() - abstractionMoveMemory,
                               node->history.end(), ",");
  }

  return diceKey + "|" + historyKey;
}

std::vector<int> *LiarsDice::getValidActions(LiarsDiceGameNode *node) {
  if (node->history.size() == 0) {
    return &(this->initActions);
  }

  int lastAction = node->history.back();
  return &(this->validActionsLUT[lastAction]);
}

void LiarsDice::takeAction(LiarsDiceGameNode *node, int action,
                           LiarsDiceGameNode *resNode) {
  resNode->deal = false;
  resNode->decidingPlayerIndex = (node->decidingPlayerIndex + 1) % numPlayers;
  resNode->history = node->history;
  resNode->history.push_back(action);
  resNode->dice = node->dice;
}

}  // namespace game
}  // namespace pokerai

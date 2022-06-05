#include "external_sampling_mccfr.h"

#include <vector>

#include "game/game.h"
#include "infotable.h"
#include "rng.h"

namespace pokerai {
ExternalSamplingMCCFR::ExternalSamplingMCCFR(game::Game *game,
                                             InfoTable *infotable) {
  this->game = game;
  this->infotable = infotable;
  this->rng = RandomNumberGenerator();
}

int ExternalSamplingMCCFR::singleIteration(game::GameNode *node,
                                           int traversingPlayer) {

  if (game->isChance(node)) {
    auto sampledNode = game->sampleChance(node);
    auto rv = singleIteration(sampledNode, traversingPlayer);
    delete sampledNode;
    return rv;
  }

  if (game->isTerminal(node)) {
    return game->getTerminalValue(node, traversingPlayer);
  }

  std::vector<int> *validActions = game->getValidActions(node);
  int numActions = validActions->size();

  auto infoset = infotable->get(game->getInfosetKey(node), numActions);
  auto strategy = infoset->getStrategy();

  if (game->getDecidingPlayerIndex(node) == traversingPlayer) {
    // We are traversing the node that is the deciding player.
    // Try each action and update regrets.
    float *utils = new float[numActions];
    float infosetUtil = 0;

    for (int actionIndex = 0; actionIndex < numActions; actionIndex++) {
      int action = validActions->at(actionIndex);
      auto nextNode = game->takeAction(node, action);
      utils[actionIndex] = singleIteration(nextNode, traversingPlayer);
      infosetUtil += strategy[actionIndex] * utils[actionIndex];
    }

    for (int i = 0; i < numActions; i++) {
      infoset->regretSums[i] += utils[i] - infosetUtil;
    }

    delete[] utils;
    return infosetUtil;
  }

  // Other player, just sample.
  int actionIndex = rng.sampleFromProbabilities(strategy, numActions);
  int action = validActions->at(actionIndex);
  auto nextNode = game->takeAction(node, action);
  auto util = singleIteration(nextNode, traversingPlayer);

  for (int i = 0; i < numActions; i++) {
    infoset->strategySums[i] += strategy[i];
  }

  delete nextNode;
  return util;
}
} // namespace pokerai
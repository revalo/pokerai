#include "external_sampling_mccfr.h"

#include <future>
#include <thread>
#include <vector>

#include "game/game.h"
#include "infotable.h"
#include "rng.h"

namespace pokerai {
ExternalSamplingMCCFR::ExternalSamplingMCCFR(game::Game *game,
                                             InfoTable *infotable,
                                             bool parallel) {
  this->game = game;
  this->infotable = infotable;
  this->rng = RandomNumberGenerator();
  this->parallel = parallel;
}

float ExternalSamplingMCCFR::singleIterationInternal(game::GameNode *node,
                                                     int traversingPlayer,
                                                     bool launched) {
  if (game->isChance(node)) {
    auto sampledNode = game->sampleChance(node);
    auto rv = singleIterationInternal(sampledNode, traversingPlayer, launched);
    delete sampledNode;
    return rv;
  }

  if (game->isTerminal(node)) {
    return game->getTerminalValue(node, traversingPlayer);
  }

  std::vector<int> *validActions = game->getValidActions(node);
  int numActions = validActions->size();

  infotableLock.lock();
  auto infoset = infotable->get(game->getInfosetKey(node), numActions);
  infotableLock.unlock();

  auto strategy = infoset->getStrategy();

  if (game->getDecidingPlayerIndex(node) == traversingPlayer) {
    // We are traversing the node that is the deciding player.
    // Try each action and update regrets.
    float *utils = new float[numActions];
    float infosetUtil = 0;

    if (!parallel || launched) {
      for (int actionIndex = 0; actionIndex < numActions; actionIndex++) {
        int action = validActions->at(actionIndex);
        auto nextNode = game->takeAction(node, action);
        utils[actionIndex] =
            singleIterationInternal(nextNode, traversingPlayer, launched);
        infosetUtil += strategy[actionIndex] * utils[actionIndex];
      }
    } else {
      // Launch these in parallel instead.
      // We also need the return value of each thread.
      std::vector<std::future<float>> futures;
      for (int actionIndex = 0; actionIndex < numActions; actionIndex++) {
        int action = validActions->at(actionIndex);
        auto nextNode = game->takeAction(node, action);
        futures.push_back(std::async(
            std::launch::async, &ExternalSamplingMCCFR::singleIterationInternal,
            this, nextNode, traversingPlayer, true));
      }

      for (int actionIndex = 0; actionIndex < numActions; actionIndex++) {
        utils[actionIndex] = futures[actionIndex].get();
        infosetUtil += strategy[actionIndex] * utils[actionIndex];
      }
    }

    infotableLock.lock();
    for (int i = 0; i < numActions; i++) {
      infoset->regretSums[i] += utils[i] - infosetUtil;
    }
    infotableLock.unlock();

    delete[] utils;
    return infosetUtil;
  }

  // Other player, just sample.
  int actionIndex = rng.sampleFromProbabilities(strategy, numActions);
  int action = validActions->at(actionIndex);
  auto nextNode = game->takeAction(node, action);
  auto util = singleIterationInternal(nextNode, traversingPlayer, launched);

  infotableLock.lock();
  for (int i = 0; i < numActions; i++) {
    infoset->strategySums[i] += strategy[i];
  }
  infotableLock.unlock();

  delete nextNode;
  return util;
}

float ExternalSamplingMCCFR::singleIteration(game::GameNode *node,
                                             int traversingPlayer) {
  return singleIterationInternal(node, traversingPlayer, false);
}
}  // namespace pokerai
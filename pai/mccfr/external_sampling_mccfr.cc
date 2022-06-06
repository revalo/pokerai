#include "external_sampling_mccfr.h"

#include <future>
#include <thread>
#include <vector>

#include "game/game.h"
#include "game/liars_dice.h"
#include "infotable.h"
#include "rng.h"

namespace pokerai {
template <typename T>
ExternalSamplingMCCFR<T>::ExternalSamplingMCCFR(game::Game<T> *game,
                                                InfoTable *infotable,
                                                bool parallel,
                                                bool pruneRegrets,
                                                float regretPruneThreshold) {
  this->game = game;
  this->infotable = infotable;
  this->rng = RandomNumberGenerator();
  this->parallel = parallel;
  this->pruneRegrets = pruneRegrets;
  this->regretPruneThreshold = regretPruneThreshold;
}

template <typename T>
float ExternalSamplingMCCFR<T>::singleIterationInternal(T *node,
                                                        int traversingPlayer,
                                                        bool launched) {
  if (game->isChance(node)) {
    T nextNode = T();
    game->sampleChance(node, &nextNode);
    auto rv = singleIterationInternal(&nextNode, traversingPlayer, launched);
    return rv;
  }

  if (game->isTerminal(node)) {
    return game->getTerminalValue(node, traversingPlayer);
  }

  std::vector<int> *validActions = game->getValidActions(node);
  int numActions = (int)(validActions->size());

  auto infoset = infotable->get(game->getInfosetKey(node), numActions);

  auto strategy = infoset->getStrategy();

  if (game->getDecidingPlayerIndex(node) == traversingPlayer) {
    // We are traversing the node that is the deciding player.
    // Try each action and update regrets.
    float *utils = new float[numActions];
    float infosetUtil = 0;

    if (!parallel || launched) {
      for (int actionIndex = 0; actionIndex < numActions; actionIndex++) {
        if (pruneRegrets &&
            infoset->regretSums[actionIndex] < regretPruneThreshold &&
            rng.randFloat() < 0.95) {
          continue;
        }

        int action = validActions->at(actionIndex);
        T nextNode = T();
        game->takeAction(node, action, &nextNode);
        utils[actionIndex] =
            singleIterationInternal(&nextNode, traversingPlayer, launched);
        infosetUtil += strategy[actionIndex] * utils[actionIndex];
      }
    } else {
      // Launch these in parallel instead.
      // We also need the return value of each thread.
      // std::vector<std::future<float>> futures;
      // for (int actionIndex = 0; actionIndex < numActions; actionIndex++) {
      //   int action = validActions->at(actionIndex);
      //   auto nextNode = game->takeAction(node, action);
      //   futures.push_back(std::async(
      //       std::launch::async,
      //       &ExternalSamplingMCCFR::singleIterationInternal, this, nextNode,
      //       traversingPlayer, true));
      // }

      // for (int actionIndex = 0; actionIndex < numActions; actionIndex++) {
      //   utils[actionIndex] = futures[actionIndex].get();
      //   infosetUtil += strategy[actionIndex] * utils[actionIndex];
      // }
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
  T nextNode = T();
  game->takeAction(node, action, &nextNode);
  auto util = singleIterationInternal(&nextNode, traversingPlayer, launched);

  for (int i = 0; i < numActions; i++) {
    infoset->strategySums[i] += strategy[i];
  }
  return util;
}

template <typename T>
float ExternalSamplingMCCFR<T>::singleIteration(T *node, int traversingPlayer) {
  return singleIterationInternal(node, traversingPlayer, false);
}

// Definition for LiarsDiceGameNode.
template class ExternalSamplingMCCFR<game::LiarsDiceGameNode>;

}  // namespace pokerai
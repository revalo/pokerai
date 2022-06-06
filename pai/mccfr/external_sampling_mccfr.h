#ifndef EXT_SAMPLING_MCCFR_H
#define EXT_SAMPLING_MCCFR_H

#include <mutex>

#include "game/game.h"
#include "infotable.h"
#include "rng.h"

namespace pokerai {
class ExternalSamplingMCCFR {
  std::mutex infotableLock;

  float singleIterationInternal(game::GameNode *node, int traversingPlayer,
                                bool launched);

  bool parallel;

 public:
  game::Game *game;
  InfoTable *infotable;
  RandomNumberGenerator rng;

  ExternalSamplingMCCFR(game::Game *game, InfoTable *infotable,
                        bool parallel = false);

  float singleIteration(game::GameNode *node, int traversingPlayer);
};
}  // namespace pokerai

#endif  // EXT_SAMPLING_MCCFR_H

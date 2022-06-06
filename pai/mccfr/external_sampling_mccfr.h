#ifndef EXT_SAMPLING_MCCFR_H
#define EXT_SAMPLING_MCCFR_H

#include <mutex>

#include "game/game.h"
#include "infotable.h"
#include "rng.h"

namespace pokerai {
template <typename T>
class ExternalSamplingMCCFR {
  float singleIterationInternal(T *node, int traversingPlayer, bool launched);

  bool parallel;

 public:
  game::Game<T> *game;
  InfoTable *infotable;
  RandomNumberGenerator rng;

  bool pruneRegrets;
  float regretPruneThreshold;

  ExternalSamplingMCCFR(game::Game<T> *game, InfoTable *infotable,
                        bool parallel = false, bool pruneRegrets = true,
                        float regretPruneThreshold = -50.0f);

  float singleIteration(T *node, int traversingPlayer);
};
}  // namespace pokerai

#endif  // EXT_SAMPLING_MCCFR_H

#ifndef EXT_SAMPLING_MCCFR_H
#define EXT_SAMPLING_MCCFR_H

#include "game/game.h"
#include "infotable.h"
#include "rng.h"

namespace pokerai {
class ExternalSamplingMCCFR {
public:
  game::Game *game;
  InfoTable *infotable;
  RandomNumberGenerator rng;

  ExternalSamplingMCCFR(game::Game *game, InfoTable *infotable);

  int singleIteration(game::GameNode *node, int traversingPlayer);
};
} // namespace pokerai

#endif // EXT_SAMPLING_MCCFR_H

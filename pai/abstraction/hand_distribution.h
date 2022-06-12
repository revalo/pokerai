#ifndef HAND_DIST_H
#define HAND_DIST_H

#include <omp/EquityCalculator.h>
#include <omp/HandEvaluator.h>

#include "../mccfr/rng.h"

using namespace omp;

namespace pokerai {
class HandDistribution {
  HandEvaluator evaluator;

 public:
  HandDistribution();
  ~HandDistribution();

  Hand getBoardFromBitmask(uint64_t cards);

  float handStrength(const Hand &playerHand, const Hand &boardHand,
                     uint64_t playerCardsMask, uint64_t boardCardsMask);

  void getHandDistribution(uint64_t playerCardsMask,
                           std::vector<float> &distribution,
                           uint64_t existingBoardCardMask = 0,
                           int trials = 10000);
  void makeBuckets(const std::vector<float> &distribution, uint16_t *buckets,
                   int numBuckets);
};

}  // namespace pokerai

#endif  // HAND_DIST_H

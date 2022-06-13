// http://poker.cs.ualberta.ca/publications/AAMAS13-abstraction.pdf

#ifndef OCHS_H
#define OCHS_H

#include <omp/EquityCalculator.h>
#include <omp/HandEvaluator.h>

#include <vector>

#include "../mccfr/rng.h"
#include "hand_indexer.h"

using namespace omp;

namespace pokerai {
class OCHS {
 public:
  HandEvaluator evaluator;
  HandIndexer *indexer;
  std::vector<int> preflopClusterAssignments;
  int numPreflopClusters = 0;
  std::vector<std::vector<Hand>> preflopClusters;
  std::vector<std::vector<uint64_t>> preflopClustersMasks;

  OCHS(const std::string &preflopClusterFilename);
  ~OCHS();

  void computeOCHS(const Hand &playerHand, const Hand &boardHand,
                   uint64_t playerCardsMask, uint64_t boardCardsMask,
                   std::vector<uint16_t> &distribution);
};

}  // namespace pokerai

#endif  // OCHS_H

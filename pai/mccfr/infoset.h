#ifndef INFOSET_H
#define INFOSET_H

#include <string>

#include "leveldb/db.h"

namespace pokerai {
class InfoSet {
 public:
  int numActions;
  float uniformStrategyProb;
  float *regretSums;
  float *strategySums;
  float *strategy;
  float *averageStrategy;

  InfoSet(int numActions);
  ~InfoSet();

  void reset();
  float *getStrategy();
  float *getAverageStrategy();

  std::string serialize();
  void deserialize(const std::string &slice);
};
}  // namespace pokerai

#endif  // INFOSET_H

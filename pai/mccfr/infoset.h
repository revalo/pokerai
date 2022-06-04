#ifndef INFOSET_H
#define INFOSET_H

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
};
}  // namespace pokerai

#endif  // INFOSET_H

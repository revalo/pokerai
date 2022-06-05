#ifndef RNG_H
#define RNG_H

#include <cstdint>

namespace pokerai {
class RandomNumberGenerator {
  uint64_t s0;
  uint64_t s1;

 public:
  RandomNumberGenerator(int seed = 0);
  uint64_t next();

  // Returns a random integer between min and max, inclusive.
  int randInt(int min, int max);
  //   int sampleFromProbabilities(const float* probabilities, int numActions);
};
}  // namespace pokerai

#endif  // RNG_H

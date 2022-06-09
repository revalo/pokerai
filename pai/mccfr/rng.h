#ifndef RNG_H
#define RNG_H

#include <cstdint>

namespace pokerai {
class RandomNumberGenerator {
  uint64_t s0;
  uint64_t s1;

 public:
  // Initialize the RNG with a seed.
  // If seed is negative, a random seed will be generated.
  RandomNumberGenerator(int seed = -1);
  uint64_t next();

  // Returns a random integer between min and max, inclusive.
  int randInt(int min, int max);
  float randFloat();
  int sampleFromProbabilities(const float* probabilities, int numActions);
};

class UniqueRandomNumberGenerator {
  RandomNumberGenerator rng;
  int currentIndex;
  int* slots;
  int max;

 public:
  // Generated numbers will be between 0 and max. Inclusive.
  UniqueRandomNumberGenerator(int max, uint64_t excludeMask = 0);
  ~UniqueRandomNumberGenerator();
  void reset();
  int next();
};
}  // namespace pokerai

#endif  // RNG_H

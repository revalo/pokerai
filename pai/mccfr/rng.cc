#include "rng.h"

#include <cstdint>
#include <iostream>
#include <random>

namespace pokerai {
RandomNumberGenerator::RandomNumberGenerator(int seed) {
  if (seed < 0) {
    std::random_device rd;
    seed = rd();
  }

  srand(seed);
  s0 = rand();
  s1 = rand();

  // Cycle the seed.
  for (int i = 0; i < 10000; i++) {
    next();
  }
}

uint64_t RandomNumberGenerator::next() {
  uint64_t cs1 = s0;
  uint64_t cs0 = s1;
  uint64_t result = cs0 + cs1;
  s0 = cs0;
  cs1 ^= cs1 << 23;                          // a
  s1 = cs1 ^ cs0 ^ (cs1 >> 18) ^ (cs0 >> 5); // b, c
  return result;
}

int RandomNumberGenerator::randInt(int min, int max) {
  return min + (next() % (max - min + 1));
}

float RandomNumberGenerator::randFloat() { return next() / (float)UINT64_MAX; }

int RandomNumberGenerator::sampleFromProbabilities(const float *probabilities,
                                                   int numActions) {
  float random = randFloat();
  float current = 0.0f;

  for (int i = 0; i < numActions; i++) {
    current += probabilities[i];
    if (random < current) {
      return i;
    }
  }

  return numActions - 1;
}

} // namespace pokerai
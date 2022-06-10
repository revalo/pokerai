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
  cs1 ^= cs1 << 23;                           // a
  s1 = cs1 ^ cs0 ^ (cs1 >> 18) ^ (cs0 >> 5);  // b, c
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

UniqueRandomNumberGenerator::UniqueRandomNumberGenerator(int max,
                                                         uint64_t excludeMask) {
  // Count number of 1s in excludeMask.
  int numExclude = std::popcount(excludeMask);

  this->max = max - numExclude;
  slots = new int[(max + 1) - numExclude + 1];
  int slotIndex = 0;
  for (int i = 0; i < max + 1; i++) {
    if (excludeMask & (1ull << i)) {
      continue;
    }
    slots[slotIndex] = i;
    slotIndex++;
  }
  currentIndex = this->max;
  this->rng = RandomNumberGenerator();
}

UniqueRandomNumberGenerator::~UniqueRandomNumberGenerator() { delete[] slots; }

void UniqueRandomNumberGenerator::reset() { currentIndex = max; }

int UniqueRandomNumberGenerator::next() {
  if (currentIndex == 0) {
    throw std::runtime_error("Ran out of unique numbers!");
  }

  // Get a random index.
  int randIdx = rng.randInt(0, currentIndex);

  // Get the value at that index.
  int result = slots[randIdx];

  // Swap the random index with the last index.
  int lastIdx = slots[currentIndex];
  slots[currentIndex] = slots[randIdx];
  slots[randIdx] = lastIdx;

  // Decrement the current index.
  currentIndex--;

  return result;
}

}  // namespace pokerai
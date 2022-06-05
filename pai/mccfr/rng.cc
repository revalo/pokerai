#include "rng.h"

#include <cstdint>
#include <random>

namespace pokerai {
RandomNumberGenerator::RandomNumberGenerator(int seed) {
  srand(seed);
  s0 = rand();
  s1 = rand();
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

}  // namespace pokerai
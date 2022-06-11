#include "hand_indexer.h"

#include <bit>
#include <bitset>

namespace pokerai {
HandIndexer::HandIndexer() {
  nCrGroups = new long[MAX_GROUP_INDEX][N_SUITS + 1];
  memset(nCrGroups, 0, sizeof(long) * MAX_GROUP_INDEX * (N_SUITS + 1));

  for (uint_fast32_t i = 0; i < 1 << (N_SUITS - 1); ++i) {
    for (uint_fast32_t j = 1; j < N_SUITS; ++j) {
      equal[i][j] = i & 1 << (j - 1);
    }
  }

  for (uint_fast32_t i = 0; i < 1 << N_RANKS; ++i) {
    for (uint_fast32_t j = 0, set = ~i & (1 << N_RANKS) - 1; j < N_RANKS;
         ++j, set &= set - 1) {
      nthUnset[i][j] = set ? std::countr_zero(set) : 0xff;
    }
  }

  nCrRanks[0][0] = 1;
  for (uint_fast32_t i = 1; i < N_RANKS + 1; ++i) {
    nCrRanks[i][0] = nCrRanks[i][i] = 1;
    for (uint_fast32_t j = 1; j < i; ++j) {
      nCrRanks[i][j] = nCrRanks[i - 1][j - 1] + nCrRanks[i - 1][j];
    }
  }

  nCrGroups[0][0] = 1;
  for (uint_fast32_t i = 1; i < MAX_GROUP_INDEX; ++i) {
    nCrGroups[i][0] = 1;
    if (i < N_SUITS + 1) {
      nCrGroups[i][i] = 1;
    }
    for (uint_fast32_t j = 1; j < (i < (N_SUITS + 1) ? i : (N_SUITS + 1));
         ++j) {
      nCrGroups[i][j] = nCrGroups[i - 1][j - 1] + nCrGroups[i - 1][j];
    }
  }

  for (uint_fast32_t i = 0; i < 1 << N_RANKS; ++i) {
    for (uint_fast32_t set = i, j = 1; set; ++j, set &= set - 1) {
      rankSetToIndex[i] += nCrRanks[std::countr_zero(set)][j];
    }
    indexToRankSet[std::popcount(i)][rankSetToIndex[i]] = i;
  }

  uint_fast32_t num_permutations = 1;
  for (uint_fast32_t i = 2; i <= N_SUITS; ++i) {
    num_permutations *= i;
  }

  suitPermutations = new int[num_permutations][N_SUITS];
  memset(suitPermutations, 0, sizeof(int) * num_permutations * N_SUITS);

  for (uint_fast32_t i = 0; i < num_permutations; ++i) {
    for (uint_fast32_t j = 0, index = i, used = 0; j < N_SUITS; ++j) {
      uint_fast32_t suit = index % (N_SUITS - j);
      index /= N_SUITS - j;
      uint_fast32_t shifted_suit = nthUnset[used][suit];
      suitPermutations[i][j] = shifted_suit;
      used |= 1 << shifted_suit;
    }
  }
}

HandIndexer::~HandIndexer() {
  delete[] suitPermutations;
  delete[] nCrGroups;
}
}  // namespace pokerai
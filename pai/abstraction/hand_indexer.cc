#include "hand_indexer.h"

#include <bit>
#include <bitset>
#include <stdexcept>

namespace pokerai {
HandIndexer::HandIndexer(std::vector<int> cardsPerRound) {
  // Precompute tables.
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

  // Initialize hand indexer.
  this->cardsPerRound = cardsPerRound;
  rounds = cardsPerRound.size();

  permutationToConfiguration = new int*[rounds];
  permutationToPi = new int*[rounds];
  configurationToEqual = new int*[rounds];
  configuration = new int**[rounds];
  configurationToSuitSize = new int**[rounds];
  configurationToOffset = new long*[rounds];

  for (int i = 0, count = 0; i < rounds; ++i) {
    count += cardsPerRound[i];
    if (count > N_CARDS) throw std::runtime_error("Too many cards.");
  }

  roundStart = new int[rounds];
  memset(roundStart, 0, sizeof(int) * rounds);

  for (int i = 0, j = 0; i < rounds; ++i) {
    roundStart[i] = j;
    j += cardsPerRound[i];
  }

  configurations = new int[rounds];
  memset(configurations, 0, sizeof(int) * rounds);

  for (int i = 0; i < rounds; ++i) {
    configurationToEqual[i] = new int[configurations[i]];
    memset(configurationToEqual[i], 0, sizeof(int) * configurations[i]);

    configurationToOffset[i] = new long[configurations[i]];
    memset(configurationToOffset[i], 0, sizeof(long) * configurations[i]);

    configuration[i] = new int*[configurations[i]];
    configurationToSuitSize[i] = new int*[configurations[i]];
    for (int j = 0; j < configurations[i]; ++j) {
      configuration[i][j] = new int[N_SUITS];
      memset(configuration[i][j], 0, sizeof(int) * N_SUITS);
      configurationToSuitSize[i][j] = new int[N_SUITS];
      memset(configurationToSuitSize[i][j], 0, sizeof(int) * N_SUITS);
    }
  }

  roundSize = new long[rounds];
  for (int i = 0; i < rounds; ++i) {
    long accum = 0;
    for (int j = 0; j < configurations[i]; ++j) {
      long next = accum + configurationToOffset[i][j];
      configurationToOffset[i][j] = accum;
      accum = next;
    }
    roundSize[i] = accum;
  }

  permutations = new int[rounds];
  memset(permutations, 0, sizeof(int) * rounds);

  for (int i = 0; i < rounds; ++i) {
    permutationToConfiguration[i] = new int[permutations[i]];
    permutationToPi[i] = new int[permutations[i]];
  }
}

HandIndexer::~HandIndexer() {
  delete[] suitPermutations;
  delete[] nCrGroups;
}

void HandIndexer::EnumerateConfigurations(bool tabulate) {
  int used[N_SUITS] = {0};
  int configuration[N_SUITS] = {0};
  enumerateConfigurationsR(0, cardsPerRound[0], 0, ((1 << N_SUITS) - 2), used,
                           configuration, tabulate);
}

void HandIndexer::enumerateConfigurationsR(int round, int remaining, int suit,
                                           int equal, int* used,
                                           int* configuration, bool tabulate) {
  // hi
}

}  // namespace pokerai
#include "hand_indexer.h"

#include <bit>
#include <bitset>
#include <cmath>
#include <stdexcept>

namespace pokerai {
HandIndexer::HandIndexer(std::vector<int> cardsPerRound) {
  // Precompute tables.
  nCrGroups = new int64_t[MAX_GROUP_INDEX][N_SUITS + 1];
  memset(nCrGroups, 0, sizeof(int64_t) * MAX_GROUP_INDEX * (N_SUITS + 1));

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
  rounds = (int)cardsPerRound.size();

  permutationToConfiguration = new int*[rounds];
  permutationToPi = new int*[rounds];
  configurationToEqual = new int*[rounds];
  configuration = new int**[rounds];
  configurationToSuitSize = new int**[rounds];
  configurationToOffset = new int64_t*[rounds];

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
  EnumerateConfigurations(false);  // Count.

  for (int i = 0; i < rounds; ++i) {
    configurationToEqual[i] = new int[configurations[i]];
    memset(configurationToEqual[i], 0, sizeof(int) * configurations[i]);

    configurationToOffset[i] = new int64_t[configurations[i]];
    memset(configurationToOffset[i], 0, sizeof(int64_t) * configurations[i]);

    configuration[i] = new int*[configurations[i]];
    configurationToSuitSize[i] = new int*[configurations[i]];
    for (int j = 0; j < configurations[i]; ++j) {
      configuration[i][j] = new int[N_SUITS];
      memset(configuration[i][j], 0, sizeof(int) * N_SUITS);
      configurationToSuitSize[i][j] = new int[N_SUITS];
      memset(configurationToSuitSize[i][j], 0, sizeof(int) * N_SUITS);
    }
  }

  memset(configurations, 0, sizeof(int) * rounds);
  EnumerateConfigurations(true);  // Tabulate.

  roundSize = new int64_t[rounds];
  for (int i = 0; i < rounds; ++i) {
    int64_t accum = 0;
    for (int j = 0; j < configurations[i]; ++j) {
      int64_t next = accum + configurationToOffset[i][j];
      configurationToOffset[i][j] = accum;
      accum = next;
    }
    roundSize[i] = accum;
  }

  permutations = new int[rounds];
  memset(permutations, 0, sizeof(int) * rounds);
  enumeratePermutations(false);  // Count.

  for (int i = 0; i < rounds; ++i) {
    permutationToConfiguration[i] = new int[permutations[i]];
    permutationToPi[i] = new int[permutations[i]];
  }

  enumeratePermutations(true);  // Tabulate.

  publicFlopHands = new int*[roundSize[0]];
  for (int i = 0; i < roundSize[0]; ++i) {
    publicFlopHands[i] = new int[cardsPerRound[0]];  // check
    memset(publicFlopHands[i], 0, sizeof(int) * cardsPerRound[0]);
  }
}

HandIndexer::~HandIndexer() {
  delete[] suitPermutations;
  delete[] nCrGroups;

  // TODO(shreyask): Delete all the dynamically allocated memory.
}

void HandIndexer::CreatePublicFlopHands() {
  bool* publicFlopHandsFound = new bool[roundSize[0]];

  // Reset to 0.
  for (int i = 0; i < roundSize[0]; ++i) {
    memset(publicFlopHands[i], 0, sizeof(int) * cardsPerRound[0]);
  }

  for (int card1 = 0; card1 < 52; card1++) {
    for (int card2 = 0; card2 < 52; card2++) {
      for (int card3 = 0; card3 < 52; card3++) {
        if (card1 != card2 && card2 != card3 && card1 != card3) {
          int64_t index = indexLast(new int[]{card1, card2, card3});
          if (!publicFlopHandsFound[index]) {
            publicFlopHandsFound[index] = true;
            publicFlopHands[index] = new int[]{card1, card2, card3};
          }
        }
      }
    }
  }

  delete[] publicFlopHandsFound;
}

void HandIndexer::Swap(int* suitIndex, int u, int v) {
  if (suitIndex[u] > suitIndex[v]) {
    int tmp = suitIndex[u];
    suitIndex[u] = suitIndex[v];
    suitIndex[v] = tmp;
  }
}

int64_t HandIndexer::indexNextRound(HandIndexerState& state, int* cards) {
  int round = state.round++;

  int ranks[N_SUITS] = {0};
  int shiftedRanks[N_SUITS] = {0};

  for (int i = 0, j = roundStart[round]; i < cardsPerRound[round]; ++i, ++j) {
    int rank = cards[j] >> 2, suit = cards[j] & 3, rankBit = 1 << rank;
    ranks[suit] |= rankBit;
    shiftedRanks[suit] |=
        (rankBit >>
         std::popcount((uint32_t)((rankBit - 1) & state.usedRanks[suit])));
  }

  for (int i = 0; i < N_SUITS; ++i) {
    int usedSize = std::popcount((uint32_t)state.usedRanks[i]),
        thisSize = std::popcount((uint32_t)ranks[i]);
    state.suitIndex[i] +=
        state.suitMultiplier[i] * rankSetToIndex[shiftedRanks[i]];
    state.suitMultiplier[i] *= nCrRanks[N_RANKS - usedSize][thisSize];
    state.usedRanks[i] |= ranks[i];
  }

  for (int i = 0, remaining = cardsPerRound[round]; i < N_SUITS - 1; ++i) {
    int thisSize = std::popcount((uint32_t)(ranks[i]));
    state.permutationIndex += state.permutationMultiplier * thisSize;
    state.permutationMultiplier *= remaining + 1;
    remaining -= thisSize;
  }

  int configuration = permutationToConfiguration[round][state.permutationIndex];
  int piIndex = permutationToPi[round][state.permutationIndex];
  int equalIndex = configurationToEqual[round][configuration];
  int64_t offset = configurationToOffset[round][configuration];
  int* pi = suitPermutations[piIndex];

  int suitIndex[N_SUITS];
  int suitMultiplier[N_SUITS];
  memset(suitIndex, 0, sizeof(int) * N_SUITS);
  memset(suitMultiplier, 0, sizeof(int) * N_SUITS);

  for (int i = 0; i < N_SUITS; ++i) {
    suitIndex[i] = state.suitIndex[pi[i]];
    suitMultiplier[i] = state.suitMultiplier[pi[i]];
  }

  int64_t index = offset, multiplier = 1;

  for (int i = 0; i < N_SUITS;) {
    int64_t part, size;

    if (i + 1 < N_SUITS && equal[equalIndex, i + 1]) {
      if (i + 2 < N_SUITS && equal[equalIndex, i + 2]) {
        if (i + 3 < N_SUITS && equal[equalIndex, i + 3]) {
          Swap(suitIndex, i, i + 1);
          Swap(suitIndex, i + 2, i + 3);
          Swap(suitIndex, i, i + 2);
          Swap(suitIndex, i + 1, i + 3);
          Swap(suitIndex, i + 1, i + 2);
          part = suitIndex[i] + nCrGroups[suitIndex[i + 1] + 1][2] +
                 nCrGroups[suitIndex[i + 2] + 2][3] +
                 nCrGroups[suitIndex[i + 3] + 3][4];
          size = nCrGroups[suitMultiplier[i] + 3][4];
          i += 4;
        } else {
          Swap(suitIndex, i, i + 1);
          Swap(suitIndex, i, i + 2);
          Swap(suitIndex, i + 1, i + 2);
          part = suitIndex[i] + nCrGroups[suitIndex[i + 1] + 1][2] +
                 nCrGroups[suitIndex[i + 2] + 2][3];
          size = nCrGroups[suitMultiplier[i] + 2][3];
          i += 3;
        }
      } else {
        Swap(suitIndex, i, i + 1);
        part = suitIndex[i] + nCrGroups[suitIndex[i + 1] + 1][2];
        size = nCrGroups[suitMultiplier[i] + 1][2];
        i += 2;
      }
    } else {
      part = suitIndex[i];
      size = suitMultiplier[i];
      i += 1;
    }

    index += multiplier * part;
    multiplier *= size;
  }
  return index;
}

int64_t HandIndexer::indexAll(int* cards, int64_t* indices) {
  if (rounds > 0) {
    HandIndexerState state;
    for (int i = 0; i < rounds; i++) {
      indices[i] = indexNextRound(state, cards);
    }
    return indices[rounds - 1];
  }
  return 0;
}

int64_t HandIndexer::indexLast(int* cards) {
  int64_t* indices = new int64_t[rounds];
  memset(indices, 0, sizeof(int64_t) * rounds);
  auto rv = indexAll(cards, indices);
  delete[] indices;
  return rv;
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
  if (suit == N_SUITS) {
    if (tabulate) {
      tabulateConfigurations(round, configuration);
    } else {
      ++configurations[round];
    }

    if (round + 1 < rounds) {
      enumerateConfigurationsR(round + 1, cardsPerRound[round + 1], 0, equal,
                               used, configuration, tabulate);
    }
  } else {
    int min = 0;
    if (suit == N_SUITS - 1) {
      min = remaining;
    }

    int max = N_RANKS - used[suit];
    if (remaining < max) {
      max = remaining;
    }

    int previous = N_RANKS + 1;
    bool wasEqual = (equal & 1 << suit) != 0;
    if (wasEqual) {
      previous =
          configuration[suit - 1] >> (ROUND_SHIFT * (rounds - round - 1)) &
          ROUND_MASK;
      if (previous < max) {
        max = previous;
      }
    }

    int oldConfiguration = configuration[suit], oldUsed = used[suit];
    for (int i = min; i <= max; ++i) {
      int newConfiguration =
          oldConfiguration | i << (ROUND_SHIFT * (rounds - round - 1));
      int newEqual = ((equal & ~(1 << suit)) |
                      (wasEqual & (i == previous) ? 1 : 0) << suit);

      used[suit] = oldUsed + i;
      configuration[suit] = newConfiguration;
      enumerateConfigurationsR(round, remaining - i, suit + 1, newEqual, used,
                               configuration, tabulate);
      configuration[suit] = oldConfiguration;
      used[suit] = oldUsed;
    }
  }
}

void HandIndexer::tabulateConfigurations(int round, int* configuration) {
  int id = configurations[round]++;
  for (; id > 0; --id) {
    for (int i = 0; i < N_SUITS; ++i) {
      if (configuration[i] < this->configuration[round][id - 1][i]) {
        break;
      } else if (configuration[i] > this->configuration[round][id - 1][i]) {
        goto OUT;
      }
    }
    for (int i = 0; i < N_SUITS; ++i) {
      this->configuration[round][id][i] = this->configuration[round][id - 1][i];
      configurationToSuitSize[round][id][i] =
          configurationToSuitSize[round][id - 1][i];
    }
    configurationToOffset[round][id] = configurationToOffset[round][id - 1];
    configurationToEqual[round][id] = configurationToEqual[round][id - 1];
  }
OUT:
  configurationToOffset[round][id] = 1;
  memcpy(this->configuration[round][id], configuration, N_SUITS * sizeof(int));

  int equal = 0;
  for (int i = 0; i < N_SUITS;) {
    int size = 1;
    int j = 0;
    for (int remaining = N_RANKS; j <= round; ++j) {
      int ranks =
          configuration[i] >> (ROUND_SHIFT * (rounds - j - 1)) & ROUND_MASK;
      size *= nCrRanks[remaining][ranks];
      remaining -= ranks;
    }

    j = i + 1;
    while (j < N_SUITS && configuration[j] == configuration[i]) {
      ++j;
    }

    for (int k = i; k < j; ++k) {
      configurationToSuitSize[round][id][k] = size;
    }

    configurationToOffset[round][id] *= nCrGroups[size + j - i - 1][j - i];

    for (int k = i + 1; k < j; ++k) {
      equal |= 1 << k;
    }

    i = j;
  }

  configurationToEqual[round][id] = equal >> 1;
}

void HandIndexer::enumeratePermutations(bool tabulate) {
  int used[N_SUITS];
  int count[N_SUITS];

  memset(used, 0, sizeof(int) * N_SUITS);
  memset(count, 0, sizeof(int) * N_SUITS);

  enumeratePermutationsR(0, cardsPerRound[0], 0, used, count, tabulate);
}

void HandIndexer::enumeratePermutationsR(int round, int remaining, int suit,
                                         int* used, int* count, bool tabulate) {
  if (suit == N_SUITS) {
    if (tabulate) {
      tabulatePermutations(round, count);
    } else {
      countPermutations(round, count);
    }

    if (round + 1 < rounds) {
      enumeratePermutationsR(round + 1, cardsPerRound[round + 1], 0, used,
                             count, tabulate);
    }
  } else {
    int min = 0;
    if (suit == N_SUITS - 1) {
      min = remaining;
    }

    int max = N_RANKS - used[suit];
    if (remaining < max) {
      max = remaining;
    }

    int oldCount = count[suit], oldUsed = used[suit];
    for (int i = min; i <= max; ++i) {
      int newCount = oldCount | i << (ROUND_SHIFT * (rounds - round - 1));

      used[suit] = oldUsed + i;
      count[suit] = newCount;
      enumeratePermutationsR(round, remaining - i, suit + 1, used, count,
                             tabulate);
      count[suit] = oldCount;
      used[suit] = oldUsed;
    }
  }
}

void HandIndexer::countPermutations(int round, int* count) {
  int idx = 0, mult = 1;
  for (int i = 0; i <= round; ++i) {
    for (int j = 0, remaining = cardsPerRound[i]; j < N_SUITS - 1; ++j) {
      int size = count[j] >> ((rounds - i - 1) * ROUND_SHIFT) & ROUND_MASK;
      idx += mult * size;
      mult *= remaining + 1;
      remaining -= size;
    }
  }

  if (permutations[round] < idx + 1) {
    permutations[round] = idx + 1;
  }
}

void HandIndexer::tabulatePermutations(int round, int* count) {
  int idx = 0, mult = 1;
  for (int i = 0; i <= round; ++i) {
    for (int j = 0, remaining = cardsPerRound[i]; j < N_SUITS - 1; ++j) {
      int size = count[j] >> ((rounds - i - 1) * ROUND_SHIFT) & ROUND_MASK;
      idx += mult * size;
      mult *= remaining + 1;
      remaining -= size;
    }
  }

  int pi[N_SUITS];
  for (int i = 0; i < N_SUITS; ++i) {
    pi[i] = i;
  }

  for (int i = 1; i < N_SUITS; ++i) {
    int j = i, pi_i = pi[i];
    for (; j > 0; --j) {
      if (count[pi_i] > count[pi[j - 1]]) {
        pi[j] = pi[j - 1];
      } else {
        break;
      }
    }
    pi[j] = pi_i;
  }

  int pi_idx = 0, pi_mult = 1, pi_used = 0;
  for (int i = 0; i < N_SUITS; ++i) {
    int this_bit = (1 << pi[i]);
    int smaller = std::popcount((uint32_t)((this_bit - 1) & pi_used));
    pi_idx += (pi[i] - smaller) * pi_mult;
    pi_mult *= N_SUITS - i;
    pi_used |= this_bit;
  }

  permutationToPi[round][idx] = pi_idx;

  int low = 0, high = configurations[round];
  while (low < high) {
    int mid = (low + high) / 2;

    int compare = 0;
    for (int i = 0; i < N_SUITS; ++i) {
      int that = count[pi[i]];
      int other = configuration[round][mid][i];
      if (other > that) {
        compare = -1;
        break;
      } else if (other < that) {
        compare = 1;
        break;
      }
    }

    if (compare == -1) {
      high = mid;
    } else if (compare == 0) {
      low = high = mid;
    } else {
      low = mid + 1;
    }
  }

  permutationToConfiguration[round][idx] = low;
}

bool HandIndexer::unindex(int round, int64_t index, int* cards) {
  if (round >= rounds || index >= roundSize[round]) return false;
  int low = 0, high = configurations[round];
  int configurationIdx = 0;

  while ((uint32_t)low <
         (uint32_t)high)  // while (integer.compareUnsigned(low, high) < 0)
  {
    // int mid = integer.divideUnsigned(low + high, 2);
    int mid = ((low + high) / 2);
    if (configurationToOffset[round][mid] <= index) {
      configurationIdx = mid;
      low = mid + 1;
    } else {
      high = mid;
    }
  }
  index -= configurationToOffset[round][configurationIdx];

  int64_t suitIndex[N_SUITS];
  memset(suitIndex, 0, sizeof(int64_t) * N_SUITS);

  for (int i = 0; i < N_SUITS;) {
    int j = i + 1;
    while (j < N_SUITS && configuration[round][configurationIdx][j] ==
                              configuration[round][configurationIdx][i]) {
      ++j;
    }

    int suitSize = configurationToSuitSize[round][configurationIdx][i];
    int64_t groupSize = nCrGroups[suitSize + j - i - 1][j - i];
    // int groupIndex = int.remainderUnsigned(index, groupSize);

    int64_t groupIndex = (int64_t)((uint64_t)index % (uint64_t)groupSize);
    // index = int.divideUnsigned(index, groupSize);

    index = (int64_t)((uint64_t)index / (uint64_t)groupSize);

    for (; i < j - 1; ++i) {
      // suitIndex[i] = (int)Math.Floor(
      //     Math.Exp(Math.Log(groupIndex) / (j - i) - 1 + Math.Log(j - i)) - j
      //     - i);
      suitIndex[i] = (int)std::floor(
          std::exp(std::log(groupIndex) / (j - i) - 1 + std::log(j - i)) - j -
          i);

      // low = (int)Math.Floor(
      //     Math.Exp(Math.Log(groupIndex) / (j - i) - 1 + Math.Log(j - i)) - j
      //     - i);
      low = (int)std::floor(
          std::exp(std::log(groupIndex) / (j - i) - 1 + std::log(j - i)) - j -
          i);
      // high = (int)Math.Ceiling(
      //     Math.Exp(Math.Log(groupIndex) / (j - i) + Math.Log(j - i)) - j + i
      //     + 1);
      high = (int)std::ceil(
          std::exp(std::log(groupIndex) / (j - i) + std::log(j - i)) - j + i +
          1);
      if ((uint32_t)high > (uint32_t)suitSize) {
        high = suitSize;
      }
      if ((uint32_t)high <= (uint32_t)low) {
        low = 0;
      }
      while ((uint32_t)low < (uint32_t)high) {
        int mid = (int)((uint32_t)(low + high) / 2);
        if (nCrGroups[mid + j - i - 1][j - i] <= groupIndex) {
          suitIndex[i] = mid;
          low = mid + 1;
        } else {
          high = mid;
        }
      }
      groupIndex -= nCrGroups[(suitIndex[i] + j - i - 1)][j - i];
    }

    suitIndex[i] = groupIndex;
    ++i;
  }

  int* location = new int[rounds];
  memcpy(location, roundStart, sizeof(int) * rounds);
  for (int i = 0; i < N_SUITS; ++i) {
    int used = 0, m = 0;
    for (int j = 0; j < rounds; ++j) {
      int n = configuration[round][configurationIdx][i] >>
                  (ROUND_SHIFT * (rounds - j - 1)) &
              ROUND_MASK;
      int roundSize = nCrRanks[N_RANKS - m][n];
      m += n;
      int roundIdx = (int)((uint64_t)suitIndex[i] % (uint64_t)roundSize);
      suitIndex[i] = (int64_t)((uint64_t)suitIndex[i] / (uint64_t)roundSize);
      int shiftedCards = indexToRankSet[n][roundIdx], rankSet = 0;
      for (int k = 0; k < n; ++k) {
        int shiftedCard = shiftedCards & -shiftedCards;
        shiftedCards ^= shiftedCard;
        int card = nthUnset[used][std::countr_zero((uint32_t)shiftedCard)];
        rankSet |= (1 << card);
        cards[location[j]++] = card << 2 | i;
      }
      used |= rankSet;
    }
  }
  delete[] location;
  return true;
}

}  // namespace pokerai
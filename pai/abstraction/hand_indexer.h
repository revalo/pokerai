#ifndef HAND_INDEX_H
#define HAND_INDEX_H

#include <vector>

#define N_SUITS 4
#define N_RANKS 13
#define N_CARDS 52
#define MAX_GROUP_INDEX 0x100000
#define ROUND_SHIFT 4
#define ROUND_MASK 0xf

namespace pokerai {
class HandIndexerState {
 public:
  int suitIndex[N_SUITS];
  int suitMultiplier[N_SUITS];
  int round, permutationIndex, permutationMultiplier;
  int usedRanks[N_SUITS];

  HandIndexerState() {
    permutationMultiplier = 1;
    for (int i = 0; i < N_SUITS; i++) {
      suitIndex[i] = 0;
      suitMultiplier[i] = 1;
      usedRanks[i] = 0;
    }
  }
};

class HandIndexer {
  void EnumerateConfigurations(bool tabulate);
  void enumerateConfigurationsR(int round, int remaining, int suit, int equal,
                                int* used, int* configuration, bool tabulate);
  void tabulateConfigurations(int round, int* configuration);
  void enumeratePermutations(bool tabulate);
  void enumeratePermutationsR(int round, int remaining, int suit, int* used,
                              int* count, bool tabulate);
  void countPermutations(int round, int* count);
  void tabulatePermutations(int round, int* count);

 public:
  int nthUnset[1 << N_RANKS][N_RANKS] = {0};
  bool equal[1 << (N_SUITS - 1)][N_SUITS] = {false};
  int nCrRanks[N_RANKS + 1][N_RANKS + 1] = {0};
  int rankSetToIndex[1 << N_RANKS] = {0};
  int indexToRankSet[N_RANKS + 1][1 << N_RANKS] = {0};
  int (*suitPermutations)[N_SUITS] = {0};
  long (*nCrGroups)[N_SUITS + 1] = {0};

  int rounds;
  std::vector<int> cardsPerRound;
  int* configurations;
  int* permutations;
  long* roundSize;
  int* roundStart;
  int** permutationToConfiguration;
  int** permutationToPi;
  int** configurationToEqual;
  int*** configuration;
  int*** configurationToSuitSize;
  long** configurationToOffset;
  int* publicFlopHands;

  HandIndexer(std::vector<int> cardsPerRound);
  ~HandIndexer();
};
}  // namespace pokerai

#endif  // HAND_INDEX_H

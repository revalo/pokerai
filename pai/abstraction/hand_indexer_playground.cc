#include <chrono>
#include <iostream>
#include <vector>

#include "hand_indexer.h"

void main() {
  // pokerai::HandIndexer handIndexer(std::vector<int>{2, 3, 1, 1});
  pokerai::HandIndexer handIndexer(std::vector<int>{2, 3, 1, 1});
  for (int round = 0; round < handIndexer.rounds; round++) {
    std::cout << handIndexer.roundSize[round] << " non-isomorphic hands"
              << std::endl;
    std::cout << handIndexer.configurations[round] << " configurations"
              << std::endl;
    std::cout << handIndexer.permutations[round] << " permutations"
              << std::endl;
  }

  int cards[7] = {0};
  handIndexer.unindex(3, 7812, cards);

  std::cout << "Done!" << std::endl;
}

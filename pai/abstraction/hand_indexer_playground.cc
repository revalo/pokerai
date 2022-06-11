#include <chrono>
#include <iostream>
#include <vector>

#include "hand_indexer.h"

void main() {
  pokerai::HandIndexer handIndexer(std::vector<int>{3, 1, 1});
  std::cout << "Done!" << std::endl;
}

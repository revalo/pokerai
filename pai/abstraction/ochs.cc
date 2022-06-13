#include "ochs.h"

#include <fstream>
#include <iostream>

namespace pokerai {
OCHS::OCHS(const std::string &preflopClusterFilename) {
  indexer = new HandIndexer(std::vector<int>{2, 3, 1, 1});
  std::ifstream inputFile(preflopClusterFilename);
  std::string line;
  numPreflopClusters = 0;
  while (std::getline(inputFile, line)) {
    std::istringstream iss(line);
    int clusterId;
    iss >> clusterId;
    preflopClusterAssignments.push_back(clusterId);
    numPreflopClusters = std::max(numPreflopClusters, clusterId + 1);
  }
  inputFile.close();

  // Make preflopClusters.
  preflopClusters.resize(numPreflopClusters);
  preflopClustersMasks.resize(numPreflopClusters);
  int cards[2];
  for (size_t i = 0; i < preflopClusterAssignments.size(); i++) {
    int clusterId = preflopClusterAssignments[i];
    indexer->unindex(0, i, cards);
    preflopClusters[clusterId].push_back(Hand(cards[0]) + Hand(cards[1]));
    preflopClustersMasks[clusterId].push_back((1ULL << cards[0]) |
                                              (1ULL << cards[1]));
  }
}
OCHS::~OCHS() { delete indexer; }

void OCHS::computeOCHS(const Hand &playerHand, const Hand &boardHand,
                       uint64_t playerCardsMask, uint64_t boardCardsMask,
                       std::vector<uint16_t> &distribution) {
  // Compute the OCHS equity for each cluster.
  // Set distribution to zero for all clusters.
  for (size_t i = 0; i < numPreflopClusters; i++) {
    distribution[i] = 0;
  }
  uint64_t deadCardsMask = boardCardsMask | playerCardsMask;
  uint16_t playerRank = evaluator.evaluate(playerHand);

  Hand oppHand;
  uint64_t clusterMask;
  for (int clusterId = 0; clusterId < numPreflopClusters; clusterId++) {
    std::vector<Hand> &cluster = preflopClusters[clusterId];
    std::vector<uint64_t> &clusterMasks = preflopClustersMasks[clusterId];
    uint8_t clusterCount = 0;
    uint8_t distCount = 0;
    for (size_t i = 0; i < cluster.size(); i++) {
      clusterMask = clusterMasks[i];
      if (clusterMask & deadCardsMask) {
        continue;
      }

      oppHand = boardHand + cluster[i];
      uint16_t oppRank = evaluator.evaluate(oppHand);
      if (playerRank > oppRank) {
        distCount += 1;
      }
      clusterCount += 1;
    }
    distribution[clusterId] = (distCount << 8) | (clusterCount & 0xff);
  }
}

}  // namespace pokerai
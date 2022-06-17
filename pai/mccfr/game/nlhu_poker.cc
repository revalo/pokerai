#include "nlhu_poker.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

#include "../rng.h"
#include "absl/strings/str_join.h"

#define COERCE_PLAYER_INDEX(playerIndex, numPlayers) \
  (numPlayers + (playerIndex % numPlayers)) % numPlayers

namespace pokerai {
namespace game {
NLHUPokerNode::NLHUPokerNode() {}

NLHUPokerNode::~NLHUPokerNode() {}

NLHUPoker::NLHUPoker(std::string flopClustersFile, std::string turnClustersFile,
                     std::string riverClustersFile) {
  // Initialize indexer.
  handIndexer = new HandIndexer(std::vector<int>{2, 3, 1, 1});

  // Allocate memory for abstraction clusters.
  // Starts at one because preflop is not abstracted.
  for (int round = 1; round < 4; round++) {
    roundClusters[round].resize(handIndexer->roundSize[round], -1);
  }

  // Again starts at one because preflop is not abstracted.
  std::vector<std::string> clustersFilenames = {
      "", flopClustersFile, turnClustersFile, riverClustersFile};

  for (int round = 1; round < 4; round++) {
    std::cout << "Loading clusters from " << clustersFilenames[round]
              << std::endl;
    std::ifstream clustersFile(clustersFilenames[round]);
    if (!clustersFile.is_open()) {
      throw std::runtime_error("Could not open clusters file: " +
                               clustersFilenames[round]);
    }

    std::string line;
    int currentHandIndex = 0;
    while (std::getline(clustersFile, line)) {
      int clusterIndex = std::stoi(line);
      roundClusters[round][currentHandIndex] = clusterIndex;
      currentHandIndex++;
    }
  }
}

NLHUPoker::~NLHUPoker() { delete handIndexer; }

std::string NLHUPoker::name() const { return "nlhu_poker"; }

void NLHUPoker::getRootNode(NLHUPokerNode *resNode) {}

void NLHUPoker::sampleChance(NLHUPokerNode *node, NLHUPokerNode *resNode) {}

float NLHUPoker::getTerminalValue(NLHUPokerNode *node, int player) { return 0; }

bool NLHUPoker::isChance(NLHUPokerNode *node) { return false; }

bool NLHUPoker::isTerminal(NLHUPokerNode *node) { return false; }

int NLHUPoker::getDecidingPlayerIndex(NLHUPokerNode *node) { return 0; }
std::string NLHUPoker::getInfosetKey(NLHUPokerNode *node) { return ""; }

std::vector<int> *NLHUPoker::getValidActions(NLHUPokerNode *node) {
  return nullptr;
}

void NLHUPoker::takeAction(NLHUPokerNode *node, int action,
                           NLHUPokerNode *resNode) {}

}  // namespace game
}  // namespace pokerai

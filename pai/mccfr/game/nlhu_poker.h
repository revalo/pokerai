#ifndef GAME_NLHU_POKER_H
#define GAME_NLHU_POKER_H

#include <omp/HandEvaluator.h>

#include <string>
#include <vector>

#include "../../abstraction/hand_indexer.h"
#include "../rng.h"
#include "game.h"

#define MAX_NLHU_PLAYERS 2
#define NUM_NLHU_ROUNDS 4

namespace pokerai {
namespace game {
class NLHUPokerNode : public GameNode {
 public:
  bool deal;
  int decidingPlayerIndex;
  uint8_t holeCards[MAX_NLHU_PLAYERS][2] = {0};
  uint8_t boardCards[5] = {0};
  int pot = 0;
  int round = -1;

  int bets[MAX_NLHU_PLAYERS] = {0};
  int stacks[MAX_NLHU_PLAYERS] = {0};
  bool isPlayerInGame[MAX_NLHU_PLAYERS] = {0};  // Has player folded?
  bool isPlayerAllIn[MAX_NLHU_PLAYERS] = {false};

  // Action history.
  std::vector<uint8_t> history;

  NLHUPokerNode();
  ~NLHUPokerNode();
};

class NLHUPoker : public Game<NLHUPokerNode> {
 public:
  RandomNumberGenerator rng;
  HandIndexer *handIndexer;
  omp::HandEvaluator handEvaluator;

  std::vector<short> roundClusters[4];

  NLHUPoker(std::string flopClustersFile = "flop_clusters.txt",
            std::string turnClustersFile = "turn_clusters.txt",
            std::string riverClustersFile = "river_clusters.txt");
  ~NLHUPoker();
  std::string name() const;
  void getRootNode(NLHUPokerNode *resNode);
  void sampleChance(NLHUPokerNode *node, NLHUPokerNode *resNode);
  void takeAction(NLHUPokerNode *node, int action, NLHUPokerNode *resNode);

  bool isChance(NLHUPokerNode *node);
  bool isTerminal(NLHUPokerNode *node);
  int getDecidingPlayerIndex(NLHUPokerNode *node);
  std::string getInfosetKey(NLHUPokerNode *node);

  float getTerminalValue(NLHUPokerNode *node, int player);
  std::vector<int> *getValidActions(NLHUPokerNode *node);
};
}  // namespace game
}  // namespace pokerai

#endif  // GAME_NLHU_POKER_H

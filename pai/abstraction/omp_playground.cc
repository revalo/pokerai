#include <omp/EquityCalculator.h>
#include <omp/HandEvaluator.h>

#include <chrono>
#include <iostream>
#include <vector>

#include "../mccfr/rng.h"
#include "hand_distribution.h"

using namespace omp;

void main() {
  //   std::string playerCards = "TsJs";
  //   uint64_t playerCardsMask = CardRange::getCardMask(playerCards);
  //   std::vector<float> distribution;

  pokerai::HandDistribution distGenerator;
  //   distGenerator.getHandDistribution(playerCardsMask, distribution);
  //   for (int i = 0; i < distribution.size(); i++) {
  //     std::cout << distribution[i] << std::endl;
  //   }

  //   std::string playerCards = "TsJs";
  //   uint64_t playerCardsMask = CardRange::getCardMask(playerCards);
  //   std::vector<float> distribution;

  //   HandEvaluator evaluator;
  //   getHandDistribution(evaluator, playerCardsMask, distribution);
  //   for (int i = 0; i < distribution.size(); i++) {
  //     std::cout << distribution[i] << std::endl;
  //   }

  std::string playerCards = "AsJs";
  uint64_t boardCardsMask = CardRange::getCardMask("2c4c5hAcAd");
  HandEvaluator eval;
  uint64_t playerCardsMask = CardRange::getCardMask(playerCards);

  //   // Benchmark the hand strength calculation.
  uint64_t trials = 100000;
  Hand hand = Hand::empty() + Hand(1) + Hand(2) + Hand(3) + Hand(4) + Hand(5) +
              Hand(6) + Hand(7);
  auto start = std::chrono::high_resolution_clock::now();
  float v = 0;
  Hand playerHand =
      distGenerator.getBoardFromBitmask(playerCardsMask | boardCardsMask);
  Hand boardHand = distGenerator.getBoardFromBitmask(boardCardsMask);

  for (uint64_t i = 0; i < trials; i++) {
    v = distGenerator.handStrength(playerHand, boardHand, playerCardsMask,
                                   boardCardsMask);
    // eval.evaluate(hand);
  }

  std::cout << "Value: " << v << std::endl;

  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "Time: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                     start)
                   .count()
            << "ms" << std::endl;

  // Print executions per second.
  std::cout << "Executions per second: "
            << (trials * 1000000000) /
                   std::chrono::duration_cast<std::chrono::nanoseconds>(end -
                                                                        start)
                       .count()
            << std::endl;

  //   std::cout << "Mine: "
  //             << handStrength(eval, playerHand, playerCardsMask,
  //             boardCardsMask)
  //             << std::endl;

  std::vector<CardRange> ranges{playerCards, "random"};
  omp::EquityCalculator eq;
  eq.start(ranges, boardCardsMask, true);
  eq.wait();
  auto r = eq.getResults();
  std::cout << "Theirs: " << r.equity[0] << std::endl;
}
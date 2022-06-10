#include "hand_distribution.h"

#include <omp/EquityCalculator.h>
#include <omp/HandEvaluator.h>

#include <bit>
#include <bitset>
#include <iostream>

#include "../mccfr/rng.h"

uint64_t CARDS[53] = {0,
                      (1ULL << 1),
                      (1ULL << 2),
                      (1ULL << 3),
                      (1ULL << 4),
                      (1ULL << 5),
                      (1ULL << 6),
                      (1ULL << 7),
                      (1ULL << 8),
                      (1ULL << 9),
                      (1ULL << 10),
                      (1ULL << 11),
                      (1ULL << 12),
                      (1ULL << 13),
                      (1ULL << 14),
                      (1ULL << 15),
                      (1ULL << 16),
                      (1ULL << 17),
                      (1ULL << 18),
                      (1ULL << 19),
                      (1ULL << 20),
                      (1ULL << 21),
                      (1ULL << 22),
                      (1ULL << 23),
                      (1ULL << 24),
                      (1ULL << 25),
                      (1ULL << 26),
                      (1ULL << 27),
                      (1ULL << 28),
                      (1ULL << 29),
                      (1ULL << 30),
                      (1ULL << 31),
                      (1ULL << 32),
                      (1ULL << 33),
                      (1ULL << 34),
                      (1ULL << 35),
                      (1ULL << 36),
                      (1ULL << 37),
                      (1ULL << 38),
                      (1ULL << 39),
                      (1ULL << 40),
                      (1ULL << 41),
                      (1ULL << 42),
                      (1ULL << 43),
                      (1ULL << 44),
                      (1ULL << 45),
                      (1ULL << 46),
                      (1ULL << 47),
                      (1ULL << 48),
                      (1ULL << 49),
                      (1ULL << 50),
                      (1ULL << 51),
                      (1ULL << 52)};

#define OPP_COMBOS 990

namespace pokerai {
HandDistribution::HandDistribution() { evaluator = HandEvaluator(); }
HandDistribution::~HandDistribution() {}

Hand HandDistribution::getBoardFromBitmask(uint64_t cards) {
  Hand board = Hand::empty();
  for (unsigned c = 0; c < CARD_COUNT; ++c) {
    if (cards & (1ull << c)) board += c;
  }
  return board;
}

float HandDistribution::handStrength(const Hand &playerHand,
                                     const Hand &boardHand,
                                     uint64_t playerCardsMask,
                                     uint64_t boardCardsMask) {
  uint64_t usedCardsMask = playerCardsMask | boardCardsMask;

  float wins = 0;
  // int total = 0;

  uint64_t cardI = 0;
  uint64_t cardJ = 0;
  uint16_t playerRank = evaluator.evaluate(playerHand);

  for (unsigned i = 1; i <= CARD_COUNT; i++) {
    cardI = CARDS[i];
    if (usedCardsMask & cardI) continue;
    for (unsigned j = i + 1; j <= CARD_COUNT; j++) {
      cardJ = CARDS[j];
      if (usedCardsMask & cardJ) continue;

      Hand oppHand = boardHand + Hand(i) + Hand(j);
      uint16_t oppRank = evaluator.evaluate(oppHand);

      if (playerRank > oppRank) wins++;
    }
  }

  return wins / OPP_COMBOS;
}

void HandDistribution::getHandDistribution(uint64_t playerCardsMask,
                                           std::vector<float> &distribution,
                                           uint64_t existingBoardCardMask,
                                           int trials) {
  pokerai::UniqueRandomNumberGenerator rng(
      CARD_COUNT - 1, playerCardsMask | existingBoardCardMask);
  int numExisting = std::popcount(existingBoardCardMask);

  for (int trial = 0; trial < trials; trial++) {
    // Sample board cards.
    uint64_t boardCardsMask = existingBoardCardMask;
    for (int i = 0; i < (5 - numExisting); i++) {
      boardCardsMask |= (1ull << (rng.next() + 1));
    }
    rng.reset();

    Hand playerHand = getBoardFromBitmask(playerCardsMask | boardCardsMask);
    Hand boardHand = getBoardFromBitmask(boardCardsMask);
    float strength =
        handStrength(playerHand, boardHand, playerCardsMask, boardCardsMask);
    distribution.push_back(strength);
  }
}
}  // namespace pokerai

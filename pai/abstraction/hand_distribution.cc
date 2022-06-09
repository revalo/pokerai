#include "hand_distribution.h"

#include <omp/EquityCalculator.h>
#include <omp/HandEvaluator.h>

#include "../mccfr/rng.h"

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
                                     uint64_t playerCardsMask,
                                     uint64_t boardCardsMask) {
  uint64_t usedCardsMask = playerCardsMask | boardCardsMask;

  float wins = 0;
  int total = 0;

  uint64_t cardI = 0;
  uint64_t cardJ = 0;

  for (unsigned i = 1; i <= CARD_COUNT; i++) {
    cardI = (1ull << i);
    if (usedCardsMask & cardI) continue;
    for (unsigned j = i + 1; j <= CARD_COUNT; j++) {
      cardJ = (1ull << j);
      if (usedCardsMask & cardJ) continue;
      Hand oppHand = getBoardFromBitmask(cardI | cardJ | boardCardsMask);
      uint16_t playerRank = evaluator.evaluate(playerHand);
      uint16_t oppRank = evaluator.evaluate(oppHand);

      if (playerRank > oppRank) wins++;
      total++;
    }
  }

  return wins / total;
}

void HandDistribution::getHandDistribution(uint64_t playerCardsMask,
                                           std::vector<float> &distribution,
                                           int trials) {
  pokerai::UniqueRandomNumberGenerator rng(CARD_COUNT - 1, playerCardsMask);

  for (int trial = 0; trial < trials; trial++) {
    // Sample board cards.
    uint64_t boardCardsMask = 0;
    for (int i = 0; i < 5; i++) {
      boardCardsMask |= (1ull << (rng.next() + 1));
    }
    rng.reset();

    Hand playerHand = getBoardFromBitmask(playerCardsMask | boardCardsMask);
    float strength = handStrength(playerHand, playerCardsMask, boardCardsMask);
    distribution.push_back(strength);
  }
}
}  // namespace pokerai

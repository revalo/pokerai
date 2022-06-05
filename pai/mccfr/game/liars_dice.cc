#include "liars_dice.h"

namespace pokerai {
namespace game {
LiarsDiceGameNode::LiarsDiceGameNode(bool deal, int decidingPlayerIndex)
    : deal(deal), decidingPlayerIndex(decidingPlayerIndex) {}

LiarsDice::LiarsDice() {}
std::string LiarsDice::name() const { return "LiarsDice"; }
}  // namespace game
}  // namespace pokerai

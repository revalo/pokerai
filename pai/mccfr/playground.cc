#include <iostream>

#include "game/nlhu_poker.h"

using namespace std;

int main() {
  pokerai::game::NLHUPoker game("flop_clusters.txt", "turn_clusters.txt",
                                "river_clusters.txt");

  return 0;
}

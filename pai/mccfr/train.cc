#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include "external_sampling_mccfr.h"
#include "game/game.h"
#include "game/liars_dice.h"
#include "infotable.h"

using namespace std;

ABSL_FLAG(string, game, "liars_dice", "Name of the game.");
ABSL_FLAG(int, num_players, 2, "Number of players.");
ABSL_FLAG(int, num_dice, 2, "Number of dice.");
ABSL_FLAG(int, max_dice_face, 6, "Maximum dice face.");
ABSL_FLAG(int, seed, 0, "Random seed.");
ABSL_FLAG(int, iterations, 100000, "Number of iterations.");

void main(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);
  string gameName = absl::GetFlag(FLAGS_game);

  pokerai::game::Game *game;
  if (gameName == "liars_dice") {
    game = new pokerai::game::LiarsDice(
        absl::GetFlag(FLAGS_num_players), absl::GetFlag(FLAGS_num_dice),
        absl::GetFlag(FLAGS_max_dice_face), absl::GetFlag(FLAGS_seed));
  } else {
    cerr << "Unknown game: " << gameName << endl;
    exit(1);
  }

  pokerai::InfoTable infoTable("info_table.dat");
  pokerai::ExternalSamplingMCCFR mccfr(game, &infoTable);

  for (int i = 0; i < absl::GetFlag(FLAGS_iterations); i++) {
    if (i % 100 == 0) {
      cout << "Iteration " << i << endl;
    }
    for (int playerIndex = 0; playerIndex < absl::GetFlag(FLAGS_num_players);
         playerIndex++) {
      mccfr.singleIteration(game->getRootNode(), playerIndex);
    }
  }
}

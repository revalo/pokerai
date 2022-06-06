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
#include "rng.h"

using namespace std;

ABSL_FLAG(string, game, "liars_dice", "Name of the game.");
ABSL_FLAG(int, num_players, 2, "Number of players.");
ABSL_FLAG(int, num_dice, 2, "Number of dice.");
ABSL_FLAG(int, max_dice_face, 6, "Maximum dice face.");
ABSL_FLAG(int, seed, 0, "Random seed.");
ABSL_FLAG(int, iterations, 100000, "Number of iterations.");
ABSL_FLAG(bool, parallel, false, "Parallelize?");

int playRoundWithRandom(
    pokerai::game::Game<pokerai::game::LiarsDiceGameNode> *game,
    pokerai::InfoTable *infotable, int botPlayerIndex = 0) {
  pokerai::RandomNumberGenerator rng(0);

  pokerai::game::LiarsDiceGameNode currentNode =
      pokerai::game::LiarsDiceGameNode();

  game->getRootNode(&currentNode);
  game->sampleChance(&currentNode, &currentNode);

  while (true) {
    if (game->isTerminal(&currentNode)) {
      return game->getTerminalValue(&currentNode, botPlayerIndex);
    }

    auto validActions = game->getValidActions(&currentNode);
    pokerai::InfoSet infoset(validActions->size());

    if (game->getDecidingPlayerIndex(&currentNode) == botPlayerIndex) {
      infotable->get(game->getInfosetKey(&currentNode), &infoset);
      auto strategy = infoset.getAverageStrategy();
      auto actionIndex =
          rng.sampleFromProbabilities(strategy, validActions->size());
      auto action = validActions->at(actionIndex);
      game->takeAction(&currentNode, action, &currentNode);
    } else {
      auto actionIndex = rng.randInt(0, validActions->size() - 1);
      auto action = validActions->at(actionIndex);
      game->takeAction(&currentNode, action, &currentNode);
    }
  }
}

float evaluate(pokerai::game::Game<pokerai::game::LiarsDiceGameNode> *game,
               pokerai::InfoTable *infotable, int numGames,
               int botPlayerIndex = 0) {
  float total = 0;
  for (int i = 0; i < numGames; i++) {
    total += playRoundWithRandom(game, infotable, botPlayerIndex);
  }
  return total / numGames;
}

void main(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);
  string gameName = absl::GetFlag(FLAGS_game);

  pokerai::game::LiarsDice game = pokerai::game::LiarsDice(
      absl::GetFlag(FLAGS_num_players), absl::GetFlag(FLAGS_num_dice),
      absl::GetFlag(FLAGS_max_dice_face), absl::GetFlag(FLAGS_seed));

  pokerai::InfoTable infoTable("mimmitable");
  auto gamePtr =
      reinterpret_cast<pokerai::game::Game<pokerai::game::LiarsDiceGameNode> *>(
          &game);
  pokerai::ExternalSamplingMCCFR<pokerai::game::LiarsDiceGameNode> mccfr =
      pokerai::ExternalSamplingMCCFR<pokerai::game::LiarsDiceGameNode>(
          gamePtr, &infoTable, absl::GetFlag(FLAGS_parallel));

  for (int i = 0; i < absl::GetFlag(FLAGS_iterations); i++) {
    if (i % 100 == 0) {
      cout << "Iteration " << i << " Infosets: " << infoTable.getSize() << endl;
    }
    for (int playerIndex = 0; playerIndex < absl::GetFlag(FLAGS_num_players);
         playerIndex++) {
      pokerai::game::LiarsDiceGameNode rootNode =
          pokerai::game::LiarsDiceGameNode();
      game.getRootNode(&rootNode);
      mccfr.singleIteration(&rootNode, playerIndex);
    }
  }

  cout << "Evaluation: " << evaluate(&game, &infoTable, 1000) << endl;
}

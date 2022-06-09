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

ABSL_FLAG(int, num_players, 2, "Number of players.");
ABSL_FLAG(int, num_dice, 2, "Number of dice.");
ABSL_FLAG(int, max_dice_face, 6, "Maximum dice face.");
ABSL_FLAG(int, seed, 0, "Random seed.");
ABSL_FLAG(string, output_file, "strategy.dat", "Output file.");

void main(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);

  pokerai::RandomNumberGenerator rng(-1);

  pokerai::game::LiarsDice gameObj = pokerai::game::LiarsDice(
      absl::GetFlag(FLAGS_num_players), absl::GetFlag(FLAGS_num_dice),
      absl::GetFlag(FLAGS_max_dice_face), absl::GetFlag(FLAGS_seed));
  auto game = &gameObj;

  auto outputFilename = absl::GetFlag(FLAGS_output_file);

  pokerai::InfoTable infoTable("");
  cout << "Loading infotable... from " << outputFilename << endl;
  infoTable.loadFromFile(outputFilename);
  cout << "Loaded infotable." << endl;

  pokerai::game::LiarsDiceGameNode currentNode =
      pokerai::game::LiarsDiceGameNode();

  game->getRootNode(&currentNode);
  game->sampleChance(&currentNode, &currentNode);

  int botPlayerIndex = 1;

  cout << "Your dice are: ";
  for (int i = 0; i < absl::GetFlag(FLAGS_num_dice); i++) {
    cout << currentNode.dice[1 - botPlayerIndex][i] << " ";
  }
  cout << endl;

  while (true) {
    if (game->isTerminal(&currentNode)) {
      auto val = game->getTerminalValue(&currentNode, botPlayerIndex);

      // Print the dice.
      auto botDice = currentNode.dice[botPlayerIndex];

      cout << "Bot Dice: (";
      for (int i = 0; i < absl::GetFlag(FLAGS_num_dice); i++) {
        cout << botDice[i] << ",";
      }
      cout << ")" << endl;

      cout << "Val = " << val << endl;
      if (val > 0) {
        cout << "Bot won!" << endl;
      } else {
        cout << "Bot lost!" << endl;
      }

      break;
    }

    auto validActions = game->getValidActions(&currentNode);

    if (game->getDecidingPlayerIndex(&currentNode) == botPlayerIndex) {
      if (!infoTable.contains(game->getInfosetKey(&currentNode))) {
        cout << "Bot doesn't know." << endl;
      }

      auto infoset = infoTable.get(game->getInfosetKey(&currentNode));
      auto strategy = infoset->getAverageStrategy();
      auto actionIndex =
          rng.sampleFromProbabilities(strategy, validActions->size());
      auto action = validActions->at(actionIndex);

      if (action == game->liarAction) {
        cout << "Bot calls liar!" << endl;
      } else {
        auto actionQty = game->actionQuantity[action];
        auto actionFace = game->actionFace[action];

        cout << "Bot chooses action " << action << " (" << actionQty << ","
             << actionFace << ")" << endl;
      }

      game->takeAction(&currentNode, action, &currentNode);
    } else {
      int diceQty, diceFace;
      cin >> diceQty >> diceFace;

      if (diceQty < 0) {
        auto action = game->liarAction;
        game->takeAction(&currentNode, action, &currentNode);
      } else {
        for (int actionIndex = 0; actionIndex < game->maxNumActions;
             actionIndex++) {
          auto actionQty = game->actionQuantity[actionIndex];
          auto actionFace = game->actionFace[actionIndex];

          if (actionQty == diceQty && actionFace == diceFace) {
            bool takenAction = false;
            for (int i = 0; i < validActions->size(); i++) {
              if (validActions->at(i) == actionIndex) {
                game->takeAction(&currentNode, actionIndex, &currentNode);
                takenAction = true;
                break;
              }
            }

            if (!takenAction) {
              cout << "Invalid action!" << endl;
            }

            break;
          }
        }
      }
    }
  }
}

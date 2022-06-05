#ifndef GAME_H
#define GAME_H

#include <string>

namespace pokerai {
namespace game {
class GameNode {
 public:
  virtual bool isChance() = 0;
  virtual bool isTerminal() = 0;
  virtual int getDecidingPlayerIndex() = 0;
  // virtual std::string getInfosetKey() const = 0;
};

class Game {
 public:
  virtual std::string name() const = 0;
  virtual GameNode* getRootNode() = 0;
  virtual GameNode* sampleChance(GameNode* node) = 0;
  virtual int getTerminalValue(GameNode* node) = 0;
};
}  // namespace game
}  // namespace pokerai

#endif  // GAME_H

#ifndef GAME_H
#define GAME_H

#include <string>

namespace pokerai {
namespace game {
class GameNode {
 public:
  virtual bool isChance() const = 0;
  virtual bool isTerminal() const = 0;
  virtual int getDecidingPlayerIndex() const = 0;
  virtual std::string getInfosetKey() const = 0;
};

class Game {
 public:
  virtual std::string name() const = 0;
  GameNode* getRootNode() const;
};
}  // namespace game
}  // namespace pokerai

#endif  // GAME_H

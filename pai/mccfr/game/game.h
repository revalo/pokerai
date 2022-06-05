#ifndef GAME_H
#define GAME_H

#include <string>

namespace pokerai {
namespace game {
class GameNode {};

class Game {
 public:
  virtual std::string name() const = 0;
  virtual GameNode* getRootNode() = 0;
  virtual GameNode* sampleChance(GameNode* node) = 0;
  virtual GameNode* takeAction(GameNode* node, int action) = 0;

  virtual bool isChance(GameNode* node) = 0;
  virtual bool isTerminal(GameNode* node) = 0;
  virtual int getDecidingPlayerIndex(GameNode* node) = 0;
  virtual std::string getInfosetKey(GameNode* node) = 0;

  virtual int getTerminalValue(GameNode* node, int player) = 0;
  virtual std::vector<int> getValidActions(GameNode* node) = 0;
};
}  // namespace game
}  // namespace pokerai

#endif  // GAME_H

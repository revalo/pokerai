#ifndef GAME_H
#define GAME_H

#include <string>
#include <vector>

namespace pokerai {
namespace game {
class GameNode {};

template <typename T>
class Game {
 public:
  virtual std::string name() const = 0;
  virtual void getRootNode(T *resNode) = 0;
  virtual void sampleChance(T *node, T *resNode) = 0;
  virtual void takeAction(T *node, int action, T *resNode) = 0;

  virtual bool isChance(T *node) = 0;
  virtual bool isTerminal(T *node) = 0;
  virtual int getDecidingPlayerIndex(T *node) = 0;
  virtual std::string getInfosetKey(T *node) = 0;

  virtual int getTerminalValue(T *node, int player) = 0;
  virtual std::vector<int> *getValidActions(T *node) = 0;
};
}  // namespace game
}  // namespace pokerai

#endif  // GAME_H

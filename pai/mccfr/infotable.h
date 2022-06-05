#ifndef INFOTABLE_H
#define INFOTABLE_H

#include <map>
#include <string>

#include "infoset.h"

namespace pokerai {
class InfoTable {
  std::string filename;
  std::map<std::string, InfoSet*> infosets;

 public:
  InfoTable(const std::string& filename);
  void clear();
  bool contains(const std::string& key);
  InfoSet* get(const std::string& key, int numActions = 0);
};
}  // namespace pokerai

#endif  // INFOTABLE_H

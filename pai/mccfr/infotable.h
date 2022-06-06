#ifndef INFOTABLE_H
#define INFOTABLE_H

#include <map>
#include <mutex>
#include <string>

#include "absl/container/flat_hash_map.h"
#include "infoset.h"

namespace pokerai {
class InfoTable {
  std::string filename;
  bool diskBacked;
  absl::flat_hash_map<std::string, InfoSet *> infosets;
  // Haven't been written to disk.
  std::map<std::string, bool> dirty;

 public:
  InfoTable(const std::string &filename);
  void clear();
  bool contains(const std::string &key);
  InfoSet *get(const std::string &key, int numActions = 0);
  size_t getSize();
};
}  // namespace pokerai

#endif  // INFOTABLE_H

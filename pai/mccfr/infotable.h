#ifndef INFOTABLE_H
#define INFOTABLE_H

#include <map>
#include <mutex>
#include <string>

#include "absl/container/flat_hash_map.h"
#include "infoset.h"
#include "leveldb/db.h"

namespace pokerai {
class InfoTable {
  std::string filename;
  bool diskBacked;
  absl::flat_hash_map<std::string, InfoSet *> infosets;
  absl::flat_hash_map<std::string, std::string> infostrings;
  // Haven't been written to disk.
  std::map<std::string, bool> dirty;
  leveldb::DB *db;

 public:
  InfoTable(const std::string &filename);
  ~InfoTable();
  void clear();
  bool contains(const std::string &key);
  void get(const std::string &key, InfoSet *infoset);
  void put(const std::string &key, InfoSet *infoset);
  size_t getSize();
};
}  // namespace pokerai

#endif  // INFOTABLE_H

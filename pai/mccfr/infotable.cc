#include "infotable.h"

#include "infoset.h"

pokerai::InfoTable::InfoTable(const std::string &filename) {
  this->filename = filename;
  this->diskBacked = filename.length() > 0;
}

void pokerai::InfoTable::clear() {
  for (std::map<std::string, InfoSet *>::iterator it = infosets.begin();
       it != infosets.end(); ++it) {
    delete it->second;
  }
  infosets.clear();
}

bool pokerai::InfoTable::contains(const std::string &key) {
  return infosets.find(key) != infosets.end();
}

pokerai::InfoSet *pokerai::InfoTable::get(const std::string &key,
                                          int numActions) {
  std::map<std::string, InfoSet *>::iterator it = infosets.find(key);
  if (it == infosets.end()) {
    InfoSet *infoSet = new InfoSet(numActions);
    infosets[key] = infoSet;
    return infoSet;
  } else {
    return it->second;
  }
}

size_t pokerai::InfoTable::getSize() { return infosets.size(); }

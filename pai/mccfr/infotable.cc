#include "infotable.h"

#include "infoset.h"

#define MOCK_RAM 0

pokerai::InfoTable::InfoTable(const std::string &filename) {
  this->filename = filename;
  this->diskBacked = filename.length() > 0;

#ifndef MOCK_RAM
  leveldb::Options options;
  options.create_if_missing = true;
  options.write_buffer_size = 4294967296ULL;
  leveldb::Status status = leveldb::DB::Open(options, filename, &db);
  assert(status.ok());
#endif
}

pokerai::InfoTable::~InfoTable() { delete db; }

void pokerai::InfoTable::clear() {
  // for (std::map<std::string, InfoSet *>::iterator it = infosets.begin();
  //      it != infosets.end(); ++it) {
  //   delete it->second;
  // }
  // infosets.clear();
}

bool pokerai::InfoTable::contains(const std::string &key) {
#ifdef MOCK_RAM
  return infostrings.find(key) != infostrings.end();
#else
  std::string value;
  return db->Get(leveldb::ReadOptions(), key, &value).ok();
#endif
}

void pokerai::InfoTable::get(const std::string &key,
                             pokerai::InfoSet *infoset) {
#ifdef MOCK_RAM
  auto it = infostrings.find(key);
  if (it == infostrings.end()) {
    infoset->reset();
    infostrings[key] = infoset->serialize();
    return;
  }

  infoset->deserialize(it->second);
#else
  std::string value;
  auto res = db->Get(leveldb::ReadOptions(), key, &value);
  if (res.ok()) {
    infoset->deserialize(value);
    return;
  }

  infoset->reset();
  db->Put(leveldb::WriteOptions(), key, infoset->serialize());
#endif
}

void pokerai::InfoTable::put(const std::string &key,
                             pokerai::InfoSet *infoset) {
#ifdef MOCK_RAM
  infostrings[key] = infoset->serialize();
#else
  db->Put(leveldb::WriteOptions(), key, infoset->serialize());
#endif
}

size_t pokerai::InfoTable::getSize() {
#ifdef MOCK_RAM
  return infostrings.size();
#else
  return 0;
#endif
}

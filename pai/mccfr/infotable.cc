#include "infotable.h"

#include "infoset.h"
#include "leveldb/db.h"

pokerai::InfoTable::InfoTable(const std::string &filename) {
  this->filename = filename;
  this->diskBacked = filename.length() > 0;
}

void pokerai::InfoTable::clear() {
  // for (std::map<std::string, InfoSet *>::iterator it = infosets.begin();
  //      it != infosets.end(); ++it) {
  //   delete it->second;
  // }
  // infosets.clear();
}

bool pokerai::InfoTable::contains(const std::string &key) {
  return infosets.find(key) != infosets.end();
}

pokerai::InfoSet *pokerai::InfoTable::get(const std::string &key,
                                          int numActions) {
  auto res = infosets.find(key);
  if (res != infosets.end()) {
    return res->second;
  }

  InfoSet *infoSet = new InfoSet(numActions);
  infosets[key] = infoSet;
  return infoSet;
}

size_t pokerai::InfoTable::getSize() { return infosets.size(); }

void pokerai::InfoTable::writeToFile(std::string filename) {
  leveldb::DB *db;
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, filename, &db);
  assert(status.ok());

  char *scratch = new char[1024];

  for (auto it = infosets.begin(); it != infosets.end(); ++it) {
    auto key = it->first;
    auto infoSet = it->second;
    size_t bytesPerArray = sizeof(float) * infoSet->numActions;
    memcpy(scratch, infoSet->regretSums, bytesPerArray);
    memcpy(scratch + bytesPerArray, infoSet->strategySums, bytesPerArray);
    leveldb::Slice slice(scratch, bytesPerArray * 2);
    status = db->Put(leveldb::WriteOptions(), key, slice);
    assert(status.ok());
  }

  delete scratch;
  delete db;
}

void pokerai::InfoTable::loadFromFile(std::string filename) {
  leveldb::DB *db;
  leveldb::Options options;
  leveldb::Status status = leveldb::DB::Open(options, filename, &db);
  assert(status.ok());

  leveldb::Iterator *it = db->NewIterator(leveldb::ReadOptions());
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    auto key = it->key();
    auto value = it->value();

    size_t numActions = value.size() / (sizeof(float) * 2);
    size_t bytesPerArray = sizeof(float) * numActions;
    InfoSet *infoSet = new InfoSet(numActions);
    memcpy(infoSet->regretSums, value.data(), bytesPerArray);
    memcpy(infoSet->strategySums, value.data() + bytesPerArray, bytesPerArray);
    infosets[key.ToString()] = infoSet;
  }
}
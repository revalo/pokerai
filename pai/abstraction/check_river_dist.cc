#include <algorithm>
#include <execution>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "hand_distribution.h"
#include "hand_indexer.h"
#include "ochs.h"

using namespace std;

ABSL_FLAG(string, input_file, "river_dist.dat", "Output file.");

#define NUM_BUCKETS 8

int main(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);

  pokerai::HandIndexer indexer(std::vector<int>{2, 3, 1, 1});
  size_t numHands = indexer.roundSize[3];

  std::string inputFilename = absl::GetFlag(FLAGS_input_file);
  std::ifstream inputFile(inputFilename, ios::in | ios::binary);
  if (!inputFile.is_open()) {
    std::cerr << "Could not open input file " << inputFilename << std::endl;
    return 1;
  }

  std::vector<uint16_t> dist_uint16(NUM_BUCKETS);
  std::vector<bool> marked(numHands, false);
  size_t numDone = 0;
  while (!inputFile.eof()) {
    int64_t handIndex;
    inputFile.read(reinterpret_cast<char *>(&handIndex), sizeof(int64_t));
    inputFile.read(reinterpret_cast<char *>(dist_uint16.data()),
                   sizeof(uint16_t) * NUM_BUCKETS);
    marked[handIndex] = true;
    numDone++;

    if (numDone % 1000000 == 0) {
      std::cout << "Done " << numDone << " hands." << std::endl;
      // Output percent done.
      std::cout << fixed << setprecision(2) << setw(6) << setfill(' ')
                << (double)numDone / numHands * 100 << "% done" << std::endl;
      std::cout << "Stream position: " << inputFile.tellg() << std::endl;
    }
  }
  inputFile.close();

  cout << "Checking!" << endl;
  for (size_t i = 0; i < numHands; i++) {
    if (!marked[i]) {
      cout << "Hand " << i << " not marked." << endl;
    }
  }

  cout << "Done!" << endl;

  return 0;
}

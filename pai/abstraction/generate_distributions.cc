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

ABSL_FLAG(int, num_samples, 1000, "Number of samples to use.");
ABSL_FLAG(string, output_file, "dists.dat", "Output file.");
ABSL_FLAG(string, round, "preflop", "Output file.");
ABSL_FLAG(string, preflop_cluster, "preflop_cluster_8.txt",
          "Preflop cluster assignments for OCHS.");

#define NUM_BUCKETS 50

#define OCHS_BATCH_SIZE 100000

void ochsRiverDists() {
  cout << "Doing OCHS river distributions." << endl;
  pokerai::OCHS ochs(absl::GetFlag(FLAGS_preflop_cluster));
  size_t numHands = ochs.indexer->roundSize[3];
  cout << "River has " << numHands << " hands." << endl;
  cout << "Num clusters " << ochs.numPreflopClusters << endl;

  string outputFilename = absl::GetFlag(FLAGS_output_file);
  cout << "Outputting to " << outputFilename << endl;
  ofstream outputFile(outputFilename, ios::out | ios::binary);
  mutex outputMutex;
  char *buffer = new char[sizeof(float) * ochs.numPreflopClusters];

  size_t numDone = 0;

  std::thread progressThread([&]() {
    int lastNumDone = 0;
    while (true) {
      this_thread::sleep_for(chrono::seconds(2));

      outputMutex.lock();
      // Print percentage done with two decimal places.
      cout << fixed << setprecision(2) << setw(6) << setfill(' ')
           << (double)numDone / numHands * 100 << "% done" << endl;
      cout << "Generated " << numDone << " distributions." << endl;
      cout << "Speed is " << (numDone - lastNumDone) / 2 << " per second."
           << endl;
      // Print ETA.
      int64_t eta = (numHands - numDone) / (numDone - lastNumDone) / 2;
      cout << "ETA: " << eta / 3600 << " hours, " << (eta % 3600) / 60
           << " minutes, " << (eta % 3600) % 60 << " seconds." << endl;
      cout << endl;
      outputMutex.unlock();

      lastNumDone = numDone;
    }
  });

  vector<size_t> indexes(OCHS_BATCH_SIZE);
  for (size_t currentIndex = 0; currentIndex < numHands;
       currentIndex += OCHS_BATCH_SIZE) {
    for (size_t i = 0; i < OCHS_BATCH_SIZE; i++) {
      indexes[i] = currentIndex + i;
    }

    std::for_each(
        std::execution::par, indexes.begin(), indexes.end(), [&](size_t index) {
          if (index >= numHands) {
            return;
          }
          std::vector<uint16_t> distribution(ochs.numPreflopClusters);
          int cards[7];
          ochs.indexer->unindex(3, index, cards);
          uint64_t playerCardsMask = (1ULL << cards[0]) | (1ULL << cards[1]);
          uint64_t boardCardsMask = (1ULL << cards[2]) | (1ULL << cards[3]) |
                                    (1ULL << cards[4]) | (1ULL << cards[5]) |
                                    (1ULL << cards[6]);
          Hand boardHand = Hand::empty() + Hand(cards[2]) + Hand(cards[3]) +
                           Hand(cards[4]) + Hand(cards[5]) + Hand(cards[6]);
          Hand playerHand = boardHand + Hand(cards[0]) + Hand(cards[1]);
          ochs.computeOCHS(playerHand, boardHand, playerCardsMask,
                           boardCardsMask, distribution);
          int64_t tag[1] = {(int64_t)index};
          char tagBuffer[sizeof(tag)];
          memcpy(tagBuffer, tag, sizeof(tag));

          outputMutex.lock();
          memcpy(buffer, distribution.data(),
                 sizeof(uint16_t) * ochs.numPreflopClusters);
          outputFile.write(tagBuffer, sizeof(tag));
          outputFile.write(buffer, sizeof(uint16_t) * ochs.numPreflopClusters);
          numDone++;
          outputMutex.unlock();
        });
  }

  outputFile.close();
}

int main(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);

  int numSamples = absl::GetFlag(FLAGS_num_samples);
  string roundStr = absl::GetFlag(FLAGS_round);

  if (roundStr == "river") {
    // Use OCHS to cluster the river hand distributions.
    ochsRiverDists();
    cout << "Done." << endl;
    return 0;
  }

  std::vector<int> roundCards = {2, 3, 1, 1};

  pokerai::HandDistribution distGenerator;
  pokerai::HandIndexer handIndexer(roundCards);

  string outputFilename = absl::GetFlag(FLAGS_output_file);
  cout << "Outputting to " << outputFilename << endl;

  char *buffer = new char[sizeof(float) * numSamples];
  ofstream outputFile(outputFilename, ios::out | ios::binary);
  mutex outputMutex;

  int round = 0;

  if (roundStr == "preflop") {
    round = 0;
  } else if (roundStr == "flop") {
    round = 1;
  } else if (roundStr == "turn") {
    round = 2;
  } else if (roundStr == "river") {
    round = 3;
  } else {
    cout << "Invalid round: " << roundStr << endl;
    return 1;
  }

  int numBoardCards = 0;
  for (int i = 1; i <= round; i++) {
    numBoardCards += roundCards[i];
  }
  cout << "Num Board Cards: " << numBoardCards << endl;

  int64_t numIndexes = handIndexer.roundSize[round];
  cout << "There are " << numIndexes << " non-isomorphic hands." << endl;
  cout << "Loading index vectors..." << endl;

  std::vector<int64_t> indexes(numIndexes);
  for (int64_t i = 0; i < numIndexes; i++) {
    indexes[i] = i;
  }

  cout << "Index vectors loaded." << endl;

  int numDone = 0;

  cout << "Starting to generate distributions..." << endl;

  std::thread progressThread([&]() {
    int lastNumDone = 0;
    while (true) {
      this_thread::sleep_for(chrono::seconds(2));

      outputMutex.lock();
      // Print percentage done with two decimal places.
      cout << fixed << setprecision(2) << setw(6) << setfill(' ')
           << (double)numDone / numIndexes * 100 << "% done" << endl;
      cout << "Generated " << numDone << " distributions." << endl;
      cout << "Speed is " << (numDone - lastNumDone) / 2 << " per second."
           << endl;
      // Print ETA.
      int64_t eta = (numIndexes - numDone) / (numDone - lastNumDone) / 2;
      cout << "ETA: " << eta / 3600 << " hours, " << (eta % 3600) / 60
           << " minutes, " << (eta % 3600) % 60 << " seconds." << endl;
      cout << endl;
      outputMutex.unlock();

      lastNumDone = numDone;
    }
  });

  std::for_each(
      std::execution::par, indexes.begin(), indexes.end(),
      [&distGenerator, &numSamples, &buffer, &outputFile, &outputMutex,
       &handIndexer, &round, &numBoardCards, &numDone](auto &&index) {
        vector<float> dist;
        uint16_t buckets[NUM_BUCKETS];
        int cards[7] = {0};
        handIndexer.unindex(round, index, cards);

        uint64_t playerCardsMask = (1ULL << cards[0]) | (1ULL << cards[1]);
        uint64_t boardCardsMask = 0;
        for (int i = 2; i < 2 + numBoardCards; i++) {
          boardCardsMask |= (1ULL << cards[i]);
        }

        distGenerator.getHandDistribution(playerCardsMask, dist, boardCardsMask,
                                          numSamples);
        distGenerator.makeBuckets(dist, buckets, NUM_BUCKETS);

        int64_t tag[1] = {index};
        char tagBuffer[sizeof(tag)];
        memcpy(tagBuffer, tag, sizeof(tag));

        outputMutex.lock();
        memcpy(buffer, buckets, sizeof(uint16_t) * NUM_BUCKETS);
        outputFile.write(tagBuffer, sizeof(tag));
        outputFile.write(buffer, sizeof(uint16_t) * NUM_BUCKETS);
        numDone++;
        outputMutex.unlock();
      });

  outputFile.close();
  delete[] buffer;

  return 0;
}

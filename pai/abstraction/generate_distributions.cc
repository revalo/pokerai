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

using namespace std;

ABSL_FLAG(int, num_samples, 1000, "Number of samples to use.");
ABSL_FLAG(string, output_file, "dists.dat", "Output file.");
ABSL_FLAG(string, round, "preflop", "Output file.");

#define NUM_BUCKETS 50

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  int numSamples = absl::GetFlag(FLAGS_num_samples);
  string roundStr = absl::GetFlag(FLAGS_round);

  std::vector<int> roundCards = {2, 3, 1, 1};

  pokerai::HandDistribution distGenerator;
  pokerai::HandIndexer handIndexer(roundCards);

  string outputFilename = absl::GetFlag(FLAGS_output_file);
  cout << "Outputting to " << outputFilename << endl;

  char* buffer = new char[sizeof(float) * numSamples];
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

  std::vector<int> indexes(numIndexes);
  for (int i = 0; i < numIndexes; i++) {
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
       &handIndexer, &round, &numBoardCards, &numDone](auto&& index) {
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

        int tag[1] = {index};
        char tagBuffer[sizeof(tag)];
        memcpy(tagBuffer, tag, sizeof(tag));

        outputMutex.lock();
        memcpy(buffer, buckets, sizeof(uint16_t) * NUM_BUCKETS);
        outputFile.write(tagBuffer, sizeof(tag));
        outputFile.write(buffer, sizeof(uint16_t) * NUM_BUCKETS);
        numDone++;
        outputMutex.unlock();
      });

  // if (round == "preflop") {
  //   for (int i = 1; i <= 52; i++) {
  //     cout << "Card " << i << endl;
  //     for (int j = i + 1; j <= 52; j++) {
  //       vector<float> dist;
  //       uint64_t playerCardsMask = (1ULL << i) | (1ULL << j);
  //       distGenerator.getHandDistribution(playerCardsMask, dist, 0,
  //       numSamples); memcpy(buffer, dist.data(), sizeof(float) *
  //       dist.size()); outputFile.write(buffer, sizeof(float) *
  //       dist.size());
  //     }
  //   }
  // } else if (round == "flop") {
  //   // Player hand.
  //   for (int i = 1; i <= 52; i++) {
  //     cout << "Card " << i << endl;

  //     vector<int> indexes;
  //     for (int j = i + 1; j <= 52; j++) {
  //       indexes.push_back(j);
  //     }

  //     std::for_each(
  //         std::execution::par, indexes.begin(), indexes.end(),
  //         [&distGenerator, &numSamples, &buffer, &outputFile,
  //         &outputMutex,
  //          i](auto&& j) {
  //           // Flop cards.
  //           for (int k = j + 1; k <= 52; k++) {
  //             for (int l = k + 1; l <= 52; l++) {
  //               for (int m = l + 1; m <= 52; m++) {
  //                 vector<float> dist;
  //                 uint64_t playerCardsMask = (1ULL << i) | (1ULL << j);
  //                 uint64_t boardCardsMask =
  //                     (1ULL << k) | (1ULL << l) | (1ULL << m);
  //                 distGenerator.getHandDistribution(playerCardsMask,
  //                 dist,
  //                                                   boardCardsMask,
  //                                                   numSamples);

  //                 int tag[5] = {i, j, k, l, m};
  //                 char tagBuffer[sizeof(tag)];
  //                 memcpy(tagBuffer, tag, sizeof(tag));

  //                 outputMutex.lock();
  //                 memcpy(buffer, dist.data(), sizeof(float) *
  //                 dist.size()); outputFile.write(tagBuffer, sizeof(tag));
  //                 outputFile.write(buffer, sizeof(float) * dist.size());
  //                 outputMutex.unlock();
  //               }
  //             }
  //           }
  //         });
  //   }
  // } else {
  //   cout << "Unknown round: " << round << endl;
  // }

  outputFile.close();
  delete[] buffer;

  return 0;
}

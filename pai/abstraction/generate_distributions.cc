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

using namespace std;

ABSL_FLAG(int, num_threads, 1, "Number of threads to use.");
ABSL_FLAG(int, num_samples, 1000, "Number of samples to use.");
ABSL_FLAG(string, output_file, "dists.dat", "Output file.");
ABSL_FLAG(string, round, "preflop", "Output file.");

void main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  int numSamples = absl::GetFlag(FLAGS_num_samples);
  string round = absl::GetFlag(FLAGS_round);

  pokerai::HandDistribution distGenerator;

  string outputFilename = absl::GetFlag(FLAGS_output_file);
  cout << "Outputting to " << outputFilename << endl;

  char* buffer = new char[sizeof(float) * numSamples];
  ofstream outputFile(outputFilename, ios::out | ios::binary);
  mutex outputMutex;

  if (round == "preflop") {
    for (int i = 1; i <= 52; i++) {
      cout << "Card " << i << endl;
      for (int j = i + 1; j <= 52; j++) {
        vector<float> dist;
        uint64_t playerCardsMask = (1ULL << i) | (1ULL << j);
        distGenerator.getHandDistribution(playerCardsMask, dist, 0, numSamples);
        memcpy(buffer, dist.data(), sizeof(float) * dist.size());
        outputFile.write(buffer, sizeof(float) * dist.size());
      }
    }
  } else if (round == "flop") {
    // Player hand.
    for (int i = 1; i <= 52; i++) {
      cout << "Card " << i << endl;

      vector<int> indexes;
      for (int j = i + 1; j <= 52; j++) {
        indexes.push_back(j);
      }

      std::for_each(
          std::execution::par, indexes.begin(), indexes.end(),
          [&distGenerator, &numSamples, &buffer, &outputFile, &outputMutex,
           i](auto&& j) {
            // Flop cards.
            for (int k = j + 1; k <= 52; k++) {
              for (int l = k + 1; l <= 52; l++) {
                for (int m = l + 1; m <= 52; m++) {
                  vector<float> dist;
                  uint64_t playerCardsMask = (1ULL << i) | (1ULL << j);
                  uint64_t boardCardsMask =
                      (1ULL << k) | (1ULL << l) | (1ULL << m);
                  distGenerator.getHandDistribution(playerCardsMask, dist,
                                                    boardCardsMask, numSamples);

                  int tag[5] = {i, j, k, l, m};
                  char tagBuffer[sizeof(tag)];
                  memcpy(tagBuffer, tag, sizeof(tag));

                  outputMutex.lock();
                  memcpy(buffer, dist.data(), sizeof(float) * dist.size());
                  outputFile.write(tagBuffer, sizeof(tag));
                  outputFile.write(buffer, sizeof(float) * dist.size());
                  outputMutex.unlock();
                }
              }
            }
          });
    }
  } else {
    cout << "Unknown round: " << round << endl;
  }

  outputFile.close();
  delete[] buffer;
}

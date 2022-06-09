#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "hand_distribution.h"

using namespace std;

ABSL_FLAG(string, output_file, "dists.dat", "Output file.");

void main(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);

  pokerai::HandDistribution distGenerator;

  string outputFilename = absl::GetFlag(FLAGS_output_file);
  cout << "Outputting to " << outputFilename << endl;

  for (int i = 1; i <= 52; i++) {
    cout << "I " << i << endl;
    for (int j = i + 1; j <= 52; j++) {
      cout << "Generating distribution for " << i << " " << j << " card..."
           << endl;
      uint64_t playerCardsMask = (1ULL << i) | (1ULL << j);
      vector<float> dist;
      distGenerator.getHandDistribution(playerCardsMask, dist, 1000);
    }
  }
}

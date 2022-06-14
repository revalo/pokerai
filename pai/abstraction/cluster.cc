#include <algorithm>
#include <execution>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "../mccfr/rng.h"
#include "absl/container/flat_hash_map.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "hand_distribution.h"
#include "hand_indexer.h"

using namespace std;

ABSL_FLAG(string, input_file, "dists.dat", "Input file.");
ABSL_FLAG(string, output_file, "clusters.txt", "Output file.");
ABSL_FLAG(int, num_clusters, 8, "Number of clusters.");
ABSL_FLAG(int, restarts, 10, "Number of restarts.");

#define NUM_BUCKETS 50

float emd(const std::vector<float>& a, const std::vector<float>& b) {
  float sum = 0;
  float emd_i = 0;
  for (size_t i = 0; i < a.size(); i++) {
    emd_i += a[i] - b[i];
    sum += fabs(emd_i);
  }
  return sum;
}

float squared_l2(const std::vector<float>& a, const std::vector<float>& b) {
  float sum = 0;
  float d = 0;
  for (size_t i = 0; i < a.size(); i++) {
    d = (a[i] - b[i]);
    sum += d * d;
  }
  return sum;
}

float getVariance(
    const absl::flat_hash_map<int64_t, std::vector<float>>& distributionsMap,
    const std::vector<int>& clusterAssignments,
    const std::vector<std::vector<float>>& clusterCenters) {
  float variance = 0;
  for (size_t i = 0; i < distributionsMap.size(); i++) {
    int clusterId = clusterAssignments[i];
    const std::vector<float>& distribution = distributionsMap.at(i);
    const std::vector<float>& center = clusterCenters[clusterId];
    float d = emd(distribution, center);
    variance += d * d;
  }
  return variance / clusterCenters.size();
}

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  pokerai::RandomNumberGenerator rng;

  string inputFilename = absl::GetFlag(FLAGS_input_file);
  int numClusters = absl::GetFlag(FLAGS_num_clusters);
  cout << "Reading from " << inputFilename << endl;

  absl::flat_hash_map<int64_t, std::vector<float>> distributionsMap;

  // Read the file.
  ifstream inputFile(inputFilename, ios::in | ios::binary);
  if (!inputFile.is_open()) {
    cout << "Could not open file: " << inputFilename << endl;
    return 1;
  }

  int numRead = 0;

  while (true) {
    int64_t key;
    uint16_t buckets[NUM_BUCKETS];
    inputFile.read(reinterpret_cast<char*>(&key), sizeof(key));
    inputFile.read(reinterpret_cast<char*>(buckets), sizeof(buckets));
    std::vector<float> vecBuckets(buckets, buckets + NUM_BUCKETS);

    // Normalize.
    float sum = 0;
    for (size_t i = 0; i < NUM_BUCKETS; i++) {
      sum += buckets[i];
    }
    for (size_t i = 0; i < NUM_BUCKETS; i++) {
      vecBuckets[i] /= sum;
    }

    distributionsMap[key] = vecBuckets;

    numRead++;

    if (numRead % 100000 == 0) {
      cout << "Read " << numRead << " entries." << endl;
    }

    if (inputFile.eof()) {
      break;
    }
  }

  cout << "Read " << numRead - 1 << " entries." << endl;
  cout << "Map Size Is " << distributionsMap.size() << endl;
  cout << "Example EMD: " << emd(distributionsMap[0], distributionsMap[3])
       << endl;

  cout << "Building indexes..." << endl;
  std::vector<int64_t> indexes(distributionsMap.size());
  for (size_t i = 0; i < indexes.size(); i++) {
    indexes[i] = i;
  }

  int numRestarts = absl::GetFlag(FLAGS_restarts);
  float bestError = std::numeric_limits<float>::max();
  std::vector<int> bestAssignments;

  for (int restart = 0; restart < numRestarts; restart++) {
    cout << "Restart " << restart << endl;

    // K-Means++ Initialization.
    cout << "KMeans++ Initialization." << endl;
    std::vector<int64_t> centers(numClusters);

    {
      std::vector<bool> chosen(distributionsMap.size(), false);
      std::vector<float> currentBestDistances(
          distributionsMap.size(), std::numeric_limits<float>::infinity());

      // Step 1. Pick a random center.
      centers[0] = rng.next() % distributionsMap.size();
      chosen[centers[0]] = true;

      for (int currentCenterIndex = 0; currentCenterIndex < numClusters;
           currentCenterIndex++) {
        if (currentCenterIndex % 10 == 0) {
          cout << "Picking center " << currentCenterIndex << "("
               << currentCenterIndex / (float)numClusters * 100.0f << "%)"
               << endl;
        }
        std::vector<float>* currentCenter =
            &(distributionsMap[centers[currentCenterIndex]]);

        // Step 2. For each point, compute the distance to this newly added
        std::for_each(std::execution::par, indexes.begin(), indexes.end(),
                      [&](int64_t dataPoint) {
                        if (chosen[dataPoint]) {
                          return;
                        }
                        float d =
                            emd(*currentCenter, distributionsMap[dataPoint]);
                        d = d * d;

                        if (d < currentBestDistances[dataPoint]) {
                          currentBestDistances[dataPoint] = d;
                        }
                      });

        if (currentCenterIndex == numClusters - 1) {
          break;
        }

        // Step 3. Pick a random point based on the currentBestDistances.
        double currentBestDistancesSum = 0;
        for (int i = 0; i < distributionsMap.size(); i++) {
          if (chosen[i]) {
            continue;
          }
          currentBestDistancesSum += currentBestDistances[i];
        }

        int64_t newClusterCandidateIndex = -1;
        do {
          double randomPointer =
              double(rng.next()) * currentBestDistancesSum / double(UINT64_MAX);
          double current = 0.0f;
          for (size_t i = 0; i < distributionsMap.size(); i++) {
            if (chosen[i]) {
              continue;
            }
            current += currentBestDistances[i];
            if (randomPointer < current) {
              newClusterCandidateIndex = i;
              break;
            }
          }
        } while (newClusterCandidateIndex == -1 ||
                 chosen[newClusterCandidateIndex]);

        chosen[newClusterCandidateIndex] = true;
        centers[currentCenterIndex + 1] = newClusterCandidateIndex;
      }
    }

    // Run traditional K-Means.
    cout << "Running K-Means." << endl;
    std::vector<int> clusterAssignments(distributionsMap.size(), -1);
    std::vector<std::vector<float>> clusterCenters(numClusters);
    std::vector<int64_t> clusterSizes(numClusters, 1);
    clusterCenters.resize(numClusters, std::vector<float>(NUM_BUCKETS, 0));

    // Seed with the centers we already found.
    for (int i = 0; i < numClusters; i++) {
      clusterAssignments[centers[i]] = i;
      clusterCenters[i] = distributionsMap[centers[i]];
    }

    int numIterations = 0;
    double lastError = 1;

    while (true) {
      std::cout << "Iteration " << numIterations << endl;
      // Find the closest center for each point.
      std::for_each(
          std::execution::par, indexes.begin(), indexes.end(),
          [&](int64_t dataPoint) {
            float minDistance = std::numeric_limits<float>::infinity();
            int minDistanceIndex = -1;
            for (int i = 0; i < numClusters; i++) {
              float d = emd(clusterCenters[i], distributionsMap[dataPoint]);
              if (d < minDistance) {
                minDistance = d;
                minDistanceIndex = i;
              }
            }
            clusterAssignments[dataPoint] = minDistanceIndex;
          });

      // Update the centers, by computing the mean of each cluster.
      // Reset the cluster centers to 0.
      for (int i = 0; i < numClusters; i++) {
        clusterSizes[i] = 0;
        std::vector<float>& cluster = clusterCenters[i];
        for (int j = 0; j < NUM_BUCKETS; j++) {
          cluster[j] = 0;
        }
      }
      // Compute sums.
      for (int i = 0; i < distributionsMap.size(); i++) {
        int clusterIndex = clusterAssignments[i];
        clusterSizes[clusterIndex]++;
        std::vector<float>& cluster = clusterCenters[clusterIndex];
        std::vector<float>& point = distributionsMap[i];
        for (int j = 0; j < NUM_BUCKETS; j++) {
          cluster[j] += point[j];
        }
      }
      // Compute means.
      for (int i = 0; i < numClusters; i++) {
        std::vector<float>& cluster = clusterCenters[i];
        for (int j = 0; j < NUM_BUCKETS; j++) {
          cluster[j] /= clusterSizes[i];
        }
      }

      double error =
          getVariance(distributionsMap, clusterAssignments, clusterCenters);

      std::cout << "Variance: " << error << endl;

      if (numIterations != 0) {
        double improvement = (lastError - error) / lastError * 100.f;
        std::cout << "Improvement: " << improvement << "%" << endl;

        if (improvement < 0 || improvement < 0.1) {
          break;
        }
      }

      lastError = error;

      numIterations++;
      if (numIterations > 50) {
        break;
      }
    }

    if (lastError < bestError) {
      bestError = (float)lastError;
      bestAssignments = clusterAssignments;
    }
  }

  cout << "Best error: " << bestError << endl;

  // Write the results to a file.
  std::string outputFilename = absl::GetFlag(FLAGS_output_file);
  std::cout << "Writing results to " << outputFilename << std::endl;
  std::ofstream outFile(outputFilename);
  for (int i = 0; i < distributionsMap.size(); i++) {
    outFile << bestAssignments[i] << std::endl;
  }
  outFile.close();

  return 0;
}

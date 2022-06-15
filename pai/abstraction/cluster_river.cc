// A low memory version of cluster for OHCS river clustering.
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

ABSL_FLAG(string, input_file, "river_dist_small.dat", "Input file.");
ABSL_FLAG(string, output_file, "clusters.txt", "Output file.");
ABSL_FLAG(int, num_clusters, 8, "Number of clusters.");
ABSL_FLAG(int, restarts, 1, "Number of restarts.");

#define NUM_BUCKETS 8
#define NUM_CHUNKS 64

float squared_l2(const std::vector<uint16_t> &a,
                 const std::vector<uint16_t> &b) {
  float sum = 0;
  float d = 0;
  uint8_t a_num, b_num, a_den, b_den;
  for (size_t i = 0; i < a.size(); i++) {
    a_num = a[i] >> 8;
    a_den = a[i] & 0xFF;
    b_num = b[i] >> 8;
    b_den = b[i] & 0xFF;

    if (a_den == 0 || b_den == 0) {
      d = 0;
    } else {
      d = (a_num * b_den - a_den * b_num) / (float)(a_den * b_den);
    }
    sum += d * d;
  }
  return sum;
}

float squared_l2_float(const std::vector<double> &a,
                       const std::vector<uint16_t> &b) {
  float sum = 0;
  float d = 0;
  uint8_t b_num, b_den;
  for (size_t i = 0; i < a.size(); i++) {
    b_num = b[i] >> 8;
    b_den = b[i] & 0xFF;

    if (b_den == 0) {
      d = 0;
    } else {
      d = a[i] - (double)b_num / (double)b_den;
    }
    sum += d * d;
  }
  return sum;
}

float getVariance(const std::string &inputFilename,
                  const std::vector<short> &clusterAssignments,
                  const std::vector<std::vector<double>> &clusterCenters) {
  float variance = 0;
  std::ifstream inputFile(inputFilename, std::ios::binary);

  std::vector<uint16_t> thisDist(NUM_BUCKETS);
  int64_t handIndex;
  while (!inputFile.eof()) {
    inputFile.read(reinterpret_cast<char *>(&handIndex), sizeof(int64_t));
    inputFile.read(reinterpret_cast<char *>(thisDist.data()),
                   sizeof(uint16_t) * NUM_BUCKETS);

    int clusterId = clusterAssignments[handIndex];
    const std::vector<double> &center = clusterCenters[clusterId];
    float d = squared_l2_float(center, thisDist);
    variance += d * d;
  }

  inputFile.close();
  return variance / clusterCenters.size();
}

void getDistIndexedFromFile(const std::string &inputFilename, int64_t streamPos,
                            std::vector<uint16_t> &dist) {
  // std::ifstream inputFile(inputFilename, std::ios::binary);
  // while (!inputFile.eof()) {
  //   int64_t handIndex;
  //   inputFile.read(reinterpret_cast<char *>(&handIndex), sizeof(int64_t));
  //   inputFile.read(reinterpret_cast<char *>(dist.data()),
  //                  sizeof(uint16_t) * NUM_BUCKETS);
  //   if (handIndex == index) {
  //     return;
  //   }
  // }
  // inputFile.close();

  // cout << "Could not find hand " << index << endl;
  std::ifstream inputFile(inputFilename, std::ios::binary);
  inputFile.seekg(streamPos);
  while (!inputFile.eof()) {
    int64_t handIndex;
    inputFile.read(reinterpret_cast<char *>(&handIndex), sizeof(int64_t));
    inputFile.read(reinterpret_cast<char *>(dist.data()),
                   sizeof(uint16_t) * NUM_BUCKETS);
    break;
  }
  inputFile.close();

  // cout << "Could not find hand " << index << endl;
}

void getDistIndexedFromFileFloat(const std::string &inputFilename,
                                 int64_t streamPos, std::vector<double> &dist) {
  // std::ifstream inputFile(inputFilename, std::ios::binary);
  // std::vector<uint16_t> dist_uint16(NUM_BUCKETS);
  // while (!inputFile.eof()) {
  //   int64_t handIndex;
  //   inputFile.read(reinterpret_cast<char *>(&handIndex), sizeof(int64_t));
  //   inputFile.read(reinterpret_cast<char *>(dist_uint16.data()),
  //                  sizeof(uint16_t) * NUM_BUCKETS);
  //   if (handIndex == index) {
  //     uint8_t num, den;
  //     for (size_t i = 0; i < NUM_BUCKETS; i++) {
  //       num = dist_uint16[i] >> 8;
  //       den = dist_uint16[i] & 0xFF;
  //       dist[i] = (double)num / (double)den;
  //     }
  //     return;
  //   }
  // }
  // inputFile.close();

  // cout << "Could not find hand " << index << endl;

  std::ifstream inputFile(inputFilename, std::ios::binary);
  inputFile.seekg(streamPos);
  std::vector<uint16_t> dist_uint16(NUM_BUCKETS);
  while (!inputFile.eof()) {
    int64_t handIndex;
    inputFile.read(reinterpret_cast<char *>(&handIndex), sizeof(int64_t));
    inputFile.read(reinterpret_cast<char *>(dist_uint16.data()),
                   sizeof(uint16_t) * NUM_BUCKETS);

    uint8_t num, den;
    for (size_t i = 0; i < NUM_BUCKETS; i++) {
      num = dist_uint16[i] >> 8;
      den = dist_uint16[i] & 0xFF;
      dist[i] = (double)num / (double)den;
    }
    break;
  }
  inputFile.close();
}

int main(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);

  pokerai::RandomNumberGenerator rng;
  pokerai::HandIndexer handIndexer(std::vector<int>{2, 3, 1, 1});

  size_t numHands = handIndexer.roundSize[3];
  // size_t numHands = 100000;

  // Build chunked index for parallel chunked file reading.
  std::vector<int> chunkIndexes(NUM_CHUNKS);
  for (int i = 0; i < NUM_CHUNKS; i++) {
    chunkIndexes[i] = i;
  }
  size_t chunkSize = numHands / NUM_CHUNKS;

  cout << "River has " << numHands << " hands." << endl;

  string inputFilename = absl::GetFlag(FLAGS_input_file);
  int numClusters = absl::GetFlag(FLAGS_num_clusters);
  cout << "Reading from " << inputFilename << endl;

  // Read the file.
  ifstream inputFile(inputFilename, ios::in | ios::binary);
  if (!inputFile.is_open()) {
    cout << "Could not open file: " << inputFilename << endl;
    return 1;
  }

  cout << "Building file index ..." << endl;

  // 19 GB.
  std::vector<int64_t> handStreamPos(numHands, 0);

  cout << "Reading file ..." << endl;
  std::for_each(
      std::execution::par, chunkIndexes.begin(), chunkIndexes.end(),
      [&](int chunkIndex) {
        bool last = chunkIndex == NUM_CHUNKS - 1;
        size_t start = chunkIndex * chunkSize;
        size_t end = last ? numHands : (chunkIndex + 1) * chunkSize;
        ifstream inputFile(inputFilename, ios::in | ios::binary);
        size_t currentStreamPos =
            start * (sizeof(int64_t) + sizeof(uint16_t) * NUM_BUCKETS);
        inputFile.seekg(currentStreamPos);
        std::vector<uint16_t> thisDist(NUM_BUCKETS);
        int64_t handIndex;
        for (size_t i = start; i < end; i++) {
          inputFile.read(reinterpret_cast<char *>(&handIndex), sizeof(int64_t));
          inputFile.read(reinterpret_cast<char *>(thisDist.data()),
                         sizeof(uint16_t) * NUM_BUCKETS);

          handStreamPos[handIndex] = currentStreamPos;
          currentStreamPos += sizeof(int64_t) + sizeof(uint16_t) * NUM_BUCKETS;
        }
        inputFile.close();
      });

  cout << "Done building file index." << endl;

  int numRead = 0;

  int numRestarts = absl::GetFlag(FLAGS_restarts);
  float bestError = std::numeric_limits<float>::max();
  std::vector<short> bestAssignments;

  for (int restart = 0; restart < numRestarts; restart++) {
    cout << "Restart " << restart << endl;

    // K-Means++ Initialization.
    cout << "KMeans++ Initialization." << endl;
    std::vector<int64_t> centers(numClusters);

    {
      // 2.4 GB
      std::vector<bool> chosen(numHands, false);
      // 9.7 GB
      std::vector<float> currentBestDistances(
          numHands, std::numeric_limits<float>::infinity());

      // Step 1. Pick a random center.
      centers[0] = rng.next() % numHands;
      chosen[centers[0]] = true;

      for (int currentCenterIndex = 0; currentCenterIndex < numClusters;
           currentCenterIndex++) {
        // if (currentCenterIndex % 10 == 0) {
        cout << "Picking center " << currentCenterIndex << "("
             << currentCenterIndex / (float)numClusters * 100.0f << "%)"
             << endl;
        // }
        std::vector<uint16_t> currentCenter(NUM_BUCKETS);
        getDistIndexedFromFile(inputFilename,
                               handStreamPos[centers[currentCenterIndex]],
                               currentCenter);

        cout << "Starting parallel" << endl;

        std::for_each(
            std::execution::par, chunkIndexes.begin(), chunkIndexes.end(),
            [&](int chunkIndex) {
              bool last = chunkIndex == NUM_CHUNKS - 1;
              size_t start = chunkIndex * chunkSize;
              size_t end = last ? numHands : (chunkIndex + 1) * chunkSize;
              ifstream inputFile(inputFilename, ios::in | ios::binary);
              inputFile.seekg(
                  start * (sizeof(int64_t) + sizeof(uint16_t) * NUM_BUCKETS));
              std::vector<uint16_t> thisDist(NUM_BUCKETS);
              int64_t handIndex;
              for (size_t i = start; i < end; i++) {
                inputFile.read(reinterpret_cast<char *>(&handIndex),
                               sizeof(int64_t));
                inputFile.read(reinterpret_cast<char *>(thisDist.data()),
                               sizeof(uint16_t) * NUM_BUCKETS);

                if (handIndex < 0 || handIndex >= numHands) {
                  cout << "Bad hand index: " << handIndex << " by "
                       << chunkIndex << endl;
                  exit(1);
                }

                if (chosen[handIndex]) {
                  continue;
                }

                float d = squared_l2(currentCenter, thisDist);
                if (d < currentBestDistances[handIndex]) {
                  currentBestDistances[handIndex] = d;
                }
              }
              inputFile.close();
            });

        if (currentCenterIndex == numClusters - 1) {
          break;
        }

        cout << "Done parallel" << endl;

        // Step 3. Pick a random point based on the currentBestDistances.
        double currentBestDistancesSum = 0;
        for (int i = 0; i < numHands; i++) {
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
          for (size_t i = 0; i < numHands; i++) {
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
    std::vector<short> clusterAssignments(numHands, -1);
    std::vector<std::vector<double>> clusterCenters(
        numClusters, std::vector<double>(NUM_BUCKETS, 0));
    std::vector<int64_t> clusterSizes(numClusters, 1);

    // Seed with the centers we already found.
    for (int i = 0; i < numClusters; i++) {
      clusterAssignments[centers[i]] = i;
      getDistIndexedFromFileFloat(inputFilename, handStreamPos[centers[i]],
                                  clusterCenters[i]);
    }

    int numIterations = 0;
    double lastError = 1;

    while (true) {
      std::cout << "Iteration " << numIterations << endl;
      // Find the closest center for each point.

      std::for_each(
          std::execution::par, chunkIndexes.begin(), chunkIndexes.end(),
          [&](int chunkIndex) {
            bool last = chunkIndex == NUM_CHUNKS - 1;
            size_t start = chunkIndex * chunkSize;
            size_t end = last ? numHands : (chunkIndex + 1) * chunkSize;
            ifstream inputFile(inputFilename, ios::in | ios::binary);
            inputFile.seekg(start *
                            (sizeof(int64_t) + sizeof(uint16_t) * NUM_BUCKETS));
            std::vector<uint16_t> thisDist(NUM_BUCKETS);
            int64_t handIndex;
            for (size_t i = start; i < end; i++) {
              inputFile.read(reinterpret_cast<char *>(&handIndex),
                             sizeof(int64_t));
              inputFile.read(reinterpret_cast<char *>(thisDist.data()),
                             sizeof(uint16_t) * NUM_BUCKETS);

              float minDistance = std::numeric_limits<float>::infinity();
              int minDistanceIndex = -1;
              for (int j = 0; j < numClusters; j++) {
                float d = squared_l2_float(clusterCenters[j], thisDist);
                if (d < minDistance) {
                  minDistance = d;
                  minDistanceIndex = j;
                }
              }
              clusterAssignments[handIndex] = minDistanceIndex;
            }
            inputFile.close();
          });

      // Update the centers, by computing the mean of each cluster.
      // Reset the cluster centers to 0.
      for (int i = 0; i < numClusters; i++) {
        clusterSizes[i] = 0;
        std::vector<double> &cluster = clusterCenters[i];
        for (int j = 0; j < NUM_BUCKETS; j++) {
          cluster[j] = 0;
        }
      }

      ifstream inputFile(inputFilename, ios::in | ios::binary);
      std::vector<uint16_t> thisDist(NUM_BUCKETS);
      int64_t handIndex;
      while (!inputFile.eof()) {
        inputFile.read(reinterpret_cast<char *>(&handIndex), sizeof(int64_t));
        inputFile.read(reinterpret_cast<char *>(thisDist.data()),
                       sizeof(uint16_t) * NUM_BUCKETS);

        int clusterIndex = clusterAssignments[handIndex];
        clusterSizes[clusterIndex]++;
        std::vector<double> &cluster = clusterCenters[clusterIndex];
        for (int j = 0; j < NUM_BUCKETS; j++) {
          cluster[j] += thisDist[j];
        }
      }
      inputFile.close();

      // Compute means.
      for (int i = 0; i < numClusters; i++) {
        std::vector<double> &cluster = clusterCenters[i];
        for (int j = 0; j < NUM_BUCKETS; j++) {
          cluster[j] /= clusterSizes[i];
        }
      }

      double error =
          getVariance(inputFilename, clusterAssignments, clusterCenters);

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
  for (int i = 0; i < bestAssignments.size(); i++) {
    outFile << bestAssignments[i] << std::endl;
  }
  outFile.close();

  return 0;
}

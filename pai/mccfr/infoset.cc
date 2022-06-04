#include "infoset.h"

#define MAX_FLOAT(x, y) ((x) > (y) ? (x) : (y))
#define EPSILON 0.0001f

pokerai::InfoSet::InfoSet(int numActions) {
  this->numActions = numActions;
  this->regretSums = new float[numActions];
  this->strategySums = new float[numActions];
  this->averageStrategy = new float[numActions];
  this->strategy = new float[numActions];

  this->uniformStrategyProb = 1.0f / numActions;

  this->reset();
}

pokerai::InfoSet::~InfoSet() {
  delete[] regretSums;
  delete[] strategySums;
  delete[] averageStrategy;
  delete[] strategy;
}

void pokerai::InfoSet::reset() {
  for (int i = 0; i < numActions; i++) {
    regretSums[i] = 0.0f;
    strategySums[i] = 0.0f;
  }
}

float *pokerai::InfoSet::getStrategy() {
  float clippedDenom = 0.0f;

  // Use strategy as temporary storage for clipped regrets.
  for (int i = 0; i < numActions; i++) {
    strategy[i] = MAX_FLOAT(regretSums[i], 0.0f);
    clippedDenom += strategy[i];
  }

  // If the denominator is 0, then all regrets are 0, so the strategy is
  // uniform.
  if (clippedDenom < EPSILON) {
    for (int i = 0; i < numActions; i++) {
      strategy[i] = uniformStrategyProb;
    }
  } else {
    for (int i = 0; i < numActions; i++) {
      strategy[i] /= clippedDenom;
    }
  }

  return strategy;
}

float *pokerai::InfoSet::getAverageStrategy() {
  float normalizingSum = 0.0f;

  for (int i = 0; i < numActions; i++) {
    normalizingSum += strategySums[i];
  }

  if (normalizingSum < EPSILON) {
    for (int i = 0; i < numActions; i++) {
      averageStrategy[i] = uniformStrategyProb;
    }
  } else {
    for (int i = 0; i < numActions; i++) {
      averageStrategy[i] = strategySums[i] / normalizingSum;
    }
  }

  return averageStrategy;
}
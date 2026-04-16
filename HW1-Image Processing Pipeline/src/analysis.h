#pragma once
#include "image.h"

// Mean Square Error: sum over all pixels and channels of (orig - out)^2
double computeMSE(const Image& original, const Image& output);

// Mean Absolute Error: sum over all pixels and channels of |orig - out|
double computeMAE(const Image& original, const Image& output);

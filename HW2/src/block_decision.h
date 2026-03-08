#ifndef BLOCK_DECISION_H
#define BLOCK_DECISION_H

#include <vector>

struct Block {
    // starting point of the block with block size
    int x0;
    int y0;
    int N;
};

// Compute the luminance buffer from the RGB data
std::vector<double> computeLumBuffer(const unsigned char* rgbData, int width, int height);

// Compute the block map for the image based on the luminance buffer and the allowed block sizes.
std::vector<Block> computeBlockMap(const std::vector<double>& lum, int width, int height,
                                      double threshold = 500.0);

#endif
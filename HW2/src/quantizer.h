#ifndef QUANTIZER_H
#define QUANTIZER_H

#include <vector>

std::vector<std::vector<int>> quantizeBlock(const std::vector<std::vector<double>> &dct, int Q);
std::vector<std::vector<double>> dequantizBlock(const std::vector<std::vector<int>>& qdct, int Q);

#endif
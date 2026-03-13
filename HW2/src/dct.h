#ifndef DCT_H
#define DCT_H

#include <vector>

std::vector<std::vector<double>> dct2d(const std::vector<std::vector<double>> & block);
std::vector<std::vector<double>> idct2d(const std::vector<std::vector<double>>& coeffs);
#endif
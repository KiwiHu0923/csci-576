#include <cmath>
#include <vector>

static std::vector<std::vector<int>> quantizeBlock(const std::vector<std::vector<double>> &dct, int Q) {
    int N = static_cast<int>(dct.size());
    std::vector<std::vector<int>> out(N, std::vector<int>(N, 0));
    double denom = std::pow(2.0, Q);
    for(int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j) 
            out[i][j] = static_cast<int>(std::round(dct[i][j] / denom));
    return out;
}

static std::vector<std::vector<double>> dequantizBlock(const std::vector<std::vector<int>>& qdct, int Q) {
    int N = static_cast<int>(qdct.size());
    std::vector<std::vector<double>> out(N, std::vector<double>(N, 0.0));
    double mult = std::pow(2.0, Q);
    for(int i = 0; i < N; ++i) 
        for (int j = 0; j < N; ++j) 
            out[i][j] = static_cast<double>(qdct[i][j]) * mult;
    return out;
}
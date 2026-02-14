#include "analysis.h"

double computeMSE(const Image& original, const Image& output) {
    double sum = 0.0;
    int n = original.width * original.height * 3;
    for (int i = 0; i < n; i++) {
        double diff = (double)original.data[i] - (double)output.data[i];
        sum += diff * diff;
    }
    return sum;
}

double computeMAE(const Image& original, const Image& output) {
    double sum = 0.0;
    int n = original.width * original.height * 3;
    for (int i = 0; i < n; i++) {
        double diff = (double)original.data[i] - (double)output.data[i];
        sum += (diff < 0 ? -diff : diff);
    }
    return sum;
}

#include "dct.h"

#include <vector>
#include <cmath>

static std::vector<double> dct1d(const std::vector<double>& v) {
    int N = v.size();
    std::vector<double> V(N, 0.0);
    double factor = M_PI / (2.0 * N);
    for (int k = 0; k < N; ++k) {
        double sum = 0.0;
        for (int n = 0; n < N; ++n) {
            sum += v[n] * cos((2.0 * n + 1) * k * factor);
        }
        double ck = (k == 0) ? sqrt(1.0 / N) : sqrt(2.0 / N);
        V[k] = ck * sum;
    }
    return V;
}

static std::vector<double> idct1d(const std::vector<double> & V) {
    int N = static_cast<int>(V.size());
    std::vector<double> v(N, 0.0);
    double factor = M_PI / (2.0 * N);
    for (int n = 0; n < N; ++n) {
        double sum = 0.0;
        for (int k = 0; k < N; ++k) {
            double ck = (k == 0) ? sqrt(1.0 / N) : sqrt(2.0 / N);
            sum += ck * V[k] * cos((2.0 * n + 1) * k * factor);
        }
        v[n] = sum;
    }
    return v;
}

// Compute the 2D DCT of an NxN block by applying 1D DCT to rows and then to columns.
std::vector<std::vector<double>> dct2d(const std::vector<std::vector<double>> & block) {
    int N = static_cast<int>(block.size());
    std::vector<std::vector<double>> tmp(N, std::vector<double>(N, 0.0));
    std::vector<std::vector<double>> out(N, std::vector<double>(N, 0.0));

    for (int r = 0; r < N; ++r) {
        auto rowCoeffs = dct1d(block[r]);
        for (int c = 0; c < N; ++c) {
            tmp[r][c] = rowCoeffs[c];
        }
    }

    for (int c = 0; c < N; ++c) {
        std::vector<double> col(N);
        for (int r = 0; r < N; ++r) {
            col[r] = tmp[r][c];
        }
        auto colCoeffs = dct1d(col);
        for (int r = 0; r < N; ++r) {
            out[r][c] = colCoeffs[r];
        }
    }
    return out;
}

std::vector<std::vector<double>> idct2d(const std::vector<std::vector<double>>& coeffs) {
    int N = static_cast<int>(coeffs.size());
    std::vector<std::vector<double>> tmp(N, std::vector<double>(N, 0.0));
    std::vector<std::vector<double>> out(N, std::vector<double>(N, 0.0));

    for (int c = 0; c < N; ++c) {
        std::vector<double> col(N);
        for (int r = 0; r < N; ++r) {
            col[r] = coeffs[r][c];
        }
        auto colVals = idct1d(col);
        for (int r = 0; r < N; ++r) {
            tmp[r][c] = colVals[r];
        }
    }

    for (int r = 0; r < N; ++r) {
        auto rowVals = idct1d(tmp[r]);
        for (int c = 0; c < N; ++c) {
            out[r][c] = rowVals[c];
        }
    }
    return out;
}
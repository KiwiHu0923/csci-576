#include <iostream>
#include <vector>
#include <string>

#include "block_decision.h"
#include "dct.h"

static const int MAX_BS = 32;
static const int MIN_BS = 2;

static inline int clampInt(int v, int minVal, int maxVal) {
    if (v < minVal) return minVal;
    else if (v > maxVal) return maxVal;
    else return v;
}

static inline double TforSize(double base, int bs) {
    switch(bs) {
        case 32: return base * 0.25;
        case 16: return base * 0.5;
        case 8: return base;
        case 4: return base * 2.0;
        case 2: return base * 4.0;
        default: return base;
    }
}

std::vector<double> computeLumBuffer(const unsigned char* rgbData, int width, int height) {
    std::vector<double> lum(size_t (width * height), 0.0);
    if (!rgbData) return lum;

    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            size_t idx = (size_t) (r * width +c);
            int base = (int)idx * 3;
            unsigned char R = rgbData[base];
            unsigned char G = rgbData[base + 1];
            unsigned char B = rgbData[base + 2];
            lum[idx] = 0.299 * (double)R + 0.587 * (double)G + 0.114 * (double)B;
        }
    }
    return lum;
}

static inline double lumClamped(const std::vector<double>& lum, int width, int height, int r, int c) {
    int rr = clampInt(r, 0, height - 1);
    int cc = clampInt(c, 0, width - 1);
    return lum[(size_t)rr * width + cc];
}

static std::vector<std::vector<double>> buildBlock2d(const std::vector<double>& lum, int width, int height, int x0, int y0, int N) {
    std::vector<std::vector<double>> block;
    block.assign(N, std::vector<double>(N, 0.0));

    for (int yy = 0; yy < N; ++yy) {
        for (int xx = 0; xx < N; ++xx) {
            double v = lumClamped(lum, width, height, y0 + yy, x0 + xx);
            block[yy][xx] = v - 128.0;   // level shift by subtracting 128
        }
    }
    return block;
}

static double blockDCTHighFreqRatio(const std::vector<double>& lum, int width, int height, int x0, int y0, int N) {
    auto block2d = buildBlock2d(lum, width, height, x0, y0, N);
    std::vector<std::vector<double>> coeffs = dct2d(block2d);

    int H = (int)coeffs.size();
    if (H <= 0) return 0.0;
    int W = (int)coeffs[0].size();
    if (W <= 0) return 0.0;

    double E_total = 0.0;
    for (int r = 0; r < H; ++r) {
        for (int c = 0; c < W; ++c) {
            double v = coeffs[r][c];
            E_total += v * v;
        }
    }

    if (E_total <= 0.0) return 0.0;

    double E_dc = coeffs[0][0] * coeffs[0][0];
    double E_high = E_total - E_dc;

    if (E_high <= 0.0) return 0.0;

    return E_high / E_total;
}

static void split(const std::vector<double>& lum,
                 int width,
                int height,
                int x0, int y0,
                double baseThreshold,
                std::vector<Block>& outBlocks,
                int bs) {
    
    if (bs <= MIN_BS) {
        outBlocks.push_back({x0, y0, bs});
        return;
    }

    double ratio = blockDCTHighFreqRatio(lum, width, height, x0, y0, bs);
    double threshold = TforSize(baseThreshold, bs);
    
    if (ratio > threshold) {
        int subBs = bs / 2;

        split(lum, width, height, x0, y0, baseThreshold, outBlocks, subBs);
        split(lum, width, height, x0 + subBs, y0, baseThreshold, outBlocks, subBs);
        split(lum, width, height, x0, y0 + subBs, baseThreshold, outBlocks, subBs);
        split(lum, width, height, x0 + subBs, y0 + subBs, baseThreshold, outBlocks, subBs);
    }
    else {
        outBlocks.push_back({x0, y0, bs});
    }
}

std::vector<Block> computeBlockMap(const std::vector<double>& lum,
                                    int width,
                                    int height,
                                    double baseT) {
    std::vector<Block> blocks;
    
    for (int y0 = 0; y0 < height; y0 += MAX_BS) {
        for (int x0 = 0; x0 < width; x0 += MAX_BS) {
            split(lum, width, height, x0, y0, baseT, blocks, MAX_BS);
        }
    }

    std::sort(blocks.begin(), blocks.end(), [](const Block& a, const Block& b) {
        if (a.y0 != b.y0) return a.y0 < b.y0;
        else if (a.x0 != b.x0) return a.x0 < b.x0;
        else return a.N < b.N;
    });

    return blocks;
    
}
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cassert>
#include <cstdlib>
#include <sys/stat.h>

#include "controller.h"
#include "dct.h"
#include "quantizer.h"
#include "block_decision.h"

// Global variable to store block metadata for visualization
std::vector<int> g_blockMetadata;

// A simple pass-through function to read image data from a file.
static unsigned char *readImageData_simple(const std::string &imagePath) {
    std::ifstream inputFile(imagePath, std::ios::binary);
    if (!inputFile.is_open()) {
        std::cerr << "Error opening file: " << imagePath << std::endl;
        return nullptr;
    }

    std::vector<char> Rbuf(IMAGE_WIDTH * IMAGE_HEIGHT);
    std::vector<char> Gbuf(IMAGE_WIDTH * IMAGE_HEIGHT);
    std::vector<char> Bbuf(IMAGE_WIDTH * IMAGE_HEIGHT);

    inputFile.read(Rbuf.data(), IMAGE_WIDTH * IMAGE_HEIGHT);
    inputFile.read(Gbuf.data(), IMAGE_WIDTH * IMAGE_HEIGHT);
    inputFile.read(Bbuf.data(), IMAGE_WIDTH * IMAGE_HEIGHT);
    inputFile.close();

    unsigned char *inData =
        (unsigned char *)malloc(IMAGE_WIDTH * IMAGE_HEIGHT * 3 * sizeof(unsigned char));

    if (!inData) return nullptr;

    for (int i = 0; i < IMAGE_WIDTH * IMAGE_HEIGHT; i++) {
        inData[3 * i] = static_cast<unsigned char>(Rbuf[i]);
        inData[3 * i + 1] = static_cast<unsigned char>(Gbuf[i]);
        inData[3 * i + 2] = static_cast<unsigned char>(Bbuf[i]);
    }
    return inData;

}

static double calCompressedBpp(const std::string& filePath) {
    std::string gzipCmd = "gzip -c \"" + filePath + "\" > \"" + filePath + ".gz\"";
    int result = system(gzipCmd.c_str());

    if (result != 0) {
        std::cerr << "Error compressing file with gzip: " << filePath << std::endl;
        return -1.0;
    }

    //Get the size of the compressed file
    struct stat st;
    std::string gzipFile = filePath + ".gz";
    if (stat(gzipFile.c_str(), &st) != 0) {
        std::cerr << "Error getting file size: " << gzipFile << std::endl;
        return -1.0;
    }

    size_t compressedBytes = st.st_size;
    double bitsPerPixel = (compressedBytes * 8.0) / (IMAGE_WIDTH * IMAGE_HEIGHT);

    // Clean up the temporary gzip file
    std::remove(gzipFile.c_str());

    return bitsPerPixel;
}

static void writeDCTfile(const std::string &basename, int blockSize, const std::vector<int> &blockMeta, const std::vector<int32_t> &flattenedCoeffs) {
    std::ofstream out(basename, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Error opening file for writing: " << basename << std::endl;
        return;
    }

    uint32_t w = static_cast<uint32_t>(IMAGE_WIDTH);
    uint32_t h = static_cast<uint32_t>(IMAGE_HEIGHT);
    uint8_t ch = 3;
    uint32_t bs = static_cast<uint32_t>(blockSize);

    if (blockMeta.size() % 3 != 0) {
        std::cerr << "Error: blockMeta size should be a multiple of 3 (x0, y0, N for each block)." << std::endl;
        return;
    }
    uint32_t numBlocks = static_cast<uint32_t>(blockMeta.size() / 3); // each block has 3 metadata integers (x0, y0, N)

    out.write(reinterpret_cast<const char*>(&w), sizeof(w));
    out.write(reinterpret_cast<const char*>(&h), sizeof(h));
    out.write(reinterpret_cast<const char*>(&ch), sizeof(ch));
    out.write(reinterpret_cast<const char*>(&bs), sizeof(bs));
    out.write(reinterpret_cast<const char*>(&numBlocks), sizeof(numBlocks));

    // blockMeta is a list of (x0, y0, N) for each block
    for (int v: blockMeta) {
        int32_t val = static_cast<int32_t>(v);
        out.write(reinterpret_cast<const char*>(&val), sizeof(val));
    }

    // flattenedCoeffs is a list of quantized DCT coefficients for all blocks and channels
    for (int32_t coeff: flattenedCoeffs) {
        out.write(reinterpret_cast<const char*>(&coeff), sizeof(coeff));
    }
    out.close();
}

static void writeDCTfile_adaptive(const std::string &basename, const std::vector<int> &blockMeta, const std::vector<int32_t> &flattenedCoeffs) {
    std::ofstream out(basename, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Error opening file for writing: " << basename << std::endl;
        return;
    }

    uint32_t w = static_cast<uint32_t>(IMAGE_WIDTH);
    uint32_t h = static_cast<uint32_t>(IMAGE_HEIGHT);
    uint8_t ch = 3;

    uint32_t bs = 0; // block size is not fixed in adaptive pipeline, keep it as 0 to indicate variable block size
    if (blockMeta.size() % 3 != 0) {
        std::cerr << "Error: blockMeta size should be a multiple of 3 (x0, y0, N for each block)." << std::endl;
        return;
    }
    uint32_t numBlocks = static_cast<uint32_t>(blockMeta.size() / 3); // each block has 3 metadata integers (x0, y0, N)

    out.write(reinterpret_cast<const char*>(&w), sizeof(w));
    out.write(reinterpret_cast<const char*>(&h), sizeof(h));
    out.write(reinterpret_cast<const char*>(&ch), sizeof(ch));
    out.write(reinterpret_cast<const char*>(&bs), sizeof(bs)); // block size is not fixed
    out.write(reinterpret_cast<const char*>(&numBlocks), sizeof(numBlocks));

    for (int v : blockMeta) {
        int32_t val = static_cast<int32_t>(v);
        out.write(reinterpret_cast<const char*>(&val), sizeof(val));
    }

    for (int32_t coeff : flattenedCoeffs) {
        out.write(reinterpret_cast<const char*>(&coeff), sizeof(coeff));
    }

    out.close();
}

// Calculate the DCT coefficients for each block in the image using a fixed block size of 8x8.
// only call this function for pre-procssing of DCT coefficients when B > 0 and Q == -1, so that we can reuse the DCT coefficients for different Q values when searching for the optimal Q that meets the target BPP.
static ImageDCTData calDCT_fixed(const std::string &imagePath) {
    ImageDCTData dctData;

    int N = 8;

    unsigned char *inData = readImageData_simple(imagePath);
    if (!inData) {
        std::cerr << "Failed to read image data from: " << imagePath << std::endl;
        return dctData;
    }

    assert(IMAGE_WIDTH % N == 0 && IMAGE_HEIGHT % N == 0);

    // Calculate number of blocks in x and y directions
    const int blocksX = IMAGE_WIDTH / N;
    const int blocksY = IMAGE_HEIGHT / N;

    for (int by = 0; by < blocksY; ++by) {
        for (int bx = 0; bx < blocksX; ++bx) {
            // Starting coordinates of the block in the image
            int x0 = bx * N;
            int y0 = by * N;

            // Store block metadata
            dctData.blockMeta.push_back(x0);
            dctData.blockMeta.push_back(y0);
            dctData.blockMeta.push_back(N);

            // Store DCT coefficients for all 3 channels
            // Each blockChannels[ch] is a 2D vector of DCT coefficients for that channel
            std::vector<std::vector<std::vector<double>>> blockChannels(3);

            for (int ch = 0; ch < 3; ++ch) {
                // Extract block from image (each block is size of 8x8)
                std::vector<std::vector<double>> block(N, std::vector<double>(N, 0.0));
                for (int yy = 0; yy < N; ++yy) {
                    const int py = y0 + yy;
                    for (int xx = 0; xx < N; ++xx) {
                        const int px = x0 + xx;
                        int index = (py * IMAGE_WIDTH + px) * 3 + ch;
                        // Level shift the pixel value by 128 to center around 0 for DCT
                        block[yy][xx] = static_cast<double>(inData[index]) - 128.0;
                    }
                }

                // Compute DCT and store
                blockChannels[ch] = dct2d(block);
            }
            // Store the DCT coefficients for this block (all 3 channels)
            dctData.dctCoeffs.push_back(blockChannels);
        }
    }

    free(inData);
    return dctData;
}

// Calculate the DCT coefficients for each block in the image using a adaptive block size of NxN.
// only call this function for pre-procssing of DCT coefficients when B > 0 and Q == -1, so that we can reuse the DCT coefficients for different Q values when searching for the optimal Q that meets the target BPP.
static ImageDCTData calDCT_adaptive(const std::string &imagePath) {
    ImageDCTData dctData;

    unsigned char *inData = readImageData_simple(imagePath);
    if (!inData) {
        std::cerr << "Failed to read image data from: " << imagePath << std::endl;
        return dctData;
    }

    // Compute luminance buffer and block map
    std::vector<double> lum = computeLumBuffer(inData, IMAGE_WIDTH, IMAGE_HEIGHT);
    std::vector<Block> blocks = computeBlockMap(lum, IMAGE_WIDTH, IMAGE_HEIGHT, 0.15);

    for (const Block &blk : blocks) {
        int x0 = blk.x0;
        int y0 = blk.y0;
        // Note: N can vary for each block in the adaptive pipeline, so we need to read it from the block metadata
        int N = blk.N;

        // Store block metadata
        dctData.blockMeta.push_back(x0);
        dctData.blockMeta.push_back(y0);
        dctData.blockMeta.push_back(N);

        // Store DCT coefficients for all 3 channels
        std::vector<std::vector<std::vector<double>>> blockChannels(3);

        for (int ch = 0; ch < 3; ++ch) {
            // Extract block from image
            std::vector<std::vector<double>> block(N, std::vector<double>(N, 0.0));
            for (int yy = 0; yy < N; ++yy) {
                int py = y0 + yy;
                for (int xx = 0; xx < N; ++xx) {
                    int px = x0 + xx;

                    // Clamp coordinates to image bounds
                    int tmpY = py;
                    int tmpX = px;
                    if (tmpY >= IMAGE_HEIGHT) tmpY = IMAGE_HEIGHT - 1;
                    if (tmpX >= IMAGE_WIDTH) tmpX = IMAGE_WIDTH - 1;
                    if (tmpY < 0) tmpY = 0;
                    if (tmpX < 0) tmpX = 0;

                    int index = (tmpY * IMAGE_WIDTH + tmpX) * 3 + ch;
                    block[yy][xx] = static_cast<double>(inData[index]) - 128.0;
                }
            }

            // Compute DCT and store
            blockChannels[ch] = dct2d(block);
        }

        dctData.dctCoeffs.push_back(blockChannels);
    }

    free(inData);
    return dctData;
}

static void quantizeWriteDCT_fixed(const ImageDCTData &dctData, int Q, const std::string &basename) {
    int N = 8;
    std::vector<int32_t> flattenedCoeffs;

    size_t numBlocks = dctData.dctCoeffs.size();
    for (size_t blkIdx = 0; blkIdx < numBlocks; ++blkIdx) {
        const auto &blockChannels = dctData.dctCoeffs[blkIdx];
        for (int ch = 0; ch < 3; ++ch) {
            const auto& coeffs = dctData.dctCoeffs[blkIdx][ch];

            auto qcoeffs = quantizeBlock(coeffs, Q);

            for (int i = 0; i < N; ++i) {
                for (int j = 0; j < N; ++j) {
                    flattenedCoeffs.push_back(static_cast<int32_t>(qcoeffs[i][j]));
                }
            }
        }
    }

    writeDCTfile(basename, N, dctData.blockMeta, flattenedCoeffs);
}

static void quantizeWriteDCT_adaptive(const ImageDCTData &dctData, int Q, const std::string &basename) {
    std::vector<int32_t> flattenedCoeffs;

    size_t numBlocks = dctData.dctCoeffs.size();
    for (size_t blkIdx = 0; blkIdx < numBlocks; ++blkIdx) {
        int N = dctData.blockMeta[blkIdx * 3 + 2];  // N is stored in blockMeta at index blkIdx*3 + 2

        for (int ch = 0; ch < 3; ++ch) {
            const auto& coeffs = dctData.dctCoeffs[blkIdx][ch];

            auto qcoeffs = quantizeBlock(coeffs, Q);

            for (int i = 0; i < N; ++i) {
                for (int j = 0; j < N; ++j) {
                    flattenedCoeffs.push_back(static_cast<int32_t>(qcoeffs[i][j]));
                }
            }
        }
    }

    // Write to DCT file (use adaptive version)
    writeDCTfile_adaptive(basename, dctData.blockMeta, flattenedCoeffs);
}

// use binary search to find the best Q that meets the target BPP
static int findQ_fixed(const ImageDCTData &dctData, double targetBpp, const std::string &imagePath) {
    int Q_min = 0;
    int Q_max = 20;
    int bestQ = 0;
    double bestBpp = 1e9;

    while (Q_min <= Q_max) {
        int Q_mid = (Q_min + Q_max) / 2;

        std::string tempDctPath = "dct/" + imagePath + "_temp_Q" + std::to_string(Q_mid) + ".DCT";

        quantizeWriteDCT_fixed(dctData, Q_mid, tempDctPath);
        double actBpp = calCompressedBpp(tempDctPath);

        if (actBpp < 0) {
            std::cerr << "Error calculating compressed bpp for Q=" << Q_mid << std::endl;
            break;
        }

        if (std::abs(actBpp - targetBpp) < std::abs(bestBpp - targetBpp)) {
            bestBpp = actBpp;
            bestQ = Q_mid;
        }

        if (actBpp > targetBpp) {
            Q_min = Q_mid + 1;
        } else {
            Q_max = Q_mid - 1;
        }

        std::remove(tempDctPath.c_str());

        if (std::abs(actBpp - targetBpp) < 0.05) {
            break;
        }
    }

    return bestQ;
}

static int findQ_adaptive(const ImageDCTData &dctData, double targetBpp, const std::string &imagePath) {
    int Q_min = 0;
    int Q_max = 20;
    int bestQ = 0;
    double bestBpp = 1e9;

    while (Q_min <= Q_max) {
        int Q_mid = (Q_min + Q_max) / 2;

        std::string tempDctPath = "dct_adapt/" + imagePath + "_temp_Q" + std::to_string(Q_mid) + ".DCT";

        quantizeWriteDCT_adaptive(dctData, Q_mid, tempDctPath);
        double actBpp = calCompressedBpp(tempDctPath);

        if (actBpp < 0) {
            std::cerr << "Error calculating compressed bpp for Q=" << Q_mid << std::endl;
            break;
        }

        if (std::abs(actBpp - targetBpp) < std::abs(bestBpp - targetBpp)) {
            bestBpp = actBpp;
            bestQ = Q_mid;
        }

        if (actBpp > targetBpp) {
            Q_min = Q_mid + 1;
        } else {
            Q_max = Q_mid - 1;
        }

        std::remove(tempDctPath.c_str());

        if (std::abs(actBpp - targetBpp) < 0.05) {
            break;
        }
    }

    return bestQ;
}

unsigned char* fixedPipeline(const std::string &imagePath, int M, int Q, double B) {
    int N = 8; // block size

    int actualQ = Q;
    ImageDCTData dctData;

    if (B > 0.0 && Q == -1) {
        std::cout << "B > 0 mode: Finding optimal Q for target BPP = " << B << std::endl;

        // Compute DCT coeffs only once
        dctData = calDCT_fixed(imagePath);

        // Find the best Q that meets the target Bpp
        std::string basename = imagePath;
        size_t lastSlash = basename.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            basename = basename.substr(lastSlash + 1);
        }
        size_t lastDot = basename.find_last_of('.');
        if (lastDot != std::string::npos) {
            basename = basename.substr(0, lastDot);
        }
        
        // Find the best Q that meets the target Bpp via binary search
        actualQ = findQ_fixed(dctData, B, basename);
        std::cout << "Found optimal Q = " << actualQ << std::endl;
    }

    unsigned char *inData = readImageData_simple(imagePath);
    if (!inData) {
        std::cerr << "Failed to read image data from: " << imagePath << std::endl;
        return nullptr;
    }

    // reconstruction buffer
    std::vector<unsigned char> recon(IMAGE_WIDTH * IMAGE_HEIGHT * 3, 0);

    // block processing loop
    assert(IMAGE_WIDTH % N == 0 && IMAGE_HEIGHT % N == 0);
    const int blocksX = IMAGE_WIDTH / N;  // 64
    const int blocksY = IMAGE_HEIGHT / N; // 64
    const int blocksTotal = blocksX * blocksY;

    std::vector<int> blockMeta;
    std::vector<int32_t> flattenedCoeffs;

    // Clear the global block metadata
    g_blockMetadata.clear();

    bool useCachedDCT = (B > 0.0 && Q == -1);
    size_t blkIdx = 0;

    for (int by = 0; by < blocksY; ++by) {
        for (int bx = 0; bx < blocksX; ++bx) {
            // starting coordinates of the block in the image
            int x0 = bx * N;
            int y0 = by * N;

            // store block metadata (x0, y0, N) for later use in reconstruction
            blockMeta.push_back(x0);
            blockMeta.push_back(y0);
            blockMeta.push_back(N);

            // 3 channels
            for (int ch = 0; ch < 3; ++ch) {
                std::vector<std::vector<double>> coeffs;

                if (useCachedDCT) {
                    coeffs = dctData.dctCoeffs[blkIdx][ch];
                } else {
                    // each channel is a 8x8 block of pixel values
                    std::vector<std::vector<double>> block(N, std::vector<double>(N, 0.0));
                    for (int yy = 0; yy < N; ++yy) {
                        // py and px is the actual pixel coordinates in the image
                        const int py = y0 + yy;
                        for (int xx = 0; xx < N; ++xx) {
                            const int px = x0 + xx;
                            int index = (py * IMAGE_WIDTH + px) * 3 + ch;
                            // level shift the pixel value by 128 to center around 0 for DCT
                            block[yy][xx] = static_cast<double>(inData[index]) - 128.0;
                        }
                    }
                    coeffs = dct2d(block);
                }

                auto qcoeffs = quantizeBlock(coeffs, actualQ);

                // flatten the quantized coefficients and store for later use in reconstruction
                for (int i = 0; i < N; ++i) {
                    for (int j = 0; j < N; ++j)
                        flattenedCoeffs.push_back(static_cast<int32_t>(qcoeffs[i][j]));
                }

                auto deqCoeffs = dequantizBlock(qcoeffs, actualQ);
                auto reconBlock = idct2d(deqCoeffs);

                // store the reconstructed block in the recon buffer
                for (int yy = 0; yy < N; ++yy) {
                    const int py = y0 + yy;
                    for (int xx = 0; xx < N; ++xx) {
                        const int px = x0 + xx;
                        int index = (py * IMAGE_WIDTH + px) * 3 + ch;

                        // level shift back by adding 128
                        double v = reconBlock[yy][xx] + 128.0;
                        int iv = static_cast<int>(std::round(v));

                        // clamp to [0, 255]
                        if (iv < 0) iv = 0;
                        else if (iv > 255) iv = 255;

                        recon[index] = static_cast<unsigned char>(iv);
                    }
                }
            }   // channel
            blkIdx++;  // Move to next block
        }   //bx
    }   //by

    // Write the DCT file with quantized coefficients
    std::string basename = imagePath;
    size_t lastSlash = basename.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        basename = basename.substr(lastSlash + 1);
    }
    size_t lastDot = basename.find_last_of('.');
    if (lastDot != std::string::npos) {
        basename = basename.substr(0, lastDot);
    }
    std::string dctFilename = "dct/" + basename + "_Q" + std::to_string(actualQ) + ".DCT";

    writeDCTfile(dctFilename, N, blockMeta, flattenedCoeffs);

    // Store block metadata globally for visualization
    g_blockMetadata = blockMeta;

    // return the reconstructed image data
    unsigned char *out = (unsigned char *) malloc(static_cast<size_t>(IMAGE_WIDTH * IMAGE_HEIGHT * 3 * sizeof(unsigned char)));
    if (!out) {
        std::cerr << "Failed to allocate memory for output image." << std::endl;
        return nullptr;
    }
    memcpy(out, recon.data(), static_cast<size_t>(IMAGE_WIDTH * IMAGE_HEIGHT * 3 * sizeof(unsigned char)));
    return out;
}

unsigned char* adaptivePipeline(const std::string &imagePath, int M, int Q, double B) {

    if (M != 2) {
        std::cerr << "Currently only M=2 is supported in adaptive pipeline." << std::endl;
        return nullptr;
    }

    // Handle B > 0: find optimal Q using binary search
    int actualQ = Q;
    ImageDCTData dctData;

    if (B > 0.0 && Q == -1) {
        std::cout << "B > 0 mode: Finding optimal Q for target BPP = " << B << std::endl;

        // Compute DCT only once
        dctData = calDCT_adaptive(imagePath);

        // Binary search for optimal Q
        std::string basename = imagePath;
        size_t lastSlash = basename.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            basename = basename.substr(lastSlash + 1);
        }
        size_t lastDot = basename.find_last_of('.');
        if (lastDot != std::string::npos) {
            basename = basename.substr(0, lastDot);
        }
        
        // Find the best Q that meets the target Bpp via binary search
        actualQ = findQ_adaptive(dctData, B, basename);
        std::cout << "Found optimal Q = " << actualQ << std::endl;
    }

    unsigned char *inData = readImageData_simple(imagePath);
    if (!inData) {
        std::cerr << "Failed to read image data from: " << imagePath << std::endl;
        return nullptr;
    }

    std::vector<unsigned char> recon(IMAGE_WIDTH * IMAGE_HEIGHT * 3, 0);

    // Compute the luminance buffer and block map for adaptive processing
    std::vector<double> lum = computeLumBuffer(inData, IMAGE_WIDTH, IMAGE_HEIGHT);
    std::vector<Block> blocks = computeBlockMap(lum, IMAGE_WIDTH, IMAGE_HEIGHT, 0.15);

    std::vector<int> blockMeta;
    std::vector<int32_t> flattenedCoeffs;

    // Clear the global block metadata
    g_blockMetadata.clear();

    bool useCachedDCT = (B > 0.0 && Q == -1);
    size_t blkIdx = 0;

    for (const Block &blk: blocks) {
        int x0 = blk.x0;
        int y0 = blk.y0;
        int N = blk.N;

        blockMeta.push_back(x0);
        blockMeta.push_back(y0);
        blockMeta.push_back(N);

        for (int ch = 0; ch < 3; ++ch) {
            std::vector<std::vector<double>> coeffs;

            if (useCachedDCT) {
                // Use cached DCT coefficients
                coeffs = dctData.dctCoeffs[blkIdx][ch];
            } else {
                // Compute DCT as before
                std::vector<std::vector<double>> block(N, std::vector<double>(N, 0.0));
                for (int yy = 0; yy < N; ++yy) {
                    int py = y0 + yy;
                    for (int xx = 0; xx < N; ++xx) {
                        int px = x0 + xx;
                        int tmpY = py;
                        int tmpX = px;
                        if (tmpY >= IMAGE_HEIGHT) tmpY = IMAGE_HEIGHT - 1;
                        if (tmpX >= IMAGE_WIDTH) tmpX = IMAGE_WIDTH - 1;
                        if (tmpY < 0) tmpY = 0;
                        if (tmpX < 0) tmpX = 0;

                        int index = (tmpY * IMAGE_WIDTH + tmpX) * 3 + ch;
                        block[yy][xx] = static_cast<double>(inData[index]) - 128.0;
                    }
                }
                coeffs = dct2d(block);
            }

            auto qcoeffs = quantizeBlock(coeffs, actualQ);

            for (int i = 0; i < N; ++i) {
                for (int j = 0; j < N; ++j)
                    flattenedCoeffs.push_back(static_cast<int32_t>(qcoeffs[i][j]));
            }

            auto deqCoeffs = dequantizBlock(qcoeffs, actualQ);
            auto reconBlock = idct2d(deqCoeffs);

            for (int yy = 0; yy < N; ++yy) {
                int py = y0 + yy;
                for (int xx = 0; xx < N; ++xx) {
                    int px = x0 + xx;
                    int tmpY = py;
                    int tmpX = px;

                    if (tmpY >= IMAGE_HEIGHT) tmpY = IMAGE_HEIGHT - 1;
                    if (tmpX >= IMAGE_WIDTH) tmpX = IMAGE_WIDTH - 1;
                    if (tmpY < 0) tmpY = 0;
                    if (tmpX < 0) tmpX = 0;

                    double v = reconBlock[yy][xx] + 128.0;
                    int iv = static_cast<int>(std::round(v));
                    if (iv < 0) iv = 0;
                    else if (iv > 255) iv = 255;

                    int index = (tmpY * IMAGE_WIDTH + tmpX) * 3 + ch;
                    recon[index] = static_cast<unsigned char>(iv);
                }
            }
        } // channel
        blkIdx++;  // Move to next block
    } // blocks

    // Write the DCT file with quantized coefficients
    std::string basename = imagePath;
    size_t lastSlash = basename.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        basename = basename.substr(lastSlash + 1);
    }
    size_t lastDot = basename.find_last_of('.');
    if (lastDot != std::string::npos) {
        basename = basename.substr(0, lastDot);
    }
    std::string dctFilename = "dct_adapt/" + basename + "_Q" + std::to_string(actualQ) + ".DCT";

    writeDCTfile_adaptive(dctFilename, blockMeta, flattenedCoeffs);

    // Store block metadata globally for visualization
    g_blockMetadata = blockMeta;

    unsigned char *out = (unsigned char *) malloc(static_cast<size_t>(IMAGE_WIDTH * IMAGE_HEIGHT * 3 * sizeof(unsigned char)));
    if (!out) {
        std::cerr << "Failed to allocate memory for output image." << std::endl;
        return nullptr;
    }
    memcpy(out, recon.data(), static_cast<size_t>(IMAGE_WIDTH * IMAGE_HEIGHT * 3 * sizeof(unsigned char)));
    return out;
}
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cassert>
#include <cstdlib>
#include <sys/stat.h>

#include "controller.h"
#include "dct.cpp"
#include "quantizer.cpp"
#include "block_decision.h"

// Global variable to store block metadata for visualization
std::vector<int> g_blockMetadata;

// A simple pass-through function to read image data from a file.
unsigned char *readImageData_simple(const std::string &imagePath, int width, int height) {
    std::ifstream inputFile(imagePath, std::ios::binary);
    if (!inputFile.is_open()) {
        std::cerr << "Error opening file: " << imagePath << std::endl;
        return nullptr;
    }

    std::vector<char> Rbuf(width * height);
    std::vector<char> Gbuf(width * height);
    std::vector<char> Bbuf(width * height);

    inputFile.read(Rbuf.data(), width * height);
    inputFile.read(Gbuf.data(), width * height);
    inputFile.read(Bbuf.data(), width * height);
    inputFile.close();

    unsigned char *inData =
        (unsigned char *)malloc(width * height * 3 * sizeof(unsigned char));

    if (!inData) return nullptr;

    for (int i = 0; i < width * height; i++) {
        inData[3 * i] = static_cast<unsigned char>(Rbuf[i]);
        inData[3 * i + 1] = static_cast<unsigned char>(Gbuf[i]);
        inData[3 * i + 2] = static_cast<unsigned char>(Bbuf[i]);
    }
    return inData;

}

static double calCompressedBpp(const std::string& filePath, int width, int height) {
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
    double bitsPerPixel = (compressedBytes * 8.0) / (width * height);

    // Clean up the temporary gzip file
    std::remove(gzipFile.c_str());

    return bitsPerPixel;
}

static void writeDCTfile(const std::string &basename, int width, int height, int blockSize, const std::vector<int> &blockMeta, const std::vector<int32_t> &flattenedCoeffs) {
    std::ofstream out(basename, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Error opening file for writing: " << basename << std::endl;
        return;
    }

    uint32_t w = static_cast<uint32_t>(width);
    uint32_t h = static_cast<uint32_t>(height);
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

static void writeDCTfile_adaptive(const std::string &basename, int width, int height, const std::vector<int> &blockMeta, const std::vector<int32_t> &flattenedCoeffs) {
    std::ofstream out(basename, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Error opening file for writing: " << basename << std::endl;
        return;
    }

    uint32_t w = static_cast<uint32_t>(width);
    uint32_t h = static_cast<uint32_t>(height);
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

unsigned char* fixedPipeline(const std::string &imagePath,
                                            int &width, int &height,
                                            int M, int Q, double B) {
    width = 512;
    height = 512;
    int N = 8; // block size

    unsigned char *inData = readImageData_simple(imagePath, width, height);
    if (!inData) {
        std::cerr << "Failed to read image data from: " << imagePath << std::endl;
        return nullptr;
    }

    // reconstruction buffer
    std::vector<unsigned char> recon(width * height * 3, 0);

    // block processing loop
    assert(width % N == 0 && height % N == 0);
    const int blocksX = width / N;  // 64
    const int blocksY = height / N; // 64
    const int blocksTotal = blocksX * blocksY;

    std::vector<int> blockMeta;
    std::vector<int32_t> flattenedCoeffs;

    // Clear the global block metadata
    g_blockMetadata.clear();

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
                // each channel is a 8x8 block of pixel values
                std::vector<std::vector<double>> block(N, std::vector<double>(N, 0.0));
                for (int yy = 0; yy < N; ++yy) {
                    // py and px is the actual pixel coordinates in the image
                    const int py = y0 + yy;
                    for (int xx = 0; xx < N; ++xx) {
                        const int px = x0 + xx;
                        int index = (py * width + px) * 3 + ch;
                        // level shift the pixel value by 128 to center around 0 for DCT
                        block[yy][xx] = static_cast<double>(inData[index]) - 128.0;
                    }
                }
                
                // DCT and quantization
                auto coeffs = dct2d(block);
                auto qcoeffs = quantizeBlock(coeffs, Q);

                // flatten the quantized coefficients and store for later use in reconstruction
                for (int i = 0; i < N; ++i) {
                    for (int j = 0; j < N; ++j) 
                        flattenedCoeffs.push_back(static_cast<int32_t>(qcoeffs[i][j]));
                }

                auto deqCoeffs = dequantizBlock(qcoeffs, Q);
                auto reconBlock = idct2d(deqCoeffs);

                // store the reconstructed block in the recon buffer
                for (int yy = 0; yy < N; ++yy) {
                    const int py = y0 + yy;
                    for (int xx = 0; xx < N; ++xx) {
                        const int px = x0 + xx;
                        int index = (py * width + px) * 3 + ch;

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
    std::string dctFilename = "dct/" + basename + "_Q" + std::to_string(Q) + ".DCT";

    writeDCTfile(dctFilename, width, height, N, blockMeta, flattenedCoeffs);

    // Store block metadata globally for visualization
    g_blockMetadata = blockMeta;

    // return the reconstructed image data
    unsigned char *out = (unsigned char *) malloc(static_cast<size_t>(width * height * 3 * sizeof(unsigned char)));
    if (!out) {
        std::cerr << "Failed to allocate memory for output image." << std::endl;
        return nullptr;
    }
    memcpy(out, recon.data(), static_cast<size_t>(width * height * 3 * sizeof(unsigned char)));
    return out;
}

unsigned char* adaptivePipeline(const std::string &imagePath,
                                            int &width, int &height,
                                            int M, int Q, double B) {
                    
    width = 512, height = 512;

    if (M != 2) {
        std::cerr << "Currently only M=2 is supported in adaptive pipeline." << std::endl;
        return nullptr;
    }
    // if (Q != -1) {
    //     std::cerr << "Currently only Q=-1 (i.e. no quantization) is supported in adaptive pipeline." << std::endl;
    //     return nullptr;
    // }

    unsigned char *inData = readImageData_simple(imagePath, width, height);
    if (!inData) {
        std::cerr << "Failed to read image data from: " << imagePath << std::endl;
        return nullptr; 
    }

    std::vector<unsigned char> recon(width * height * 3, 0);

    // Compute the luminance buffer and block map for adaptive processing
    std::vector<double> lum = computeLumBuffer(inData, width, height);
    std::vector<Block> blocks = computeBlockMap(lum, width, height, 0.15);

    std::vector<int> blockMeta;
    std::vector<int32_t> flattenedCoeffs;

    // Clear the global block metadata
    g_blockMetadata.clear();

    for (const Block &blk: blocks) {
        int x0 = blk.x0;
        int y0 = blk.y0;
        int N = blk.N;

        blockMeta.push_back(x0);
        blockMeta.push_back(y0);
        blockMeta.push_back(N);

        for (int ch = 0; ch < 3; ++ch) {
            std::vector<std::vector<double>> block(N, std::vector<double>(N, 0.0));
            for (int yy = 0; yy < N; ++yy) {
                int py = y0 + yy;
                // level shift for DCT
                for (int xx = 0; xx < N; ++xx) {
                    int px = x0 + xx;
                    int tmpY = py;
                    int tmpX = px;
                    if (tmpY >= height) tmpY = height - 1;
                    if (tmpX >= width) tmpX = width - 1;
                    if (tmpY < 0) tmpY = 0;
                    if (tmpX < 0) tmpX = 0;

                    int index = (tmpY * width + tmpX) * 3 + ch;
                    block[yy][xx] = static_cast<double>(inData[index]) - 128.0;
                }
            }

            auto coeffs = dct2d(block);
            auto qcoeffs = quantizeBlock(coeffs, Q);

            for (int i = 0; i < N; ++i) {
                for (int j = 0; j < N; ++j) 
                    flattenedCoeffs.push_back(static_cast<int32_t>(qcoeffs[i][j]));
            }

            auto deqCoeffs = dequantizBlock(qcoeffs, Q);
            auto reconBlock = idct2d(deqCoeffs);

            for (int yy = 0; yy < N; ++yy) {
                int py = y0 + yy;
                for (int xx = 0; xx < N; ++xx) {
                    int px = x0 + xx;
                    int tmpY = py;
                    int tmpX = px;

                    if (tmpY >= height) tmpY = height - 1;
                    if (tmpX >= width) tmpX = width - 1;
                    if (tmpY < 0) tmpY = 0;
                    if (tmpX < 0) tmpX = 0;

                    double v = reconBlock[yy][xx] + 128.0;
                    int iv = static_cast<int>(std::round(v));
                    if (iv < 0) iv = 0;
                    else if (iv > 255) iv = 255;

                    int index = (tmpY * width + tmpX) * 3 + ch;
                    recon[index] = static_cast<unsigned char>(iv);
                }
            }
        } // channel
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
    std::string dctFilename = "dct_adapt/" + basename + "_Q" + std::to_string(Q) + ".DCT";

    writeDCTfile_adaptive(dctFilename, width, height, blockMeta, flattenedCoeffs);

    // Store block metadata globally for visualization
    g_blockMetadata = blockMeta;

    unsigned char *out = (unsigned char *) malloc(static_cast<size_t>(width * height * 3 * sizeof(unsigned char)));
    if (!out) {
        std::cerr << "Failed to allocate memory for output image." << std::endl;
        return nullptr;
    }
    memcpy(out, recon.data(), static_cast<size_t>(width * height * 3 * sizeof(unsigned char)));
    return out;
}


unsigned char* controller_process_pass_through(const std::string &imagePath,
                                               int &width, int &height,
                                               int M, int Q, double B) {
    width = 512; height = 512;
    std::ifstream inputFile(imagePath, std::ios::binary);
    if (!inputFile.is_open()) {
        std::cerr << "Error opening file: " << imagePath << std::endl;
        return nullptr;
    }

    std::vector<char> Rbuf(width * height);
    std::vector<char> Gbuf(width * height);
    std::vector<char> Bbuf(width * height);

    inputFile.read(Rbuf.data(), width * height);
    inputFile.read(Gbuf.data(), width * height);
    inputFile.read(Bbuf.data(), width * height);
    inputFile.close();

    unsigned char *out = (unsigned char *)malloc(width * height * 3 * sizeof(unsigned char));
    for (int i = 0; i < width * height; i++) {
        out[3 * i] = Rbuf[i];
        out[3 * i + 1] = Gbuf[i];
        out[3 * i + 2] = Bbuf[i];
    }

    return out;
}
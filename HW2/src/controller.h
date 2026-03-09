#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <string>
#include <vector>

// Global variable to store block metadata for visualization
// Each block is represented by 3 integers: x0, y0, N
extern std::vector<int> g_blockMetadata;

struct ImageDCTData {
    std::vector<int> blockMeta; // metadata for each block (x0, y0, N)
    std::vector<std::vector<std::vector<std::vector<double>>>> dctCoeffs;
    int width;
    int height;
};

unsigned char* fixedPipeline(const std::string &imagePath,
                                               int &width, int &height,
                                               int M, int Q, double B);

unsigned char* adaptivePipeline(const std::string &imagePath,
                                                int &width, int &height,
                                                int M, int Q, double B);
#endif
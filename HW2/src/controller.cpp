#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "controller.h"

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
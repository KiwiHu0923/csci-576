

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "image.h"
#include "processing.h"
#include "analysis.h"

// Required by processing.cpp's runImagePipeline (declared as extern there).
// Defined here since we don't link Main.cpp.
unsigned char* readImageData(std::string imagePath, int width, int height) {
    std::ifstream inputFile(imagePath, std::ios::binary);
    if (!inputFile.is_open()) {
        std::cerr << "Error Opening File for Reading" << std::endl;
        std::exit(1);
    }
    std::vector<char> Rbuf(width * height), Gbuf(width * height), Bbuf(width * height);
    inputFile.read(Rbuf.data(), width * height);
    inputFile.read(Gbuf.data(), width * height);
    inputFile.read(Bbuf.data(), width * height);
    inputFile.close();

    unsigned char* inData = (unsigned char*)malloc(width * height * 3);
    for (int i = 0; i < width * height; i++) {
        inData[3 * i]     = Rbuf[i];
        inData[3 * i + 1] = Gbuf[i];
        inData[3 * i + 2] = Bbuf[i];
    }
    return inData;
}

static Image loadRGB(const std::string& path, int width, int height) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) {
        std::cerr << "Error: cannot open " << path << std::endl;
        std::exit(1);
    }
    std::vector<unsigned char> R(width * height), G(width * height), B(width * height);
    f.read(reinterpret_cast<char*>(R.data()), width * height);
    f.read(reinterpret_cast<char*>(G.data()), width * height);
    f.read(reinterpret_cast<char*>(B.data()), width * height);
    f.close();

    Image img(width, height);
    for (int i = 0; i < width * height; i++) {
        img.data[i * 3 + 0] = R[i];
        img.data[i * 3 + 1] = G[i];
        img.data[i * 3 + 2] = B[i];
    }
    return img;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <imagePath> <Q> <M>" << std::endl;
        return 1;
    }

    std::string imagePath = argv[1];
    int Q = std::stoi(argv[2]);
    int M = std::stoi(argv[3]);
    const float S = 1.0f;
    const int WIDTH = 512, HEIGHT = 512;

    // Load original (before quantization)
    Image original = loadRGB(imagePath, WIDTH, HEIGHT);

    // Run pipeline (S=1.0 skips filter/resample, applies only quantization)
    int w = WIDTH, h = HEIGHT;
    unsigned char* outData = runImagePipeline(imagePath, w, h, S, Q, M);

    Image output(w, h);
    output.data.assign(outData, outData + w * h * 3);
    free(outData);

    double mse = computeMSE(original, output);
    double mae = computeMAE(original, output);

    // Extract just the filename for the CSV
    std::string name = imagePath;
    auto slash = name.rfind('/');
    if (slash != std::string::npos) name = name.substr(slash + 1);

    // Output CSV row: imageName,Q,M,MSE,MAE
    std::cout << name << "," << Q << "," << M << ","
              << mse << "," << mae << std::endl;

    return 0;
}

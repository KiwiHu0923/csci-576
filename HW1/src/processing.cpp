#include "processing.h"
#include "image.h"

extern unsigned char* readImageData(std::string imagePath, int width, int height);

unsigned char* runImagePipeline(
    const std::string& imagePath,
    int& width,
    int& height,
    float S,
    int Q,
    int M
) {
    unsigned char* rawData = readImageData(imagePath, width, height);

    Image img(width, height);
    img.data.assign(rawData, rawData + width * height * 3);

    free(rawData);

    if (std::abs(S - 1.0f) > 1e-6) {
        Image filtered = applyFilter(img);
        img = resample(filtered, S);

        width = img.width;
        height = img.height;
    }

    quantize(img, Q, M);

    unsigned char* outData =
        (unsigned char*)malloc(width * height * 3);

    std::memcpy(outData, img.data.data(), width * height * 3);

    return outData;
}
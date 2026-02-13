#pragma once
#include "image.h"

Image applyFilter(const Image& img);
Image resample(const Image &img, float S);
void quantize(Image &img, int Q, int M);
unsigned char* runImagePipeline(
    const std::string& imagePath,
    int& width,
    int& height,
    float S,
    int Q,
    int M
);
#include "image.h"

Image::Image(int w, int h) : width(w), height(h) {
    data.resize(width * height * 3);
}

unsigned char Image::get(int x, int y, int c) const {
    return data[(y * width + x) * 3 + c];
}

void Image::set(int x, int y, int c, unsigned char val) {
    data[(y * width + x) * 3 + c] = val;
}
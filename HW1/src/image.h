#pragma once
#include <vector>

class Image {
public:
    int width;
    int height;

    std::vector<unsigned char> data; //RGBRGBRGB...

    Image(int w, int h);

    unsigned char* get(int x, int y, int c) const;
    void set(int x, int y, int c, unsigned char val);
};
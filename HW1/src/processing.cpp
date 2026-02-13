#include <cmath>

#include "processing.h"
#include "image.h"

extern unsigned char* readImageData(std::string imagePath, int width, int height);

void build_uniform_map(std::array<std::array<unsigned char, 256>, 3>& map, int levels);
void build_pivot_map(std::array<std::array<unsigned char, 256>, 3>& map, int levels, int M);
void build_optimal_equal_map(std::array<std::array<unsigned char, 256>, 3>& map, const Image& img, int levels);

Image applyFilter(const Image& img) {
    Image out(img.width, img.height);

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            for (int c = 0; c < 3; c++) {
                int sum = 0;
                int count = 0;
                for (int dy : {-1, 0, 1}) {
                    for (int dx : {-1, 0, 1}) {
                        int nx = x + dx;
                        int ny = y + dy;
                        if (nx >= 0 && nx < img.width && ny >= 0 && ny < img.height) {
                            count += 1;
                            sum += img.get(nx, ny, c);
                        }
                    }
                }

                unsigned char val = static_cast<unsigned char>(std::round((double) sum / count)); 
                out.set(x, y, c, val);
            }
        }
    }
    return out;
}

Image resample(const Image &img, float scale) {
    int newWidth = std::max(1, int(img.width * scale));
    int newHeight = std::max(1, int(img.height * scale));

    Image out(newWidth, newHeight);

    for (int y = 0; y < newHeight; y++) {
        for (int x = 0; x < newWidth; x++) {
            int srcX = std::min(int(x / scale), img.width - 1);
            int srcY = std::min(int(y / scale), img.height - 1);

            for (int c = 0; c < 3; c++) {
                out.set(x, y, c, img.get(srcX, srcY, c));
            }
        }
    }
    return out;
}

void quantize(Image &img, int Q, int M) {
    if (Q < 3 || Q > 24 || (Q % 3 != 0)) {
        // invalid Q
        return;
    }

    int bitsPerChannel = Q / 3;
    int levels = 1 << bitsPerChannel; // 2^(bitsPerChannel)

    std::array<std::array<unsigned char, 256>, 3> map;

    if (M == -1) {
        build_uniform_map(map, levels);
    } else if (M >= 0 && M <= 255) {
        build_pivot_map(map, levels, M);
    } else if (M == 256) {
        build_optimal_equal_map(map, img, levels);
    }
    else {
        return;
    }

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            int base = (y * img.width + x) * 3;
            for (int c = 0; c < 3; c++) {
                unsigned char v = img.data[base + c];
                img.data[base + c] = map[c][v];
            }
        }
    }
}
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
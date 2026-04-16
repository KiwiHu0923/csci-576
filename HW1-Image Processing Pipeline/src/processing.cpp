#include <cmath>

#include "processing.h"
#include "image.h"

extern unsigned char* readImageData(std::string imagePath, int width, int height);

void build_uniform_map(std::array<std::array<unsigned char, 256>, 3>& map, int levels);
void build_pivot_map(std::array<std::array<unsigned char, 256>, 3>& map, int levels, int M);
void build_optimal_equal_map(std::array<std::array<unsigned char, 256>, 3>& map, const Image& img, int levels);

void build_uniform_map(std::array<std::array<unsigned char, 256>, 3>(&map), int levels) {
    float interval = 256.0f / levels;       // e.g., for 4 levels, interval = 64.0
    for (int v=0; v<256; v++) {
        int idx = std::min(int(v / interval), levels - 1); // e.g., for v=128, idx=2
        int quantizedVal = int(std::round((idx + 0.5f) * interval));
        if (quantizedVal > 255) quantizedVal = 255;
        if (quantizedVal < 0) quantizedVal = 0;

        for (int c = 0; c < 3; c++) {
            map[c][v] = (unsigned char)quantizedVal;
        }
    }
}

void build_pivot_map(std::array<std::array<unsigned char, 256>, 3>(&map), int levels, int M) {
    float frac = float(M+1) / 256.0f;

    int l_low = std::max(1, int(std::round(levels * frac)));
    int l_high = levels - l_low;

    l_low = std::clamp(l_low, 1, levels - 1);
    l_high = levels - l_low;

    if (M == 255) {
        l_low = levels;
        l_high = 0;
    }
    if (M == 0) {
        l_low = 1;
        l_high = levels - 1;
    }

    float low_interval = (M+1) / (float)l_low;
    float high_interval = (l_high > 0) ? (255 - M) / (float)l_high : 0.0f;


    for (int v=0; v<256; v++) {
        int quantizedVal;
        if (v <= M) {
            int idx = std::min(int(floor(v / low_interval)), l_low - 1);
            quantizedVal = int(std::round((idx + 0.5f) * low_interval));
            if (quantizedVal > M) quantizedVal = M;
        } else {
            int idx = std::min(int(floor((v - (M+1)) / high_interval)), l_high - 1);
            quantizedVal = int(std::round((M+1) + (idx + 0.5f) * high_interval));
            if (quantizedVal > 255) quantizedVal = 255;
        }

        for (int c = 0; c < 3; c++) {
            map[c][v] = (unsigned char)quantizedVal;
        }
    }
}

void build_optimal_equal_map(std::array<std::array<unsigned char, 256>, 3>(&map), const Image& img, int levels) {
    // equal frequency histogram quantization
    int totalPixels = img.width * img.height;
    double threshold = double(totalPixels) / levels;

    for(int c=0; c<3; c++) {
        std::vector<int> histogram(256, 0);
        for (int i=0; i<totalPixels; i++) {
            unsigned char v = img.data[i*3 + c];
            histogram[v]++;
        }

        int cur_level = 0;
        int cur_count = 0;
        std::vector<int> level_sum(levels, 0);
        std::vector<int> level_count(levels, 0);

        for (int v=0; v<256; v++) {
            int cnt = histogram[v];

            level_sum[cur_level] += v * cnt;
            level_count[cur_level] += cnt;
            cur_count += cnt;

            if (cur_count >= threshold && cur_level < levels - 1) {
                cur_level++;
                cur_count = cur_count - threshold;
            }
        }

        std::vector<unsigned char> quantized_values(levels, 0);
        for (int l=0; l<levels; l++) {
            if (level_count[l] > 0) {
                int r = int(std::round(double(level_sum[l]) / level_count[l]));
                quantized_values[l] = (unsigned char) std::clamp(r, 0, 255);
            } else {
                float interval = 256.0f / levels;
                int r = int(std::round((l + 0.5f) * interval));
                quantized_values[l] = (unsigned char) std::clamp(r, 0, 255);
            }
        }

        int cur = 0;
        int count = 0;
        for (int v=0; v<256; v++) {
            count += histogram[v];
            if (count >= threshold && cur < levels - 1) {
                cur++;
                count = count - threshold;
            }
            map[c][v] = quantized_values[cur];
        }
    }
}


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
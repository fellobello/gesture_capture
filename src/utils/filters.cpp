#include "filters.h"
#include <cmath>
#include <algorithm>
#include <vector>
#include <cstdint>

// convert rgb image to grayscale (output must have 1 channel)
void grayscale(const utils::Image& input, utils::Image& output) {
    int w = input.width;
    int h = input.height;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = (y * w + x) * 3;
            uint8_t r = input.data[idx];
            uint8_t g = input.data[idx + 1];
            uint8_t b = input.data[idx + 2];
            // integer approximation of luminosity
            output.data[y * w + x] = (r * 299 + g * 587 + b * 114) / 1000;
        }
    }
}

// basic sobel edge detector (magnitude, no threshold)
void sobel_edge(const utils::Image& input, utils::Image& output) {
    int w = input.width;
    int h = input.height;
    const int kx[3][3] = {
        { -1, 0, 1 },
        { -2, 0, 2 },
        { -1, 0, 1 }
    };
    const int ky[3][3] = {
        { -1, -2, -1 },
        {  0,  0,  0 },
        {  1,  2,  1 }
    };
    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            int gx = 0, gy = 0;
            for (int j = 0; j < 3; ++j) {
                for (int i = 0; i < 3; ++i) {
                    int px = x + i - 1;
                    int py = y + j - 1;
                    gx += input.data[py * w + px] * kx[j][i];
                    gy += input.data[py * w + px] * ky[j][i];
                }
            }
            int mag = std::min(255, static_cast<int>(std::sqrt(gx * gx + gy * gy)));
            output.data[y * w + x] = static_cast<uint8_t>(mag);
        }
    }
}

// fast separable box blur (3-channel)
void box_blur(const utils::Image& input, utils::Image& output, int kernel_size) {
    int w = input.width;
    int h = input.height;
    int channels = input.channels; // will be 3
    int half = kernel_size / 2;

    // temp buffer now needs to be 3-channel
    std::vector<uint8_t> temp(w * h * channels, 0);

    // horizontal blur (per channel)
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            for (int c = 0; c < channels; ++c) { // loop for b, g, r
                int sum = 0;
                int count = 0;
                for (int k = -half; k <= half; ++k) {
                    int xi = x + k;
                    if (xi >= 0 && xi < w) {
                        sum += input.data[(y * w + xi) * channels + c];
                        count++;
                    }
                }
                temp[(y * w + x) * channels + c] = sum / count;
            }
        }
    }
    // vertical blur (per channel)
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            for (int c = 0; c < channels; ++c) { // loop for b, g, r
                int sum = 0;
                int count = 0;
                for (int k = -half; k <= half; ++k) {
                    int yi = y + k;
                    if (yi >= 0 && yi < h) {
                        sum += temp[(yi * w + x) * channels + c];
                        count++;
                    }
                }
                output.data[(y * w + x) * channels + c] = sum / count;
            }
        }
    }
}

void skin(const utils::Image& input, utils::Image& output) {
    int w = input.width, h = input.height;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = (y * w + x) * 3;
            uint8_t r = input.data[idx + 2], g = input.data[idx + 1], b = input.data[idx];

            // RGB to HSV conversion
            float rf = r / 255.f, gf = g / 255.f, bf = b / 255.f;
            float maxc = std::max({rf, gf, bf}), minc = std::min({rf, gf, bf});
            float delta = maxc - minc;

            float h_val = 0.f, s_val = maxc == 0 ? 0 : delta / maxc, v_val = maxc;

            if (delta != 0) {
                if (maxc == rf) h_val = 60 * ((gf - bf) / delta);
                else if (maxc == gf) h_val = 60 * (2 + (bf - rf) / delta);
                else h_val = 60 * (4 + (rf - gf) / delta);
                if (h_val < 0) h_val += 360;
            }

			bool skin_pixel = ((h_val >= 295 && h_val <= 360) || (h_val >= 10 && h_val <= 50)) && (s_val >= 0.15f) && (v_val >= 0.10f);
            output.data[y * w + x] = skin_pixel ? 255 : 0;
        }
    }
}


// generic convolution
void convolve(const utils::Image& input, utils::Image& output, const std::vector<int>& kernel, int ksize, int divisor, int offset) {
    int w = input.width;
    int h = input.height;
    int half = ksize / 2;
    for (int y = half; y < h - half; ++y) {
        for (int x = half; x < w - half; ++x) {
            int sum = 0;
            for (int j = 0; j < ksize; ++j) {
                for (int i = 0; i < ksize; ++i) {
                    int px = x + i - half;
                    int py = y + j - half;
                    sum += input.data[py * w + px] * kernel[j * ksize + i];
                }
            }
            int v = sum / divisor + offset;
            output.data[y * w + x] = std::clamp(v, 0, 255);
        }
    }
}
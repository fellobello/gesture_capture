//
// Created by steven on 11/18/25.
//

#include "Image_utils.h"
#include "Params.h"
#include <queue>

void draw_rect(Image& img, const BoundingBox& box, uint8_t r, uint8_t g, uint8_t b) {
    int w = img.width, h = img.height;
    int x1 = box.x, y1 = box.y, x2 = box.x + box.w - 1, y2 = box.y + box.h - 1;

    if(w*h > params::min_draw_rect_area) {
        // draw top and bottom lines
        for (int x = x1; x <= x2; ++x) {
            if (y1 >= 0 && y1 < h) {
                int idx = (y1 * w + x) * 3;
                img.data[idx] = r; img.data[idx+1] = g; img.data[idx+2] = b;
            }
            if (y2 >= 0 && y2 < h) {
                int idx = (y2 * w + x) * 3;
                img.data[idx] = r; img.data[idx+1] = g; img.data[idx+2] = b;
            }
        }
        // draw left and right lines
        for (int y = y1; y <= y2; ++y) {
            if (x1 >= 0 && x1 < w) {
                int idx = (y * w + x1) * 3;
                img.data[idx] = r; img.data[idx+1] = g; img.data[idx+2] = b;
            }
            if (x2 >= 0 && x2 < w) {
                int idx = (y * w + x2) * 3;
                img.data[idx] = r; img.data[idx+1] = g; img.data[idx+2] = b;
            }
        }
    }
}

void draw_poly(Image& img, const std::vector<std::pair<int, int>>& pts, uint8_t r, uint8_t g, uint8_t b) {
    if (pts.empty()) return;
    int w = img.width, h = img.height, n = static_cast<int>(pts.size());
    for (int i = 0; i < n; ++i) {
        int x0 = pts[i].first, y0 = pts[i].second;
        int x1 = pts[(i + 1) % n].first, y1 = pts[(i + 1) % n].second;
        // basic bresenham line
        int dx = std::abs(x1 - x0), dy = -std::abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1;
        int sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;
        while (true) {
            if (x0 >= 0 && x0 < w && y0 >= 0 && y0 < h) {
                int idx = (y0 * w + x0) * 3;
                img.data[idx] = r; img.data[idx+1] = g; img.data[idx+2] = b;
            }
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }
}

std::vector<BoundingBox> find_bounding_boxes(const Image& binary) {
    int w = binary.width;
    int h = binary.height;
    std::vector<std::vector<bool>> visited(h, std::vector<bool>(w, false));
    std::vector<BoundingBox> boxes;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            if (binary.data[y * w + x] == params::mask_value_off || visited[y][x]) continue;
            int min_x = x, max_x = x, min_y = y, max_y = y;
            std::queue<std::pair<int, int>> q;
            q.emplace(x, y);
            visited[y][x] = true;

            // flood fill to group all connected pixels in the blob
            for(; !q.empty(); ) {
                auto [cx, cy] = q.front(); q.pop();
                for (int dx = -1; dx <= 1; ++dx) {
                    for (int dy = -1; dy <= 1; ++dy) {
                        if (dx == 0 && dy == 0) continue;
                        int nx = cx + dx, ny = cy + dy;
                        if (nx >= 0 && nx < w && ny >= 0 && ny < h && !visited[ny][nx]
                            && binary.data[ny * w + nx] != params::mask_value_off) {
                            visited[ny][nx] = true;
                            q.emplace(nx, ny);
                            // track bounding box
                            min_x = std::min(min_x, nx);
                            max_x = std::max(max_x, nx);
                            min_y = std::min(min_y, ny);
                            max_y = std::max(max_y, ny);
                            }
                    }
                }
            }
            // add bounding box for this connected region
            boxes.push_back({min_x, min_y, max_x - min_x + 1, max_y - min_y + 1});
        }
    }
    return boxes;
}

// convert rgb image to grayscale (output must have 1 channel)
void gray_mask(const Image& input, Image& output) {
    int w = input.width;
    int h = input.height;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = (y * w + x) * params::color_channel_count;
            uint8_t r = input.data[idx];
            uint8_t g = input.data[idx + 1];
            uint8_t b = input.data[idx + 2];
            // integer approximation of luminosity
            output.data[y * w + x] = (
                r * params::grayscale_weight_r +
                g * params::grayscale_weight_g +
                b * params::grayscale_weight_b
            ) / params::grayscale_divisor;
        }
    }
}

// basic sobel edge detector (magnitude, no threshold)
void edge_mask(const Image& input, Image& output) {
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
            int mag = std::min(static_cast<int>(params::mask_value_on), static_cast<int>(std::sqrt(gx * gx + gy * gy)));
            output.data[y * w + x] = static_cast<uint8_t>(mag);
        }
    }
}

// fast separable box blur (3-channel)
void blur_mask(const Image& input, Image& output, int kernel_size) {
    int w = input.width;
    int h = input.height;
    int channels = input.channels; // will be params::color_channel_count
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

void skin_mask(const Image& input, Image& output) {
    int w = input.width, h = input.height;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = (y * w + x) * params::color_channel_count;
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

            bool skin_pixel = (
                ((h_val >= params::skin_hue_low1 && h_val <= params::skin_hue_high1) ||
                 (h_val >= params::skin_hue_low2 && h_val <= params::skin_hue_high2))
                 && (s_val >= params::skin_min_s)
                 && (v_val >= params::skin_min_v)
            );
            output.data[y * w + x] = skin_pixel ? params::mask_value_on : params::mask_value_off;
        }
    }
}
void motion_mask(const Image& current, const Image& previous, Image& output_mask, int threshold_val)
{
    // get image dims and channels
    int w = current.width;
    int h = current.height;
    int channels = current.channels;

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            int idx = (y * w + x) * channels;
            int idx_mask = y * w + x;

            // grayscale current
            uint8_t b0 = current.data[idx];
            uint8_t g0 = current.data[idx + 1];
            uint8_t r0 = current.data[idx + 2];
            float gray0 = params::grayscale_weight_b_f * b0 +
                          params::grayscale_weight_g_f * g0 +
                          params::grayscale_weight_r_f * r0;

            // grayscale previous
            uint8_t b1 = previous.data[idx];
            uint8_t g1 = previous.data[idx + 1];
            uint8_t r1 = previous.data[idx + 2];
            float gray1 = params::grayscale_weight_b_f * b1 +
                          params::grayscale_weight_g_f * g1 +
                          params::grayscale_weight_r_f * r1;

            // diff and mask
            int diff = static_cast<int>(std::abs(gray0 - gray1));
            output_mask.data[idx_mask] = (diff > threshold_val) ? params::mask_value_on : params::mask_value_off;
        }
    }
}

void history_mask(const Image& motion_mask, Image& history_mask, uint8_t decay_speed)
{
    // get size
    int n = history_mask.width * history_mask.height;

    for (int i = 0; i < n; ++i)
    {
        if (motion_mask.data[i] == params::mask_value_on)
            history_mask.data[i] = params::mask_value_on;
        else if (history_mask.data[i] > decay_speed)
            history_mask.data[i] -= decay_speed;
        else
            history_mask.data[i] = params::mask_value_off;
    }
}

// generic convolution
void convolve(const Image& input, Image& output, const std::vector<int>& kernel, int ksize, int divisor, int offset) {
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
            output.data[y * w + x] = static_cast<uint8_t>(
    std::clamp(v, static_cast<int>(params::mask_value_off), static_cast<int>(params::mask_value_on)));
        }
    }
}

// convert Image to cv::Mat for display
cv::Mat image_to_mat(const Image& img) {
    if (img.channels == 1)
        return cv::Mat(img.height, img.width, CV_8UC1, (void*)img.data.data()).clone();
    else if (img.channels == 3)
        return cv::Mat(img.height, img.width, CV_8UC3, (void*)img.data.data()).clone();
    throw std::runtime_error("unexpected image channels");
}
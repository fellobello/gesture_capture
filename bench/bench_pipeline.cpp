// times each stage of the main loop's pipeline on a synthetic 640x480 frame
// (random noise plus a bright blob), so it runs without a camera.
// usage: bench_pipeline [frames]   (default 300)

#include "Image.h"
#include "Image_utils.h"
#include "Params.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>
#include <vector>

namespace {

using clock_type = std::chrono::steady_clock;

struct Stage {
    std::string name;
    std::vector<double> ms;
};

double percentile(std::vector<double> v, double p) {
    std::sort(v.begin(), v.end());
    size_t i = static_cast<size_t>(p * static_cast<double>(v.size() - 1));
    return v[i];
}

// a YUYV frame: noisy background with a bright rectangle in the middle
std::vector<uint8_t> make_frame(int w, int h, std::mt19937& rng) {
    std::uniform_int_distribution<int> noise(16, 60);
    std::vector<uint8_t> yuyv(static_cast<size_t>(w) * h * 2);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; x += 2) {
            size_t i = (static_cast<size_t>(y) * w + x) * 2;
            bool blob = x > w / 3 && x < 2 * w / 3 && y > h / 4 && y < 3 * h / 4;
            yuyv[i] = blob ? 170 : static_cast<uint8_t>(noise(rng));
            yuyv[i + 1] = blob ? 100 : 128;
            yuyv[i + 2] = blob ? 170 : static_cast<uint8_t>(noise(rng));
            yuyv[i + 3] = blob ? 160 : 128;
        }
    }
    return yuyv;
}

} // namespace

int main(int argc, char** argv) {
    int frames = argc > 1 ? std::atoi(argv[1]) : 300;
    if (frames < 1) frames = 1;
    const int w = params::cap_width, h = params::cap_height;

    std::mt19937 rng(42);
    std::vector<std::vector<uint8_t>> inputs;
    for (int i = 0; i < 8; ++i) inputs.push_back(make_frame(w, h, rng));

    std::vector<Stage> stages = {
        {"yuyv_to_bgr", {}}, {"blur_mask (k=3)", {}}, {"gray_mask", {}},
        {"skin_mask", {}}, {"threshold", {}}, {"find_bounding_boxes", {}},
        {"draw_rect", {}}, {"total", {}},
    };

    Image gray(w, h, 1), skin(w, h, 1);
    long checksum = 0;

    for (int f = 0; f < frames; ++f) {
        const auto& yuyv = inputs[f % inputs.size()];
        auto t0 = clock_type::now();
        Image color = yuyv_to_bgr(yuyv.data(), w, h);
        auto t1 = clock_type::now();
        blur_mask(color, color, 3);
        auto t2 = clock_type::now();
        gray_mask(color, gray);
        auto t3 = clock_type::now();
        skin_mask(color, skin);
        auto t4 = clock_type::now();
        for (auto& v : gray.data) v = v > 30 ? params::mask_value_on : params::mask_value_off;
        auto t5 = clock_type::now();
        auto boxes = find_bounding_boxes(gray);
        auto t6 = clock_type::now();
        for (const auto& box : boxes) draw_rect(color, box, 0, 255, 0);
        auto t7 = clock_type::now();

        auto ms = [](auto a, auto b) { return std::chrono::duration<double, std::milli>(b - a).count(); };
        stages[0].ms.push_back(ms(t0, t1));
        stages[1].ms.push_back(ms(t1, t2));
        stages[2].ms.push_back(ms(t2, t3));
        stages[3].ms.push_back(ms(t3, t4));
        stages[4].ms.push_back(ms(t4, t5));
        stages[5].ms.push_back(ms(t5, t6));
        stages[6].ms.push_back(ms(t6, t7));
        stages[7].ms.push_back(ms(t0, t7));
        checksum += static_cast<long>(boxes.size()) + color.data[f % color.data.size()] + skin.data[f % skin.data.size()];
    }

    std::printf("%d frames at %dx%d\n\n", frames, w, h);
    std::printf("%-22s %10s %10s\n", "stage", "median ms", "p99 ms");
    for (const auto& s : stages)
        std::printf("%-22s %10.3f %10.3f\n", s.name.c_str(), percentile(s.ms, 0.5), percentile(s.ms, 0.99));
    std::printf("\nmedian throughput: %.0f frames/s (checksum %ld)\n", 1000.0 / percentile(stages[7].ms, 0.5), checksum);
    return 0;
}

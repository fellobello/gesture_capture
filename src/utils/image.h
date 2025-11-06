#ifndef IMAGE_H
#define IMAGE_H

#include <cstdint>
#include <vector>
#include <string>

namespace utils {

struct Image {
    int width;
    int height;
    int channels;
    std::vector<uint8_t> data;

    Image(int w, int h, int c = 3);
    bool save_to_ppm(const std::string& filename) const; // save as ppm
};

} // namespace utils

#endif // IMAGE_H


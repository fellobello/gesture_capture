#include "image.h"
#include <fstream>

namespace utils {

Image::Image(int w, int h, int c) : width(w), height(h), channels(c), data(w * h * c, 0) {
    // allocate and zero data
}

bool Image::save_to_ppm(const std::string& filename) const {
    if (channels != 3) return false; // only rgb supported
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) return false;
    ofs << "P6\n" << width << " " << height << "\n255\n";
    ofs.write(reinterpret_cast<const char*>(data.data()), width * height * 3);
    return ofs.good();
}

} // namespace utils
//
// created by steven on 11/18/25
//

#ifndef NEW_GESTURE_WEBCAM_H
#define NEW_GESTURE_WEBCAM_H

#include <vector>
#include <string>
#include <optional>
#include "Image.h"


class Webcam {
public:
    explicit Webcam(const std::string& device_path);
    ~Webcam();
    bool open();
    void close();
    std::optional<Image> capture();
    Webcam(const Webcam&) = delete;
    Webcam& operator=(const Webcam&) = delete;

    struct Buffer {
        void* start = nullptr;
        size_t size = 0;
    };

private:
    void cleanup_on_error();
    std::string device_path_;
    int fd_ = -1;
    std::vector<Buffer> buffers_;
    int width_ = 0;
    int height_ = 0;
};

#endif //NEW_GESTURE_WEBCAM_H

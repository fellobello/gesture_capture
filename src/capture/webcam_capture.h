#ifndef WEBCAM_CAPTURE_H
#define WEBCAM_CAPTURE_H

#include <vector>
#include <string>
#include <optional>
#include "../utils/image.h"

namespace capture {

class WebcamCapture {
public:
    explicit WebcamCapture(const std::string& device_path);
    ~WebcamCapture();

    bool open_device(); // open device for capture
    void close_device(); // close and clean up
    std::optional<utils::Image> capture_frame(); // get one rgb frame

    WebcamCapture(const WebcamCapture&) = delete;
    WebcamCapture& operator=(const WebcamCapture&) = delete;

private:
    std::string device_path_;
    int fd_ = -1;
    struct Buffer {
        void* start = nullptr;
        size_t length = 0;
    };
    std::vector<Buffer> buffers_;
    int width_ = 0;
    int height_ = 0;
};

} // namespace capture

#endif // WEBCAM_CAPTURE_H


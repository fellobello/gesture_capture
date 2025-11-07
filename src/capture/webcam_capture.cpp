#include "webcam_capture.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>

namespace capture {

#define CHECK_ERR(x, msg) if ((x) == -1) { perror(msg); return false; }

namespace {
struct Buffer {
    void* start = nullptr;
    size_t length = 0;
};
}

WebcamCapture::WebcamCapture(const std::string& device_path)
    : device_path_(device_path), fd_(-1) {}

WebcamCapture::~WebcamCapture() {
    close_device();
}

bool WebcamCapture::open_device() {
    fd_ = open(device_path_.c_str(), O_RDWR);
    if (fd_ < 0) { perror("open"); return false; }

    v4l2_capability cap = {};
    if (ioctl(fd_, VIDIOC_QUERYCAP, &cap) < 0) { perror("querycap"); close(fd_); return false; }
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) { std::cerr << "not a video capture device\n"; close(fd_); return false; }
    if (!(cap.capabilities & V4L2_CAP_STREAMING)) { std::cerr << "streaming not supported\n"; close(fd_); return false; }

    v4l2_format fmt = {};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = 640;
    fmt.fmt.pix.height = 480;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    if (ioctl(fd_, VIDIOC_S_FMT, &fmt) < 0) { perror("set format"); close(fd_); return false; }

    v4l2_requestbuffers req = {};
    req.count = 2; // allocate 2 buffers for double buffering
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd_, VIDIOC_REQBUFS, &req) < 0) { perror("reqbuf"); close(fd_); return false; }

    buffers_.resize(2);
    for (int i = 0; i < 2; ++i) {
        v4l2_buffer buf = {};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (ioctl(fd_, VIDIOC_QUERYBUF, &buf) < 0) { perror("querybuf"); close(fd_); return false; }
        void* buffer = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, buf.m.offset);
        if (buffer == MAP_FAILED) { perror("mmap"); close(fd_); return false; }
        buffers_[i].start = buffer;
        buffers_[i].length = buf.length;
    }

    // queue all buffers for capture
    for (int i = 0; i < 2; ++i) {
        v4l2_buffer buf = {};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (ioctl(fd_, VIDIOC_QBUF, &buf) < 0) { perror("qbuf"); close_device(); return false; }
    }
    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd_, VIDIOC_STREAMON, &type) < 0) { perror("streamon"); close_device(); return false; }

    width_ = fmt.fmt.pix.width;
    height_ = fmt.fmt.pix.height;
    return true;
}

void WebcamCapture::close_device() {
    if (fd_ != -1) {
        v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ioctl(fd_, VIDIOC_STREAMOFF, &type);
        for (auto& b : buffers_) {
            if (b.start) munmap(b.start, b.length); // free mmap memory
        }
        close(fd_);
        fd_ = -1;
        buffers_.clear();
    }
}

std::optional<utils::Image> WebcamCapture::capture_frame() {
    if (fd_ == -1) return std::nullopt;
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd_, &fds);
    timeval tv = {};
    tv.tv_sec = 2;
    int r = select(fd_+1, &fds, NULL, NULL, &tv);
    if (r <= 0) { perror("select"); return std::nullopt; }
    v4l2_buffer buf = {};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    // don't set index, driver gives which buffer to use
    if (ioctl(fd_, VIDIOC_DQBUF, &buf) < 0) { perror("dqbuf"); return std::nullopt; }
    int buf_idx = buf.index; // which buffer was filled

    const unsigned char* yuyv = (const unsigned char*)buffers_[buf_idx].start;
    utils::Image img(width_, height_, 3);
    for (int i = 0, j = 0; i < width_ * height_ * 2; i += 4, j += 6) {
        int y0 = yuyv[i], u = yuyv[i+1], y1 = yuyv[i+2], v = yuyv[i+3];
        auto conv = [](int y, int u, int v) -> std::array<uint8_t,3> {
            int c = y - 16, d = u - 128, e = v - 128;
            int r = std::clamp((298*c + 409*e + 128)>>8, 0, 255);
            int g = std::clamp((298*c - 100*d - 208*e + 128)>>8, 0, 255);
            int b = std::clamp((298*c + 516*d + 128)>>8, 0, 255);
            return {(uint8_t)r, (uint8_t)g, (uint8_t)b};
        };
        auto rgb0 = conv(y0, u, v); auto rgb1 = conv(y1, u, v);
        img.data[j] = rgb0[0]; img.data[j+1] = rgb0[1]; img.data[j+2] = rgb0[2];
        img.data[j+3] = rgb1[0]; img.data[j+4] = rgb1[1]; img.data[j+5] = rgb1[2];
    }
    if (ioctl(fd_, VIDIOC_QBUF, &buf) < 0) { perror("qbuf2"); }
    return img;
}

} // namespace capture

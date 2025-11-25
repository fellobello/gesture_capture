//
// created by steven on 11/18/25
//

#include "Webcam.h"
#include "Params.h"
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
#include <optional>

Webcam::Webcam(const std::string& device_path){
    device_path_ = device_path;
    open();
}

Webcam::~Webcam(){
    close();
}

bool Webcam::open() {
    fd_ = ::open(device_path_.c_str(), O_RDWR);
    if (fd_ < 0) { perror("open"); return false; }

    v4l2_capability cap = {};
    if (ioctl(fd_, VIDIOC_QUERYCAP, &cap) < 0) { perror("querycap"); cleanup_on_error(); return false; }
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) { std::cerr << "not a video capture device\n"; cleanup_on_error(); return false; }
    if (!(cap.capabilities & V4L2_CAP_STREAMING)) { std::cerr << "streaming not supported\n"; cleanup_on_error(); return false; }

    v4l2_format fmt = {};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = params::cap_width;
    fmt.fmt.pix.height = params::cap_height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    if (ioctl(fd_, VIDIOC_S_FMT, &fmt) < 0) { perror("set format"); cleanup_on_error(); return false; }

    v4l2_requestbuffers req = {};
    req.count = 2;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd_, VIDIOC_REQBUFS, &req) < 0) { perror("reqbuf"); cleanup_on_error(); return false; }

    buffers_.resize(2);
    for (int i = 0; i < 2; ++i) {
        v4l2_buffer buf = {};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (ioctl(fd_, VIDIOC_QUERYBUF, &buf) < 0) { perror("querybuf"); cleanup_on_error(); return false; }
        void* buffer = mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, buf.m.offset);
        if (buffer == MAP_FAILED) { perror("mmap"); cleanup_on_error(); return false; }
        buffers_[i].start = buffer;
        buffers_[i].size = buf.length;
    }

    // queue buffers
    for (int i = 0; i < 2; ++i) {
        v4l2_buffer buf = {};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (ioctl(fd_, VIDIOC_QBUF, &buf) < 0) { perror("qbuf"); close(); return false; }
    }
    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd_, VIDIOC_STREAMON, &type) < 0) { perror("streamon"); close(); return false; }

    width_ = static_cast<int>(fmt.fmt.pix.width);
    height_ = static_cast<int>(fmt.fmt.pix.height);
    return true;
}

void Webcam::close() {
    if (fd_ != -1) {
        v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ioctl(fd_, VIDIOC_STREAMOFF, &type);
        for (auto& b : buffers_) {
            if (b.start) munmap(b.start, b.size);
        }
        buffers_.clear();
        ::close(fd_);
        fd_ = -1;
    }
}

void Webcam::cleanup_on_error() {
    if (fd_ != -1) {
        for (auto& b : buffers_) {
            if (b.start) munmap(b.start, b.size);
        }
        buffers_.clear();
        ::close(fd_);
        fd_ = -1;
    }
}

std::optional<Image> Webcam::capture() {
    if (fd_ == -1) return std::nullopt;
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd_, &fds);
    timeval tv = {};
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    int r = select(fd_ + 1, &fds, nullptr, nullptr, &tv);
    if (r <= 0) { perror("select"); return std::nullopt; }
    v4l2_buffer buf = {};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd_, VIDIOC_DQBUF, &buf) < 0) { perror("dqbuf"); return std::nullopt; }
    int buf_idx = static_cast<int>(buf.index);

    const auto* yuyv = (const unsigned char*)buffers_[buf_idx].start;
    Image img(width_, height_, params::color_channel_count);
    for (int i = 0, j = 0; i < width_ * height_ * 2; i += 4, j += 6) {
        int y0 = yuyv[i], u = yuyv[i + 1], y1 = yuyv[i + 2], v = yuyv[i + 3];
        auto conv = [](int y, int u, int v) -> std::array<uint8_t, 3> {
            int c = y - 16, d = u - 128, e = v - 128;
            int r = std::clamp((298 * c + 409 * e + 128) >> 8, static_cast<int>(params::mask_value_off), static_cast<int>(params::mask_value_on));
            int g = std::clamp((298 * c - 100 * d - 208 * e + 128) >> 8, static_cast<int>(params::mask_value_off), static_cast<int>(params::mask_value_on));
            int b = std::clamp((298 * c + 516 * d + 128) >> 8, static_cast<int>(params::mask_value_off), static_cast<int>(params::mask_value_on));
            return {(uint8_t)b, (uint8_t)g, (uint8_t)r};
        };
        auto bgr0 = conv(y0, u, v);
        auto bgr1 = conv(y1, u, v);
        img.data[j] = bgr0[0];
        img.data[j + 1] = bgr0[1];
        img.data[j + 2] = bgr0[2];
        img.data[j + 3] = bgr1[0];
        img.data[j + 4] = bgr1[1];
        img.data[j + 5] = bgr1[2];
    }
    if (ioctl(fd_, VIDIOC_QBUF, &buf) < 0) { perror("qbuf2"); }
    return img;
}
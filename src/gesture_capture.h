#pragma once
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "capture/webcam_capture.h"
#include "utils/image.h"
#include "keypoints/contour.h"

cv::Mat image_to_mat(const utils::Image& img);

class GestureCapture {
public:
    GestureCapture(const std::string& device = "/dev/video0", bool debug = false);
    ~GestureCapture();

    int open_device();
    void close_device();
    void run();

    void show_view(utils::Image& image_3c,
                   const std::vector<BoundingBox>& boxes,
                   const std::string& window_name);

    void show_debug_view(const utils::Image& image_1c,
                         const std::vector<BoundingBox>& boxes,
                         const std::string& window_name);

private:
    bool debug_view;
    capture::WebcamCapture webcam;
    utils::Image skin_mask, blurred, edges, binary;
    utils::Image prev_rgb;
    utils::Image motion_mask;
    utils::Image motion_history_mask;
    utils::Image final_mask;
    bool first_frame;
};
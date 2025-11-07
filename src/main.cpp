#include "capture/webcam_capture.h"
#include "imageproc/filters.h"
#include "keypoints/contour.h"
#include "utils/image.h"
#include "utils/image_utils.h"
#include <opencv2/opencv.hpp>
#include <iostream>

// handles both rgb (3) and grayscale (1) images safely
cv::Mat image_to_mat(const utils::Image& img) {
    if (img.channels == 1)
        return cv::Mat(img.height, img.width, CV_8UC1, (void*)img.data.data()).clone();
    else if (img.channels == 3)
        return cv::Mat(img.height, img.width, CV_8UC3, (void*)img.data.data()).clone();
    throw std::runtime_error("unexpected channels in image_to_mat");
}

int main() {
    bool debug_view = true; // set to true for blurred grayscale view, false for color

    capture::WebcamCapture webcam("/dev/video0");
    if (!webcam.open_device()) {
        std::cerr << "error: could not open webcam." << std::endl;
        return 1;
    }
    utils::Image gray(640, 480, 1),
                 blurred(640, 480, 1),
                 edges(640, 480, 1),
                 binary(640, 480, 1);

    while (true) {
        auto frame_opt = webcam.capture_frame();
        if (!frame_opt) break;
        utils::Image& rgb = *frame_opt;

        grayscale(rgb, gray);        // rgb to grayscale
        box_blur_separable(gray, blurred, 11);  // blurred grayscale
        sobel_edge(blurred, edges);  // edge detection after blur

        // threshold edges to get binary edge map
        for (size_t i = 0; i < edges.data.size(); ++i)
            binary.data[i] = edges.data[i] > 100 ? 255 : 0;

        auto boxes = find_bounding_boxes(binary);

        int min_area = 500;
        std::vector<BoundingBox> filtered_boxes;
        for (const auto& box : boxes) {
        if (box.w * box.h >= min_area)
                filtered_boxes.push_back(box);
        }

        if (!debug_view) {
            for (const auto& box : filtered_boxes)
                draw_rectangle(rgb, box);
            cv::Mat mat = image_to_mat(rgb);
            cv::imshow("live webcam", mat);
        } else {
            utils::Image blur_rgb(640, 480, 3);
            for (int i = 0; i < 640*480; ++i) {
                uint8_t v = blurred.data[i];
                blur_rgb.data[i*3] = v;
                blur_rgb.data[i*3+1] = v;
                blur_rgb.data[i*3+2] = v;
            }
            for (const auto& box : filtered_boxes)
                draw_rectangle(blur_rgb, box);
            cv::Mat mat = image_to_mat(blur_rgb);
            cv::imshow("live webcam", mat);
        }


        if (cv::waitKey(10) == 27) break;
    }
    webcam.close_device();
    return 0;
}


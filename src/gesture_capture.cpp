#include "gesture_capture.h"
#include "utils/filters.h"
#include "keypoints/contour.h"
#include "utils/image_utils.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <opencv2/highgui.hpp> // for cv::imshow

cv::Mat image_to_mat(const utils::Image& img) {
    if (img.channels == 1)
        return cv::Mat(img.height, img.width, CV_8UC1, (void*)img.data.data()).clone();
    else if (img.channels == 3)
        return cv::Mat(img.height, img.width, CV_8UC3, (void*)img.data.data()).clone();
    throw std::runtime_error("unexpected channels in image_to_mat");
}

GestureCapture::GestureCapture(const std::string& device, bool debug)
    : debug_view{debug},
      webcam{device},
      skin_mask{640, 480, 1},
      blurred{640, 480, 3},
      edges{640, 480, 1},
      binary{640, 480, 1},
      prev_rgb{640, 480, 3},
      motion_mask{640, 480, 1},
      motion_history_mask{640, 480, 1},
      final_mask{640, 480, 1},
      first_frame{true} {}

GestureCapture::~GestureCapture() = default;

int GestureCapture::open_device() {
    return webcam.open_device() ? 0 : 1;
}

void GestureCapture::close_device() {
    webcam.close_device();
}


// creates a 1-channel motion mask from two 3-channel frames
void create_motion_mask(const utils::Image& current,
                        const utils::Image& previous,
                        utils::Image& output_mask,
                        int threshold_val)
{
    int w = current.width;
    int h = current.height;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx_3c = (y * w + x) * 3;
            int idx_1c = y * w + x;

            // get current pixel and convert to grayscale (luma)
            uint8_t b_curr = current.data[idx_3c];
            uint8_t g_curr = current.data[idx_3c + 1];
            uint8_t r_curr = current.data[idx_3c + 2];
            float gray_curr = 0.114f * b_curr + 0.587f * g_curr + 0.299f * r_curr;

            // get previous pixel and convert to grayscale
            uint8_t b_prev = previous.data[idx_3c];
            uint8_t g_prev = previous.data[idx_3c + 1];
            uint8_t r_prev = previous.data[idx_3c + 2];
            float gray_prev = 0.114f * b_prev + 0.587f * g_prev + 0.299f * r_prev;

            // diff and threshold
            int diff = static_cast<int>(std::abs(gray_curr - gray_prev));
            output_mask.data[idx_1c] = (diff > threshold_val) ? 255 : 0;
        }
    }
}

// updates the motion history mask with a decay
void update_motion_history(const utils::Image& motion_mask,
                           utils::Image& history_mask,
                           uint8_t decay_speed)
{
    const size_t num_pixels = history_mask.width * history_mask.height;
    uint8_t* history_ptr = history_mask.data.data();
    const uint8_t* motion_ptr = motion_mask.data.data();

    for (size_t i = 0; i < num_pixels; ++i) {
        if (*motion_ptr++ == 255) {
            // motion detected, reset to full brightness
            *history_ptr = 255;
        } else {
            // no motion, decay the existing value
            if (*history_ptr > decay_speed) {
                *history_ptr -= decay_speed;
            } else {
                *history_ptr = 0;
            }
        }
        history_ptr++;
    }
}

void GestureCapture::show_view(utils::Image& image_3c,
                             const std::vector<BoundingBox>& boxes,
                             const std::string& window_name)
{
    for (const auto& box : boxes) {
        draw_rectangle(image_3c, box);
    }
    cv::Mat mat = image_to_mat(image_3c);
    cv::imshow(window_name, mat);
}

void GestureCapture::show_debug_view(const utils::Image& image_1c,
                                   const std::vector<BoundingBox>& boxes,
                                   const std::string& window_name)
{
    // create 3-channel version of mask to draw color rectangles
    utils::Image display_image(image_1c.width, image_1c.height, 3);

    auto* in_ptr = image_1c.data.data();
    auto* out_ptr = display_image.data.data();
    const size_t num_pixels = image_1c.width * image_1c.height;

    for (size_t i = 0; i < num_pixels; ++i) {
        uint8_t val = *in_ptr++;
        *out_ptr++ = val; // b
        *out_ptr++ = val; // g
        *out_ptr++ = val; // r
    }

    for (const auto& box : boxes) {
        draw_rectangle(display_image, box);
    }

    cv::Mat mat = image_to_mat(display_image);
    cv::imshow(window_name, mat);
}

void GestureCapture::run() {
    bool running = true;
    while (running) {
        auto frame_opt = webcam.capture_frame();
        if (!frame_opt) {
            running = false;
            break;
        }
        utils::Image& rgb = *frame_opt;
        if (first_frame) {
            std::copy(rgb.data.begin(), rgb.data.end(), prev_rgb.data.begin());
            first_frame = false;
            continue;
        }
        // blur
        box_blur(rgb, blurred, 5);
        // skin mask
        skin(blurred, skin_mask);
        // motion mask
        create_motion_mask(blurred, prev_rgb, motion_mask, 25);
        update_motion_history(motion_mask, motion_history_mask, 10);
        // combine skin and motion
        std::transform(skin_mask.data.begin(), skin_mask.data.end(),
                       motion_history_mask.data.begin(),
                       final_mask.data.begin(),
                       [](uint8_t skin, uint8_t history) {
                           return (history > 0) ? skin : 0;
                       }
        );
        // find contours
        auto contours = find_contours(final_mask);
        std::vector<std::vector<std::pair<int, int>>> hand_contours;
        for (const auto& contour : contours) {
            if (is_hand_candidate(contour)) {
                hand_contours.push_back(contour);
            }
        }
        // draw filtered contours
        for (const auto& contour : hand_contours) {
            draw_polygon(rgb, contour, 0, 255, 0);
        }
        // display
        if (!debug_view) {
            cv::Mat mat = image_to_mat(rgb);
            cv::imshow("live webcam", mat);
        } else {
            utils::Image debug_img = final_mask;
            utils::Image debug_color_img(debug_img.width, debug_img.height, 3);
            for (int i = 0; i < debug_img.width * debug_img.height; ++i) {
                uint8_t val = debug_img.data[i];
                debug_color_img.data[i * 3] = val;
                debug_color_img.data[i * 3 + 1] = val;
                debug_color_img.data[i * 3 + 2] = val;
            }
            for (const auto& contour : hand_contours) {
                draw_polygon(debug_color_img, contour, 0, 255, 0);
            }
            cv::Mat mat = image_to_mat(debug_color_img);
            cv::imshow("final mask", mat);
        }
        switch (cv::waitKey(10)) {
            case 27: running = false; break;
            case 'd': debug_view = !debug_view; break;
        }
        std::copy(rgb.data.begin(), rgb.data.end(), prev_rgb.data.begin());
    }
}

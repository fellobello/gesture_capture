//
// Created by steven on 11/18/25.
//

#ifndef NEW_GESTURE_PARAMS_H
#define NEW_GESTURE_PARAMS_H

#include <cstdint>

namespace params {
    constexpr int cap_width = 640;
    constexpr int cap_height = 480;
    // drawing threshold for minimal drawn region (used in draw_rectangle)
    constexpr int min_draw_rect_area = 8000;
    // box blur kernel size (used in box_blur)
    constexpr int blur_kernel_size = 5;
    // motion mask pixel threshold (used in create_motion_mask)
    constexpr int motion_mask_threshold = 25;
    // decay speed for motion history (used in update_motion_history)
    constexpr int motion_history_decay = 10;
    // skin detection (used in skin)
    constexpr int skin_hue_low1 = 295;
    constexpr int skin_hue_high1 = 360;
    constexpr int skin_hue_low2 = 10;
    constexpr int skin_hue_high2 = 50;
    constexpr float skin_min_s = 0.15f;
    constexpr float skin_min_v = 0.10f;
    // is_hand_candidate filtering
    constexpr float min_hand_area = 5000;   // for polygons, may want higher
    constexpr float max_hand_area = 50000;
    constexpr float min_aspect = 0.4f;
    constexpr float max_aspect = 2.5f;
    // contour extraction
    constexpr int min_contour_points = 5;
    // grayscale weights (approximate and float)
    constexpr int grayscale_weight_r = 299;
    constexpr int grayscale_weight_g = 587;
    constexpr int grayscale_weight_b = 114;
    constexpr int grayscale_divisor = 1000;
    constexpr float grayscale_weight_r_f = 0.299f;
    constexpr float grayscale_weight_g_f = 0.587f;
    constexpr float grayscale_weight_b_f = 0.114f;
    // cv::waitKey delay for UI/frame swap
    constexpr int window_delay_ms = 10;
    // binary mask values
    constexpr uint8_t mask_value_on = 255;
    constexpr uint8_t mask_value_off = 0;
    constexpr int color_channel_count = 3;
}

#endif //NEW_GESTURE_PARAMS_H
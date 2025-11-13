#pragma once

namespace gesture_params {
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
}

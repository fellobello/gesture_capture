//
// Created by steven on 11/18/25.
//

#ifndef NEW_GESTURE_IMAGE_UTILS_H
#define NEW_GESTURE_IMAGE_UTILS_H

#include "Image.h"
#include <vector>
#include <opencv2/highgui.hpp>

// bounding box for a contour (min x/y, width/height)
struct BoundingBox {
    int x, y, w, h;
};

void draw_rect(Image& img, const BoundingBox& box, uint8_t r = 255, uint8_t g = 0, uint8_t b = 0);

void draw_poly(Image& img, const std::vector<std::pair<int, int>>& pts, uint8_t r, uint8_t g, uint8_t b);

std::vector<BoundingBox> find_bounding_boxes(const Image& binary);

void gray_mask(const Image& input, Image& output);
void edge_mask(const Image& input, Image& output);
void blur_mask(const Image& input, Image& output, int kernel_size);
void skin_mask(const Image& input, Image& output);
void motion_mask(const Image& current, const Image& previous, Image& output_mask, int threshold_val);
void history_mask(const Image& motion_mask, Image& history_mask, uint8_t decay_speed);

void convolve(const Image& input, Image& output, const std::vector<int>& kernel, int ksize, int divisor = 1, int offset = 0);

cv::Mat image_to_mat(const Image& img);

#endif //NEW_GESTURE_IMAGE_UTILS_H
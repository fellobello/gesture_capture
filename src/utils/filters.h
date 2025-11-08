#pragma once

#include "image.h"
#include <vector>

// grayscale conversion for rgb images
void grayscale(const utils::Image& input, utils::Image& output);

// basic edge detection using sobel operator
void sobel_edge(const utils::Image& input, utils::Image& output);

// arbitrary convolution for single-channel images
void convolve(const utils::Image& input, utils::Image& output, const std::vector<int>& kernel, int ksize, int divisor = 1, int offset = 0);

// box blur kernel for filtering before edge detection
void box_blur_separable(const utils::Image& input, utils::Image& output, int kernel_size);

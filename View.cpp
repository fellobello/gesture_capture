//
// Created by steven on 11/19/25.
//

#include "View.h"
#include "Image_utils.h"
#include <opencv2/imgproc.hpp>
#include <sstream>

View::View()
    : debug_enabled_(false), debug_view_index_(0) {}

void View::handle_key(int key) {
    // toggle debug mode with 'd' or 'D'
    if (key == 'd' || key == 'D') {
        debug_enabled_ = !debug_enabled_;
        if (!debug_enabled_) debug_view_index_ = 0;
    }
    // if debug enabled, allow 0-9 keys to select view index
    if (debug_enabled_) {
        if (key >= '0' && key <= '9') {
            debug_view_index_ = key - '0';
        }
    }
}

cv::Mat View::get_display_image(const Image& img_color, const Image& img_gray, const Image& img_skin) {
    if (!debug_enabled_ || debug_view_index_ == 0) {
        return image_to_mat(img_color);
    }
    switch (debug_view_index_) {
        case 1: return image_to_mat(img_gray);
        case 2: return image_to_mat(img_skin);
            // add more cases here as you add more images
        default: return image_to_mat(img_color);
    }
}

void View::draw_overlay(cv::Mat& mat) {
    if (!debug_enabled_) return;
    std::string text = "debug view: " + current_view_name();
    int font = cv::FONT_HERSHEY_SIMPLEX;
    double scale = 0.5;
    int thickness = 1;
    cv::putText(mat, text, cv::Point(10, 20), font, scale, cv::Scalar(0, 255, 0), thickness);
}

std::string View::current_view_name() const {
    switch (debug_view_index_) {
        case 0: return "final";
        case 1: return "grayscale mask";
        case 2: return "skin mask";
        default: return "unknown";
    }
}

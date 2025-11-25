#ifndef VIEW_H
#define VIEW_H

#include "Image.h"
#include <opencv2/core.hpp>
#include <string>

class View {
public:
    View();

    // call on every key press to update debug state or selected view index
    void handle_key(int key);

    // returns cv::Mat ready to display depending on debug mode and selected view
    cv::Mat get_display_image(const Image& img_color,
                              const Image& img_gray,
                              const Image& img_skin);

    // optionally draw overlay info on mat (modifies input cv::Mat)
    void draw_overlay(cv::Mat& mat);

private:
    bool debug_enabled_;
    int debug_view_index_;

    // returns descriptive name of current debug view
    std::string current_view_name() const;
};

#endif // VIEW_H
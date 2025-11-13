#include "utils/image.h"
#include "keypoints/contour.h" // for BoundingBox

// draws a rectangle on the rgb image using a simple color
void draw_rectangle(utils::Image& img, const BoundingBox& box, uint8_t r = 255, uint8_t g = 0, uint8_t b = 0);

void draw_polygon(utils::Image& img, const std::vector<std::pair<int, int>>& pts, uint8_t r, uint8_t g, uint8_t b);
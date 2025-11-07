#include "image_utils.h"

void draw_rectangle(utils::Image& img, const BoundingBox& box, uint8_t r, uint8_t g, uint8_t b) {
    int w = img.width, h = img.height;
    int x1 = box.x, y1 = box.y, x2 = box.x + box.w - 1, y2 = box.y + box.h - 1;

    // draw top and bottom lines
    for (int x = x1; x <= x2; ++x) {
        if (y1 >= 0 && y1 < h) {
            int idx = (y1 * w + x) * 3;
            img.data[idx] = r; img.data[idx+1] = g; img.data[idx+2] = b;
        }
        if (y2 >= 0 && y2 < h) {
            int idx = (y2 * w + x) * 3;
            img.data[idx] = r; img.data[idx+1] = g; img.data[idx+2] = b;
        }
    }
    // draw left and right lines
    for (int y = y1; y <= y2; ++y) {
        if (x1 >= 0 && x1 < w) {
            int idx = (y * w + x1) * 3;
            img.data[idx] = r; img.data[idx+1] = g; img.data[idx+2] = b;
        }
        if (x2 >= 0 && x2 < w) {
            int idx = (y * w + x2) * 3;
            img.data[idx] = r; img.data[idx+1] = g; img.data[idx+2] = b;
        }
    }
}

#include "image_utils.h"
#include "params.h"
#include <cmath>

void draw_rectangle(utils::Image& img, const BoundingBox& box, uint8_t r, uint8_t g, uint8_t b) {
    int w = img.width, h = img.height;
    int x1 = box.x, y1 = box.y, x2 = box.x + box.w - 1, y2 = box.y + box.h - 1;

    if(w*h > params::min_draw_rect_area) {
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
}

void draw_polygon(utils::Image& img, const std::vector<std::pair<int, int>>& pts, uint8_t r, uint8_t g, uint8_t b) {
    if (pts.empty()) return;
    int w = img.width, h = img.height, n = pts.size();
    for (int i = 0; i < n; ++i) {
        int x0 = pts[i].first, y0 = pts[i].second;
        int x1 = pts[(i + 1) % n].first, y1 = pts[(i + 1) % n].second;
        // basic bresenham line
        int dx = std::abs(x1 - x0), dy = -std::abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1;
        int sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;
        while (true) {
            if (x0 >= 0 && x0 < w && y0 >= 0 && y0 < h) {
                int idx = (y0 * w + x0) * 3;
                img.data[idx] = r; img.data[idx+1] = g; img.data[idx+2] = b;
            }
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }
}

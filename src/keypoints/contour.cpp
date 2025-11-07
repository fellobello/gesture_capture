#include "contour.h"
#include <vector>
#include <queue>

// finds axis-aligned bounding boxes for blobs in a binary image
std::vector<BoundingBox> find_bounding_boxes(const utils::Image& binary) {
    int w = binary.width;
    int h = binary.height;
    std::vector<std::vector<bool>> visited(h, std::vector<bool>(w, false));
    std::vector<BoundingBox> boxes;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            if (binary.data[y * w + x] == 0 || visited[y][x]) continue;
            int min_x = x, max_x = x, min_y = y, max_y = y;
            std::queue<std::pair<int, int>> q;
            q.push({x, y});
            visited[y][x] = true;

            // flood fill to group all connected pixels in the blob
            while (!q.empty()) {
                auto [cx, cy] = q.front(); q.pop();
                for (int dx = -1; dx <= 1; ++dx) {
                    for (int dy = -1; dy <= 1; ++dy) {
                        if (dx == 0 && dy == 0) continue;
                        int nx = cx + dx, ny = cy + dy;
                        if (nx >= 0 && nx < w && ny >= 0 && ny < h && !visited[ny][nx]
                            && binary.data[ny * w + nx] != 0) {
                            visited[ny][nx] = true;
                            q.push({nx, ny});
                            // track bounding box
                            min_x = std::min(min_x, nx);
                            max_x = std::max(max_x, nx);
                            min_y = std::min(min_y, ny);
                            max_y = std::max(max_y, ny);
                            }
                    }
                }
            }
            // add bounding box for this connected region
            boxes.push_back({min_x, min_y, max_x - min_x + 1, max_y - min_y + 1});
        }
    }
    return boxes;
}
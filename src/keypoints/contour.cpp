#include "contour.h"
#include "utils/geometry.h"
#include "params.h"
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
            if (binary.data[y * w + x] == params::mask_value_off || visited[y][x]) continue;
            int min_x = x, max_x = x, min_y = y, max_y = y;
            std::queue<std::pair<int, int>> q;
            q.push({x, y});
            visited[y][x] = true;

            // flood fill to group all connected pixels in the blob
            for(; !q.empty(); ) {
                auto [cx, cy] = q.front(); q.pop();
                for (int dx = -1; dx <= 1; ++dx) {
                    for (int dy = -1; dy <= 1; ++dy) {
                        if (dx == 0 && dy == 0) continue;
                        int nx = cx + dx, ny = cy + dy;
                        if (nx >= 0 && nx < w && ny >= 0 && ny < h && !visited[ny][nx]
                            && binary.data[ny * w + nx] != params::mask_value_off) {
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

// 8-connected contour tracing
std::vector<std::vector<std::pair<int, int>>> find_contours(const utils::Image& binary) {
    int w = binary.width, h = binary.height;
    std::vector<std::vector<bool>> visited(h, std::vector<bool>(w, false));
    std::vector<std::vector<std::pair<int, int>>> contours;

    int dx[8] = {1, 1, 0, -1, -1, -1, 0, 1};
    int dy[8] = {0, 1, 1, 1, 0, -1, -1, -1};

    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            if (binary.data[y*w + x] == params::mask_value_off || visited[y][x]) continue;

            // find border pixel (first 'on' pixel of a blob)
            std::vector<std::pair<int, int>> contour;
            int cx = x, cy = y;
            int dir = 0, start_dir = -1, start_cx = cx, start_cy = cy;
            bool closed = false;

            do {
                contour.push_back({cx, cy});
                visited[cy][cx] = true;
                int found = -1;
                // look around neighborhood
                for (int k = 0; k < 8; ++k) {
                    int nx = cx + dx[(dir + k) % 8];
                    int ny = cy + dy[(dir + k) % 8];
                    if (nx >= 0 && nx < w && ny >= 0 && ny < h &&
                        binary.data[ny * w + nx] != params::mask_value_off && !visited[ny][nx]) {
                        found = (dir + k) % 8;
                        break;
                    }
                }
                if (found != -1) {
                    cx += dx[found];
                    cy += dy[found];
                    dir = (found + 6) % 8; // turn back (left) for next search
                } else {
                    closed = true;
                }
                // stop if back at start or closed
                if (contour.size() > 2 && cx == start_cx && cy == start_cy)
                    closed = true;
            } while (!closed);

            if (contour.size() >= params::min_contour_points) // discard small noise
                contours.push_back(contour);
        }
    }
    return contours;
}

bool is_hand_candidate(const std::vector<std::pair<int,int>>& pts) {
    float area = polygon_area(pts);
    float ar = aspect_ratio(pts);
    if(area < params::min_hand_area || area > params::max_hand_area)
        return false;
    if(ar < params::min_aspect || ar > params::max_aspect)
        return false;
    // add more tests here if desired (solidity, convex hull, etc)
    return true;
}
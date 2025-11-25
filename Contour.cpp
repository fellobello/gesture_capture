//
// Created by steven on 11/18/25.
//

#include "Contour.h"
#include "Geo.h"
#include "Params.h"
#include <vector>
#include <queue>

// 8-connected contour tracing
std::vector<std::vector<std::pair<int, int>>> find_contours(const Image& binary) {
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

bool handidate(const std::vector<std::pair<int,int>>& pts) {
    float area = polygon_area(pts);
    float ar = aspect_ratio(pts);
    if(area < params::min_hand_area || area > params::max_hand_area)
        return false;
    if(ar < params::min_aspect || ar > params::max_aspect)
        return false;
    // add more tests here if desired (solidity, convex hull, etc)
    return true;
}
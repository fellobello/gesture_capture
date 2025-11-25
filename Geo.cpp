//
// Created by steven on 11/18/25.
//

#include "Geo.h"
#include "Params.h"
#include <algorithm>
#include <cmath>

float polygon_area(const std::vector<std::pair<int,int>>& pts) {
    float area = 0;
    int n = pts.size();
    for(int i = 0; i < n; ++i) {
        const auto& p0 = pts[i];
        const auto& p1 = pts[(i+1)%n];
        area += (p0.first * p1.second - p1.first * p0.second);
    }
    return std::abs(area) * 0.5f; // You could parametrize the 0.5f if you wish, but it's a standard geometry formula
}

float aspect_ratio(const std::vector<std::pair<int,int>>& pts) {
    int min_x = pts[0].first, max_x = pts[0].first, min_y = pts[0].second, max_y = pts[0].second;
    for(const auto& p : pts) {
        min_x = std::min(min_x, p.first); max_x = std::max(max_x, p.first);
        min_y = std::min(min_y, p.second); max_y = std::max(max_y, p.second);
    }
    float w = max_x - min_x + 1, h = max_y - min_y + 1; // The +1 is a common pixel math pattern
    return (h == 0) ? 1 : w / h;
}
//
// Created by steven on 11/18/25.
//

#ifndef NEW_GESTURE_GEO_H
#define NEW_GESTURE_GEO_H

#include <vector>
#include <utility>

float polygon_area(const std::vector<std::pair<int,int>>& pts);
float aspect_ratio(const std::vector<std::pair<int,int>>& pts);

#endif //NEW_GESTURE_GEO_H
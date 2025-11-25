//
// Created by steven on 11/18/25.
//

#ifndef NEW_GESTURE_CONTOUR_H
#define NEW_GESTURE_CONTOUR_H

#include <vector>
#include "Image.h"

std::vector<std::vector<std::pair<int, int>>> find_contours(const Image& binary);
bool handidate(const std::vector<std::pair<int,int>>& pts);

#endif //NEW_GESTURE_CONTOUR_H
#pragma once
#include "image.h"
#include <vector>

// bounding box for a contour (min x/y, width/height)
struct BoundingBox {
    int x, y, w, h;
};

// detects bounding boxes of connected nonzero regions (blobs) in binary edge map
std::vector<BoundingBox> find_bounding_boxes(const utils::Image& binary);

//
// Created by steven on 11/18/25.
//

#include "Image.h"

Image::Image(int w, int h, int c) : width(w), height(h), channels(c), data(w * h * c, 0) {
    // allocate and zero data
}
//
// Created by steven on 11/18/25.
//

#ifndef NEW_GESTURE_IMAGE_H
#define NEW_GESTURE_IMAGE_H

#include <cstdint>
#include <vector>

class Image {
    public:
        Image(int w, int h, int c = 3);
        int width;
        int height;
        int channels;
        std::vector<uint8_t> data;
};


#endif //NEW_GESTURE_IMAGE_H
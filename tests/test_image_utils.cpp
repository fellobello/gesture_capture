// unit tests for the image pipeline, on small synthetic images (no camera needed)

#include "Image.h"
#include "Image_utils.h"
#include <gtest/gtest.h>
#include <cstdint>
#include <vector>

namespace {

// set one pixel of a 3-channel BGR image
void set_bgr(Image& img, int x, int y, uint8_t b, uint8_t g, uint8_t r) {
    int idx = (y * img.width + x) * 3;
    img.data[idx] = b; img.data[idx + 1] = g; img.data[idx + 2] = r;
}

uint8_t at(const Image& img, int x, int y, int c = 0) {
    return img.data[(y * img.width + x) * img.channels + c];
}

} // namespace

TEST(YuyvToBgr, BlackAndWhiteShareChroma) {
    // one YUYV group = 2 pixels: Y0=16 (black), Y1=235 (white), neutral chroma
    std::vector<uint8_t> yuyv = {16, 128, 235, 128};
    Image img = yuyv_to_bgr(yuyv.data(), 2, 1);
    ASSERT_EQ(img.channels, 3);
    for (int c = 0; c < 3; ++c) {
        EXPECT_EQ(at(img, 0, 0, c), 0);
        EXPECT_EQ(at(img, 1, 0, c), 255);
    }
}

TEST(YuyvToBgr, PureRedLandsInLastChannel) {
    // BT.601 studio-range red is roughly Y=81, U=90, V=240
    std::vector<uint8_t> yuyv = {81, 90, 81, 240};
    Image img = yuyv_to_bgr(yuyv.data(), 2, 1);
    EXPECT_EQ(at(img, 0, 0, 0), 0);    // blue
    EXPECT_EQ(at(img, 0, 0, 1), 0);    // green
    EXPECT_EQ(at(img, 0, 0, 2), 255);  // red
}

TEST(GrayMask, UsesBgrChannelOrder) {
    Image img(2, 1, 3);
    set_bgr(img, 0, 0, 255, 0, 0);  // pure blue
    set_bgr(img, 1, 0, 0, 0, 255);  // pure red
    Image gray(2, 1, 1);
    gray_mask(img, gray);
    EXPECT_EQ(at(gray, 0, 0), 255 * 114 / 1000);  // 29
    EXPECT_EQ(at(gray, 1, 0), 255 * 299 / 1000);  // 76
}

TEST(BlurMask, ConstantImageIsUnchangedIncludingEdges) {
    Image img(7, 5, 3);
    std::fill(img.data.begin(), img.data.end(), 100);
    Image out(7, 5, 3);
    blur_mask(img, out, 5);
    for (uint8_t v : out.data) EXPECT_EQ(v, 100);
}

TEST(BlurMask, SinglePixelSpreadsOverKernel) {
    Image img(5, 5, 1);
    img.data[2 * 5 + 2] = 255;
    Image out(5, 5, 1);
    blur_mask(img, out, 3);
    // horizontal pass gives 255/3 = 85, vertical pass gives 85/3 = 28
    for (int y = 1; y <= 3; ++y)
        for (int x = 1; x <= 3; ++x)
            EXPECT_EQ(at(out, x, y), 28) << "at " << x << "," << y;
    EXPECT_EQ(at(out, 0, 0), 0);
    EXPECT_EQ(at(out, 4, 4), 0);
}

TEST(BlurMask, WorksInPlace) {
    Image img(5, 5, 1);
    img.data[2 * 5 + 2] = 255;
    blur_mask(img, img, 3);
    EXPECT_EQ(at(img, 2, 2), 28);
    EXPECT_EQ(at(img, 1, 1), 28);
}

TEST(SkinMask, SkinToneOnBlueAndGrayOff) {
    Image img(3, 1, 3);
    set_bgr(img, 0, 0, 105, 172, 224);  // skin tone, hue ~34 degrees
    set_bgr(img, 1, 0, 255, 0, 0);      // blue, hue 240
    set_bgr(img, 2, 0, 128, 128, 128);  // gray, no saturation
    Image mask(3, 1, 1);
    skin_mask(img, mask);
    EXPECT_EQ(at(mask, 0, 0), 255);
    EXPECT_EQ(at(mask, 1, 0), 0);
    EXPECT_EQ(at(mask, 2, 0), 0);
}

TEST(FindBoundingBoxes, SeparateBlobsAndDiagonalConnectivity) {
    Image bin(10, 10, 1);
    // blob A: an L shape
    bin.data[1 * 10 + 1] = 255;
    bin.data[1 * 10 + 2] = 255;
    bin.data[2 * 10 + 2] = 255;
    // blob B: two pixels touching only at a corner (8-connected, so one blob)
    bin.data[7 * 10 + 7] = 255;
    bin.data[8 * 10 + 8] = 255;

    auto boxes = find_bounding_boxes(bin);
    ASSERT_EQ(boxes.size(), 2u);
    EXPECT_EQ(boxes[0].x, 1); EXPECT_EQ(boxes[0].y, 1);
    EXPECT_EQ(boxes[0].w, 2); EXPECT_EQ(boxes[0].h, 2);
    EXPECT_EQ(boxes[1].x, 7); EXPECT_EQ(boxes[1].y, 7);
    EXPECT_EQ(boxes[1].w, 2); EXPECT_EQ(boxes[1].h, 2);
}

TEST(FindBoundingBoxes, EmptyImageHasNoBoxes) {
    Image bin(8, 8, 1);
    EXPECT_TRUE(find_bounding_boxes(bin).empty());
}

TEST(DrawRect, BoxPastTheEdgeIsClippedNotWrapped) {
    Image img(100, 100, 3);
    // reaches past the right and bottom edges; area 16000 is above the draw threshold
    BoundingBox box{50, 20, 200, 80};
    draw_rect(img, box, 255, 0, 0);

    // top edge (row 20) and left edge (column 50) are drawn, in BGR order
    EXPECT_EQ(at(img, 99, 20, 2), 255);
    EXPECT_EQ(at(img, 99, 20, 0), 0);
    EXPECT_EQ(at(img, 50, 99, 2), 255);
    // bottom edge (row 99) is inside the image and drawn up to the last column
    EXPECT_EQ(at(img, 99, 99, 2), 255);
    // the top edge must not wrap around into the start of the next row
    EXPECT_EQ(at(img, 0, 21, 2), 0);
    // inside the box stays untouched
    EXPECT_EQ(at(img, 75, 60, 2), 0);
}

TEST(DrawRect, SmallBoxesAreSkipped) {
    Image img(50, 50, 3);
    draw_rect(img, BoundingBox{10, 10, 5, 5}, 255, 255, 255);
    for (uint8_t v : img.data) EXPECT_EQ(v, 0);
}

TEST(HistoryMask, SetsOnMotionAndDecaysOtherwise) {
    Image motion(3, 1, 1), history(3, 1, 1);
    motion.data = {255, 0, 0};
    history.data = {0, 255, 5};
    history_mask(motion, history, 10);
    EXPECT_EQ(history.data[0], 255);
    EXPECT_EQ(history.data[1], 245);
    EXPECT_EQ(history.data[2], 0);
}

TEST(MotionMask, ThresholdsGrayDifference) {
    Image cur(2, 1, 3), prev(2, 1, 3);
    set_bgr(cur, 0, 0, 200, 200, 200);  // big change from black
    set_bgr(cur, 1, 0, 10, 10, 10);     // small change from black
    Image mask(2, 1, 1);
    motion_mask(cur, prev, mask, 25);
    EXPECT_EQ(at(mask, 0, 0), 255);
    EXPECT_EQ(at(mask, 1, 0), 0);
}

TEST(Convolve, IdentityKernelCopiesInterior) {
    Image img(5, 5, 1), out(5, 5, 1);
    for (int i = 0; i < 25; ++i) img.data[i] = static_cast<uint8_t>(i * 10);
    std::vector<int> identity = {0, 0, 0, 0, 1, 0, 0, 0, 0};
    convolve(img, out, identity, 3);
    for (int y = 1; y < 4; ++y)
        for (int x = 1; x < 4; ++x)
            EXPECT_EQ(at(out, x, y), at(img, x, y));
}

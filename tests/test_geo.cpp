// unit tests for the polygon helpers and the hand-candidate filter

#include "Contour.h"
#include "Geo.h"
#include <gtest/gtest.h>
#include <utility>
#include <vector>

using Pts = std::vector<std::pair<int, int>>;

TEST(PolygonArea, SquareAndTriangle) {
    EXPECT_FLOAT_EQ(polygon_area(Pts{{0, 0}, {10, 0}, {10, 10}, {0, 10}}), 100.f);
    EXPECT_FLOAT_EQ(polygon_area(Pts{{0, 0}, {4, 0}, {0, 3}}), 6.f);
}

TEST(PolygonArea, WindingDirectionDoesNotMatter) {
    EXPECT_FLOAT_EQ(polygon_area(Pts{{0, 10}, {10, 10}, {10, 0}, {0, 0}}), 100.f);
}

TEST(AspectRatio, CountsPixelsInclusively) {
    // spans x 0..19 and y 0..9: 20 pixels wide, 10 tall
    EXPECT_FLOAT_EQ(aspect_ratio(Pts{{0, 0}, {19, 0}, {19, 9}, {0, 9}}), 2.f);
}

TEST(HandCandidate, AcceptsHandSizedBlobAndRejectsOthers) {
    EXPECT_TRUE(handidate(Pts{{0, 0}, {100, 0}, {100, 100}, {0, 100}}));   // area 10000, square
    EXPECT_FALSE(handidate(Pts{{0, 0}, {10, 0}, {10, 10}, {0, 10}}));      // too small
    EXPECT_FALSE(handidate(Pts{{0, 0}, {300, 0}, {300, 30}, {0, 30}}));    // too long and thin
}

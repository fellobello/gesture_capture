cc_library(
    name = "gesture_capture",
    srcs = [
        "capture/webcam_capture.cpp",
        "gesture_capture.cpp",
        "keypoints/contour.cpp",
        "utils/filters.cpp",
        "utils/geometry.cpp",
        "utils/image.cpp",
        "utils/image_utils.cpp",
    ],
    hdrs = [
        "capture/webcam_capture.h",
        "gesture_capture.h",
        "keypoints/contour.h",
        "params.h",
        "utils/filters.h",
        "utils/geometry.h",
        "utils/image.h",
        "utils/image_utils.h",
    ],
    copts = [
        "-std=c++20",
        "-I/usr/include/opencv4",
        "-Wno-deprecated-enum-enum-conversion",
    ],
    includes = [
        ".",  # allow includes relative to src/
        "capture",
        "keypoints",
        "utils",
    ],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "gesture_app",
    srcs = ["main.cpp"],
    copts = [
        "-std=c++20",
        "-Wall",
        "-Wextra",
        "-I/usr/include/opencv4",
    ],
    includes = [
        ".",  # allow includes relative to src/
        "capture",
        "keypoints",
        "utils",
    ],
    linkopts = [
        "-lopencv_core",
        "-lopencv_highgui",
        "-lopencv_imgproc",
        "-lopencv_videoio",
    ],
    deps = [":gesture_capture"],
)

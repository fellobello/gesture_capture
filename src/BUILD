cc_library(
    name = "gesture_capture",
    srcs = [
        "gesture_capture.cpp",
        "capture/webcam_capture.cpp",
        "utils/image.cpp",
        "utils/image_utils.cpp",
        "imageproc/filters.cpp",
        "keypoints/contour.cpp",
    ],
    hdrs = [
        "gesture_capture.h",
        "capture/webcam_capture.h",
        "utils/image.h",
        "utils/image_utils.h",
        "imageproc/filters.h",
        "keypoints/contour.h",
    ],
    includes = [
        ".",
        "capture",
        "utils",
        "imageproc",
        "keypoints",
    ],
    copts = ["-std=c++20", "-Wall", "-Werror", "-Wextra"],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "gesture_app",
    srcs = ["main.cpp"],
    deps = [":gesture_capture"],
    includes = [
        ".",  # allow includes relative to src/
        "capture",
        "utils",
        "imageproc",
        "keypoints",
    ],
    copts = [
        "-std=c++20", "-Wall", "-Wextra",
        "-I/usr/include/opencv4",
    ],
    linkopts = [
        "-lopencv_core",
        "-lopencv_highgui",
        "-lopencv_imgproc",
        "-lopencv_videoio",
    ],
)

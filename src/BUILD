cc_library(
    name = "gesture_capture",
    srcs = [
        "gesture_capture.cpp",
        "capture/webcam_capture.cpp",
        "utils/image.cpp",
    ],
    hdrs = [
        "gesture_capture.h",
        "capture/webcam_capture.h",
        "utils/image.h",
    ],
    copts = ["-std=c++20", "-Wall", "-Werror", "-Wextra"],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "gesture_app",
    srcs = ["main.cpp"],
    deps = [":gesture_capture"],
    copts = [
        "-std=c++20", "-Wall", "-Wextra",  #  <-- Notice -Werror is gone!
        "-I/usr/include/opencv4"
    ],
    linkopts = [
        "-lopencv_core",
        "-lopencv_highgui",
        "-lopencv_imgproc",
        "-lopencv_videoio",
    ],
)
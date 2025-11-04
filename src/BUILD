cc_library(
name = "gesture_capture",
	srcs = ["gesture_capture.cpp"],
	hdrs = ["gesture_capture.h"],
	copts = ["-std=c++20", "-Wall", "-Werror", "-Wextra"],
	visibility = ["//visibility:public"],
)

cc_binary(
	name = "gesture_app",
	srcs = ["main.cpp"],
	deps = [":gesture_capture"],
	copts = ["-std=c++20", "-Wall", "-Werror", "-Wextra"],
)

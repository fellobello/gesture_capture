cmake_minimum_required(VERSION 3.20)
project(new_gesture CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

option(GESTURE_TESTS "Build the unit tests" ON)
option(GESTURE_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

if(GESTURE_SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

# the image pipeline: plain C++, no camera and no OpenCV, so it builds and tests anywhere
add_library(vision STATIC
        Image.cpp
        Image_utils.cpp
        Contour.cpp
        Geo.cpp
        Params.cpp
)
target_include_directories(vision PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(vision PRIVATE -Wall -Wextra -Wpedantic)

# the live app needs V4L2 (Linux) and OpenCV for the window
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(OpenCV REQUIRED)
    add_executable(new_gesture
            main.cpp
            Webcam.cpp
            View.cpp
    )
    target_include_directories(new_gesture PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(new_gesture PRIVATE vision ${OpenCV_LIBS})
    target_compile_options(new_gesture PRIVATE -Wall -Wextra -Wpedantic)
else()
    message(STATUS "Not Linux: skipping the webcam app (V4L2 is Linux only)")
endif()

# time each pipeline stage on a synthetic 640x480 frame
add_executable(bench_pipeline bench/bench_pipeline.cpp)
target_link_libraries(bench_pipeline PRIVATE vision)

if(GESTURE_TESTS)
    include(FetchContent)
    FetchContent_Declare(googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    add_executable(vision_tests
            tests/test_image_utils.cpp
            tests/test_geo.cpp
    )
    target_link_libraries(vision_tests PRIVATE vision GTest::gtest_main)
    target_compile_options(vision_tests PRIVATE -Wall -Wextra)
    include(GoogleTest)
    gtest_discover_tests(vision_tests)
endif()

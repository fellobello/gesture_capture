cmake_minimum_required(VERSION 4.0)
project(new_gesture)

set(CMAKE_CXX_STANDARD 23)

find_package(OpenCV REQUIRED)

add_executable(new_gesture
        main.cpp
        Webcam.cpp
        Params.cpp
        Image.cpp
        Image_utils.cpp
        Image_utils.h
        Contour.cpp
        Contour.h
        Geo.cpp
        Geo.h
        View.cpp
        View.h
)

target_include_directories(new_gesture PRIVATE ${OpenCV_INCLUDE_DIRS})

target_link_libraries(new_gesture PRIVATE ${OpenCV_LIBS})

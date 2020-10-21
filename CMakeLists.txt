cmake_minimum_required(VERSION 2.8)

project(opencv-calibrator CXX)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

add_executable(undistort src/undistort.cpp)
target_link_libraries(undistort ${OpenCV_LIBS})

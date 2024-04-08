cmake_minimum_required(VERSION 3.5)

project(udp_image LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(Qt5 COMPONENTS Widgets Network REQUIRED)
find_package(Qwt REQUIRED)


set(OpenCV_INCLUDE_DIRS
  /usr/local/include/opencv4
  /usr/local/include/opencv4/opencv2
)

set(OpenCV_LIB_DIR
  /usr/local/lib
)

set(OpenCV_LIBS
  opencv_core
  opencv_highgui
  opencv_imgcodecs
  opencv_videoio
  opencv_imgproc
  opencv_features2d
  opencv_flann
  opencv_calib3d
)

include_directories(${catkin_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIB_DIR})

set(UI_FILES
    ui/udpimage.ui
)

qt5_wrap_ui(UI_HEADERS ${UI_FILES})
qt5_add_resources(RESOURCE_FILES resources/images.qrc)

add_executable(udp_image
  include/udpimage.h
  src/main.cpp
  src/udpimage.cpp
  udp/udp.h
  ${UI_HEADERS}
  ${RESOURCE_FILES} 
)

target_link_libraries(udp_image PRIVATE Qt5::Widgets Qt5::Network Qwt::Qwt ${OpenCV_LIBS})

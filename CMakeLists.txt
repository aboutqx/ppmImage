cmake_minimum_required(VERSION 3.0)
project(ppmImage)

find_package(wxWidgets REQUIRED)
include(${wxWidgets_USE_FILE})
link_libraries(${wxWidgets_LIBRARIES})


set(SOURCES
    "src/camera.h"
    "src/drand48.h"
    "src/hitable.h"
    "src/hitableList.h"
    "src/material.h"
    "src/ray.h"
    "src/sphere.h"
    "src/ltexture.h"
    "src/vec3.h"
    "src/scene.h"
    "src/interface/wxraytracer.h"
    "src/scene.cpp"
    "src/interface/wxraytracer.cpp"
    )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(${PROJECT_NAME} WIN32 src/interface/wxraytracer.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "ppmImage")
include_directories(src/interface)
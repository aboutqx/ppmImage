cmake_minimum_required(VERSION 3.1.0)
project(ppmImage)

find_package(wxWidgets REQUIRED)
include(${wxWidgets_USE_FILE})
link_libraries(${wxWidgets_LIBRARIES})

set(SOURCES
    src/raytrace/vec3.h
    src/raytrace/drand48.h
    src/raytrace/hitable.h
    src/raytrace/hitableList.h
    src/raytrace/camera.h
    src/raytrace/material.h
    src/raytrace/ray.h
    src/raytrace/sphere.h
    src/raytrace/texture.h
    src/scene.h
    src/scene.cpp
    src/job.h
    src/job.cpp
    src/interface/wxraytracer.h
    src/interface/wxraytracerFrame.h
    src/interface/wxraytracer.cpp    
    src/interface/wxraytracerFrame.cpp
)

include_directories(src/interface)
include_directories(src/raytrace)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(${PROJECT_NAME} WIN32 ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "ppmImage")

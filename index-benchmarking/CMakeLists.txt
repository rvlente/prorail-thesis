cmake_minimum_required(VERSION 3.10)

project(index-benchmarking)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(PROJ REQUIRED CONFIG)

add_subdirectory(libs/shapelib)
add_subdirectory(libs/absl)
add_subdirectory(libs/geos)
add_subdirectory(libs/s2)
add_subdirectory(libs/h3)

add_executable(geos-strtree src/geos-strtree.cpp)
add_executable(geos-quadtree src/geos-quadtree.cpp)
add_executable(s2-shapeindex src/s2-shapeindex.cpp)
add_executable(h3-cindex src/h3-cindex.cpp)

target_link_libraries(geos-strtree  shp PROJ::proj geos)
target_link_libraries(geos-quadtree shp PROJ::proj geos)
target_link_libraries(s2-shapeindex shp s2)
target_link_libraries(h3-cindex     shp h3)

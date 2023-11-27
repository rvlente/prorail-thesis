cmake_minimum_required(VERSION 3.10)

project(index-benchmarking)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(PROJ REQUIRED CONFIG)
find_package(GDAL REQUIRED CONFIG)

add_subdirectory(libs/absl)

add_subdirectory(libs/geos)
add_subdirectory(libs/s2)
add_subdirectory(libs/h3)

add_executable(geos-strtree  src/geos-strtree.cpp)
add_executable(geos-quadtree src/geos-quadtree.cpp)
add_executable(s2-pointindex src/s2-pointindex.cpp)
add_executable(s2-shapeindex src/s2-shapeindex.cpp)
add_executable(h3-cindex     src/h3-cindex.cpp)

target_link_libraries(geos-strtree  GDAL::GDAL PROJ::proj geos)
target_link_libraries(geos-quadtree GDAL::GDAL PROJ::proj geos)
target_link_libraries(s2-pointindex GDAL::GDAL s2)
target_link_libraries(s2-shapeindex GDAL::GDAL s2)
target_link_libraries(h3-cindex     GDAL::GDAL h3)

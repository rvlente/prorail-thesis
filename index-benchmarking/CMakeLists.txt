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

add_executable(reprod-geos-strtree  src/reprod/geos-strtree.cpp)
add_executable(reprod-geos-quadtree src/reprod/geos-quadtree.cpp)
add_executable(reprod-s2-pointindex src/reprod/s2-pointindex.cpp)

target_link_libraries(geos-strtree  GDAL::GDAL PROJ::proj geos)
target_link_libraries(geos-quadtree GDAL::GDAL PROJ::proj geos)
target_link_libraries(s2-pointindex GDAL::GDAL s2)

cmake_minimum_required(VERSION 3.10)

project(index-benchmarking)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(PROJ REQUIRED CONFIG)
# find_package(GDAL REQUIRED CONFIG)

add_subdirectory(libs/absl)
add_subdirectory(libs/geos)
add_subdirectory(libs/s2)

add_executable(exp11 src/11-nyc-taxi.cpp)
add_executable(exp12 src/12-shippensburg-taxi.cpp)
add_executable(exp13 src/13-aogaki-taxi.cpp)
add_executable(exp14 src/14-germany-taxi.cpp)
add_executable(exp15 src/15-japan-taxi.cpp)
add_executable(exp21 src/21-synthetic-nyc.cpp)
add_executable(exp22 src/22-synthetic-tokyo.cpp)
add_executable(exp23 src/23-synthetic-delhi.cpp)
add_executable(exp24 src/24-synthetic-saopaolo.cpp)

target_link_libraries(exp11 PROJ::proj tcmalloc geos s2)
target_link_libraries(exp12 PROJ::proj tcmalloc geos s2)
target_link_libraries(exp13 PROJ::proj tcmalloc geos s2)
target_link_libraries(exp14 PROJ::proj tcmalloc geos s2)
target_link_libraries(exp15 PROJ::proj tcmalloc geos s2)
target_link_libraries(exp21 PROJ::proj tcmalloc geos s2)
target_link_libraries(exp22 PROJ::proj tcmalloc geos s2)
target_link_libraries(exp23 PROJ::proj tcmalloc geos s2)
target_link_libraries(exp24 PROJ::proj tcmalloc geos s2)

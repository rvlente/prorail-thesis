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

add_executable(01-reprod src/01-vpanday2020-reprod.cpp)

target_link_libraries(01-reprod s2)

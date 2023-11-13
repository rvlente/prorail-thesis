cmake_minimum_required(VERSION 3.10)

project(index-benchmarking)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(deps/absl)
add_subdirectory(deps/s2)
add_subdirectory(deps/geos)

add_executable(main src/main.cpp)

target_link_libraries(main geos)

cmake_minimum_required(VERSION 3.10)

project(index-benchmarking)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(PROJ REQUIRED CONFIG)
# find_package(GDAL REQUIRED CONFIG)

add_subdirectory(libs/absl)
add_subdirectory(libs/geos)
add_subdirectory(libs/s2)

add_executable(exp01 src/01-nyc-taxi.cpp)
add_executable(exp02 src/02-syracuse-taxi.cpp)
add_executable(exp03 src/03-aogaki-taxi.cpp)
add_executable(exp04 src/04-germany-taxi.cpp)
add_executable(exp05 src/05-japan-taxi.cpp)
add_executable(exp06 src/06-synthetic-nyc.cpp)
add_executable(exp07 src/07-synthetic-tokyo.cpp)
add_executable(exp08 src/08-synthetic-delhi.cpp)
add_executable(exp09 src/09-synthetic-saopaolo.cpp)

target_link_libraries(exp01 PROJ::proj tcmalloc geos s2)
target_link_libraries(exp02 PROJ::proj tcmalloc geos s2)
target_link_libraries(exp03 PROJ::proj tcmalloc geos s2)
target_link_libraries(exp04 PROJ::proj tcmalloc geos s2)
target_link_libraries(exp05 PROJ::proj tcmalloc geos s2)
target_link_libraries(exp06 PROJ::proj tcmalloc geos s2)
target_link_libraries(exp07 PROJ::proj tcmalloc geos s2)
target_link_libraries(exp08 PROJ::proj tcmalloc geos s2)
target_link_libraries(exp09 PROJ::proj tcmalloc geos s2)

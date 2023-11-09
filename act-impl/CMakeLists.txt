cmake_minimum_required(VERSION 3.10)

project(my_project)

# Abseil requires C++14
set(CMAKE_CXX_STANDARD 14)

# Process Abseil's CMake build system
add_subdirectory(abseil-cpp)
add_subdirectory(s2geometry)

add_executable(hello_world hello_world.cc)

# Declare dependency on the absl::strings library
target_link_libraries(hello_world absl::strings)
target_link_libraries(hello_world s2)
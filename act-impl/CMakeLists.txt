cmake_minimum_required(VERSION 3.10)

project(act-impl)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(deps/absl)
add_subdirectory(deps/s2)

add_executable(main main.cpp)

target_link_libraries(main absl::strings)
target_link_libraries(main s2)
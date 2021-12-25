cmake_minimum_required(VERSION 3.5)
project(parallel-rank VERSION 0.0.1)

# Export commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

 # C++ standard requirement
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# file(GLOB_RECURSE SRC_FILES src/*.cc)
file(GLOB_RECURSE SRC_FILES src/parallel-rank.cc)

add_executable(parallel-rank ${SRC_FILES})
target_compile_options(parallel-rank PRIVATE -O3 -march=native)

find_package(fmt REQUIRED)
find_package(MPI REQUIRED)

target_link_libraries(parallel-rank PUBLIC fmt::fmt)
target_link_libraries(parallel-rank PUBLIC MPI::MPI_CXX)

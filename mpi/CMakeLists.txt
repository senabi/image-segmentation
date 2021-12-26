cmake_minimum_required(VERSION 3.5)
project(mpi VERSION 0.0.1)

# Export commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

 # C++ standard requirement
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRC_PARALLEL_RANK src/parallel-rank.cc)
file(GLOB_RECURSE SRC_IMG_SEG src/img-seg.cc)

# Projects
set(MPI_PARALLEL_RANK "${CMAKE_PROJECT_NAME}-parallel-rank")
set(MPI_IMG_SEG "${CMAKE_PROJECT_NAME}-img-seg")

# add_executable(${MPI_PARALLEL_RANK} ${SRC_PARALLEL_RANK})
add_executable(${MPI_IMG_SEG} ${SRC_IMG_SEG})
# target_compile_options(${MPI_PARALLEL_RANK} PUBLIC -O3)
target_compile_options(${MPI_IMG_SEG} PUBLIC -O3)

# find_package(fmt REQUIRED)
find_package(MPI REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenMP REQUIRED)
find_package(OpenCV REQUIRED) 

# target_link_libraries(${MPI_PARALLEL_RANK} PUBLIC MPI::MPI_CXX)
# target_link_libraries(${MPI_PARALLEL_RANK} PUBLIC fmt::fmt)

target_link_libraries(${MPI_IMG_SEG} PUBLIC MPI::MPI_CXX)
target_link_libraries(${MPI_IMG_SEG} PUBLIC OpenMP::OpenMP_CXX)
target_link_libraries(${MPI_IMG_SEG} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${MPI_IMG_SEG} PUBLIC ${OpenCV_LIBS})
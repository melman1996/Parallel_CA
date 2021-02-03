cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

project(CellularAutomaton)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

add_executable(CellularAutomaton ${SOURCES})

target_compile_options(CellularAutomaton PRIVATE ${MPI_CXX_COMPILE_FLAGS})
target_link_libraries(CellularAutomaton ${MPI_CXX_LIBRARIES} ${MPI_CXX_LINK_FLAGS})
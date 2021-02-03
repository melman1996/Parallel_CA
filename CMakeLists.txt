cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

project(CellularAutomaton)

find_package(OpenMP)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")
add_executable(CellularAutomaton ${SOURCES})
if (OpenMP_CXX_FOUND)
	target_link_libraries(CellularAutomaton PUBLIC OpenMP::OpenMP_CXX)
endif()
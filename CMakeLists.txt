cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

project(CellularAutomaton)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_executable(CellularAutomaton ${SOURCES})
cmake_minimum_required(VERSION 3.12)
project(graph)

set(CMAKE_CXX_STANDARD 14)

include_directories(
    src
)

add_executable(graph
        src/main.cpp
        src/JumpNetwork.cpp
        src/chess/bitboard.cc
        src/chess/board.cc)

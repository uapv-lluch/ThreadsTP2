cmake_minimum_required(VERSION 3.10)
project(ThreadsTP2)

set(CMAKE_CXX_STANDARD 11)

add_executable(ThreadsTP2 exo2_bis.cpp)

target_link_libraries(ThreadsTP2 pthread)
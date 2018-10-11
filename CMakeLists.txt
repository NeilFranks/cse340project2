cmake_minimum_required(VERSION 3.12)
project(project2_cc)

set(CMAKE_CXX_STANDARD 11)

add_executable(project2_cc main.cpp lexer.h lexer.cpp inputbuf.h inputbuf.cpp)
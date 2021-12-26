cmake_minimum_required(VERSION 3.13)
project(CFG2NPDA)

set(CMAKE_CXX_STANDARD 11)

add_executable(CFG2NPDA cfg2greibach2npda.cpp utils.cpp)

cmake_minimum_required(VERSION 3.16)
project(threadpool)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)


set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)
add_library(threadpool STATIC ${SRC})

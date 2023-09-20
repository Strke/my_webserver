cmake_minimum_required(VERSION 3.16)
project(project)

include_directories(${PROJECT_SOURCE_DIR}/locker)

add_subdirectory(threadpool)
add_subdirectory(http_conn)
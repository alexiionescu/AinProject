cmake_minimum_required(VERSION 3.13)
project(AinProject)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

if (UNIX)
SET(CMAKE_C_FLAGS "-Wall")
endif()

add_subdirectory(ain_server/build)
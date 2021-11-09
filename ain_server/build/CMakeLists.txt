cmake_minimum_required(VERSION 3.13)
project(ain_server C)

set(CMAKE_C_STANDARD 99)
SET(CMAKE_C_FLAGS "-Wall")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin)

include_directories(${PROJECT_SOURCE_DIR}/../include)

file(GLOB CoreSrc ../src/core/*.c)
file(GLOB OsSrc ../src/os/*.c)
file(GLOB OsLinuxSrc ../src/os/linux/*.c)

add_executable(ain_server ${CoreSrc} ${OsLinuxSrc} ${OsSrc})
target_link_libraries(ain_server uring)
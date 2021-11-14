cmake_minimum_required(VERSION 3.13)
project(ain_server C)



set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin)

include_directories(${PROJECT_SOURCE_DIR}/../include)
configure_file(${PROJECT_SOURCE_DIR}/../include/config.h.in 
               ${PROJECT_SOURCE_DIR}/../include/config.h)

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/../include/config_site.h)
    file(WRITE ${PROJECT_SOURCE_DIR}/../include/config_site.h)
endif()

file(GLOB CoreSrc ../src/core/*.c)
file(GLOB OsSrc ../src/os/*.c)
file(GLOB OsLinuxSrc ../src/os/linux/*.c)
file(GLOB OsWinSrc ../src/os/win/*.c)


if(MSVC)
add_executable(ain_server ${CoreSrc} ${OsSrc} ${OsWinSrc} )
elseif(UNIX)
add_executable(ain_server ${CoreSrc} ${OsSrc} ${OsLinuxSrc} )
target_link_libraries(ain_server uring)
endif (MSVC)
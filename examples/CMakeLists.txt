cmake_minimum_required(VERSION 3.30)

PROJECT(az-examples)

#if you don't want the full compiler output, remove the following line
SET(CMAKE_VERBOSE_MAKEFILE ON)
SET(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for Mac OS X" FORCE)

set(LIBS
    arikkei
    az
)

ADD_EXECUTABLE(example
    example.c
)

target_link_libraries(example
    ${LIBS}
)

include_directories(
    ${CMAKE_SOURCE_DIR}/arikkei
    ${CMAKE_SOURCE_DIR}/az}
)

link_directories(
    ${CMAKE_BINARY_DIR}/arikkei/arikkei
    ${CMAKE_BINARY_DIR}/az/az}
)


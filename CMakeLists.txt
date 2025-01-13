cmake_minimum_required(VERSION 3.30)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED YES)

PROJECT(az)

#if you don't want the full compiler output, remove the following line
SET(CMAKE_VERBOSE_MAKEFILE ON)
SET(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for Mac OS X" FORCE)

add_subdirectory(az)

#if(BUILD_TESTING)
#  add_subdirectory(tests)
#endif()

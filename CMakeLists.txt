cmake_minimum_required(VERSION 3.30)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED YES)

PROJECT(az)

#if you don't want the full compiler output, remove the following line
SET(CMAKE_VERBOSE_MAKEFILE ON)
SET(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for Mac OS X" FORCE)

find_package(Doxygen)

add_subdirectory(az)

if(DOXYGEN_FOUND)
    configure_file(${PROJECT_SOURCE_DIR}/etc/Doxyfile.in Doxyfile @ONLY)
    add_custom_target(az-docs ALL
        Doxygen::doxygen Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
#    install(DIRECTORY ${CMAKE_BINARY_DIR}/doc/ DESTINATION ${CMAKE_INSTALL_DOCDIR} )
#    install(DIRECTORY doc/ DESTINATION ${CMAKE_INSTALL_DOCDIR} )
endif()

#if(BUILD_TESTING)
#  add_subdirectory(tests)
#endif()

# If this is part of another project, inform the parent build system
if(NOT PROJECT_IS_TOP_LEVEL)
  set(HAS_AZ true PARENT_SCOPE)
endif()

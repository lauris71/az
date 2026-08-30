cmake_minimum_required(VERSION 3.28)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED YES)

include(CMakePrintHelpers)

PROJECT(az)

# The MSVC variable is only set by project(), so this check must come after it
if(MSVC)
    add_compile_options(/std:c11 /experimental:c11atomics)
endif()

#if you don't want the full compiler output, remove the following line
SET(CMAKE_VERBOSE_MAKEFILE ON)
SET(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for Mac OS X" FORCE)

find_package(Doxygen)

# arikkei is a drop-in static library, expected either nested (az/arikkei) or as a
# sibling (toplevel_project_dir/arikkei). If the parent project already added it,
# that target is used instead. Linking the arikkei target resolves the library
# location, name (libarikkei/libarikkeid) and per-config subdirectory automatically.
if(NOT TARGET arikkei)
	if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/arikkei/CMakeLists.txt)
		add_subdirectory(arikkei)
	elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../arikkei/CMakeLists.txt)
		# Outside the source tree - needs an explicit binary directory
		add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../arikkei ${CMAKE_BINARY_DIR}/arikkei)
	else()
		message(FATAL_ERROR "arikkei not found - expected at ${CMAKE_CURRENT_SOURCE_DIR}/arikkei or ${CMAKE_CURRENT_SOURCE_DIR}/../arikkei")
	endif()
endif()

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

include(CTest)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# If this is part of another project, inform the parent build system
if(NOT PROJECT_IS_TOP_LEVEL)
  set(HAS_AZ true PARENT_SCOPE)
endif()

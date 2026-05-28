set(CMAKE_BUILD_TYPE Debug)

include(CMakePrintHelpers)

cmake_print_variables(CMAKE_SOURCE_DIR)
cmake_print_variables(PROJECT_SOURCE_DIR)

find_library(MATH_LIBRARY m)
find_library(LIBARIKKEI
    NAMES arikkei arikkeid
    PATHS
        ${CMAKE_SOURCE_DIR}/arikkei/build/arikkei ${CMAKE_SOURCE_DIR}/arikkei/build/arikkei/Release
        ${CMAKE_SOURCE_DIR}/../arikkei/build/arikkei ${CMAKE_SOURCE_DIR}/../arikkei/build/arikkei/Release
        ${CMAKE_SOURCE_DIR}/../../build/arikkei/arikkei ${CMAKE_SOURCE_DIR}/../../build/arikkei/arikkei/Release
        ${CMAKE_SOURCE_DIR}/../build/arikkei/arikkei ${CMAKE_SOURCE_DIR}/../build/arikkei/arikkei/Release
        ${PROJECT_SOURCE_DIR}/../build/arikkei/arikkei ${PROJECT_SOURCE_DIR}/../build/arikkei/arikkei/Release
)

add_executable(az_test
    unity/unity.c
    test.c
    hash-map.c
    hash-set.c
)

target_compile_definitions(az_test PRIVATE UNITY_INCLUDE_DOUBLE)
target_include_directories(az_test PRIVATE ${CMAKE_SOURCE_DIR} ..)
target_link_directories(az_test PRIVATE ${CMAKE_SOURCE_DIR}/az)

target_link_libraries(az_test PRIVATE az ${LIBARIKKEI})

if(MATH_LIBRARY)
    target_link_libraries(az_test PRIVATE ${MATH_LIBRARY})
endif()

add_test(NAME types COMMAND az_test types)
add_test(NAME boxed-value COMMAND az_test boxed-value)
add_test(NAME array-list COMMAND az_test array-list)
add_test(NAME hash-map COMMAND az_test hash-map)
add_test(NAME hash-set COMMAND az_test hash-set)

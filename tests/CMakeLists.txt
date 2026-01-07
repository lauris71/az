set(CMAKE_BUILD_TYPE Debug)

find_library(MATH_LIBRARY m)
find_library(LIBARIKKEI arikkei PATHS ${CMAKE_SOURCE_DIR}/arikkei/build/arikkei ${CMAKE_SOURCE_DIR}/arikkei/build/arikkei/Release)

add_executable(az_test
    unity/unity.c
    test.c
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

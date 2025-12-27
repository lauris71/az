set(CMAKE_BUILD_TYPE Debug)

find_library(MATH_LIBRARY m)

add_executable(az_test
    unity/unity.c
    test.c
)

target_compile_definitions(az_test PRIVATE UNITY_INCLUDE_DOUBLE)
target_include_directories(az PRIVATE ${CMAKE_SOURCE_DIR} ..)
target_link_directories(az PRIVATE ${CMAKE_SOURCE_DIR}/az)

target_link_libraries(az_test PRIVATE az arikkei)

if(MATH_LIBRARY)
    target_link_libraries(az_test PRIVATE ${MATH_LIBRARY})
endif()

add_test(NAME types COMMAND az_test types)

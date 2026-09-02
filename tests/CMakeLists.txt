find_library(MATH_LIBRARY m)

add_executable(az_test
    unity/unity.c
    test.c
    hash-map.c
    hash-set.c
    call-native.c
    array-list.c
)

target_compile_definitions(az_test PRIVATE UNITY_INCLUDE_DOUBLE)
target_include_directories(az_test PRIVATE ${CMAKE_SOURCE_DIR} ..)

# The arikkei target (drop-in, added by the top-level CMakeLists) resolves the
# library location, name (libarikkei/libarikkeid) and per-config subdirectory
target_link_libraries(az_test PRIVATE az arikkei)

if(MATH_LIBRARY)
    target_link_libraries(az_test PRIVATE ${MATH_LIBRARY})
endif()

add_test(NAME types COMMAND az_test types)
add_test(NAME types-mt COMMAND az_test types-mt)
add_test(NAME to-string COMMAND az_test to-string)
add_test(NAME boxed-value COMMAND az_test boxed-value)
add_test(NAME array-list COMMAND az_test array-list)
add_test(NAME array COMMAND az_test array)
add_test(NAME call-native COMMAND az_test call-native)
add_test(NAME object-list COMMAND az_test object-list)
add_test(NAME hash-map COMMAND az_test hash-map)
add_test(NAME hash-set COMMAND az_test hash-set)
add_test(NAME masked-field COMMAND az_test masked-field)
add_test(NAME masked-field-widths COMMAND az_test masked-field-widths)
add_test(NAME circular-reference COMMAND az_test circular-reference)
add_test(NAME strings-mt COMMAND az_test strings-mt)
add_test(NAME boxed-interface-conversion COMMAND az_test boxed-interface-conversion)
add_test(NAME value-array COMMAND az_test value-array)

SET(SOURCES
    array.c array.h
    array-list.c array-list.h
    collection.c collection.h
    hash-map.c hash-map.h
    hash-set.c hash-set.h
    list.c list.h
    map.c map.h
    set.c set.h
)

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

TARGET_SOURCES(az PRIVATE ${SOURCES})

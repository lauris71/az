SET(SOURCES
    array.c array.h
    array-list.c array-list.h
    collection.c collection.h
    list.c list.h
)

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

TARGET_SOURCES(az PRIVATE ${SOURCES})

SET(SOURCES
    collection.c collection.h
    list.c list.h
    array-list.c array-list.h
)

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

TARGET_SOURCES(az PRIVATE ${SOURCES})

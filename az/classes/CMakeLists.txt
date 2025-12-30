SET(SOURCES
active-object.c active-object.h
    array-of.c array-of.h
    attrib-dict.c attrib-dict.h
    interface-list.c interface-list.h
    map.c map.h
    object-interface-list.c object-interface-list.h
    object-list.c object-list.h
    value-array.c value-array.h
    value-array-ref.c value-array-ref.h)

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

TARGET_SOURCES(az PRIVATE ${SOURCES})


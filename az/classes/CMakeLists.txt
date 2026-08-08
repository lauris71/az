SET(SOURCES
    active-object.c active-object.h
    array-object.c array-object.h
    attrib-dict.c attrib-dict.h
    interface-list.c interface-list.h
    object-interface-list.c object-interface-list.h
    object-list.c object-list.h
    packed-value-array.c packed-value-array.h
    static-array-of.c static-array-of.h
    value-array.c value-array.h
    value-array-ref.c value-array-ref.h
    weak-object-list.c weak-object-list.h)

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

TARGET_SOURCES(az PRIVATE ${SOURCES})


SET(SOURCES
    buffer-input-stream.c buffer-input-stream.h
    buffer-output-stream.c buffer-output-stream.h
    input-stream.c input-stream.h
    memory-output-stream.c memory-output-stream.h
    os-output-stream.c os-output-stream.h
    output-stream.c output-stream.h
    stream-utils.c stream-utils.h
)

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

TARGET_SOURCES(az PRIVATE ${SOURCES})

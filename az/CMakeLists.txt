set(AZ_HEADERS
	az.h
	base.h
	boxed-interface.h
	boxed-value.h
	class.h
	complex.h
	config.h
	extend.h
	field.h
	function-value.h
	function.h
	interface.h
	object.h
	packed-value.h
	primitives.h
	private.h
	reference-of.h
	reference.h
	serialization.h
	string.h
	types.h
	value.h
	weak-reference.h
)

set(AZ_SOURCES
	base.c
	boxed-interface.c
	boxed-value.c
	class.c
	field.c
	function-value.c
	function.c
	interface.c
	object.c
	packed-value.c
	primitives.c
	private.c
	reference-of.c
	reference.c
	serialization.c
	string.c
	types.c
	value.c
	weak-reference.c
)

add_library(az STATIC
	${AZ_HEADERS}
	${AZ_SOURCES}
)

set_property(TARGET az PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(az PUBLIC
	${PROJECT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/arikkei
)

add_subdirectory(classes)

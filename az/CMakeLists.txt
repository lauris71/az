set(AZ_HEADERS
	active-object.h
	array-of.h
	attrib-dict.h
	az.h
	base.h
	boxed-interface.h
	boxed-value.h
	class.h
	collection.h
	complex.h
	field.h
	function-value.h
	function.h
	interface-list.h
	interface.h
	list.h
	map.h
	object-interface-list.h
	object-list.h
	object.h
	packed-value.h
	primitives.h
	private.h
	reference-of.h
	reference.h
	serialization.h
	string.h
	types.h
	value-array-ref.h
	value-array.h
	value.h
	weak-reference.h
)

set(AZ_SOURCES
	active-object.c
	array-of.c
	attrib-dict.c
	boxed-interface.c
	boxed-value.c
	class.c
	collection.c
	field.c
	function-value.c
	function.c
	interface-list.c
	interface.c
	list.c
	map.c
	object-interface-list.c
	object-list.c
	object.c
	packed-value.c
	primitives.c
	private.c
	reference-of.c
	reference.c
	serialization.c
	string.c
	types.c
	value-array-ref.c
	value-array.c
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

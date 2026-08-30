set(AZ_HEADERS
	az.h
	base.h
	boxed-interface.h
	boxed-value.h
	class.h
	complex.h
	convert.h
	config.h
	extend.h
	field.h
	function-native.h
	function-value.h
	function.h
	instance.c
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
	convert.c
	field.c
	function-native.c
	function-value.c
	function.c
	interface.c
	instance.h
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

# Windows x64 native call trampoline (MSVC, Microsoft x64 calling convention)
# ARM64EC defines _M_X64 as well and is binary-compatible with the x64 trampoline
if(MSVC AND (CMAKE_VS_PLATFORM_NAME MATCHES "^(x64|ARM64EC)$" OR CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64|x86_64|X64|ARM64EC"))
	enable_language(ASM_MASM)
	set(AZ_SOURCES ${AZ_SOURCES} native-call-win64.asm)
	# Single source of truth for both the assembler source and the C guard in function.c
	add_compile_definitions(AZ_NATIVE_CALL_WIN64=1)
endif()

add_library(az STATIC
	${AZ_HEADERS}
	${AZ_SOURCES}
)

set_property(TARGET az PROPERTY POSITION_INDEPENDENT_CODE ON)

# Debug builds produce libarikkeid; consumers linking the target never see the difference
set_target_properties(arikkei PROPERTIES DEBUG_POSTFIX d)

target_include_directories(az PUBLIC
	${PROJECT_SOURCE_DIR}
)

# arikkei is a drop-in static library (added by the top-level CMakeLists);
# linking the target provides its include path transitively
target_link_libraries(az PUBLIC arikkei)

add_subdirectory(classes)
add_subdirectory(collections)
add_subdirectory(io)

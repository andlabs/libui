# 3 june 2016
cmake_minimum_required(VERSION 2.8.11)

project(libui LANGUAGES C CXX)
option(BUILD_SHARED_LIBS "Whether to build libui as a shared library or a static library" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/out")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/out")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/out")
set(CMAKE_PDB_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/out")

if(APPLE)
	set(_OSNAME darwin)
elseif(WIN32)
	set(_OSNAME windows)

	# and don't include the default libraries with ANY of the builds
	# note the CACHE FORCE stuff is required here
	set(CMAKE_C_STANDARD_LIBRARIES CACHE STRING "" FORCE)
	set(CMAKE_CXX_STANDARD_LIBRARIES CACHE STRING "" FORCE)
else()
	set(_OSNAME unix)
endif()

if(BUILD_SHARED_LIBS)
	# shared libraries link against system libs; executables don't
	set(_LIBUI_LINKMODE PRIVATE)
else()
	# static libraries don't link against system libs; executables do
	set(_LIBUI_LINKMODE INTERFACE)
endif()

# common flags
if(MSVC)
	# TODO subsystem version

	# TODO /Wall does too much
	# TODO -Wno-switch equivalent
	# TODO /sdl turns C4996 into an ERROR
	# don't use /analyze; that requires us to write annotations everywhere
	# TODO undecided flags from qo?
	# /RTCc is not supplied because it's discouraged as of VS2015; see https://www.reddit.com/r/cpp/comments/46mhne/rtcc_rejects_conformant_code_with_visual_c_2015/d06auq5
	# /EHsc is to shut the compiler up in some cases
	# TODO make /EHsc C++-only
	set(_COMMON_CFLAGS
		/W4 /wd4100
		/bigobj /nologo
		/RTC1 /RTCs /RTCu
		/EHsc
	)

	# note the /MANIFEST:NO (which must be / and uppercase); thanks FraGag (https://github.com/andlabs/libui/issues/93#issuecomment-223183436)
	# TODO warnings on undefined symbols
	set(_COMMON_LDFLAGS
		/LARGEADDRESSAWARE
		/NOLOGO
		/INCREMENTAL:NO
		/MANIFEST:NO
	)

	# TODO autogenerate a .def file?
else()
endif()

# TODO why do I need to do this? why doesn't target_link_libraries() work?
macro(_target_link_options_private _target)
	foreach(_opt IN LISTS ${ARGN})
		set_property(TARGET ${_target} APPEND_STRING PROPERTY
			LINK_FLAGS " ${_opt}")
	endforeach()
endmacro()

add_subdirectory("common")
add_subdirectory("${_OSNAME}")
add_library(${_LIBUINAME} ${_LIBUI_SOURCES})
target_include_directories(${_LIBUINAME}
	PUBLIC .
	PRIVATE ${_LIBUI_INCLUEDIRS})
target_compile_definitions(${_LIBUINAME}
	PRIVATE ${_LIBUI_DEFS})
target_compile_options(${_LIBUINAME}
	PUBLIC ${_COMMON_CFLAGS}
	PRIVATE ${_LIBUI_CFLAGS})
# TODO link directories?
target_link_libraries(${_LIBUINAME}
	${_LIBUI_LINKMODE} ${_LIBUI_LIBS})
# on Windows the linker for static libraries is different; don't give it the flags
if(BUILD_SHARED_LIBS)
	_target_link_options_private(${_LIBUINAME}
		_COMMON_LDFLAGS
		_LIBUI_LDFLAGS)
endif()
if(NOT BUILD_SHARED_LIBS)
	_handle_static()
	target_compile_definitions(${_LIBUINAME}
			PRIVATE _UI_STATIC)
endif()
# don't put this in the OS CMakeLists.txt to be safe about quoting
if(WIN32)
	target_compile_definitions(${_LIBUINAME}
		PRIVATE "_UI_EXTERN=__declspec(dllexport) extern")
else()
	target_compile_definitions(${_LIBUINAME}
		PRIVATE "_UI_EXTERN=__attribute__((visibility(\"default\"))) extern")
endif()

macro(_add_exec _name)
	add_executable(${_name}
		WIN32 EXCLUDE_FROM_ALL
		${ARGN})
	target_link_libraries(${_name} libui)
	_target_link_options_private(${_name}
		_COMMON_LDFLAGS)
endmacro()
add_subdirectory("test")

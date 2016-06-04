# 3 june 2016
cmake_minimum_required(VERSION 2.8.11)

# TODOs:
# - Darwin, Unix: static linking temporary target makes PUBLIC properties not propagate

# the docs say we need to set this up prior to project()
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.8")

project(libui LANGUAGES C CXX)
option(BUILD_SHARED_LIBS "Whether to build libui as a shared library or a static library" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/out")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/out")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/out")
set(CMAKE_PDB_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/out")

if(APPLE)
	set(_OSNAME darwin)
	set(_HASVERSION TRUE)
	set(_VERSION "A")

	# always use our rpath
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	# the / is required by some older versions of OS X
	set(CMAKE_INSTALL_RPATH "@executable_path/")
	set(CMAKE_MACOSX_RPATH TRUE)
elseif(WIN32)
	set(_OSNAME windows)

	# and don't include the default libraries with ANY of the builds
	# note the CACHE FORCE stuff is required here
	set(CMAKE_C_STANDARD_LIBRARIES CACHE STRING "" FORCE)
	set(CMAKE_CXX_STANDARD_LIBRARIES CACHE STRING "" FORCE)
else()
	set(_OSNAME unix)
	set(_HASVERSION TRUE)
	set(_VERSION "0")

	# always use our rpath
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	set(CMAKE_INSTALL_RPATH "\$ORIGIN")
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
	set(_COMMON_CFLAGS
		-Wall -Wextra -pedantic
		-Wno-unused-parameter
		-Wno-switch
		-fvisibility=hidden
	)
	# don't use C_VERSION or CXX_VERSION because they use GNU standards
	string(APPEND CMAKE_C_FLAGS " --std=c99")
	string(APPEND CMAKE_CXX_FLAGS " --std=c++11")

endif()

# problem:
# - target_link_libraries() only supports - for flags
# - but cmake only doesn't generate the manifest if the flag has a /
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
# cmake produces this for us by default but only for shared libraries
target_compile_definitions(${_LIBUINAME}
	PRIVATE libui_EXPORTS)
target_compile_options(${_LIBUINAME}
	PUBLIC ${_COMMON_CFLAGS}
	PRIVATE ${_LIBUI_CFLAGS})
# TODO link directories?
# because we need 2.8.11 for CentOS, we can't use target_link_libraries(INTERFACE) for static executables :(
if(BUILD_SHARED_LIBS)
	target_link_libraries(${_LIBUINAME}
		PRIVATE ${_LIBUI_LIBS})
endif()
# on Windows the linker for static libraries is different; don't give it the flags
if(BUILD_SHARED_LIBS)
	_target_link_options_private(${_LIBUINAME}
		_COMMON_LDFLAGS
		_LIBUI_LDFLAGS)
endif()
if(NOT BUILD_SHARED_LIBS)
	_handle_static()
	# TODO this really should be PRIVATE but I haven't fully figured this out
	target_compile_definitions(${_LIBUINAME}
			PUBLIC _UI_STATIC)
endif()
if(NOT MSVC)
	# on non-MSVC compilers cmake adds an extra lib-
	# note that we apply this to libui, not to any intermediates
	set_target_properties(libui PROPERTIES
		OUTPUT_NAME ui)
endif()
if(BUILD_SHARED_LIBS)
	if(_HASVERSION)
		set_target_properties(${_LIBUINAME} PROPERTIES
			SOVERSION "${_VERSION}")
	endif()
endif()

macro(_add_exec _name)
	add_executable(${_name}
		WIN32 EXCLUDE_FROM_ALL
		${ARGN})
	target_link_libraries(${_name} libui ${_LIBUI_STATIC_RES})
	_target_link_options_private(${_name}
		_COMMON_LDFLAGS)
	# make shared-linked executables PIC too
	if(BUILD_SHARED_LIBS)
		set_property(TARGET ${_name} PROPERTY
			POSITION_INDEPENDENT_CODE True)
	endif()
	# because we need 2.8.11 for CentOS, we can't use target_link_libraries(PUBLIC) for static executables :(
	if(NOT BUILD_SHARED_LIBS)
		target_link_libraries(${_name} ${_LIBUI_LIBS})
	endif()
endmacro()
add_subdirectory("test")
add_subdirectory("examples")

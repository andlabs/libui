# 30 may 2016
cmake_minimum_required(VERSION 2.8.12)
project(libui)

set(_NOSHARED FALSE)
if(WIN32)
	if(NOT MSVC)
		set(_NOSHARED TRUE)
	endif()
endif()
macro(nosharedmingw _target)
	add_custom_target(${_target}
		COMMAND exit 1
		COMMENT "Sorry, libui for Windows cannot be built as a DLL with MinGW. You will need to either build as a static library or build with MSVC.")
endmacro(nosharedmingw)

if(APPLE)
	set(_OSDIR darwin)
	set(_OSSRCEXT m)
	set(_SETVERSION TRUE)
	set(_VERSION "A")

	set(_COMMON_CFLAGS
		"${_COMMON_CFLAGS} -mmacosx-version-min=10.8 -DMACOSX_DEPLOYMENT_TARGET=10.8")
	set(_COMMON_LDFLAGS
		"${_COMMON_LDFLAGS} -mmacosx-version-min=10.8")

	set(_LIBUI_LDFLAGS
		"${_LIBUI_LDFLAGS} -lobjc -framework Foundation -framework AppKit")

	# always use our rpath
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	# the / is required by some older versions of OS X
	set(CMAKE_INSTALL_RPATH "@executable_path/")
	set(CMAKE_MACOSX_RPATH TRUE)
elseif(WIN32)
	set(_OSDIR windows)
	set(_OSSRCEXT cpp)
	set(_SETVERSION FALSE)
else()
	set(_OSDIR unix)
	set(_OSSRCEXT c)
	set(_SETVERSION TRUE)
	set(_VERSION "0")

	# always use our rpath
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()

if(MSVC)
	# TODO
else()
	# don't use C_VERSION or CXX_VERSION because they use GNU standards
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --std=c99")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")

	set(_COMMON_CFLAGS
		"${_COMMON_CFLAGS} -Wall -Wextra -pedantic")
	set(_COMMON_CFLAGS
		"${_COMMON_CFLAGS} -Wno-unused-parameter")
	set(_COMMON_CFLAGS
		"${_COMMON_CFLAGS} -Wno-switch")
	set(_COMMON_LDFLAGS
		"${_COMMON_LDFLAGS}")
	if(NOT WIN32)
		set(_PICFLAG "-fPIC")
	endif()

	set(_LIBUI_CFLAGS
		"${_LIBUI_CFLAGS} -D_UI_EXTERN='__attribute__((visibility(\"default\"))) extern' -fvisibility=hidden")
	set(_LIBUI_LDFLAGS
		"${_LIBUI_LDFLAGS} -fvisibility=hidden")
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${_COMMON_CFLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_COMMON_CFLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${_COMMON_LDFLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${_COMMON_LDFLAGS} ${_PICFLAG}")
# don't set ${CMAKE_STATIC_LINKER_FLAGS}; that's just ar

include_directories(. common ${_OSDIR})
file(GLOB SOURCES
	common/*.c
	${_OSDIR}/*.${_OSSRCEXT})

macro(libui _name _mode)
	add_library(${_name} ${_mode} ${SOURCES})
	set_target_properties(${_name} PROPERTIES
		OUTPUT_NAME ui)
	if("${_mode}" STREQUAL "SHARED")
		# only put version number on shared build
		set_target_properties(${_name} PROPERTIES
			SOVERSION ${_VERSION})
	else()
		# don't build libui-static by default
		set_target_properties(${_name} PROPERTIES
			EXCLUDE_FROM_ALL 1)
	endif()
	set_target_properties(${_name} PROPERTIES
		COMPILE_FLAGS "${_LIBUI_CFLAGS}")
	set_target_properties(${_name} PROPERTIES
		LINK_FLAGS "${_PICFLAG} ${_LIBUI_LDFLAGS}")
endmacro()

libui(libui SHARED ${_SETVERSION} FALSE)
if(${_NOSHARED})
	nosharedmingw(libui-static)
else()
	libui(libui-static STATIC FALSE TRUE)
endif()

include_directories(test)
file(GLOB TESTSOURCES test/*.c)

macro(executable_base _name _outname _libui _static)
	add_executable(${_name} ${XSRC})
	set_target_properties(${_name} PROPERTIES
		OUTPUT_NAME ${_outname}
		EXCLUDE_FROM_ALL 1)
	target_link_libraries(${_name} ${_libui})
	# be sure to include libui libraries in the output
	if(${_static})
		set_target_properties(${_name} PROPERTIES
			LINK_FLAGS "${_LIBUI_LDFLAGS}")
	else()
		set_target_properties(${_name} PROPERTIES
			LINK_FLAGS "${_PICFLAG}")
	endif()
endmacro()

macro(executable _name _outname _dir)
	include_directories(${_dir})
	file(GLOB XSRC ${_dir}/*.c ${_dir}/*.cpp)
	executable_base(${_name} ${_outname} libui FALSE)
	if(${_NOSHARED})
		nosharedmingw(${_name}-static)
	else()
		executable_base(${_name}-static ${_outname} libui-static TRUE)
	endif()
	set(XSRC)
endmacro()

executable(tester test test)
executable(controlgallery controlgallery examples/controlgallery)
executable(histogram histogram examples/histogram)
executable(cpp-multithread cpp-multithread examples/cpp-multithread)

add_custom_target(examples
	DEPENDS
		controlgallery
		histogram
		cpp-multithread)
add_custom_target(examples-static
	DEPENDS
		controlgallery-static
		histogram-static
		cpp-multithread-static)

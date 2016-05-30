# 30 may 2016
cmake_minimum_required(VERSION 2.8.12)
project(libui)

if(APPLE)
	set(_OSDIR darwin)
	set(_OSSRCEXT m)
	set(_SETVERSION TRUE)
	set(_VERSION "A")

	# always use our rpath
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	# the / is required by some older versions of OS X
	set(CMAKE_INSTALL_RPATH "@executable_path/")
	set(CMAKE_MACOSX_RPATH TRUE)
elseif(WINDOWS)
	set(_OSDIR windows)
	set(_OSSRCEXT cpp)
	set(_SETVERSION FALSE)
else(APPLE)
	set(_OSDIR unix)
	set(_OSSRCEXT c)
	set(_SETVERSION TRUE)
	set(_VERSION "0")

	# always use our rpath
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif(APPLE)

include_directories(. common ${_OSDIR} test)
file(GLOB SOURCES
	common/*.c
	${_OSDIR}/*.${_OSSRCEXT})

macro(libui _name _mode _setver _exclude)
	add_library(${_name} ${_mode} ${SOURCES})
	set_target_properties(${_name} PROPERTIES
		OUTPUT_NAME ui)
	if(${_setver})
		set_target_properties(${_name} PROPERTIES
			SOVERSION ${_VERSION})
	endif(${_setver})
	# omit libui-shared from default builds
	if(${_exclude})
		set_target_properties(${_name} PROPERTIES
			EXCLUDE_FROM_ALL 1)
	endif(${_exclude})
	target_link_libraries(${_name} "-framework Foundation -framework AppKit")
endmacro(libui)

libui(libui SHARED ${_SETVERSION} FALSE)
libui(libui-static STATIC FALSE TRUE)

include_directories(test)
file(GLOB TESTSOURCES test/*.c)

macro(test _name _libui)
	add_executable(${_name} ${TESTSOURCES})
	set_target_properties(${_name} PROPERTIES
		OUTPUT_NAME test
		EXCLUDE_FROM_ALL 1)
	target_link_libraries(${_name} ${_libui})
endmacro(test)

test(tester libui)
test(tester-static libui-static)

# 30 may 2016
cmake_minimum_required(VERSION 2.8.12)
project(libui)

option(examples "Build the example projects" OFF)
option(test "Build the test project" OFF)

set(_OSDIR unix)
set(_OSSRCEXT c)
set(_OUTNAME libui.so.0)
if(APPLE)
	set(_OSDIR darwin)
	set(_OSSRCEXT m)
	set(_OUTNAME libui.A.dylib)

	# always use our rpath
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	# the / is required by some older versions of OS X
	set(CMAKE_INSTALL_RPATH "@executable_path/")
	set(CMAKE_MACOSX_RPATH TRUE)
elseif(WINDOWS)
	set(_OSDIR windows)
	set(_OSSRCEXT cpp)
	set(_OUTNAME libui.dll)
endif(APPLE)

include_directories(. common ${_OSDIR} test)

file(GLOB SOURCES
	common/*.c
	${_OSDIR}/*.${_OSSRCEXT})

add_library(libui SHARED ${SOURCES})
set_target_properties(libui PROPERTIES OUTPUT_NAME ${_OUTNAME})
target_link_libraries(libui "-framework Foundation -framework AppKit")

if(test)
	file(GLOB TESTSOURCES test/*.c)
	add_executable(tester ${TESTSOURCES})
	set_target_properties(tester PROPERTIES OUTPUT_NAME test)
	target_link_libraries(tester libui)
endif(test)

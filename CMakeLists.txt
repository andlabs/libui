# 30 may 2016
cmake_minimum_required(VERSION 2.8.12)

# set up our configurations
set(CMAKE_CONFIGURATION_TYPES Debug Static Release ReleaseStatic)
# we load the variables after calling project()
# default to Debug if no configuration specified
if(NOT CMAKE_BUILD_TYPE)
	# the CACHE FORCE is necessary for this to work properly
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type; one of: Debug Release Static ReleaseStatic" FORCE)
endif()
# and save whether this is shared in a variable
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(_SHARED TRUE)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
	set(_SHARED TRUE)
endif()
if(WIN32)
	if(NOT MSVC)
		if(_SHARED)
			message(FATAL_ERROR
				"Sorry, libui for Windows cannot be built as a DLL with MinGW. You will need to either build as a static library or build with MSVC.")
		endif()
	endif()
endif()


# and we need to set this up prior to project() too
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.8")

project(libui)

# now that we called project(), load our config variables
macro(cfgcopy _prefix)
	set(${_prefix}_STATIC "${${_prefix}_DEBUG}")
	set(${_prefix}_RELEASESTATIC "${${_prefix}_RELEASE}")
endmacro()
cfgcopy(CMAKE_C_FLAGS)
cfgcopy(CMAKE_CXX_FLAGS)
cfgcopy(CMAKE_SHARED_LINKER_FLAGS)
cfgcopy(CMAKE_STATIC_LINKER_FLAGS)
cfgcopy(CMAKE_EXE_LINKER_FLAGS)

macro(append _var _val)
	set(${_var} "${${_var}} ${_val}")
endmacro()
macro(append2 _var1 _var2 _val)
	append(${_var1} "${_val}")
	append(${_var2} "${_val}")
endmacro()

if(APPLE)
	set(_OSDIR darwin)
	set(_OSSRCEXT m)
	set(_SETVERSION TRUE)
	set(_VERSION "A")

	set(_PLATFORM_LIBS
		"-lobjc -framework Foundation -framework AppKit")

	# always use our rpath
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	# the / is required by some older versions of OS X
	set(CMAKE_INSTALL_RPATH "@executable_path/")
	set(CMAKE_MACOSX_RPATH TRUE)
elseif(WIN32)
	set(_OSDIR windows)
	set(_OSSRCEXT cpp)
	set(_SETVERSION FALSE)

	# note that usp10 comes before gdi32
	# TODO prune this list
	set(_PLATFORM_LIBS_BASE
		user32 kernel32 usp10 gdi32 comctl32 uxtheme msimg32 comdlg32 d2d1 dwrite ole32 oleaut32 oleacc uuid)
	if(MSVC)
		string(REPLACE ";" ".lib " _PLATFORM_LIBS "${_PLATFORM_LIBS_BASE}")
		set(_PLATFORM_LIBS "${_PLATFORM_LIBS}.lib")
	else()
		string(REPLACE ";" " -l" _PLATFORM_LIBS "${_PLATFORM_LIBS_BASE}")
		set(_PLATFORM_LIBS "-l${_PLATFORM_LIBS}")
	endif()
else()
	set(_OSDIR unix)
	set(_OSSRCEXT c)
	set(_SETVERSION TRUE)
	set(_VERSION "0")

	pkg_check_modules(GTK REQUIRED gtk+-3.0)
	set(_LIBUI_CFLAGS "${GTK_CFLAGS}")
	set(_PLATFORM_LIBS "${GTK_LDFLAGS} -lm -ldl")

	# always use our rpath
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()

if(MSVC)
	# TODO
else()
	append2(CMAKE_C_FLAGS CMAKE_CXX_FLAGS
		" -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-switch")
	# don't use C_VERSION or CXX_VERSION because they use GNU standards
	append(CMAKE_C_FLAGS " --std=c99")
	append(CMAKE_CXX_FLAGS " --std=c++11")

	if(NOT WIN32)
		append(CMAKE_C_FLAGS_DEBUG " -fPIC")
		append(CMAKE_CXX_FLAGS_DEBUG "  -fPIC")
		append(CMAKE_SHARED_LINKER_FLAGS_DEBUG " -fPIC")
		append(CMAKE_EXE_LINKER_FLAGS_DEBUG " -fPIC")
		append(CMAKE_C_FLAGS_RELEASE " -fPIC")
		append(CMAKE_CXX_FLAGS_RELEASE " -fPIC")
		append(CMAKE_SHARED_LINKER_FLAGS_RELEASE " -fPIC")
		append(CMAKE_EXE_LINKER_FLAGS_RELEASE " -fPIC")
	endif()

	append(_LIBUI_CFLAGS
		" -D_UI_EXTERN='__attribute__((visibility(\"default\"))) extern' -fvisibility=hidden ${_PLATFORM_CFLAGS}")

	append(CMAKE_SHARED_LINKER_FLAGS " -fvisibility=hidden")
	# don't amend CMAKE_STATIC_LINKER_FLAGS; that's for ar
endif()

# and add the platform libraries to the three places that need it: shared library links and the two static executable links
append(CMAKE_SHARED_LINKER_FLAGS " ${_PLATFORM_LIBS}")
append2(CMAKE_EXE_LINKER_FLAGS_STATIC CMAKE_EXE_LINKER_FLAGS_RUNTIMESTATIC
	" ${_PLATFORM_LIBS}")

add_subdirectory("common")
add_subdirectory("${_OSDIR}")
if(_SHARED)
	add_library(libui SHARED
		$<TARGET_OBJECTS:libui-common>
		$<TARGET_OBJECTS:libui-${_OSDIR}>
	)
	if(_SETVERSION)
		set_target_properties(libui PROPERTIES
			SOVERSION "${_VERSION}")
	endif()
else()
	_add_static(libui
		$<TARGET_OBJECTS:libui-common>
		$<TARGET_OBJECTS:libui-${_OSDIR}>
	)
endif()
set_target_properties(libui PROPERTIES
	OUTPUT_NAME ui)

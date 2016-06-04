# 30 may 2016
cmake_minimum_required(VERSION 2.8.12)

# TODOs
# - ensure all set_target_properties() calls are approrpiately set, APPEND, and APPEND_STRING

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

# and we need to set this up prior to project() too
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.8")

project(libui)

# TODO can this be above the project()?
if(WIN32)
	if(NOT MSVC)
		if(_SHARED)
			message(FATAL_ERROR
				"Sorry, libui for Windows cannot be built as a DLL with MinGW. You will need to either build as a static library or build with MSVC.")
		endif()
	endif()
endif()

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
	set(_SETVERSION TRUE)
	set(_VERSION "A")

	set(_PLATFORM_LIBS
		-lobjc "-framework Foundation" "-framework AppKit"
	)

	# always use our rpath
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	# the / is required by some older versions of OS X
	set(CMAKE_INSTALL_RPATH "@executable_path/")
	set(CMAKE_MACOSX_RPATH TRUE)
elseif(WIN32)
	set(_OSDIR windows)
	set(_SETVERSION FALSE)

	# note that usp10 comes before gdi32
	# TODO prune this list
	set(_PLATFORM_LIBS
		user32 kernel32 usp10 gdi32 comctl32 uxtheme msimg32 comdlg32 d2d1 dwrite ole32 oleaut32 oleacc uuid
	)
	# and don't include the default libraries
	# note the CACHE FORCE stuff is required here
	set(CMAKE_C_STANDARD_LIBRARIES CACHE STRING "" FORCE)
	set(CMAKE_CXX_STANDARD_LIBRARIES CACHE STRING "" FORCE)

	set(_RESOURCES_RC resources.rc)
else()
	set(_OSDIR unix)
	set(_SETVERSION TRUE)
	set(_VERSION "0")

	find_package(PkgConfig REQUIRED)
	pkg_check_modules(GTK REQUIRED gtk+-3.0)
	string(REPLACE ";" " " _LIBUI_CFLAGS "${GTK_CFLAGS}")
	set(_PLATFORM_LIBS "${GTK_LDFLAGS} -lm -ldl")

	# always use our rpath
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	set(CMAKE_INSTALL_RPATH "\$ORIGIN")
endif()

if(MSVC)
	append2(CMAKE_C_FLAGS CMAKE_CXX_FLAGS
		"/W4 /wd4100 /bigobj /RTC1 /RTCs /RTCu")

	# shut the compiler up in some cases
	# LONGTERM still needed?
	append(CMAKE_CXX_FLAGS " /EHsc")

	append(_LIBUI_CFLAGS " ${_PLATFORM_CFLAGS}")

	# note the /MANIFEST:NO (which must be / and uppercase); thanks FraGag (https://github.com/andlabs/libui/issues/93#issuecomment-223183436)
	# also don't apply to CMAKE_STATIC_LINKER_FLAGS; those are passed to a different tool that doesn't support them
	append2(CMAKE_SHARED_LINKER_FLAGS CMAKE_EXE_LINKER_FLAGS
		" /LARGEADDRESSAWARE /INCREMENTAL:NO /MANIFEST:NO")
else()
	append2(CMAKE_C_FLAGS CMAKE_CXX_FLAGS
		" -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-switch")

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

	if(WIN32)
		append(_LIBUI_CFLAGS " ${_PLATFORM_CFLAGS}")
	else()
		append(_LIBUI_CFLAGS " -fvisibility=hidden ${_PLATFORM_CFLAGS}")
	endif()

	append(CMAKE_SHARED_LINKER_FLAGS " -fvisibility=hidden")
	# don't amend CMAKE_STATIC_LINKER_FLAGS; that's for ar
endif()

if(NOT _SHARED)
	append(_LIBUI_CFLAGS " -D_UI_STATIC")
endif()

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
	target_link_libraries(libui PRIVATE ${_PLATFORM_LIBS})
else()
	_add_static(libui
		$<TARGET_OBJECTS:libui-common>
		$<TARGET_OBJECTS:libui-${_OSDIR}>
	)
endif()
# non-Windows platforms add an extra lib- at the beginning
if(NOT WIN32)
	set_target_properties(libui PROPERTIES
		OUTPUT_NAME ui)
endif()
# let cmake handle quoting and escaping for us
if(WIN32)
	target_compile_definitions(libui
		PRIVATE "_UI_EXTERN=__declspec(dllexport) extern")
else()
	target_compile_definitions(libui
		PRIVATE "_UI_EXTERN=__attribute__((visibility(\"default\"))) extern")
endif()

add_subdirectory("test")

add_subdirectory("examples")

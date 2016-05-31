# 30 may 2016
cmake_minimum_required(VERSION 2.8.12)

# set up our configurations
set(CMAKE_CONFIGURATION_TYPES Debug Static Release ReleaseStatic)
# we load the variables after calling project()

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

macro(append _var _val)
	set(${_var} "${${_var}} ${_val}")
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
	set(_PLATFORM_CFLAGS "${GTK_CFLAGS}")
	set(_PLATFORM_LIBS "${GTK_LDFLAGS} -lm -ldl")

	# always use our rpath
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()

if(MSVC)
	# TODO
else()
	set(_COMMON_FLAGS_BASE
		"-Wall -Wextra -pedantic -Wno-unused-parameter -Wno-switch")
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

	set(_LIBUI_CFLAGS
		"-D_UI_EXTERN='__attribute__((visibility(\"default\"))) extern' -fvisibility=hidden ${_PLATFORM_CFLAGS}")
	set(_LIBUI_LDFLAGS
		"-fvisibility=hidden")
endif()

set(_COMMON_C_FLAGS
	"${CMAKE_C_FLAGS} ${_COMMON_C_FLAGS}")
set(_COMMON_CXX_FLAGS
	"${CMAKE_CXX_FLAGS} ${_COMMON_C_FLAGS}")

set(_LIBUI_SHARED_CFLAGS
	"${_COMMON_CFLAGS} ${_LIBUI_CFLAGS} ${_PICFLAG}")
set(_LIBUI_STATIC_CFLAGS
	"${_COMMON_CFLAGS} ${_LIBUI_CFLAGS}")
set(_LIBUI_SHARED_CXXFLAGS
	"${_COMMON_CXXFLAGS} ${_LIBUI_CFLAGS} ${_PICFLAG}")
set(_LIBUI_STATIC_CXXFLAGS
	"${_COMMON_CXXFLAGS} ${_LIBUI_CFLAGS}")
set(_LIBUI_SHARED_LDFLAGS
	"${CMAKE_SHARED_LINKER_FLAGS} ${_COMMON_LDFLAGS} ${_LIBUI_LDFLAGS} ${_PLATFORM_LDFLAGS} ${_PICFLAG}")
set(_LIBUI_STATIC_LDFLAGS
	"${_COMMON_LDFLAGS} ${_LIBUI_LDFLAGS}")

set(_EXE_SHARED_CFLAGS
	"${_COMMON_CFLAGS} ${_PICFLAG}")
set(_EXE_STATIC_CFLAGS
	"${_COMMON_CFLAGS}")
set(_EXE_SHARED_CXXFLAGS
	"${_COMMON_CXXFLAGS} ${_PICFLAG}")
set(_EXE_STATIC_CXXFLAGS
	"${_COMMON_CXXFLAGS}")
set(_EXE_SHARED_LDFLAGS
	"${CMAKE_SHARED_LINKER_FLAGS} ${_COMMON_LDFLAGS} ${_PICFLAG}")
set(_EXE_STATIC_LDFLAGS
	"${_COMMON_LDFLAGS} ${_PLATFORM_LDFLAGS}")

set(_LIBUI_HEADERS . common ${_OSDIR})
file(GLOB _LIBUI_SOURCES
	common/*.c
	${_OSDIR}/*.${_OSSRCEXT})

macro(_begin_shared _cflags _cxxflags _ldflags)
	set(CMAKE_C_FLAGS "${_cflags}")
	set(CMAKE_CXX_FLAGS "${_cxxflags}")
	set(CMAKE_SHARED_LINKER_FLAGS "${_ldflags}")
endmacro()

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

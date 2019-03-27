# 3 june 2016

	set(CMAKE_INSTALL_RPATH "\$ORIGIN")

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
add_library(libui ${_LIBUI_SOURCES})
target_include_directories(libui
	PUBLIC .
	PRIVATE ${_LIBUI_INCLUEDIRS})
target_compile_definitions(libui
	PRIVATE ${_LIBUI_DEFS})
# cmake produces this for us by default but only for shared libraries
target_compile_definitions(libui
	PRIVATE libui_EXPORTS)
target_compile_options(libui
	PUBLIC ${_COMMON_CFLAGS}
	PRIVATE ${_LIBUI_CFLAGS})
# TODO link directories?
if(BUILD_SHARED_LIBS)
	target_link_libraries(libui
		PRIVATE ${_LIBUI_LIBS})
endif()
# TODO INTERFACE libs don't inherit to grandhcildren?
# on Windows the linker for static libraries is different; don't give it the flags
if(BUILD_SHARED_LIBS)
	_target_link_options_private(libui
		_COMMON_LDFLAGS
		_LIBUI_LDFLAGS)
endif()
if(NOT BUILD_SHARED_LIBS)
	# TODO figure out a way to tell libui that it's static
	target_compile_definitions(libui
		PUBLIC _UI_STATIC)
endif()
if(NOT MSVC)
	# on non-MSVC compilers cmake adds an extra lib-
	# note that we apply this to libui, not to any intermediates
	set_target_properties(libui PROPERTIES
		OUTPUT_NAME ui)

	# flags for warning on undefined symbols
	# TODO figure out why FreeBSD follows linked libraries here
	# TODO figure out MSVC equivalents
	if(BUILD_SHARED_LIBS)
		if(NOT (${CMAKE_SYSTEM_NAME} STREQUAL FreeBSD))
			# on OS X we don't need to do this; Apple's linker warns about undefined symbols in -shared builds!
			if(NOT APPLE)
				target_link_libraries(libui
					PRIVATE -Wl,--no-undefined -Wl,--no-allow-shlib-undefined
				)
			endif()
		endif()
	endif()
endif()
if(BUILD_SHARED_LIBS)
	if(_HASVERSION)
		set_target_properties(libui PROPERTIES
			SOVERSION "${_VERSION}")
	endif()
endif()
# TODO why is this not a default?!
set_property(TARGET libui PROPERTY
	POSITION_INDEPENDENT_CODE True)

macro(_add_exec _name)
	# TODOTODO re-add WIN32 when merging back
	add_executable(${_name}
		EXCLUDE_FROM_ALL
		${ARGN})
	target_link_libraries(${_name} libui)
	_target_link_options_private(${_name}
		_COMMON_LDFLAGS)
	# TODO does this propagate?
	set_property(TARGET ${_name} PROPERTY
		POSITION_INDEPENDENT_CODE True)
	# TODO see above about INTERFACE
	if(NOT BUILD_SHARED_LIBS)
		target_link_libraries(${_name}
			${_LIBUI_LIBS})
	endif()

	# TODOfor some reason these don't propagate
	if(NOT WIN32)
		target_include_directories(${_name}
			PUBLIC .)
		target_compile_options(${_name}
			PUBLIC ${_COMMON_CFLAGS})
	endif()
endmacro()
add_subdirectory("test")
add_subdirectory("examples")

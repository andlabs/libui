# 3 june 2016

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

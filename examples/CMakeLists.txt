# 3 june 2016

if(WIN32)
	set(_EXAMPLE_RESOURCES_RC resources.rc)
endif()

macro(_add_example _name)
	_add_exec(${_name} ${ARGN})
	# because Microsoft's toolchain is dumb
	if(MSVC)
		set_property(TARGET ${_name} APPEND_STRING PROPERTY
			LINK_FLAGS " /ENTRY:mainCRTStartup")
	endif()
endmacro()

_add_example(controlgallery
	controlgallery/main.c
	${_EXAMPLE_RESOURCES_RC}
)

_add_example(histogram
	histogram/main.c
	${_EXAMPLE_RESOURCES_RC}
)

_add_example(cpp-multithread
	cpp-multithread/main.cpp
	${_EXAMPLE_RESOURCES_RC}
)
if(NOT WIN32)
	target_link_libraries(cpp-multithread pthread)
endif()

add_custom_target(examples
	DEPENDS
		controlgallery
		histogram
		cpp-multithread)

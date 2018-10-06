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
if(APPLE)
	# since we use a deployment target of 10.8, the non-C++11-compliant libstdc++ is chosen by default; we need C++11
	# see issue #302 for more details
	target_compile_options(cpp-multithread PRIVATE --stdlib=libc++)
	target_link_libraries(cpp-multithread --stdlib=libc++)
endif()

_add_example(drawtext
	drawtext/main.c
	${_EXAMPLE_RESOURCES_RC}
)

_add_example(timer
        timer/main.c
        ${_EXAMPLE_RESOURCES_RC}
)

_add_example(datetime
	datetime/main.c
	${_EXAMPLE_RESOURCES_RC}
)

_add_example(opengl
	opengl/main.c opengl/glew/glew.c
	${_EXAMPLE_RESOURCES_RC}
)
target_compile_definitions(opengl PRIVATE GLEW_STATIC GLEW_NO_GLU)
find_package(OpenGL REQUIRED)
target_include_directories(opengl PRIVATE ${OPENGL_INCLUDE_DIR})
target_link_libraries(opengl ${OPENGL_LIBRARIES} m)

add_custom_target(examples
	DEPENDS
		controlgallery
		histogram
		cpp-multithread
		drawtext
		timer
		datetime
		opengl)

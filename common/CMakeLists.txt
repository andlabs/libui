# 31 may 2016

include_directories(.. .)

add_library(libui-common OBJECT
	areaevents.c
	control.c
	debug.c
	matrix.c
	shouldquit.c
	userbugs.c
)
set_target_properties(libui-common PROPERTIES
	COMPILE_FLAGS "${_LIBUI_CFLAGS}"
)

# 1 june 2016

include_directories(.. . ../common)

add_library(libui-unix OBJECT
	alloc.c
	area.c
	box.c
	button.c
	checkbox.c
	child.c
	colorbutton.c
	combobox.c
	control.c
	datetimepicker.c
	debug.c
	draw.c
	drawmatrix.c
	drawpath.c
	drawtext.c
	editablecombo.c
	entry.c
	fontbutton.c
	graphemes.c
	group.c
	label.c
	main.c
	menu.c
	multilineentry.c
	progressbar.c
	radiobuttons.c
	separator.c
	slider.c
	spinbox.c
	stddialogs.c
	tab.c
	text.c
	util.c
	window.c
)
set_target_properties(libui-unix PROPERTIES
	COMPILE_FLAGS "${_LIBUI_CFLAGS}"
)

# thanks to Mr-Hide in irc.freenode.net/#cmake
macro(_add_static _name)
	add_library(${_name}-temporary STATIC "${ARGN}")
	set_target_properties(${_name}-temporary PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
	set(_aname $<TARGET_FILE:${_name}-temporary>)
	set(_oname ${_name}-combined.o)
	add_custom_command(
		OUTPUT ${_oname}
		COMMAND
			ld -r --whole-archive ${_aname} -o ${_oname}
		COMMAND
			objcopy --localize-hidden ${_oname}
		COMMENT "Removing hidden symbols")
	add_library(${_name} STATIC ${_oname})
	# otherwise cmake won't know which linker to use
	set_target_properties(${_name} PROPERTIES
		LINKER_LANGUAGE C)
	set(_aname)
	set(_oname)
endmacro()

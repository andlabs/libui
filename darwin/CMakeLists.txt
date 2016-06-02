# 31 may 2016

include_directories(.. . ../common)

add_library(libui-darwin OBJECT
	alloc.m
	area.m
	areaevents.m
	autolayout.m
	box.m
	button.m
	checkbox.m
	colorbutton.m
	combobox.m
	control.m
	datetimepicker.m
	debug.m
	draw.m
	drawtext.m
	editablecombo.m
	entry.m
	fontbutton.m
	group.m
	label.m
	main.m
	map.m
	menu.m
	multilineentry.m
	progressbar.m
	radiobuttons.m
	scrollview.m
	separator.m
	slider.m
	spinbox.m
	stddialogs.m
	tab.m
	text.m
	util.m
	window.m
)
set_target_properties(libui-darwin PROPERTIES
	COMPILE_FLAGS "${_LIBUI_CFLAGS}"
)

# thanks to Mr-Hide in irc.freenode.net/#cmake
macro(_add_static _name)
	add_library(${_name}-temporary STATIC "${ARGN}")
	set_target_properties(${_name}-temporary PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
	set(_aname $<TARGET_FILE:${_name}-temporary>)
	set(_lname ${_name}-combined.list)
	set(_oname ${_name}-combined.o)
	add_custom_command(
		OUTPUT ${_oname}
		COMMAND
			nm -m ${_aname} | sed -E -n "'s/^[0-9a-f]* \\([A-Z_]+,[a-z_]+\\) external //p'" > ${_lname}
		COMMAND
			ld -exported_symbols_list ${_lname} -r -all_load ${_aname} -o ${_oname}
		COMMENT "Removing hidden symbols")
	add_library(${_name} STATIC ${_oname})
	# otherwise cmake won't know which linker to use
	set_target_properties(${_name} PROPERTIES
		LINKER_LANGUAGE C)
	set(_aname)
	set(_lname)
	set(_oname)
endmacro()

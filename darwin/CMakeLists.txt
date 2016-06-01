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

macro(_add_static _name)
	add_library(${_name} STATIC "${ARGN}")
	file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/sharedhidden)
	add_custom_command(
		TARGET ${_name} POST_BUILD
		COMMAND
			${CMAKE_AR} x $<TARGET_FILE:${_name}>
		COMMAND
			nm -m *.o | sed -E -n "'s/^[0-9a-f]* \\([A-Z_]+,[a-z_]+\\) external //p'" > ${_name}.lst
		COMMAND
			ld -exported_symbols_list ${_name}.lst -r *.o -o ../_combined_${_name}.o
		COMMAND
			rm $<TARGET_FILE:${_name}>
		COMMAND
			${CMAKE_AR} rcs $<TARGET_FILE:${_name}> ../_combined_${_name}.o
		WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/sharedhidden
		COMMENT "Removing hidden symbols")
endmacro()

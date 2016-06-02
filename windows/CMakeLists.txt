# 1 june 2016

include_directories(.. . ../common)

add_library(libui-windows OBJECT
	alloc.cpp
	area.cpp
	areadraw.cpp
	areaevents.cpp
	areascroll.cpp
	areautil.cpp
	box.cpp
	button.cpp
	checkbox.cpp
	colorbutton.cpp
	colordialog.cpp
	combobox.cpp
	container.cpp
	control.cpp
	d2dscratch.cpp
	datetimepicker.cpp
	debug.cpp
	draw.cpp
	drawmatrix.cpp
	drawpath.cpp
	drawtext.cpp
	dwrite.cpp
	editablecombo.cpp
	entry.cpp
	events.cpp
	fontbutton.cpp
	fontdialog.cpp
	graphemes.cpp
	group.cpp
	init.cpp
	label.cpp
	main.cpp
	menu.cpp
	multilineentry.cpp
	parent.cpp
	progressbar.cpp
	radiobuttons.cpp
	separator.cpp
	sizing.cpp
	slider.cpp
	spinbox.cpp
	stddialogs.cpp
	tab.cpp
	tabpage.cpp
	text.cpp
	utf16.cpp
	utilwin.cpp
	window.cpp
	winpublic.cpp
	winutil.cpp
	resources.rc
)
set_target_properties(libui-windows PROPERTIES
	COMPILE_FLAGS "${_LIBUI_CFLAGS}"
)

macro(_add_static _name)
	add_library(${_name} STATIC "${ARGN}")
endmacro()

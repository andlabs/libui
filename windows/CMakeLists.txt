# 3 june 2016

list(APPEND _LIBUI_SOURCES
	windows/alloc.cpp
	windows/area.cpp
	windows/areadraw.cpp
	windows/areaevents.cpp
	windows/areascroll.cpp
	windows/areautil.cpp
	windows/attrstr.cpp
	windows/box.cpp
	windows/button.cpp
	windows/checkbox.cpp
	windows/colorbutton.cpp
	windows/colordialog.cpp
	windows/combobox.cpp
	windows/container.cpp
	windows/control.cpp
	windows/d2dscratch.cpp
	windows/datetimepicker.cpp
	windows/debug.cpp
	windows/draw.cpp
	windows/drawmatrix.cpp
	windows/drawpath.cpp
	windows/drawtext.cpp
	windows/dwrite.cpp
	windows/editablecombo.cpp
	windows/entry.cpp
	windows/events.cpp
	windows/fontbutton.cpp
	windows/fontdialog.cpp
	windows/fontmatch.cpp
	windows/form.cpp
	windows/graphemes.cpp
	windows/grid.cpp
	windows/group.cpp
	windows/init.cpp
	windows/label.cpp
	windows/main.cpp
	windows/menu.cpp
	windows/multilineentry.cpp
	windows/opentype.cpp
	windows/parent.cpp
	windows/progressbar.cpp
	windows/radiobuttons.cpp
	windows/separator.cpp
	windows/sizing.cpp
	windows/slider.cpp
	windows/spinbox.cpp
	windows/stddialogs.cpp
	windows/tab.cpp
	windows/tabpage.cpp
	windows/text.cpp
	windows/utf16.cpp
	windows/utilwin.cpp
	windows/window.cpp
	windows/winpublic.cpp
	windows/winutil.cpp
)
# resources.rc only contains the libui manifest.
# For a DLL, we have to include this directly, so we do so.
# Windows won't link resources in static libraries, so including this would have no effect.
# In those cases, we just need them to include the manifest with the executable (or link it directly into the output executable themselves); they can also customize the manifest as they see fit (assuming nothing breaks in the process).
# TODO make sure this gets added to both binary-only archives and install rules in this case
if(BUILD_SHARED_LIBS)
	list(APPEND _LIBUI_SOURCES
		windows/resources.rc
	)
endif()
set(_LIBUI_SOURCES ${_LIBUI_SOURCES} PARENT_SCOPE)

list(APPEND _LIBUI_INCLUDEDIRS
	windows
)
set(_LIBUI_INCLUDEDIRS _LIBUI_INCLUDEDIRS PARENT_SCOPE)

# TODO prune this list
set(_LIBUI_LIBS
	user32 kernel32 gdi32 comctl32 uxtheme msimg32 comdlg32 d2d1 dwrite ole32 oleaut32 oleacc uuid
PARENT_SCOPE)

if(NOT MSVC)
	if(BUILD_SHARED_LIBS)
		message(FATAL_ERROR
			"Sorry, but libui for Windows can currently only be built as a static library with MinGW. You will need to either build as a static library or switch to MSVC."
		)
	endif()
endif()

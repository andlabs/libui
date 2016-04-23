# 22 april 2015

CXXFILES += \
	windows/alloc.cpp \
	windows/area.cpp \
	windows/areadraw.cpp \
	windows/areaevents.cpp \
	windows/areascroll.cpp \
	windows/areautil.cpp \
	windows/box.cpp \
	windows/button.cpp \
	windows/checkbox.cpp \
	windows/child.cpp \
	windows/combobox.cpp \
	windows/container.cpp \
	windows/control.cpp \
	windows/d2dscratch.cpp \
	windows/datetimepicker.cpp \
	windows/debug.cpp \
	windows/draw.cpp \
	windows/drawtext.cpp \
	windows/dwrite.cpp \
	windows/entry.cpp \
	windows/events.cpp \
	windows/fontbutton.cpp \
	windows/fontdialog.cpp \
	windows/group.cpp \
	windows/init.cpp \
	windows/label.cpp \
	windows/main.cpp \
	windows/menu.cpp \
	windows/multilineentry.cpp \
	windows/parent.cpp \
	windows/progressbar.cpp \
	windows/radiobuttons.cpp \
	windows/resize.cpp \
	windows/separator.cpp \
	windows/slider.cpp \
	windows/spinbox.cpp \
	windows/stddialogs.cpp \
	windows/tab.cpp \
	windows/tabpage.cpp \
	windows/text.cpp \
	windows/utf16.cpp \
	windows/utilwin.cpp \
	windows/window.cpp \
	windows/winutil.cpp

HFILES += \
	windows/_uipriv_migrate.hpp \
	windows/area.hpp \
	windows/compilerver.hpp \
	windows/resources.hpp \
	windows/uipriv_windows.hpp \
	windows/winapi.hpp

RCFILES += \
	windows/resources.rc

# TODO split into a separate file or put in GNUmakefile.libui somehow?

# flags for the Windows API
# TODO prune this list
LDFLAGS += \
	user32.lib kernel32.lib gdi32.lib comctl32.lib uxtheme.lib msimg32.lib comdlg32.lib d2d1.lib dwrite.lib ole32.lib oleaut32.lib oleacc.lib uuid.lib

# flags for building a shared library
LDFLAGS += \
	-dll

# TODO flags for warning on undefined symbols

# no need for a soname

# TODO .def file

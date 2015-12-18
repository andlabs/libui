# 22 april 2015

CFILES += \
	windows/alloc.c \
	windows/area.c \
	windows/areadraw.c \
	windows/areaevents.c \
	windows/areascroll.c \
	windows/box.c \
	windows/button.c \
	windows/checkbox.c \
	windows/child.c \
	windows/combobox.c \
	windows/container.c \
	windows/control.c \
	windows/datetimepicker.c \
	windows/debug.c \
	windows/draw.c \
	windows/entry.c \
	windows/events.c \
	windows/group.c \
	windows/init.c \
	windows/label.c \
	windows/main.c \
	windows/menu.c \
	windows/multilineentry.c \
	windows/parent.c \
	windows/progressbar.c \
	windows/radiobuttons.c \
	windows/resize.c \
	windows/separator.c \
	windows/slider.c \
	windows/spinbox.c \
	windows/stddialogs.c \
	windows/tab.c \
	windows/tabpage.c \
	windows/text.c \
	windows/util.c \
	windows/utilwin.c \
	windows/window.c

HFILES += \
	windows/area.h \
	windows/compilerver.h \
	windows/resources.h \
	windows/uipriv_windows.h \
	windows/winapi.h

RCFILES += \
	windows/resources.rc

# TODO split into a separate file or put in GNUmakefile.libui somehow?

# flags for the Windows API
LDFLAGS += \
	user32.lib kernel32.lib gdi32.lib comctl32.lib uxtheme.lib msimg32.lib comdlg32.lib d2d1.lib ole32.lib oleaut32.lib oleacc.lib uuid.lib

# flags for building a shared library
LDFLAGS += \
	/dll

# TODO flags for warning on undefined symbols

# no need for a soname

# TODO .def file

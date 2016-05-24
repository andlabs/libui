# 22 april 2015

CFILES += \
	unix/alloc.c \
	unix/area.c \
	unix/box.c \
	unix/button.c \
	unix/checkbox.c \
	unix/child.c \
	unix/colorbutton.c \
	unix/combobox.c \
	unix/control.c \
	unix/datetimepicker.c \
	unix/debug.c \
	unix/draw.c \
	unix/drawmatrix.c \
	unix/drawpath.c \
	unix/drawtext.c \
	unix/editablecombo.c \
	unix/entry.c \
	unix/fontbutton.c \
	unix/group.c \
	unix/label.c \
	unix/main.c \
	unix/menu.c \
	unix/multilineentry.c \
	unix/progressbar.c \
	unix/radiobuttons.c \
	unix/separator.c \
	unix/slider.c \
	unix/spinbox.c \
	unix/stddialogs.c \
	unix/tab.c \
	unix/text.c \
	unix/util.c \
	unix/window.c

HFILES += \
	unix/draw.h \
	unix/uipriv_unix.h

# TODO split into a separate file or put in GNUmakefile.libui somehow?

# flags for GTK+
CFLAGS += \
	`pkg-config --cflags gtk+-3.0`
CXXFLAGS += \
	`pkg-config --cflags gtk+-3.0`
LDFLAGS += \
	`pkg-config --libs gtk+-3.0` -lm -ldl

# flags for building a shared library
# OS X does support -shared but it has a preferred name for this so let's use that there instead; hence this is not gcc-global
LDFLAGS += \
	-shared

# flags for warning on undefined symbols
# this is not gcc-global because OS X doesn't support these flags
# TODO figure out why FreeBSD follows linked libraries here
ifneq ($(shell uname -s),FreeBSD)
LDFLAGS += \
	-Wl,--no-undefined -Wl,--no-allow-shlib-undefined
endif

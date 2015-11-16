# 22 april 2015

CFILES += \
	unix/alloc.c \
	unix/area.c \
	unix/box.c \
	unix/button.c \
	unix/checkbox.c \
	unix/child.c \
	unix/combobox.c \
	unix/control.c \
	unix/datetimepicker.c \
	unix/draw.c \
	unix/entry.c \
	unix/group.c \
	unix/label.c \
	unix/main.c \
	unix/menu.c \
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
	unix/uipriv_unix.h

# thanks ebassi in irc.gimp.net/#gtk+
CFLAGS += \
	-D_UI_EXTERN='__attribute__((visibility("default"))) extern' \
	-fvisibility=hidden \
	-fPIC \
	`pkg-config --cflags gtk+-3.0`

LDFLAGS += \
	-fvisibility=hidden \
	-fPIC \
	`pkg-config --libs gtk+-3.0` -lm

# flags for warning on undefined symbols
LDFLAGS += \
	-Wl,--no-undefined -Wl,--no-allow-shlib-undefined

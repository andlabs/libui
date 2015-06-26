# 22 april 2015

osCFILES = \
	unix/alloc.c \
	unix/button.c \
	unix/checkbox.c \
	unix/combobox.c \
	unix/control.c \
	unix/container.c \
	unix/datetimepicker.c \
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
	unix/tab.c \
	unix/text.c \
	unix/util.c \
	unix/window.c

osHFILES = \
	unix/uipriv_unix.h

# thanks ebassi in irc.gimp.net/#gtk+
osCFLAGS = \
	-D_UI_EXTERN='__attribute__((visibility("default"))) extern' \
	-fvisibility=hidden \
	-fPIC \
	`pkg-config --cflags gtk+-3.0`

osLDFLAGS = \
	-fvisibility=hidden \
	-fPIC \
	`pkg-config --libs gtk+-3.0`

osLDWarnUndefinedFlags = -Wl,--no-undefined -Wl,--no-allow-shlib-undefined

osLIBSUFFIX = .so
osEXESUFFIX =

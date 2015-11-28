# 22 april 2015

CFILES += \
	windows/alloc.c \
	windows/area.c \
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
	windows/compilerver.h \
	windows/resources.h \
	windows/uipriv_windows.h \
	windows/winapi.h

RCFILES += \
	windows/resources.rc

# thanks ebassi in irc.gimp.net/#gtk+
CFLAGS += \
	-D_UI_EXTERN='__declspec(dllexport) extern'

LDFLAGS += \
	-static-libgcc \
	-luser32 -lkernel32 -lgdi32 -lcomctl32 -luxtheme -lmsimg32 -lcomdlg32 -ld2d1 -lole32 -loleaut32 -loleacc -luuid

# warnings on undefined symbols
LDFLAGS += \
	-Wl,--no-undefined -Wl,--no-allow-shlib-undefined

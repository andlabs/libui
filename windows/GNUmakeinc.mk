OSCFILES = \
	alloc.c \
	button.c \
	checkbox.c \
	comctl32.c \
	debug.c \
	entry.c \
	init.c \
	label.c \
	main.c \
	newcontrol.c \
	parent.c \
	tab.c \
	text.c \
	util.c \
	window.c

xLDFLAGS += \
	-luser32 -lkernel32 -lgdi32 -luxtheme -lmsimg32 -lcomdlg32 -lole32 -loleaut32 -loleacc -luuid

OUT = new.exe

ifeq ($(ARCH),64)
	CC = x86_64-w64-mingw32-gcc
	RC = x86_64-w64-mingw32-windres
	xCFLAGS += -m64
	xLDFLAGS += -m64
else
	CC = i686-w64-mingw32-gcc
	RC = i686-w64-mingw32-windres
	xCFLAGS += -m32
	xLDFLAGS += -m32
endif

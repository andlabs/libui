OSCFILES = \
	alloc.c \
	button.c \
	checkbox.c \
	entry.c \
	init.c \
	label.c \
	main.c \
	newcontrol.c \
	parent.c \
	tab.c \
	util.c \
	window.c

xCFLAGS += `pkg-config --cflags gtk+-3.0`
xLDFLAGS += `pkg-config --libs gtk+-3.0`

OUT = $(OUTBASE)

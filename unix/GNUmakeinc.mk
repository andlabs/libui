OSCFILES = \
	alloc.c \
	button.c \
	checkbox.c \
	entry.c \
	init.c \
	label.c \
	lifetimes.c \
	main.c \
	menu.c \
	newcontrol.c \
	parent.c \
	tab.c \
	text.c \
	util.c \
	window.c

xCFLAGS += `pkg-config --cflags gtk+-3.0`
xLDFLAGS += `pkg-config --libs gtk+-3.0`

OUT = $(OUTBASE)

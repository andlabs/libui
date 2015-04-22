# 22 april 2015

osCFILES = \
	unix/alloc.c \
	unix/main.c \
	unix/oscontainer.c \
	unix/util.c

osHFILES = \
	unix/uipriv_unix.h

# thanks ebassi in irc.gimp.net/#gtk+
osCFLAGS = \
	-D_UI_EXTERN='__attribute__((visibility("default"))) extern' \
	-fvisibility=hidden \
	`pkg-config --cflags gtk+-3.0`

osLDFLAGS = \
	-fvisibility=hidden \
	`pkg-config --libs gtk+-3.0`

osLIBSUFFIX = .so
osEXESUFFIX =

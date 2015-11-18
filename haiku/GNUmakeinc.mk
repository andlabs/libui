# 22 april 2015

CXXFILES += \
	haiku/alloc.cpp \
	haiku/control.cpp \
	haiku/main.cpp \
	haiku/text.cpp \
	haiku/util.cpp \
	haiku/window.cpp

HFILES += \
	haiku/uipriv_haiku.hpp

# thanks ebassi in irc.gimp.net/#gtk+
CXXFLAGS += \
	-D_UI_EXTERN='__attribute__((visibility("default"))) extern' \
	-fvisibility=hidden \
	-fPIC

LDFLAGS += \
	-fvisibility=hidden \
	-fPIC \
	-lbe

# flags for warning on undefined symbols
LDFLAGS += \
	-Wl,--no-undefined -Wl,--no-allow-shlib-undefined

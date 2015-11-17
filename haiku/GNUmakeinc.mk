# 22 april 2015

CFILES += \

HFILES += \
	haiku/uipriv_haiku.h

# thanks ebassi in irc.gimp.net/#gtk+
CFLAGS += \
	-D_UI_EXTERN='__attribute__((visibility("default"))) extern' \
	-fvisibility=hidden \
	-fPIC

LDFLAGS += \
	-fvisibility=hidden \
	-fPIC \
	-lbe -lm

# flags for warning on undefined symbols
LDFLAGS += \
	-Wl,--no-undefined -Wl,--no-allow-shlib-undefined

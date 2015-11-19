# 22 april 2015

CXXFILES += \
	haiku/alloc.cpp \
	haiku/area.cpp \
	haiku/box.cpp \
	haiku/button.cpp \
	haiku/checkbox.cpp \
	haiku/combobox.cpp \
	haiku/control.cpp \
	haiku/datetimepicker.cpp \
	haiku/draw.cpp \
	haiku/entry.cpp \
	haiku/group.cpp \
	haiku/label.cpp \
	haiku/main.cpp \
	haiku/menu.cpp \
	haiku/progressbar.cpp \
	haiku/radiobuttons.cpp \
	haiku/separator.cpp \
	haiku/slider.cpp \
	haiku/spinbox.cpp \
	haiku/stddialogs.cpp \
	haiku/tab.cpp \
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

# 28 may 2016

CXXFILES += \
	qt5/alloc.cpp \
	qt5/area.cpp \
	qt5/box.cpp \
	qt5/button.cpp \
	qt5/checkbox.cpp \

#	qt5/child.cpp \

CXXFILES += \
	qt5/colorbutton.cpp \
	qt5/combobox.cpp \
	qt5/control.cpp \
	qt5/datetimepicker.cpp \
	qt5/debug.cpp \
	qt5/draw.cpp \
	qt5/drawmatrix.cpp \
	qt5/drawpath.cpp \
	qt5/drawtext.cpp \
	qt5/editablecombo.cpp \
	qt5/entry.cpp \
	qt5/fontbutton.cpp \
	qt5/group.cpp \
	qt5/label.cpp \
	qt5/main.cpp \
	qt5/menu.cpp \
	qt5/multilineentry.cpp \
	qt5/progressbar.cpp \
	qt5/radiobuttons.cpp \
	qt5/separator.cpp \
	qt5/slider.cpp \
	qt5/spinbox.cpp \
	qt5/stddialogs.cpp \
	qt5/tab.cpp \
	qt5/text.cpp \

#	qt5/util.cpp \

CXXFILES += \
	qt5/window.cpp

HFILES += \
	qt5/draw.hpp \
	qt5/uipriv_qt5.hpp

# TODO split into a separate file or put in GNUmakefile.libui somehow?

# flags for Qt5
CFLAGS += $(NATIVE_UI_CFLAGS)
CXXFLAGS += $(NATIVE_UI_CXXFLAGS)
LDFLAGS += $(NATIVE_UI_LDFLAGS)


ifneq ($(RELEASE),1)
	CXXFLAGS += -DQT_DEBUG
else
	# disable Q_ASSERT checks
	CXXFLAGS += -DQT_NO_DEBUG
endif

# flags for building a shared library
# OS X does support -shared but it has a preferred name for this so let's use that there instead; hence this is not gcc-global
LDFLAGS += \
	-shared

# flags for warning on undefined symbols
# this is not gcc-global because OS X doesn't support these flags
# TODO figure out why FreeBSD follows linked libraries here
ifneq ($(shell uname -s),FreeBSD)
LDFLAGS += \
	-Wl,--no-undefined -Wl,--no-allow-shlib-undefined
endif

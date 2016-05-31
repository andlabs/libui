# 28 april 2015

MFILES += \
	darwin/alloc.m \
	darwin/area.m \
	darwin/areaevents.m \
	darwin/autolayout.m \
	darwin/box.m \
	darwin/button.m \
	darwin/checkbox.m \
	darwin/colorbutton.m \
	darwin/combobox.m \
	darwin/control.m \
	darwin/datetimepicker.m \
	darwin/debug.m \
	darwin/draw.m \
	darwin/drawtext.m \
	darwin/editablecombo.m \
	darwin/entry.m \
	darwin/fontbutton.m \
	darwin/group.m \
	darwin/label.m \
	darwin/main.m \
	darwin/map.m \
	darwin/menu.m \
	darwin/multilineentry.m \
	darwin/progressbar.m \
	darwin/radiobuttons.m \
	darwin/scrollview.m \
	darwin/separator.m \
	darwin/slider.m \
	darwin/spinbox.m \
	darwin/stddialogs.m \
	darwin/tab.m \
	darwin/text.m \
	darwin/util.m \
	darwin/window.m

HFILES += \
	darwin/uipriv_darwin.h

# LONGTERM split into a separate file or put in GNUmakefile.libui somehow?

# flags for Cocoa
LDFLAGS += $(NATIVE_UI_LDFLAGS)

# flags for OS X versioning
CFLAGS += \
	-mmacosx-version-min=10.8
CXXFLAGS += \
	-mmacosx-version-min=10.8
LDFLAGS += \
	-mmacosx-version-min=10.8

ifeq (,$(STATIC))
# flags for building a shared library
LDFLAGS += \
	-dynamiclib
endif

# on warning about undefined symbols:
# the gcc flags don't work with Apple's linker
# fortunately, we don't need any; Apple's linker warns about undefined symbols in -shared builds!

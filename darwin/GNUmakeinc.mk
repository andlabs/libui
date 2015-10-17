# 28 april 2015

MFILES += \
	darwin/alloc.m \
	darwin/area.m \
	darwin/areaevents.m \
	darwin/autolayout.m \
	darwin/box.m \
	darwin/button.m \
	darwin/checkbox.m \
	darwin/combobox.m \
	darwin/control.m \
	darwin/datetimepicker.m \
	darwin/draw.m \
	darwin/entry.m \
	darwin/group.m \
	darwin/label.m \
	darwin/main.m \
	darwin/map.m \
	darwin/menu.m \
	darwin/progressbar.m \
	darwin/radiobuttons.m \
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

CFLAGS += \
	-D_UI_EXTERN='__attribute__((visibility("default"))) extern' \
	-fvisibility=hidden \
	-mmacosx-version-min=10.7 \
	-DMACOSX_DEPLOYMENT_TARGET=10.7

LDFLAGS += \
	-fvisibility=hidden \
	-mmacosx-version-min=10.7 \
	-lobjc \
	-framework Foundation \
	-framework AppKit

# on warning about undefined symbols:
# the gcc flags don't work with Apple's linker
# fortunately, we don't need any; Apple's linker warns about undefined symbols in -shared builds!

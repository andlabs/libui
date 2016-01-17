set(APPLE_C_FILES
	darwin/alloc.m
	darwin/area.m
	darwin/areaevents.m
	darwin/autolayout.m
	darwin/box.m
	darwin/button.m
	darwin/checkbox.m
	darwin/combobox.m
	darwin/control.m
	darwin/datetimepicker.m
	darwin/draw.m
	darwin/drawtext.m
	darwin/entry.m
	darwin/group.m
	darwin/label.m
	darwin/main.m
	darwin/map.m
	darwin/menu.m
	darwin/multilineentry.m
	darwin/progressbar.m
	darwin/radiobuttons.m
	darwin/separator.m
	darwin/slider.m
	darwin/spinbox.m
	darwin/stddialogs.m
	darwin/tab.m
	darwin/text.m
	darwin/util.m
	darwin/window.m
)

set(APPLE_H_FILES
  	darwin/uipriv_darwin.h
)

set(APPLE_FILES ${APPLE_C_FILES} ${APPLE_H_FILES} PARENT_SCOPE)

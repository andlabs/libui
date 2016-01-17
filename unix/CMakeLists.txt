set(UNIX_C_FILES
	unix/alloc.c
	unix/area.c
	unix/box.c
	unix/button.c
	unix/checkbox.c
	unix/child.c
	unix/combobox.c
	unix/control.c
	unix/datetimepicker.c
	unix/draw.c
	unix/entry.c
	unix/group.c
	unix/label.c
	unix/main.c
	unix/menu.c
	unix/multilineentry.c
	unix/progressbar.c
	unix/radiobuttons.c
	unix/separator.c
	unix/slider.c
	unix/spinbox.c
	unix/stddialogs.c
	unix/tab.c
	unix/text.c
	unix/util.c
	unix/window.c
)

set(UNIX_H_FILES
	unix/uipriv_unix.h
)

set(UNIX_FILES ${UNIX_C_FILES} ${UNIX_H_FILES} PARENT_SCOPE)

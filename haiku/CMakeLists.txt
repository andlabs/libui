set(HAIKU_C_FILES
	haiku/alloc.cpp
	haiku/area.cpp
	haiku/box.cpp
	haiku/button.cpp
	haiku/checkbox.cpp
	haiku/combobox.cpp
	haiku/control.cpp
	haiku/datetimepicker.cpp
	haiku/draw.cpp
	haiku/entry.cpp
	haiku/group.cpp
	haiku/label.cpp
	haiku/main.cpp
	haiku/menu.cpp
	haiku/multilineentry.cpp
	haiku/progressbar.cpp
	haiku/radiobuttons.cpp
	haiku/separator.cpp
	haiku/singlechild.cpp
	haiku/slider.cpp
	haiku/spinbox.cpp
	haiku/stddialogs.cpp
	haiku/tab.cpp
	haiku/text.cpp
	haiku/util.cpp
	haiku/window.cpp
)

set(HAIKU_H_FILES
	haiku/uipriv_haiku.hpp
)

set(HAIKU_FILES ${HAIKU_C_FILES} ${HAIKU_H_FILES})

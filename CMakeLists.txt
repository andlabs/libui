cmake_minimum_required(VERSION 3.4)
project(libui VERSION 1.0.0)

set(COMMON_SOURCES
    common/areaevents.c
    common/control.c
    common/matrix.c
    common/menu.c
    common/ptrarray.c
    common/shouldquit.c
    common/types.c
    common/uipriv.h
    common/uthash/utarray.h
    common/uthash/uthash.h
    common/uthash/utlist.h
    common/uthash/utstring.h
    )

set(DARWIN_SOURCES
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
    darwin/uipriv_darwin.h
    darwin/util.m
    darwin/window.m
    )

set(UNIX_SOURCES
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
    unix/uipriv_unix.h
    unix/util.c
    unix/window.c
    )

set(WINDOWS_SOURCES
    windows/alloc.c
    windows/area.c
    windows/area.h
    windows/areadraw.c
    windows/areaevents.c
    windows/areascroll.c
    windows/areautil.c
    windows/box.c
    windows/button.c
    windows/checkbox.c
    windows/child.c
    windows/combobox.c
    windows/compilerver.h
    windows/container.c
    windows/control.c
    windows/datetimepicker.c
    windows/debug.c
    windows/draw.c
    windows/drawtext.cpp
    windows/entry.c
    windows/events.c
    windows/group.c
    windows/init.c
    windows/label.c
    windows/main.c
    windows/menu.c
    windows/multilineentry.c
    windows/notes
    windows/parent.c
    windows/progressbar.c
    windows/radiobuttons.c
    windows/resize.c
    windows/resources.h
    windows/resources.rc
    windows/separator.c
    windows/slider.c
    windows/spinbox.c
    windows/stddialogs.c
    windows/tab.c
    windows/tabpage.c
    windows/text.c
    windows/uipriv_windows.h
    windows/util.c
    windows/utilwin.c
    windows/winapi.h
    windows/window.c
    )

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

if(UNIX)
    add_compile_options(-Wall -Wextra -pedantic -Wno-unused-parameter -Wno-switch)
    set(UI_EXTERN __attribute__\(\(visibility\(\"default\"\)\)\)\ extern)
endif()

if(MSVC)
    add_compile_options(/W4 /wd4100 /TP /bigobj /nologo /RTC1 /RTCc /RTCs /RTCu)
    set(CMAKE_SHARED_LINKER_FLAGS /MANIFEST:NO)
    set(UI_EXTERN __declspec\(dllexport\)\ extern)
endif()

if(APPLE)
    set(UI_SOURCES ${DARWIN_SOURCES})
endif()

if (UNIX AND NOT APPLE)
endif()

if(WIN32)
    set(UI_SOURCES ${WINDOWS_SOURCES})
endif()

add_library(libui SHARED ${COMMON_SOURCES} ${UI_SOURCES})
target_compile_definitions(libui PRIVATE
    _UI_EXTERN=${UI_EXTERN}
    )
target_link_libraries(libui PRIVATE
    "$<$<PLATFORM_ID:Darwin>:-framework Foundation -framework AppKit>"
    "$<$<PLATFORM_ID:Windows>:user32;kernel32;gdi32;comctl32;uxtheme;msimg32;comdlg32;d2d1;dwrite;ole32;oleaut32;oleacc;uuid>"
    )

add_executable(controlgallery examples/controlgallery/main.c)
target_link_libraries(controlgallery PRIVATE libui)

add_executable(histogram examples/histogram/main.c)
target_link_libraries(histogram PRIVATE libui)

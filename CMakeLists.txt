cmake_minimum_required(VERSION 3.2)
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
    windows/entry.c
    windows/events.c
    windows/group.c
    windows/init.c
    windows/label.c
    windows/main.c
    windows/menu.c
    windows/multilineentry.c
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

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
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
    set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(LINUX TRUE)
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
    include(FindPkgConfig)
    pkg_search_module(GTK REQUIRED gtk+-3.0)
    set(UI_SOURCES ${UNIX_SOURCES})
endif()

if(WIN32)
    set(UI_SOURCES ${WINDOWS_SOURCES})
endif()

add_library(ui SHARED ${COMMON_SOURCES} ${UI_SOURCES})
set_target_properties(ui PROPERTIES
    VERSION ${PROJECT_VERSION_MINOR}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    )
target_compile_definitions(ui PRIVATE
    _UI_EXTERN=${UI_EXTERN}
    )
target_compile_options(ui PRIVATE
    "$<$<PLATFORM_ID:Linux>:${GTK_CFLAGS}>"
    )
target_link_libraries(ui PRIVATE
    "$<$<PLATFORM_ID:Linux>:${GTK_LDFLAGS}>"
    "$<$<PLATFORM_ID:Darwin>:-framework Foundation -framework AppKit>"
    "$<$<PLATFORM_ID:Windows>:user32;kernel32;gdi32;comctl32;uxtheme;msimg32;comdlg32;d2d1;dwrite;ole32;oleaut32;oleacc;uuid>"
    )

add_executable(controlgallery examples/controlgallery/main.c)
target_link_libraries(controlgallery PRIVATE ui)

add_executable(histogram examples/histogram/main.c)
target_link_libraries(histogram PRIVATE ui)

add_executable(tests
    test/drawtests.c
    test/main.c
    test/menus.c
    test/page1.c
    test/page2.c
    test/page3.c
    test/page4.c
    test/page5.c
    test/page6.c
    test/page7.c
    test/page7a.c
    test/page7b.c
    test/page7c.c
    test/page8.c
    test/page9.c
    test/resources.rc
    test/spaced.c
    test/test.h
    )
target_link_libraries(tests PRIVATE ui)

install(TARGETS ui controlgallery histogram
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    )

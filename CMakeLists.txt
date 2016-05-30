# Source: https://github.com/kainjow/libui-cmake

cmake_minimum_required(VERSION 2.8)

project(libui)

option(LIBUI_BUILD_EXAMPLES "Build the example projects" ON)
option(LIBUI_BUILD_TEST "Build the test project" ON)

set(LIBUI_SOURCES
    ui.h
    common/areaevents.c
    common/control.c
    common/controlsigs.h
    common/debug.c
    common/matrix.c
    common/shouldquit.c
    common/uipriv.h
    common/userbugs.c
)
if(APPLE)
    list(APPEND LIBUI_SOURCES
        ui_darwin.h
        darwin/alloc.m
        darwin/area.m
        darwin/areaevents.m
        darwin/autolayout.m
        darwin/box.m
        darwin/button.m
        darwin/checkbox.m
        darwin/colorbutton.m
        darwin/combobox.m
        darwin/control.m
        darwin/datetimepicker.m
        darwin/debug.m
        darwin/draw.m
        darwin/drawtext.m
        darwin/editablecombo.m
        darwin/entry.m
        darwin/fontbutton.m
        darwin/group.m
        darwin/label.m
        darwin/main.m
        darwin/map.m
        darwin/menu.m
        darwin/multilineentry.m
        darwin/progressbar.m
        darwin/radiobuttons.m
        darwin/scrollview.m
        darwin/separator.m
        darwin/slider.m
        darwin/spinbox.m
        darwin/stddialogs.m
        darwin/tab.m
        darwin/text.m
        darwin/util.m
        darwin/window.m
    )
endif()

add_library(libui ${LIBUI_SOURCES})

macro(use_libui TARGET)
    target_link_libraries(${TARGET} libui)
    if(APPLE)
        target_link_libraries(${TARGET} "-framework Cocoa")
    endif()
endmacro()

if(LIBUI_BUILD_EXAMPLES)
    add_executable(controlgallery MACOSX_BUNDLE examples/controlgallery/main.c)
    add_executable(cpp-multithread MACOSX_BUNDLE examples/cpp-multithread/main.cpp)
    add_executable(histogram MACOSX_BUNDLE examples/histogram/main.c)
    use_libui(controlgallery)
    use_libui(cpp-multithread)
    use_libui(histogram)
endif()

if(LIBUI_BUILD_TEST)
    # The target name "test" is reserved in CMake
    add_executable(tester MACOSX_BUNDLE
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
    	test/page10.c
    	test/page11.c
    	test/page12.c
    	test/page13.c
    	test/spaced.c
        test/test.h
    )
    use_libui(tester)
endif()

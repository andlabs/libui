
cmake_minimum_required(VERSION 2.8)

project(libui)

option(LIBUI_BUILD_EXAMPLES "Build the example projects" ON)
option(LIBUI_BUILD_TEST "Build the test project" ON)

set(LIBUI_COMMON_SOURCES
    ${CMAKE_SOURCE_DIR}/ui.h
    ${CMAKE_SOURCE_DIR}/common/areaevents.c
    ${CMAKE_SOURCE_DIR}/common/control.c
    ${CMAKE_SOURCE_DIR}/common/controlsigs.h
    ${CMAKE_SOURCE_DIR}/common/debug.c
    ${CMAKE_SOURCE_DIR}/common/matrix.c
    ${CMAKE_SOURCE_DIR}/common/shouldquit.c
    ${CMAKE_SOURCE_DIR}/common/uipriv.h
    ${CMAKE_SOURCE_DIR}/common/userbugs.c
)

if(APPLE)
    add_subdirectory(darwin)
endif()

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

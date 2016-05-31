
cmake_minimum_required(VERSION 2.8.3)

project(libui)

option(LIBUI_BUILD_EXAMPLES "Build the example projects" ON)
option(LIBUI_BUILD_TEST "Build the test project" ON)

set(LIBUI_COMMON_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/ui.h
    ${CMAKE_CURRENT_LIST_DIR}/common/areaevents.c
    ${CMAKE_CURRENT_LIST_DIR}/common/control.c
    ${CMAKE_CURRENT_LIST_DIR}/common/controlsigs.h
    ${CMAKE_CURRENT_LIST_DIR}/common/debug.c
    ${CMAKE_CURRENT_LIST_DIR}/common/matrix.c
    ${CMAKE_CURRENT_LIST_DIR}/common/shouldquit.c
    ${CMAKE_CURRENT_LIST_DIR}/common/uipriv.h
    ${CMAKE_CURRENT_LIST_DIR}/common/userbugs.c
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

add_subdirectory(examples)
add_subdirectory(test)


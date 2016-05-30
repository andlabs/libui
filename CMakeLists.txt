
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

add_subdirectory(examples)
add_subdirectory(test)


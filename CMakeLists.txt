project(ui)

cmake_minimum_required(VERSION 2.6)

option(BUILD_TEST "test" OFF)
option(NODEBUG "Disable Debug" OFF)

add_subdirectory(common)

set(PLATFORM_FILES)
set(LINKS)

set(UNIX_BASED_CFLAGS "-fPIC -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-switch --std=c99")
set(WINDOWS_CFLAGS "/W4 /wd4100 /TP /bigobj /nologo /RTC1 /RTCc /RTCs /RTCu")


if(WIN32)
  add_subdirectory(windows)
  list(APPEND PLATFORM_FILES ${WINDOWS_FILES})
  set(CFLAGS ${WINDOWS_CFLAGS})
  set(CXXFLAGS ${WINDOWS_CFLAGS})
  set(LINKS user32.lib kernel32.lib gdi32.lib comctl32.lib uxtheme.lib msimg32.lib comdlg32.lib d2d1.lib dwrite.lib ole32.lib oleaut32.lib oleacc.lib uuid.lib)

  if(NOT NODEBUG)
  list(APPEND CFLAGS "/Zi")
  list(APPEND CXXFLAGS "/Zi")
  list(APPEND LINKS "/debug")
  endif()
endif(WIN32)

if(UNIX)
  if(APPLE)
    set(CMAKE_MACOSX_RPATH 1)
    add_subdirectory(darwin)
    list(APPEND PLATFORM_FILES ${APPLE_FILES})
    set(LINKS objc "-framework Foundation" "-framework AppKit")
  elseif(HAIKU)
    add_subdirectory(haiku)
    list(APPEND PLATFORM_FILES ${HAIKU_FILES})
    set(LINKS be)
  else()
    add_subdirectory(unix)
    list(APPEND PLATFORM_FILES ${UNIX_FILES})
    set(CXXFLAGS "pkg-config --cflags gtk+-3.0")
    set(CFLAGS "pkg-config --cflags gtk+-3.0")
    set(LINKS m "pkg-config --libs gtk+-3.0")
  endif()

  list(APPEND CFLAGS ${UNIX_BASED_CFLAGS})
  list(APPEND CXXFLAGS ${UNIX_BASED_CFLAGS})

  if(NOT NODEBUG)
  list(APPEND CFLAGS "-g")
  list(APPEND CXXFLAGS "-g")
  list(APPEND LINKS "-g")
  endif()
endif(UNIX)


add_library(ui SHARED ${COMMON_FILES} ${PLATFORM_FILES})
target_link_libraries(ui ${LINKS})


if(BUILD_TEST)
  add_subdirectory(test)
  add_executable(uitest ${TEST_FILES})
  target_link_libraries(uitest ui)
endif(BUILD_TEST)

// 9 june 2015

// Visual Studio (Microsoft's compilers)
// VS2013 is needed for va_copy().
#ifdef _MSC_VER
#if _MSC_VER < 1800
#error Visual Studio 2013 or higher is required to build libui.
#endif
#endif

// MinGW
// Only MinGW-w64 is supported due to supporting APIs introduced after 2001.
// 4.0.0 is required due to its improved Vista support.
#ifdef __MINGW32__
#ifndef __MINGW64_VERSION_MAJOR
#error At present, only MinGW-w64 (>= 4.0.0) is supported. Other toolchains will be supported in the future (but other variants of MinGW will not).
#endif
#if __MINGW64_VERSION_MAJOR > 4
#define ui_good_mingw
#elif (__MINGW64_VERSION_MAJOR == 4) && (__MINGW64_VERSION_MINOR > 0)
#define ui_good_mingw
#endif
#ifndef ui_good_mingw
#error MinGW-w64 version 4.0.0 or newer is required.
#endif
// TODO exact version once my changes get pushed
#undef ui_good_mingw
#endif

// other compilers can be added here as necessary

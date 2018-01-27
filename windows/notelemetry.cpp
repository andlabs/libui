// This file desables MSVC CRT from calling telemetry functions.

// See: https://www.reddit.com/r/cpp/comments/4ibauu/visual_studio_adding_telemetry_function_calls_to/d2wrf57/
// See also: C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.12.25827\crt\src\linkopts\notelemetry.cpp

#include <windows.h>

extern "C"
{
    void __cdecl __vcrt_initialize_telemetry_provider() { }
    void __cdecl __vcrt_uninitialize_telemetry_provider() { }
    void __cdecl __telemetry_main_invoke_trigger() { }
    void __cdecl __telemetry_main_return_trigger() { }
    void __cdecl __telemetry_main_invoke_trigger(HINSTANCE) { }
    void __cdecl __telemetry_main_return_trigger(HINSTANCE) { }
}
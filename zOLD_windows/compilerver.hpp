// 9 june 2015

// Visual Studio (Microsoft's compilers)
// VS2013 is needed for va_copy().
#ifdef _MSC_VER
#if _MSC_VER < 1800
#error Visual Studio 2013 or higher is required to build libui.
#endif
#endif

// LONGTERM MinGW

// other compilers can be added here as necessary

/* TODO this should not be necessary, but I don't know

here's @bcampbell's original comment:
// sanity check - make sure wchar_t is 16 bits (the assumption on windows)
// (MinGW-w64 gcc does seem to define a 16bit wchar_t, but you never know. Other windows gcc ports might not)

here's what I got when I tried to investigate on irc.oftc.net/#mingw-w64:
{
[08:45:20]  <lh_mouse>	andlabs, the c++ standard requires `wchar_t` to be a distinct type. On Windows you can simply `static_assert(sizeof(wchar_t) == sizeof(unsigned short) && alignof(wchar_t) == alignof(unsigned short), "");`  then reinterpret_cast those pointers.
[09:22:16]  <andlabs>	lh_mouse: yes; that was the point of my question =P but whether that static_assert is always true is another question I have, because again, windows embeds the idea of wchar_t being UTF-16 throughout the API, but when I went to look, I found that the clang developers had a very heated debate about it :S
[09:22:28]  <andlabs>	and I couldn't find any concrete information other than the msvc docs
[09:23:04]  <lh_mouse>	Since Windows 2000 the NT kernel uses UTF-16.
[09:23:50]  <lh_mouse>	If you don't care about Windows 9x you can just pretend non-UTF-16 APIs didn't exist.
[09:24:04]  <andlabs>	that's not what I meant
[09:24:06]  <lh_mouse>	Actually long long long ago Windows used UCS2.
[09:24:15]  <andlabs>	I meant whether sizeof(wchar_t) must necessarily equal sizeof(uint16_t)
[09:24:18]  <andlabs>	and likewise for alignof
[09:24:27]  <andlabs>	for all windows compilers
[09:24:29]  <andlabs>	anyway afk
[09:24:31]  <lh_mouse>	Yes. That is what the ABI says.
[09:24:40]  <andlabs>	is there a source for that I can point at other people
[09:24:45]  <lh_mouse>	the ABI != on Windows
[09:26:00]  <andlabs>	okay I really need to afk now but I was about to ask what you meant
[09:26:06]  <andlabs>	and by source I meant URL
[09:49:18]  <m2bot>	andlabs: Sent 17 minutes ago: <lh_mouse> Here is what Microsoft people describe `wchar_t`: https://docs.microsoft.com/en-us/cpp/cpp/char-wchar-t-char16-t-char32-t
[09:49:19]  <m2bot>	andlabs: Sent 17 minutes ago: <lh_mouse> It is quite guaranteed: 'In the Microsoft compiler, it represents a 16-bit wide character used to store Unicode encoded as UTF-16LE, the native character type on Windows operating systems.'
[09:50:08]  <lh_mouse>	andlabs, If you build for cygwin then `wchar_t` is probably `int`, just like what it is on Linux.
[09:51:00]  <andlabs>	yes but that's still a compiler-specific reference; I still don't know hwere Microsoft keeps its ABI documentation, and I'm still wondering what you mean by "the ABI != on Windows" with regards to establishing that guarantee
[09:52:13]  <lh_mouse>	This is already the ABI documentation:   https://docs.microsoft.com/en-us/cpp/cpp/char-wchar-t-char16-t-char32-t    
[09:52:15]  <m2bot>	Title: char, wchar_t, char16_t, char32_t | Microsoft Docs (at docs.microsoft.com)
[09:52:19]  <lh_mouse>	It describes C++ types,
[09:53:09]  <andlabs>	oh, ok
[09:54:47]  <andlabs>	I assume by the != statement you mean code that doesn't make any windows API calls can theoretically be compiled any which way, right
[09:55:05]  <lh_mouse>	yes. think about MSYS and Cygwin.
[09:55:21]  <lh_mouse>	They have 8-byte `long` and 4-byte `wchar_t`.
[09:57:37]  <andlabs>	right, except the code I'm trying to compile does use the Windows API, so that wouldn't apply to me
[09:57:43]  <andlabs>	I assume
[09:57:53]  <lh_mouse>	it wouldn't.
[09:59:12]  <lh_mouse>	On Windows it is sometimes necessary to assume specific ABI definition. For example, when a callback function returning a `DWORD` is to be declared in a header, in order to prevent `#include`'ing windows.h, you can just write `unsigned long` there.
[09:59:32]  <lh_mouse>	This is guaranteed to work on Windows. Linux will say otherwise.
[10:00:41]  <lh_mouse>	We all know `#include <windows.h>` in a public header lets the genie out of the bottle, doesn't it?
[10:04:24]  <andlabs>	the zombie of win32_lean_and_mean lives forever
[10:04:53]  <andlabs>	of course now we have stdint.h and cstdint (which took longer because lolC++03) which helps stuff
[10:06:19]  <lh_mouse>	no `uint32_t` is `unsigned int` while `DWORD` is `unsigned long` hence they are incompatible. :(
[10:06:39]  <andlabs>	in what sense
[10:06:55]  <lh_mouse>	a `unsigned int *` cannot be converted to `unsigned long *` implicitly.
[10:07:41]  <lh_mouse>	the C standard says they are distinct pointer types and are not compatible, although `unsigned int` and `unsigned long` might have the same bit representation and alignment requirement.
[10:08:04]  <andlabs>	oh
[10:08:22]  <lh_mouse>	casting would indeed make code compile, but I tend to keep away from them unless necessary.
[10:08:24]  <andlabs>	wel yeah, but we haven't left the world of windows-specific code yet
[10:08:38]  <andlabs>	my point was more we don't need to use names like DWORD anymore
[10:08:51]  <andlabs>	of course it's easier to do so
[10:09:04]  <lh_mouse>	just use `uint32_t`.
[10:09:44]  <lh_mouse>	I just tested GCC 8 today and noticed they had added a warning for casting between incompatible function pointer types.
[10:10:10]  <lh_mouse>	So casting from `unsigned (*)(void)` to `unsigned long (*)(void)` now results in a warning.
[10:10:43]  <lh_mouse>	With `-Werror` it is a hard error. This can be worked around by casting the operand to an intermediate result of `intptr_t`.
[10:10:59]  <lh_mouse>	... not so serious.
[10:11:42]  <andlabs>	oh good I wonder what else will break :D
[10:12:19]  <andlabs>	though the docs for dlsym() tell you what you should do instead for systems that use libdl (cast the address of your destination variable to void**)
[10:13:23]  <lh_mouse>	POSIX requires casting from `void *` to function pointers to work explicitly (see the docs for `dlsym()`). I am not sure what GCC people think about it.
[10:13:45]  <andlabs>	yes that's what I just said =P it avoids the problem entirely
[10:13:49]  <lh_mouse>	C++ says this is 'conditionally supported' and it is not a warning or error there.
[10:13:50]  <andlabs>	dlsym already returns void*
[10:14:13]  <andlabs>	something like dlsym would require an ABI guarantee on the matter anyway
[10:14:16]  <andlabs>	by definition
[10:14:32]  <lh_mouse>	Casting is evil. Double casting is double evil. So I keep myself away from them.
[10:15:03]  <andlabs>	sadly this is C (and C++) =P
[10:15:25]  <lh_mouse>	for `*-w64-mingw32` targets it is safe to cast between `unsigned short`, `wchar_t` and `char16_t`.
[10:15:33]  <lh_mouse>	as well as pointers to them.
[10:16:30]  <lh_mouse>	you just need to `static_assert` it, so something naughty will not compile.
[12:36:14]  <andlabs>	actually I didn't notice that last message until just now
[12:36:23]  <andlabs>	I was asking because I was sitting here thinking such a static_assert was unnecessary
}
clang debate: http://clang-developers.42468.n3.nabble.com/Is-that-getting-wchar-t-to-be-32bit-on-win32-a-good-idea-for-compatible-with-Unix-world-by-implement-td4045412.html

so I'm not sure what is correct, but I do need to find out
*/
#include <limits.h>
#if WCHAR_MAX > 0xFFFF
#error unexpected: wchar_t larger than 16-bit on a Windows ABI build; contact andlabs with your build setup information
#endif

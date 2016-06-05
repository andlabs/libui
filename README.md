# libui: a portable GUI library for C

This README is being written.<br>
[![Build Status](https://travis-ci.org/andlabs/libui.png)](https://travis-ci.org/andlabs/libui)
*(currently failing because the version of cmake that Travis uses treats Objective-C files as C++; if you know the fix please file a PR)*

## Announcements

* **5 June 2016**
	* **Alpha 3.1 is here.** This was a much-needed update to Alpha 3 that changes a few things:
		* **The build system is now cmake.** cmake 2.8.11 or higher is needed.
		* Static linking is now fully possible.
		* MinGW linking is back, but static only.

* **29 May 2016**
	* **Alpha 3 is here!** Get it [here](https://github.com/andlabs/libui/releases/tag/alpha3).
	* The next packaged release will introduce:
		* uiGrid, another way to lay out controls, a la GtkGrid
		* uiOpenGLArea, a way to render OpenGL content in a libui uiArea
		* uiTable, a data grid control that may or may not have tree facilities (if it does, it will be called uiTree instead)
		* a complete, possibly rewritten, drawing and text rendering infrastructure

* **24 May 2016**
	* You can now help choose [a potential new build system for libui](https://github.com/andlabs/libui/issues/62).
	* Tomorrow I will decide if OS X 10.7 will also be dropped alongside GTK+ 3.4-3.8 this Saturday. Stay tuned.

* **22 May 2016**
	* Two more open questions I'd like your feedback on are available [here](https://github.com/andlabs/libui/issues/48) and [here](https://github.com/andlabs/libui/issues/25).
	* Sometime in the next 48 hours (before 23:59 EDT on 24 May 2016) I will split `uiCombobox` into two separate controls, `uiCombobox` and `uiEditableCombobox`, each with slightly different events and "selected item" mechanics. Prepare your existing code.

* **21 May 2016**
	* I will now post announcements and updates here.
	* Now that Ubuntu 16.04 LTS is here, no earlier than next Saturday, 28 May 2016 at noon EDT, **I will bump the minimum GTK+ version from 3.4 to 3.10**. This will add a lot of new features that I can now add to libui, such as search-oriented uiEntries, lists of arbitrary control layouts, and more. If you are still running a Linux distribution that doesn't come with 3.10, you will either need to upgrade or use jhbuild to set up a newer version of GTK+ in a private environment.
	* You can decide if I should also drop OS X 10.7 [here](https://github.com/andlabs/libui/issues/46).

## Updates

*Note that today's entry may be updated later today.*

* **29 May 2016**
	* Thanks to @pcwalton, we can now statically link libui! Simply do `make STATIC=1` instead of just `make`.
		* On Windows you must link both `libui.lib` and `libui.res` AND provide a Common Controls 6 manifest for output static binaries to work properly.

* **28 May 2016**
	* As promised, **the minimum system requirements are now OS X 10.8 and GTK+ 3.10 for OS X and Unix, respectively**.

* **26 May 2016**
	* Two OS X-specific functions have been added: `uiDarwinMarginAmount()` and `uiDarwinPaddingAmount()`. These return the amount of margins and padding, respectively, to give to a control, and are intended for container implementations. These are suitable for the constant of a NSLayoutConstraint. They both take a pointer parameter that is reserved for future use and should be `NULL`.

* **25 May 2016**
	* uiDrawTextLayout attributes are now specified in units of *graphemes* on all platforms. This means characters as seen from a user's perspective, not Unicode codepoints or UTF-8 bytes. So a long string of combining marker codepoints after one codepoint would still count as one grapheme.

* **24 May 2016**
	* As promised, `uiCombobox` is now split into `uiCombobox` for non-editable comboboxes and `uiEditableCombobox` for editable comboboxes. Mind the function changes as well :)
	* There is a new function `uiMainStep()`, which runs one iteration of the main loop. It takes a single boolean argument, indicating whether to wait for an event to occur or not. It returns true if an event was processed (or if no event is available if you don't want to wait) and false if the event loop was told to stop (for instance, `uiQuit()` was called).

* **23 May 2016**
	* Fixed surrogate pair drawing on OS X.

* **22 May 2016**
	* Removed `uiControlVerifyDestroy()`; that is now part of `uiFreeControl()` itself.
	* Added `uiPi`, a constant for π. This is provided for C and C++ programmers, where there is no standard named constant for π; bindings authors shouldn't need to worry about this.
	* Fixed uiMultilineEntry not properly having line breaks on Windows.
	* Added `uiNewNonWrappingMultilineEntry()`, which creates a uiMultilineEntry that scrolls horizontally instead of wrapping lines. (This is not documented as being changeable after the fact on Windows, hence it's a creation-time choice.)
	* uiAreas on Windows and some internal Direct2D areas now respond to `WM_PRINTCLIENT` properly, which should hopefully increase the quality of screenshots.
	* uiDateTimePicker on GTK+ works properly on RTL layouts and no longer disappears off the bottom of the screen if not enough room is available. It will also no longer be marked for localization of the time format (what the separator should be and whether to use 24-hour time), as that information is not provided by the locale system. :(
	* Added `uiUserBugCannotSetParentOnToplevel()`, which should be used by implementations of toplevel controls in their `SetParent()` implementations. This will also be the beginning of consolidating common user bug messages into a single place, though this will be one of the only few exported user bug functions.
	* uiSpinbox and uiSlider now merely swap their min and max if min ≥ max. They will no longer panic and do nothing, respectively.
	* Matrix scaling will no longer leave the matrix in an invalid state on OS X and GTK+.
	* `uiMultilineEntrySetText()` and `uiMutlilineEntryAppend()` on GTK+ no longer fire `OnChanged()` events.

## Runtime Requirements

* Windows: Windows Vista SP2 with Platform Update or newer
* Unix: GTK+ 3.10 or newer
* Mac OS X: OS X 10.8 or newer

## Build Requirements

* All platforms:
	* CMake 2.8.11 or newer
* Windows: either
	* Microsoft Visual Studio 2013 or newer (2013 is needed for `va_copy()`) — you can build either a static or a shared library
	* MinGW-w64 (other flavors of MinGW may not work) — **you can only build a static library**; shared library support will be re-added once the following features come in:
		* [Isolation awareness](https://msdn.microsoft.com/en-us/library/aa375197%28v=vs.85%29.aspx), which is how you get themed controls from a DLL without needing a manifest
* Unix: nothing else specific
* Mac OS X: nothing else specific, so long as you can build Cocoa programs

## Building

Out-of-tree builds typical of cmake are preferred:

```
$ mkdir build
$ cd build
$ cmake ..
```

Pass `-DBUILD_SHARED_LIBS=OFF` to `cmake` to build a static library. The standard cmake build configurations are provided; if none is specified, `Debug` is used.

If you use a makefile generator with cmake, then

```
$ make
$ make tester         # for the test program
$ make examples       # for examples
```

and pass `VERBOSE=1` to see build commands. Build targets will be in the `build/out` folder.

Project file generators should work, but are untested by me.

On Windows, I use the `Unix Makefiles` generator and GNU make (built using the `build_w32.bat` script included in the source and run in the Visual Studio command line). In this state, if MinGW-w64 (either 32-bit or 64-bit) is not in your `%PATH%`, cmake will use MSVC by default; otherwise, cmake will use with whatever MinGW-w64 is in your path. `set PATH=%PATH%;c:\msys2\mingw(32/64)\bin` should be enough to temporarily change to a MinGW-w64 build for the current command line session only if you installed MinGW-w64 through [MSYS2](https://msys2.github.io/); no need to change global environment variables constantly.

## Documentation

Needs to be written. Consult ui.h and the examples for details for now.

## Language Bindings

libui was originally written as part of my [package ui for Go](https://github.com/andlabs/ui). Now that libui is separate, package ui has become a binding to libui. As such, package ui is the only official binding.

Other people have made bindings to other languages:

Language | Bindings
--- | ---
C#/.net | [LibUI.Binding](https://github.com/NattyNarwhal/LibUI.Binding), [SharpUI](https://github.com/benpye/sharpui/)
Crystal | [libui.cr](https://github.com/Fusion/libui.cr)
D | [DerelictLibui](https://github.com/Extrawurst/DerelictLibui)
Haskell | [libui-haskell](https://github.com/ajnsit/libui-haskell)
JavaScript | [libui.js (merged into libui-node?)](https://github.com/mavenave/libui.js)
Julia | [Libui.jl](https://github.com/joa-quim/Libui.jl)
Lua | [libuilua](https://github.com/zevv/libuilua), [libui-lua](https://github.com/mdombroski/libui-lua)
Node.js | [libui-node](https://github.com/parro-it/libui-node)
Python | [pylibui](https://github.com/joaoventura/pylibui)
Ruby | [libui-ruby](https://github.com/jamescook/libui-ruby)
Rust | [libui-rs](https://github.com/pcwalton/libui-rs)

## Screenshots

From examples/controlgallery:

![Windows](examples/controlgallery/windows.png)

![Unix](examples/controlgallery/unix.png)

![OS X](examples/controlgallery/darwin.png)

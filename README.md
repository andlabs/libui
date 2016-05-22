# libui: a portable GUI library for C

This README is being written.<br>
[![Build Status](https://travis-ci.org/andlabs/libui.png)](https://travis-ci.org/andlabs/libui)

## Announcements

**21 May 2016**
* I will now post announcements and updates here.
* Now that Ubuntu 16.04 LTS is here, no earlier than next Saturday, 28 May 2016 at noon EDT, **I will bump the minimum GTK+ version from 3.4 to 3.10**. This will add a lot of new features that I can now add to libui, such as search-oriented uiEntries, lists of arbitrary control layouts, and more. If you are still running a Linux distribution that doesn't come with 3.10, you will either need to upgrade or use jhbuild to set up a newer version of GTK+ in a private environment.
* You can decide if I should also drop OS X 10.7 [here](https://github.com/andlabs/libui/issues/46).

## Updates

(none yet)

## Runtime Requirements

* Windows: Windows Vista SP2 with Platform Update or newer
* Unix: GTK+ 3.4 or newer
* Mac OS X: OS X 10.7 or newer

## Build Requirements

* All platforms:
	* GNU make 3.81 or newer (Xcode comes with this; on Windows you will need to get it yourself)
* Windows: Microsoft Visual Studio 2013 or newer (2013 is needed for `va_copy()`)
	* MinGW is currently unsupported. MinGW-w64 support will be re-added once the following features come in:
		* [Isolation awareness](https://msdn.microsoft.com/en-us/library/aa375197%28v=vs.85%29.aspx)
		* Linker symbols for some functions such as `TaskDialog()` (which I thought I submitted...)
* Unix: nothing specific
* Mac OS X: nothing specific, so long as you can build Cocoa programs

(TODO write some notes on make variables and cross-compiling)

## Documentation

Needs to be written. Consult ui.h and the examples for details for now.

## Language Bindings

libui was originally written as part of my [package ui for Go](https://github.com/andlabs/ui). Now that libui is separate, package ui has become a binding to libui. As such, package ui is the only official binding.

Other people have made bindings to other languages:
* C#/.NET: [LibUI.Binding](https://github.com/NattyNarwhal/LibUI.Binding)
* D: [DerelictLibui](https://github.com/Extrawurst/DerelictLibui)
* Haskell: [libui-haskell](https://github.com/ajnsit/libui-haskell)
* Lua: [libuilua](https://github.com/zevv/libuilua)
* Rust: [libui-rs](https://github.com/pcwalton/libui-rs)

* TODO list more here

## Screenshots

From examples/controlgallery:

![Windows](examples/controlgallery/windows.png)

![Unix](examples/controlgallery/unix.png)

![OS X](examples/controlgallery/darwin.png)
